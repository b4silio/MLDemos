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
#include "interfaceKNNDynamic.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>

using namespace std;

DynamicKNN::DynamicKNN()
{
	params = new Ui::ParametersKNNDynamic();
	params->setupUi(widget = new QWidget());
    connect(params->knnNormCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeOptions()));
    ChangeOptions();
}

void DynamicKNN::ChangeOptions()
{
    params->knnNormSpin->setVisible(params->knnNormCombo->currentIndex() == 2);
    params->labelPower->setVisible(params->knnNormCombo->currentIndex() == 2);
}

void DynamicKNN::SetParams(Dynamical *dynamical)
{
	if(!dynamical) return;
    SetParams(dynamical, GetParams());
}

fvec DynamicKNN::GetParams()
{
    fvec par(3);
    par[0] = params->knnKspin->value();
    par[1] = params->knnNormCombo->currentIndex();
    par[2] = params->knnNormSpin->value();
    return par;
}

void DynamicKNN::SetParams(Dynamical *dynamical, fvec parameters)
{
    if(!dynamical) return;
    int k = parameters.size() > 0 ? parameters[0] : 1;
    int metricType = parameters.size() > 1 ? parameters[1] : 0;
    int metricP = parameters.size() > 2 ? parameters[2] : 0;
    ((DynamicalKNN *)dynamical)->SetParams(k, metricType, metricP);
}

void DynamicKNN::GetParameterList(std::vector<QString> &parameterNames,
                             std::vector<QString> &parameterTypes,
                             std::vector< std::vector<QString> > &parameterValues)
{
    parameterNames.push_back("K");
    parameterNames.push_back("Metric Type");
    parameterNames.push_back("Metric Power");
    parameterTypes.push_back("Integer");
    parameterTypes.push_back("List");
    parameterTypes.push_back("Integer");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("1");
    parameterValues.back().push_back("999");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("L1");
    parameterValues.back().push_back("L2");
    parameterValues.back().push_back("Lp");
    parameterValues.back().push_back("L-Inf");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("1");
    parameterValues.back().push_back("150");
}

Dynamical *DynamicKNN::GetDynamical()
{
	DynamicalKNN *dynamical = new DynamicalKNN();
	SetParams(dynamical);
	return dynamical;
}

void DynamicKNN::SaveOptions(QSettings &settings)
{
	settings.setValue("knnK", params->knnKspin->value());
	settings.setValue("knnNorm", params->knnNormCombo->currentIndex());
	settings.setValue("knnPower", params->knnNormSpin->value());
}

bool DynamicKNN::LoadOptions(QSettings &settings)
{
	if(settings.contains("knnK")) params->knnKspin->setValue(settings.value("knnK").toFloat());
	if(settings.contains("knnNorm")) params->knnNormCombo->setCurrentIndex(settings.value("knnNorm").toInt());
	if(settings.contains("knnPower")) params->knnNormSpin->setValue(settings.value("knnPower").toFloat());
	return true;
}

void DynamicKNN::SaveParams(QTextStream &file)
{
	file << "dynamicalOptions" << ":" << "knnK" << " " << params->knnKspin->value() << "\n";
	file << "dynamicalOptions" << ":" << "knnNorm" << " " << params->knnNormCombo->currentIndex() << "\n";
	file << "dynamicalOptions" << ":" << "knnPower" << " " << params->knnNormSpin->value() << "\n";
}

bool DynamicKNN::LoadParams(QString name, float value)
{
	if(name.endsWith("knnK")) params->knnKspin->setValue((int)value);
	if(name.endsWith("knnNorm")) params->knnNormCombo->setCurrentIndex((int)value);
	if(name.endsWith("knnPower")) params->knnNormSpin->setValue((int)value);
	return true;
}
