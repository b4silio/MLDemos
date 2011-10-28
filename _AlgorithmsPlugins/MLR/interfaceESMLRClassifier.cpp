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
	const u32 genCount = params->esmlrGenSpin->value();
	const u32 indPerDim = params->esmlrIndPerDimSpin->value();

	ClassifierESMLR* c(dynamic_cast<ClassifierESMLR*>(classifier));
	assert(c);
	c->SetParams(cutCount, alpha, genCount, indPerDim);
}

QString ClassESMLR::GetAlgoString()
{
	const u32 cutCount = params->esmlrCutSpin->value();
	const float alpha = params->esmlrAlphaSpin->value();
	const u32 genCount = params->esmlrGenSpin->value();
	const u32 indPerDim = params->esmlrIndPerDimSpin->value();
	
	return QString("ESMLR %1 %2 %3 %4").arg(cutCount).arg(alpha).arg(genCount).arg(indPerDim);
}

Classifier *ClassESMLR::GetClassifier()
{
	ClassifierESMLR *classifier = new ClassifierESMLR();
	SetParams(classifier);
	return classifier;
}

void ClassESMLR::DrawModel(Canvas *canvas, QPainter &painter, Classifier *classifier)
{
	if(!classifier || !canvas)
		return;
	painter.setRenderHint(QPainter::Antialiasing, true);
	int posClass = 1;
	FOR(i, canvas->data->GetCount())
	{
		const fvec sample = canvas->data->GetSample(i);
		const int label = canvas->data->GetLabel(i);
		const QPointF point = canvas->toCanvasCoords(canvas->data->GetSample(i));
		const float response = classifier->Test(sample);
		if(response > 0)
		{
			if(label == posClass)
				Canvas::drawSample(painter, point, 9, 1);
			else
				Canvas::drawCross(painter, point, 6, 2);
		}
		else
		{
			if(label != posClass)
				Canvas::drawSample(painter, point, 9, 0);
			else
				Canvas::drawCross(painter, point, 6, 0);
		}
	}
}

void ClassESMLR::SaveOptions(QSettings &settings)
{
	settings.setValue("esmlrCut", params->esmlrCutSpin->value());
	settings.setValue("esmlrAlpha", params->esmlrAlphaSpin->value());
	settings.setValue("esmlrGen", params->esmlrGenSpin->value());
	settings.setValue("esmlrIndPerDim", params->esmlrIndPerDimSpin->value());
}

bool ClassESMLR::LoadOptions(QSettings &settings)
{
	if(settings.contains("esmlrCut")) params->esmlrCutSpin->setValue(settings.value("esmlrCut").toUInt());
	if(settings.contains("esmlrAlpha")) params->esmlrAlphaSpin->setValue(settings.value("esmlrAlpha").toFloat());
	if(settings.contains("esmlrGen")) params->esmlrGenSpin->setValue(settings.value("esmlrGen").toUInt());
	if(settings.contains("esmlrIndPerDim")) params->esmlrGenSpin->setValue(settings.value("esmlrIndPerDim").toUInt());
	return true;
}

void ClassESMLR::SaveParams(QTextStream &file)
{
	file << "classificationOptions" << ":" << "esmlrCut" << " " << params->esmlrCutSpin->value() << "\n";
	file << "classificationOptions" << ":" << "esmlrAlpha" << " " << params->esmlrAlphaSpin->value() << "\n";
	file << "classificationOptions" << ":" << "esmlrGen" << " " << params->esmlrGenSpin->value() << "\n";
	file << "classificationOptions" << ":" << "esmlrIndPerDim" << " " << params->esmlrIndPerDimSpin->value() << "\n";
}

bool ClassESMLR::LoadParams(QString name, float value)
{
	if(name.endsWith("esmlrCut")) params->esmlrCutSpin->setValue((int)value);
	if(name.endsWith("esmlrAlpha")) params->esmlrAlphaSpin->setValue(value);
	if(name.endsWith("esmlrGen")) params->esmlrGenSpin->setValue((int)value);
	if(name.endsWith("esmlrIndPerDim")) params->esmlrIndPerDimSpin->setValue((int)value);
	return true;
}