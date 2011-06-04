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

using namespace std;

MaximizeBasic::MaximizeBasic()
{
	params = new Ui::ParametersMaximizers();
	params->setupUi(widget = new QWidget());
	connect(params->maximizeType, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeOptions()));
}

void MaximizeBasic::ChangeOptions()
{
	params->varianceSpin->setEnabled(false);
	params->kSpin->setEnabled(false);
	params->adaptiveCheck->setEnabled(false);
	switch(params->maximizeType->currentIndex())
	{
	case 0: // Random Search
		break;
	case 1: // Random Walk
		params->varianceSpin->setEnabled(true);
		break;
	case 2: // PoWER
		params->varianceSpin->setEnabled(true);
		params->kSpin->setEnabled(true);
		params->adaptiveCheck->setEnabled(true);
		break;
	}
}

void MaximizeBasic::SetParams(Maximizer *maximizer)
{
	if(!maximizer) return;
	int type = params->maximizeType->currentIndex();
//	int iterations = params->iterationsSpin->value();
	double variance = params->varianceSpin->value();
	int k = params->kSpin->value();
	bool bAdaptive = params->adaptiveCheck->isChecked();
	switch(type)
	{
	case 0: // random search
		((MaximizeRandom *)maximizer)->SetParams();
		break;
	case 1: // random walk
		((MaximizeRandom *)maximizer)->SetParams(variance*variance);
		break;
	case 2: // power
		((MaximizePower *)maximizer)->SetParams(k, variance*variance, bAdaptive);
		break;
	}
}

Maximizer *MaximizeBasic::GetMaximizer()
{
	Maximizer *maximizer = NULL;
	switch(params->maximizeType->currentIndex())
	{
	case 0:
		maximizer = new MaximizeRandom();
		break;
	case 1:
		maximizer = new MaximizeRandom();
		break;
	case 2:
		maximizer = new MaximizePower();
		break;
	}
	SetParams(maximizer);
	return maximizer;
}

void MaximizeBasic::DrawInfo(Canvas *canvas, Maximizer *maximizer)
{
	if(!canvas || !maximizer) return;
	int w = canvas->width();
	int h = canvas->height();
	QPixmap infoPixmap(w, h);
	QBitmap bitmap(w,h);
	bitmap.clear();
	infoPixmap.setMask(bitmap);
	infoPixmap.fill(Qt::transparent);
	QPainter painter(&canvas->infoPixmap);
	painter.setRenderHint(QPainter::Antialiasing);

	// draw the current maximum
	//painter.drawEllipse(QPointF());
	canvas->infoPixmap = infoPixmap;
}

void MaximizeBasic::Draw(Canvas *canvas, Maximizer *maximizer)
{
	if(!maximizer || !canvas) return;
	canvas->liveTrajectory.clear();
	DrawInfo(canvas, maximizer);
	int w = canvas->width();
	int h = canvas->height();

	canvas->confidencePixmap = QPixmap(w,h);
	canvas->modelPixmap = QPixmap(w,h);
	QBitmap bitmap(w,h);
	bitmap.clear();
	canvas->modelPixmap.setMask(bitmap);
	canvas->modelPixmap.fill(Qt::transparent);
	QPainter painter(&canvas->modelPixmap);
	painter.setRenderHint(QPainter::Antialiasing, true);

	fvec sample;
	sample.resize(2,0);

	canvas->confidencePixmap.fill();

	int steps = w;
	QPointF oldPoint(-FLT_MAX,-FLT_MAX);
	QPointF oldPointUp(-FLT_MAX,-FLT_MAX);
	QPointF oldPointDown(-FLT_MAX,-FLT_MAX);
	FOR(x, steps)
	{
		sample = canvas->toSampleCoords(x,0);
		fvec res = maximizer->Test(sample);
		if(res[0] != res[0] || res[1] != res[1]) continue;
		QPointF point = canvas->toCanvasCoords(sample[0], res[0]);
		QPointF pointUp = canvas->toCanvasCoords(sample[0],res[0] + res[1]);
		pointUp.setX(0);
		pointUp.setY(pointUp.y() - point.y());
		QPointF pointDown = -pointUp;
		if(x)
		{
			painter.setPen(QPen(Qt::black, 1));
			painter.drawLine(point, oldPoint);
			painter.setPen(QPen(Qt::black, 0.5));
			painter.drawLine(pointUp, oldPointUp);
			painter.drawLine(pointDown, oldPointDown);
		}
		oldPoint = point;
		oldPointUp = pointUp;
		oldPointDown = pointDown;
	}
	canvas->repaint();
}

void MaximizeBasic::SaveOptions(QSettings &settings)
{
	settings.setValue("maximizeType", params->maximizeType->currentIndex());
//	settings.setValue("iterationsSpin", params->iterationsSpin->value());
	settings.setValue("varianceSpin", params->varianceSpin->value());
	settings.setValue("adaptiveCheck", params->adaptiveCheck->isChecked());
	settings.setValue("kSpin", params->kSpin->value());
}

bool MaximizeBasic::LoadOptions(QSettings &settings)
{
	if(settings.contains("maximizeType")) params->maximizeType->setCurrentIndex(settings.value("maximizeType").toInt());
//	if(settings.contains("iterationsSpin")) params->iterationsSpin->setValue(settings.value("iterationsSpin").toInt());
	if(settings.contains("varianceSpin")) params->varianceSpin->setValue(settings.value("varianceSpin").toFloat());
	if(settings.contains("adaptiveCheck")) params->adaptiveCheck->setChecked(settings.value("adaptiveCheck").toBool());
	if(settings.contains("kSpin")) params->kSpin->setValue(settings.value("kSpin").toInt());
	return true;
}

void MaximizeBasic::SaveParams(std::ofstream &file)
{
	file << "maximizationOptions" << ":" << "maximizeType" << " " << params->maximizeType->currentIndex() << std::endl;
//	file << "maximizationOptions" << ":" << "iterationsSpin" << " " << params->iterationsSpin->value() << std::endl;
	file << "maximizationOptions" << ":" << "varianceSpin" << " " << params->varianceSpin->value() << std::endl;
	file << "maximizationOptions" << ":" << "adaptiveCheck" << " " << params->adaptiveCheck->isChecked() << std::endl;
	file << "maximizationOptions" << ":" << "kSpin" << " " << params->kSpin->value() << std::endl;
}

bool MaximizeBasic::LoadParams(char *line, float value)
{
	if(endsWith(line,"maximizeType")) params->maximizeType->setCurrentIndex((int)value);
//	if(endsWith(line,"iterationsSpin")) params->iterationsSpin->setValue((int)value);
	if(endsWith(line,"varianceSpin")) params->varianceSpin->setValue((float)value);
	if(endsWith(line,"adaptiveCheck")) params->adaptiveCheck->setChecked((bool)value);
	if(endsWith(line,"kSpin")) params->kSpin->setValue((int)value);
	return true;
}
