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
#include "mldemos.h"
#include "basicMath.h"
#include "classifier.h"
#include "regressor.h"
#include "dynamical.h"
#include "clusterer.h"
#include "maximize.h"
#include "roc.h"
#include <QDebug>
#include <fstream>
#include <QPixmap>
#include <QBitmap>
#include <QSettings>
#include <QMutexLocker>
#include <QFileDialog>
#include <QProgressDialog>
#include <qcontour.h>

using namespace std;

void MLDemos::Classify()
{
    if(!canvas || !canvas->data->GetCount()) return;
    drawTimer->Stop();
    drawTimer->Clear();
    mutex.lock();
    DEL(clusterer);
    DEL(regressor);
    DEL(dynamical);
    DEL(classifier);
    DEL(maximizer);
    DEL(reinforcement);
    DEL(projector);
    lastTrainingInfo = "";
    if(!optionsClassify->algoList->count()) return;
    int tab = optionsClassify->algoList->currentIndex();
    if(tab >= classifiers.size() || !classifiers[tab]) return;

    classifier = classifiers[tab]->GetClassifier();
    tabUsedForTraining = tab;
    float ratios [] = {.1f,.25f,1.f/3.f,.5f,2.f/3.f,.75f,.9f,1.f};
    int ratioIndex = optionsClassify->traintestRatioCombo->currentIndex();
    float trainRatio = ratios[ratioIndex];
    int positive = optionsClassify->positiveSpin->value();
    vector<bool> trainList;
    if(optionsClassify->manualTrainButton->isChecked())
    {
        // we get the list of samples that are checked
        trainList = GetManualSelection();
    }

    bool trained = Train(classifier, positive, trainRatio, trainList);
    if(trained)
    {
        classifiers[tab]->Draw(canvas, classifier);
        UpdateInfo();
        if(drawTimer && classifier->UsesDrawTimer())
        {
            drawTimer->classifier = &this->classifier;
            drawTimer->start(QThread::NormalPriority);
        }
        if(canvas->canvasType) CanvasOptionsChanged();
        // we fill in the canvas sampleColors
        vector<fvec> samples = canvas->data->GetSamples();
        canvas->sampleColors.resize(samples.size());
        FOR(i, samples.size())
        {
            canvas->sampleColors[i] = DrawTimer::GetColor(classifier, samples[i]);
        }
        if(canvas->canvasType)
        {
            canvas->maps.model = QPixmap();
            CanvasOptionsChanged();
        }
        canvas->repaint();
    }
    else
    {
        mutex.unlock();
        Clear();
        mutex.lock();
        UpdateInfo();
    }
    mutex.unlock();
}

vector<bool> MLDemos::GetManualSelection()
{
    vector<bool> trainList;
    if(!canvas || !canvas->data->GetCount()) return trainList;
    trainList.resize(manualSelection->sampleList->count(), false);
    QList<QListWidgetItem*> selected = manualSelection->sampleList->selectedItems();
    if(!selected.size()) // if nothing is selected we use all samples as training
    {
        trainList = vector<bool>(canvas->data->GetCount(), true);
        return trainList;
    }
    FOR(i, selected.size())
    {
        int index = manualSelection->sampleList->row(selected[i]);
        trainList[index] = true;
    }
    return trainList;
}

ivec MLDemos::GetInputDimensions()
{
    if(!canvas || !canvas->data->GetCount()) return ivec();
    QList<QListWidgetItem*> selected = inputDimensions->dimList->selectedItems();
    if(!selected.size() || selected.size() == inputDimensions->dimList->count()) return ivec(); // if nothing is selected we use all dimensions for training
    ivec dimList(selected.size());
    FOR(i, selected.size())
    {
        dimList[i] = inputDimensions->dimList->row(selected[i]);
    }
    return dimList;
}

void MLDemos::Regression()
{
    if(!canvas || !canvas->data->GetCount()) return;
    drawTimer->Stop();
    drawTimer->Clear();

    QMutexLocker lock(&mutex);
    DEL(clusterer);
    DEL(regressor);
    DEL(dynamical);
    DEL(classifier);
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
        ui.canvasX2Spin->setValue(outputDim+1);
        DisplayOptionChanged();
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
    }
    UpdateInfo();
}

void MLDemos::Dynamize()
{
    if(!canvas || !canvas->data->GetCount() || !canvas->data->GetSequences().size()) return;
    drawTimer->Stop();
    drawTimer->Clear();
    QMutexLocker lock(&mutex);
    DEL(clusterer);
    DEL(regressor);
    DEL(dynamical);
    DEL(classifier);
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
        QBitmap bitmap(w,h);
        bitmap.clear();
        canvas->maps.model.setMask(bitmap);
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
            pair<fvec,fvec> bounds = canvas->data->GetBounds();
            Expose::DrawTrajectories(canvas->maps.model, testTrajectories, vector<QColor>(), canvas->canvasType-1, 1, bounds);
        }
    }

    // the first index is "none", so we subtract 1
    int avoidIndex = optionsDynamic->obstacleCombo->currentIndex()-1;
    if(avoidIndex >=0 && avoidIndex < avoiders.size() && avoiders[avoidIndex])
    {
        DEL(dynamical->avoid);
        dynamical->avoid = avoiders[avoidIndex]->GetObstacleAvoidance();
    }
    UpdateInfo();
    if(dynamicals[tab]->UsesDrawTimer())
    {
        drawTimer->bColorMap = bColorMap;
        drawTimer->start(QThread::NormalPriority);
    }
}

void MLDemos::Avoidance()
{
    if(!canvas || !dynamical) return;
    if(!optionsDynamic->obstacleCombo->count()) return;
    drawTimer->Stop();
    QMutexLocker lock(&mutex);
    // the first index is "none", so we subtract 1
    int index = optionsDynamic->obstacleCombo->currentIndex()-1;
    if(index >=0 && index >= avoiders.size() || !avoiders[index]) return;
    DEL(dynamical->avoid);
    dynamical->avoid = avoiders[index]->GetObstacleAvoidance();
    UpdateInfo();
    drawTimer->Clear();
    drawTimer->start(QThread::NormalPriority);
}

fvec ClusterMetrics(std::vector<fvec> samples, ivec labels, std::vector<fvec> scores, float ratio = 1.f)
{
    fvec results(4, 0);
    results[0] = drand48();
    if(!samples.size() || !scores.size()) return results;
    int dim = samples[0].size();
    int nbClusters = scores[0].size();
    int count = samples.size();
    // compute bic
    double loglik = 0;

    vector<fvec> means(nbClusters);
    FOR(k, nbClusters)
    {
        means[k] = fvec(dim, 0);
        float contrib = 0;
        FOR(i, count)
        {
            contrib += scores[i][k];
            means[k] += samples[i]*scores[i][k];
        }
        means[k] /= contrib;
    }

    float log_lik=0;
    float like;
    float *pxi = new float[nbClusters];
    int data_i=0;
    int state_i;

    fvec loglikes(nbClusters);
    FOR(k, nbClusters)
    {
        float rss = 0;
        double contrib = 0;
        FOR(i, count)
        {
            contrib += scores[i][k];
            if(contrib==0) continue;
            fvec diff = samples[i]-means[k];
            rss += diff*diff*scores[i][k];
        }
        loglikes[k] = rss;
    }
    FOR(k, nbClusters) loglik += loglikes[k];
    //loglik /= nbClusters;

    results[0] = loglik; // RSS
    results[1] = log(count)*nbClusters + loglik; // BIC
    results[2] = 2*nbClusters + loglik; // AIC


    // we compute the f-measures for each class
    map<int,int> classcounts;
    int cnt = 0;
    FOR(i, labels.size()) if(!classcounts.count(labels[i])) classcounts[labels[i]] = cnt++;
    int classCount = classcounts.size();
    map<int, fvec> classScores;
    fvec clusterScores(nbClusters);
    map<int,float> labelScores;

    if(ratio == 1.f)
    {
        FOR(i, labels.size())
        {
            labelScores[labels[i]] += 1.f;
            if(!classScores.count(labels[i]))classScores[labels[i]].resize(nbClusters);
            FOR(k, nbClusters)
            {
                classScores[labels[i]][k] += scores[i][k];
                clusterScores[k] += scores[i][k];
            }
        }
    }
    else
    {
        u32 *perm = randPerm(labels.size());
        map<int, ivec> indices;
        FOR(i, labels.size()) indices[labels[perm[i]]].push_back(perm[i]);
        for(map<int,ivec>::iterator it = indices.begin(); it != indices.end(); it++)
        {
            int labelCount = max(1,int(it->second.size()*ratio));
            FOR(i, labelCount)
            {
                labelScores[labels[it->second[i]]] += 1.f;
                if(!classScores.count(labels[it->second[i]]))classScores[labels[it->second[i]]].resize(nbClusters);
                FOR(k, nbClusters)
                {
                    classScores[labels[it->second[i]]][k] += scores[it->second[i]][k];
                    clusterScores[k] += scores[it->second[i]][k];
                }
            }
        }
        delete [] perm;
    }

    float fmeasure = 0;
    map<int,float>::iterator it2 = labelScores.begin();
    for(map<int,fvec>::iterator it = classScores.begin(); it != classScores.end(); it++, it2++)
    {
        float maxScore = -FLT_MAX;
        FOR(k, nbClusters)
        {
            float precision = it->second[k] / it2->second;
            float recall = it->second[k] / clusterScores[k];
            float f1 = 2*precision*recall/(precision+recall);
            maxScore = max(maxScore,f1);
        }
        fmeasure += maxScore;
    }
    int classAndClusterCount = classCount;
    // we penalize empty clusters
    FOR(k, nbClusters) if(clusterScores[k] == 0) classAndClusterCount++; // we have an empty cluster!
    fmeasure /= classAndClusterCount;

    results[3] = -fmeasure; // F-Measure

    return results;
}


void MLDemos::Cluster()
{
    if(!canvas || !canvas->data->GetCount()) return;
    drawTimer->Stop();
    QMutexLocker lock(&mutex);
    DEL(clusterer);
    DEL(regressor);
    DEL(dynamical);
    DEL(classifier);
    DEL(maximizer);
    DEL(reinforcement);
    DEL(projector);
    lastTrainingInfo = "";
    if(!optionsCluster->algoList->count()) return;
    int tab = optionsCluster->algoList->currentIndex();
    if(tab >= clusterers.size() || !clusterers[tab]) return;
    clusterer = clusterers[tab]->GetClusterer();
    tabUsedForTraining = tab;
    vector<bool> trainList;
    float ratios [] = {.1f,.25f,1.f/3.f,.5f,2.f/3.f,.75f,.9f,1.f};
    int ratioIndex = optionsCluster->trainTestCombo->currentIndex();
    float trainRatio = ratios[ratioIndex];

    if(optionsCluster->manualTrainButton->isChecked())
    {
        // we get the list of samples that are checked
        trainList = GetManualSelection();
    }

    float testError;
    Train(clusterer, trainRatio, trainList, &testError);
    drawTimer->Stop();
    drawTimer->Clear();
    clusterers[tab]->Draw(canvas,clusterer);

    // we compute the stats on the clusters (f-measure, bic etc)

    vector<fvec> samples = canvas->data->GetSamples();
    ivec labels = canvas->data->GetLabels();
    vector<fvec> clusterScores(samples.size());
    FOR(i, canvas->data->GetCount())
    {
        fvec result = clusterer->Test(samples[i]);
        if(clusterer->NbClusters()==1) clusterScores[i] = result;
        else if(result.size()>1) clusterScores[i] = result;
        else if(result.size())
        {
            fvec res(clusterer->NbClusters(),0);
            res[result[0]] = 1.f;
        }
    }

    int f1ratioIndex = optionsCluster->trainRatioCombo->currentIndex();
    float f1ratios[] = {0.01f, 0.05f, 0.1f, 0.2f, 1.f/3.f, 0.5f, 0.75f, 1.f};
    float f1ratio = f1ratios[f1ratioIndex];

    fvec clusterMetrics = ClusterMetrics(samples, labels, clusterScores, f1ratio);

    optionsCluster->resultList->clear();
    optionsCluster->resultList->addItem(QString("rss: %1").arg(clusterMetrics[0], 0, 'f', 2));
    optionsCluster->resultList->addItem(QString("bic: %1").arg(clusterMetrics[1], 0, 'f', 2));
    optionsCluster->resultList->addItem(QString("aic: %1").arg(clusterMetrics[2], 0, 'f', 2));
    optionsCluster->resultList->addItem(QString("f1: %1").arg(clusterMetrics[3], 0, 'f', 2));
    FOR(i, clusterMetrics.size())
    {
        optionsCluster->resultList->item(i)->setForeground(i ? SampleColor[i%SampleColorCnt] : Qt::gray);
    }

    // we fill in the canvas sampleColors for the alternative display types
    canvas->sampleColors.resize(samples.size());
    FOR(i, samples.size())
    {
        fvec res = clusterer->Test(samples[i]);
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
        else if(res.size())
        {
            r = (1-res[0])*255 + res[0]* 255;
            g = (1-res[0])*255;
            b = (1-res[0])*255;
        }
        canvas->sampleColors[i] = QColor(r,g,b);
    }
    canvas->maps.model = QPixmap();
    canvas->repaint();

    UpdateInfo();
    QString infoText = showStats->infoText->text();
    infoText += "\nClustering as Classifier\n";
    infoText += QString("F-Measure: %1\n").arg(testError, 0, 'f', 3);
    showStats->infoText->setText(infoText);

    drawTimer->clusterer= &this->clusterer;
    drawTimer->start(QThread::NormalPriority);
}

void MLDemos::ClusterTest()
{
    if(!canvas || !canvas->data->GetCount()) return;
    drawTimer->Stop();
    drawTimer->Clear();
    QMutexLocker lock(&mutex);
    DEL(clusterer);
    DEL(regressor);
    DEL(dynamical);
    DEL(classifier);
    DEL(maximizer);
    DEL(reinforcement);
    DEL(projector);
    lastTrainingInfo = "";
    if(!optionsCluster->algoList->count()) return;
    int tab = optionsCluster->algoList->currentIndex();
    if(tab >= clusterers.size() || !clusterers[tab]) return;
    clusterer = clusterers[tab]->GetClusterer();
    tabUsedForTraining = tab;

    int startCount=optionsCluster->rangeStartSpin->value(), stopCount=optionsCluster->rangeStopSpin->value();
    if(startCount>stopCount) startCount ^= stopCount ^= startCount ^= stopCount;

    vector<fvec> samples = canvas->data->GetSamples();
    ivec labels = canvas->data->GetLabels();
    int f1ratioIndex = optionsCluster->trainRatioCombo->currentIndex();
    float f1ratios[] = {0.01f, 0.05f, 0.1f, 0.2f, 1.f/3.f, 0.5f, 0.75f, 1.f};
    float ratio = f1ratios[f1ratioIndex];

    float ratios [] = {.1f,.25f,1.f/3.f,.5f,2.f/3.f,.75f,.9f,1.f};
    int ratioIndex = optionsCluster->trainTestCombo->currentIndex();
    float trainRatio = ratios[ratioIndex];

    vector<bool> trainList;
    if(optionsCluster->manualTrainButton->isChecked())
    {
        // we get the list of samples that are checked
        trainList = GetManualSelection();
    }

    float testError = 0;
    int crossValCount = 5;
    fvec testErrors(crossValCount);

    FOR(j, crossValCount)
    {
        Train(clusterer, trainRatio, trainList, &testError);
        testErrors[j] = testError;
    }

    // we fill in the canvas sampleColors for the alternative display types
    canvas->sampleColors.resize(samples.size());
    FOR(i, samples.size())
    {
        fvec res = clusterer->Test(samples[i]);
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
        else if(res.size())
        {
            r = (1-res[0])*255 + res[0]* 255;
            g = (1-res[0])*255;
            b = (1-res[0])*255;
        }
        canvas->sampleColors[i] = QColor(r,g,b);
    }
    canvas->maps.model = QPixmap();
    clusterers[tab]->Draw(canvas, clusterer);

    UpdateInfo();
    QString infoText = showStats->infoText->text();
    infoText += "\nClustering as Classifier\nF-Measures:\n";

    float mean=0, stdev=0;
    FOR(j, testErrors.size()) mean += testErrors[j];
    mean /= testErrors.size();
    FOR(j, testErrors.size()) stdev += (testErrors[j] - mean)*(testErrors[j] - mean);
    stdev /= testErrors.size();
    stdev = sqrtf(stdev);
    infoText += QString("%1 clusters: %2 (+- %3)\n").arg(clusterer->NbClusters()).arg(mean, 0, 'f', 3).arg(stdev, 0, 'f', 3);

    showStats->infoText->setText(infoText);
    showStats->tabWidget->setCurrentIndex(1); // we show the info panel
    statsDialog->show();

    drawTimer->clusterer= &this->clusterer;
    drawTimer->start(QThread::NormalPriority);
    canvas->repaint();
}

void MLDemos::ClusterOptimize()
{
    if(!canvas || !canvas->data->GetCount()) return;
    drawTimer->Stop();
    drawTimer->Clear();
    QMutexLocker lock(&mutex);
    DEL(clusterer);
    DEL(regressor);
    DEL(dynamical);
    DEL(classifier);
    DEL(maximizer);
    DEL(reinforcement);
    DEL(projector);
    lastTrainingInfo = "";
    if(!optionsCluster->algoList->count()) return;
    int tab = optionsCluster->algoList->currentIndex();
    if(tab >= clusterers.size() || !clusterers[tab]) return;
    clusterer = clusterers[tab]->GetClusterer();
    tabUsedForTraining = tab;

    int startCount=optionsCluster->rangeStartSpin->value(), stopCount=optionsCluster->rangeStopSpin->value();
    if(startCount>stopCount) startCount ^= stopCount ^= startCount ^= stopCount;

    vector<fvec> samples = canvas->data->GetSamples();
    ivec labels = canvas->data->GetLabels();
    int f1ratioIndex = optionsCluster->trainRatioCombo->currentIndex();
    float f1ratios[] = {0.01f, 0.05f, 0.1f, 0.2f, 1.f/3.f, 0.5f, 0.75f, 1.f};
    float ratio = f1ratios[f1ratioIndex];

    float ratios [] = {.1f,.25f,1.f/3.f,.5f,2.f/3.f,.75f,.9f,1.f};
    int ratioIndex = optionsCluster->trainTestCombo->currentIndex();
    float trainRatio = ratios[ratioIndex];

    vector<bool> trainList;
    if(optionsCluster->manualTrainButton->isChecked())
    {
        // we get the list of samples that are checked
        trainList = GetManualSelection();
    }

    float testError = 0;
    ivec kCounts;
    vector< vector<fvec> > resultList(4);
    vector<fvec> testErrors(stopCount-startCount+1);
    int crossValCount = 5;
    FOR(i, resultList.size()) resultList[i].resize(crossValCount);
    for(int k=startCount; k<=stopCount; k++)
    {
        clusterer->SetNbClusters(k);
        testErrors[k-startCount].resize(crossValCount);
        FOR(j, crossValCount)
        {
            Train(clusterer, trainRatio, trainList, &testError);
            testErrors[k-startCount][j] = testError;

            int folds = 10;
            fvec metricMeans(resultList.size());
            ivec foldCount(resultList.size());
            FOR(f, folds)
            {
                vector<fvec> clusterScores(samples.size());
                FOR(i, canvas->data->GetCount())
                {
                    fvec result = clusterer->Test(samples[i]);
                    if(clusterer->NbClusters()==1) clusterScores[i] = result;
                    else if(result.size()>1) clusterScores[i] = result;
                    else if(result.size())
                    {
                        fvec res(clusterer->NbClusters(),0);
                        res[result[0]] = 1.f;
                    }
                }
                fvec clusterMetrics = ClusterMetrics(samples, labels, clusterScores, ratio);
                FOR(d, clusterMetrics.size())
                {
                    if(clusterMetrics[d] != clusterMetrics[d]) continue; // not a number
                    metricMeans[d] += clusterMetrics[d];
                    foldCount[d]++;
                }
            }
            FOR(d, metricMeans.size()) metricMeans[d] /= foldCount[d];
            FOR(i, metricMeans.size())
            {
                resultList[i][j].push_back(metricMeans[i]);
            }
        }
        kCounts.push_back(k);
    }

    vector<fvec> results(4);
    FOR(i, resultList.size())
    {
        results[i].resize(resultList[i][0].size());
        FOR(k, resultList[i][0].size())
        {
            double value = 0;
            FOR(j, crossValCount)
            {
                value += resultList[i][j][k];
            }
            value /= crossValCount;
            results[i][k] = value;
        }
    }

    int w = optionsCluster->graphLabel->width();
    int h = optionsCluster->graphLabel->height();
    int pad = 6;
    QPixmap pixmap(w,h);
    QBitmap bitmap(w,h);
    bitmap.clear();
    pixmap.setMask(bitmap);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);

    painter.setPen(QPen(Qt::black, 1.f));
    painter.drawLine(pad, h - 2*pad, w-pad, h-2*pad);
    painter.drawLine(pad, 0, pad, h-2*pad);
    QFont font = painter.font();
    font.setPointSize(9);
    painter.setFont(font);
    FOR(k, kCounts.size())
    {
        float x = k/(float)(kCounts.size()-1);
        painter.drawLine(x*(w-2*pad)+pad, h-2*pad-1, x*(w-2*pad)+pad, h-2*pad+1);
        if(k == kCounts.size()-1) x -= 0.05;
        painter.drawText(x*(w-2*pad)-2+pad, h-1, QString("%1").arg(kCounts[k]));
    }

    painter.setRenderHint(QPainter::Antialiasing);
    fvec mins(results.size(), FLT_MAX), maxes(results.size(), -FLT_MAX);
    FOR(i, results.size())
    {
        FOR(j, results[i].size())
        {
            mins[i] = min(mins[i], results[i][j]);
            maxes[i] = max(maxes[i], results[i][j]);
        }
    }

    vector< pair<float,int> > bests(results.size());
    FOR(i, results.size())
    {
        QPointF old;
        painter.setPen(QPen(i ? SampleColor[i%SampleColorCnt] : Qt::gray,2));
        bests[i] = make_pair(FLT_MAX, 0);
        FOR(k, kCounts.size())
        {
            if(results[i][k] < bests[i].first)
            {
                bests[i] = make_pair(results[i][k], kCounts[k]);
            }
            float x = k/(float)(kCounts.size()-1);
            float y = (results[i][k] - mins[i])/(maxes[i]-mins[i]);
            //if(i == 3) y = 1.f - y; // fmeasures needs to be maximized
            QPointF point(x*(w-2*pad)+pad, (1.f-y)*(h-2*pad));
            if(k) painter.drawLine(old, point);
            old = point;
        }
    }
    optionsCluster->graphLabel->setPixmap(pixmap);

    optionsCluster->resultList->clear();
    optionsCluster->resultList->addItem(QString("rss: %1 (%2)").arg(bests[0].second).arg(bests[0].first, 0, 'f', 2));
    optionsCluster->resultList->addItem(QString("bic: %1 (%2)").arg(bests[1].second).arg(bests[1].first, 0, 'f', 2));
    optionsCluster->resultList->addItem(QString("aic: %1 (%2)").arg(bests[2].second).arg(bests[2].first, 0, 'f', 2));
    optionsCluster->resultList->addItem(QString("f1: %1 (%2)").arg(bests[3].second).arg(bests[3].first, 0, 'f', 2));
    FOR(i, results.size())
    {
        optionsCluster->resultList->item(i)->setForeground(i ? SampleColor[i%SampleColorCnt] : Qt::gray);
    }

    int bestIndex = optionsCluster->optimizeCombo->currentIndex();
    clusterer->SetNbClusters(bests[bestIndex].second);
    Train(clusterer, trainRatio, trainList, &testError);

    // we fill in the canvas sampleColors for the alternative display types
    canvas->sampleColors.resize(samples.size());
    FOR(i, samples.size())
    {
        fvec res = clusterer->Test(samples[i]);
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
        else if(res.size())
        {
            r = (1-res[0])*255 + res[0]* 255;
            g = (1-res[0])*255;
            b = (1-res[0])*255;
        }
        canvas->sampleColors[i] = QColor(r,g,b);
    }
    canvas->maps.model = QPixmap();

    clusterers[tab]->Draw(canvas, clusterer);
    UpdateInfo();
    QString infoText = showStats->infoText->text();
    infoText += "\nClustering as Classifier\nF-Measures:\n";
    FOR(i, testErrors.size())
    {
        float mean=0, stdev=0;
        FOR(j, testErrors[i].size()) mean += testErrors[i][j];
        mean /= testErrors[i].size();
        FOR(j, testErrors[i].size()) stdev += (testErrors[i][j] - mean)*(testErrors[i][j] - mean);
        stdev /= testErrors[i].size();
        stdev = sqrtf(stdev);
        infoText += QString("%1 clusters: %2 (+- %3)\n").arg(i+1).arg(mean, 0, 'f', 3).arg(stdev, 0, 'f', 3);
    }
    showStats->infoText->setText(infoText);

    drawTimer->clusterer= &this->clusterer;
    drawTimer->start(QThread::NormalPriority);
    canvas->repaint();
}

void MLDemos::ClusterIterate()
{
    if(!canvas || !canvas->data->GetCount()) return;
    drawTimer->Stop();
    int tab = optionsCluster->algoList->currentIndex();
    if(tab >= clusterers.size() || !clusterers[tab]) return;
    QMutexLocker lock(&mutex);
    if(!clusterer)
    {
        clusterer = clusterers[tab]->GetClusterer();
        tabUsedForTraining = tab;
    }
    else clusterers[tab]->SetParams(clusterer);
    clusterer->SetIterative(true);
    Train(clusterer);
    clusterers[tab]->Draw(canvas,clusterer);

    // we fill in the canvas sampleColors
    vector<fvec> samples = canvas->data->GetSamples();
    canvas->sampleColors.resize(samples.size());
    FOR(i, samples.size())
    {
        fvec res = clusterer->Test(samples[i]);
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
        else if(res.size())
        {
            r = (1-res[0])*255 + res[0]* 255;
            g = (1-res[0])*255;
            b = (1-res[0])*255;
        }
        canvas->sampleColors[i] = QColor(r,g,b);
    }
    canvas->maps.model = QPixmap();
    canvas->repaint();

    UpdateInfo();
}

void MLDemos::Maximize()
{
    if(!canvas) return;
    if(canvas->maps.reward.isNull()) return;
    QMutexLocker lock(&mutex);
    drawTimer->Stop();
    DEL(clusterer);
    DEL(regressor);
    DEL(dynamical);
    DEL(classifier);
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
    QBitmap bitmap(canvas->width(), canvas->height());
    canvas->maps.info.setMask(bitmap);
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

    UpdateInfo();
    drawTimer->Stop();
    drawTimer->Clear();
    drawTimer->start(QThread::NormalPriority);
}

void MLDemos::MaximizeContinue()
{
    if(!canvas || !maximizer) return;
    QMutexLocker lock(&mutex);
    if(drawTimer)
    {
        drawTimer->Stop();
    }
    maximizer->SetConverged(!maximizer->hasConverged());

    UpdateInfo();
    if(drawTimer)
    {
        drawTimer->start(QThread::NormalPriority);
    }
}

void MLDemos::Reinforce()
{
    if(!canvas) return;
    if(canvas->maps.reward.isNull()) return;
    QMutexLocker lock(&mutex);
    drawTimer->Stop();
    DEL(clusterer);
    DEL(regressor);
    DEL(dynamical);
    DEL(classifier);
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

    // we draw the contours for the current maximization
    int w = 65;
    int h = 65;
    int W = canvas->width();
    int H = canvas->height();
    canvas->maps.info = QPixmap(W, H);
    QBitmap bitmap(canvas->width(), canvas->height());
    canvas->maps.info.setMask(bitmap);
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

    UpdateInfo();
    drawTimer->Stop();
    drawTimer->Clear();
    drawTimer->start(QThread::NormalPriority);
}

void MLDemos::ReinforceContinue()
{
    if(!canvas || !reinforcement) return;
    QMutexLocker lock(&mutex);
    if(drawTimer)
    {
        drawTimer->Stop();
    }
    reinforcement->SetConverged(!reinforcement->hasConverged());

    UpdateInfo();
    if(drawTimer)
    {
        drawTimer->start(QThread::NormalPriority);
    }
}

void MLDemos::Project()
{
    if(!canvas || !canvas->data->GetCount()) return;
    QMutexLocker lock(&mutex);
    drawTimer->Stop();
    drawTimer->Clear();
    DEL(clusterer);
    DEL(regressor);
    DEL(dynamical);
    DEL(classifier);
    DEL(maximizer);
    DEL(reinforcement);
    DEL(projector);
    lastTrainingInfo = "";
    if(!optionsProject->algoList->count()) return;
    int tab = optionsProject->algoList->currentIndex();
    if(tab >= projectors.size() || !projectors[tab]) return;
    projector = projectors[tab]->GetProjector();
    projectors[tab]->SetParams(projector);
    tabUsedForTraining = tab;
    bool bHasSource = false;
    if(sourceData.size() && sourceData.size() == canvas->data->GetCount())
    {
        bHasSource = true;
        canvas->data->SetSamples(sourceData);
        canvas->data->SetLabels(sourceLabels);
    }
    vector<bool> trainList;
    if(optionsProject->manualTrainButton->isChecked())
    {
        // we get the list of samples that are checked
        trainList = GetManualSelection();
    }
    Train(projector, trainList);
    if(!bHasSource)
    {
        sourceData = canvas->data->GetSamples();
        sourceLabels = canvas->data->GetLabels();
    }
    projectedData = projector->GetProjected();
    if(projectedData.size())
    {
        canvas->data->SetSamples(projectedData);
        canvas->data->bProjected = true;
    }
    if(optionsProject->fitCheck->isChecked()) canvas->FitToData();
    CanvasTypeChanged();
    CanvasOptionsChanged();
    ResetPositiveClass();
    projectors[tab]->Draw(canvas, projector);
    canvas->repaint();
    UpdateInfo();
}

void MLDemos::ProjectRevert()
{
    QMutexLocker lock(&mutex);
    drawTimer->Stop();
    drawTimer->Clear();
    DEL(clusterer);
    DEL(regressor);
    DEL(dynamical);
    DEL(classifier);
    DEL(maximizer);
    DEL(reinforcement);
    DEL(projector);
    lastTrainingInfo = "";
    if(!sourceData.size()) return;
    canvas->data->SetSamples(sourceData);
    canvas->data->SetLabels(sourceLabels);
    canvas->data->bProjected = false;
    canvas->maps.info = QPixmap();
    canvas->maps.model = QPixmap();
    canvas->maps.confidence = QPixmap();
    if(optionsProject->fitCheck->isChecked()) canvas->FitToData();
    CanvasTypeChanged();
    CanvasOptionsChanged();
    ResetPositiveClass();
    canvas->repaint();
    UpdateInfo();
    sourceData.clear();
    sourceLabels.clear();
}

void MLDemos::ProjectReproject()
{
    if(!canvas || !canvas->data->GetCount()) return;
    mutex.lock();
    sourceData = canvas->data->GetSamples();
    sourceLabels = canvas->data->GetLabels();
    mutex.unlock();
    Project();
}

void MLDemos::ExportAnimation()
{
    if(!canvas->data->GetSamples().size()) return;
}


void MLDemos::UpdateLearnedModel()
{
    if(!canvas) return;
    if(!clusterer && !regressor && !dynamical && !classifier && !projector) return;
    if(classifier)
    {
        QMutexLocker lock(&mutex);
        classifiers[tabUsedForTraining]->Draw(canvas, classifier);
        if(classifier->UsesDrawTimer() && !drawTimer->isRunning())
        {
            drawTimer->start(QThread::NormalPriority);
        }
    }
    if(clusterer)
    {
        QMutexLocker lock(&mutex);
        clusterers[tabUsedForTraining]->Draw(canvas, clusterer);
    }
    if(regressor)
    {
        QMutexLocker lock(&mutex);
        regressors[tabUsedForTraining]->Draw(canvas, regressor);
        // here we draw the errors for each sample
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
        }
    }
    if(dynamical)
    {
        QMutexLocker lock(&mutex);
        dynamicals[tabUsedForTraining]->Draw(canvas, dynamical);
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
            QBitmap bitmap(w,h);
            bitmap.clear();
            canvas->maps.model.setMask(bitmap);
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
                pair<fvec,fvec> bounds = canvas->data->GetBounds();
                Expose::DrawTrajectories(canvas->maps.model, testTrajectories, vector<QColor>(), canvas->canvasType-1, 1, bounds);
            }
        }

        // the first index is "none", so we subtract 1
        int avoidIndex = optionsDynamic->obstacleCombo->currentIndex()-1;
        if(avoidIndex >=0 && avoidIndex < avoiders.size() && avoiders[avoidIndex])
        {
            DEL(dynamical->avoid);
            dynamical->avoid = avoiders[avoidIndex]->GetObstacleAvoidance();
        }
        UpdateInfo();
        if(dynamicals[tabUsedForTraining]->UsesDrawTimer())
        {
            drawTimer->bColorMap = bColorMap;
            drawTimer->start(QThread::NormalPriority);
        }

    }
    if(!canvas->canvasType && projector)
    {
        projectors[tabUsedForTraining]->Draw(canvas, projector);
    }
    UpdateInfo();
}
