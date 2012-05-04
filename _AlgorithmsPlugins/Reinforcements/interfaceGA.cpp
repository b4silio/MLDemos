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
#include "interfaceGA.h"
#include "reinforcementGA.h"
#include <QPixmap>
#include <QDebug>
#include <QBitmap>
#include <QPainter>
#include <datasetManager.h>
#include <canvas.h>

using namespace std;

ReinforcementInterfaceGA::ReinforcementInterfaceGA()
{
	params = new Ui::ParametersGA();
	params->setupUi(widget = new QWidget());
}

void ReinforcementInterfaceGA::SetParams(Reinforcement *reinforcement)
{
    if(!reinforcement) return;
	double mutation = params->mutationSpin->value();
	double cross = params->crossSpin->value();
	double survival = params->survivalSpin->value();
	int population = params->populationSpin->value();
    ((ReinforcementGA *)reinforcement)->SetParams(mutation, cross, survival, population);
}

QString ReinforcementInterfaceGA::GetAlgoString()
{
	double mutation = params->mutationSpin->value();
	double cross = params->crossSpin->value();
	double survival = params->survivalSpin->value();
	int population = params->populationSpin->value();
	QString	algo = QString("GA %1 %2 %3 %4").arg(population).arg(mutation).arg(cross).arg(survival);
	return algo;
}

Reinforcement *ReinforcementInterfaceGA::GetReinforcement()
{
    Reinforcement *reinforcement = new ReinforcementGA();
    SetParams(reinforcement);
    return reinforcement;
}

void ReinforcementInterfaceGA::SaveOptions(QSettings &settings)
{
	settings.setValue("populationSpin", params->populationSpin->value());
	settings.setValue("mutationSpin", params->mutationSpin->value());
	settings.setValue("crossSpin", params->crossSpin->value());
	settings.setValue("survivalSpin", params->survivalSpin->value());
}

bool ReinforcementInterfaceGA::LoadOptions(QSettings &settings)
{
	if(settings.contains("populationSpin")) params->populationSpin->setValue(settings.value("populationSpin").toFloat());
	if(settings.contains("mutationSpin")) params->mutationSpin->setValue(settings.value("mutationSpin").toFloat());
	if(settings.contains("crossSpin")) params->crossSpin->setValue(settings.value("crossSpin").toFloat());
	if(settings.contains("survivalSpin")) params->survivalSpin->setValue(settings.value("survivalSpin").toFloat());
	return true;
}

void ReinforcementInterfaceGA::SaveParams(QTextStream &file)
{
	file << "maximizationOptions" << ":" << "populationSpin" << " " << params->populationSpin->value() << "\n";
	file << "maximizationOptions" << ":" << "mutationSpin" << " " << params->mutationSpin->value() << "\n";
	file << "maximizationOptions" << ":" << "crossSpin" << " " << params->crossSpin->value() << "\n";
	file << "maximizationOptions" << ":" << "survivalSpin" << " " << params->survivalSpin->value() << "\n";
}

bool ReinforcementInterfaceGA::LoadParams(QString name, float value)
{
	if(name.endsWith("populationSpin")) params->populationSpin->setValue((int)value);
	if(name.endsWith("mutationSpin")) params->mutationSpin->setValue((float)value);
	if(name.endsWith("crossSpin")) params->crossSpin->setValue((float)value);
	if(name.endsWith("survivalSpin")) params->survivalSpin->setValue((float)value);
	return true;
}
