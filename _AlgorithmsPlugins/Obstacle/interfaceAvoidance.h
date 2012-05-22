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
#ifndef _INTERFACEDSAVOID_H_
#define _INTERFACEDSAVOID_H_

#include <vector>
#include <interfaces.h>
#include "DSAvoid.h"

class PluginAvoid : public QObject, public AvoidanceInterface
{
	Q_OBJECT
	Q_INTERFACES(AvoidanceInterface)
private:
	QWidget *widget;
public:
	PluginAvoid();
	// virtual functions to manage the algorithm creation
	ObstacleAvoidance  *GetObstacleAvoidance();

	// virtual functions to manage the GUI and I/O
    QString GetName(){return QString("DS Avoid");}
    QString GetAlgoString(){return GetName();}
    QString GetInfoFile(){return "avoidDS.html";}
    QWidget *GetParameterWidget(){return widget;}
	void SetParams(ObstacleAvoidance *avoid);
	void SaveOptions(QSettings &settings);
	bool LoadOptions(QSettings &settings);
	void SaveParams(QTextStream &stream);
	bool LoadParams(QString name, float value);
};

#endif // _INTERFACEDSAVOID_H_
