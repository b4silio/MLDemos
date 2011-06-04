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
#include "interfaceParticles.h"
#include "maximizeParticles.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>
#include <datasetManager.h>
#include <canvas.h>

using namespace std;

MaximizeInterfaceParticles::MaximizeInterfaceParticles()
{
	params = new Ui::ParametersParticles();
	params->setupUi(widget = new QWidget());
}

void MaximizeInterfaceParticles::SetParams(Maximizer *maximizer)
{
	if(!maximizer) return;
	int particleCount = params->particleSpin->value();
	double variance = params->varianceSpin->value();
	bool adaptive = params->adaptiveCheck->isChecked();
	int samplingType = params->samplingType->currentIndex();
	((MaximizeParticles *)maximizer)->SetParams(particleCount, variance, adaptive, samplingType);
}

Maximizer *MaximizeInterfaceParticles::GetMaximizer()
{
	Maximizer *maximizer = new MaximizeParticles();
	SetParams(maximizer);
	return maximizer;
}

void MaximizeInterfaceParticles::DrawInfo(Canvas *canvas, Maximizer *maximizer)
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

void MaximizeInterfaceParticles::Draw(Canvas *canvas, Maximizer *maximizer)
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

void MaximizeInterfaceParticles::SaveOptions(QSettings &settings)
{
	settings.setValue("samplingType", params->samplingType->currentIndex());
	settings.setValue("varianceSpin", params->varianceSpin->value());
	settings.setValue("adaptiveCheck", params->adaptiveCheck->isChecked());
	settings.setValue("particleSpin", params->particleSpin->value());
}

bool MaximizeInterfaceParticles::LoadOptions(QSettings &settings)
{
	if(settings.contains("samplingType")) params->samplingType->setCurrentIndex(settings.value("samplingType").toInt());
	if(settings.contains("varianceSpin")) params->varianceSpin->setValue(settings.value("varianceSpin").toFloat());
	if(settings.contains("adaptiveCheck")) params->adaptiveCheck->setChecked(settings.value("adaptiveCheck").toBool());
	if(settings.contains("particleSpin")) params->particleSpin->setValue(settings.value("particleSpin").toInt());
	return true;
}

void MaximizeInterfaceParticles::SaveParams(std::ofstream &file)
{
	file << "maximizationOptions" << ":" << "samplingType" << " " << params->samplingType->currentIndex() << std::endl;
	file << "maximizationOptions" << ":" << "varianceSpin" << " " << params->varianceSpin->value() << std::endl;
	file << "maximizationOptions" << ":" << "adaptiveCheck" << " " << params->adaptiveCheck->isChecked() << std::endl;
	file << "maximizationOptions" << ":" << "particleSpin" << " " << params->particleSpin->value() << std::endl;
}

bool MaximizeInterfaceParticles::LoadParams(char *line, float value)
{
	if(endsWith(line,"samplingType")) params->samplingType->setCurrentIndex((int)value);
	if(endsWith(line,"varianceSpin")) params->varianceSpin->setValue((float)value);
	if(endsWith(line,"adaptiveCheck")) params->adaptiveCheck->setChecked((bool)value);
	if(endsWith(line,"particleSpin")) params->particleSpin->setValue((int)value);
	return true;
}
