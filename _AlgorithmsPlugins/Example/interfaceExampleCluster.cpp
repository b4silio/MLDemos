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
#include "interfaceExampleCluster.h"
#include "drawUtils.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>

using namespace std;

ClustExample::ClustExample()
{
    // we initialize the hyperparameter widget
    params = new Ui::ParametersExample();
	params->setupUi(widget = new QWidget());
}

void ClustExample::SetParams(Clusterer *clusterer)
{
    if(!clusterer) return;
    // the dynamic cast ensures that the pointer we received is really a clusterExample
    ClustererExample * myExample = dynamic_cast<ClustererExample *>(clusterer);
    // if it isnt, we return
    if(!myExample) return;

    // here we gather the different hyperparameters from the interface
    double param1 = params->param1Spin->value();
    int param2 = params->param2Combo->currentIndex();
    bool param3 = params->param3Check->isChecked();

    // and finally we set the parameters of the algorithm
    myExample->SetParams(param1, param2, param3);
}

Clusterer *ClustExample::GetClusterer()
{
    // we instanciate the algorithm object
    ClustererExample *clusterer = new ClustererExample();
    // we set its parameters
    SetParams(clusterer);
    // we return it to the main program
	return clusterer;
}

void ClustExample::DrawInfo(Canvas *canvas, QPainter &painter, Clusterer *clusterer)
{
    if(!canvas || !clusterer) return;
    painter.setRenderHint(QPainter::Antialiasing);

    ClustererExample * myExample = dynamic_cast<ClustererExample*>(clusterer);
    if(!myExample) return;

    // to give an example, we use the QPainter interface to paint a circle close to the center of the data space


    // first we need to know which 2 dimensions are currently being displayed (in case of multi-dimensional data)
    // if the data is 2-dimensional it will be 0 and 1
    int xIndex = canvas->xIndex;
    int yIndex = canvas->yIndex;

    // now we get the current position of the center of the dataspace
    fvec sample = canvas->center;

    // and we add a random noise around it
    sample[xIndex] += (drand48()-0.5f)*0.1;
    sample[yIndex] += (drand48()-0.5f)*0.1;

    // we need to convert the sample coordinates from dataspace (N-dimensional in R) to the canvas coordinates (2D pixel by pixel)
    QPointF pointInCanvas = canvas->toCanvasCoords(sample);

    // we make the painter paint nicely (work well with forms, not so much with text)
    painter.setRenderHint(QPainter::Antialiasing);

    // we set the brush and pen, in our case no brush (hollow circle) and a thick red edge
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(Qt::red, 4));

    // and we finally draw it with a radius of 10 pixels
    painter.drawEllipse(pointInCanvas, 10, 10);
}

void ClustExample::DrawModel(Canvas *canvas, QPainter &painter, Clusterer *clusterer)
{
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

void ClustExample::SaveOptions(QSettings &settings)
{
    // we save to the system registry each parameter value
    settings.setValue("Param1", params->param1Spin->value());
    settings.setValue("Param2", params->param2Combo->currentIndex());
    settings.setValue("Param3", params->param3Check->isChecked());
}

bool ClustExample::LoadOptions(QSettings &settings)
{
    // we load the parameters from the registry so that when we launch the program we keep all values
    if(settings.contains("Param1")) params->param1Spin->setValue(settings.value("Param1").toFloat());
    if(settings.contains("Param2")) params->param2Combo->setCurrentIndex(settings.value("Param2").toInt());
    if(settings.contains("Param3")) params->param3Check->setChecked(settings.value("Param3").toBool());
    return true;
}

void ClustExample::SaveParams(QTextStream &file)
{
    // same as above but for files/string saving
    file << "clusterOptions" << ":" << "Param1" << " " << params->param1Spin->value() << "\n";
    file << "clusterOptions" << ":" << "Param2" << " " << params->param2Combo->currentIndex() << "\n";
    file << "clusterOptions" << ":" << "Param3" << " " << params->param3Check->isChecked() << "\n";
}

bool ClustExample::LoadParams(QString name, float value)
{
    // same as above but for files/string saving
    if(name.endsWith("Param1")) params->param1Spin->setValue((int)value);
    if(name.endsWith("Param2")) params->param2Combo->setCurrentIndex((int)value);
    if(name.endsWith("Param3")) params->param3Check->setChecked((int)value);
    return true;
}
