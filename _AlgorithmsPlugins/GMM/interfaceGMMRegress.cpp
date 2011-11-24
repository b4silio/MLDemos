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

QString RegrGMM::GetAlgoString()
{
	int clusters = params->gmmCount->value();
	int covType = params->gmmCovarianceCombo->currentIndex();
	int initType = params->gmmInitCombo->currentIndex();
	QString algo = QString("GMM %1").arg(clusters);
	switch(covType)
	{
	case 0:
		algo += " Ful";
		break;
	case 1:
		algo += " Dia";
		break;
	case 2:
		algo += " Sph";
		break;
	}
	switch(initType)
	{
	case 0:
		algo += " Rnd";
		break;
	case 1:
		algo += " Uni";
		break;
	case 2:
		algo += " K-M";
		break;
	}
	return algo;
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
	int dim = canvas->data->GetCount() ? canvas->data->GetSample(0).size() : gmm->dim;
    int outputDim = regressor->outputDim;
	int xIndex = canvas->xIndex;
	int yIndex = canvas->yIndex;
    if(outputDim != -1 && outputDim < dim-1);
    {
        if(xIndex == dim-1) xIndex = outputDim;
        else if (xIndex == outputDim) xIndex = dim-1;
        if(yIndex == dim-1) yIndex = outputDim;
        else if(yIndex == outputDim) yIndex = dim-1;
    }
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

		painter.setPen(QPen(Qt::black, 1));
		DrawEllipse(mean, sigma, 1, &painter, canvas);
		painter.setPen(QPen(Qt::black, 0.5));
		DrawEllipse(mean, sigma, 2, &painter, canvas);
		QPointF point = canvas->toCanvasCoords(mean[0],mean[1]);
		painter.setPen(QPen(Qt::black, 4));
		painter.drawEllipse(point, 2, 2);
		painter.setPen(QPen(Qt::white, 2));
		painter.drawEllipse(point, 2, 2);
		/*
		QColor color = CVColor[(i+1)%CVColorCnt];
		painter.setPen(QPen(Qt::black, 12));
		painter.drawEllipse(point, 8, 8);
		painter.setPen(QPen(color,4));
		painter.drawEllipse(point, 8, 8);
		*/
	}
}

void RegrGMM::DrawConfidence(Canvas *canvas, Regressor *regressor)
{
	int w = canvas->width();
	int h = canvas->height();

	RegressorGMR *gmr = ((RegressorGMR *)regressor);
    int outputDim = regressor->outputDim;

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
            int dim = sample.size();
            if(outputDim != -1 && outputDim < dim)
            {
                float tmp = sample[outputDim];
                sample[outputDim] = sample[dim-1];
                sample[dim-1] = tmp;
            }
			float val = gmr->gmm->pdf(&sample[0]);
			int color = min(255,(int)(128 + val*10));
			density.setPixel(i,j, qRgb(color,color,color));
		}
	}
	canvas->maps.confidence = QPixmap::fromImage(density.scaled(QSize(w,h),Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

void RegrGMM::DrawModel(Canvas *canvas, QPainter &painter, Regressor *regressor)
{
	if(!regressor || !canvas) return;
	int w = canvas->width();
	int h = canvas->height();
	painter.setRenderHint(QPainter::Antialiasing, true);
    int xIndex = canvas->xIndex;
    int yIndex = canvas->yIndex;
    int outputDim = regressor->outputDim;

	int steps = w;
	QPointF oldPoint(-FLT_MAX,-FLT_MAX);
	QPointF oldPointUp(-FLT_MAX,-FLT_MAX);
	QPointF oldPointDown(-FLT_MAX,-FLT_MAX);
	fvec sample;sample.resize(2, 0);
	painter.setBrush(Qt::NoBrush);
	FOR(x, steps)
	{
        sample = canvas->toSampleCoords(x, 0);
        int dim = sample.size();
        if(dim > 2) continue;
        if(outputDim==-1) outputDim = dim-1;
        fvec res = regressor->Test(sample);
		if(res[0] != res[0] || res[1] != res[1]) continue;
        sample[outputDim] = res[0];
        QPointF point = canvas->toCanvasCoords(sample);
        sample[outputDim] = res[0]+res[1];
        QPointF pointUp = canvas->toCanvasCoords(sample);
        pointUp.setX(0);
        pointUp.setY(pointUp.y() - point.y());
        sample[outputDim] = res[0]-res[1];
        QPointF pointDown = canvas->toCanvasCoords(sample);
        //pointDown = -pointUp;
        pointDown.setX(0);
        pointDown.setY(pointDown.y() - point.y());
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

void RegrGMM::SaveParams(QTextStream &file)
{
	file << "regressionOptions" << ":" << "gmmCount" << " " << params->gmmCount->value() << "\n";
	file << "regressionOptions" << ":" << "gmmCovariance" << " " << params->gmmCovarianceCombo->currentIndex() << "\n";
	file << "regressionOptions" << ":" << "gmmInit" << " " << params->gmmInitCombo->currentIndex() << "\n";
}

bool RegrGMM::LoadParams(QString name, float value)
{
	if(name.endsWith("gmmCount")) params->gmmCount->setValue((int)value);
	if(name.endsWith("gmmCovariance")) params->gmmCovarianceCombo->setCurrentIndex((int)value);
	if(name.endsWith("gmmInit")) params->gmmInitCombo->setCurrentIndex((int)value);
	return true;
}
