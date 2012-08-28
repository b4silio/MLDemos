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
#include "interfaceGMMClassifier.h"
#include "drawUtils.h"
#include <basicMath.h>
#include <QPixmap>
#include <QBitmap>
#include <QPainter>
#include <QDebug>

using namespace std;

ClassGMM::ClassGMM()
{
	params = new Ui::ParametersGMM();
	params->setupUi(widget = new QWidget());
}

void ClassGMM::SetParams(Classifier *classifier)
{
	if(!classifier) return;
	int clusters = params->gmmCount->value();
    int covType = params->gmmCovarianceCombo->currentIndex();
	int initType = params->gmmInitCombo->currentIndex();

	((ClassifierGMM *)classifier)->SetParams(clusters, covType, initType);
}

QString ClassGMM::GetAlgoString()
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

Classifier *ClassGMM::GetClassifier()
{
	ClassifierGMM *classifier = new ClassifierGMM();
	SetParams(classifier);
	return classifier;
}

void ClassGMM::DrawInfo(Canvas *canvas, QPainter &painter, Classifier *classifier)
{
	if(!canvas || !classifier) return;
	painter.setRenderHint(QPainter::Antialiasing);

	ClassifierGMM * gmm = (ClassifierGMM*)classifier;
	vector<Gmm*> gmms = gmm->gmms;
	if(!gmms.size()) return;
	int xIndex = canvas->xIndex;
	int yIndex = canvas->yIndex;
	int dim = gmms[0]->dim;
	float mean[2];
	float sigma[3];
	painter.setBrush(Qt::NoBrush);
	FOR(g, gmms.size())
	{
		FOR(i, gmms[g]->nstates)
		{
			float* bigSigma = new float[dim*dim];
			float* bigMean = new float[dim];
			gmms[g]->getCovariance(i, bigSigma, false);
			sigma[0] = bigSigma[xIndex*dim + xIndex];
			sigma[1] = bigSigma[yIndex*dim + xIndex];
			sigma[2] = bigSigma[yIndex*dim + yIndex];
			gmms[g]->getMean(i, bigMean);
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
            QColor color = SampleColor[classifier->inverseMap[g]%SampleColorCnt];
			painter.setPen(QPen(Qt::black, 12));
			painter.drawEllipse(point, 6, 6);
			painter.setPen(QPen(color,4));
			painter.drawEllipse(point, 6, 6);
		}
	}
}

void ClassGMM::DrawModel(Canvas *canvas, QPainter &painter, Classifier *classifier)
{
	int posClass = 1;
	// we draw the samples
	painter.setRenderHint(QPainter::Antialiasing, true);
	FOR(i, canvas->data->GetCount())
	{
		fvec sample = canvas->data->GetSample(i);
		int label = canvas->data->GetLabel(i);
		QPointF point = canvas->toCanvasCoords(canvas->data->GetSample(i));
		fvec res = classifier->TestMulti(sample);
		if(res.size()==1)
		{
			float response = res[0];
			if(response > 0)
			{
                if(classifier->classMap[label] == posClass) Canvas::drawSample(painter, point, 9, 1);
				else Canvas::drawCross(painter, point, 6, 2);
			}
			else
			{
                if(classifier->classMap[label] != posClass) Canvas::drawSample(painter, point, 9, 0);
				else Canvas::drawCross(painter, point, 6, 0);
			}
		}
		else
		{
			int max = 0;
			for(int i=1; i<res.size(); i++) if(res[max] < res[i]) max = i;
            int resp = classifier->inverseMap[max];
			if(label == resp) Canvas::drawSample(painter, point, 9, label);
			else Canvas::drawCross(painter, point, 6, label);
		}
	}
}

void ClassGMM::SaveOptions(QSettings &settings)
{
	settings.setValue("gmmCount", params->gmmCount->value());
	settings.setValue("gmmCovariance", params->gmmCovarianceCombo->currentIndex());
	settings.setValue("gmmInit", params->gmmInitCombo->currentIndex());
}

bool ClassGMM::LoadOptions(QSettings &settings)
{
	if(settings.contains("gmmCount")) params->gmmCount->setValue(settings.value("gmmCount").toFloat());
	if(settings.contains("gmmCovariance")) params->gmmCovarianceCombo->setCurrentIndex(settings.value("gmmCovariance").toInt());
	if(settings.contains("gmmInit")) params->gmmInitCombo->setCurrentIndex(settings.value("gmmInit").toInt());
	return true;
}

void ClassGMM::SaveParams(QTextStream &file)
{
	file << "classificationOptions" << ":" << "gmmCount" << " " << params->gmmCount->value() << "\n";
	file << "classificationOptions" << ":" << "gmmCovariance" << " " << params->gmmCovarianceCombo->currentIndex() << "\n";
	file << "classificationOptions" << ":" << "gmmInit" << " " << params->gmmInitCombo->currentIndex() << "\n";
}

bool ClassGMM::LoadParams(QString name, float value)
{
	if(name.endsWith("gmmCount")) params->gmmCount->setValue((int)value);
	if(name.endsWith("gmmCovariance")) params->gmmCovarianceCombo->setCurrentIndex((int)value);
	if(name.endsWith("gmmInit")) params->gmmInitCombo->setCurrentIndex((int)value);
	return true;
}
