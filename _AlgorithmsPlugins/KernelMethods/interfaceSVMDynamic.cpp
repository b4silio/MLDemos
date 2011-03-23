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
#include "interfaceSVMDynamic.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>

using namespace std;

DynamicSVM::DynamicSVM()
{
	params = new Ui::ParametersDynamic();
	params->setupUi(widget = new QWidget());
	connect(params->svmTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeOptions()));
}

void DynamicSVM::ChangeOptions()
{
	params->svmCLabel->setText("C");
	params->svmPSpin->setRange(0.0001, 1.0);
	params->svmPSpin->setSingleStep(0.01);
	params->svmPSpin->setDecimals(4);
	params->svmCSpin->setEnabled(true);
	params->svmCSpin->setRange(0.1, 9999.9);
	params->svmCSpin->setDecimals(1);
	switch(params->svmTypeCombo->currentIndex())
	{
	case 0: // C-SVM
		params->svmEpsLabel->setText("eps");
		break;
	case 1: // Nu-SVM
		params->svmEpsLabel->setText("Nu");
		break;
	case 2: // SOGP
		params->svmEpsLabel->setText("Noise");
		params->svmCLabel->setText("Capacity");
		params->svmCSpin->setRange(-1, 500);
		params->svmCSpin->setDecimals(0);
		params->svmPSpin->setRange(0.001, 1.0);
		params->svmPSpin->setSingleStep(0.01);
		params->svmPSpin->setDecimals(3);
		break;
	}
}

void DynamicSVM::SetParams(Dynamical *dynamical)
{
	if(!dynamical) return;
	int kernelMethod = params->svmTypeCombo->currentIndex();
	float svmC = params->svmCSpin->value();
	int kernelType = params->kernelTypeCombo->currentIndex();
	float kernelGamma = params->kernelWidthSpin->value();
	float kernelDegree = params->kernelDegSpin->value();
	float svmP = params->svmPSpin->value();

	if(kernelMethod == 2) // sogp
	{
		DynamicalGPR *gpr = (DynamicalGPR*)dynamical;
		int capacity = svmC;
		double kernelNoise = svmP;
		gpr->SetParams(kernelGamma, kernelNoise, capacity, kernelType, kernelDegree);
	}
	else
	{
		DynamicalSVR *svm = (DynamicalSVR*)dynamical;
		switch(kernelMethod)
		{
		case 0:
			svm->param.svm_type = EPSILON_SVR;
			break;
		case 1:
			svm->param.svm_type = NU_SVR;
			break;
		}
		switch(kernelType)
		{
		case 0:
			svm->param.kernel_type = LINEAR;
			break;
		case 1:
			svm->param.kernel_type = POLY;
			break;
		case 2:
			svm->param.kernel_type = RBF;
			break;
		}
		svm->param.C = svmC;
		svm->param.nu = svmP;
		svm->param.p = svmP;
		svm->param.gamma = 1 / kernelGamma;
		svm->param.degree = kernelDegree;
	}
}

Dynamical *DynamicSVM::GetDynamical()
{
	int svmType = params->svmTypeCombo->currentIndex();
	Dynamical *dynamical = 0;
	switch(svmType)
	{
	case 2:
		dynamical = new DynamicalGPR();
		break;
	default:
		dynamical = new DynamicalSVR();
		break;
	}
	SetParams(dynamical);
	return dynamical;
}

void DynamicSVM::DrawInfo(Canvas *canvas, Dynamical *dynamical)
{
	if(!canvas || !dynamical) return;
	int w = canvas->width();
	int h = canvas->height();
	QPixmap infoPixmap(w, h);
	QBitmap bitmap(w,h);
	bitmap.clear();
	infoPixmap.setMask(bitmap);
	infoPixmap.fill(Qt::transparent);

	QPainter painter(&infoPixmap);
	painter.setRenderHint(QPainter::Antialiasing);

	canvas->infoPixmap = infoPixmap;
}

void DynamicSVM::Draw(Canvas *canvas, Dynamical *dynamical)
{
	if(!dynamical || !canvas) return;
	if(!dynamical) return;
	DrawInfo(canvas, dynamical);
	int w = canvas->width();
	int h = canvas->height();
	canvas->modelPixmap = QPixmap(w,h);
	canvas->confidencePixmap = QPixmap(w,h);
	QBitmap bitmap(w,h);
	bitmap.clear();
	canvas->modelPixmap.setMask(bitmap);
	canvas->modelPixmap.fill(Qt::transparent);
	canvas->repaint();
}

void DynamicSVM::SaveOptions(QSettings &settings)
{
	settings.setValue("kernelDeg", params->kernelDegSpin->value());
	settings.setValue("kernelType", params->kernelTypeCombo->currentIndex());
	settings.setValue("kernelWidth", params->kernelWidthSpin->value());
	settings.setValue("svmC", params->svmCSpin->value());
	settings.setValue("svmP", params->svmPSpin->value());
	settings.setValue("svmType", params->svmTypeCombo->currentIndex());
}

bool DynamicSVM::LoadOptions(QSettings &settings)
{
	if(settings.contains("kernelDeg")) params->kernelDegSpin->setValue(settings.value("kernelDeg").toFloat());
	if(settings.contains("kernelType")) params->kernelTypeCombo->setCurrentIndex(settings.value("kernelType").toInt());
	if(settings.contains("kernelWidth")) params->kernelWidthSpin->setValue(settings.value("kernelWidth").toFloat());
	if(settings.contains("svmC")) params->svmCSpin->setValue(settings.value("svmC").toFloat());
	if(settings.contains("svmP")) params->svmPSpin->setValue(settings.value("svmP").toFloat());
	if(settings.contains("svmType")) params->svmTypeCombo->setCurrentIndex(settings.value("svmType").toInt());
	return true;
}

void DynamicSVM::SaveParams(std::ofstream &file)
{
	file << "dynamicalOptions" << ":" << "kernelDeg" << " " << params->kernelDegSpin->value() << std::endl;
	file << "dynamicalOptions" << ":" << "kernelType" << " " << params->kernelTypeCombo->currentIndex() << std::endl;
	file << "dynamicalOptions" << ":" << "kernelWidth" << " " << params->kernelWidthSpin->value() << std::endl;
	file << "dynamicalOptions" << ":" << "svmC" << " " << params->svmCSpin->value() << std::endl;
	file << "dynamicalOptions" << ":" << "svmP" << " " << params->svmPSpin->value() << std::endl;
	file << "dynamicalOptions" << ":" << "svmType" << " " << params->svmTypeCombo->currentIndex() << std::endl;
}

bool DynamicSVM::LoadParams(char *line, float value)
{
	if(endsWith(line,"kernelDeg")) params->kernelDegSpin->setValue((int)value);
	if(endsWith(line,"kernelType")) params->kernelTypeCombo->setCurrentIndex((int)value);
	if(endsWith(line,"kernelWidth")) params->kernelWidthSpin->setValue(value);
	if(endsWith(line,"svmC")) params->svmCSpin->setValue(value);
	if(endsWith(line,"svmP")) params->svmPSpin->setValue(value);
	if(endsWith(line,"svmType")) params->svmTypeCombo->setCurrentIndex((int)value);
	return true;
}
