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
#include "interfaceSEDSDynamic.h"
#include "drawUtils.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>

using namespace std;

DynamicSEDS::DynamicSEDS()
{
	params = new Ui::ParametersSEDS();
	params->setupUi(widget = new QWidget());
}

void DynamicSEDS::SetParams(Dynamical *dynamical)
{
	if(!dynamical) return;
	int clusters = params->sedsCount->value();
	float penalty = params->sedsPenaltySpin->value();
	bool bPrior = params->sedsCheckPrior->isChecked();
	bool bMu = params->sedsCheckMu->isChecked();
	bool bSigma = params->sedsCheckSigma->isChecked();
	int objectiveType = params->sedsObjectiveCombo->currentIndex();

	((DynamicalSEDS *)dynamical)->SetParams(clusters, penalty, bPrior, bMu, bSigma, objectiveType);
}

Dynamical *DynamicSEDS::GetDynamical()
{
	DynamicalSEDS *dynamical = new DynamicalSEDS();
	SetParams(dynamical);
	return dynamical;
}

void DynamicSEDS::DrawInfo(Canvas *canvas, Dynamical *dynamical)
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

	Gmm *gmm = ((DynamicalSEDS*)dynamical)->gmm;
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

		fvec endpoint = ((DynamicalSEDS*)dynamical)->endpoint;
		FOR(j,2) drawMean[j] = drawMean[j]/1000.f + endpoint[j];
		FOR(j,3) drawSigma[j] /= 1000000;

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

void DynamicSEDS::Draw(Canvas *canvas, Dynamical *dynamical)
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

void DynamicSEDS::SaveOptions(QSettings &settings)
{
	settings.setValue("sedsCount", params->sedsCount->value());
	settings.setValue("sedsPenalty", params->sedsPenaltySpin->value());
	settings.setValue("sedsObjective", params->sedsObjectiveCombo->currentIndex());
	settings.setValue("sedsPrior", params->sedsCheckPrior->isChecked());
	settings.setValue("sedsMu", params->sedsCheckMu->isChecked());
	settings.setValue("sedsSigma", params->sedsCheckSigma->isChecked());
}

bool DynamicSEDS::LoadOptions(QSettings &settings)
{
	if(settings.contains("sedsCount")) params->sedsCount->setValue(settings.value("sedsCount").toInt());
	if(settings.contains("sedsPenalty")) params->sedsPenaltySpin->setValue(settings.value("sedsPenalty").toFloat());
	if(settings.contains("sedsObjective")) params->sedsObjectiveCombo->setCurrentIndex(settings.value("sedsObjective").toInt());
	if(settings.contains("sedsPrior")) params->sedsCheckPrior->setChecked(settings.value("sedsPrior").toBool());
	if(settings.contains("sedsMu")) params->sedsCheckMu->setChecked(settings.value("sedsMu").toBool());
	if(settings.contains("sedsSigma")) params->sedsCheckSigma->setChecked(settings.value("sedsSigma").toBool());
	return true;
}

void DynamicSEDS::SaveParams(std::ofstream &file)
{
	file << "dynamicalOptions" << ":" << "sedsCount" << " " << params->sedsCount->value() << std::endl;
	file << "dynamicalOptions" << ":" << "sedsPenalty" << " " << params->sedsPenaltySpin->value() << std::endl;
	file << "dynamicalOptions" << ":" << "sedsObjective" << " " << params->sedsObjectiveCombo->currentIndex() << std::endl;
	file << "dynamicalOptions" << ":" << "sedsPrior" << " " << params->sedsCheckPrior->isChecked() << std::endl;
	file << "dynamicalOptions" << ":" << "sedsMu" << " " << params->sedsCheckMu->isChecked() << std::endl;
	file << "dynamicalOptions" << ":" << "sedsSigma" << " " << params->sedsCheckSigma->isChecked() << std::endl;
}

bool DynamicSEDS::LoadParams(char *line, float value)
{
	if(endsWith(line,"sedsCount")) params->sedsCount->setValue((int)value);
	if(endsWith(line,"sedsPenalty")) params->sedsPenaltySpin->setValue(value);
	if(endsWith(line,"sedsObjective")) params->sedsObjectiveCombo->setCurrentIndex((int)value);
	if(endsWith(line,"sedsPrior")) params->sedsCheckPrior->setChecked((int)value);
	if(endsWith(line,"sedsMu")) params->sedsCheckMu->setChecked((int)value);
	if(endsWith(line,"sedsSigma")) params->sedsCheckSigma->setChecked((int)value);
	return true;
}

Q_EXPORT_PLUGIN2(mld_SEDS, DynamicSEDS)
