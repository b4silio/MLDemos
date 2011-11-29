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

void DynamicGMM::DrawInfo(Canvas *canvas, QPainter &painter, Dynamical *dynamical)
{
	if(!canvas || !dynamical) return;
	painter.setRenderHint(QPainter::Antialiasing);

    Gmm *gmm = ((DynamicalGMR*)dynamical)->gmm;
    int xIndex = canvas->xIndex;
    int yIndex = canvas->yIndex;
    int dim = gmm->dim;
    float mean[2];
    float sigma[3];
    painter.setBrush(Qt::NoBrush);
    FOR(i, gmm->nstates)
    {
        float* bigSigma = new float[dim*dim];
        float* bigMean = new float[dim];
        gmm->getCovariance(i, bigSigma, false);
        sigma[0] = bigSigma[xIndex*dim + xIndex];
        sigma[1] = bigSigma[yIndex*dim + xIndex];
        sigma[2] = bigSigma[yIndex*dim + yIndex];
        gmm->getMean(i, bigMean);
        mean[0] = bigMean[xIndex];
        mean[1] = bigMean[yIndex];
        delete [] bigSigma;
        delete [] bigMean;
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

void DynamicGMM::DrawModel(Canvas *canvas, QPainter &painter, Dynamical *dynamical)
{
	if(!canvas || !dynamical) return;
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

void DynamicGMM::SaveParams(QTextStream &file)
{
	file << "dynamicalOptions" << ":" << "gmmCount" << " " << params->gmmCount->value() << "\n";
	file << "dynamicalOptions" << ":" << "gmmCovariance" << " " << params->gmmCovarianceCombo->currentIndex() << "\n";
	file << "dynamicalOptions" << ":" << "gmmInit" << " " << params->gmmInitCombo->currentIndex() << "\n";
}

bool DynamicGMM::LoadParams(QString name, float value)
{
	if(name.endsWith("gmmCount")) params->gmmCount->setValue((int)value);
	if(name.endsWith("gmmCovariance")) params->gmmCovarianceCombo->setCurrentIndex((int)value);
	if(name.endsWith("gmmInit")) params->gmmInitCombo->setCurrentIndex((int)value);
	return true;
}
