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
#include <QDebug>
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

QString RegrLWPR::GetAlgoString()
{
	float gen = params->lwprGenSpin->value();
	float delta = params->lwprInitialDSpin->value();
	float alpha = params->lwprAlphaSpin->value();

	QString algo = QString("LWPR %1 %2 %3").arg(gen).arg(delta).arg(alpha);
	return algo;
}

Regressor *RegrLWPR::GetRegressor()
{
	RegressorLWPR *regressor = new RegressorLWPR();
	SetParams(regressor);
	return regressor;
}

void RegrLWPR::DrawInfo(Canvas *canvas, QPainter &painter, Regressor *regressor)
{
	if(!canvas || !regressor) return;
	int w = canvas->width();
	int h = canvas->height();
    int xIndex = canvas->xIndex;
    int yIndex = canvas->yIndex;
    int outputDim = regressor->outputDim;
	painter.setRenderHint(QPainter::Antialiasing);
    fvec sample = canvas->toSampleCoords(0,0);
    if(sample.size() > 2) return;

	RegressorLWPR* _lwpr = (RegressorLWPR*)regressor;
	LWPR_Object *lwpr= _lwpr->GetModel();
	painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(Qt::black, 1));
    FOR(i, lwpr->numRFS()[0])
	{
		LWPR_ReceptiveFieldObject rf = lwpr->getRF(0,i);
		double var = sqrt(rf.varX()[0]);
		var = fabs((canvas->toCanvasCoords(0,0) - canvas->toCanvasCoords(var,0)).x());
        double centerX = rf.center()[xIndex];
        double centerY = lwpr->predict(rf.center())[0];
		double radius = rf.D()[0][0];
		double slope = rf.slope()[0];
		QPointF point = canvas->toCanvasCoords(centerX, centerY);
        painter.drawEllipse(point, 4, 4);
		painter.drawEllipse(point, var, var);
		painter.drawLine(point - QPointF(30, slope*var*2),point + QPointF(30, slope*var*2));
	}
}

void RegrLWPR::DrawModel(Canvas *canvas, QPainter &painter, Regressor *regressor)
{
	if(!regressor || !canvas) return;
	int w = canvas->width();
	int h = canvas->height();
    int outputDim = regressor->outputDim;
    int xIndex = canvas->xIndex;

	painter.setRenderHint(QPainter::Antialiasing, true);
    fvec sample = canvas->toSampleCoords(0,0);
    int dim = sample.size();
    if(dim > 2) return;
	canvas->maps.confidence = QPixmap();
	int steps = w;
	painter.setBrush(Qt::NoBrush);
    QPainterPath path, pathUp, pathDown;
	FOR(x, steps)
	{
		sample = canvas->toSampleCoords(x,0);
		fvec res = regressor->Test(sample);
		if(res[0] != res[0]) continue;
        QPointF point = canvas->toCanvasCoords(sample[xIndex], res[0]);
        QPointF pointUp = canvas->toCanvasCoords(sample[xIndex],res[0] + res[1]);
		pointUp.setX(0);
		pointUp.setY(pointUp.y() - point.y());
		QPointF pointDown = -pointUp;
		if(x)
		{
            path.lineTo(point);
            pathUp.lineTo(point+pointUp);
            pathDown.lineTo(point+pointDown);
        }
        else
        {
            path.moveTo(point);
            pathUp.moveTo(point+pointUp);
            pathDown.moveTo(point+pointDown);
        }
	}
    painter.setPen(QPen(Qt::black, 1));
    painter.drawPath(path);
    painter.setPen(QPen(Qt::black, 0.5));
    painter.drawPath(pathUp);
    painter.drawPath(pathDown);
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

void RegrLWPR::SaveParams(QTextStream &file)
{
	file << "regressionOptions" << ":" << "lwprAlpha" << " " << params->lwprAlphaSpin->value() << "\n";
	file << "regressionOptions" << ":" << "lwprInitialD" << " " << params->lwprInitialDSpin->value() << "\n";
	file << "regressionOptions" << ":" << "lwprGen" << " " << params->lwprGenSpin->value() << "\n";
}

bool RegrLWPR::LoadParams(QString name, float value)
{
	if(name.endsWith("lwprAlpha")) params->lwprAlphaSpin->setValue(value);
	if(name.endsWith("lwprInitialD")) params->lwprInitialDSpin->setValue(value);
	if(name.endsWith("lwprGen")) params->lwprGenSpin->setValue(value);
	return true;
}
