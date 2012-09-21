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
#include "interfaceExampleDynamic.h"
#include "drawUtils.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>

using namespace std;

DynamicExample::DynamicExample()
{
    params = new Ui::ParametersExample();
	params->setupUi(widget = new QWidget());
}

void DynamicExample::SetParams(Dynamical *dynamical)
{    
    if(!dynamical) return;
    // the dynamic cast ensures that the pointer we received is really a dynamicalExample
    DynamicalExample * myExample = dynamic_cast<DynamicalExample *>(dynamical);
    // if it isnt, we return
    if(!myExample) return;

    // here we gather the different hyperparameters from the interface
    double param1 = params->param1Spin->value();
    int param2 = params->param2Combo->currentIndex();
    bool param3 = params->param3Check->isChecked();

    // and finally we set the parameters of the algorithm
    myExample->SetParams(param1, param2, param3);
}

Dynamical *DynamicExample::GetDynamical()
{
    // we instanciate the algorithm object
    DynamicalExample *dynamical = new DynamicalExample();
    // we set its parameters
    SetParams(dynamical);
    // we return it to the main program
	return dynamical;
}

void DynamicExample::DrawInfo(Canvas *canvas, QPainter &painter, Dynamical *dynamical)
{
    if(!canvas || !dynamical) return;
    painter.setRenderHint(QPainter::Antialiasing);

    DynamicalExample* myExample = dynamic_cast<DynamicalExample*>(dynamical);
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

void DynamicExample::DrawModel(Canvas *canvas, QPainter &painter, Dynamical *dynamical)
{
    // nothing much to do really
	if(!canvas || !dynamical) return;
}

void DynamicExample::SaveOptions(QSettings &settings)
{
    // we save to the system registry each parameter value
    settings.setValue("Param1", params->param1Spin->value());
    settings.setValue("Param2", params->param2Combo->currentIndex());
    settings.setValue("Param3", params->param3Check->isChecked());
}

bool DynamicExample::LoadOptions(QSettings &settings)
{
    // we load the parameters from the registry so that when we launch the program we keep all values
    if(settings.contains("Param1")) params->param1Spin->setValue(settings.value("Param1").toFloat());
    if(settings.contains("Param2")) params->param2Combo->setCurrentIndex(settings.value("Param2").toInt());
    if(settings.contains("Param3")) params->param3Check->setChecked(settings.value("Param3").toBool());
    return true;
}

void DynamicExample::SaveParams(QTextStream &file)
{
    // same as above but for files/string saving
    file << "dynamicalOptions" << ":" << "Param1" << " " << params->param1Spin->value() << "\n";
    file << "dynamicalOptions" << ":" << "Param2" << " " << params->param2Combo->currentIndex() << "\n";
    file << "dynamicalOptions" << ":" << "Param3" << " " << params->param3Check->isChecked() << "\n";
}

bool DynamicExample::LoadParams(QString name, float value)
{
    // same as above but for files/string saving
    if(name.endsWith("Param1")) params->param1Spin->setValue((int)value);
    if(name.endsWith("Param2")) params->param2Combo->setCurrentIndex((int)value);
    if(name.endsWith("Param3")) params->param3Check->setChecked((int)value);
    return true;
}
