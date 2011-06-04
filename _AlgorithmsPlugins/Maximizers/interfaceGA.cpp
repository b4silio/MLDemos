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
#include "interfaceGA.h"
#include "maximizeGA.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>
#include <datasetManager.h>
#include <canvas.h>

using namespace std;

MaximizeInterfaceGA::MaximizeInterfaceGA()
{
	params = new Ui::ParametersGA();
	params->setupUi(widget = new QWidget());
}

void MaximizeInterfaceGA::SetParams(Maximizer *maximizer)
{
	if(!maximizer) return;
	double mutation = params->mutationSpin->value();
	double cross = params->crossSpin->value();
	double survival = params->survivalSpin->value();
	int population = params->populationSpin->value();
	((MaximizeGA *)maximizer)->SetParams(mutation, cross, survival, population);
}

Maximizer *MaximizeInterfaceGA::GetMaximizer()
{
	Maximizer *maximizer = new MaximizeGA();
	SetParams(maximizer);
	return maximizer;
}

void MaximizeInterfaceGA::DrawInfo(Canvas *canvas, Maximizer *maximizer)
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

void MaximizeInterfaceGA::Draw(Canvas *canvas, Maximizer *maximizer)
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

void MaximizeInterfaceGA::SaveOptions(QSettings &settings)
{
	settings.setValue("mutationSpin", params->mutationSpin->value());
	settings.setValue("crossSpin", params->crossSpin->value());
	settings.setValue("survivalSpin", params->survivalSpin->value());
}

bool MaximizeInterfaceGA::LoadOptions(QSettings &settings)
{
	if(settings.contains("mutationSpin")) params->mutationSpin->setValue(settings.value("mutationSpin").toFloat());
	if(settings.contains("crossSpin")) params->crossSpin->setValue(settings.value("crossSpin").toFloat());
	if(settings.contains("survivalSpin")) params->survivalSpin->setValue(settings.value("survivalSpin").toFloat());
	return true;
}

void MaximizeInterfaceGA::SaveParams(std::ofstream &file)
{
	file << "maximizationOptions" << ":" << "mutationSpin" << " " << params->mutationSpin->value() << std::endl;
	file << "maximizationOptions" << ":" << "crossSpin" << " " << params->crossSpin->value() << std::endl;
	file << "maximizationOptions" << ":" << "survivalSpin" << " " << params->survivalSpin->value() << std::endl;
}

bool MaximizeInterfaceGA::LoadParams(char *line, float value)
{
	if(endsWith(line,"mutationSpin")) params->mutationSpin->setValue((float)value);
	if(endsWith(line,"crossSpin")) params->crossSpin->setValue((float)value);
	if(endsWith(line,"survivalSpin")) params->survivalSpin->setValue((int)value);
	return true;
}
