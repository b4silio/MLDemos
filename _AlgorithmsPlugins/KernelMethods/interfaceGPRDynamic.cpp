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
#include "interfaceGPRDynamic.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>

using namespace std;

DynamicGPR::DynamicGPR()
{
        params = new Ui::ParametersGPRDynamic();
	params->setupUi(widget = new QWidget());
        connect(params->kernelTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeOptions()));
}

void DynamicGPR::ChangeOptions()
{
    bool bSparse = params->sparseCheck->isChecked();

    params->capacitySpin->setEnabled(bSparse);
    params->noiseSpin->setEnabled(bSparse);

    switch(params->kernelTypeCombo->currentIndex())
    {
    case 0: // linear
        params->kernelDegSpin->setEnabled(false);
        params->kernelDegSpin->setVisible(false);
        break;
    case 1: // poly
        params->kernelDegSpin->setEnabled(true);
        params->kernelDegSpin->setVisible(true);
        params->kernelWidthSpin->setEnabled(false);
        params->kernelWidthSpin->setVisible(false);
        break;
    case 2: // RBF
        params->kernelDegSpin->setEnabled(false);
        params->kernelDegSpin->setVisible(false);
        params->kernelWidthSpin->setEnabled(true);
        params->kernelWidthSpin->setVisible(true);
        break;
    }
}

void DynamicGPR::SetParams(Dynamical *dynamical)
{
	if(!dynamical) return;
        float noise = params->noiseSpin->value();
	int kernelType = params->kernelTypeCombo->currentIndex();
	float kernelGamma = params->kernelWidthSpin->value();
	float kernelDegree = params->kernelDegSpin->value();
        int capacity = params->capacitySpin->value();

        DynamicalGPR *gpr = (DynamicalGPR*)dynamical;
        gpr->SetParams(kernelGamma, noise, capacity, kernelType, kernelDegree);
}

Dynamical *DynamicGPR::GetDynamical()
{
        Dynamical *dynamical = new DynamicalGPR();
	SetParams(dynamical);
	return dynamical;
}

void DynamicGPR::DrawModel(Canvas *canvas, QPainter &painter, Dynamical *dynamical)
{
}

void DynamicGPR::DrawInfo(Canvas *canvas, QPainter &painter, Dynamical *dynamical)
{
}

void DynamicGPR::SaveOptions(QSettings &settings)
{
	settings.setValue("kernelDeg", params->kernelDegSpin->value());
	settings.setValue("kernelType", params->kernelTypeCombo->currentIndex());
	settings.setValue("kernelWidth", params->kernelWidthSpin->value());
        settings.setValue("capacitySpin", params->capacitySpin->value());
        settings.setValue("noiseSpin", params->noiseSpin->value());
}

bool DynamicGPR::LoadOptions(QSettings &settings)
{
	if(settings.contains("kernelDeg")) params->kernelDegSpin->setValue(settings.value("kernelDeg").toFloat());
	if(settings.contains("kernelType")) params->kernelTypeCombo->setCurrentIndex(settings.value("kernelType").toInt());
	if(settings.contains("kernelWidth")) params->kernelWidthSpin->setValue(settings.value("kernelWidth").toFloat());
        if(settings.contains("capacitySpin")) params->capacitySpin->setValue(settings.value("capacitySpin").toInt());
        if(settings.contains("noiseSpin")) params->noiseSpin->setValue(settings.value("noiseSpin").toFloat());
        ChangeOptions();
	return true;
}

void DynamicGPR::SaveParams(QTextStream &file)
{
	file << "dynamicalOptions" << ":" << "kernelDeg" << " " << params->kernelDegSpin->value() << "\n";
	file << "dynamicalOptions" << ":" << "kernelType" << " " << params->kernelTypeCombo->currentIndex() << "\n";
	file << "dynamicalOptions" << ":" << "kernelWidth" << " " << params->kernelWidthSpin->value() << "\n";
        file << "dynamicalOptions" << ":" << "capacitySpin" << " " << params->capacitySpin->value() << "\n";
        file << "dynamicalOptions" << ":" << "noiseSpin" << " " << params->noiseSpin->value() << "\n";
}

bool DynamicGPR::LoadParams(QString name, float value)
{
	if(name.endsWith("kernelDeg")) params->kernelDegSpin->setValue((int)value);
	if(name.endsWith("kernelType")) params->kernelTypeCombo->setCurrentIndex((int)value);
	if(name.endsWith("kernelWidth")) params->kernelWidthSpin->setValue(value);
        if(name.endsWith("capacitySpin")) params->capacitySpin->setValue(value);
        if(name.endsWith("noiseSpin")) params->noiseSpin->setValue(value);
        ChangeOptions();
	return true;
}
