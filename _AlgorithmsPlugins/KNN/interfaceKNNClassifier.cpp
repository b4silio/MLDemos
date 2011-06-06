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
#include "interfaceKNNClassifier.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>

using namespace std;

ClassKNN::ClassKNN()
{
	params = new Ui::ParametersKNN();
	params->setupUi(widget = new QWidget());
}

void ClassKNN::SetParams(Classifier *classifier)
{
	if(!classifier) return;
	int k = params->knnKspin->value();
	int metricType = params->knnNormCombo->currentIndex();
	int metricP = params->knnNormSpin->value();

	((ClassifierKNN *)classifier)->SetParams(k, metricType, metricP);
}

Classifier *ClassKNN::GetClassifier()
{
	ClassifierKNN *classifier = new ClassifierKNN();
	SetParams(classifier);
	return classifier;
}

void ClassKNN::DrawModel(Canvas *canvas, QPainter &painter, Classifier *classifier)
{
	if(!classifier || !canvas) return;
	int w = canvas->width();
	int h = canvas->height();

	int posClass = 1;
	painter.setRenderHint(QPainter::Antialiasing, true);
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

void ClassKNN::SaveOptions(QSettings &settings)
{
	settings.setValue("knnK", params->knnKspin->value());
	settings.setValue("knnNorm", params->knnNormCombo->currentIndex());
	settings.setValue("knnPower", params->knnNormSpin->value());
}

bool ClassKNN::LoadOptions(QSettings &settings)
{
	if(settings.contains("knnK")) params->knnKspin->setValue(settings.value("knnK").toFloat());
	if(settings.contains("knnNorm")) params->knnNormCombo->setCurrentIndex(settings.value("knnNorm").toInt());
	if(settings.contains("knnPower")) params->knnNormSpin->setValue(settings.value("knnPower").toFloat());
	return true;
}

void ClassKNN::SaveParams(std::ofstream &file)
{
	file << "classificationOptions" << ":" << "knnK" << " " << params->knnKspin->value() << std::endl;
	file << "classificationOptions" << ":" << "knnNorm" << " " << params->knnNormCombo->currentIndex() << std::endl;
	file << "classificationOptions" << ":" << "knnPower" << " " << params->knnNormSpin->value() << std::endl;
}

bool ClassKNN::LoadParams(char *line, float value)
{
	if(endsWith(line,"knnK")) params->knnKspin->setValue((int)value);
	if(endsWith(line,"knnNorm")) params->knnNormCombo->setCurrentIndex((int)value);
	if(endsWith(line,"knnPower")) params->knnNormSpin->setValue((int)value);
	return true;
}
