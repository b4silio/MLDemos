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
#include "interfaceSVMDynamic.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>

using namespace std;

DynamicSVM::DynamicSVM()
{
    params = new Ui::ParametersDynamic();
    params->setupUi(widget = new QWidget());
    connect(params->kernelTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeOptions()));
    connect(params->svmTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeOptions()));
    ChangeOptions();
}

DynamicSVM::~DynamicSVM()
{
    delete params;
}

void DynamicSVM::ChangeOptions()
{
    params->svmCLabel->setText("C");
    params->svmPSpin->setRange(0.0001, 1.0);
    params->svmPSpin->setSingleStep(0.01);
    params->svmPSpin->setDecimals(4);
    params->svmCSpin->setEnabled(true);
    params->svmCSpin->setRange(0.1, 9999.9);
    params->svmCSpin->setDecimals(1);
    switch(params->svmTypeCombo->currentIndex())
    {
    case 0: // C-SVM
        params->svmEpsLabel->setText("eps");
        break;
    case 1: // Nu-SVM
        params->svmEpsLabel->setText("Nu");
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
    }
}

void DynamicSVM::SetParams(Dynamical *dynamical)
{
    if(!dynamical) return;
    int kernelMethod = params->svmTypeCombo->currentIndex();
    float svmC = params->svmCSpin->value();
    int kernelType = params->kernelTypeCombo->currentIndex();
    float kernelGamma = params->kernelWidthSpin->value();
    float kernelDegree = params->kernelDegSpin->value();
    float svmP = params->svmPSpin->value();

    DynamicalSVR *svm = dynamic_cast<DynamicalSVR*>(dynamical);
    if(!svm) return;
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
        break;
    case 2:
        svm->param.kernel_type = RBF;
        break;
    }
    svm->param.C = svmC;
    svm->param.nu = svmP;
    svm->param.p = svmP;
    svm->param.gamma = 1 / kernelGamma;
    svm->param.degree = kernelDegree;
}

fvec DynamicSVM::GetParams()
{
    int kernelMethod = params->svmTypeCombo->currentIndex();
    float svmC = params->svmCSpin->value();
    int kernelType = params->kernelTypeCombo->currentIndex();
    float kernelGamma = params->kernelWidthSpin->value();
    float kernelDegree = params->kernelDegSpin->value();
    float svmP = params->svmPSpin->value();

    fvec par(6);
    par[0] = kernelMethod;
    par[1] = svmC;
    par[2] = kernelType;
    par[3] = kernelGamma;
    par[4] = kernelDegree;
    par[5] = svmP;
    return par;
}

void DynamicSVM::SetParams(Dynamical *dynamical, fvec parameters)
{
    if(!dynamical) return;
    int kernelMethod = parameters.size() > 0 ? parameters[0] : 1;
    float svmC = parameters.size() > 1 ? parameters[1] : 1;
    int kernelType = parameters.size() > 2 ? parameters[2] : 0;
    float kernelGamma = parameters.size() > 3 ? parameters[3] : 0.1;
    int kernelDegree = parameters.size() > 4 ? parameters[4] : 1;
    float svmP = parameters.size() > 5 ? parameters[5] : 0.1;

    DynamicalSVR *svm = dynamic_cast<DynamicalSVR*>(dynamical);
    if(!svm) return;
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
        break;
    case 2:
        svm->param.kernel_type = RBF;
        break;
    }
    svm->param.C = svmC;
    svm->param.nu = svmP;
    svm->param.p = svmP;
    svm->param.gamma = 1 / kernelGamma;
    svm->param.degree = kernelDegree;
}

void DynamicSVM::GetParameterList(std::vector<QString> &parameterNames,
                                std::vector<QString> &parameterTypes,
                                std::vector< std::vector<QString> > &parameterValues)
{
    parameterNames.push_back("Kernel Method");
    parameterNames.push_back("Penalty (C)");
    parameterNames.push_back("Kernel Type");
    parameterNames.push_back("Kernel Width");
    parameterNames.push_back("Kernel Degree");
    parameterNames.push_back("Epsilon / Nu");
    parameterTypes.push_back("List");
    parameterTypes.push_back("Real");
    parameterTypes.push_back("List");
    parameterTypes.push_back("Real");
    parameterTypes.push_back("Integer");
    parameterTypes.push_back("Real");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("Epsilon-SVR");
    parameterValues.back().push_back("Nu-SVR");
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
}

Dynamical *DynamicSVM::GetDynamical()
{
    int svmType = params->svmTypeCombo->currentIndex();
    Dynamical *dynamical = new DynamicalSVR();
    SetParams(dynamical);
    return dynamical;
}

void DynamicSVM::DrawModel(Canvas *canvas, QPainter &painter, Dynamical *dynamical)
{
}

void DynamicSVM::DrawInfo(Canvas *canvas, QPainter &painter, Dynamical *dynamical)
{
}

void DynamicSVM::SaveOptions(QSettings &settings)
{
    settings.setValue("kernelDeg", params->kernelDegSpin->value());
    settings.setValue("kernelType", params->kernelTypeCombo->currentIndex());
    settings.setValue("kernelWidth", params->kernelWidthSpin->value());
    settings.setValue("svmC", params->svmCSpin->value());
    settings.setValue("svmP", params->svmPSpin->value());
    settings.setValue("svmType", params->svmTypeCombo->currentIndex());
}

bool DynamicSVM::LoadOptions(QSettings &settings)
{
    if(settings.contains("kernelDeg")) params->kernelDegSpin->setValue(settings.value("kernelDeg").toFloat());
    if(settings.contains("kernelType")) params->kernelTypeCombo->setCurrentIndex(settings.value("kernelType").toInt());
    if(settings.contains("kernelWidth")) params->kernelWidthSpin->setValue(settings.value("kernelWidth").toFloat());
    if(settings.contains("svmC")) params->svmCSpin->setValue(settings.value("svmC").toFloat());
    if(settings.contains("svmP")) params->svmPSpin->setValue(settings.value("svmP").toFloat());
    if(settings.contains("svmType")) params->svmTypeCombo->setCurrentIndex(settings.value("svmType").toInt());
    ChangeOptions();
    return true;
}

void DynamicSVM::SaveParams(QTextStream &file)
{
    file << "dynamicalOptions" << ":" << "kernelDeg" << " " << params->kernelDegSpin->value() << "\n";
    file << "dynamicalOptions" << ":" << "kernelType" << " " << params->kernelTypeCombo->currentIndex() << "\n";
    file << "dynamicalOptions" << ":" << "kernelWidth" << " " << params->kernelWidthSpin->value() << "\n";
    file << "dynamicalOptions" << ":" << "svmC" << " " << params->svmCSpin->value() << "\n";
    file << "dynamicalOptions" << ":" << "svmP" << " " << params->svmPSpin->value() << "\n";
    file << "dynamicalOptions" << ":" << "svmType" << " " << params->svmTypeCombo->currentIndex() << "\n";
}

bool DynamicSVM::LoadParams(QString name, float value)
{
    if(name.endsWith("kernelDeg")) params->kernelDegSpin->setValue((int)value);
    if(name.endsWith("kernelType")) params->kernelTypeCombo->setCurrentIndex((int)value);
    if(name.endsWith("kernelWidth")) params->kernelWidthSpin->setValue(value);
    if(name.endsWith("svmC")) params->svmCSpin->setValue(value);
    if(name.endsWith("svmP")) params->svmPSpin->setValue(value);
    if(name.endsWith("svmType")) params->svmTypeCombo->setCurrentIndex((int)value);
    ChangeOptions();
    return true;
}
