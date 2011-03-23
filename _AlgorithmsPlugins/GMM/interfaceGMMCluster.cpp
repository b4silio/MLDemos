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
#include "interfaceGMMCluster.h"
#include "basicOpenCV.h"
#include "drawUtils.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>

using namespace std;

ClustGMM::ClustGMM()
{
	params = new Ui::ParametersGMMClust();
	params->setupUi(widget = new QWidget());
}

void ClustGMM::SetParams(Clusterer *clusterer)
{
	if(!clusterer) return;
	int clusters = params->gmmCount->value();
	int covType = params->gmmCovarianceCombo->currentIndex();
	int initType = params->gmmInitCombo->currentIndex();

	((ClustererGMM *)clusterer)->SetParams(clusters, covType, initType);
}

Clusterer *ClustGMM::GetClusterer()
{
	ClustererGMM *clusterer = new ClustererGMM();
	SetParams(clusterer);
	return clusterer;
}

void ClustGMM::DrawInfo(Canvas *canvas, Clusterer *clusterer)
{
	if(!canvas || !clusterer) return;
	int w = canvas->width();
	int h = canvas->height();
	QPixmap infoPixmap(w, h);
	QBitmap bitmap(w,h);
	bitmap.clear();
	infoPixmap.setMask(bitmap);
	infoPixmap.fill(Qt::transparent);

	QPainter painter(&infoPixmap);
	painter.setRenderHint(QPainter::Antialiasing);

	ClustererGMM * _gmm = (ClustererGMM*)clusterer;
	Gmm *gmm = _gmm->gmm;
	float mean[2];
	float sigma[4];
	FOR(i, gmm->nstates)
	{
		gmm->getMean(i, mean);
		gmm->getCovariance(i, sigma, true);
		//FOR(j,4) sigma[j] = sqrt(sigma[j]);
		painter.setPen(QPen(Qt::black, 2));
		DrawEllipse(mean, sigma, 1, &painter, canvas);
		painter.setPen(QPen(Qt::black, 1));
		DrawEllipse(mean, sigma, 2, &painter, canvas);
		QPointF point = canvas->toCanvasCoords(mean[0], mean[1]);
		CvScalar color = CV::color[(i+1)%CV::colorCnt];
		painter.setPen(QPen(Qt::black, 12));
		painter.drawEllipse(point, 8, 8);
		painter.setPen(QPen(QColor(color.val[0],color.val[1],color.val[2]),4));
		painter.drawEllipse(point, 8, 8);
	}

	canvas->infoPixmap = infoPixmap;
}

void ClustGMM::Draw(Canvas *canvas, Clusterer *clusterer)
{
	if(!canvas || !clusterer) return;
	canvas->liveTrajectory.clear();
	DrawInfo(canvas, clusterer);
	int w = canvas->width();
	int h = canvas->height();

#define RES 256
	bool bDrawConfidence = canvas->bDisplayMap;

	if(bDrawConfidence)
	{
		IplImage *image = NULL;
		image = cvCreateImage(cvSize(canvas->width(),canvas->height()), 8, 3);
		cvSet(image, CV_RGB(255,255,255));

		fvec sample;
		sample.resize(2,0);
		FOR(y, image->height)
		{
			FOR(x, image->width)
			{
				sample = canvas->toSampleCoords(x,y);
				fvec res = clusterer->Test(sample);
				float r=0,g=0,b=0;
				if(res.size() > 1)
				{
					FOR(i, res.size())
					{
						r += CV::color[(i+1)%CV::colorCnt].val[0]*res[i];
						g += CV::color[(i+1)%CV::colorCnt].val[1]*res[i];
						b += CV::color[(i+1)%CV::colorCnt].val[2]*res[i];
					}
				}
				else if(res.size())
				{
					r = (1-res[0])*255 + res[0]* 255;
					g = (1-res[0])*255;
					b = (1-res[0])*255;
				}
				if( r < 10 && g < 10 && b < 10) r = b = g = 255;

				image->imageData[y*image->widthStep + x*3 + 2] = (u8)r;
				image->imageData[y*image->widthStep + x*3 + 1] = (u8)g;
				image->imageData[y*image->widthStep + x*3 + 0] = (u8)b;
			}
		}

		IplImage *big = cvCreateImage(cvSize(canvas->width(), canvas->height()),8,3);
		cvResize(image, big, CV_INTER_CUBIC);
		canvas->confidencePixmap = Canvas::toPixmap(big);
		IMKILL(image);
		IMKILL(big);
	}
	else
	{
		canvas->confidencePixmap = QPixmap();
	}

	QPixmap modelPixmap(w, h);
	QBitmap bitmap(w,h);
	bitmap.clear();
	modelPixmap.setMask(bitmap);
	modelPixmap.fill(Qt::transparent);
	QPainter painter(&modelPixmap);
	painter.setRenderHint(QPainter::Antialiasing);

	FOR(i, canvas->data->GetSamples().size())
	{
		fvec sample = canvas->data->GetSample(i);
		QPointF point = canvas->toCanvasCoords(sample);
		fvec res = clusterer->Test(sample);
		float r=0,g=0,b=0;
		if(res.size() > 1)
		{
			FOR(j, res.size())
			{
				r += CV::color[(j+1)%CV::colorCnt].val[0]*res[j];
				g += CV::color[(j+1)%CV::colorCnt].val[1]*res[j];
				b += CV::color[(j+1)%CV::colorCnt].val[2]*res[j];
			}
		}
		else if(res.size())
		{
			r = (1-res[0])*255 + res[0]* 255;
			g = (1-res[0])*255;
			b = (1-res[0])*255;
		}
		painter.setBrush(QColor(r,g,b));
		painter.setPen(Qt::black);
		painter.drawEllipse(point,5,5);
	}
	canvas->modelPixmap = modelPixmap;
	canvas->repaint();
}

void ClustGMM::SaveOptions(QSettings &settings)
{
	settings.setValue("gmmCount", params->gmmCount->value());
	settings.setValue("gmmCovariance", params->gmmCovarianceCombo->currentIndex());
	settings.setValue("gmmInit", params->gmmInitCombo->currentIndex());
}

bool ClustGMM::LoadOptions(QSettings &settings)
{
	if(settings.contains("gmmCount")) params->gmmCount->setValue(settings.value("gmmCount").toFloat());
	if(settings.contains("gmmCovariance")) params->gmmCovarianceCombo->setCurrentIndex(settings.value("gmmCovariance").toInt());
	if(settings.contains("gmmInit")) params->gmmInitCombo->setCurrentIndex(settings.value("gmmInit").toInt());
	return true;
}

void ClustGMM::SaveParams(std::ofstream &file)
{
	file << "clusterOptions" << ":" << "gmmCount" << " " << params->gmmCount->value() << std::endl;
	file << "clusterOptions" << ":" << "gmmCovariance" << " " << params->gmmCovarianceCombo->currentIndex() << std::endl;
	file << "clusterOptions" << ":" << "gmmInit" << " " << params->gmmInitCombo->currentIndex() << std::endl;
}

bool ClustGMM::LoadParams(char *line, float value)
{
	if(endsWith(line,"gmmCount")) params->gmmCount->setValue((int)value);
	if(endsWith(line,"gmmCovariance")) params->gmmCovarianceCombo->setCurrentIndex((int)value);
	if(endsWith(line,"gmmInit")) params->gmmInitCombo->setCurrentIndex((int)value);
	return true;
}
