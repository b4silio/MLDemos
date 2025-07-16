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
#include "interfaceMLPRegress.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>

using namespace std;

RegrMLP::RegrMLP()
{
	params = new Ui::ParametersMLPRegress();
	params->setupUi(widget = new QWidget());
}

RegrMLP::~RegrMLP()
{
    delete params;
}

void RegrMLP::SetParams(Regressor *regressor)
{
	if(!regressor) return;
	float alpha = params->mlpAlphaSpin->value();
	float beta = params->mlpBetaSpin->value();
	int layers = params->mlpLayerSpin->value();
	int neurons = params->mlpNeuronSpin->value();
	int activation = params->mlpFunctionCombo->currentIndex()+1; // 1: sigmoid, 2: gaussian
    int trainingType = params->mlpTrainCombo->currentIndex();

    ((RegressorMLP *)regressor)->SetParams(activation, neurons, layers, alpha, beta, trainingType);
}

fvec RegrMLP::GetParams()
{
    float alpha = params->mlpAlphaSpin->value();
    float beta = params->mlpBetaSpin->value();
    int layers = params->mlpLayerSpin->value();
    int neurons = params->mlpNeuronSpin->value();
    int activation = params->mlpFunctionCombo->currentIndex()+1; // 1: sigmoid, 2: gaussian
    int trainingType = params->mlpTrainCombo->currentIndex();

    fvec par(6);
    par[0] = alpha;
    par[1] = beta;
    par[2] = layers;
    par[3] = neurons;
    par[4] = activation;
    par[5] = trainingType;
    return par;
}

void RegrMLP::SetParams(Regressor *regressor, fvec parameters)
{
    if(!regressor) return;
    float alpha = parameters.size() > 0 ? parameters[0] : 1;
    float beta = parameters.size() > 1 ? parameters[1] : 1;
    int layers = parameters.size() > 2 ? parameters[2] : 1;
    int neurons = parameters.size() > 3 ? parameters[3] : 1;
    int activation = parameters.size() > 4 ? parameters[4] : 0;
    int trainingType = parameters.size() > 5 ? parameters[5] : 0;

    ((RegressorMLP *)regressor)->SetParams(activation, neurons, layers, alpha, beta, trainingType);
}

void RegrMLP::GetParameterList(std::vector<QString> &parameterNames,
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
    parameterNames.push_back("Training Method");
    parameterTypes.push_back("Real");
    parameterTypes.push_back("Real");
    parameterTypes.push_back("Integer");
    parameterTypes.push_back("Integer");
    parameterTypes.push_back("List");
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
    parameterValues.back().push_back("Backpropagation");
    parameterValues.back().push_back("RPROP");
}

QString RegrMLP::GetAlgoString()
{
	float alpha = params->mlpAlphaSpin->value();
	float beta = params->mlpBetaSpin->value();
	int layers = params->mlpLayerSpin->value();
	int neurons = params->mlpNeuronSpin->value();
	int activation = params->mlpFunctionCombo->currentIndex()+1; // 1: sigmoid, 2: gaussian
    int training = params->mlpTrainCombo->currentIndex();

    QString algo = QString("MLP %1 %2 %3 %4 %5 %6")
            .arg(neurons)
            .arg(layers)
            .arg(activation==1 ? "S" : "G")
            .arg(alpha).arg(beta)
            .arg(training?"R" : "B");
    return algo;
}

Regressor *RegrMLP::GetRegressor()
{
	RegressorMLP *regressor = new RegressorMLP();
	SetParams(regressor);
	return regressor;
}

void RegrMLP::DrawConfidence(Canvas *canvas, Regressor *regressor)
{
	canvas->maps.confidence = QPixmap();
}

void RegrMLP::DrawModel(Canvas *canvas, QPainter &painter, Regressor *regressor)
{
	if(!regressor || !canvas) return;
	painter.setRenderHint(QPainter::Antialiasing, true);
    int xIndex = canvas->xIndex;

    int w = canvas->width();
    fvec sample = canvas->toSampleCoords(0,0);
    int dim = sample.size();
    if(dim > 2) return;
	int steps = w;
	QPointF oldPoint(-FLT_MAX,-FLT_MAX);
	FOR(x, steps)
	{
        sample = canvas->toSampleCoords(x,0);
		fvec res = regressor->Test(sample);
        if(res[0] != res[0]) continue; // NaN!
        QPointF point = canvas->toCanvasCoords(sample[xIndex], res[0]);
		if(x)
		{
			painter.setPen(QPen(Qt::black, 1));
			painter.drawLine(point, oldPoint);
		}
		oldPoint = point;
	}
}

void RegrMLP::SaveOptions(QSettings &settings)
{
	settings.setValue("mlpNeuron", params->mlpNeuronSpin->value());
	settings.setValue("mlpAlpha", params->mlpAlphaSpin->value());
	settings.setValue("mlpBeta", params->mlpBetaSpin->value());
	settings.setValue("mlpLayer", params->mlpLayerSpin->value());
	settings.setValue("mlpFunction", params->mlpFunctionCombo->currentIndex());
    settings.setValue("mlpTrain", params->mlpTrainCombo->currentIndex());
}

bool RegrMLP::LoadOptions(QSettings &settings)
{
	if(settings.contains("mlpNeuron")) params->mlpNeuronSpin->setValue(settings.value("mlpNeuron").toFloat());
	if(settings.contains("mlpAlpha")) params->mlpAlphaSpin->setValue(settings.value("mlpAlpha").toFloat());
	if(settings.contains("mlpBeta")) params->mlpBetaSpin->setValue(settings.value("mlpBeta").toFloat());
	if(settings.contains("mlpLayer")) params->mlpLayerSpin->setValue(settings.value("mlpLayer").toFloat());
	if(settings.contains("mlpFunction")) params->mlpFunctionCombo->setCurrentIndex(settings.value("mlpFunction").toInt());
    if(settings.contains("mlpTrain")) params->mlpTrainCombo->setCurrentIndex(settings.value("mlpTrain").toInt());
    return true;
}

void RegrMLP::SaveParams(QTextStream &file)
{
	file << "regressionOptions" << ":" << "mlpNeuron" << " " << params->mlpNeuronSpin->value() << "\n";
	file << "regressionOptions" << ":" << "mlpAlpha" << " " << params->mlpAlphaSpin->value() << "\n";
	file << "regressionOptions" << ":" << "mlpBeta" << " " << params->mlpBetaSpin->value() << "\n";
	file << "regressionOptions" << ":" << "mlpLayer" << " " << params->mlpLayerSpin->value() << "\n";
	file << "regressionOptions" << ":" << "mlpFunction" << " " << params->mlpFunctionCombo->currentIndex() << "\n";
    file << "classificationOptions" << ":" << "mlpTrain" << " " << params->mlpTrainCombo->currentIndex() << "\n";
}

bool RegrMLP::LoadParams(QString name, float value)
{
	if(name.endsWith("mlpNeuron")) params->mlpNeuronSpin->setValue((int)value);
	if(name.endsWith("mlpAlpha")) params->mlpAlphaSpin->setValue(value);
	if(name.endsWith("mlpBeta")) params->mlpBetaSpin->setValue(value);
	if(name.endsWith("mlpLayer")) params->mlpLayerSpin->setValue((int)value);
	if(name.endsWith("mlpFunction")) params->mlpFunctionCombo->setCurrentIndex((int)value);
    if(name.endsWith("mlpTrain")) params->mlpTrainCombo->setCurrentIndex((int)value);
    return true;
}
