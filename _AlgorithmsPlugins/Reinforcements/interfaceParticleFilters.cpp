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
#include "interfaceParticleFilters.h"
#include "maximizeParticles.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>
#include <datasetManager.h>
#include <canvas.h>

using namespace std;

MaximizeInterfaceParticleFilters::MaximizeInterfaceParticleFilters()
{
    params = new Ui::ParametersParticleFilters();
	params->setupUi(widget = new QWidget());
}

void MaximizeInterfaceParticleFilters::SetParams(Maximizer *maximizer)
{
	if(!maximizer) return;
    int particleCount = params->countSpin->value();
    double keeperCount = params->copiesSpin->value() / 100.; // it's a percentage!
    double newCount = params->newParticlesSpin->value() / 100.; // it's a percentage!
    double variance = params->varianceSpin->value();
    bool bAdaptive = params->adaptiveCheck->isChecked();

    MaximizeParticles *parts = dynamic_cast<MaximizeParticles*>(maximizer);
    if(!parts) return;
    parts->SetParams(particleCount, variance, keeperCount, newCount, bAdaptive);
}

QString MaximizeInterfaceParticleFilters::GetAlgoString()
{
    int particleCount = params->countSpin->value();
    double keepers = params->copiesSpin->value();
    bool adaptive = params->adaptiveCheck->isChecked();
    double randoms = params->newParticlesSpin->value();
    double variance = params->varianceSpin->value();

    QString algo = QString("PF %1 %2 %3 %4").arg(particleCount).arg(variance).arg(keepers).arg(randoms);
    if(adaptive) algo += " A";

	return algo;
}

Maximizer *MaximizeInterfaceParticleFilters::GetMaximizer()
{
    Maximizer *maximizer = new MaximizeParticles();
	SetParams(maximizer);
	return maximizer;
}

void MaximizeInterfaceParticleFilters::SaveOptions(QSettings &settings)
{
    settings.setValue("adaptiveCheck", params->adaptiveCheck->isChecked());
    settings.setValue("countSpin", params->countSpin->value());
    settings.setValue("copiesSpin", params->copiesSpin->value());
    settings.setValue("newParticlesSpin", params->newParticlesSpin->value());
    settings.setValue("varianceSpin", params->varianceSpin->value());
}

bool MaximizeInterfaceParticleFilters::LoadOptions(QSettings &settings)
{
	if(settings.contains("adaptiveCheck")) params->adaptiveCheck->setChecked(settings.value("adaptiveCheck").toBool());
    if(settings.contains("countSpin")) params->countSpin->setValue(settings.value("countSpin").toInt());
    if(settings.contains("copiesSpin")) params->copiesSpin->setValue(settings.value("copiesSpin").toDouble());
    if(settings.contains("newParticlesSpin")) params->newParticlesSpin->setValue(settings.value("newParticlesSpin").toDouble());
    if(settings.contains("varianceSpin")) params->varianceSpin->setValue(settings.value("varianceSpin").toDouble());
	return true;
}

void MaximizeInterfaceParticleFilters::SaveParams(QTextStream &file)
{
	file << "maximizationOptions" << ":" << "adaptiveCheck" << " " << params->adaptiveCheck->isChecked() << "\n";
    file << "maximizationOptions" << ":" << "countSpin" << " " << params->countSpin->value() << "\n";
    file << "maximizationOptions" << ":" << "copiesSpin" << " " << params->copiesSpin->value() << "\n";
    file << "maximizationOptions" << ":" << "newParticlesSpin" << " " << params->newParticlesSpin->value() << "\n";
    file << "maximizationOptions" << ":" << "varianceSpin" << " " << params->varianceSpin->value() << "\n";
}

bool MaximizeInterfaceParticleFilters::LoadParams(QString name, float value)
{
	if(name.endsWith("adaptiveCheck")) params->adaptiveCheck->setChecked((bool)value);
    if(name.endsWith("countSpin")) params->countSpin->setValue((int)value);
    if(name.endsWith("copiesSpin")) params->copiesSpin->setValue(value);
    if(name.endsWith("newParticlesSpin")) params->newParticlesSpin->setValue(value);
    if(name.endsWith("varianceSpin")) params->varianceSpin->setValue(value);
	return true;
}
