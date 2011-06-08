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
#include "interfaceMLPClassifier.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>

using namespace std;

ClassMLP::ClassMLP()
{
	params = new Ui::ParametersMLP();
	params->setupUi(widget = new QWidget());
}

void ClassMLP::SetParams(Classifier *classifier)
{
	if(!classifier) return;
	float alpha = params->mlpAlphaSpin->value();
	float beta = params->mlpBetaSpin->value();
	int layers = params->mlpLayerSpin->value();
	int neurons = params->mlpNeuronSpin->value();
	int activation = params->mlpFunctionCombo->currentIndex()+1; // 1: sigmoid, 2: gaussian

	((ClassifierMLP *)classifier)->SetParams(activation, neurons, layers, alpha, beta);
}

Classifier *ClassMLP::GetClassifier()
{
	ClassifierMLP *classifier = new ClassifierMLP();
	SetParams(classifier);
	return classifier;
}

void ClassMLP::DrawModel(Canvas *canvas, QPainter &painter, Classifier *classifier)
{
	if(!classifier || !canvas) return;
	painter.setRenderHint(QPainter::Antialiasing, true);
	int posClass = 1;
	bool bUseMinMax = true;
	float resMin = FLT_MAX;
	float resMax = -FLT_MAX;
	if(bUseMinMax)
	{
		// TODO: get the min and max for all samples
		std::vector<fvec> samples = canvas->data->GetSamples();
		FOR(i, samples.size())
		{
			float val = classifier->Test(samples[i]);
			if(val > resMax) resMax = val;
			if(val < resMin) resMin = val;
		}
		if(resMin == resMax) resMin -= 3;
	}
	FOR(i, canvas->data->GetCount())
	{
		fvec sample = canvas->data->GetSample(i);
		int label = canvas->data->GetLabel(i);
		QPointF point = canvas->toCanvasCoords(canvas->data->GetSample(i));
		float response = classifier->Test(sample);
		if(response > 0)
		{
			if(label == posClass) Canvas::drawSample(painter, point, 9, 2);
			else Canvas::drawCross(painter, point, 6, 2);
		}
		else
		{
			if(label != posClass) Canvas::drawSample(painter, point, 9, 0);
			else Canvas::drawCross(painter, point, 6, 0);
		}
	}
}

void ClassMLP::SaveOptions(QSettings &settings)
{
	settings.setValue("mlpNeuron", params->mlpNeuronSpin->value());
	settings.setValue("mlpAlpha", params->mlpAlphaSpin->value());
	settings.setValue("mlpBeta", params->mlpBetaSpin->value());
	settings.setValue("mlpLayer", params->mlpLayerSpin->value());
	settings.setValue("mlpFunction", params->mlpFunctionCombo->currentIndex());
}

bool ClassMLP::LoadOptions(QSettings &settings)
{
	if(settings.contains("mlpNeuron")) params->mlpNeuronSpin->setValue(settings.value("mlpNeuron").toFloat());
	if(settings.contains("mlpAlpha")) params->mlpAlphaSpin->setValue(settings.value("mlpAlpha").toFloat());
	if(settings.contains("mlpBeta")) params->mlpBetaSpin->setValue(settings.value("mlpBeta").toFloat());
	if(settings.contains("mlpLayer")) params->mlpLayerSpin->setValue(settings.value("mlpLayer").toFloat());
	if(settings.contains("mlpFunction")) params->mlpFunctionCombo->setCurrentIndex(settings.value("mlpFunction").toInt());
	return true;
}

void ClassMLP::SaveParams(std::ofstream &file)
{
	file << "classificationOptions" << ":" << "mlpNeuron" << " " << params->mlpNeuronSpin->value() << std::endl;
	file << "classificationOptions" << ":" << "mlpAlpha" << " " << params->mlpAlphaSpin->value() << std::endl;
	file << "classificationOptions" << ":" << "mlpBeta" << " " << params->mlpBetaSpin->value() << std::endl;
	file << "classificationOptions" << ":" << "mlpLayer" << " " << params->mlpLayerSpin->value() << std::endl;
	file << "classificationOptions" << ":" << "mlpFunction" << " " << params->mlpFunctionCombo->currentIndex() << std::endl;
}

bool ClassMLP::LoadParams(char *line, float value)
{
	if(endsWith(line,"mlpNeuron")) params->mlpNeuronSpin->setValue((int)value);
	if(endsWith(line,"mlpAlpha")) params->mlpAlphaSpin->setValue(value);
	if(endsWith(line,"mlpBeta")) params->mlpBetaSpin->setValue(value);
	if(endsWith(line,"mlpLayer")) params->mlpLayerSpin->setValue((int)value);
	if(endsWith(line,"mlpFunction")) params->mlpFunctionCombo->setCurrentIndex((int)value);
	return true;
}
