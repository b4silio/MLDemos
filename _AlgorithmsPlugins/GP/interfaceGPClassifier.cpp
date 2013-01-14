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
#include "interfaceGPClassifier.h"
#include "drawUtils.h"
#include <basicMath.h>
#include <QPixmap>
#include <QBitmap>
#include <QPainter>
#include <QDebug>

using namespace std;

ClassGP::ClassGP()
{
    // we initialize the hyperparameter widget
    params = new Ui::ParametersGP();
    params->setupUi(widget = new QWidget());
}

void ClassGP::SetParams(Classifier *classifier)
{
    if(!classifier) return;
    // the dynamic cast ensures that the pointer we received is really a classifierGP
    ClassifierGP * myGP = dynamic_cast<ClassifierGP *>(classifier);
    // if it isnt, we return
    if(!myGP) return;

    // here we gather the different hyperparameters from the interface

    double lengthscale = 1.f/params->lengthscale->value();
    lengthscale  = lengthscale*lengthscale;
    int Method = params->evalmethod->currentIndex();
    int Nsamp = params->Nsamp->value();

    // and finally we set the parameters of the algorithm
    myGP->SetParams(lengthscale,Method,Nsamp);
}

fvec ClassGP::GetParams()
{
    double lengthscale = 1.f/params->lengthscale->value();
    lengthscale  = lengthscale*lengthscale;
    int Method = params->evalmethod->currentIndex();
    int Nsamp = params->Nsamp->value();

    fvec par(3);
    par[0] = lengthscale;
    par[1] = Method;
    par[2] = Nsamp;
    return par;
}

void ClassGP::SetParams(Classifier *classifier, fvec parameters)
{
    if(!classifier) return;
    ClassifierGP * myGP = dynamic_cast<ClassifierGP *>(classifier);
    if(!myGP) return;

    int i = 0;
    double lengthscale = parameters.size() > i ? parameters[i] : 0; i++;
    int Method = parameters.size() > i ? parameters[i] : 0; i++;
    int Nsamp = parameters.size() > i ? parameters[i] : 0; i++;

    myGP->SetParams(lengthscale,Method,Nsamp);
}

void ClassGP::GetParameterList(std::vector<QString> &parameterNames,
                             std::vector<QString> &parameterTypes,
                             std::vector< std::vector<QString> > &parameterValues)
{
    parameterNames.clear();
    parameterTypes.clear();
    parameterValues.clear();
    parameterNames.push_back("Length Scale");
    parameterNames.push_back("Evaluation Method");
    parameterNames.push_back("Sampling Count");
    parameterTypes.push_back("Real");
    parameterTypes.push_back("List");
    parameterTypes.push_back("Integer");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("0.00000001f");
    parameterValues.back().push_back("99999999999");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("Numerical");
    parameterValues.back().push_back("Monte Carlo");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("1");
    parameterValues.back().push_back("9999");
}

QString ClassGP::GetAlgoString()
{
    // here we gather the different hyperparameters from the interface

    int param2 = params->evalmethod->currentIndex();
    double lengthscale  =params->lengthscale->value();




    // and we generate the algorithm string with something that is understandable
    QString algo = QString("GP classifier");
    switch(param2)
    {
    case 0:
        algo += " Numerical Integration.";
        break;
    case 1:
        algo += "MonteCarlo.";
        break;

    }
    algo+=" lengthscale: ";
    algo+=QString("%1").arg(lengthscale);

    return algo;
}

Classifier *ClassGP::GetClassifier()
{
    // we instanciate the algorithm object
    ClassifierGP *classifier = new ClassifierGP();
    // we set its parameters
    SetParams(classifier);
    // we return it to the main program
    return classifier;
}

void ClassGP::DrawInfo(Canvas *canvas, QPainter &painter, Classifier *classifier)
{
    if(!canvas || !classifier) return;
    painter.setRenderHint(QPainter::Antialiasing);

    ClassifierGP * myGP = dynamic_cast<ClassifierGP*>(classifier);
    if(!myGP) return;

    // to give an GP, we use the QPainter interface to paint a circle close to the center of the data space


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

    // and we finally draw it with a radius of 10
    painter.drawEllipse(pointInCanvas, 10, 10);

}

void ClassGP::DrawModel(Canvas *canvas, QPainter &painter, Classifier *classifier)
{
    // we want to draw the samples

    // we start by making the painter paint things nicely
    painter.setRenderHint(QPainter::Antialiasing);

    // for every point in the current dataset
    FOR(i, canvas->data->GetCount())
    {
        // we get the sample
        fvec sample = canvas->data->GetSample(i);
        // and its label
        int label = canvas->data->GetLabel(i);

        // and we test it using the classifier (TestMulti is for multi-class classification)
        fvec res = classifier->TestMulti(sample);

        // we get the point in canvas coordinates (2D pixel by pixel) corresponding to the sample (N-dimensional in R)
        QPointF point = canvas->toCanvasCoords(canvas->data->GetSample(i));

        // if we only have one response it is a binary classification
        if(res.size()==1)
        {
            // if the response is positive
            if(res[0] > 0)
            {
                // if the sample is also positive then we draw it with the corresponding color
                if(label == 1) Canvas::drawSample(painter, point, 9, 1);
                // else we draw a cross
                else Canvas::drawCross(painter, point, 6, 2);
            }
            // vice-versa here if the response is negative
            else
            {
                if(label != 1) Canvas::drawSample(painter, point, 9, 0);
                else Canvas::drawCross(painter, point, 6, 0);
            }
        }
        // if we have multiple responses
        else
        {
            // we look for the class with the maximum response
            int max = 0;
            for(int i=1; i<res.size(); i++) if(res[max] < res[i]) max = i;
            int resp = classifier->inverseMap[max];
            // if it corresponds to the actual label we draw it as a circle
            if(label == resp) Canvas::drawSample(painter, point, 9, label);
            // otherwise we draw a cross
            else Canvas::drawCross(painter, point, 6, label);
        }
    }
}

void ClassGP::SaveOptions(QSettings &settings)
{
    // we save to the system registry each parameter value
    settings.setValue("Param1", params->lengthscale->value());
    settings.setValue("Param2", params->evalmethod->currentIndex());

}

bool ClassGP::LoadOptions(QSettings &settings)
{
    // we load the parameters from the registry so that when we launch the program we keep all values
    if(settings.contains("Param1")) params->lengthscale->setValue(settings.value("Param1").toFloat());
    if(settings.contains("Param2")) params->evalmethod->setCurrentIndex(settings.value("Param2").toInt());

    return true;
}

void ClassGP::SaveParams(QTextStream &file)
{
    // same as above but for files/string saving
    file << "classificationOptions" << ":" << "Param1" << " " << params->lengthscale->value() << "\n";
    file << "classificationOptions" << ":" << "Param2" << " " << params->evalmethod->currentIndex() << "\n";

}

bool ClassGP::LoadParams(QString name, float value)
{
    // same as above but for files/string saving
    if(name.endsWith("Param1")) params->lengthscale->setValue((int)value);
    if(name.endsWith("Param2")) params->evalmethod->setCurrentIndex((int)value);

    return true;
}
