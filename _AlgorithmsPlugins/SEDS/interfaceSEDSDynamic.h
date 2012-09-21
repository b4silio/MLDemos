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
#ifndef _INTERFACESEDSRDYNAMIC_H_
#define _INTERFACESEDSRDYNAMIC_H_

#include <vector>
#include <interfaces.h>
#include "dynamicalSEDS.h"
#include "ui_paramsSEDS.h"

class DynamicSEDS : public QObject, public DynamicalInterface
{
	Q_OBJECT
	Q_INTERFACES(DynamicalInterface)
private:
	QWidget *widget;
	Ui::ParametersSEDS *params;
public:
	DynamicSEDS();
	// virtual functions to manage the algorithm creation
	Dynamical *GetDynamical();
	void DrawInfo(Canvas *canvas, QPainter &painter, Dynamical *dynamical);
    void DrawModel(Canvas *canvas, QPainter &painter, Dynamical *dynamical){}
    void SaveModel(QString filename, Dynamical *dynamical);
    bool LoadModel(QString filename, Dynamical *dynamical);

    // virtual functions to manage the GUI and I/O
    QString GetName(){return QString("SEDS");}
    QString GetAlgoString(){return GetName();}
    QString GetInfoFile(){return "seds.html";}
    bool UsesDrawTimer(){return true;}
    QWidget *GetParameterWidget(){return widget;}
	void SetParams(Dynamical *dynamical);
	void SaveOptions(QSettings &settings);
	bool LoadOptions(QSettings &settings);
	void SaveParams(QTextStream &stream);
	bool LoadParams(QString name, float value);
public slots:
	void OptionsChanged();
};

#endif // _INTERFACESEDSRDYNAMIC_H_
