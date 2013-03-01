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
#include "interfaceSVMClassifier.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>
#include <QDebug>

using namespace std;

ClassSVM::ClassSVM()
{
    params = new Ui::Parameters();
    params->setupUi(widget = new QWidget());
    ardLabel = 0;
    connect(params->svmTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeOptions()));
    connect(params->kernelTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeOptions()));
    connect(params->optimizeCheck, SIGNAL(clicked()), this, SLOT(ChangeOptions()));
    connect(params->kernelButton, SIGNAL(clicked()), this, SLOT(DisplayARDKernel()));
    ChangeOptions();
}

ClassSVM::~ClassSVM()
{
    delete params;
    DEL(ardLabel);
}

void ClassSVM::ChangeOptions()
{
    int C = params->svmCSpin->value();
    params->maxSVSpin->setVisible(false);
    params->labelMaxSV->setVisible(false);
    params->svmCSpin->setRange(0.0001, 1.0);
    params->svmCSpin->setSingleStep(0.0001);
    params->svmCSpin->setDecimals(4);
    params->optimizeCheck->setVisible(true);
    if(C > 1) params->svmCSpin->setValue(0.001);
    switch(params->svmTypeCombo->currentIndex())
    {
    case 0: // C-SVM
        params->svmCSpin->setRange(0.1, 9999);
        params->svmCSpin->setSingleStep(1);
        params->svmCSpin->setDecimals(1);
        params->svmCSpin->setValue(C);
        if(params->svmCSpin->value() < 1) params->svmCSpin->setValue(100);
        params->svmTypeLabel->setText("C");
        if(params->kernelTypeCombo->count() < 4) params->kernelTypeCombo->addItem("Sigmoid");
        break;
    case 1: // Nu-SVM
        params->svmTypeLabel->setText("Nu");
        if(params->kernelTypeCombo->count() < 4) params->kernelTypeCombo->addItem("Sigmoid");
        break;
    case 2: // Pegasos
        params->optimizeCheck->setVisible(false);
        params->svmTypeLabel->setText("lambda");
        params->maxSVSpin->setVisible(true);
        params->labelMaxSV->setVisible(true);
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
    params->kernelButton->setVisible(params->optimizeCheck->isChecked());
}

void ClassSVM::DisplayARDKernel()
{
    if(!ardLabel)
    {
        ardLabel = new QLabel();
        ardLabel->setScaledContents(true);
    }
    QPixmap pixmap(200,200);
    //QBitmap bitmap(pixmap.width(), pixmap.height());
    //bitmap.clear();
    //pixmap.setMask(bitmap);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    if(ardKernel.size())
    {
        QPointF center(pixmap.width()/2,pixmap.height()/2);

        int dim = ardKernel.size();
        float maxVal = -FLT_MAX;
        FOR(d, dim) maxVal = max(maxVal, ardKernel[d]);
        float maxRadius = pixmap.width()/2*0.75;
        painter.setPen(QPen(Qt::black,0.5));
        FOR(d, dim)
        {
            float angle = d*2*M_PI/dim;
            float radius = pixmap.width()/2*0.8;
            QPointF point(cosf(angle)*radius, sinf(angle)*radius);
            painter.drawLine(center, center + point);
        }
        QPolygonF poly;
        FOR(d, dim+1)
        {
            float value = ardKernel[d%dim];
            float angle = d*2*M_PI/dim;
            float radius = value/maxVal*maxRadius;
            QPointF point(cosf(angle)*radius, sinf(angle)*radius);
            poly << center + point;
        }
        painter.setBrush(Qt::red);
        painter.setPen(Qt::NoPen);
        painter.setOpacity(0.3);
        painter.drawPolygon(poly);
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(Qt::red,2));
        painter.drawPolygon(poly);
    }
    ardLabel->setPixmap(pixmap);
    ardLabel->show();
}

QString ClassSVM::GetAlgoString()
{
    double C = params->svmCSpin->value();
    int sv = params->maxSVSpin->value();
    int kernelType = params->kernelTypeCombo->currentIndex();
    float kernelGamma = params->kernelWidthSpin->value();
    float kernelDegree = params->kernelDegSpin->value();
    bool bOptimize = params->optimizeCheck->isChecked();

    QString algo;
    switch(params->svmTypeCombo->currentIndex())
    {
    case 0: // C-SVM
        algo += "C-SVM";
        algo += QString(" %1").arg(C);
        break;
    case 1: // Nu-SVM
        algo += "Nu-SVM";
        algo += QString(" %1").arg(C);
        break;
    case 2: // Pegasos
        algo += "Pegasos";
        algo += QString(" %1 %2").arg(C).arg(sv);
        break;
    }
    switch(kernelType)
    {
    case 0:
        algo += " Lin";
        break;
    case 1:
        algo += QString(" Pol %1").arg(kernelDegree);
        break;
    case 2:
        algo += QString(" RBF %1").arg(kernelGamma);
        break;
    case 3:
        algo += QString(" Sig %1").arg(kernelGamma);
        break;
    }
    if(bOptimize) algo += QString(" Opt");
    return algo;
}

void ClassSVM::SetParams(Classifier *classifier)
{
    if(!classifier) return;
    int svmType = params->svmTypeCombo->currentIndex();
    int maxSV = params->maxSVSpin->value();
    float svmC = params->svmCSpin->value();
    int kernelType = params->kernelTypeCombo->currentIndex();
    float kernelGamma = params->kernelWidthSpin->value();
    float kernelDegree = params->kernelDegSpin->value();
    bool bOptimize = params->optimizeCheck->isChecked();

    ClassifierPegasos *pegasos = dynamic_cast<ClassifierPegasos *>(classifier);
    if(pegasos) pegasos->SetParams(svmC, max(2,(int)maxSV), kernelType, kernelGamma, kernelDegree);

    ClassifierSVM *svm = dynamic_cast<ClassifierSVM *>(classifier);
    if(svm)
    {
        switch(svmType)
        {
        case 0:
            svm->param.svm_type = C_SVC;
            break;
        case 1:
            svm->param.svm_type = NU_SVC;
            break;
        }
        switch(kernelType)
        {
        case 0:
            svm->param.kernel_type = LINEAR;
            break;
        case 1:
            svm->param.kernel_type = POLY;
            break;
        case 2:
            svm->param.kernel_type = RBF;
            break;
        case 3:
            svm->param.kernel_type = SIGMOID;
            break;
        }
        svm->param.C = svm->param.nu = svmC;
        svm->param.gamma = 1 / kernelGamma;
        svm->param.coef0 = 0;
        svm->param.degree = kernelDegree;
        svm->bOptimize = bOptimize;
    }
}

fvec ClassSVM::GetParams()
{
    int svmType = params->svmTypeCombo->currentIndex();
    int maxSV = params->maxSVSpin->value();
    float svmC = params->svmCSpin->value();
    int kernelType = params->kernelTypeCombo->currentIndex();
    float kernelGamma = params->kernelWidthSpin->value();
    float kernelDegree = params->kernelDegSpin->value();
    bool bOptimize = params->optimizeCheck->isChecked();

    if(svmType == 2) // pegasos
    {
        fvec par(5);
        par[0] = svmC;
        par[1] = maxSV;
        par[2] = kernelType;
        par[3] = kernelGamma;
        par[4] = kernelDegree;
        return par;
    }
    else
    {
        fvec par(6);
        par[0] = svmType;
        par[1] = svmC;
        par[2] = kernelType;
        par[3] = kernelGamma;
        par[4] = kernelDegree;
        par[5] = bOptimize;
        return par;
    }
}

void ClassSVM::SetParams(Classifier *classifier, fvec parameters)
{
    if(!classifier) return;
    int svmType = params->svmTypeCombo->currentIndex();
    float svmC, kernelGamma;
    int maxSV, kernelType, kernelDegree;
    bool bOptimize;
    if(svmType == 2) // pegasos
    {
        svmC = parameters.size() > 0 ? parameters[0] : 1;
        maxSV = parameters.size() > 1 ? parameters[1] : 0;
        kernelType = parameters.size() > 2 ? parameters[2] : 0;
        kernelGamma = parameters.size() > 3 ? parameters[3] : 0.1;
        kernelDegree = parameters.size() > 4 ? parameters[4] : 0;
    }
    else
    {
        svmType = parameters.size() > 0 ? parameters[0] : 0;
        svmC = parameters.size() > 1 ? parameters[1] : 1;
        kernelType = parameters.size() > 2 ? parameters[2] : 0;
        kernelGamma = parameters.size() > 3 ? parameters[3] : 0.1;
        kernelDegree = parameters.size() > 4 ? parameters[4] : 0;
        bOptimize = parameters.size() > 5 ? parameters[5] : 0;
    }

    ClassifierPegasos *pegasos = dynamic_cast<ClassifierPegasos *>(classifier);
    if(pegasos) pegasos->SetParams(svmC, max(2,(int)maxSV), kernelType, kernelGamma, kernelDegree);

    ClassifierSVM *svm = dynamic_cast<ClassifierSVM *>(classifier);
    if(svm)
    {
        switch(svmType)
        {
        case 0:
            svm->param.svm_type = C_SVC;
            break;
        case 1:
            svm->param.svm_type = NU_SVC;
            break;
        }
        switch(kernelType)
        {
        case 0:
            svm->param.kernel_type = LINEAR;
            break;
        case 1:
            svm->param.kernel_type = POLY;
            break;
        case 2:
            svm->param.kernel_type = RBF;
            break;
        case 3:
            svm->param.kernel_type = SIGMOID;
            break;
        }
        svm->param.C = svm->param.nu = svmC;
        svm->param.gamma = 1. / kernelGamma;
        svm->param.coef0 = 0;
        svm->param.degree = kernelDegree;
        svm->bOptimize = bOptimize;
    }
}

void ClassSVM::GetParameterList(std::vector<QString> &parameterNames,
                                std::vector<QString> &parameterTypes,
                                std::vector< std::vector<QString> > &parameterValues)
{
    int svmType = params->svmTypeCombo->currentIndex();
    if(svmType == 2)
    {
        parameterNames.push_back("Penalty (C)");
        parameterNames.push_back("Max SV");
        parameterNames.push_back("Kernel Type");
        parameterNames.push_back("Kernel Width");
        parameterNames.push_back("Kernel Degree");
        parameterTypes.push_back("Real");
        parameterTypes.push_back("Integer");
        parameterTypes.push_back("List");
        parameterTypes.push_back("Real");
        parameterTypes.push_back("Integer");
        parameterValues.push_back(vector<QString>());
        parameterValues.back().push_back("0");
        parameterValues.back().push_back("9999999999999");
        parameterValues.push_back(vector<QString>());
        parameterValues.back().push_back("1");
        parameterValues.back().push_back("999999999");
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
    }
    else
    {
        parameterNames.push_back("SVM Type");
        parameterNames.push_back("Penalty (C) / Nu");
        parameterNames.push_back("Kernel Type");
        parameterNames.push_back("Kernel Width");
        parameterNames.push_back("Kernel Degree");
        parameterNames.push_back("Optimize Kernel");
        parameterTypes.push_back("List");
        parameterTypes.push_back("Real");
        parameterTypes.push_back("List");
        parameterTypes.push_back("Real");
        parameterTypes.push_back("Integer");
        parameterTypes.push_back("List");
        parameterValues.push_back(vector<QString>());
        parameterValues.back().push_back("Epsilon-SVM");
        parameterValues.back().push_back("Nu-SVM");
        parameterValues.push_back(vector<QString>());
        parameterValues.back().push_back("1");
        parameterValues.back().push_back("999999999");
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
        parameterValues.back().push_back("False");
        parameterValues.back().push_back("True");
    }
}

Classifier *ClassSVM::GetClassifier()
{
    int svmType = params->svmTypeCombo->currentIndex();
    Classifier *classifier = 0;
    switch(svmType)
    {
    case 2:
        classifier = new ClassifierPegasos();
        break;
    default:
        classifier = new ClassifierSVM();
        break;
    }
    SetParams(classifier);
    ardKernel.clear();
    ardNames.clear();
    return classifier;
}

void ClassSVM::DrawInfo(Canvas *canvas, QPainter &painter, Classifier *classifier)
{
    ardKernel.clear();
    ardNames.clear();
    painter.setRenderHint(QPainter::Antialiasing);

    if(dynamic_cast<ClassifierPegasos*>(classifier))
    {
        // we want to draw the support vectors
        vector<fvec> sv = dynamic_cast<ClassifierPegasos*>(classifier)->GetSVs();
        int radius = 9;
        int dim = canvas->data->GetDimCount();
        FOR(i, sv.size())
        {
            fvec sample = sv[i];
            if(canvas->sourceDims.size())
            {
                fvec newSample(dim, 0);
                FOR(d, canvas->sourceDims.size())
                {
                    newSample[canvas->sourceDims[d]] = sample[d];
                }
                sample = newSample;
            }
            QPointF point = canvas->toCanvasCoords(sample);
            painter.setPen(QPen(Qt::black,6));
            painter.drawEllipse(point, radius, radius);
            painter.setPen(QPen(Qt::white,4));
            painter.drawEllipse(point, radius, radius);
        }
    }
    else if(dynamic_cast<ClassifierSVM*>(classifier))
    {
        int dim = canvas->data->GetDimCount();
        // we want to draw the support vectors
        svm_model *svm = dynamic_cast<ClassifierSVM*>(classifier)->GetModel();
        if(svm)
        {
            if(svm->param.kernel_type == RBFWEIGH)
            {
                ardKernel.resize(svm->param.kernel_dim,1.f);
                FOR(d, svm->param.kernel_dim) ardKernel[d] = svm->param.kernel_weight[d];
                ardNames = canvas->dimNames;
                if(ardLabel && ardLabel->isVisible()) DisplayARDKernel();
            }
            fvec sample(dim,0);
            FOR(i, svm->l)
            {
                if(canvas->sourceDims.size())
                {
                    FOR(d, canvas->sourceDims.size()) sample[canvas->sourceDims[d]] = svm->SV[i][d].value;
                }
                else
                {
                    FOR(d, dim) sample[d] = (f32)svm->SV[i][d].value;
                }
                int radius = 9;
                QPointF point = canvas->toCanvasCoords(sample);
                if(abs((*svm->sv_coef)[i]) == svm->param.C)
                {
                    painter.setPen(QPen(Qt::black, 6));
                    painter.drawEllipse(point, radius, radius);
                    painter.setPen(QPen(Qt::white,4));
                    painter.drawEllipse(point, radius, radius);
                }
                else
                {
                    painter.setPen(QPen(Qt::white, 6));
                    painter.drawEllipse(point, radius, radius);
                    painter.setPen(QPen(Qt::black,4));
                    painter.drawEllipse(point, radius, radius);
                }
            }
        }
    }
}

void ClassSVM::DrawGL(Canvas *canvas, GLWidget *glw, Classifier *classifier)
{
    int xInd = canvas->xIndex;
    int yInd = canvas->yIndex;
    int zInd = canvas->zIndex;
    GLObject o;
    o.objectType = "Samples";
    o.style = "rings,pointsize:24";
    vector<fvec> svs;
    int dim = canvas->data->GetDimCount();
    if(dynamic_cast<ClassifierPegasos*>(classifier))
    {
        // we want to draw the support vectors
        svs = dynamic_cast<ClassifierPegasos*>(classifier)->GetSVs();
    }
    else if(dynamic_cast<ClassifierSVM*>(classifier))
    {
        // we want to draw the support vectors
        svm_model *svm = dynamic_cast<ClassifierSVM*>(classifier)->GetModel();
        if(svm)
        {
            fvec sv(dim);
            FOR(i, svm->l)
            {
                FOR(d, dim) sv[d] = svm->SV[i][d].value;
                svs.push_back(sv);
            }
        }
    }
    FOR(i, svs.size())
    {
        o.vertices.append(QVector3D(svs[i][xInd],svs[i][yInd], zInd>=0 && zInd<dim ? svs[i][zInd] : 0));
        o.colors.append(QVector4D(0,0,0,1));
    }
    glw->mutex->lock();
    glw->AddObject(o);
    glw->mutex->unlock();
}

void ClassSVM::SaveOptions(QSettings &settings)
{
    settings.setValue("kernelDeg", params->kernelDegSpin->value());
    settings.setValue("kernelType", params->kernelTypeCombo->currentIndex());
    settings.setValue("kernelWidth", params->kernelWidthSpin->value());
    settings.setValue("svmC", params->svmCSpin->value());
    settings.setValue("svmType", params->svmTypeCombo->currentIndex());
    settings.setValue("optimizeCheck", params->optimizeCheck->isChecked());
    settings.setValue("maxSVSpin", params->maxSVSpin->value());
}

bool ClassSVM::LoadOptions(QSettings &settings)
{
    if(settings.contains("kernelDeg")) params->kernelDegSpin->setValue(settings.value("kernelDeg").toFloat());
    if(settings.contains("kernelType")) params->kernelTypeCombo->setCurrentIndex(settings.value("kernelType").toInt());
    if(settings.contains("kernelWidth")) params->kernelWidthSpin->setValue(settings.value("kernelWidth").toFloat());
    if(settings.contains("svmC")) params->svmCSpin->setValue(settings.value("svmC").toFloat());
    if(settings.contains("svmType")) params->svmTypeCombo->setCurrentIndex(settings.value("svmType").toInt());
    if(settings.contains("optimizeCheck")) params->optimizeCheck->setChecked(settings.value("optimizeCheck").toInt());
    if(settings.contains("maxSVSpin")) params->maxSVSpin->setValue(settings.value("maxSVSpin").toInt());
    ChangeOptions();
    return true;
}

void ClassSVM::SaveParams(QTextStream &file)
{
    file << "classificationOptions" << ":" << "kernelDeg" << " " << params->kernelDegSpin->value() << "\n";
    file << "classificationOptions" << ":" << "kernelType" << " " << params->kernelTypeCombo->currentIndex() << "\n";
    file << "classificationOptions" << ":" << "kernelWidth" << " " << params->kernelWidthSpin->value() << "\n";
    file << "classificationOptions" << ":" << "svmC" << " " << params->svmCSpin->value() << "\n";
    file << "classificationOptions" << ":" << "svmType" << " " << params->svmTypeCombo->currentIndex() << "\n";
    file << "classificationOptions" << ":" << "optimizeCheck" << " " << params->optimizeCheck->isChecked() << "\n";
    file << "classificationOptions" << ":" << "maxSVSpin" << " " << params->maxSVSpin->value() << "\n";
}

bool ClassSVM::LoadParams(QString name, float value)
{
    if(name.endsWith("kernelDeg")) params->kernelDegSpin->setValue((int)value);
    if(name.endsWith("kernelType")) params->kernelTypeCombo->setCurrentIndex((int)value);
    if(name.endsWith("kernelWidth")) params->kernelWidthSpin->setValue(value);
    if(name.endsWith("svmC")) params->svmCSpin->setValue(value);
    if(name.endsWith("svmType")) params->svmTypeCombo->setCurrentIndex((int)value);
    if(name.endsWith("optimizeCheck")) params->optimizeCheck->setChecked((int)value);
    if(name.endsWith("maxSVSpin")) params->maxSVSpin->setValue((int)value);
    ChangeOptions();
    return true;
}
