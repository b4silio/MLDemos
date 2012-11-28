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
#include "interfaceASVMDynamic.h"
#include "drawUtils.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>
#include <QDebug>
#include <qcontour.h>

using namespace std;

DynamicASVM::DynamicASVM()
{
    params = new Ui::ParametersASVM();
    params->setupUi(widget = new QWidget());
}

void DynamicASVM::SetParams(Dynamical *dynamical)
{
    if(!dynamical) return;

    int clusters = params->gmmCount->value();
    double alphaTol = params->alphaTolSpin->value();
    double betaTol = params->betaTolSpin->value();
    double betaRelax = params->betaRelaxSpin->value();
    double Cparam = params->CSpin->value();
    double kernelWidth = params->kernelWidthSpin->value();
    double epsilon = params->epsilonSpin->value();
    int maxIteration = params->iterationCount->value();

    DynamicalASVM *asvm = dynamic_cast<DynamicalASVM *>(dynamical);
    if(!asvm) return;

    asvm->SetParams(clusters, kernelWidth, Cparam, alphaTol, betaTol, betaRelax, epsilon, maxIteration);
}

Dynamical *DynamicASVM::GetDynamical()
{
    DynamicalASVM *dynamical = new DynamicalASVM();
    SetParams(dynamical);
    return dynamical;
}

void DynamicASVM::DrawInfo(Canvas *canvas, QPainter &painter, Dynamical *dynamical)
{
    if(!canvas || !dynamical) return;
    painter.setRenderHint(QPainter::Antialiasing);

    DynamicalASVM *asvm = dynamic_cast<DynamicalASVM*>(dynamical);
    if(!asvm) return;

    /*
    // we display the gmms
    vector<Gmm*> gmms = asvm->gmms;
    FOR(c, gmms.size())
    {
        Gmm gmm = *(gmms[c]);

        int xIndex = canvas->xIndex;
        int yIndex = canvas->yIndex;
        int dim = gmm.dim;
        float mean[2];
        float sigma[3];
        painter.setBrush(Qt::NoBrush);
        FOR(i, gmm.nstates)
        {
            float* bigSigma = new float[dim*dim];
            float* bigMean = new float[dim];
            gmm.getCovariance(i, bigSigma, false);
            sigma[0] = bigSigma[xIndex*dim + xIndex];
            sigma[1] = bigSigma[yIndex*dim + xIndex];
            sigma[2] = bigSigma[yIndex*dim + yIndex];
            gmm.getMean(i, bigMean);
            mean[0] = bigMean[xIndex];
            mean[1] = bigMean[yIndex];
            delete [] bigSigma;
            delete [] bigMean;

            painter.setPen(QPen(Qt::black, 1));
            DrawEllipse(mean, sigma, 1, &painter, canvas);
            painter.setPen(QPen(Qt::black, 0.5));
            DrawEllipse(mean, sigma, 2, &painter, canvas);
            QPointF point = canvas->toCanvasCoords(mean[0],mean[1]);
            painter.setPen(QPen(Qt::black, 4));
            painter.drawEllipse(point, 2, 2);
            painter.setPen(QPen(Qt::white, 2));
            painter.drawEllipse(point, 2, 2);
        }
    }
    */

    // we display the support vectors
    painter.setPen(QPen(Qt::black, 1.5));
    FOR(i, asvm->asvms.size())
    {
        FOR(j, asvm->asvms[i].numAlpha)
        {
            double *sv = asvm->asvms[i].svalpha[j];
            fvec sample(asvm->asvms[i].dim);
            FOR(d,asvm->asvms[i].dim) sample[d] = sv[d];
            QPointF point = canvas->toCanvasCoords(sample);
            painter.drawLine(point-QPointF(-5,-5),point-QPointF(5,-5));
            painter.drawLine(point-QPointF(5,-5),point-QPointF(5,5));
            painter.drawLine(point-QPointF(5,5),point-QPointF(-5,5));
            painter.drawLine(point-QPointF(-5,5),point-QPointF(-5,-5));
        }
        FOR(j, asvm->asvms[i].numBeta)
        {
            double *sv = asvm->asvms[i].svbeta[j];
            fvec sample(asvm->asvms[i].dim);
            FOR(d,asvm->asvms[i].dim) sample[d] = sv[d];
            QPointF point = canvas->toCanvasCoords(sample);
            painter.drawLine(point-QPointF(-5,-5),point-QPointF(5,-5));
            painter.drawLine(point-QPointF(5,-5),point-QPointF(0,5));
            painter.drawLine(point-QPointF(-5,-5),point-QPointF(0,5));
        }
    }

    // we display the contour lines of the svm classifier
    int W = painter.viewport().width();
    int H = painter.viewport().height();
    int w = 129;
    int h = 129;
    int classCount = asvm->classCount;

    // we draw the contours of the classification function
    double **valueList = new double*[classCount];
    FOR(c, classCount)
    {
        valueList[c] = new double[w*h];
        FOR(i, w)
        {
            FOR(j, h)
            {
                valueList[c][j*w+i] = 0.;
            }
        }
    }

    FOR(i, w)
    {
        FOR(j, h)
        {
            int x = i*W/w;
            int y = j*H/h;
            fvec sample = canvas->fromCanvas(x,y);
            fvec res = asvm->Classify(sample);
            int c = res[0];
            double value = res[1];
            // to avoid some numerical weird stuff
            value = value*1000.;
            if(c < classCount && c >= 0)
            {
                valueList[c][j*w + i] = value;
            }
        }
    }

    FOR(k, classCount)
    {
        QContour contour(valueList[k], w, h);
        contour.bDrawColorbar = false;
        int classColor = asvm->inverseMap.count(k) ? asvm->inverseMap[k] : k;
        contour.plotColor = classColor ? SampleColor[(classColor)%SampleColorCnt] : Qt::black;
        contour.plotThickness = 4;
        contour.style = Qt::DotLine;

        double vmin, vmax;
        contour.GetLimits(vmin, vmax);
        vmin += (vmax - vmin)/30; // we take out the smallest levels to avoid numerical issues
        contour.SetLimits(vmin, vmax);
        contour.Paint(painter, 10);
        delete [] valueList[k];
        valueList[k] = 0;
    }
    delete [] valueList;
}

void DynamicASVM::SaveModel(QString filename, Dynamical *dynamical)
{
    DynamicalASVM *asvm = dynamic_cast<DynamicalASVM*>(dynamical);
    if(!asvm) return;
    asvm->SaveModel(filename.toStdString());
}

bool DynamicASVM::LoadModel(QString filename, Dynamical *dynamical)
{
    DynamicalASVM *asvm = dynamic_cast<DynamicalASVM*>(dynamical);
    if(!asvm) return false;
    return asvm->LoadModel(filename.toStdString());
}

void DynamicASVM::SaveOptions(QSettings &settings)
{
    settings.setValue("gmmCount", params->gmmCount->value());
    settings.setValue("alphaTol", params->alphaTolSpin->value());
    settings.setValue("betaTol", params->betaTolSpin->value());
    settings.setValue("betaRelax", params->betaRelaxSpin->value());
    settings.setValue("Cparam", params->CSpin->value());
    settings.setValue("kernelWidth", params->kernelWidthSpin->value());
    settings.setValue("epsilon", params->epsilonSpin->value());
    settings.setValue("iterationCount", params->iterationCount->value());
}

bool DynamicASVM::LoadOptions(QSettings &settings)
{
    if(settings.contains("gmmCount")) params->gmmCount->setValue(settings.value("gmmCount").toInt());
    if(settings.contains("alphaTol")) params->alphaTolSpin->setValue(settings.value("alphaTol").toDouble());
    if(settings.contains("betaTol")) params->betaTolSpin->setValue(settings.value("betaTol").toDouble());
    if(settings.contains("betaRelax")) params->betaRelaxSpin->setValue(settings.value("betaRelax").toDouble());
    if(settings.contains("Cparam")) params->CSpin->setValue(settings.value("Cparam").toDouble());
    if(settings.contains("kernelWidth")) params->kernelWidthSpin->setValue(settings.value("kernelWidth").toDouble());
    if(settings.contains("epsilon")) params->epsilonSpin->setValue(settings.value("epsilon").toDouble());
    if(settings.contains("iterationCount")) params->iterationCount->setValue(settings.value("iterationCount").toInt());
    return true;
}

void DynamicASVM::SaveParams(QTextStream &file)
{
    file << "dynamicalOptions" << ":" << "gmmCount" << " " << params->gmmCount->value() << "\n";
    file << "dynamicalOptions" << ":" << "alphaTol" << " " << params->alphaTolSpin->value() << "\n";
    file << "dynamicalOptions" << ":" << "betaTol" << " " << params->betaTolSpin->value() << "\n";
    file << "dynamicalOptions" << ":" << "betaRelax" << " " << params->betaRelaxSpin->value() << "\n";
    file << "dynamicalOptions" << ":" << "Cparam" << " " << params->CSpin->value() << "\n";
    file << "dynamicalOptions" << ":" << "kernelWidth" << " " << params->kernelWidthSpin->value() << "\n";
    file << "dynamicalOptions" << ":" << "epsilon" << " " << params->epsilonSpin->value() << "\n";
    file << "dynamicalOptions" << ":" << "iterationCount" << " " << params->iterationCount->value() << "\n";
}

bool DynamicASVM::LoadParams(QString name, float value)
{
    if(name.endsWith("gmmCount")) params->gmmCount->setValue((int)value);
    if(name.endsWith("alphaTol")) params->alphaTolSpin->setValue((double)value);
    if(name.endsWith("betaTol")) params->betaTolSpin->setValue((double)value);
    if(name.endsWith("betaRelax")) params->betaRelaxSpin->setValue((double)value);
    if(name.endsWith("Cparam")) params->CSpin->setValue((double)value);
    if(name.endsWith("kernelWidth")) params->kernelWidthSpin->setValue((double)value);
    if(name.endsWith("epsilon")) params->epsilonSpin->setValue((double)value);
    if(name.endsWith("iterationCount")) params->iterationCount->setValue((int)value);
    return true;
}

Q_EXPORT_PLUGIN2(mld_ASVM, DynamicASVM)
