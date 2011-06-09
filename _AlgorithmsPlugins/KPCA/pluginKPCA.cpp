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
#include "pluginKPCA.h"
#include "classifierKPCA.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>
#include <QDebug>

using namespace std;

ClassKPCA::ClassKPCA()
{
	params = new Ui::ParametersKPCA();
	params->setupUi(widget = new QWidget());
	connect(params->kernelTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeOptions()));
}


void ClassKPCA::ChangeOptions()
{
	switch(params->kernelTypeCombo->currentIndex())
	{
	case 0: // linear
		params->kernelDegSpin->setEnabled(false);
		params->kernelWidthSpin->setEnabled(false);
		break;
	case 1: // poly
		params->kernelDegSpin->setEnabled(true);
		params->kernelWidthSpin->setEnabled(false);
		break;
	case 2: // rbf
		params->kernelDegSpin->setEnabled(false);
		params->kernelWidthSpin->setEnabled(true);
		break;
	}
}


void ClassKPCA::SetParams(Classifier *classifier)
{
	if(!classifier) return;
	int kernelType = params->kernelTypeCombo->currentIndex();
	float kernelGamma = params->kernelWidthSpin->value();
	int kernelDegree = params->kernelDegSpin->value();
	((ClassifierKPCA *)classifier)->SetParams(kernelType, kernelDegree, kernelGamma);
}

QString ClassKPCA::GetAlgoString()
{
	int kernelType = params->kernelTypeCombo->currentIndex();
	float kernelGamma = params->kernelWidthSpin->value();
	int kernelDegree = params->kernelDegSpin->value();
	QString algo = "KPCA";
	switch(kernelType)
	{
	case 0:
		algo += " L";
		break;
	case 1:
		algo += QString(" P %1").arg(kernelDegree);
		break;
	case 2:
		algo += QString(" R %1").arg(kernelGamma);
		break;
	}
	return algo;
}

Classifier *ClassKPCA::GetClassifier()
{
	ClassifierKPCA *classifier = new ClassifierKPCA();
	SetParams(classifier);
	return classifier;
}

void ClassKPCA::DrawModel(Canvas *canvas, QPainter &painter, Classifier *classifier)
{
	if(!classifier || !canvas) return;
	int w = canvas->width();
	int h = canvas->height();
	painter.setRenderHint(QPainter::Antialiasing);

	ClassifierKPCA *kpca = (ClassifierKPCA*)classifier;

	vector<fvec> results = kpca->GetResults();
	ivec labels = kpca->GetLabels();

	vector<fvec> samples = kpca->GetSamples();
	painter.setPen(QPen(Qt::black, 0.2));
	FOR(i, samples.size())
	{
		QPointF pt1 = canvas->toCanvasCoords(samples[i]);
		QPointF pt2 = canvas->toCanvasCoords(results[i]);
		painter.drawLine(pt1, pt2);
	}

	FOR(i, results.size())
	{
		QPointF point = canvas->toCanvasCoords(results[i]);
		Canvas::drawSample(painter, point, 6, labels[i] == 1);
	}
}

void ClassKPCA::SaveOptions(QSettings &settings)
{
	settings.setValue("kernelDeg", params->kernelDegSpin->value());
	settings.setValue("kernelType", params->kernelTypeCombo->currentIndex());
	settings.setValue("kernelWidth", params->kernelWidthSpin->value());
}

bool ClassKPCA::LoadOptions(QSettings &settings)
{
	if(settings.contains("kernelDeg")) params->kernelDegSpin->setValue(settings.value("kernelDeg").toFloat());
	if(settings.contains("kernelType")) params->kernelTypeCombo->setCurrentIndex(settings.value("kernelType").toInt());
	if(settings.contains("kernelWidth")) params->kernelWidthSpin->setValue(settings.value("kernelWidth").toFloat());
	return true;
}

void ClassKPCA::SaveParams(QTextStream &file)
{
	file << "classificationOptions" << ":" << "kernelDeg" << " " << params->kernelDegSpin->value() << "\n";
	file << "classificationOptions" << ":" << "kernelType" << " " << params->kernelTypeCombo->currentIndex() << "\n";
	file << "classificationOptions" << ":" << "kernelWidth" << " " << params->kernelWidthSpin->value() << "\n";
}

bool ClassKPCA::LoadParams(QString name, float value)
{
	if(name.endsWith("kernelDeg")) params->kernelDegSpin->setValue((int)value);
	if(name.endsWith("kernelType")) params->kernelTypeCombo->setCurrentIndex((int)value);
	if(name.endsWith("kernelWidth")) params->kernelWidthSpin->setValue(value);
	return true;
}


Q_EXPORT_PLUGIN2(mld_KPCA, ClassKPCA)
