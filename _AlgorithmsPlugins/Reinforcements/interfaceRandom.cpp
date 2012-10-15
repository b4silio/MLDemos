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
#include "interfaceRandom.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>
#include <datasetManager.h>
#include <canvas.h>
#include "reinforcementRandom.h"

using namespace std;

ReinforcementInterfaceRandom::ReinforcementInterfaceRandom()
{
    params = new Ui::ParametersRandom();
	params->setupUi(widget = new QWidget());
    ChangeOptions();
}

void ReinforcementInterfaceRandom::ChangeOptions()
{
}

void ReinforcementInterfaceRandom::SetParams(Reinforcement *reinforcement)
{
    if(!reinforcement) return;
	double variance = params->varianceSpin->value();
    bool bSingleDim = params->singleDimCheck->isChecked();
    ((ReinforcementRandom *)reinforcement)->SetParams(variance*variance, bSingleDim);
}

Reinforcement *ReinforcementInterfaceRandom::GetReinforcement()
{
    Reinforcement *reinforcement = NULL;
    reinforcement = new ReinforcementRandom();
    SetParams(reinforcement);
    return reinforcement;
}

QString ReinforcementInterfaceRandom::GetAlgoString()
{
	double variance = params->varianceSpin->value();
    bool singleDimCheck = params->singleDimCheck->isChecked();
    return QString("Random Walk: %1 ").arg(variance) + (singleDimCheck ? "Single" : "");
}

void ReinforcementInterfaceRandom::SaveOptions(QSettings &settings)
{
	settings.setValue("varianceSpin", params->varianceSpin->value());
    settings.setValue("singleDimCheck", params->singleDimCheck->isChecked());
}

bool ReinforcementInterfaceRandom::LoadOptions(QSettings &settings)
{
	if(settings.contains("varianceSpin")) params->varianceSpin->setValue(settings.value("varianceSpin").toFloat());
    if(settings.contains("singleDimCheck")) params->singleDimCheck->setChecked(settings.value("singleDimCheck").toBool());
    return true;
}

void ReinforcementInterfaceRandom::SaveParams(QTextStream &file)
{
	file << "maximizationOptions:" << "varianceSpin" << " " << params->varianceSpin->value() << "\n";
    file << "maximizationOptions:" << "singleDimCheck" << " " << params->singleDimCheck->isChecked() << "\n";
}

bool ReinforcementInterfaceRandom::LoadParams(QString name, float value)
{
	if(name.endsWith("varianceSpin")) params->varianceSpin->setValue((float)value);
    if(name.endsWith("singleDimCheck")) params->singleDimCheck->setChecked((bool)value);
    return true;
}
