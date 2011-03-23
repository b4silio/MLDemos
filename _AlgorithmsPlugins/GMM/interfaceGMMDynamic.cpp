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
#include "interfaceGMMDynamic.h"
#include "drawUtils.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>

using namespace std;

DynamicGMM::DynamicGMM()
{
	params = new Ui::ParametersGMMDynamic();
	params->setupUi(widget = new QWidget());
}

void DynamicGMM::SetParams(Dynamical *dynamical)
{
	if(!dynamical) return;
	int clusters = params->gmmCount->value();
	int covType = params->gmmCovarianceCombo->currentIndex();
	int initType = params->gmmInitCombo->currentIndex();

	((DynamicalGMR *)dynamical)->SetParams(clusters, covType, initType);
}

Dynamical *DynamicGMM::GetDynamical()
{
	DynamicalGMR *dynamical = new DynamicalGMR();
	SetParams(dynamical);
	return dynamical;
}

void DynamicGMM::DrawInfo(Canvas *canvas, Dynamical *dynamical)
{
	if(!canvas || !dynamical) return;
	int w = canvas->width();
	int h = canvas->height();
	QPixmap infoPixmap(w, h);
	QBitmap bitmap(w,h);
	bitmap.clear();
	infoPixmap.setMask(bitmap);
	infoPixmap.fill(Qt::transparent);

	QPainter painter(&infoPixmap);
	painter.setRenderHint(QPainter::Antialiasing);

	Gmm *gmm = ((DynamicalGMR*)dynamical)->gmm;
	float mean[4];
	float sigma[10];
	float drawMean[2];
	float drawSigma[3];
	FOR(i, gmm->nstates)
	{
		gmm->getMean(i, mean);
		gmm->getCovariance(i, sigma, true);
		drawMean[0] = mean[0]; drawMean[1] = mean[1];
		drawSigma[0] = sigma[0];
		drawSigma[1] = sigma[1];
		drawSigma[2] = sigma[4];
		painter.setPen(QPen(Qt::black, 1));
		DrawEllipse(drawMean, drawSigma, 1, &painter, canvas);
		painter.setPen(QPen(Qt::black, 0.5));
		DrawEllipse(drawMean, drawSigma, 2, &painter, canvas);
		QPointF point = canvas->toCanvasCoords(drawMean[0],drawMean[1]);
		painter.setPen(QPen(Qt::black, 4));
		painter.drawEllipse(point, 2, 2);
		painter.setPen(QPen(Qt::white, 2));
		painter.drawEllipse(point, 2, 2);
	}

	canvas->infoPixmap = infoPixmap;
}

void DynamicGMM::Draw(Canvas *canvas, Dynamical *dynamical)
{
	if(!dynamical || !canvas) return;
	if(!dynamical) return;
	DrawInfo(canvas, dynamical);
	int w = canvas->width();
	int h = canvas->height();
	canvas->modelPixmap = QPixmap(w,h);
	canvas->confidencePixmap = QPixmap(w,h);
	QBitmap bitmap(w,h);
	bitmap.clear();
	canvas->modelPixmap.setMask(bitmap);
	canvas->modelPixmap.fill(Qt::transparent);
	canvas->repaint();
}

void DynamicGMM::SaveOptions(QSettings &settings)
{
	settings.setValue("gmmCount", params->gmmCount->value());
	settings.setValue("gmmCovariance", params->gmmCovarianceCombo->currentIndex());
	settings.setValue("gmmInit", params->gmmInitCombo->currentIndex());
}

bool DynamicGMM::LoadOptions(QSettings &settings)
{
	if(settings.contains("gmmCount")) params->gmmCount->setValue(settings.value("gmmCount").toFloat());
	if(settings.contains("gmmCovariance")) params->gmmCovarianceCombo->setCurrentIndex(settings.value("gmmCovariance").toInt());
	if(settings.contains("gmmInit")) params->gmmInitCombo->setCurrentIndex(settings.value("gmmInit").toInt());
	return true;
}

void DynamicGMM::SaveParams(std::ofstream &file)
{
	file << "dynamicalOptions" << ":" << "gmmCount" << " " << params->gmmCount->value() << std::endl;
	file << "dynamicalOptions" << ":" << "gmmCovariance" << " " << params->gmmCovarianceCombo->currentIndex() << std::endl;
	file << "dynamicalOptions" << ":" << "gmmInit" << " " << params->gmmInitCombo->currentIndex() << std::endl;
}

bool DynamicGMM::LoadParams(char *line, float value)
{
	if(endsWith(line,"gmmCount")) params->gmmCount->setValue((int)value);
	if(endsWith(line,"gmmCovariance")) params->gmmCovarianceCombo->setCurrentIndex((int)value);
	if(endsWith(line,"gmmInit")) params->gmmInitCombo->setCurrentIndex((int)value);
	return true;
}
