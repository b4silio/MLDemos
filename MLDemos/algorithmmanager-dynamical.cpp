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

using namespace std;

void AlgorithmManager::Dynamize()
{
    if(!canvas || !canvas->data->GetCount() || !canvas->data->GetSequences().size()) return;
    drawTimer->Stop();
    drawTimer->Clear();
    QMutexLocker lock(mutex);
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
    if(!optionsDynamic->algoList->count()) return;
    int tab = optionsDynamic->algoList->currentIndex();
    if(tab >= dynamicals.size() || !dynamicals[tab]) return;
    dynamical = dynamicals[tab]->GetDynamical();
    tabUsedForTraining = tab;

    Train(dynamical);
    dynamicals[tab]->Draw(canvas,dynamical);
    glw->clearLists();
    if(canvas->canvasType == 1)
    {
        dynamicals[tab]->DrawGL(canvas, glw, dynamical);
        if(canvas->data->GetDimCount() == 3)
        {
            int displayStyle = optionsDynamic->displayCombo->currentIndex();
            if(displayStyle == 3) // DS animation
            {
            }
            else Draw3DDynamical(glw, dynamical, displayStyle);
        }
    }

    int w = canvas->width(), h = canvas->height();

    int resampleType = optionsDynamic->resampleCombo->currentIndex();
    int resampleCount = optionsDynamic->resampleSpin->value();
    int centerType = optionsDynamic->centerCombo->currentIndex();
    float dT = optionsDynamic->dtSpin->value();
    int zeroEnding = optionsDynamic->zeroCheck->isChecked();
    bool bColorMap = optionsDynamic->colorCheck->isChecked();

    // we draw the current trajectories
    vector< vector<fvec> > trajectories = canvas->data->GetTrajectories(resampleType, resampleCount, centerType, dT, zeroEnding);
    vector< vector<fvec> > testTrajectories;
    int steps = 300;
    if(trajectories.size())
    {
        testTrajectories.resize(trajectories.size());
        int dim = trajectories[0][0].size() / 2;
        FOR(i, trajectories.size())
        {
            fvec start(dim,0);
            FOR(d, dim) start[d] = trajectories[i][0][d];
            vector<fvec> result = dynamical->Test(start, steps);
            testTrajectories[i] = result;
        }
        canvas->maps.model = QPixmap(w,h);
        //QBitmap bitmap(w,h);
        //bitmap.clear();
        //canvas->maps.model.setMask(bitmap);
        canvas->maps.model.fill(Qt::transparent);

        if(canvas->canvasType == 0) // standard canvas
        {
            /*
            QPainter painter(&canvas->maps.model);
            painter.setRenderHint(QPainter::Antialiasing);
            FOR(i, testTrajectories.size())
            {
                vector<fvec> &result = testTrajectories[i];
                fvec oldPt = result[0];
                int count = result.size();
                FOR(j, count-1)
                {
                    fvec pt = result[j+1];
                    painter.setPen(QPen(Qt::green, 2));
                    painter.drawLine(canvas->toCanvasCoords(pt), canvas->toCanvasCoords(oldPt));
                    oldPt = pt;
                }
                painter.setBrush(Qt::NoBrush);
                painter.setPen(Qt::green);
                painter.drawEllipse(canvas->toCanvasCoords(result[0]), 5, 5);
                painter.setPen(Qt::red);
                painter.drawEllipse(canvas->toCanvasCoords(result[count-1]), 5, 5);
            }
            */
        }
        else
        {
            //pair<fvec,fvec> bounds = canvas->data->GetBounds();
            //Expose::DrawTrajectories(canvas->maps.model, testTrajectories, vector<QColor>(), canvas->canvasType-1, 1, bounds);
        }
    }

    // the first index is "none", so we subtract 1
    int avoidIndex = optionsDynamic->obstacleCombo->currentIndex()-1;
    if(avoidIndex >=0 && avoidIndex < avoiders.size() && avoiders[avoidIndex])
    {
        DEL(dynamical->avoid);
        dynamical->avoid = avoiders[avoidIndex]->GetObstacleAvoidance();
    }
    emit UpdateInfo();


    //Draw2DDynamical(canvas, dynamical);
    if(dynamicals[tab]->UsesDrawTimer())
    {
        drawTimer->bColorMap = bColorMap;
        drawTimer->start(QThread::NormalPriority);
    }
}

void AlgorithmManager::Avoidance()
{
    if(!canvas || !dynamical) return;
    if(!optionsDynamic->obstacleCombo->count()) return;
    drawTimer->Stop();
    QMutexLocker lock(mutex);
    // the first index is "none", so we subtract 1
    int index = optionsDynamic->obstacleCombo->currentIndex()-1;
    if(index >=0 && index >= avoiders.size() || !avoiders[index]) return;
    DEL(dynamical->avoid);
    dynamical->avoid = avoiders[index]->GetObstacleAvoidance();
    emit UpdateInfo();

    drawTimer->Clear();
    drawTimer->start(QThread::NormalPriority);
}

// returns respectively the reconstruction error for the training points individually, per trajectory, and the error to target
fvec AlgorithmManager::Train(Dynamical *dynamical)
{
    if(!dynamical) return fvec();
    vector<fvec> samples = canvas->data->GetSamples();
    vector<ipair> sequences = canvas->data->GetSequences();
    ivec labels = canvas->data->GetLabels();
    if(!samples.size() || !sequences.size()) return fvec();
    int dim = samples[0].size();
    int count = optionsDynamic->resampleSpin->value();
    int resampleType = optionsDynamic->resampleCombo->currentIndex();
    int centerType = optionsDynamic->centerCombo->currentIndex();
    bool zeroEnding = optionsDynamic->zeroCheck->isChecked();

    ivec trajLabels(sequences.size());
    FOR(i, sequences.size())
    {
        trajLabels[i] = canvas->data->GetLabel(sequences[i].first);
    }

    //float dT = 10.f; // time span between each data frame
    float dT = optionsDynamic->dtSpin->value();
    dynamical->dT = dT;
    //dT = 10.f;
    vector< vector<fvec> > trajectories = canvas->data->GetTrajectories(resampleType, count, centerType, dT, zeroEnding);
    interpolate(trajectories[0],count);

    dynamical->Train(trajectories, trajLabels);
    return Test(dynamical, trajectories, trajLabels);
}

// returns respectively the reconstruction error for the training points individually, per trajectory, and the error to target
fvec AlgorithmManager::Test(Dynamical *dynamical, vector< vector<fvec> > trajectories, ivec labels)
{
    if(!dynamical || !trajectories.size()) return fvec();
    int dim = trajectories[0][0].size()/2;
    //(int dim = dynamical->Dim();
    float dT = dynamical->dT;
    fvec sample; sample.resize(dim,0);
    fvec vTrue; vTrue.resize(dim, 0);
    fvec xMin(dim, FLT_MAX);
    fvec xMax(dim, -FLT_MAX);

    // test each trajectory for errors
    int errorCnt=0;
    float errorOne = 0, errorAll = 0;
    FOR(i, trajectories.size())
    {
        vector<fvec> t = trajectories[i];
        float errorTraj = 0;
        FOR(j, t.size())
        {
            FOR(d, dim)
            {
                sample[d] = t[j][d];
                vTrue[d] = t[j][d+dim];
                if(xMin[d] > sample[d]) xMin[d] = sample[d];
                if(xMax[d] < sample[d]) xMax[d] = sample[d];
            }
            fvec v = dynamical->Test(sample);
            float error = 0;
            FOR(d, dim) error += (v[d] - vTrue[d])*(v[d] - vTrue[d]);
            errorTraj += error;
            errorCnt++;
        }
        errorOne += errorTraj;
        errorAll += errorTraj / t.size();
    }
    errorOne /= errorCnt;
    errorAll /= trajectories.size();
    fvec res;
    res.push_back(errorOne);

    vector<fvec> endpoints;

    float errorTarget = 0;
    // test each trajectory for target
    fvec pos(dim), end(dim);
    FOR(i, trajectories.size())
    {
        FOR(d, dim)
        {
            pos[d] = trajectories[i].front()[d];
            end[d] = trajectories[i].back()[d];
        }
        if(!endpoints.size()) endpoints.push_back(end);
        else
        {
            bool bExists = false;
            FOR(j, endpoints.size())
            {
                if(endpoints[j] == end)
                {
                    bExists = true;
                    break;
                }
            }
            if(!bExists) endpoints.push_back(end);
        }
        int steps = 500;
        float eps = FLT_MIN;
        FOR(j, steps)
        {
            fvec v = dynamical->Test(pos);
            float speed = 0;
            FOR(d, dim) speed += v[d]*v[d];
            speed = sqrtf(speed);
            if(speed*dT < eps) break;
            pos += v*dT;
        }
        float error = 0;
        FOR(d, dim)
        {
            error += (pos[d] - end[d])*(pos[d] - end[d]);
        }
        error = sqrtf(error);
        errorTarget += error;
    }
    errorTarget /= trajectories.size();
    res.push_back(errorTarget);

    fvec xDiff = xMax - xMin;
    errorTarget = 0;
    int testCount = 100;
    FOR(i, testCount)
    {
        FOR(d, dim)
        {
            pos[d] = ((drand48()*2 - 0.5)*xDiff[d] + xMin[d]);
        }

        int steps = 500;
        float eps = FLT_MIN;
        FOR(j, steps)
        {
            fvec v = dynamical->Test(pos);
            float speed = 0;
            FOR(d, dim) speed += v[d]*v[d];
            speed = sqrtf(speed);
            if(speed*dT < eps) break;
            pos += v*dT;
        }
        float minError = FLT_MAX;
        FOR(j, endpoints.size())
        {
            float error = 0;
            FOR(d, dim)
            {
                error += (pos[d] - endpoints[j][d])*(pos[d] - endpoints[j][d]);
            }
            error = sqrtf(error);
            if(minError > error) minError = error;
        }
        errorTarget += minError;
    }
    errorTarget /= testCount;
    res.push_back(errorTarget);

    return res;
}

