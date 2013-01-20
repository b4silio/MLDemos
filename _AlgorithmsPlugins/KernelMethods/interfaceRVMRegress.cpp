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
#include "interfaceRVMRegress.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>

using namespace std;

RegrRVM::RegrRVM()
{
    params = new Ui::ParametersRVMRegr();
    params->setupUi(widget = new QWidget());
    connect(params->kernelTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeOptions()));
    ChangeOptions();
}

void RegrRVM::ChangeOptions()
{
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

void RegrRVM::SetParams(Regressor *regressor)
{
    if(!regressor) return;
    SetParams(regressor, GetParams());
}

fvec RegrRVM::GetParams()
{
    float svmP = params->svmPSpin->value();
    int kernelType = params->kernelTypeCombo->currentIndex();
    float kernelGamma = params->kernelWidthSpin->value();
    float kernelDegree = params->kernelDegSpin->value();

    fvec par(4);
    par[0] = svmP;
    par[1] = kernelType;
    par[2] = kernelGamma;
    par[3] = kernelDegree;
    return par;
}

void RegrRVM::SetParams(Regressor *regressor, fvec parameters)
{
    if(!regressor) return;
    float svmP = parameters.size() > 0 ? parameters[0] : 1;
    int kernelType = parameters.size() > 1 ? parameters[1] : 0;
    float kernelGamma = parameters.size() > 2 ? parameters[2] : 0;
    int kernelDegree = parameters.size() > 3 ? parameters[3] : 0;

    RegressorRVM *rvm = dynamic_cast<RegressorRVM *>(regressor);
    if(rvm) rvm->SetParams(svmP, kernelType, kernelGamma, kernelDegree);
}

void RegrRVM::GetParameterList(std::vector<QString> &parameterNames,
                                std::vector<QString> &parameterTypes,
                                std::vector< std::vector<QString> > &parameterValues)
{
    parameterNames.push_back("Epsilon");
    parameterNames.push_back("Kernel Type");
    parameterNames.push_back("Kernel Width");
    parameterNames.push_back("Kernel Degree");
    parameterTypes.push_back("Real");
    parameterTypes.push_back("List");
    parameterTypes.push_back("Real");
    parameterTypes.push_back("Integer");
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
}

QString RegrRVM::GetAlgoString()
{
    float svmP = params->svmPSpin->value();
    int kernelType = params->kernelTypeCombo->currentIndex();
    float kernelGamma = params->kernelWidthSpin->value();
    float kernelDegree = params->kernelDegSpin->value();

    QString algo = QString("RVM %1").arg(svmP);

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
    }
    return algo;
}

Regressor *RegrRVM::GetRegressor()
{
    Regressor *regressor = new RegressorRVM();
    SetParams(regressor);
    return regressor;
}

void RegrRVM::DrawInfo(Canvas *canvas, QPainter &painter, Regressor *regressor)
{
    painter.setRenderHint(QPainter::Antialiasing);
    int xIndex = canvas->xIndex;
    int yIndex = canvas->yIndex;

    vector<fvec> sv = ((RegressorRVM*)regressor)->GetSVs();
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

void RegrRVM::DrawConfidence(Canvas *canvas, Regressor *regressor)
{
    canvas->maps.confidence = QPixmap();
}

void RegrRVM::DrawModel(Canvas *canvas, QPainter &painter, Regressor *regressor)
{
    painter.setRenderHint(QPainter::Antialiasing, true);
    int w = canvas->width();
    int h = canvas->height();
    int xIndex = canvas->xIndex;
    fvec sample = canvas->toSampleCoords(0,0);
    int dim = sample.size();
    if(dim > 2) return;

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

void RegrRVM::SaveOptions(QSettings &settings)
{
    settings.setValue("svmP", params->svmPSpin->value());
    settings.setValue("kernelDeg", params->kernelDegSpin->value());
    settings.setValue("kernelType", params->kernelTypeCombo->currentIndex());
    settings.setValue("kernelWidth", params->kernelWidthSpin->value());
}

bool RegrRVM::LoadOptions(QSettings &settings)
{
    if(settings.contains("svmP")) params->svmPSpin->setValue(settings.value("svmP").toFloat());
    if(settings.contains("kernelDeg")) params->kernelDegSpin->setValue(settings.value("kernelDeg").toFloat());
    if(settings.contains("kernelType")) params->kernelTypeCombo->setCurrentIndex(settings.value("kernelType").toInt());
    if(settings.contains("kernelWidth")) params->kernelWidthSpin->setValue(settings.value("kernelWidth").toFloat());
    ChangeOptions();
    return true;
}

void RegrRVM::SaveParams(QTextStream &file)
{
    file << "regressionOptions" << ":" << "svmP" << " " << params->svmPSpin->value() << "\n";
    file << "regressionOptions" << ":" << "kernelDeg" << " " << params->kernelDegSpin->value() << "\n";
    file << "regressionOptions" << ":" << "kernelType" << " " << params->kernelTypeCombo->currentIndex() << "\n";
    file << "regressionOptions" << ":" << "kernelWidth" << " " << params->kernelWidthSpin->value() << "\n";
}

bool RegrRVM::LoadParams(QString name, float value)
{
    if(name.endsWith("svmP")) params->svmPSpin->setValue(value);
    if(name.endsWith("kernelDeg")) params->kernelDegSpin->setValue((int)value);
    if(name.endsWith("kernelType")) params->kernelTypeCombo->setCurrentIndex((int)value);
    if(name.endsWith("kernelWidth")) params->kernelWidthSpin->setValue(value);
    ChangeOptions();
    return true;
}
