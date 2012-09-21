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
#include "interfaceDP.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>
#include <datasetManager.h>
#include <canvas.h>
#include "reinforcementDP.h"

using namespace std;

ReinforcementInterfaceDP::ReinforcementInterfaceDP()
{
    params = new Ui::ParametersDP();
	params->setupUi(widget = new QWidget());
    ChangeOptions();
}

void ReinforcementInterfaceDP::ChangeOptions()
{
}

void ReinforcementInterfaceDP::SetParams(Reinforcement *reinforcement)
{
    if(!reinforcement) return;
//	double variance = params->varianceSpin->value();
//    bool batchCheck= params->batchCheck->isChecked();
//    ((ReinforcementDP *)reinforcement)->SetParams(variance*variance, batchCheck);
}

Reinforcement *ReinforcementInterfaceDP::GetReinforcement()
{
    Reinforcement *reinforcement = NULL;
    reinforcement = new ReinforcementDP();
    SetParams(reinforcement);
    return reinforcement;
}

QString ReinforcementInterfaceDP::GetAlgoString()
{
//	double variance = params->varianceSpin->value();
//    bool batchCheck = params->batchCheck->isChecked();
    //    return QString("DP: %1 ").arg(variance) + (batchCheck ? "Batch" : "");
    return "DP";
}

void ReinforcementInterfaceDP::SaveOptions(QSettings &settings)
{
//	settings.setValue("varianceSpin", params->varianceSpin->value());
//    settings.setValue("batchCheck", params->batchCheck->isChecked());
}

bool ReinforcementInterfaceDP::LoadOptions(QSettings &settings)
{
//    if(settings.contains("varianceSpin")) params->varianceSpin->setValue(settings.value("varianceSpin").toFloat());
//    if(settings.contains("batchCheck")) params->batchCheck->setChecked(settings.value("batchCheck").toBool());
    return true;
}

void ReinforcementInterfaceDP::SaveParams(QTextStream &file)
{
//	file << "maximizationOptions:" << "varianceSpin" << " " << params->varianceSpin->value() << "\n";
//    file << "maximizationOptions:" << "batchCheck" << " " << params->batchCheck->isChecked() << "\n";
}

bool ReinforcementInterfaceDP::LoadParams(QString name, float value)
{
//	if(name.endsWith("varianceSpin")) params->varianceSpin->setValue((float)value);
//    if(name.endsWith("batchCheck")) params->batchCheck->setChecked((bool)value);
    return true;
}
