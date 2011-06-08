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
#include "interfaceBasic.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>
#include <datasetManager.h>
#include <canvas.h>
#include "maximizeRandom.h"
#include "maximizePower.h"
#include "maximizeParticles.h"
#include "maximizeGradient.h"

using namespace std;

MaximizeBasic::MaximizeBasic()
{
	params = new Ui::ParametersMaximizers();
	params->setupUi(widget = new QWidget());
	connect(params->maximizeType, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeOptions()));
}

void MaximizeBasic::ChangeOptions()
{
	params->varianceSpin->setEnabled(false);
	params->kSpin->setEnabled(false);
	params->adaptiveCheck->setEnabled(false);
	params->countLabel->setText("K");
	params->varianceLabel->setText("Search Variance");
	switch(params->maximizeType->currentIndex())
	{
	case 0: // Random Search
		break;
	case 1: // Random Walk
		params->varianceSpin->setEnabled(true);
		break;
	case 2: // PoWER
		params->varianceSpin->setEnabled(true);
		params->kSpin->setEnabled(true);
		params->adaptiveCheck->setEnabled(true);
		break;
	case 3: // Particle Filters
		params->varianceSpin->setEnabled(true);
		params->kSpin->setEnabled(true);
		params->adaptiveCheck->setEnabled(true);
		params->countLabel->setText("Particles");
		break;
	case 4: // Gradient Descent
		params->adaptiveCheck->setEnabled(true);
		params->varianceSpin->setEnabled(true);
		params->varianceLabel->setText("Speed");
		break;
	}
}

void MaximizeBasic::SetParams(Maximizer *maximizer)
{
	if(!maximizer) return;
	int type = params->maximizeType->currentIndex();
	double variance = params->varianceSpin->value();
	int k = params->kSpin->value();
	bool bAdaptive = params->adaptiveCheck->isChecked();
	switch(type)
	{
	case 0: // random search
		((MaximizeRandom *)maximizer)->SetParams();
		break;
	case 1: // random walk
		((MaximizeRandom *)maximizer)->SetParams(variance*variance);
		break;
	case 2: // power
		((MaximizePower *)maximizer)->SetParams(k, variance*variance, bAdaptive);
		break;
	case 3: // particle filters
		((MaximizeParticles *)maximizer)->SetParams(k, variance*variance, bAdaptive);
		break;
	case 4: // particle filters
		((MaximizeGradient *)maximizer)->SetParams(variance, bAdaptive);
		break;
	}
}

Maximizer *MaximizeBasic::GetMaximizer()
{
	Maximizer *maximizer = NULL;
	switch(params->maximizeType->currentIndex())
	{
	case 0:
		maximizer = new MaximizeRandom();
		break;
	case 1:
		maximizer = new MaximizeRandom();
		break;
	case 2:
		maximizer = new MaximizePower();
		break;
	case 3:
		maximizer = new MaximizeParticles();
		break;
	case 4:
		maximizer = new MaximizeGradient();
		break;
	}
	SetParams(maximizer);
	return maximizer;
}

void MaximizeBasic::SaveOptions(QSettings &settings)
{
	settings.setValue("maximizeType", params->maximizeType->currentIndex());
	settings.setValue("varianceSpin", params->varianceSpin->value());
	settings.setValue("adaptiveCheck", params->adaptiveCheck->isChecked());
	settings.setValue("kSpin", params->kSpin->value());
}

bool MaximizeBasic::LoadOptions(QSettings &settings)
{
	if(settings.contains("maximizeType")) params->maximizeType->setCurrentIndex(settings.value("maximizeType").toInt());
	if(settings.contains("varianceSpin")) params->varianceSpin->setValue(settings.value("varianceSpin").toFloat());
	if(settings.contains("adaptiveCheck")) params->adaptiveCheck->setChecked(settings.value("adaptiveCheck").toBool());
	if(settings.contains("kSpin")) params->kSpin->setValue(settings.value("kSpin").toInt());
	return true;
}

void MaximizeBasic::SaveParams(std::ofstream &file)
{
	file << "maximizationOptions" << ":" << "maximizeType" << " " << params->maximizeType->currentIndex() << std::endl;
	file << "maximizationOptions" << ":" << "varianceSpin" << " " << params->varianceSpin->value() << std::endl;
	file << "maximizationOptions" << ":" << "adaptiveCheck" << " " << params->adaptiveCheck->isChecked() << std::endl;
	file << "maximizationOptions" << ":" << "kSpin" << " " << params->kSpin->value() << std::endl;
}

bool MaximizeBasic::LoadParams(char *line, float value)
{
	if(endsWith(line,"maximizeType")) params->maximizeType->setCurrentIndex((int)value);
	if(endsWith(line,"varianceSpin")) params->varianceSpin->setValue((float)value);
	if(endsWith(line,"adaptiveCheck")) params->adaptiveCheck->setChecked((bool)value);
	if(endsWith(line,"kSpin")) params->kSpin->setValue((int)value);
	return true;
}
