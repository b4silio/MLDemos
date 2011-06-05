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
#include "interfaceParticles.h"
#include "maximizeParticles.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>
#include <datasetManager.h>
#include <canvas.h>

using namespace std;

MaximizeInterfaceParticles::MaximizeInterfaceParticles()
{
	params = new Ui::ParametersParticles();
	params->setupUi(widget = new QWidget());
}

void MaximizeInterfaceParticles::SetParams(Maximizer *maximizer)
{
	if(!maximizer) return;
	int particleCount = params->particleSpin->value();
	double variance = params->varianceSpin->value();
	bool adaptive = params->adaptiveCheck->isChecked();
	int samplingType = params->samplingType->currentIndex();
	((MaximizeParticles *)maximizer)->SetParams(particleCount, variance, adaptive);
}

Maximizer *MaximizeInterfaceParticles::GetMaximizer()
{
	Maximizer *maximizer = new MaximizeParticles();
	SetParams(maximizer);
	return maximizer;
}

void MaximizeInterfaceParticles::SaveOptions(QSettings &settings)
{
	settings.setValue("samplingType", params->samplingType->currentIndex());
	settings.setValue("varianceSpin", params->varianceSpin->value());
	settings.setValue("adaptiveCheck", params->adaptiveCheck->isChecked());
	settings.setValue("particleSpin", params->particleSpin->value());
}

bool MaximizeInterfaceParticles::LoadOptions(QSettings &settings)
{
	if(settings.contains("samplingType")) params->samplingType->setCurrentIndex(settings.value("samplingType").toInt());
	if(settings.contains("varianceSpin")) params->varianceSpin->setValue(settings.value("varianceSpin").toFloat());
	if(settings.contains("adaptiveCheck")) params->adaptiveCheck->setChecked(settings.value("adaptiveCheck").toBool());
	if(settings.contains("particleSpin")) params->particleSpin->setValue(settings.value("particleSpin").toInt());
	return true;
}

void MaximizeInterfaceParticles::SaveParams(std::ofstream &file)
{
	file << "maximizationOptions" << ":" << "samplingType" << " " << params->samplingType->currentIndex() << std::endl;
	file << "maximizationOptions" << ":" << "varianceSpin" << " " << params->varianceSpin->value() << std::endl;
	file << "maximizationOptions" << ":" << "adaptiveCheck" << " " << params->adaptiveCheck->isChecked() << std::endl;
	file << "maximizationOptions" << ":" << "particleSpin" << " " << params->particleSpin->value() << std::endl;
}

bool MaximizeInterfaceParticles::LoadParams(char *line, float value)
{
	if(endsWith(line,"samplingType")) params->samplingType->setCurrentIndex((int)value);
	if(endsWith(line,"varianceSpin")) params->varianceSpin->setValue((float)value);
	if(endsWith(line,"adaptiveCheck")) params->adaptiveCheck->setChecked((bool)value);
	if(endsWith(line,"particleSpin")) params->particleSpin->setValue((int)value);
	return true;
}
