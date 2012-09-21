/*********************************************************************
MLDemos: A User-Friendly visualization toolkit for machine learning
Copyright (C) 2010  Basilio Noris
Contact: mldemos@b4silio.com

Evolution-Strategy Mixture of Logisitics Regression
Copyright (C) 2011  Stephane Magnenat
Contact: stephane at magnenat dot net

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
#ifndef _INTERFACE_ESMLR_CLASSIFIER_H_
#define _INTERFACE_ESMLR_CLASSIFIER_H_

#include "interfaces.h"

namespace Ui
{
	class ParametersESMLR;
}

class ClassESMLR: public QObject, public ClassifierInterface
{
	Q_OBJECT
	Q_INTERFACES(ClassifierInterface)
private:
	QWidget *widget;
	Ui::ParametersESMLR *params;
public:
	ClassESMLR();
	// virtual functions to manage the algorithm creation
	Classifier *GetClassifier();
	void DrawInfo(Canvas *canvas, QPainter &painter, Classifier *classifier){};
	void DrawModel(Canvas *canvas, QPainter &painter, Classifier *classifier);
	
	// virtual functions to manage the GUI and I/O
	QString GetName(){return QString("ESMLR");};
	QString GetAlgoString();
	QString GetInfoFile(){return "esmlr.html";};
	bool UsesDrawTimer(){return true;};
	QWidget *GetParameterWidget(){return widget;};
	void SetParams(Classifier *classifier);
	void SaveOptions(QSettings &settings);
	bool LoadOptions(QSettings &settings);
	void SaveParams(QTextStream &stream);
	bool LoadParams(QString name, float value);
};

#endif // _INTERFACE_ESMLR_CLASSIFIER_H_
