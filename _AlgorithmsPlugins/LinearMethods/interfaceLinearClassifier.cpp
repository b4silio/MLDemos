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
	projectionWindow = NULL;
	canvas = NULL;
	classifier = NULL;
	bDataIsFromCanvas = false;
	connect(params->projectionButton, SIGNAL(clicked()), this, SLOT(ShowProjection()));
	connect(params->toCanvasButton, SIGNAL(clicked()), this, SLOT(SendToCanvas()));
}

void ClassLinear::SetParams(Classifier *classifier)
{
	if(!classifier) return;
	int type = params->linearTypeCombo->currentIndex();
	((ClassifierLinear *)classifier)->SetParams(type);
}

QString ClassLinear::GetAlgoString()
{
	int type = params->linearTypeCombo->currentIndex();
	switch(type)
	{
	case 0:
		return "PCA";
		break;
	case 1:
		return "LDA";
		break;
	case 2:
		return "Fisher-LDA";
		break;
	case 3:
		return "ICA";
		break;
	case 4:
		return "Naive Bayes";
		break;
	}
}

Classifier *ClassLinear::GetClassifier()
{
	ClassifierLinear *classifier = new ClassifierLinear();
	SetParams(classifier);
	this->classifier = classifier;
	return classifier;
}

bool ClassLinear::UsesDrawTimer()
{
	return params->linearTypeCombo->currentIndex() == 4; // naive bayes is drawn "normally"
}

void ClassLinear::DrawInfo(Canvas *canvas, QPainter &painter, Classifier *classifier)
{
	if(!canvas || !classifier) return;
	if(!this->canvas) this->canvas = canvas;
	painter.setRenderHint(QPainter::Antialiasing);
	ClassifierLinear *linear = (ClassifierLinear*)classifier;
	fvec mean = linear->GetMean();
	fVec m(mean[0],mean[1]);
	if(linear->GetType()==3) // ICA
	{
	}
	else if(linear->GetType() < 3) // PCA, LDA, Fisher
	{
		fvec pt[5];
		QPointF point[4];
		FOR(i,5) pt[i].resize(2,0);
		pt[0][0]=1;pt[0][1]=0;
		pt[1][0]=-1;pt[1][1]=0;
		pt[2][0]=0;pt[2][1]=0;
		FOR(i, 3) pt[i] = linear->Project(pt[i]);
		point[0] = canvas->toCanvasCoords(pt[0]);
		point[1] = canvas->toCanvasCoords(pt[1]);
		painter.setPen(QPen(Qt::black, 2));
		painter.drawLine(point[0], point[1]);
	}
	if(projectionWindow && projectionWindow->isVisible()) ShowProjection();
}

void ClassLinear::DrawModel(Canvas *canvas, QPainter &painter, Classifier *classifier)
{
	if(!classifier || !canvas) return;
	if(!this->canvas) this->canvas = canvas;
	painter.setRenderHint(QPainter::Antialiasing, true);

	int posClass = 1;
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

void ClassLinear::SaveOptions(QSettings &settings)
{
	settings.setValue("linearType", params->linearTypeCombo->currentIndex());
}

bool ClassLinear::LoadOptions(QSettings &settings)
{
	if(settings.contains("linearType")) params->linearTypeCombo->setCurrentIndex(settings.value("linearType").toInt());
	return true;
}

void ClassLinear::SaveParams(QTextStream &file)
{
	file << "classificationOptions" << ":" << "linearType" << " " << params->linearTypeCombo->currentIndex() << "\n";
}

bool ClassLinear::LoadParams(QString name, float value)
{
	if(name.endsWith("linearType")) params->linearTypeCombo->setCurrentIndex((int)value);
	return true;
}

void ClassLinear::ShowProjection()
{
	//if(projectionWindow && projectionWindow->isVisible())
	//{
	//	projectionWindow->hide();
	//	return;
	//}
	if(!classifier || !canvas) return;
	// we project all the data into a new image
	int w = classifier->GetType() ==3 ? canvas->width() : canvas->width()/2;
	int h = canvas->height()/2;
	QPixmap projectionPixmap(w, h);
	projectionPixmap.fill();
	vector<fvec> samples = canvas->data->GetSamples();
	if(!samples.size()) return;
	ivec labels = canvas->data->GetLabels();

	QPainter painter(&projectionPixmap);
	painter.setRenderHint(QPainter::Antialiasing, true);

	if(classifier->GetType() < 3) // PCA, LDA, Fisher
	{
		fvec pt[5];
		QPointF point[4];
		FOR(i,5) pt[i].resize(2,0);
		pt[0][0]=1;pt[0][1]=0;
		pt[1][0]=-1;pt[1][1]=0;
		pt[2][0]=0;pt[2][1]=0;

		FOR(i, 3)
		{
			pt[i] = classifier->Project(pt[i]);
		}

		point[0] = canvas->toCanvasCoords(pt[0])*0.5f;
		point[1] = canvas->toCanvasCoords(pt[1])*0.5f;
		painter.setPen(QPen(Qt::black, 2));
		painter.drawLine(point[0], point[1]);

		// draw lines
		FOR(i, samples.size())
		{
			fvec sample = samples[i];
			int label = labels[i];
			fvec newSample = classifier->Project(sample);

			QPointF point = canvas->toCanvasCoords(newSample)/canvas->width()*w;
			QPointF original = canvas->toCanvasCoords(samples[i])/canvas->width()*w;
			painter.setPen(QPen(QColor(0,0,0,128), 0.2));
			painter.drawLine(original, point);
		}

		// draw samples
		FOR(i, samples.size())
		{
			fvec sample = samples[i];
			int label = labels[i];
			fvec newSample = classifier->Project(sample);

			QPointF point = canvas->toCanvasCoords(newSample)/canvas->width()*w;
			QPointF original = canvas->toCanvasCoords(samples[i])/canvas->width()*w;
			Canvas::drawSample(painter, point, 6, label);
			Canvas::drawSample(painter, original, 6, label);
		}
	}
	if(classifier->GetType() == 3) // ICA
	{
		fvec meanAll; meanAll.resize(samples[0].size(), 0);
		FOR(i, samples.size())
		{
			meanAll += samples[i];
		}
		meanAll /= samples.size();

		// draw lines
		FOR(i, samples.size())
		{
			fvec sample = samples[i];
			int label = labels[i];
			fvec newSample = classifier->Project(sample);

			QPointF point = canvas->toCanvasCoords(newSample)/canvas->width()*w*0.5 + QPointF(w*0.5, 0);
			QPointF original = canvas->toCanvasCoords(samples[i]-meanAll)/canvas->width()*w*0.5;
			painter.setPen(QPen(QColor(0,0,0,40), 0.2));
			painter.drawLine(original, point);
		}

		// draw samples
		FOR(i, samples.size())
		{
			fvec sample = samples[i];
			int label = labels[i];
			fvec newSample = classifier->Project(sample);

			QPointF point = canvas->toCanvasCoords(newSample)/canvas->width()*w*0.5 + QPointF(w*0.5, 0);
			QPointF original = canvas->toCanvasCoords(samples[i]-meanAll)/canvas->width()*w*0.5;
			Canvas::drawSample(painter, point, 6, label);
			Canvas::drawSample(painter, original, 6, label);
		}

		/*
		fvec pt[4];
		fVec v[4];
		QPointF p1[4], p2[4];
		FOR(i,4) pt[i].resize(2,0);
		// we get the bounding box
		float xmin=FLT_MAX,xmax=-FLT_MAX,ymin=FLT_MAX,ymax=-FLT_MAX;
		FOR(i, canvas->data->GetCount())
		{
			fvec sample = classifier->Project(samples[i]);
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
			fvec proj = classifier->InvProject(pt[i]);
			v[i] = fVec(proj[0], proj[1]);
			p2[i] = canvas->toCanvasCoords(v[i].x,v[i].y);
		}
		FOR(i, 4)
		{
			p1[i] = p1[i]/canvas->width()*w*0.5 + QPointF(w*0.5, 0);
			p2[i] = p2[i]/canvas->width()*w*0.5;
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
	  */
		painter.setPen(QPen(Qt::black, 2.f));
		painter.drawLine(QPointF(w*0.5f, 0), QPointF(w*0.5f, h));
	}

	if(!projectionWindow) projectionWindow = new QLabel("Linear Projection");
	projectionWindow->resize(projectionPixmap.size());
	projectionWindow->setPixmap(projectionPixmap);
	projectionWindow->show();
}

void ClassLinear::SendToCanvas()
{
	if(!canvas) return;

	if(bDataIsFromCanvas)
	{
		FOR(i, data.size()) canvas->data->SetSample(i, data[i]);
		params->toCanvasButton->setText("Set Projection");
		bDataIsFromCanvas = false;
	}
	else
	{
		if(!classifier) return;
		data = canvas->data->GetSamples();
		FOR(i, data.size())
		{
			fvec projected = classifier->Project(data[i]);
			canvas->data->SetSample(i, projected);
		}
		bDataIsFromCanvas = true;
		params->toCanvasButton->setText("Set Original");
	}
	canvas->modelPixmap = QPixmap();
	canvas->confidencePixmap = QPixmap();
	canvas->infoPixmap = QPixmap();
	canvas->ResetSamples();
	canvas->repaint();
}
