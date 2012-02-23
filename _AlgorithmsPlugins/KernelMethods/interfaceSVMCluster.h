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
#ifndef _INTERFACESVMCLUSTER_H_
#define _INTERFACESVMCLUSTER_H_

#include <vector>
#include <interfaces.h>
#include "clustererSVR.h"
#include "ui_paramsSVMcluster.h"

class ClustSVM : public QObject, public ClustererInterface
{
	Q_OBJECT
	Q_INTERFACES(ClustererInterface)
private:
	QWidget *widget;
	Ui::ParametersClust *params;
public:
	ClustSVM();
	// virtual functions to manage the algorithm creation
	Clusterer *GetClusterer();
	void DrawInfo(Canvas *canvas, QPainter &painter, Clusterer *clusterer);
	void DrawModel(Canvas *canvas, QPainter &painter, Clusterer *clusterer);

	// virtual functions to manage the GUI and I/O
        QString GetName(){return QString("SVM");}
        QString GetAlgoString(){return GetName();}
        QString GetInfoFile(){return "kernelClust.html";}
        QWidget *GetParameterWidget(){return widget;}
	void SetParams(Clusterer *clusterer);
	void SaveOptions(QSettings &settings);
	bool LoadOptions(QSettings &settings);
	void SaveParams(QTextStream &stream);
	bool LoadParams(QString name, float value);
public slots:
        void ChangeOptions();
};

#endif // _INTERFACESVMCLUSTER_H_
