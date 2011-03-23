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

Classifier *ClassGMM::GetClassifier()
{
	ClassifierGMM *classifier = new ClassifierGMM();
	SetParams(classifier);
	return classifier;
}

void ClassGMM::DrawInfo(Canvas *canvas, Classifier *classifier)
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

	ClassifierGMM * gmm = (ClassifierGMM*)classifier;
	Gmm *gmmPos = gmm->gmmPos;
	Gmm *gmmNeg = gmm->gmmNeg;
	float mean[2];
	float sigma[4];
	FOR(i, gmmPos->nstates)
	{
		gmmPos->getMean(i, mean);
		gmmPos->getCovariance(i, sigma, true);
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
	FOR(i, gmmNeg->nstates)
	{
		gmmNeg->getMean(i, mean);
		gmmNeg->getCovariance(i, sigma, true);
		painter.setPen(QPen(Qt::black, 1));
		DrawEllipse(mean, sigma, 1, &painter, canvas);
		painter.setPen(QPen(Qt::black, 0.5));
		DrawEllipse(mean, sigma, 2, &painter, canvas);
		//FOR(j,4) sigma[j] = sqrt(sigma[j]);
		QPointF point = canvas->toCanvasCoords(mean[0],mean[1]);
		painter.setPen(QPen(Qt::black, 4));
		painter.drawEllipse(point, 2, 2);
		painter.setPen(QPen(Qt::white, 2));
		painter.drawEllipse(point, 2, 2);
	}
	canvas->infoPixmap = infoPixmap;
}

void ClassGMM::Draw(Canvas *canvas, Classifier *classifier)
{
	if(!classifier || !canvas) return;
	canvas->liveTrajectory.clear();
	int w = canvas->width();
	int h = canvas->height();

	int posClass = 1;

	canvas->modelPixmap = QPixmap();

	DrawInfo(canvas, classifier);

	// we draw the samples
	canvas->modelPixmap = QPixmap(w,h);
	QBitmap bitmap(w,h);
	bitmap.clear();
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

	canvas->repaint();
	canvas->confidencePixmap = QPixmap();
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

void ClassGMM::SaveParams(std::ofstream &file)
{
	file << "classificationOptions" << ":" << "gmmCount" << " " << params->gmmCount->value() << std::endl;
	file << "classificationOptions" << ":" << "gmmCovariance" << " " << params->gmmCovarianceCombo->currentIndex() << std::endl;
	file << "classificationOptions" << ":" << "gmmInit" << " " << params->gmmInitCombo->currentIndex() << std::endl;
}

bool ClassGMM::LoadParams(char *line, float value)
{
	if(endsWith(line,"gmmCount")) params->gmmCount->setValue((int)value);
	if(endsWith(line,"gmmCovariance")) params->gmmCovarianceCombo->setCurrentIndex((int)value);
	if(endsWith(line,"gmmInit")) params->gmmInitCombo->setCurrentIndex((int)value);
	return true;
}
