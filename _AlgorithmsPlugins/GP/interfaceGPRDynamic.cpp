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
#include "interfaceGPRDynamic.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>

using namespace std;

DynamicGPR::DynamicGPR()
{
    params = new Ui::ParametersGPRDynamic();
    params->setupUi(widget = new QWidget());
    connect(params->kernelTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeOptions()));
    connect(params->sparseCheck, SIGNAL(clicked()), this, SLOT(ChangeOptions()));
    ChangeOptions();
}

void DynamicGPR::ChangeOptions()
{
    bool bSparse = params->sparseCheck->isChecked();

    params->capacitySpin->setVisible(bSparse);
    params->labelCapacity->setVisible(bSparse);
    params->noiseSpin->setEnabled(bSparse);

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
    }
}

void DynamicGPR::SetParams(Dynamical *dynamical)
{
    if(!dynamical) return;
    float noise = params->noiseSpin->value();
    int kernelType = params->kernelTypeCombo->currentIndex();
    float kernelGamma = params->kernelWidthSpin->value();
    float kernelDegree = params->kernelDegSpin->value();
    int capacity = params->capacitySpin->value();

    DynamicalGPR *gpr = (DynamicalGPR*)dynamical;
    gpr->SetParams(kernelGamma, noise, capacity, kernelType, kernelDegree);
}

fvec DynamicGPR::GetParams()
{
    int kernelType = params->kernelTypeCombo->currentIndex();
    float kernelGamma = params->kernelWidthSpin->value();
    float kernelDegree = params->kernelDegSpin->value();
    int capacity = params->capacitySpin->value();
    bool bSparse = params->sparseCheck->isChecked();
    double kernelNoise = params->noiseSpin->value();
    fvec par(5);
    par[0] = kernelType;
    par[1] = kernelGamma;
    par[2] = kernelDegree;
    par[3] = capacity;
    par[4] = kernelNoise;
    return par;
}

void DynamicGPR::SetParams(Dynamical *dynamical, fvec parameters)
{
    if(!dynamical) return;
    DynamicalGPR *gpr = dynamic_cast<DynamicalGPR*>(dynamical);
    if(!gpr) return;

    int i = 0;
    int kernelType = parameters.size() > i ? parameters[i] : 0; i++;
    float kernelGamma = parameters.size() > i ? parameters[i] : 0; i++;
    float kernelDegree = parameters.size() > i ? parameters[i] : 0; i++;
    int capacity = parameters.size() > i ? parameters[i] : 0; i++;
    bool bSparse = parameters.size() > i ? parameters[i] : 0; i++;
    double kernelNoise = parameters.size() > i ? parameters[i] : 0; i++;
    if(bSparse) capacity = -1;
    gpr->SetParams(kernelGamma, kernelNoise, capacity, kernelType, kernelDegree);
}

void DynamicGPR::GetParameterList(std::vector<QString> &parameterNames,
                             std::vector<QString> &parameterTypes,
                             std::vector< std::vector<QString> > &parameterValues)
{
    parameterNames.clear();
    parameterTypes.clear();
    parameterValues.clear();
    parameterNames.push_back("kernelType");
    parameterNames.push_back("kernelGamma");
    parameterNames.push_back("kernelDegree");
    parameterNames.push_back("capacity");
    parameterNames.push_back("bSparse");
    parameterNames.push_back("kernelNoise");
    parameterTypes.push_back("List");
    parameterTypes.push_back("Real");
    parameterTypes.push_back("Integer");
    parameterTypes.push_back("Integer");
    parameterTypes.push_back("List");
    parameterTypes.push_back("Real");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("Linear");
    parameterValues.back().push_back("Poly");
    parameterValues.back().push_back("RBF");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("0.00000001f");
    parameterValues.back().push_back("99999999999");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("1");
    parameterValues.back().push_back("150");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("1");
    parameterValues.back().push_back("9999999");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("False");
    parameterValues.back().push_back("True");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("0.0000000001f");
    parameterValues.back().push_back("999999999999");
}

Dynamical *DynamicGPR::GetDynamical()
{
    Dynamical *dynamical = new DynamicalGPR();
    SetParams(dynamical);
    return dynamical;
}

void DynamicGPR::DrawModel(Canvas *canvas, QPainter &painter, Dynamical *dynamical)
{
}

void DynamicGPR::DrawInfo(Canvas *canvas, QPainter &painter, Dynamical *dynamical)
{
}

void DynamicGPR::SaveOptions(QSettings &settings)
{
    settings.setValue("kernelDeg", params->kernelDegSpin->value());
    settings.setValue("kernelType", params->kernelTypeCombo->currentIndex());
    settings.setValue("kernelWidth", params->kernelWidthSpin->value());
    settings.setValue("capacitySpin", params->capacitySpin->value());
    settings.setValue("noiseSpin", params->noiseSpin->value());
}

bool DynamicGPR::LoadOptions(QSettings &settings)
{
    if(settings.contains("kernelDeg")) params->kernelDegSpin->setValue(settings.value("kernelDeg").toFloat());
    if(settings.contains("kernelType")) params->kernelTypeCombo->setCurrentIndex(settings.value("kernelType").toInt());
    if(settings.contains("kernelWidth")) params->kernelWidthSpin->setValue(settings.value("kernelWidth").toFloat());
    if(settings.contains("capacitySpin")) params->capacitySpin->setValue(settings.value("capacitySpin").toInt());
    if(settings.contains("noiseSpin")) params->noiseSpin->setValue(settings.value("noiseSpin").toFloat());
    ChangeOptions();
    return true;
}

void DynamicGPR::SaveParams(QTextStream &file)
{
    file << "dynamicalOptions" << ":" << "kernelDeg" << " " << params->kernelDegSpin->value() << "\n";
    file << "dynamicalOptions" << ":" << "kernelType" << " " << params->kernelTypeCombo->currentIndex() << "\n";
    file << "dynamicalOptions" << ":" << "kernelWidth" << " " << params->kernelWidthSpin->value() << "\n";
    file << "dynamicalOptions" << ":" << "capacitySpin" << " " << params->capacitySpin->value() << "\n";
    file << "dynamicalOptions" << ":" << "noiseSpin" << " " << params->noiseSpin->value() << "\n";
}

bool DynamicGPR::LoadParams(QString name, float value)
{
    if(name.endsWith("kernelDeg")) params->kernelDegSpin->setValue((int)value);
    if(name.endsWith("kernelType")) params->kernelTypeCombo->setCurrentIndex((int)value);
    if(name.endsWith("kernelWidth")) params->kernelWidthSpin->setValue(value);
    if(name.endsWith("capacitySpin")) params->capacitySpin->setValue(value);
    if(name.endsWith("noiseSpin")) params->noiseSpin->setValue(value);
    ChangeOptions();
    return true;
}
