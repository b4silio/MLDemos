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
#include "interfaceExampleRegress.h"
#include "drawUtils.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>

using namespace std;

RegrExample::RegrExample()
{
    // we initialize the hyperparameter widget
    params = new Ui::ParametersExample();
	params->setupUi(widget = new QWidget());
}

void RegrExample::SetParams(Regressor *regressor)
{
    if(!regressor) return;
    // the dynamic cast ensures that the pointer we received is really a classifierExample
    RegressorExample * myExample = dynamic_cast<RegressorExample *>(regressor);
    // if it isnt, we return
    if(!myExample) return;

    // here we gather the different hyperparameters from the interface
    double param1 = params->param1Spin->value();
    int param2 = params->param2Combo->currentIndex();
    bool param3 = params->param3Check->isChecked();

    // and finally we set the parameters of the algorithm
    myExample->SetParams(param1, param2, param3);
}

QString RegrExample::GetAlgoString()
{
    // here we gather the different hyperparameters from the interface
    double param1 = params->param1Spin->value();
    int param2 = params->param2Combo->currentIndex();
    bool param3 = params->param3Check->isChecked();

    // and we generate the algorithm string with something that is understandable
    QString algo = QString("MyExample %1").arg(param1);
    switch(param2)
    {
    case 0:
        algo += " Low";
        break;
    case 1:
        algo += " Med";
        break;
    case 2:
        algo += " Hig";
        break;
    }

    if(param3) algo += " Opt";
    return algo;
}

Regressor *RegrExample::GetRegressor()
{
    // we instanciate the algorithm object
    RegressorExample *regressor = new RegressorExample();
    // we set its parameters
    SetParams(regressor);
    // we return it to the main program
    return regressor;
}

void RegrExample::DrawInfo(Canvas *canvas, QPainter &painter, Regressor *regressor)
{
    if(!canvas || !regressor) return;
    painter.setRenderHint(QPainter::Antialiasing);

    RegressorExample* myExample = dynamic_cast<RegressorExample*>(regressor);
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

    // and we finally draw it with a radius 10
    painter.drawEllipse(pointInCanvas, 10, 10);
}

void RegrExample::DrawConfidence(Canvas *canvas, Regressor *regressor)
{
	int w = canvas->width();
	int h = canvas->height();

    RegressorExample* myExample = dynamic_cast<RegressorExample*>(regressor);
    if(!myExample) return;
}

void RegrExample::DrawModel(Canvas *canvas, QPainter &painter, Regressor *regressor)
{
	if(!regressor || !canvas) return;
    // get information about the canvas (size, indices and so on)
	int w = canvas->width();
	int h = canvas->height();
    int xIndex = canvas->xIndex;
    int yIndex = canvas->yIndex;
    int outputDim = regressor->outputDim;

    // initialize the points and samples
	int steps = w;
	QPointF oldPoint(-FLT_MAX,-FLT_MAX);
	QPointF oldPointUp(-FLT_MAX,-FLT_MAX);
	QPointF oldPointDown(-FLT_MAX,-FLT_MAX);
	fvec sample;sample.resize(2, 0);

    // make the painter beautiful
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setBrush(Qt::NoBrush);
	FOR(x, steps)
	{
        // get the sample coordinates corresponding to the pixels from left to right
        sample = canvas->toSampleCoords(x, 0);
        int dim = sample.size();
        if(dim > 2) continue;
        if(outputDim==-1) outputDim = dim-1;
        // compute the regression result
        fvec res = regressor->Test(sample); // res[0] contains the estimation, res[1] the confidence/variance/strength
        if(res[0] != res[0] || res[1] != res[1]) continue; // we have nans!

        // and now draw the result as a line
        sample[outputDim] = res[0];
        QPointF point = canvas->toCanvasCoords(sample);
        // plus/minus the variance (if any)
        sample[outputDim] = res[0]+res[1];
        QPointF pointUp = canvas->toCanvasCoords(sample);
        pointUp.setX(0);
        pointUp.setY(pointUp.y() - point.y());
        sample[outputDim] = res[0]-res[1];
        QPointF pointDown = canvas->toCanvasCoords(sample);
        //pointDown = -pointUp;
        pointDown.setX(0);
        pointDown.setY(pointDown.y() - point.y());
        if(x)
		{
			painter.setPen(QPen(Qt::black, 1));
			painter.drawLine(point, oldPoint);
			painter.setPen(QPen(Qt::black, 0.5));
			painter.drawLine(point + pointUp, oldPoint + oldPointUp);
			painter.drawLine(point + pointDown, oldPoint + oldPointDown);

			painter.setPen(QPen(Qt::black, 0.25));
			painter.drawLine(point + 2*pointUp, oldPoint + 2*oldPointUp);
			painter.drawLine(point + 2*pointDown, oldPoint + 2*oldPointDown);

		}
		oldPoint = point;
		oldPointUp = pointUp;
		oldPointDown = pointDown;
	}
}

void RegrExample::SaveOptions(QSettings &settings)
{
    // we save to the system registry each parameter value
    settings.setValue("Param1", params->param1Spin->value());
    settings.setValue("Param2", params->param2Combo->currentIndex());
    settings.setValue("Param3", params->param3Check->isChecked());
}

bool RegrExample::LoadOptions(QSettings &settings)
{
    // we load the parameters from the registry so that when we launch the program we keep all values
    if(settings.contains("Param1")) params->param1Spin->setValue(settings.value("Param1").toFloat());
    if(settings.contains("Param2")) params->param2Combo->setCurrentIndex(settings.value("Param2").toInt());
    if(settings.contains("Param3")) params->param3Check->setChecked(settings.value("Param3").toBool());
    return true;
}

void RegrExample::SaveParams(QTextStream &file)
{
    // same as above but for files/string saving
    file << "regressionOptions" << ":" << "Param1" << " " << params->param1Spin->value() << "\n";
    file << "regressionOptions" << ":" << "Param2" << " " << params->param2Combo->currentIndex() << "\n";
    file << "regressionOptions" << ":" << "Param3" << " " << params->param3Check->isChecked() << "\n";
}

bool RegrExample::LoadParams(QString name, float value)
{
    // same as above but for files/string saving
    if(name.endsWith("Param1")) params->param1Spin->setValue((int)value);
    if(name.endsWith("Param2")) params->param2Combo->setCurrentIndex((int)value);
    if(name.endsWith("Param3")) params->param3Check->setChecked((int)value);
    return true;
}
