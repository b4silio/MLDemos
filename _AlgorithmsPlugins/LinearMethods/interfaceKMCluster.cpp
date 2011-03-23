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
#include "interfaceKMCluster.h"
#include "basicOpenCV.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>

using namespace std;

ClustKM::ClustKM()
{
	params = new Ui::ParametersKM();
	params->setupUi(widget = new QWidget());
}

void ClustKM::SetParams(Clusterer *clusterer)
{
	if(!clusterer) return;
	int clusters = params->kmeansClusterSpin->value();
	int power = params->kmeansNormSpin->value();
	int metrictype = params->kmeansNormCombo->currentIndex();
	float beta = params->kmeansBetaSpin->value();
	int method = params->kmeansMethodCombo->currentIndex();
	if (metrictype < 3) power = metrictype;
	((ClustererKM *)clusterer)->SetParams(clusters, method, beta, power);
}

Clusterer *ClustKM::GetClusterer()
{
	ClustererKM *clusterer = new ClustererKM();
	SetParams(clusterer);
	return clusterer;
}

void ClustKM::DrawInfo(Canvas *canvas, Clusterer *clusterer)
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
	ClustererKM * _kmeans = (ClustererKM*)clusterer;
	KMeansCluster *kmeans = _kmeans->kmeans;
	FOR(i, kmeans->GetClusters())
	{
		fvec mean = kmeans->GetMean(i);
		QPointF point = canvas->toCanvasCoords(mean);
		CvScalar color = CV::color[(i+1)%CV::colorCnt];
		painter.setPen(QPen(Qt::black, 12));
		painter.drawEllipse(point, 8, 8);
		painter.setPen(QPen(QColor(color.val[0],color.val[1],color.val[2]),4));
		painter.drawEllipse(point, 8, 8);
		//painter.setPen(QPen(Qt::white, 2));
		//painter.drawEllipse(point, 8, 8);
	}
	canvas->infoPixmap = infoPixmap;
}

void ClustKM::Draw(Canvas *canvas, Clusterer *clusterer)
{
	if(!canvas || !clusterer) return;
	canvas->liveTrajectory.clear();
	DrawInfo(canvas, clusterer);
	int w = canvas->width();
	int h = canvas->height();

	bool bDrawConfidence = canvas->bDisplayMap;

	if(bDrawConfidence)
	{
		IplImage *image = NULL;
		image = cvCreateImage(cvSize(w,h), 8, 3);
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

void ClustKM::SaveOptions(QSettings &settings)
{
	settings.setValue("kmeansBeta", params->kmeansBetaSpin->value());
	settings.setValue("kmeansCluster", params->kmeansClusterSpin->value());
	settings.setValue("kmeansMethod", params->kmeansMethodCombo->currentIndex());
	settings.setValue("kmeansPower", params->kmeansNormSpin->value());
	settings.setValue("kmeansNormCombo", params->kmeansNormCombo->currentIndex());
}

bool ClustKM::LoadOptions(QSettings &settings)
{
	if(settings.contains("kmeansBeta")) params->kmeansBetaSpin->setValue(settings.value("kmeansBeta").toFloat());
	if(settings.contains("kmeansCluster")) params->kmeansClusterSpin->setValue(settings.value("kmeansCluster").toFloat());
	if(settings.contains("kmeansMethod")) params->kmeansMethodCombo->setCurrentIndex(settings.value("kmeansMethod").toInt());
	if(settings.contains("kmeansPower")) params->kmeansNormSpin->setValue(settings.value("kmeansPower").toFloat());
	if(settings.contains("kmeansNormCombo")) params->kmeansNormCombo->setCurrentIndex(settings.value("kmeansNormCombo").toInt());
	return true;
}

void ClustKM::SaveParams(std::ofstream &file)
{
	file << "clusterOptions" << ":" << "kmeansBeta" << " " << params->kmeansBetaSpin->value() << std::endl;
	file << "clusterOptions" << ":" << "kmeansCluster" << " " << params->kmeansClusterSpin->value() << std::endl;
	file << "clusterOptions" << ":" << "kmeansMethod" << " " << params->kmeansMethodCombo->currentIndex() << std::endl;
	file << "clusterOptions" << ":" << "kmeansPower" << " " << params->kmeansNormSpin->value() << std::endl;
	file << "clusterOptions" << ":" << "kmeansNormCombo" << " " << params->kmeansNormCombo->currentIndex() << std::endl;
}

bool ClustKM::LoadParams(char *line, float value)
{
	if(endsWith(line,"kmeansBeta")) params->kmeansBetaSpin->setValue(value);
	if(endsWith(line,"kmeansCluster")) params->kmeansClusterSpin->setValue((int)value);
	if(endsWith(line,"kmeansMethod")) params->kmeansMethodCombo->setCurrentIndex((int)value);
	if(endsWith(line,"kmeansPower")) params->kmeansNormSpin->setValue((int)value);
	if(endsWith(line,"kmeansNormCombo")) params->kmeansNormCombo->setCurrentIndex((int)value);
	return true;
}
