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
#include "interfaceMLPDynamic.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>

using namespace std;

DynamicMLP::DynamicMLP()
{
	params = new Ui::ParametersMLPDynamic();
	params->setupUi(widget = new QWidget());
}

void DynamicMLP::SetParams(Dynamical *dynamical)
{
	if(!dynamical) return;
	float alpha = params->mlpAlphaSpin->value();
	float beta = params->mlpBetaSpin->value();
	int layers = params->mlpLayerSpin->value();
	int neurons = params->mlpNeuronSpin->value();
	int activation = params->mlpFunctionCombo->currentIndex()+1; // 1: sigmoid, 2: gaussian

	((DynamicalMLP *)dynamical)->SetParams(activation, neurons, layers, alpha, beta);
}

Dynamical *DynamicMLP::GetDynamical()
{
	DynamicalMLP *dynamical = new DynamicalMLP();
	SetParams(dynamical);
	return dynamical;
}

void DynamicMLP::SaveOptions(QSettings &settings)
{
	settings.setValue("mlpNeuron", params->mlpNeuronSpin->value());
	settings.setValue("mlpAlpha", params->mlpAlphaSpin->value());
	settings.setValue("mlpBeta", params->mlpBetaSpin->value());
	settings.setValue("mlpLayer", params->mlpLayerSpin->value());
	settings.setValue("mlpFunction", params->mlpFunctionCombo->currentIndex());
}

bool DynamicMLP::LoadOptions(QSettings &settings)
{
	if(settings.contains("mlpNeuron")) params->mlpNeuronSpin->setValue(settings.value("mlpNeuron").toFloat());
	if(settings.contains("mlpAlpha")) params->mlpAlphaSpin->setValue(settings.value("mlpAlpha").toFloat());
	if(settings.contains("mlpBeta")) params->mlpBetaSpin->setValue(settings.value("mlpBeta").toFloat());
	if(settings.contains("mlpLayer")) params->mlpLayerSpin->setValue(settings.value("mlpLayer").toFloat());
	if(settings.contains("mlpFunction")) params->mlpFunctionCombo->setCurrentIndex(settings.value("mlpFunction").toInt());
	return true;
}

void DynamicMLP::SaveParams(std::ofstream &file)
{
	file << "dynamicalOptions" << ":" << "mlpNeuron" << " " << params->mlpNeuronSpin->value() << std::endl;
	file << "dynamicalOptions" << ":" << "mlpAlpha" << " " << params->mlpAlphaSpin->value() << std::endl;
	file << "dynamicalOptions" << ":" << "mlpBeta" << " " << params->mlpBetaSpin->value() << std::endl;
	file << "dynamicalOptions" << ":" << "mlpLayer" << " " << params->mlpLayerSpin->value() << std::endl;
	file << "dynamicalOptions" << ":" << "mlpFunction" << " " << params->mlpFunctionCombo->currentIndex() << std::endl;
}

bool DynamicMLP::LoadParams(char *line, float value)
{
	if(endsWith(line,"mlpNeuron")) params->mlpNeuronSpin->setValue((int)value);
	if(endsWith(line,"mlpAlpha")) params->mlpAlphaSpin->setValue(value);
	if(endsWith(line,"mlpBeta")) params->mlpBetaSpin->setValue(value);
	if(endsWith(line,"mlpLayer")) params->mlpLayerSpin->setValue((int)value);
	if(endsWith(line,"mlpFunction")) params->mlpFunctionCombo->setCurrentIndex((int)value);
	return true;
}
