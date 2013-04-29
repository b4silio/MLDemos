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
#ifndef _INTERFACEASVMDYNAMIC_H_
#define _INTERFACEASVMDYNAMIC_H_

#include <vector>
#include <interfaces.h>
#include "dynamicalASVM.h"
#include "ui_paramsASVM.h"

class DynamicASVM : public QObject, public DynamicalInterface
{
	Q_OBJECT
    Q_INTERFACES(DynamicalInterface)
    Q_PLUGIN_METADATA(IID "DynamicASVM" FILE "plugin.json")

private:
	QWidget *widget;
    Ui::ParametersASVM *params;
public:
    DynamicASVM();
    ~DynamicASVM();
	// virtual functions to manage the algorithm creation
	Dynamical *GetDynamical();
	void DrawInfo(Canvas *canvas, QPainter &painter, Dynamical *dynamical);
    void DrawModel(Canvas *canvas, QPainter &painter, Dynamical *dynamical){}
    void DrawGL(Canvas *canvas, GLWidget *glw, Dynamical *dynamical){}
    void SaveModel(QString filename, Dynamical *dynamical);
    bool LoadModel(QString filename, Dynamical *dynamical);

    // virtual functions to manage the GUI and I/O
    QString GetName(){return QString("ASVM");}
    QString GetAlgoString(){return GetName();}
    QString GetInfoFile(){return "asvm.html";}
    bool UsesDrawTimer(){return true;}
    QWidget *GetParameterWidget(){return widget;}
	void SetParams(Dynamical *dynamical);
	void SaveOptions(QSettings &settings);
	bool LoadOptions(QSettings &settings);
	void SaveParams(QTextStream &stream);
	bool LoadParams(QString name, float value);
    void SetParams(Dynamical *dynamical, fvec parameters);
    fvec GetParams();
    void GetParameterList(std::vector<QString> &parameterNames,
                                 std::vector<QString> &parameterTypes,
                                 std::vector< std::vector<QString> > &parameterValues);
};

#endif // _INTERFACEASVMDYNAMIC_H_
