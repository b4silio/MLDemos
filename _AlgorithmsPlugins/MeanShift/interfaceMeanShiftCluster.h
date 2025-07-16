/*********************************************************************
FLAME Implementation in MLDemos
Copyright (C) Pierre-Antoine Sondag (pasondag@gmail.com) 2012

Based on the standard implementation of FLAME data clustering algorithm.
Copyright (C) 2007, Fu Limin (phoolimin@gmail.com).
All rights reserved.

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
#ifndef INTERFACEFLAMECLUSTER_H
#define INTERFACEFLAMECLUSTER_H

#include <vector>
#include <interfaces.h>
#include "clustererMeanShift.h"
#include "ui_paramsMeanShift.h"

/**
  Example of plugin interface for a clustering algorithm
  */
class ClustMeanShift : public QObject, public ClustererInterface
{
        Q_OBJECT
        Q_INTERFACES(ClustererInterface)
private:
    QWidget *widget;
    Ui::ParametersMeanShift *params;
public:
    ClustMeanShift();
    ~ClustMeanShift();

    Clusterer *GetClusterer();
    void DrawInfo(Canvas *canvas, QPainter &painter, Clusterer *clusterer);
    void DrawModel(Canvas *canvas, QPainter &painter, Clusterer *clusterer);
    void DrawGL(Canvas *canvas, GLWidget *glw, Clusterer *clusterer){}
    QString GetName(){return QString("MeanShift");}
    QString GetAlgoString(){ return GetName();}
    QString GetInfoFile(){return "MeanShift.html";}
    QWidget *GetParameterWidget(){return widget;}
    void SetParams(Clusterer *clusterer);
    void SaveOptions(QSettings &settings);
    bool LoadOptions(QSettings &settings);
    void SaveParams(QTextStream &stream);
    bool LoadParams(QString name, float value);

    void SetParams(Clusterer *clusterer, fvec parameters);
    fvec GetParams();
    void GetParameterList(std::vector<QString> &parameterNames,
                                 std::vector<QString> &parameterTypes,
                                 std::vector< std::vector<QString> > &parameterValues);
};

#endif // INTERFACEFLAMECLUSTER_H
