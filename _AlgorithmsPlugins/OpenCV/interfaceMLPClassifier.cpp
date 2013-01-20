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

fvec ClassMLP::GetParams()
{
    float alpha = params->mlpAlphaSpin->value();
    float beta = params->mlpBetaSpin->value();
    int layers = params->mlpLayerSpin->value();
    int neurons = params->mlpNeuronSpin->value();
    int activation = params->mlpFunctionCombo->currentIndex()+1; // 1: sigmoid, 2: gaussian

    fvec par(5);
    par[0] = alpha;
    par[1] = beta;
    par[2] = layers;
    par[3] = neurons;
    par[4] = activation;
    return par;
}

void ClassMLP::SetParams(Classifier *classifier, fvec parameters)
{
    if(!classifier) return;
    float alpha = parameters.size() > 0 ? parameters[0] : 1;
    float beta = parameters.size() > 1 ? parameters[1] : 1;
    int layers = parameters.size() > 2 ? parameters[2] : 1;
    int neurons = parameters.size() > 3 ? parameters[3] : 1;
    int activation = parameters.size() > 4 ? parameters[4] : 0;

    ((ClassifierMLP *)classifier)->SetParams(activation, neurons, layers, alpha, beta);
}

void ClassMLP::GetParameterList(std::vector<QString> &parameterNames,
                             std::vector<QString> &parameterTypes,
                             std::vector< std::vector<QString> > &parameterValues)
{
    parameterNames.clear();
    parameterTypes.clear();
    parameterValues.clear();
    parameterNames.push_back("Alpha");
    parameterNames.push_back("Beta");
    parameterNames.push_back("Hidden Layers");
    parameterNames.push_back("Neurons per Layer");
    parameterNames.push_back("Activation Function");
    parameterTypes.push_back("Real");
    parameterTypes.push_back("Real");
    parameterTypes.push_back("Integer");
    parameterTypes.push_back("Integer");
    parameterTypes.push_back("List");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("0.00000001f");
    parameterValues.back().push_back("9999999.f");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("0.00000001f");
    parameterValues.back().push_back("9999999.f");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("1");
    parameterValues.back().push_back("999999");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("1");
    parameterValues.back().push_back("999999");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("Hyperbolic Tangent");
    parameterValues.back().push_back("Gaussian");
}

QString ClassMLP::GetAlgoString()
{
	float alpha = params->mlpAlphaSpin->value();
	float beta = params->mlpBetaSpin->value();
	int layers = params->mlpLayerSpin->value();
	int neurons = params->mlpNeuronSpin->value();
	int activation = params->mlpFunctionCombo->currentIndex()+1; // 1: sigmoid, 2: gaussian

	QString algo = QString("MLP %1 %2 %3 %4 %5").arg(neurons).arg(layers).arg(activation==1 ? "S" : "G").arg(alpha).arg(beta);
	return algo;
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
			if(label == posClass) Canvas::drawSample(painter, point, 9, 1);
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

void ClassMLP::SaveParams(QTextStream &file)
{
	file << "classificationOptions" << ":" << "mlpNeuron" << " " << params->mlpNeuronSpin->value() << "\n";
	file << "classificationOptions" << ":" << "mlpAlpha" << " " << params->mlpAlphaSpin->value() << "\n";
	file << "classificationOptions" << ":" << "mlpBeta" << " " << params->mlpBetaSpin->value() << "\n";
	file << "classificationOptions" << ":" << "mlpLayer" << " " << params->mlpLayerSpin->value() << "\n";
	file << "classificationOptions" << ":" << "mlpFunction" << " " << params->mlpFunctionCombo->currentIndex() << "\n";
}

bool ClassMLP::LoadParams(QString name, float value)
{
	if(name.endsWith("mlpNeuron")) params->mlpNeuronSpin->setValue((int)value);
	if(name.endsWith("mlpAlpha")) params->mlpAlphaSpin->setValue(value);
	if(name.endsWith("mlpBeta")) params->mlpBetaSpin->setValue(value);
	if(name.endsWith("mlpLayer")) params->mlpLayerSpin->setValue((int)value);
	if(name.endsWith("mlpFunction")) params->mlpFunctionCombo->setCurrentIndex((int)value);
	return true;
}
