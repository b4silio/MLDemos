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
#include "interfaceFlameCluster.h"
#include "drawUtils.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>

using namespace std;

ClustFlame::ClustFlame()
{
    // we initialize the hyperparameter widget
    params = new Ui::ParametersFlame();
	params->setupUi(widget = new QWidget());
}

ClustFlame::~ClustFlame()
{
    delete params;
}

void ClustFlame::SetParams(Clusterer *clusterer)
{
    SetParams(clusterer, GetParams());
}

fvec ClustFlame::GetParams()
{
    int knnParameter = (int)(params->knnBox->value());
    int knnMetric = (int)(params->metricBox->currentIndex());
    int maxIterationsParameter = (int)(params->maxIterationBox->value());
    bool isSeveralClasses = (bool)(params->isSeveralClassesBox->isChecked());
    float thresholdParameter = (float)(params->thresholdBox->value());

    int i=0;
    fvec par(5);
    par[i++] = knnParameter;
    par[i++] = knnMetric;
    par[i++] = maxIterationsParameter;
    par[i++] = isSeveralClasses;
    par[i++] = thresholdParameter;
    return par;
}

void ClustFlame::SetParams(Clusterer *clusterer, fvec parameters)
{
    if(!clusterer) return;
    ClustererFlame * myFlame = dynamic_cast<ClustererFlame *>(clusterer);
    if(!myFlame) return;

    int i=0;
    int knnParameter = parameters.size() > i ? parameters[i] : 0; i++;
    int knnMetric = parameters.size() > i ? parameters[i] : 0; i++;
    int maxIterationsParameter = parameters.size() > i ? parameters[i] : 0; i++;
    bool isSeveralClasses = parameters.size() > i ? parameters[i] : 0; i++;
    float thresholdParameter = parameters.size() > i ? parameters[i] : 0; i++;

    myFlame->SetParams(
        knnParameter, knnMetric,
        maxIterationsParameter,
        isSeveralClasses, thresholdParameter);
}

void ClustFlame::GetParameterList(std::vector<QString> &parameterNames,
                             std::vector<QString> &parameterTypes,
                             std::vector< std::vector<QString> > &parameterValues)
{
    parameterNames.clear();
    parameterTypes.clear();
    parameterValues.clear();
    parameterNames.push_back("KNN Parameter");
    parameterNames.push_back("KNN Metric");
    parameterNames.push_back("Max Iterations");
    parameterNames.push_back("Multiple Assignment");
    parameterNames.push_back("Multi-Assign. Thresh.");
    parameterTypes.push_back("Integer");
    parameterTypes.push_back("List");
    parameterTypes.push_back("Integer");
    parameterTypes.push_back("List");
    parameterTypes.push_back("Real");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("1");
    parameterValues.back().push_back("99999");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("Euclidean");
    parameterValues.back().push_back("Cosine");
    parameterValues.back().push_back("Pearson");
    parameterValues.back().push_back("UC Pearson");
    parameterValues.back().push_back("SQ Pearson");
    parameterValues.back().push_back("Dot Product");
    parameterValues.back().push_back("Covariance");
    parameterValues.back().push_back("Manhattan");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("1");
    parameterValues.back().push_back("9999999");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("False");
    parameterValues.back().push_back("True");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("0.f");
    parameterValues.back().push_back("1.f");
}

Clusterer *ClustFlame::GetClusterer() {
    // we instanciate the algorithm object
    ClustererFlame *clusterer = new ClustererFlame();
    // we set its parameters
    SetParams(clusterer);
    // we return it to the main program
	return clusterer;
}

void ClustFlame::DrawInfo(Canvas *canvas, QPainter &painter, Clusterer *clusterer) {
    if(!canvas || !clusterer) return;
    painter.setRenderHint(QPainter::Antialiasing);

    ClustererFlame * myFlame = dynamic_cast<ClustererFlame*>(clusterer);
    if(!myFlame) return;

    // Make the painter paint nicely (work well with forms, not so much with text).
    painter.setRenderHint(QPainter::Antialiasing);

    // Set the brush and pen, in our case no brush (hollow circle) and a thick red edge.
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(Qt::red, 4));

    // Draw a circle arround each support vector.
    vector<fvec> supportVectors = myFlame->GetSupports();
    int radius = 7;
    for (int i = 0; i < supportVectors.size(); i++) {
        /* Convert the sample coordinates from dataspace
        (N-dimensional in R) to canvas' coordinates. */
        QPointF pointInCanvas = canvas->toCanvasCoords(supportVectors[i]);

        painter.setPen(QPen(Qt::black, 4));
        painter.drawEllipse(pointInCanvas, radius, radius);
        painter.setPen(Qt::white);
        painter.drawEllipse(pointInCanvas, radius, radius);
    }
}

void ClustFlame::DrawModel(Canvas *canvas, QPainter &painter, Clusterer *clusterer) {
    // we start by making the painter paint things nicely
    painter.setRenderHint(QPainter::Antialiasing);

    // for every point in the current dataset
    FOR(i, canvas->data->GetSamples().size())
	{
        // we get the sample
        fvec sample = canvas->data->GetSample(i);

        // and we test it using the clusterer
        fvec res = clusterer->Test(sample);

        // we get the point in canvas coordinates (2D pixel by pixel) corresponding to the sample (N-dimensional in R)
        QPointF point = canvas->toCanvasCoords(sample);

        // we combine together the contribution of each cluster to color the sample properly
		float r=0,g=0,b=0;
		if(res.size() > 1)
		{
			FOR(j, res.size())
			{
				r += SampleColor[(j+1)%SampleColorCnt].red()*res[j];
				g += SampleColor[(j+1)%SampleColorCnt].green()*res[j];
				b += SampleColor[(j+1)%SampleColorCnt].blue()*res[j];
			}
		}
        // for one-class clustering
		else if(res.size())
		{
			r = (1-res[0])*255 + res[0]* 255;
			g = (1-res[0])*255;
			b = (1-res[0])*255;
		}
        // set the color of the brush and a black edge
		painter.setBrush(QColor(r,g,b));
		painter.setPen(Qt::black);
        // and draw the sample itself
		painter.drawEllipse(point,5,5);
	}
}

void ClustFlame::SaveOptions(QSettings &settings) {
    // we save to the system registry each parameter value
    settings.setValue("knn", params->knnBox->value());
    settings.setValue("knnMetric", params->metricBox->currentIndex());
    settings.setValue("maxIteration", params->maxIterationBox->value());
    settings.setValue("isSeveralClasses", params->isSeveralClassesBox->isChecked());
    settings.setValue("thresholdClasses", params->thresholdBox->value());
}

bool ClustFlame::LoadOptions(QSettings &settings) {
    // we load the parameters from the registry so that when we launch the program we keep all values
    if(settings.contains("knn")) params->knnBox->setValue(settings.value("knn").toInt());
    if(settings.contains("knnMetric")) params->metricBox->setCurrentIndex(settings.value("metric").toInt());
    if(settings.contains("maxIteration")) params->maxIterationBox->setValue(settings.value("maxIteration").toInt());
    if(settings.contains("isSeveralClasses")) params->isSeveralClassesBox->setChecked(settings.value("isSeveralClasses").toBool());
    if(settings.contains("thresholdClasses")) params->thresholdBox->setValue(settings.value("thresholdClasses").toFloat());
    return true;
}

void ClustFlame::SaveParams(QTextStream &file) {
    // same as above but for files/string saving
    file << "clusterOptions" << ":" << "knn" << " " << params->knnBox->value() << "\n";
    file << "clusterOptions" << ":" << "knnMetric" << " " <<  params->metricBox->currentIndex()<< "\n";
    file << "clusterOptions" << ":" << "maxIteration" << " " << params->maxIterationBox->value() << "\n";
    file << "clusterOptions" << ":" << "isSeveralClasses" << " " << params->isSeveralClassesBox->isChecked() << "\n";
    file << "clusterOptions" << ":" << "thresholdClasses" << " " << params->thresholdBox->value() << "\n";
}

bool ClustFlame::LoadParams(QString name, float value) {
    // same as above but for files/string saving
    if(name.endsWith("knn"))  params->knnBox->setValue((int)value);
    if(name.endsWith("knnMetric"))  params->metricBox->setCurrentIndex((int)value);
    if(name.endsWith("maxIteration"))params->maxIterationBox->setValue((int)value);
    if(name.endsWith("isSeveralClasses")) params->isSeveralClassesBox->setChecked((int)value);
    if(name.endsWith("thresholdClasses")) params->thresholdBox->setValue((int)value);
    return true;
}
