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
#include "interfacePower.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>
#include <datasetManager.h>
#include <canvas.h>
#include "reinforcementPower.h"

using namespace std;

ReinforcementInterfacePower::ReinforcementInterfacePower()
{
    params = new Ui::ParametersPower();
	params->setupUi(widget = new QWidget());
    ChangeOptions();
}

ReinforcementInterfacePower::~ReinforcementInterfacePower()
{
    delete params;
}

void ReinforcementInterfacePower::ChangeOptions()
{
}

void ReinforcementInterfacePower::SetParams(Reinforcement *reinforcement)
{
    if(!reinforcement) return;
	double variance = params->varianceSpin->value();
    bool bAdaptive = params->adaptiveCheck->isChecked();
    int k = params->kSpin->value();
    ((ReinforcementPower *)reinforcement)->SetParams(k, variance, bAdaptive);
}


fvec ReinforcementInterfacePower::GetParams()
{
    double variance = params->varianceSpin->value();
    bool bAdaptive = params->adaptiveCheck->isChecked();
    int k = params->kSpin->value();

    fvec par(3);
    par[0] = variance;
    par[1] = bAdaptive;
    par[2] = k;
    return par;
}

void ReinforcementInterfacePower::SetParams(Reinforcement *reinforcement, fvec parameters)
{
    int i=0;
    double variance = parameters.size() > i ? parameters[i] : 0; i++;
    bool bAdaptive = parameters.size() > i ? parameters[i] : 0.1;
    int k = parameters.size() > i ? parameters[i] : 10;
    ((ReinforcementPower *)reinforcement)->SetParams(k, variance, bAdaptive);
}

void ReinforcementInterfacePower::GetParameterList(std::vector<QString> &parameterNames,
                             std::vector<QString> &parameterTypes,
                             std::vector< std::vector<QString> > &parameterValues)
{
    parameterNames.push_back("Variance");
    parameterNames.push_back("Adaptive");
    parameterNames.push_back("Population Size");
    parameterTypes.push_back("Real");
    parameterTypes.push_back("List");
    parameterTypes.push_back("Integer");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("0.0000001f");
    parameterValues.back().push_back("1.f");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("False");
    parameterValues.back().push_back("True");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("1");
    parameterValues.back().push_back("999999");
}

Reinforcement *ReinforcementInterfacePower::GetReinforcement()
{
    Reinforcement *reinforcement = NULL;
    reinforcement = new ReinforcementPower();
    SetParams(reinforcement);
    return reinforcement;
}

QString ReinforcementInterfacePower::GetAlgoString()
{
	double variance = params->varianceSpin->value();
    bool adaptiveCheck = params->adaptiveCheck->isChecked();
    int k = params->kSpin->value();
    return QString("Power: %1 %2 ").arg(k).arg(variance) + (adaptiveCheck ? "Adaptive" : "");
}

void ReinforcementInterfacePower::SaveOptions(QSettings &settings)
{
	settings.setValue("varianceSpin", params->varianceSpin->value());
    settings.setValue("adaptiveCheck", params->adaptiveCheck->isChecked());
    settings.setValue("kSpin", params->kSpin->value());
}

bool ReinforcementInterfacePower::LoadOptions(QSettings &settings)
{
	if(settings.contains("varianceSpin")) params->varianceSpin->setValue(settings.value("varianceSpin").toFloat());
    if(settings.contains("adaptiveCheck")) params->adaptiveCheck->setChecked(settings.value("adaptiveCheck").toBool());
    if(settings.contains("kSpin")) params->kSpin->setValue(settings.value("kSpin").toFloat());
    return true;
}

void ReinforcementInterfacePower::SaveParams(QTextStream &file)
{
	file << "maximizationOptions:" << "varianceSpin" << " " << params->varianceSpin->value() << "\n";
    file << "maximizationOptions:" << "adaptiveCheck" << " " << params->adaptiveCheck->isChecked() << "\n";
    file << "maximizationOptions:" << "kSpin" << " " << params->kSpin->value() << "\n";
}

bool ReinforcementInterfacePower::LoadParams(QString name, float value)
{
	if(name.endsWith("varianceSpin")) params->varianceSpin->setValue((float)value);
    if(name.endsWith("adaptiveCheck")) params->adaptiveCheck->setChecked((bool)value);
    if(name.endsWith("kSpin")) params->kSpin->setValue((float)value);
    return true;
}
