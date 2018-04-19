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

void AlgorithmManager::Train(Reinforcement *reinforcement)
{
    if(!reinforcement) return;
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

    //data = canvas->data->GetReward()->GetRewardFloat();
    reinforcementProblem.Initialize(data, fVec(w,h));
    reinforcement->Initialize(&reinforcementProblem);
    reinforcement->age = 0;
    delete [] data;
}

void AlgorithmManager::Reinforce()
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
    if(!optionsReinforcement->algoList->count()) return;
    int tab = optionsReinforcement->algoList->currentIndex();
    if(tab >= reinforcements.size() || !reinforcements[tab]) return;
    reinforcement = reinforcements[tab]->GetReinforcement();

    reinforcementProblem.problemType = optionsReinforcement->problemCombo->currentIndex();
    reinforcementProblem.rewardType = optionsReinforcement->rewardCombo->currentIndex();
    reinforcementProblem.policyType = optionsReinforcement->policyCombo->currentIndex();
    reinforcementProblem.quantizeType = optionsReinforcement->quantizeCombo->currentIndex();
    reinforcementProblem.gridSize = optionsReinforcement->resolutionSpin->value();

    reinforcementProblem.simulationSteps = optionsReinforcement->iterationsSpin->value();
    reinforcementProblem.displayIterationsCount = optionsReinforcement->displayIterationSpin->value();

    reinforcement->maxAge = optionsReinforcement->iterationsSpin->value();

    tabUsedForTraining = tab;
    Train(reinforcement);
    reinforcements[tab]->Draw(canvas, reinforcement);
    glw->clearLists();
    if(canvas->canvasType == 1)
    {
        reinforcements[tab]->DrawGL(canvas, glw, reinforcement);
        if(canvas->data->GetDimCount() == 3) Draw3DReinforcement(glw, reinforcement);
    }

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
    delete [] data;
    canvas->repaint();

    emit UpdateInfo();

    drawTimer->Stop();
    drawTimer->Clear();
    drawTimer->start(QThread::NormalPriority);
}

void AlgorithmManager::ReinforceContinue()
{
    if(!canvas || !reinforcement) return;
    QMutexLocker lock(mutex);
    if(drawTimer)
    {
        drawTimer->Stop();
    }
    reinforcement->SetConverged(!reinforcement->hasConverged());

    emit UpdateInfo();

    if(drawTimer)
    {
        drawTimer->start(QThread::NormalPriority);
    }
}

