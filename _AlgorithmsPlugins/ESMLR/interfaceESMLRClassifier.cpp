/*********************************************************************
MLDemos: A User-Friendly visualization toolkit for machine learning
Copyright (C) 2010  Basilio Noris
Contact: mldemos@b4silio.com

Evolution-Strategy Mixture of Logisitics Regression
Copyright (C) 2011  Stephane Magnenat
Contact: stephane at magnenat dot net

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
#include "interfaceESMLRClassifier.h"
#include "classifierESMLR.h"
#include "ui_paramsESMLR.h"
#include <cassert>

using namespace std;

ClassESMLR::ClassESMLR()
{
	params = new Ui::ParametersESMLR();
	params->setupUi(widget = new QWidget());
}

void ClassESMLR::SetParams(Classifier *classifier)
{
	if(!classifier) return;
	
	const u32 cutCount = params->esmlrCutSpin->value();
	const float alpha = params->esmlrAlphaSpin->value();

	ClassifierESMLR* c(dynamic_cast<ClassifierESMLR*>(classifier));
	assert(c);
	c->SetParams(cutCount, alpha);
}

QString ClassESMLR::GetAlgoString()
{
	const u32 cutCount = params->esmlrCutSpin->value();
	const float alpha = params->esmlrAlphaSpin->value();
	
	return QString("ESMLR %1 %2").arg(cutCount).arg(alpha);
}

Classifier *ClassESMLR::GetClassifier()
{
	ClassifierESMLR *classifier = new ClassifierESMLR();
	SetParams(classifier);
	return classifier;
}

void ClassESMLR::DrawModel(Canvas *canvas, QPainter &painter, Classifier *classifier)
{
	if(!classifier || !canvas) return;
	/*painter.setRenderHint(QPainter::Antialiasing, true);
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
	}*/
}

void ClassESMLR::SaveOptions(QSettings &settings)
{
	settings.setValue("esmlrCut", params->esmlrCutSpin->value());
	settings.setValue("esmlrAlpha", params->esmlrAlphaSpin->value());
}

bool ClassESMLR::LoadOptions(QSettings &settings)
{
	if(settings.contains("esmlrCut")) params->esmlrCutSpin->setValue(settings.value("esmlrCut").toFloat());
	if(settings.contains("esmlrAlpha")) params->esmlrAlphaSpin->setValue(settings.value("esmlrAlpha").toFloat());
	return true;
}

void ClassESMLR::SaveParams(QTextStream &file)
{
	file << "classificationOptions" << ":" << "esmlrCut" << " " << params->esmlrCutSpin->value() << "\n";
	file << "classificationOptions" << ":" << "esmlrAlpha" << " " << params->esmlrAlphaSpin->value() << "\n";
}

bool ClassESMLR::LoadParams(QString name, float value)
{
	if(name.endsWith("esmlrCut")) params->esmlrCutSpin->setValue((int)value);
	if(name.endsWith("esmlrAlpha")) params->esmlrAlphaSpin->setValue(value);
	return true;
}