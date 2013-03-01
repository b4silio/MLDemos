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
#include "interfaceLWPRDynamic.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>

using namespace std;

DynamicLWPR::DynamicLWPR()
{
	params = new Ui::ParametersLWPRDynamic();
	params->setupUi(widget = new QWidget());
}

DynamicLWPR::~DynamicLWPR()
{
    delete params;
}

void DynamicLWPR::SetParams(Dynamical *dynamical)
{
	if(!dynamical) return;
	float gen = params->lwprGenSpin->value();
	float delta = params->lwprInitialDSpin->value();
	float alpha = params->lwprAlphaSpin->value();

	((DynamicalLWPR *)dynamical)->SetParams(delta, alpha, gen);
}

fvec DynamicLWPR::GetParams()
{
    fvec par(3);
    par[0] = params->lwprGenSpin->value();
    par[1] = params->lwprInitialDSpin->value();
    par[2] = params->lwprAlphaSpin->value();
    return par;
}

void DynamicLWPR::SetParams(Dynamical *dynamical, fvec parameters)
{
    if(!dynamical) return;
    int gen = parameters.size() > 0 ? parameters[0] : 1;
    int delta = parameters.size() > 1 ? parameters[1] : 0;
    int alpha = parameters.size() > 2 ? parameters[2] : 0;
    ((DynamicalLWPR *)dynamical)->SetParams(delta, alpha, gen);
}

void DynamicLWPR::GetParameterList(std::vector<QString> &parameterNames,
                             std::vector<QString> &parameterTypes,
                             std::vector< std::vector<QString> > &parameterValues)
{
    parameterNames.clear();
    parameterTypes.clear();
    parameterValues.clear();
    parameterNames.push_back("Generation Threshold");
    parameterNames.push_back("Initial Receptive Field Width");
    parameterNames.push_back("Alpha Learning Rate");
    parameterTypes.push_back("Real");
    parameterTypes.push_back("Real");
    parameterTypes.push_back("Real");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("0.0000001f");
    parameterValues.back().push_back("9999999");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("0.0000001f");
    parameterValues.back().push_back("9999999");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("0.0000001f");
    parameterValues.back().push_back("9999999");
}

Dynamical *DynamicLWPR::GetDynamical()
{
	DynamicalLWPR *dynamical = new DynamicalLWPR();
	SetParams(dynamical);
	return dynamical;
}

void DynamicLWPR::DrawInfo(Canvas *canvas, QPainter &painter, Dynamical *dynamical)
{
	if(!canvas || !dynamical) return;
	painter.setRenderHint(QPainter::Antialiasing, true);

	DynamicalLWPR* _lwpr = (DynamicalLWPR*)dynamical;
	LWPR_Object *lwpr= _lwpr->GetModel();
	painter.setBrush(Qt::NoBrush);
	FOR(i, lwpr->numRFS()[0])
	{
		LWPR_ReceptiveFieldObject rf = lwpr->getRF(0,i);
		double var = sqrt(rf.varX()[0]);
		var = fabs((canvas->toCanvasCoords(0,0) - canvas->toCanvasCoords(var,0)).x());
		double centerX = rf.center()[0];
		double centerY = rf.center()[1];
		double radius = rf.D()[0][0];
		QPointF slope(rf.slope()[0],rf.slope()[1]);
		slope = slope / sqrt(slope.x()*slope.x() + slope.y()*slope.y());
		QPointF point = canvas->toCanvasCoords(centerX, centerY);
		QPointF pointDiff = canvas->toCanvasCoords(centerX + slope.x(), centerY + slope.y());
		pointDiff = pointDiff - point;
		pointDiff /= sqrtf(pointDiff.x()*pointDiff.x() + pointDiff.y()*pointDiff.y()); // normalized!
		painter.setPen(QPen(Qt::black, 4));
		painter.drawEllipse(point, 2, 2);
		painter.setPen(QPen(Qt::white, 2));
		painter.drawEllipse(point, 2, 2);
		painter.setPen(QPen(Qt::black, 1));
		painter.drawEllipse(point, var, var);
		painter.drawLine(point - pointDiff*var*2,point + pointDiff*var*2);
	}
}

void DynamicLWPR::SaveOptions(QSettings &settings)
{
	settings.setValue("lwprAlpha", params->lwprAlphaSpin->value());
	settings.setValue("lwprInitialD", params->lwprInitialDSpin->value());
	settings.setValue("lwprGen", params->lwprGenSpin->value());
}

bool DynamicLWPR::LoadOptions(QSettings &settings)
{
	if(settings.contains("lwprAlpha")) params->lwprAlphaSpin->setValue(settings.value("lwprAlpha").toFloat());
	if(settings.contains("lwprInitialD")) params->lwprInitialDSpin->setValue(settings.value("lwprInitialD").toFloat());
	if(settings.contains("lwprGen")) params->lwprGenSpin->setValue(settings.value("lwprGen").toFloat());
	return true;
}

void DynamicLWPR::SaveParams(QTextStream &file)
{
	file << "dynamicalOptions" << ":" << "lwprAlpha" << " " << params->lwprAlphaSpin->value() << "\n";
	file << "dynamicalOptions" << ":" << "lwprInitialD" << " " << params->lwprInitialDSpin->value() << "\n";
	file << "dynamicalOptions" << ":" << "lwprGen" << " " << params->lwprGenSpin->value() << "\n";
}

bool DynamicLWPR::LoadParams(QString name, float value)
{
	if(name.endsWith("lwprAlpha")) params->lwprAlphaSpin->setValue(value);
	if(name.endsWith("lwprInitialD")) params->lwprInitialDSpin->setValue(value);
	if(name.endsWith("lwprGen")) params->lwprGenSpin->setValue(value);
	return true;
}
