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
#include "interfaceBoostClassifier.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>

using namespace std;

ClassBoost::ClassBoost()
{
	params = new Ui::ParametersBoost();
	params->setupUi(widget = new QWidget());
}

void ClassBoost::SetParams(Classifier *classifier)
{
	if(!classifier) return;
	int weakCount = params->boostCountSpin->value();
	int weakType = params->boostLearnerType->currentIndex();
	((ClassifierBoost *)classifier)->SetParams(weakCount, weakType);
}

QString ClassBoost::GetAlgoString()
{
	int weakCount = params->boostCountSpin->value();
	int weakType = params->boostLearnerType->currentIndex();
	QString algo = QString("Boost %1").arg(weakCount);
	switch(weakType)
	{
	case 0:
		algo += " Proj";
		break;
	case 1:
		algo += " Rect";
		break;
	case 2:
		algo += " Circ";
		break;
	}
	return algo;
}

Classifier *ClassBoost::GetClassifier()
{
	ClassifierBoost *classifier = new ClassifierBoost();
	SetParams(classifier);
	return classifier;
}

void ClassBoost::DrawModel(Canvas *canvas, QPainter &painter, Classifier *classifier)
{
	if(!classifier || !canvas) return;
	painter.setRenderHint(QPainter::Antialiasing, true);

	int posClass = 1;
	bool bUseMinMax = false;
    bUseMinMax = true;

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

void ClassBoost::SaveOptions(QSettings &settings)
{
	settings.setValue("boostCount", params->boostCountSpin->value());
	settings.setValue("boostType", params->boostLearnerType->currentIndex());
}

bool ClassBoost::LoadOptions(QSettings &settings)
{
	if(settings.contains("boostCount")) params->boostCountSpin->setValue(settings.value("boostCount").toFloat());
	if(settings.contains("boostType")) params->boostLearnerType->setCurrentIndex(settings.value("boostType").toInt());
	return true;
}

void ClassBoost::SaveParams(QTextStream &file)
{
	file << "classificationOptions" << ":" << "boostCount" << " " << params->boostCountSpin->value() << "\n";
	file << "classificationOptions" << ":" << "boostType" << " " << params->boostLearnerType->currentIndex() << "\n";
}

bool ClassBoost::LoadParams(QString name, float value)
{
	if(name.endsWith("boostCount")) params->boostCountSpin->setValue((int)value);
	if(name.endsWith("boostType")) params->boostLearnerType->setCurrentIndex((int)value);
	return true;
}
