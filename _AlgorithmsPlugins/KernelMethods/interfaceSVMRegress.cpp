/*********************************************************************
MLDemos: A User-Friendly visualization toolkit for machine learning
Copyright (C) 2010  Basilio Noris
Contact: mldemos@b4silio.com

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public License,
version 3 as published by the Free Software Foundation.

This library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free
Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*********************************************************************/
#include "interfaceSVMRegress.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>

using namespace std;

RegrSVM::RegrSVM()
{
    params = new Ui::ParametersRegr();
    params->setupUi(widget = new QWidget());
    connect(params->svmTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeOptions()));
    connect(params->kernelTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeOptions()));
    ChangeOptions();
}

RegrSVM::~RegrSVM()
{
    delete params;
}

void RegrSVM::ChangeOptions()
{
    params->svmCLabel->setText("C");
    params->svmPSpin->setRange(0.0001, 1.0);
    params->svmPSpin->setSingleStep(0.01);
    params->svmPSpin->setDecimals(4);
    params->svmCSpin->setEnabled(true);
    params->svmCSpin->setRange(0.1, 9999.9);
    params->svmCSpin->setDecimals(1);
    params->optimizeCheck->setVisible(true);
    switch(params->svmTypeCombo->currentIndex())
    {
    case 0: // C-SVM
        params->svmEpsLabel->setText("eps");
        params->svmPSpin->setRange(0.0001, 100.0);
        if(params->kernelTypeCombo->count() < 4) params->kernelTypeCombo->addItem("Sigmoid");
        break;
    case 1: // Nu-SVM
        params->svmEpsLabel->setText("Nu");
        if(params->kernelTypeCombo->count() < 4) params->kernelTypeCombo->addItem("Sigmoid");
        break;
    case 2: // KRLS
        params->optimizeCheck->setVisible(false);
        params->svmEpsLabel->setText("Tolerance");
        params->svmCLabel->setText("Capacity");
        params->svmCSpin->setRange(0, 1000);
        params->svmCSpin->setDecimals(0);
        params->svmPSpin->setRange(0.0001, 1.0);
        params->svmPSpin->setSingleStep(0.001);
        params->svmPSpin->setDecimals(4);
        if(params->kernelTypeCombo->count() > 3) params->kernelTypeCombo->removeItem(3);
        break;
    }
    switch(params->kernelTypeCombo->currentIndex())
    {
    case 0: // linear
        params->kernelDegSpin->setVisible(false);
        params->labelDegree->setVisible(false);
        params->kernelWidthSpin->setVisible(false);
        params->labelWidth->setVisible(false);
        break;
    case 1: // poly
        params->kernelDegSpin->setVisible(true);
        params->labelDegree->setVisible(true);
        params->kernelWidthSpin->setVisible(false);
        params->labelWidth->setVisible(false);
        break;
    case 2: // RBF
        params->kernelDegSpin->setVisible(false);
        params->labelDegree->setVisible(false);
        params->kernelWidthSpin->setVisible(true);
        params->labelWidth->setVisible(true);
        break;
    case 3: // SIGMOID
        params->kernelDegSpin->setEnabled(false);
        params->labelDegree->setVisible(false);
        params->kernelWidthSpin->setEnabled(true);
        params->labelWidth->setVisible(true);
        break;
    }
}

void RegrSVM::SetParams(Regressor *regressor)
{
    if(!regressor) return;
    int kernelMethod = params->svmTypeCombo->currentIndex();
    float svmC = params->svmCSpin->value();
    int kernelType = params->kernelTypeCombo->currentIndex();
    float kernelGamma = params->kernelWidthSpin->value();
    float kernelDegree = params->kernelDegSpin->value();
    float svmP = params->svmPSpin->value();
    bool bOptimize = params->optimizeCheck->isChecked();

    if(kernelMethod == 2) // KRLS
    {
        RegressorKRLS *krls = (RegressorKRLS*)regressor;
        int capacity = svmC;
        double epsilon = svmP;
        krls->SetParams(epsilon, capacity, kernelType, kernelGamma, kernelDegree);
    }
    else
    {
        RegressorSVR *svm = (RegressorSVR*)regressor;
        switch(kernelMethod)
        {
        case 0:
            svm->param.svm_type = EPSILON_SVR;
            break;
        case 1:
            svm->param.svm_type = NU_SVR;
            break;
        }
        switch(kernelType)
        {
        case 0:
            svm->param.kernel_type = LINEAR;
            break;
        case 1:
            svm->param.kernel_type = POLY;
            svm->param.gamma = 1;
            break;
        case 2:
            svm->param.kernel_type = RBF;
            svm->param.gamma = 1 / kernelGamma;
            break;
        case 3:
            svm->param.kernel_type = SIGMOID;
            svm->param.gamma = 1 / kernelGamma;
            svm->param.coef0 = 0;
            break;
        }
        svm->param.C = svmC;
        svm->param.nu = svmP;
        svm->param.p = svmP;
        svm->param.degree = kernelDegree;
        svm->param.coef0 = 0;
        svm->bOptimize = bOptimize;
    }
}

fvec RegrSVM::GetParams()
{
    int kernelMethod = params->svmTypeCombo->currentIndex();
    float svmC = params->svmCSpin->value();
    int kernelType = params->kernelTypeCombo->currentIndex();
    float kernelGamma = params->kernelWidthSpin->value();
    float kernelDegree = params->kernelDegSpin->value();
    float svmP = params->svmPSpin->value();
    bool bOptimize = params->optimizeCheck->isChecked();

    fvec par(7);
    par[0] = kernelMethod;
    par[1] = svmC;
    par[2] = kernelType;
    par[3] = kernelGamma;
    par[4] = kernelDegree;
    par[5] = svmP;
    par[6] = bOptimize;
    return par;
}

void RegrSVM::SetParams(Regressor *regressor, fvec parameters)
{
    if(!regressor) return;
    int kernelMethod = parameters.size() > 0 ? parameters[0] : 1;
    float svmC = parameters.size() > 1 ? parameters[1] : 1;
    int kernelType = parameters.size() > 2 ? parameters[2] : 0;
    float kernelGamma = parameters.size() > 3 ? parameters[3] : 0.1;
    int kernelDegree = parameters.size() > 4 ? parameters[4] : 1;
    float svmP = parameters.size() > 5 ? parameters[5] : 0.1;
    bool bOptimize = parameters.size() > 6 ? parameters[6] : 0;

    if(kernelMethod == 2) // KRLS
    {
        RegressorKRLS *krls = (RegressorKRLS*)regressor;
        int capacity = svmC;
        double epsilon = svmP;
        krls->SetParams(epsilon, capacity, kernelType, kernelGamma, kernelDegree);
    }
    else
    {
        RegressorSVR *svm = (RegressorSVR*)regressor;
        switch(kernelMethod)
        {
        case 0:
            svm->param.svm_type = EPSILON_SVR;
            break;
        case 1:
            svm->param.svm_type = NU_SVR;
            break;
        }
        switch(kernelType)
        {
        case 0:
            svm->param.kernel_type = LINEAR;
            break;
        case 1:
            svm->param.kernel_type = POLY;
            svm->param.gamma = 1;
            break;
        case 2:
            svm->param.kernel_type = RBF;
            svm->param.gamma = 1 / kernelGamma;
            break;
        case 3:
            svm->param.kernel_type = SIGMOID;
            svm->param.gamma = 1 / kernelGamma;
            svm->param.coef0 = 0;
            break;
        }
        svm->param.C = svmC;
        svm->param.nu = svmP;
        svm->param.p = svmP;
        svm->param.degree = kernelDegree;
        svm->param.coef0 = 0;
        svm->bOptimize = bOptimize;
    }
}

void RegrSVM::GetParameterList(std::vector<QString> &parameterNames,
                                std::vector<QString> &parameterTypes,
                                std::vector< std::vector<QString> > &parameterValues)
{
    parameterNames.push_back("Kernel Method");
    parameterNames.push_back("Penalty (C)");
    parameterNames.push_back("Kernel Type");
    parameterNames.push_back("Kernel Width");
    parameterNames.push_back("Kernel Degree");
    parameterNames.push_back("Epsilon / Nu");
    parameterNames.push_back("Optimize Kernel");
    parameterTypes.push_back("List");
    parameterTypes.push_back("Real");
    parameterTypes.push_back("List");
    parameterTypes.push_back("Real");
    parameterTypes.push_back("Integer");
    parameterTypes.push_back("Real");
    parameterTypes.push_back("List");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("Epsilon-SVR");
    parameterValues.back().push_back("Nu-SVR");
    parameterValues.back().push_back("Kernel Ridge Least Squares");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("0.00000001f");
    parameterValues.back().push_back("99999999999999");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("Linear");
    parameterValues.back().push_back("Poly");
    parameterValues.back().push_back("RBF");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("0.00000001f");
    parameterValues.back().push_back("9999999");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("1");
    parameterValues.back().push_back("150");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("0.0000000001f");
    parameterValues.back().push_back("9999999");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("False");
    parameterValues.back().push_back("True");
}

QString RegrSVM::GetAlgoString()
{
    int kernelMethod = params->svmTypeCombo->currentIndex();
    float svmC = params->svmCSpin->value();
    int kernelType = params->kernelTypeCombo->currentIndex();
    float kernelGamma = params->kernelWidthSpin->value();
    float kernelDegree = params->kernelDegSpin->value();
    float svmP = params->svmPSpin->value();

    QString algo;
    switch(kernelMethod)
    {
    case 0:
        algo += "eps-SVM";
        algo += QString(" %1 %2").arg(svmC).arg(svmP);
        break;
    case 1:
        algo += "nu-SVM";
        algo += QString(" %1 %2").arg(svmC).arg(svmP);
        break;
    case 2:
        algo += "KRLS";
        algo += QString(" %1 %2").arg(svmC).arg(svmP);
        break;
    }
    switch(kernelType)
    {
    case 0:
        algo += " L";
        break;
    case 1:
        algo += QString(" P %1").arg(kernelDegree);
        break;
    case 2:
        algo += QString(" R %1").arg(kernelGamma);
        break;
    case 3:
        algo += QString(" Sig %1").arg(kernelGamma);
        break;
    }
    return algo;
}

Regressor *RegrSVM::GetRegressor()
{
    int svmType = params->svmTypeCombo->currentIndex();
    Regressor *regressor = 0;
    switch(svmType)
    {
    case 2:
        regressor = new RegressorKRLS();
        break;
    default:
        regressor = new RegressorSVR();
        break;
    }
    SetParams(regressor);
    return regressor;
}

void RegrSVM::DrawInfo(Canvas *canvas, QPainter &painter, Regressor *regressor)
{
    painter.setRenderHint(QPainter::Antialiasing);
    int xIndex = canvas->xIndex;
    int yIndex = canvas->yIndex;
    if(regressor->type == REGR_KRLS)
    {
        vector<fvec> sv = ((RegressorKRLS*)regressor)->GetSVs();
        int radius = 9;
        painter.setBrush(Qt::NoBrush);
        FOR(i, sv.size())
        {
            QPointF point = canvas->toCanvasCoords(sv[i]);
            painter.setPen(QPen(Qt::black,6));
            painter.drawEllipse(point, radius, radius);
            painter.setPen(QPen(Qt::white,3));
            painter.drawEllipse(point, radius, radius);
        }
    }
    else if(regressor->type == REGR_SVR)
    {
        // we want to draw the support vectors
        svm_model *svm = ((RegressorSVR*)regressor)->GetModel();
        if(svm)
        {
            painter.setBrush(Qt::NoBrush);
            painter.setPen(QPen(Qt::black, 4));
            std::vector<fvec> samples = canvas->data->GetSamples();
            int dim = canvas->data->GetDimCount();
            fvec sv(2,0);
            FOR(i, svm->l)
            {
                sv[0] = (f32)svm->SV[i][xIndex].value;
                FOR(j, samples.size())
                {
                    if(sv[0] == samples[j][xIndex])
                    {
                        sv[1] = samples[j][yIndex];
                        break;
                    }
                }
                int radius = 9;
                QPointF point = canvas->toCanvasCoords(sv[0],sv[1]);
                painter.drawEllipse(point, radius, radius);
            }
        }
    }
}

void RegrSVM::DrawConfidence(Canvas *canvas, Regressor *regressor)
{
    canvas->maps.confidence = QPixmap();
}

void RegrSVM::DrawModel(Canvas *canvas, QPainter &painter, Regressor *regressor)
{
    painter.setRenderHint(QPainter::Antialiasing, true);
    int w = canvas->width();
    int h = canvas->height();
    int xIndex = canvas->xIndex;
    fvec sample = canvas->toSampleCoords(0,0);
    int dim = sample.size();
    if(dim > 2) return;
    if(regressor->type == REGR_KRLS)
    {
        canvas->maps.confidence = QPixmap();
        int steps = w;
        QPainterPath path;
        FOR(x, steps)
        {
            sample = canvas->toSampleCoords(x,0);
            fvec res = regressor->Test(sample);
            if(res[0] != res[0]) continue;
            QPointF point = canvas->toCanvasCoords(sample[xIndex], res[0]);
            if(x)
            {
                path.lineTo(point);
            }
            else path.moveTo(point);
        }
        painter.setPen(QPen(Qt::black, 1));
        painter.drawPath(path);
    }
    else if(regressor->type == REGR_SVR)
    {
        canvas->maps.confidence = QPixmap();
        svm_parameter params = ((RegressorSVR *)regressor)->param;

        float eps = params.p;
        if(params.svm_type == NU_SVR) eps = ((RegressorSVR *)regressor)->GetModel()->eps[0];
        eps = fabs((canvas->toCanvasCoords(eps,0) - canvas->toCanvasCoords(0,0)).x());

        int steps = w;
        QPainterPath path, pathUp, pathDown;
        FOR(x, steps)
        {
            sample = canvas->toSampleCoords(x,0);
            fvec res = regressor->Test(sample);
            if(res[0] != res[0]) continue;
            QPointF point = canvas->toCanvasCoords(sample[xIndex], res[0]);
            if(x)
            {
                path.lineTo(point);
                pathUp.lineTo(point + QPointF(0, eps));
                pathDown.lineTo(point - QPointF(0, eps));
            }
            else
            {
                path.moveTo(point);
                pathUp.moveTo(point + QPointF(0, eps));
                pathDown.moveTo(point - QPointF(0, eps));
            }
        }
        painter.setPen(QPen(Qt::black, 1));
        painter.drawPath(path);
        painter.setPen(QPen(Qt::black, 0.5));
        painter.drawPath(pathUp);
        painter.drawPath(pathDown);
    }
}

void RegrSVM::SaveOptions(QSettings &settings)
{
    settings.setValue("kernelDeg", params->kernelDegSpin->value());
    settings.setValue("kernelType", params->kernelTypeCombo->currentIndex());
    settings.setValue("kernelWidth", params->kernelWidthSpin->value());
    settings.setValue("svmC", params->svmCSpin->value());
    settings.setValue("svmP", params->svmPSpin->value());
    settings.setValue("svmType", params->svmTypeCombo->currentIndex());
    settings.setValue("optimizeCheck", params->optimizeCheck->isChecked());
}

bool RegrSVM::LoadOptions(QSettings &settings)
{
    if(settings.contains("kernelDeg")) params->kernelDegSpin->setValue(settings.value("kernelDeg").toFloat());
    if(settings.contains("kernelType")) params->kernelTypeCombo->setCurrentIndex(settings.value("kernelType").toInt());
    if(settings.contains("kernelWidth")) params->kernelWidthSpin->setValue(settings.value("kernelWidth").toFloat());
    if(settings.contains("svmC")) params->svmCSpin->setValue(settings.value("svmC").toFloat());
    if(settings.contains("svmP")) params->svmPSpin->setValue(settings.value("svmP").toFloat());
    if(settings.contains("svmType")) params->svmTypeCombo->setCurrentIndex(settings.value("svmType").toInt());
    if(settings.contains("optimizeCheck")) params->optimizeCheck->setChecked(settings.value("optimizeCheck").toBool());
    ChangeOptions();
    return true;
}

void RegrSVM::SaveParams(QTextStream &file)
{
    file << "regressionOptions" << ":" << "kernelDeg" << " " << params->kernelDegSpin->value() << "\n";
    file << "regressionOptions" << ":" << "kernelType" << " " << params->kernelTypeCombo->currentIndex() << "\n";
    file << "regressionOptions" << ":" << "kernelWidth" << " " << params->kernelWidthSpin->value() << "\n";
    file << "regressionOptions" << ":" << "svmC" << " " << params->svmCSpin->value() << "\n";
    file << "regressionOptions" << ":" << "svmP" << " " << params->svmPSpin->value() << "\n";
    file << "regressionOptions" << ":" << "svmType" << " " << params->svmTypeCombo->currentIndex() << "\n";
    file << "regressionOptions" << ":" << "optimizeCheck" << " " << params->optimizeCheck->isChecked() << "\n";
}

bool RegrSVM::LoadParams(QString name, float value)
{
    if(name.endsWith("kernelDeg")) params->kernelDegSpin->setValue((int)value);
    if(name.endsWith("kernelType")) params->kernelTypeCombo->setCurrentIndex((int)value);
    if(name.endsWith("kernelWidth")) params->kernelWidthSpin->setValue(value);
    if(name.endsWith("svmC")) params->svmCSpin->setValue(value);
    if(name.endsWith("svmP")) params->svmPSpin->setValue(value);
    if(name.endsWith("svmType")) params->svmTypeCombo->setCurrentIndex((int)value);
    if(name.endsWith("optimizeCheck")) params->optimizeCheck->setChecked((int)value);
    ChangeOptions();
    return true;
}
