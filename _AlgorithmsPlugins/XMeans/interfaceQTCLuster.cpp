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
#include "interfaceQTCluster.h"
#include "drawUtils.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>

using namespace std;

ClustQTClust::ClustQTClust()
{
	params = new Ui::ParametersQTClust();
	params->setupUi(widget = new QWidget());
}

void ClustQTClust::SetParams(Clusterer *clusterer)
{
	if(!clusterer) return;
	double distance = params->minDistanceSpin->value();
	int minCount = params->minSampleCount->value();

	((ClustererQTClust *)clusterer)->SetParams(distance, minCount);
}

Clusterer *ClustQTClust::GetClusterer()
{
	ClustererQTClust *clusterer = new ClustererQTClust();
	SetParams(clusterer);
	return clusterer;
}

void ClustQTClust::DrawInfo(Canvas *canvas, QPainter &painter, Clusterer *clusterer)
{
	if(!canvas || !clusterer) return;
	painter.setRenderHint(QPainter::Antialiasing);

	map<int,fvec> centers = ((ClustererQTClust*)clusterer)->centers;

	painter.setBrush(Qt::NoBrush);
	for(map<int,fvec>::iterator it = centers.begin(); it != centers.end(); it++)
	{
		int label = it->first+1;
		if(label == -1) continue; // outliers!
		fvec sample = it->second;
		QPointF point = canvas->toCanvasCoords(sample);
		QColor color = SampleColor[label%SampleColorCnt];
		painter.setPen(QPen(Qt::black, 12));
		painter.drawEllipse(point, 8, 8);
		painter.setPen(QPen(color,4));
		painter.drawEllipse(point, 8, 8);
	}
}

void ClustQTClust::DrawModel(Canvas *canvas, QPainter &painter, Clusterer *clusterer)
{
	painter.setRenderHint(QPainter::Antialiasing);

	vector<fvec> samples = ((ClustererQTClust*)clusterer)->samples;
	ivec clusters = ((ClustererQTClust*)clusterer)->clusters;
	FOR(i, samples.size())
	{
		fvec sample = samples[i];
		int label = clusters[i]+1;
		QPointF point = canvas->toCanvasCoords(sample);
		int radius = 8;
		QColor color = SampleColor[label%SampleColorCnt];
		QColor edge = Qt::black;
		painter.setBrush(color);
		painter.setPen(edge);
		painter.drawEllipse(QRectF(point.x()-radius/2.,point.y()-radius/2.,radius,radius));
	}
}

void ClustQTClust::SaveOptions(QSettings &settings)
{
	settings.setValue("minDistanceSpin", params->minDistanceSpin->value());
	settings.setValue("minSampleCount", params->minSampleCount->value());
}

bool ClustQTClust::LoadOptions(QSettings &settings)
{
	if(settings.contains("minDistanceSpin")) params->minDistanceSpin->setValue(settings.value("minDistanceSpin").toDouble());
	if(settings.contains("minSampleCount")) params->minSampleCount->setValue(settings.value("minSampleCount").toInt());
	return true;
}

void ClustQTClust::SaveParams(QTextStream &file)
{
	file << "clusterOptions" << ":" << "minDistanceSpin" << " " << params->minDistanceSpin->value() << "\n";
	file << "clusterOptions" << ":" << "minSampleCount" << " " << params->minSampleCount->value() << "\n";
}

bool ClustQTClust::LoadParams(QString name, float value)
{
	if(name.endsWith("minDistanceSpin")) params->minDistanceSpin->setValue((double)value);
	if(name.endsWith("minSampleCount")) params->minSampleCount->setValue((int)value);
	return true;
}

Q_EXPORT_PLUGIN2(mld_XMeans, ClustQTClust)
