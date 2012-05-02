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
#include "reinforcementRandom.h"

using namespace std;

ReinforceRandom::ReinforceRandom()
{
    params = new Ui::ParametersRandom();
	params->setupUi(widget = new QWidget());
    ChangeOptions();
}

void ReinforceRandom::ChangeOptions()
{
	params->varianceLabel->setText("Search Variance");
	params->varianceSpin->setDecimals(2);
	params->varianceSpin->setRange(0.01,1);
	params->varianceSpin->setSingleStep(0.01);
}

void ReinforceRandom::SetParams(Reinforcement *reinforcement)
{
    if(!reinforcement) return;
	double variance = params->varianceSpin->value();
    bool bSingleDim = params->singleDimCheck->isChecked();
    int policyType = params->policyCombo->currentIndex();
    int gridSize = params->resolutionSpin->value();
    int quantizeType = params->quantizeCombo->currentIndex();
    ((ReinforcementRandom *)reinforcement)->SetParams(variance*variance, policyType, gridSize, bSingleDim, quantizeType);
}

Reinforcement *ReinforceRandom::GetReinforcement()
{
    Reinforcement *reinforcement = NULL;
    reinforcement = new ReinforcementRandom();
    SetParams(reinforcement);
    return reinforcement;
}

void ReinforceRandom::DrawInfo(Canvas *canvas, QPainter &painter, Reinforcement *reinforcement)
{

}

void ReinforceRandom::DrawModel(Canvas *canvas, QPainter &painter, Reinforcement *reinforcement)
{

}

QString ReinforceRandom::GetAlgoString()
{
	double variance = params->varianceSpin->value();
    bool singleDimCheck = params->singleDimCheck->isChecked();
    int policyType = params->policyCombo->currentIndex();
    int resolution = params->resolutionSpin->value();
    int quantizeType = params->quantizeCombo->currentIndex();
    return QString("Random Walk: %1").arg(variance);
}

void ReinforceRandom::SaveOptions(QSettings &settings)
{
	settings.setValue("varianceSpin", params->varianceSpin->value());
    settings.setValue("singleDimCheck", params->singleDimCheck->isChecked());
    settings.setValue("quantizeCombo", params->quantizeCombo->currentIndex());
    settings.setValue("resolutionSpin", params->resolutionSpin->value());
    settings.setValue("policyCombo", params->policyCombo->currentIndex());
}

bool ReinforceRandom::LoadOptions(QSettings &settings)
{
	if(settings.contains("varianceSpin")) params->varianceSpin->setValue(settings.value("varianceSpin").toFloat());
    if(settings.contains("singleDimCheck")) params->singleDimCheck->setChecked(settings.value("singleDimCheck").toBool());
    if(settings.contains("quantizeCombo")) params->quantizeCombo->setCurrentIndex(settings.value("quantizeCheck").toInt());
    if(settings.contains("resolutionSpin")) params->resolutionSpin->setValue(settings.value("resolutionSpin").toInt());
    if(settings.contains("policyCombo")) params->policyCombo->setCurrentIndex(settings.value("policyCombo").toInt());
    return true;
}

void ReinforceRandom::SaveParams(QTextStream &file)
{
	file << "maximizationOptions:" << "varianceSpin" << " " << params->varianceSpin->value() << "\n";
    file << "maximizationOptions:" << "singleDimCheck" << " " << params->singleDimCheck->isChecked() << "\n";
    file << "maximizationOptions:" << "quantizeCombo" << " " << params->quantizeCombo->currentIndex() << "\n";
    file << "maximizationOptions:" << "resolutionSpin" << " " << params->resolutionSpin->value() << "\n";
    file << "maximizationOptions:" << "policyCombo" << " " << params->policyCombo->currentIndex() << "\n";
}

bool ReinforceRandom::LoadParams(QString name, float value)
{
	if(name.endsWith("varianceSpin")) params->varianceSpin->setValue((float)value);
    if(name.endsWith("singleDimCheck")) params->singleDimCheck->setChecked((bool)value);
    if(name.endsWith("quantizeCombo")) params->quantizeCombo->setCurrentIndex((int)value);
    if(name.endsWith("resolutionSpin")) params->resolutionSpin->setValue((int)value);
    if(name.endsWith("policyCombo")) params->policyCombo->setCurrentIndex((int)value);
    return true;
}
