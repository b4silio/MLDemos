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
#ifndef _INTERFACE_MAXIMIZERGA_H_
#define _INTERFACE_MAXIMIZERGA_H_

#include <vector>
#include <interfaces.h>
#include "ui_paramsGA.h"
#include <QDir>

class MaximizeInterfaceGA : public QObject, public MaximizeInterface
{
	Q_OBJECT
	Q_INTERFACES(MaximizeInterface)
private:
	QWidget *widget;
	Ui::ParametersGA *params;
public:
	MaximizeInterfaceGA();
	// virtual functions to manage the algorithm creation
	Maximizer *GetMaximizer();

	// virtual functions to manage the GUI and I/O
    QString GetName(){return QString("Genetic Algorithms");}
	QString GetAlgoString();
    QString GetInfoFile(){return "GA.html";}
    QWidget *GetParameterWidget(){return widget;}
	void SetParams(Maximizer *maximizer);
	void SaveOptions(QSettings &settings);
	bool LoadOptions(QSettings &settings);
	void SaveParams(QTextStream &stream);
	bool LoadParams(QString name, float value);
};

#endif // _INTERFACE_MAXIMIZERGA_H_
