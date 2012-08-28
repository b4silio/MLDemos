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
#include "interfaceLowess.h"
#include "drawUtils.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>

using namespace std;

RegrLowess::RegrLowess()
{
    // we initialize the hyperparameter widget
    params = new Ui::ParametersLowess();
	params->setupUi(widget = new QWidget());
}

void RegrLowess::SetParams(Regressor *regressor)
{
    if(!regressor) return;
    // the dynamic cast ensures that the pointer we received is really a classifierExample
    RegressorLowess * myRegressor = dynamic_cast<RegressorLowess *>(regressor);
    // if it isnt, we return
    if(!myRegressor) return;

    // here we gather the different hyperparameters from the interface
    double           smoothFac     =                   params->paramSmoothingFac->value();
    lowessWeightFunc weightingFunc = (lowessWeightFunc)params->paramWeightingFunc->currentIndex();
    lowessFitType    fitType       = (lowessFitType)   params->paramFitType->currentIndex();
    lowessNormType   normType      = (lowessNormType)  params->paramNormType->currentIndex();

    // and finally we set the parameters of the algorithm
    myRegressor->SetParams(smoothFac, weightingFunc, fitType, normType);
}

QString RegrLowess::GetAlgoString()
{
    // here we gather the different hyperparameters from the interface
    double           smoothFac     =                   params->paramSmoothingFac->value();
    lowessWeightFunc weightingFunc = (lowessWeightFunc)params->paramWeightingFunc->currentIndex();
    lowessFitType    fitType       = (lowessFitType)   params->paramFitType->currentIndex();
    lowessNormType   normType      = (lowessNormType)  params->paramNormType->currentIndex();

    // and we generate the algorithm string with something that is understandable
    QString algo = QString("LOWESS %1").arg(smoothFac);

    switch(weightingFunc)
    {
    case kLowessWeightFunc_Tricube:
        algo += " Tric";
        break;
    case kLowessWeightFunc_Hann:
        algo += " Hann";
        break;
    case kLowessWeightFunc_Uniform:
        algo += " Uni";
        break;
    }

    if (fitType == kLowessFitType_Linear)
        algo += " Lin";
    else
        algo += " Quad";

    if (normType == kLowessNormType_StDev)
        algo += " StDev";
    else if (normType == kLowessNormType_IQR)
        algo += " IQR";

    return algo;
}

Regressor *RegrLowess::GetRegressor()
{
    // we instantiate the algorithm object
    RegressorLowess *regressor = new RegressorLowess();
    // we set its parameters
    SetParams(regressor);
    // we return it to the main program
    return regressor;
}

void RegrLowess::DrawModel(Canvas *canvas, QPainter &painter, Regressor *regressor)
{
	if(!regressor || !canvas) return;

    //Did the LOWESS regressor find a solution?
    RegressorLowess* myRegressor = dynamic_cast<RegressorLowess*>(regressor);
    if (!myRegressor->Ready())
        return;

    //Set the dimension normalization combobox to 'None'
    //in case of 2-D regression
    if (myRegressor->GetSamples()[0].size() < 3)
    {
        params->paramNormType->setCurrentIndex(kLowessNormType_None);
        SetParams(regressor);
    }

    // get information about the canvas (size, indices and so on)
	int w = canvas->width();
    int h = canvas->height();
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

        //Store the radius of the local neighborhood in the radius vector.
        //This will be used in DrawConfidence() to visualize the width of
        //the regression window at each point Test() was called above.
        myRegressor->StoreLastRadius();
	}
}

//Visualize the changing size of the regression window by painting lines
//in varying shades of gray at each point Test() was called above.
//This has nothing to do with a confidence map! This map was just chosen
//because it is displayed behind the data points, whereas the model info
//map is overlaid in front of (and obscures) the data points.
void RegrLowess::DrawConfidence(Canvas *canvas, Regressor *regressor)
{
    if(!canvas || !regressor) return;
    QPainter painter(&canvas->confidencePixmap());
    painter.setRenderHint(QPainter::Antialiasing, false);

    RegressorLowess* myRegressor = dynamic_cast<RegressorLowess*>(regressor);
    if(!myRegressor) return;

    fvec const &radiusVec = myRegressor->GetRadiusVec();
    if (radiusVec.empty())
        return;

    float maxRadius = *max_element(radiusVec.begin(), radiusVec.end()),
          minRadius = *min_element(radiusVec.begin(), radiusVec.end());
    if (maxRadius < 1e-6f)
        return;
    maxRadius -= minRadius;

    int w = canvas->width();
    int h = canvas->height();
    QColor  lineClr(Qt::black);
    QPointF pointTop = canvas->toCanvasCoords(canvas->canvasTopLeft());
    QPointF pointBot = canvas->toCanvasCoords(canvas->canvasBottomRight());

    FOR(x, min(w, (int)radiusVec.size()))
    {
        if (radiusVec[x] <= 0.0f)
            continue;

        //Determine the shade of gray. The smallest window size will be
        //white, the largest one will be of shade [128, 128, 128].
        float normalizedRadius = 1.0f - (radiusVec[x]-minRadius)/maxRadius;
        normalizedRadius = 128 + min(max((int)(127*normalizedRadius + 0.5f), 0), 127);

        pointTop.setX(x);
        pointBot.setX(x);
        lineClr.setRgb(normalizedRadius, normalizedRadius, normalizedRadius);
        painter.setPen(lineClr);
        painter.drawLine(pointTop, pointBot);
    }
}

void RegrLowess::SaveOptions(QSettings &settings)
{
    // we save to the system registry each parameter value
    settings.setValue("lowessSmoothFac", params->paramSmoothingFac->value());
    settings.setValue("lowessWeightFnc", params->paramWeightingFunc->currentIndex());
    settings.setValue("lowessFitType",   params->paramFitType->currentIndex());
    settings.setValue("lowessDimNorm",   params->paramNormType->currentIndex());
}

bool RegrLowess::LoadOptions(QSettings &settings)
{
    // we load the parameters from the registry so that when we launch the program we keep all values
    if(settings.contains("lowessSmoothFac")) params->paramSmoothingFac->setValue(settings.value("lowessSmoothFac").toFloat());
    if(settings.contains("lowessWeightFnc")) params->paramWeightingFunc->setCurrentIndex(settings.value("lowessWeightFnc").toInt());
    if(settings.contains("lowessFitType"))   params->paramFitType->setCurrentIndex(settings.value("lowessFitType").toInt());
    if(settings.contains("lowessDimNorm"))   params->paramNormType->setCurrentIndex(settings.value("lowessDimNorm").toInt());
    return true;
}

void RegrLowess::SaveParams(QTextStream &file)
{
    // same as above but for files/string saving
    file << "regressionOptions" << ":" << "lowessSmoothFac" << " " << params->paramSmoothingFac->value() << "\n";
    file << "regressionOptions" << ":" << "lowessWeightFnc" << " " << params->paramWeightingFunc->currentIndex() << "\n";
    file << "regressionOptions" << ":" << "lowessFitType"   << " " << params->paramFitType->currentIndex() << "\n";
    file << "regressionOptions" << ":" << "lowessDimNorm"   << " " << params->paramNormType->currentIndex() << "\n";
}

bool RegrLowess::LoadParams(QString name, float value)
{
    // same as above but for files/string saving
    if(name.endsWith("lowessSmoothFac")) params->paramSmoothingFac->setValue(value);
    if(name.endsWith("lowessWeightFnc")) params->paramWeightingFunc->setCurrentIndex((int)value);
    if(name.endsWith("lowessFitType"))   params->paramFitType->setCurrentIndex((int)value);
    if(name.endsWith("lowessDimNorm"))   params->paramNormType->setCurrentIndex((int)value);
    return true;
}
