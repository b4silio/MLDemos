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
#include "interfaceGBRegress.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>

using namespace std;

RegrGB::RegrGB()
{
        params = new Ui::ParametersGBRegress();
	params->setupUi(widget = new QWidget());

}

void RegrGB::SetParams(Regressor *regressor)
{
    if(!regressor) return;
    // hHH ere we gather the different hyperparameters from the interface
    int boostIters = params->boostIters->value();
    int boostLossType = params->boostLossType->currentIndex()+1;
    int boostTreeDepths = params->boostTreeDepths->value();

    ((RegressorGB *)regressor)->SetParams(boostIters,boostLossType,boostTreeDepths);
}

QString RegrGB::GetAlgoString()
{
    int boostIters = params->boostIters->value();
    int boostLossType = params->boostLossType->currentIndex()+1;
    int boostTreeDepths = params->boostTreeDepths->value();

    QString algo = QString("MyExample %1 %2 %3").arg(boostIters).arg(boostLossType).arg(boostTreeDepths);
    return algo;
}

Regressor *RegrGB::GetRegressor()
{
    RegressorGB *regressor = new RegressorGB();
	SetParams(regressor);
	return regressor;
}

void RegrGB::DrawConfidence(Canvas *canvas, Regressor *regressor)
{
	canvas->maps.confidence = QPixmap();
}

void RegrGB::DrawModel(Canvas *canvas, QPainter &painter, Regressor *regressor)
{
	if(!regressor || !canvas) return;
	painter.setRenderHint(QPainter::Antialiasing, true);
    int xIndex = canvas->xIndex;

    int w = canvas->width();
    fvec sample = canvas->toSampleCoords(0,0);
    int dim = sample.size();
    if(dim > 2) return;
	int steps = w;
	QPointF oldPoint(-FLT_MAX,-FLT_MAX);
	FOR(x, steps)
	{
        sample = canvas->toSampleCoords(x,0);
		fvec res = regressor->Test(sample);
        if(res[0] != res[0]) continue; // NaN!
        QPointF point = canvas->toCanvasCoords(sample[xIndex], res[0]);
		if(x)
		{
			painter.setPen(QPen(Qt::black, 1));
			painter.drawLine(point, oldPoint);
		}
		oldPoint = point;
	}
}

void RegrGB::SaveOptions(QSettings &settings)
{
       // HH we save to the system registry each parameter value
        settings.setValue("boostIters", params->boostIters->value());
        settings.setValue("boostLossType", params->boostLossType->currentIndex());
        settings.setValue("boostTreeDepths", params->boostTreeDepths->value());
}

bool RegrGB::LoadOptions(QSettings &settings)
{
        // HH we load the parameters from the registry so that when we launch the program we keep all values
        if(settings.contains("boostIters")) params->boostIters->setValue(settings.value("boostIters").toInt());
        if(settings.contains("boostLossType")) params->boostLossType->setCurrentIndex(settings.value("boostLossType").toInt());
        if(settings.contains("boostTreeDepths")) params->boostTreeDepths->setValue(settings.value("boostTreeDepths").toInt());
        return true;
}


void RegrGB::SaveParams(QTextStream &file)
{
        // HH same as above but for files/string saving
        file << "regressionOptions" << ":" << "boostIters" << " " << params->boostIters->value() << "\n";
        file << "regressionOptions" << ":" << "boostLossType" << " " << params->boostLossType->currentIndex() << "\n";
        file << "regressionOptions" << ":" << "boostTreeDepths" << " " << params->boostTreeDepths->value() << "\n";
}

bool RegrGB::LoadParams(QString name, float value)
{
        // HH
        if(name.endsWith("boostIters")) params->boostIters->setValue((int)value);
        if(name.endsWith("boostLossType")) params->boostLossType->setCurrentIndex((int)value);
        if(name.endsWith("boostTreeDepths")) params->boostTreeDepths->setValue((int)value);
        return true;
}

