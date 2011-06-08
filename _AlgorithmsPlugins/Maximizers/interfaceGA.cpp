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
#include "maximizeGA.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>
#include <datasetManager.h>
#include <canvas.h>

using namespace std;

MaximizeInterfaceGA::MaximizeInterfaceGA()
{
	params = new Ui::ParametersGA();
	params->setupUi(widget = new QWidget());
}

void MaximizeInterfaceGA::SetParams(Maximizer *maximizer)
{
	if(!maximizer) return;
	double mutation = params->mutationSpin->value();
	double cross = params->crossSpin->value();
	double survival = params->survivalSpin->value();
	int population = params->populationSpin->value();
	((MaximizeGA *)maximizer)->SetParams(mutation, cross, survival, population);
}

Maximizer *MaximizeInterfaceGA::GetMaximizer()
{
	Maximizer *maximizer = new MaximizeGA();
	SetParams(maximizer);
	return maximizer;
}

void MaximizeInterfaceGA::SaveOptions(QSettings &settings)
{
	settings.setValue("mutationSpin", params->mutationSpin->value());
	settings.setValue("crossSpin", params->crossSpin->value());
	settings.setValue("survivalSpin", params->survivalSpin->value());
}

bool MaximizeInterfaceGA::LoadOptions(QSettings &settings)
{
	if(settings.contains("mutationSpin")) params->mutationSpin->setValue(settings.value("mutationSpin").toFloat());
	if(settings.contains("crossSpin")) params->crossSpin->setValue(settings.value("crossSpin").toFloat());
	if(settings.contains("survivalSpin")) params->survivalSpin->setValue(settings.value("survivalSpin").toFloat());
	return true;
}

void MaximizeInterfaceGA::SaveParams(std::ofstream &file)
{
	file << "maximizationOptions" << ":" << "mutationSpin" << " " << params->mutationSpin->value() << std::endl;
	file << "maximizationOptions" << ":" << "crossSpin" << " " << params->crossSpin->value() << std::endl;
	file << "maximizationOptions" << ":" << "survivalSpin" << " " << params->survivalSpin->value() << std::endl;
}

bool MaximizeInterfaceGA::LoadParams(char *line, float value)
{
	if(endsWith(line,"mutationSpin")) params->mutationSpin->setValue((float)value);
	if(endsWith(line,"crossSpin")) params->crossSpin->setValue((float)value);
	if(endsWith(line,"survivalSpin")) params->survivalSpin->setValue((int)value);
	return true;
}
