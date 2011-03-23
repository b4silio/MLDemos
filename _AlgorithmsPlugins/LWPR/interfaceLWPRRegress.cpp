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
#include "interfaceLWPRRegress.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>

using namespace std;

RegrLWPR::RegrLWPR()
{
	params = new Ui::ParametersLWPRRegress();
	params->setupUi(widget = new QWidget());
}

void RegrLWPR::SetParams(Regressor *regressor)
{
	if(!regressor) return;
	float gen = params->lwprGenSpin->value();
	float delta = params->lwprInitialDSpin->value();
	float alpha = params->lwprAlphaSpin->value();

	((RegressorLWPR*)regressor)->SetParams(delta, alpha, gen);
}

Regressor *RegrLWPR::GetRegressor()
{
	RegressorLWPR *regressor = new RegressorLWPR();
	SetParams(regressor);
	return regressor;
}

void RegrLWPR::DrawInfo(Canvas *canvas, Regressor *regressor)
{
	if(!canvas || !regressor) return;
	int w = canvas->width();
	int h = canvas->height();
	QPixmap infoPixmap(w, h);
	QBitmap bitmap(w,h);
	bitmap.clear();
	infoPixmap.setMask(bitmap);
	infoPixmap.fill(Qt::transparent);

	QPainter painter(&infoPixmap);
	painter.setRenderHint(QPainter::Antialiasing);

	RegressorLWPR* _lwpr = (RegressorLWPR*)regressor;
	LWPR_Object *lwpr= _lwpr->GetModel();
	FOR(i, lwpr->numRFS()[0])
	{
		LWPR_ReceptiveFieldObject rf = lwpr->getRF(0,i);
		double var = sqrt(rf.varX()[0]);
		var = fabs((canvas->toCanvasCoords(0,0) - canvas->toCanvasCoords(var,0)).x());
		double centerX = rf.center()[0];
		double centerY = lwpr->predict(rf.center())[0];
		double radius = rf.D()[0][0];
		double slope = rf.slope()[0];
		QPointF point = canvas->toCanvasCoords(centerX, centerY);
		painter.setPen(QPen(Qt::black, 4));
		painter.drawEllipse(point, 2, 2);
		painter.setPen(QPen(Qt::white, 2));
		painter.drawEllipse(point, 2, 2);
		painter.setPen(QPen(Qt::black, 1));
		painter.drawEllipse(point, var, var);
		painter.drawLine(point - QPointF(30, slope*var*2),point + QPointF(30, slope*var*2));
	}

	canvas->infoPixmap = infoPixmap;
}

void RegrLWPR::Draw(Canvas *canvas, Regressor *regressor)
{
	if(!regressor || !canvas) return;
	canvas->liveTrajectory.clear();
	DrawInfo(canvas, regressor);
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
	canvas->confidencePixmap = QPixmap();
	int steps = w;
	QPointF oldPoint(-FLT_MAX,-FLT_MAX);
	QPointF oldPointUp(-FLT_MAX,-FLT_MAX);
	QPointF oldPointDown(-FLT_MAX,-FLT_MAX);
	FOR(x, steps)
	{
		sample = canvas->toSampleCoords(x,0);
		fvec res = regressor->Test(sample);
		if(res[0] != res[0]) continue;
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
			//painter.drawLine(pointUp, oldPointUp);
			//painter.drawLine(pointDown, oldPointDown);
		}
		oldPoint = point;
		oldPointUp = pointUp;
		oldPointDown = pointDown;
	}
	canvas->repaint();
}

void RegrLWPR::SaveOptions(QSettings &settings)
{
	settings.setValue("lwprAlpha", params->lwprAlphaSpin->value());
	settings.setValue("lwprInitialD", params->lwprInitialDSpin->value());
	settings.setValue("lwprGen", params->lwprGenSpin->value());
}

bool RegrLWPR::LoadOptions(QSettings &settings)
{
	if(settings.contains("lwprAlpha")) params->lwprAlphaSpin->setValue(settings.value("lwprAlpha").toFloat());
	if(settings.contains("lwprInitialD")) params->lwprInitialDSpin->setValue(settings.value("lwprInitialD").toFloat());
	if(settings.contains("lwprGen")) params->lwprGenSpin->setValue(settings.value("lwprGen").toFloat());
	return true;
}

void RegrLWPR::SaveParams(std::ofstream &file)
{
	file << "regressionOptions" << ":" << "lwprAlpha" << " " << params->lwprAlphaSpin->value() << std::endl;
	file << "regressionOptions" << ":" << "lwprInitialD" << " " << params->lwprInitialDSpin->value() << std::endl;
	file << "regressionOptions" << ":" << "lwprGen" << " " << params->lwprGenSpin->value() << std::endl;
}

bool RegrLWPR::LoadParams(char *line, float value)
{
	if(endsWith(line,"lwprAlpha")) params->lwprAlphaSpin->setValue(value);
	if(endsWith(line,"lwprInitialD")) params->lwprInitialDSpin->setValue(value);
	if(endsWith(line,"lwprGen")) params->lwprGenSpin->setValue(value);
	return true;
}
