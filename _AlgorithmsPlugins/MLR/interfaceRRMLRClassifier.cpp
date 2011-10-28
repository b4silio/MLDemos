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
#include "interfaceRRMLRClassifier.h"
#include "classifierRRMLR.h"
#include "ui_paramsRRMLR.h"
#include <cassert>

using namespace std;

ClassRRMLR::ClassRRMLR()
{
	params = new Ui::ParametersRRMLR();
	params->setupUi(widget = new QWidget());
}

void ClassRRMLR::SetParams(Classifier *classifier)
{
	if(!classifier) return;
	
	const u32 cutCount = params->rrmlrCutSpin->value();
	const float alpha = params->rrmlrAlphaSpin->value();
	const u32 restartCount = params->rrmlrRestartSpin->value();
	const u32 maxIter = params->rrmlrMaxIterSpin->value();

	ClassifierRRMLR* c(dynamic_cast<ClassifierRRMLR*>(classifier));
	assert(c);
	c->SetParams(cutCount, alpha, restartCount, maxIter);
}

QString ClassRRMLR::GetAlgoString()
{
	const u32 cutCount = params->rrmlrCutSpin->value();
	const float alpha = params->rrmlrAlphaSpin->value();
	const u32 restartCount = params->rrmlrRestartSpin->value();
	const u32 maxIter = params->rrmlrMaxIterSpin->value();
	
	return QString("RRMLR %1 %2 %3 %4").arg(cutCount).arg(alpha).arg(restartCount).arg(maxIter);
}

Classifier *ClassRRMLR::GetClassifier()
{
	ClassifierRRMLR *classifier = new ClassifierRRMLR();
	SetParams(classifier);
	return classifier;
}

void ClassRRMLR::DrawModel(Canvas *canvas, QPainter &painter, Classifier *classifier)
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

void ClassRRMLR::SaveOptions(QSettings &settings)
{
	settings.setValue("rrmlrCut", params->rrmlrCutSpin->value());
	settings.setValue("rrmlrAlpha", params->rrmlrAlphaSpin->value());
	settings.setValue("rrmlrRestart", params->rrmlrRestartSpin->value());
	settings.setValue("rrmlrMaxIter", params->rrmlrMaxIterSpin->value());
}

bool ClassRRMLR::LoadOptions(QSettings &settings)
{
	if(settings.contains("rrmlrCut")) params->rrmlrCutSpin->setValue(settings.value("rrmlrCut").toUInt());
	if(settings.contains("rrmlrAlpha")) params->rrmlrAlphaSpin->setValue(settings.value("rrmlrAlpha").toFloat());
	if(settings.contains("rrmlrRestart")) params->rrmlrRestartSpin->setValue(settings.value("rrmlrRestart").toUInt());
	if(settings.contains("rrmlrMaxIter")) params->rrmlrRestartSpin->setValue(settings.value("rrmlrMaxIter").toUInt());
	return true;
}

void ClassRRMLR::SaveParams(QTextStream &file)
{
	file << "classificationOptions" << ":" << "rrmlrCut" << " " << params->rrmlrCutSpin->value() << "\n";
	file << "classificationOptions" << ":" << "rrmlrAlpha" << " " << params->rrmlrAlphaSpin->value() << "\n";
	file << "classificationOptions" << ":" << "rrmlrRestart" << " " << params->rrmlrRestartSpin->value() << "\n";
	file << "classificationOptions" << ":" << "rrmlrMaxIter" << " " << params->rrmlrMaxIterSpin->value() << "\n";
}

bool ClassRRMLR::LoadParams(QString name, float value)
{
	if(name.endsWith("rrmlrCut")) params->rrmlrCutSpin->setValue((int)value);
	if(name.endsWith("rrmlrAlpha")) params->rrmlrAlphaSpin->setValue(value);
	if(name.endsWith("rrmlrRestart")) params->rrmlrRestartSpin->setValue((int)value);
	if(name.endsWith("rrmlrMaxIter")) params->rrmlrMaxIterSpin->setValue((int)value);
	return true;
}