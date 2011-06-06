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
#include "interfaceGMMRegress.h"
#include "drawUtils.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>

using namespace std;

RegrGMM::RegrGMM()
{
	params = new Ui::ParametersGMMRegr();
	params->setupUi(widget = new QWidget());
}

void RegrGMM::SetParams(Regressor *regressor)
{
	if(!regressor) return;
	int clusters = params->gmmCount->value();
	int covType = params->gmmCovarianceCombo->currentIndex();
	int initType = params->gmmInitCombo->currentIndex();

	((RegressorGMR *)regressor)->SetParams(clusters, covType, initType);
}

Regressor *RegrGMM::GetRegressor()
{
	RegressorGMR *regressor = new RegressorGMR();
	SetParams(regressor);
	return regressor;
}

void RegrGMM::DrawInfo(Canvas *canvas, QPainter &painter, Regressor *regressor)
{
	if(!canvas || !regressor) return;
	painter.setRenderHint(QPainter::Antialiasing);

	RegressorGMR * gmr = (RegressorGMR*)regressor;
	Gmm *gmm = gmr->gmm;
	float mean[2];
	float sigma[4];
	FOR(i, gmm->nstates)
	{
		gmm->getMean(i, mean);
		gmm->getCovariance(i, sigma, true);
		//FOR(j,4) sigma[j] = sqrt(sigma[j]);
		painter.setPen(QPen(Qt::black, 1));
		DrawEllipse(mean, sigma, 1, &painter, canvas);
		painter.setPen(QPen(Qt::black, 0.5));
		DrawEllipse(mean, sigma, 2, &painter, canvas);
		QPointF point = canvas->toCanvasCoords(mean[0],mean[1]);
		painter.setPen(QPen(Qt::black, 4));
		painter.drawEllipse(point, 2, 2);
		painter.setPen(QPen(Qt::white, 2));
		painter.drawEllipse(point, 2, 2);
	}
}

void RegrGMM::DrawConfidence(Canvas *canvas, Regressor *regressor)
{
	int w = canvas->width();
	int h = canvas->height();

	RegressorGMR *gmr = ((RegressorGMR *)regressor);

	QImage density(QSize(256,256), QImage::Format_RGB32);
	density.fill(0);
	// we draw a density map for the probability
	fvec sample;
	sample.resize(2,0);
	float sigma[4];
	for (int i=0; i < density.width(); i++)
	{
		for (int j=0; j< density.height(); j++)
		{
			sample = canvas->toSampleCoords(i*w/density.width(),j*h/density.height());
			float val = gmr->gmm->pdf(&sample[0]);
			int color = min(255,(int)(128 + val*10));
			density.setPixel(i,j, qRgb(color,color,color));
		}
	}
	canvas->confidencePixmap = QPixmap::fromImage(density.scaled(QSize(w,h),Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

void RegrGMM::DrawModel(Canvas *canvas, QPainter &painter, Regressor *regressor)
{
	if(!regressor || !canvas) return;
	int w = canvas->width();
	int h = canvas->height();
	painter.setRenderHint(QPainter::Antialiasing, true);

	int steps = w;
	QPointF oldPoint(-FLT_MAX,-FLT_MAX);
	QPointF oldPointUp(-FLT_MAX,-FLT_MAX);
	QPointF oldPointDown(-FLT_MAX,-FLT_MAX);
	fvec sample;sample.resize(2, 0);
	painter.setBrush(Qt::NoBrush);
	FOR(x, steps)
	{
		sample = canvas->toSampleCoords(x, 0);
		fvec res = regressor->Test(sample);
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
			painter.drawLine(point + pointUp, oldPoint + oldPointUp);
			painter.drawLine(point + pointDown, oldPoint + oldPointDown);

			painter.setPen(QPen(Qt::black, 0.25));
			painter.drawLine(point + 2*pointUp, oldPoint + 2*oldPointUp);
			painter.drawLine(point + 2*pointDown, oldPoint + 2*oldPointDown);

		}
		oldPoint = point;
		oldPointUp = pointUp;
		oldPointDown = pointDown;
	}
}

void RegrGMM::SaveOptions(QSettings &settings)
{
	settings.setValue("gmmCount", params->gmmCount->value());
	settings.setValue("gmmCovariance", params->gmmCovarianceCombo->currentIndex());
	settings.setValue("gmmInit", params->gmmInitCombo->currentIndex());
}

bool RegrGMM::LoadOptions(QSettings &settings)
{
	if(settings.contains("gmmCount")) params->gmmCount->setValue(settings.value("gmmCount").toFloat());
	if(settings.contains("gmmCovariance")) params->gmmCovarianceCombo->setCurrentIndex(settings.value("gmmCovariance").toInt());
	if(settings.contains("gmmInit")) params->gmmInitCombo->setCurrentIndex(settings.value("gmmInit").toInt());
	return true;
}

void RegrGMM::SaveParams(std::ofstream &file)
{
	file << "regressionOptions" << ":" << "gmmCount" << " " << params->gmmCount->value() << std::endl;
	file << "regressionOptions" << ":" << "gmmCovariance" << " " << params->gmmCovarianceCombo->currentIndex() << std::endl;
	file << "regressionOptions" << ":" << "gmmInit" << " " << params->gmmInitCombo->currentIndex() << std::endl;
}

bool RegrGMM::LoadParams(char *line, float value)
{
	if(endsWith(line,"gmmCount")) params->gmmCount->setValue((int)value);
	if(endsWith(line,"gmmCovariance")) params->gmmCovarianceCombo->setCurrentIndex((int)value);
	if(endsWith(line,"gmmInit")) params->gmmInitCombo->setCurrentIndex((int)value);
	return true;
}
