/*********************************************************************
MeanShift Implementation in MLDemos
Copyright (C) Pierre-Antoine Sondag (pasondag@gmail.com) 2012

Based on the standard implementation of MeanShift data clustering algorithm.
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
#include "interfaceMeanShiftCluster.h"
#include "drawUtils.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>

using namespace std;

ClustMeanShift::ClustMeanShift()
{
    // we initialize the hyperparameter widget
    params = new Ui::ParametersMeanShift();
	params->setupUi(widget = new QWidget());
}

ClustMeanShift::~ClustMeanShift()
{
    delete params;
}

void ClustMeanShift::SetParams(Clusterer *clusterer)
{
    SetParams(clusterer, GetParams());
}

fvec ClustMeanShift::GetParams()
{
    float kernelWidth = (float)(params->kernelWidthSpin->value());
    float mergeRadius = (float)(params->mergeRadiusSpin->value());

    int i=0;
    fvec par(2);
    par[i++] = kernelWidth;
    par[i++] = mergeRadius;
    return par;
}

void ClustMeanShift::SetParams(Clusterer *clusterer, fvec parameters)
{
    if(!clusterer) return;
    ClustererMeanShift * clusterMS = dynamic_cast<ClustererMeanShift *>(clusterer);
    if(!clusterMS) return;

    int i=0;
    float kernelWidth = parameters.size() > i ? parameters[i] : 0; i++;
    float mergeRadius = parameters.size() > i ? parameters[i] : 0; i++;

    clusterMS->SetParams(kernelWidth, mergeRadius);
}

void ClustMeanShift::GetParameterList(std::vector<QString> &parameterNames,
                             std::vector<QString> &parameterTypes,
                             std::vector< std::vector<QString> > &parameterValues)
{
    parameterNames.clear();
    parameterTypes.clear();
    parameterValues.clear();
    parameterNames.push_back("Kernel Width");
    parameterNames.push_back("Merge Radius");
    parameterTypes.push_back("Real");
    parameterTypes.push_back("Real");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("0.001");
    parameterValues.back().push_back("99999");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("0.001");
    parameterValues.back().push_back("99999");
}

Clusterer *ClustMeanShift::GetClusterer() {
    // we instanciate the algorithm object
    ClustererMeanShift *clusterer = new ClustererMeanShift();
    // we set its parameters
    SetParams(clusterer);
    // we return it to the main program
	return clusterer;
}

void ClustMeanShift::DrawInfo(Canvas *canvas, QPainter &painter, Clusterer *clusterer) {
    if(!canvas || !clusterer) return;
    ClustererMeanShift* clusterMS = dynamic_cast<ClustererMeanShift*>(clusterer);
    if(!clusterMS) return;

    vector<MeanShiftCluster>& clusters = clusterMS->clusters;
    if(clusters.empty()) return;

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(Qt::red, 4));

    FOR(c, clusters.size()) {
        MeanShiftCluster& cluster = clusters.at(c);
        dvec& centerDouble = cluster.mode;
        fvec center(centerDouble.size());
        FOR(d, center.size()) center[d] = centerDouble[d];
        QPointF point = canvas->toCanvasCoords(center);
        painter.drawEllipse(point,9,9);
    }
}

void ClustMeanShift::DrawModel(Canvas *canvas, QPainter &painter, Clusterer *clusterer) {
    // we start by making the painter paint things nicely
    painter.setRenderHint(QPainter::Antialiasing);

    ClustererMeanShift* clusterMS = dynamic_cast<ClustererMeanShift*>(clusterer);
    if(!clusterMS) return;

    vector<MeanShiftCluster>& clusters = clusterMS->clusters;
    if(clusters.empty()) return;

    FOR(c, clusters.size()) {
        MeanShiftCluster& cluster = clusters.at(c);
        vector<dvec>& original = cluster.original_points;
        vector<dvec>& shifted = cluster.shifted_points;

        painter.setBrush(SampleColor[(c+1)%SampleColorCnt]);
        painter.setPen(Qt::black);

        FOR(i, original.size()) {
            dvec& aDouble = original.at(i);
            fvec a(aDouble.size());
            FOR(d, a.size()) a[d] = aDouble[d];

            dvec& bDouble = shifted.at(i);
            fvec b(bDouble.size());
            FOR(d, b.size()) b[d] = bDouble[d];

            QPointF pA = canvas->toCanvasCoords(a);
            QPointF pB = canvas->toCanvasCoords(b);
            painter.drawLine(pA, pB);
            painter.drawEllipse(pA,5,5);
            painter.drawEllipse(pB,5,5);
        }
    }
}

void ClustMeanShift::SaveOptions(QSettings &settings) {
    settings.setValue("kernelWidth", params->kernelWidthSpin->value());
    settings.setValue("mergeRadius", params->mergeRadiusSpin->value());
}

bool ClustMeanShift::LoadOptions(QSettings &settings) {
    if(settings.contains("kernelWidth")) params->kernelWidthSpin->setValue(settings.value("kernelWidth").toFloat());
    if(settings.contains("mergeRadius")) params->mergeRadiusSpin->setValue(settings.value("mergeRadius").toFloat());
    return true;
}

void ClustMeanShift::SaveParams(QTextStream &file) {
    file << "clusterOptions" << ":" << "kernelWidth" << " " << params->kernelWidthSpin->value() << "\n";
    file << "clusterOptions" << ":" << "mergeRadius" << " " << params->mergeRadiusSpin->value() << "\n";
}

bool ClustMeanShift::LoadParams(QString name, float value) {
    if(name.endsWith("kernelWidth"))  params->kernelWidthSpin->setValue(value);
    if(name.endsWith("mergeRadius"))  params->mergeRadiusSpin->setValue(value);
    return true;
}
