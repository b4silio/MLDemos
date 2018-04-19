/*********************************************************************
MLDemos: A User-Friendly visualization toolkit for machine learning
Copyright (C) 2010  Basilio Noris
Contact: mldemos@b4silio.com

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free
Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*********************************************************************/
#include "algorithmmanager.h"
#include <qcontour.h>

using namespace std;

void AlgorithmManager::Train(Maximizer *maximizer)
{
    if(!maximizer) return;
    if(canvas->maps.reward.isNull()) return;
    QImage rewardImage = canvas->maps.reward.toImage();
    QRgb *pixels = (QRgb*) rewardImage.bits();
    int w = rewardImage.width();
    int h = rewardImage.height();

    float *data = new float[w*h];
    float maxData = 0;
    FOR(i, w*h)
    {
        data[i] = 1.f - qBlue(pixels[i])/255.f; // all data is in a 0-1 range
        maxData = max(maxData, data[i]);
    }
    if(maxData > 0)
    {
        FOR(i, w*h) data[i] /= maxData; // we ensure that the data is normalized
    }
    ivec size;
    size.push_back(w);
    size.push_back(h);
    fvec low(2,0.f);
    fvec high(2,1.f);
    canvas->data->GetReward()->SetReward(data, size, low, high);
    //    delete [] data;

    fvec startingPoint;
    if(canvas->targets.size())
    {
        startingPoint = canvas->targets.back();
        QPointF starting = canvas->toCanvasCoords(startingPoint);
        startingPoint[0] = starting.x()/w;
        startingPoint[1] = starting.y()/h;
    }
    else
    {
        startingPoint.resize(2);
        startingPoint[0] = drand48();
        startingPoint[1] = drand48();
    }
    //data = canvas->data->GetReward()->GetRewardFloat();
    maximizer->Train(data, fVec(w,h), startingPoint);
    maximizer->age = 0;
    delete [] data;
}

void AlgorithmManager::Test(Maximizer *maximizer)
{
    if(!maximizer) return;
    do
    {
        fvec sample = maximizer->Test(maximizer->Maximum());
        maximizer->age++;
    }
    while(maximizer->age < maximizer->maxAge && maximizer->MaximumValue() < maximizer->stopValue);
}

void AlgorithmManager::Maximize()
{
    if(!canvas) return;
    if(canvas->maps.reward.isNull()) return;
    QMutexLocker lock(mutex);
    drawTimer->Stop();
    DEL(clusterer);
    DEL(regressor);
    DEL(dynamical);
    if(!classifierMulti.size()) DEL(classifier);
    classifier = 0;
    sourceDims.clear();
    FOR(i,classifierMulti.size()) DEL(classifierMulti[i]); classifierMulti.clear();
    DEL(maximizer);
    DEL(reinforcement);
    DEL(projector);
    lastTrainingInfo = "";
    if(!optionsMaximize->algoList->count()) return;
    int tab = optionsMaximize->algoList->currentIndex();
    if(tab >= maximizers.size() || !maximizers[tab]) return;
    maximizer = maximizers[tab]->GetMaximizer();
    maximizer->maxAge = optionsMaximize->iterationsSpin->value();
    maximizer->stopValue = optionsMaximize->stoppingSpin->value();
    tabUsedForTraining = tab;
    Train(maximizer);

    // we draw the contours for the current maximization
    int w = 65;
    int h = 65;
    int W = canvas->width();
    int H = canvas->height();
    canvas->maps.info = QPixmap(W, H);
    //QBitmap bitmap(canvas->width(), canvas->height());
    //bitmap.clear();
    //canvas->maps.info.setMask(bitmap);
    canvas->maps.info.fill(Qt::transparent);
    QPainter painter(&canvas->maps.info);

    double *bigData = canvas->data->GetReward()->rewards;
    double maxVal = -DBL_MAX;
    FOR(i, W*H) maxVal = max(bigData[i], maxVal);
    maxVal *= maximizer->stopValue; // used to ensure we have a maximum somewhere
    double *data = new double[w*h];
    FOR(i, w)
    {
        FOR(j, h)
        {
            int I = i*W/(w-1);
            int J = j*H/(h-1);
            if(I >= W) I = W-1;
            if(J >= H) J = H-1;
            data[j*w + i] = bigData[J*W + I];
        }
    }

    QContour contour(data, w, h);
    contour.bDrawColorbar = false;
    contour.plotColor = Qt::black;
    contour.plotThickness = 1.5;
    contour.style = Qt::DashLine;
    //contour.style;
    contour.Paint(painter, 10);
    // we want to find all the samples that are at maximum value
    painter.setPen(QColor(255,255,0));
    FOR(i,W)
    {
        FOR(j,H)
        {
            if(bigData[j*W + i] >= maxVal)
            {
                painter.drawPoint(i,j);
            }
        }
    }

    delete [] data;
    canvas->repaint();

    FOR(i, glw->objects.size())
    {
        if(!glw->objectAlive[i]) continue;
        if(glw->objects[i].objectType.contains("Maximization"))
        {
            glw->killList.push_back(i);
        }
    }

    emit UpdateInfo();

    drawTimer->Stop();
    drawTimer->Clear();
    drawTimer->start(QThread::NormalPriority);
}

void AlgorithmManager::MaximizeContinue()
{
    if(!canvas || !maximizer) return;
    QMutexLocker lock(mutex);
    if(drawTimer)
    {
        drawTimer->Stop();
    }
    maximizer->SetConverged(!maximizer->hasConverged());

    emit UpdateInfo();

    if(drawTimer)
    {
        drawTimer->start(QThread::NormalPriority);
    }
}

