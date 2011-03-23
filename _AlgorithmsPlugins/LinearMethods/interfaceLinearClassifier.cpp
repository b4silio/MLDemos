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
#include "interfaceLinearClassifier.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>
#include <QDebug>

using namespace std;

ClassLinear::ClassLinear()
{
	params = new Ui::ParametersLinear();
	params->setupUi(widget = new QWidget());
}

void ClassLinear::SetParams(Classifier *classifier)
{
	if(!classifier) return;
	int type = params->linearTypeCombo->currentIndex();
	((ClassifierLinear *)classifier)->SetParams(type);
}

Classifier *ClassLinear::GetClassifier()
{
	ClassifierLinear *classifier = new ClassifierLinear();
	SetParams(classifier);
	return classifier;
}

bool ClassLinear::UsesDrawTimer()
{
	return params->linearTypeCombo->currentIndex() == 4; // naive bayes is drawn "normally"
}

void ClassLinear::DrawInfo(Canvas *canvas, Classifier *classifier)
{
	if(!canvas || !classifier) return;
	int w = canvas->width();
	int h = canvas->height();
	QPixmap infoPixmap(w, h);
	QBitmap bitmap(w,h);
	bitmap.clear();
	infoPixmap.setMask(bitmap);
	infoPixmap.fill(Qt::transparent);

	QPainter painter(&infoPixmap);
	painter.setRenderHint(QPainter::Antialiasing);
	ClassifierLinear *linear = (ClassifierLinear*)classifier;
	fvec mean = linear->GetMean();
	cvVec2 m(mean[0],mean[1]);
	if(linear->GetType()==1) // ICA
	{
		fvec pt[4];
		cvVec2 v[4];
		QPointF p1[4], p2[4];
		FOR(i,4) pt[i].resize(2,0);
		// we get the bounding box
		float xmin=FLT_MAX,xmax=-FLT_MAX,ymin=FLT_MAX,ymax=-FLT_MAX;
		FOR(i, canvas->data->GetCount())
		{
			fvec sample = linear->Project(canvas->data->GetSample(i));
			xmin = min(sample[0],xmin);
			xmax = max(sample[0],xmax);
			ymin = min(sample[1],ymin);
			ymax = max(sample[1],ymax);
		}

		pt[0][0]=xmin;pt[0][1]=ymin;
		pt[1][0]=xmax;pt[1][1]=ymin;
		pt[2][0]=xmax;pt[2][1]=ymax;
		pt[3][0]=xmin;pt[3][1]=ymax;

		FOR(i, 4)
		{
			p1[i] = canvas->toCanvasCoords(pt[i]);
			fvec proj = linear->InvProject(pt[i]);
			v[i] = cvVec2(proj[0], proj[1]);
			p2[i] = canvas->toCanvasCoords(v[i].x,v[i].y);
		}

		painter.setPen(QPen(Qt::red, 1));
		painter.drawLine(p1[0],p1[1]);
		painter.drawLine(p1[1],p1[2]);
		painter.drawLine(p1[2],p1[3]);
		painter.drawLine(p1[3],p1[0]);

		FOR(i, 4) painter.drawLine(p1[i],p2[i]);

		painter.setPen(QPen(Qt::red, 2));
		painter.drawLine(p2[0],p2[1]);
		painter.drawLine(p2[1],p2[2]);
		painter.drawLine(p2[2],p2[3]);
		painter.drawLine(p2[3],p2[0]);
	}
	else if(linear->GetType() < 4) // PCA, LDA, Fisher
	{
		fvec pt[5];
		QPointF point[4];
		FOR(i,5) pt[i].resize(2,0);
		pt[0][0]=1;pt[0][1]=0;
		pt[1][0]=-1;pt[1][1]=0;
		pt[2][0]=0;pt[2][1]=0;

		FOR(i, 3)
		{
			pt[i] = linear->Project(pt[i]);
		}
//			pt[3][0] = pt[2][0]-pt[0][1];
//			pt[3][1] = pt[2][0]+pt[0][0];
//			pt[4][0] = pt[2][0]-pt[1][1];
//			pt[4][1] = pt[2][0]+pt[1][0];

		point[0] = canvas->toCanvasCoords(pt[0]);
		point[1] = canvas->toCanvasCoords(pt[1]);
//			point[2] = QPointF(pt[3][0]*w, pt[3][1]*h);
//			point[3] = QPointF(pt[4][0]*w, pt[4][1]*h);

		painter.setPen(QPen(Qt::black, 2));
		painter.drawLine(point[0], point[1]);
//			painter.setPen(QPen(Qt::black, 1));
//			painter.drawLine(point[2], point[3]);
	}
	canvas->infoPixmap = infoPixmap;
}

void ClassLinear::Draw(Canvas *canvas, Classifier *classifier)
{
	if(!classifier || !canvas) return;
	canvas->liveTrajectory.clear();
	int w = canvas->width();
	int h = canvas->height();

	int posClass = 1;

	canvas->modelPixmap = QPixmap();

	DrawInfo(canvas, classifier);

	if(((ClassifierLinear*)classifier)->GetType()>=4)
	{
		bool bUseMinMax = true;

		float resMin = FLT_MAX;
		float resMax = -FLT_MAX;
		if(bUseMinMax)
		{
			// TODO: get the min and max for all samples
			std::vector<fvec> samples = canvas->data->GetSamples();
			FOR(i, samples.size())
			{
				float val = classifier->Test(samples[i]);
				if(val > resMax) resMax = val;
				if(val < resMin) resMin = val;
			}
			if(resMin == resMax) resMin -= 3;
		}

		QBitmap bitmap(w,h);
		bitmap.clear();
		canvas->confidencePixmap = QPixmap(w,h);
		canvas->confidencePixmap.setMask(bitmap);
		canvas->confidencePixmap.fill(Qt::transparent);

		// we draw the samples
		canvas->modelPixmap = QPixmap(w,h);
		canvas->modelPixmap.setMask(bitmap);
		canvas->modelPixmap.fill(Qt::transparent);
		QPainter painter(&canvas->modelPixmap);
		painter.setRenderHint(QPainter::Antialiasing, true);
		FOR(i, canvas->data->GetCount())
		{
			fvec sample = canvas->data->GetSample(i);
			int label = canvas->data->GetLabel(i);
			QPointF point = canvas->toCanvasCoords(canvas->data->GetSample(i));
			float response = classifier->Test(sample);
			if(response > 0)
			{
				if(label == posClass) Canvas::drawSample(painter, point, 9, 2);
				else Canvas::drawCross(painter, point, 6, 2);
			}
			else
			{
				if(label != posClass) Canvas::drawSample(painter, point, 9, 0);
				else Canvas::drawCross(painter, point, 6, 0);
			}
		}
	}
	else
	{
		ClassifierLinear *linear = (ClassifierLinear*)classifier;

		canvas->confidencePixmap = QPixmap(w,h);
		canvas->modelPixmap = QPixmap(w,h);
		QBitmap bitmap(w,h);
		bitmap.clear();
		canvas->modelPixmap.setMask(bitmap);
		canvas->confidencePixmap.setMask(bitmap);
		canvas->modelPixmap.fill(Qt::transparent);
		canvas->confidencePixmap.fill(Qt::transparent);

		QPainter painter(&canvas->confidencePixmap);
		QPainter painter2(&canvas->modelPixmap);
		painter.setRenderHint(QPainter::Antialiasing, true);
		painter2.setRenderHint(QPainter::Antialiasing, true);
		if(linear->GetType()<4)
		{
			FOR(i, canvas->data->GetCount())
			{
				fvec sample = canvas->data->GetSample(i);
				int label = canvas->data->GetLabel(i);
				fvec newSample = linear->Project(sample);

				QPointF point = canvas->toCanvasCoords(newSample);
				QPointF original = canvas->toCanvasCoords(canvas->data->GetSample(i));
				painter.setPen(QPen(Qt::black, 0.2));
				painter.drawLine(original, point);
				Canvas::drawSample(painter2, point, 6, label);
			}
		}
	}
	canvas->repaint();
}

void ClassLinear::SaveOptions(QSettings &settings)
{
	settings.setValue("linearType", params->linearTypeCombo->currentIndex());
}

bool ClassLinear::LoadOptions(QSettings &settings)
{
	if(settings.contains("linearType")) params->linearTypeCombo->setCurrentIndex(settings.value("linearType").toInt());
	return true;
}

void ClassLinear::SaveParams(std::ofstream &file)
{
	file << "classificationOptions" << ":" << "linearType" << " " << params->linearTypeCombo->currentIndex() << std::endl;
}

bool ClassLinear::LoadParams(char *line, float value)
{
	if(endsWith(line,"linearType")) params->linearTypeCombo->setCurrentIndex((int)value);
	return true;
}
