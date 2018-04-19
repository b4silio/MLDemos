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

void AlgorithmManager::Train(Regressor *regressor, int outputDim, float trainRatio, bvec trainList, std::vector<fvec> samples, ivec labels)
{
    if(!regressor || !canvas->data->GetCount()) return;

    ivec inputDims = GetInputDimensions();
    // Bug Regression crashing --- Guillaume
    if(inputDims.size() == 0){
        unsigned int nbDim = canvas->data->GetDimCount();
        inputDims.resize(nbDim);
        FOR(i,nbDim){
           inputDims[i]=i;
        }
    }

    int outputIndexInList = -1;
    if(inputDims.size()==1 && inputDims[0] == outputDim) return; // we dont have enough dimensions for training
    FOR(i, inputDims.size()) if(outputDim == inputDims[i])
    {
        outputIndexInList = i;
        break;
    }
    if(outputIndexInList == -1) inputDims.push_back(outputDim);
    outputIndexInList = inputDims.size()-1;
    sourceDims = inputDims;

    if(!samples.size()) samples = canvas->data->GetSampleDims(inputDims, outputIndexInList == -1 ? outputDim : -1);
    else samples = canvas->data->GetSampleDims(samples, inputDims, outputIndexInList == -1 ? outputDim : -1);
    if(!labels.size()) labels = canvas->data->GetLabels();

    if(!samples.size()) return;
    int dim = samples[0].size();
    if(dim < 2) return;

    regressor->SetOutputDim(outputDim);

    fvec trainErrors, testErrors;
    if(trainRatio == 1.f && !trainList.size())
    {
        regressor->Train(samples, labels);
        trainErrors.clear();
        FOR(i, samples.size())
        {
            fvec sample = samples[i];
            fvec res = regressor->Test(sample);
            float error = fabs(res[0] - sample.back());
            trainErrors.push_back(error);
        }
        regressor->trainErrors = trainErrors;
        regressor->testErrors.clear();
    }
    else
    {
        int trainCnt = (int)(samples.size()*trainRatio);
        int testCnt = samples.size() - trainCnt;
        u32 *perm = randPerm(samples.size());
        vector<fvec> trainSamples, testSamples;
        ivec trainLabels, testLabels;
        if(trainList.size())
        {
            FOR(i, trainList.size())
            {
                if(trainList[i])
                {
                    trainSamples.push_back(samples[i]);
                    trainLabels.push_back(labels[i]);
                }
                else
                {
                    testSamples.push_back(samples[i]);
                    testLabels.push_back(labels[i]);
                }
            }
            trainCnt = trainSamples.size();
            testCnt = testSamples.size();
        }
        else
        {
            trainSamples.resize(trainCnt);
            trainLabels.resize(trainCnt);
            testSamples.resize(testCnt);
            testLabels.resize(testCnt);
            FOR(i, trainCnt)
            {
                trainSamples[i] = samples[perm[i]];
                trainLabels[i] = labels[perm[i]];
            }
            FOR(i, testCnt)
            {
                testSamples[i] = samples[perm[i+trainCnt]];
                testLabels[i] = labels[perm[i+trainCnt]];
            }
        }
        regressor->Train(trainSamples, trainLabels);
        FOR(i, trainCnt)
        {
            fvec sample = trainSamples[i];
            fvec res = regressor->Test(sample);
            float error = fabs(res[0] - sample.back());
            trainErrors.push_back(error);
        }
        FOR(i, testCnt)
        {
            fvec sample = testSamples[i];
            fvec res = regressor->Test(sample);
            float error = fabs(res[0] - sample.back());
            testErrors.push_back(error);
            //qDebug() << " test error: " << i << error;
        }
        regressor->trainErrors = trainErrors;
        regressor->testErrors = testErrors;
        KILL(perm);
    }
    //bIsCrossNew = true;
}

void AlgorithmManager::Regression()
{
    if(!canvas || !canvas->data->GetCount()) return;
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
    if(!optionsRegress->algoList->count()) return;
    int tab = optionsRegress->algoList->currentIndex();
    if(tab >= regressors.size() || !regressors[tab]) return;
    int outputDim = optionsRegress->outputDimCombo->currentIndex();
    ivec inputDims = GetInputDimensions();
    //ivec inputDims = optionsRegress->inputDimButton->isChecked() ? GetInputDimensions() : ivec();
    if(inputDims.size()==1 && inputDims[0] == outputDim) return;

    int outputIndexInList = -1;
    FOR(i, inputDims.size()) if(outputDim == inputDims[i])
    {
        outputIndexInList = i;
        break;
    }
    if(outputDim != -1)
    {
        if(canvas->canvasType == 1)
        {
            //ui.canvasX3Spin->setValue(outputDim+1);
        }
        else if(canvas->canvasType == 0)
        {
            //ui.canvasX2Spin->setValue(outputDim+1);
        }
        emit DisplayOptionsChanged();
    }

    regressor = regressors[tab]->GetRegressor();
    tabUsedForTraining = tab;

    float ratios [] = {.1f,.25f,1.f/3.f,.5f,2.f/3.f,.75f,.9f,1.f};
    int ratioIndex = optionsRegress->traintestRatioCombo->currentIndex();
    float trainRatio = ratios[ratioIndex];

    vector<bool> trainList;
    if(optionsRegress->manualTrainButton->isChecked())
    {
        // we get the list of samples that are checked
        trainList = GetManualSelection();
    }

    Train(regressor, outputDim, trainRatio, trainList);
    regressors[tab]->Draw(canvas, regressor);
    glw->clearLists();
    if(canvas->canvasType == 1)
    {
        regressors[tab]->DrawGL(canvas, glw, regressor);
        // here we compute the regression plane for 3D datasets
        if(canvas->data->GetDimCount() == 3) Draw3DRegressor(glw, regressor);
    }
    // here we draw the errors for each sample
    if(canvas->data->GetDimCount() > 2 && canvas->canvasType == 0)
    {
        vector<fvec> samples = canvas->data->GetSamples();
        vector<fvec> subsamples = canvas->data->GetSampleDims(inputDims, outputIndexInList==-1 ? outputDim : -1);
        ivec labels = canvas->data->GetLabels();
        QPainter painter(&canvas->maps.model);
        painter.setRenderHint(QPainter::Antialiasing);
        // we draw the starting sample
        painter.setOpacity(0.4);
        painter.setPen(Qt::black);
        painter.setBrush(Qt::white);
        FOR(i, samples.size())
        {
            fvec sample = samples[i];
            QPointF point = canvas->toCanvasCoords(sample);
            painter.drawEllipse(point, 6,6);
        }
        // we draw the estimated sample
        painter.setPen(Qt::white);
        painter.setBrush(Qt::black);
        FOR(i, samples.size())
        {
            fvec sample = samples[i];
            fvec estimate = regressor->Test(subsamples[i]);
            sample[outputDim] = estimate[0];
            QPointF point2 = canvas->toCanvasCoords(sample);
            painter.drawEllipse(point2, 5,5);
        }
        painter.setOpacity(1);
        // we draw the error bars
        FOR(i, samples.size())
        {
            fvec sample = samples[i];
            fvec estimate = regressor->Test(subsamples[i]);
            QPointF point = canvas->toCanvasCoords(sample);
            sample[outputDim] = estimate[0];
            QPointF point2 = canvas->toCanvasCoords(sample);
            QColor color = SampleColor[labels[i]%SampleColorCnt];
            if(!labels[i]) color = Qt::black;
            painter.setPen(QPen(color, 1));
            painter.drawLine(point, point2);
        }
        canvas->repaint();
    }
    emit UpdateInfo();
}
