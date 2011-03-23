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

Classifier *ClassBoost::GetClassifier()
{
	ClassifierBoost *classifier = new ClassifierBoost();
	SetParams(classifier);
	return classifier;
}

void ClassBoost::DrawInfo(Canvas *canvas, Classifier *classifier)
{
	if(!canvas || !classifier) return;
	int w = canvas->width();
	int h = canvas->height();
	QPixmap infoPixmap(w, h);
	QBitmap bitmap(w,h);
	bitmap.clear();
	infoPixmap.setMask(bitmap);
	infoPixmap.fill(Qt::transparent);
	canvas->infoPixmap = infoPixmap;
}

void ClassBoost::Draw(Canvas *canvas, Classifier *classifier)
{
	if(!classifier || !canvas) return;
	int w = canvas->width();
	int h = canvas->height();

	int posClass = 1;

	canvas->modelPixmap = QPixmap();

	DrawInfo(canvas, classifier);

	bool bUseMinMax = false;
	if(classifier->type == CLASS_BOOST ||
		classifier->type == CLASS_LINEAR ||
		classifier->type == CLASS_MLP) bUseMinMax = true;

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

	// we draw the samples
	canvas->modelPixmap = QPixmap(w,h);
	QBitmap bitmap(w,h);
	bitmap.clear();
	canvas->modelPixmap.setMask(bitmap);
	canvas->modelPixmap.fill(Qt::transparent);
	QPainter painter(&canvas->modelPixmap);
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

	canvas->liveTrajectory.clear();
	canvas->repaint();
	canvas->confidencePixmap = QPixmap();
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

void ClassBoost::SaveParams(std::ofstream &file)
{
	file << "classificationOptions" << ":" << "boostCount" << " " << params->boostCountSpin->value() << std::endl;
	file << "classificationOptions" << ":" << "boostType" << " " << params->boostLearnerType->currentIndex() << std::endl;
}

bool ClassBoost::LoadParams(char *line, float value)
{
	if(endsWith(line,"boostCount")) params->boostCountSpin->setValue((int)value);
	if(endsWith(line,"boostType")) params->boostLearnerType->setCurrentIndex((int)value);
	return true;
}
