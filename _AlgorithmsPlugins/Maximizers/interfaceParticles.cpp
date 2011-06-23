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
#include "maximizeSwarm.h"
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
	double mutation = params->mutationSpin->value();
	bool inertia = params->adaptiveCheck->isChecked();
	double inertiaInit = params->inertiaInitSpin->value();
	double inertiaFinal = params->inertiaFinalSpin->value();
	double particleConfidence = params->particleConfidenceSpin->value();
	double swarmConfidence = params->swarmConfidenceSpin->value();

	((MaximizeSwarm *)maximizer)->SetParams(particleCount, mutation, inertia, inertiaInit, inertiaFinal, particleConfidence, swarmConfidence);
}

QString MaximizeInterfaceParticles::GetAlgoString()
{
	int particleCount = params->particleSpin->value();
	double mutation = params->mutationSpin->value();
	bool inertia = params->adaptiveCheck->isChecked();
	double inertiaInit = params->inertiaInitSpin->value();
	double inertiaFinal = params->inertiaFinalSpin->value();
	double particleConfidence = params->particleConfidenceSpin->value();
	double swarmConfidence = params->swarmConfidenceSpin->value();

	QString algo = QString("PSO %1 %2 %3 %4").arg(particleCount).arg(mutation).arg(particleConfidence).arg(swarmConfidence);
	if(inertia)
	{
		algo += QString(" %5 %6").arg(inertiaInit).arg(inertiaFinal);
	}

	return algo;
}

Maximizer *MaximizeInterfaceParticles::GetMaximizer()
{
	Maximizer *maximizer = new MaximizeSwarm();
	SetParams(maximizer);
	return maximizer;
}

void MaximizeInterfaceParticles::SaveOptions(QSettings &settings)
{
	settings.setValue("adaptiveCheck", params->adaptiveCheck->isChecked());
	settings.setValue("particleSpin", params->particleSpin->value());
	settings.setValue("mutationSpin", params->mutationSpin->value());
	settings.setValue("inertiaInitSpin", params->inertiaInitSpin->value());
	settings.setValue("inertiaFinalSpin", params->inertiaFinalSpin->value());
	settings.setValue("particleConfidenceSpin", params->particleConfidenceSpin->value());
	settings.setValue("swarmConfidenceSpin", params->swarmConfidenceSpin->value());
}

bool MaximizeInterfaceParticles::LoadOptions(QSettings &settings)
{
	if(settings.contains("adaptiveCheck")) params->adaptiveCheck->setChecked(settings.value("adaptiveCheck").toBool());
	if(settings.contains("particleSpin")) params->particleSpin->setValue(settings.value("particleSpin").toInt());
	if(settings.contains("mutationSpin")) params->mutationSpin->setValue(settings.value("mutationSpin").toDouble());
	if(settings.contains("inertiaInitSpin")) params->inertiaInitSpin->setValue(settings.value("inertiaInitSpin").toDouble());
	if(settings.contains("inertiaFinalSpin")) params->inertiaFinalSpin->setValue(settings.value("inertiaFinalSpin").toDouble());
	if(settings.contains("particleConfidenceSpin")) params->particleConfidenceSpin->setValue(settings.value("particleConfidenceSpin").toDouble());
	if(settings.contains("swarmConfidenceSpin")) params->swarmConfidenceSpin->setValue(settings.value("swarmConfidenceSpin").toDouble());
	return true;
}

void MaximizeInterfaceParticles::SaveParams(QTextStream &file)
{
	file << "maximizationOptions" << ":" << "adaptiveCheck" << " " << params->adaptiveCheck->isChecked() << "\n";
	file << "maximizationOptions" << ":" << "particleSpin" << " " << params->particleSpin->value() << "\n";
	file << "maximizationOptions" << ":" << "mutationSpin" << " " << params->mutationSpin->value() << "\n";
	file << "maximizationOptions" << ":" << "inertiaInitSpin" << " " << params->inertiaInitSpin->value() << "\n";
	file << "maximizationOptions" << ":" << "inertiaFinalSpin" << " " << params->inertiaFinalSpin->value() << "\n";
	file << "maximizationOptions" << ":" << "particleConfidenceSpin" << " " << params->particleConfidenceSpin->value() << "\n";
	file << "maximizationOptions" << ":" << "swarmConfidenceSpin" << " " << params->swarmConfidenceSpin->value() << "\n";
}

bool MaximizeInterfaceParticles::LoadParams(QString name, float value)
{
	if(name.endsWith("adaptiveCheck")) params->adaptiveCheck->setChecked((bool)value);
	if(name.endsWith("particleSpin")) params->particleSpin->setValue((int)value);
	if(name.endsWith("mutationSpin")) params->mutationSpin->setValue(value);
	if(name.endsWith("inertiaInitSpin")) params->inertiaInitSpin->setValue(value);
	if(name.endsWith("inertiaFinalSpin")) params->inertiaFinalSpin->setValue(value);
	if(name.endsWith("particleConfidenceSpin")) params->particleConfidenceSpin->setValue(value);
	if(name.endsWith("swarmConfidenceSpin")) params->swarmConfidenceSpin->setValue(value);
	return true;
}
