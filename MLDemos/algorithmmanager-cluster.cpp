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
#include "mldemos.h"

using namespace std;

float AlgorithmManager::ClusterFMeasure(std::vector<fvec> samples, ivec labels, std::vector<fvec> scores, float ratio)
{
    if(!samples.size() || !scores.size()) return 0;
    int nbClusters = scores[0].size();

    // we compute the f-measures for each class
    map<int,int> classcounts;
    int cnt = 0;
    FOR(i, labels.size()) if(!classcounts.count(labels[i])) classcounts[labels[i]] = cnt++;
    int classCount = classcounts.size();
    map<int, fvec> classScores;
    fvec clusterScores(nbClusters);
    map<int,float> labelScores;

    if(ratio == 1.f) {
        FOR(i, labels.size()) {
            labelScores[labels[i]] += 1.f;
            if(!classScores.count(labels[i]))classScores[labels[i]].resize(nbClusters);
            FOR(k, nbClusters) {
                classScores[labels[i]][k] += scores[i][k];
                clusterScores[k] += scores[i][k];
            }
        }
    } else {
        u32 *perm = randPerm(labels.size());
        map<int, ivec> indices;
        FOR(i, labels.size()) indices[labels[perm[i]]].push_back(perm[i]);
        for(map<int,ivec>::iterator it = indices.begin(); it != indices.end(); it++) {
            int labelCount = max(1,int(it->second.size()*ratio));
            FOR(i, labelCount) {
                labelScores[labels[it->second[i]]] += 1.f;
                if(!classScores.count(labels[it->second[i]]))classScores[labels[it->second[i]]].resize(nbClusters);
                FOR(k, nbClusters) {
                    classScores[labels[it->second[i]]][k] += scores[it->second[i]][k];
                    clusterScores[k] += scores[it->second[i]][k];
                }
            }
        }
        delete [] perm;
    }

    float fmeasure = 0;
    map<int,float>::iterator it2 = labelScores.begin();
    for(map<int,fvec>::iterator it = classScores.begin(); it != classScores.end(); it++, it2++) {
        float maxScore = -FLT_MAX;
        FOR(k, nbClusters) {
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

    return -fmeasure;
}

void AlgorithmManager::Cluster()
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
    if(!optionsCluster->algoList->count()) return;
    int tab = optionsCluster->algoList->currentIndex();
    if(tab >= clusterers.size() || !clusterers[tab]) return;
    clusterer = clusterers[tab]->GetClusterer();
    tabUsedForTraining = tab;
    vector<bool> trainList;
    float ratios [] = {.1f,.25f,1.f/3.f,.5f,2.f/3.f,.75f,.9f,1.f};
    int ratioIndex = optionsCluster->trainTestCombo->currentIndex();
    float trainRatio = ratios[ratioIndex];

    if(optionsCluster->manualTrainButton->isChecked()) {
        // we get the list of samples that are checked
        trainList = GetManualSelection();
    }

    float testError;
    Train(clusterer, trainRatio, trainList, &testError);

    // we compute the stats on the clusters (f-measure, bic etc)
    ivec inputDims = GetInputDimensions();
    vector<fvec> samples = canvas->data->GetSampleDims(inputDims);
    ivec labels = canvas->data->GetLabels();
    float logL = clusterer->GetLogLikelihood(samples);
    float n = samples.size();
    float k = clusterer->GetParameterCount();
    float BIC = -2*logL + log(n)*k;
    float AIC = -2*logL + 2*k;
    float AICc = AIC + 2*(k*k + k)/(n-k-1);

    int f1ratioIndex = optionsCluster->trainRatioCombo->currentIndex();
    float f1ratios[] = {0.01f, 0.05f, 0.1f, 0.2f, 1.f/3.f, 0.5f, 0.75f, 1.f};
    float f1ratio = f1ratios[f1ratioIndex];
    vector<fvec> clusterScores(samples.size());
    FOR(i, samples.size())
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
    float F1 = ClusterFMeasure(samples, labels, clusterScores, f1ratio);

    optionsCluster->resultList->clear();
    optionsCluster->resultList->addItem(QString("Lik: %1").arg(logL, 0, 'f', 2));
    optionsCluster->resultList->addItem(QString("BIC: %1").arg(BIC, 0, 'f', 2));
    optionsCluster->resultList->addItem(QString("AIC: %1").arg(AIC, 0, 'f', 2));
    optionsCluster->resultList->addItem(QString("AICc: %1").arg(AICc, 0, 'f', 2));
    optionsCluster->resultList->addItem(QString("F1: %1").arg(F1, 0, 'f', 2));

    optionsCluster->resultList->item(0)->setForeground(Qt::gray);
    optionsCluster->resultList->item(1)->setForeground(Qt::red);
    optionsCluster->resultList->item(2)->setForeground(Qt::blue);
    optionsCluster->resultList->item(3)->setForeground(Qt::magenta);
    optionsCluster->resultList->item(4)->setForeground(QColor(255,128,0)); // orange

    clusterers[tab]->Draw(canvas,clusterer);
    glw->clearLists();
    if(canvas->canvasType == 1)
    {
        clusterers[tab]->DrawGL(canvas, glw, clusterer);
        if(canvas->data->GetDimCount() == 3) Draw3DClusterer(glw, clusterer);
    }

    // we fill in the canvas sampleColors for the alternative display types
    if(canvas->canvasType != 0) {
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
    }
    canvas->repaint();

    emit UpdateInfo();
    /*
    QString infoText = showStats->infoText->text();
    infoText += "\nClustering as Classifier\n";
    infoText += QString("F-Measure: %1\n").arg(testError, 0, 'f', 3);
    showStats->infoText->setText(infoText);
    */

    drawTimer->clusterer= &this->clusterer;
    drawTimer->inputDims = GetInputDimensions();
    drawTimer->start(QThread::NormalPriority);
}

void AlgorithmManager::ClusterTest()
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
    if(!optionsCluster->algoList->count()) return;
    int tab = optionsCluster->algoList->currentIndex();
    if(tab >= clusterers.size() || !clusterers[tab]) return;
    clusterer = clusterers[tab]->GetClusterer();
    tabUsedForTraining = tab;

    //int startCount=optionsCluster->rangeStartSpin->value(), stopCount=optionsCluster->rangeStopSpin->value();
    //if(startCount>stopCount) startCount ^= stopCount ^= startCount ^= stopCount;

    ivec inputDims = GetInputDimensions();
    vector<fvec> samples = canvas->data->GetSampleDims(inputDims);
    //ivec labels = canvas->data->GetLabels();
    //int f1ratioIndex = optionsCluster->trainRatioCombo->currentIndex();
    //float f1ratios[] = {0.01f, 0.05f, 0.1f, 0.2f, 1.f/3.f, 0.5f, 0.75f, 1.f};
    //float ratio = f1ratios[f1ratioIndex];

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
    glw->clearLists();
    if(canvas->canvasType == 1)
    {
        clusterers[tab]->DrawGL(canvas, glw, clusterer);
        if(canvas->data->GetDimCount() == 3) Draw3DClusterer(glw, clusterer);
    }


    emit UpdateInfo();
    QString infoText = mldemos->showStats->infoText->text();

    infoText += "\nClustering as Classifier\nF-Measures:\n";

    float mean=0, stdev=0;
    FOR(j, testErrors.size()) mean += testErrors[j];
    mean /= testErrors.size();
    FOR(j, testErrors.size()) stdev += (testErrors[j] - mean)*(testErrors[j] - mean);
    stdev /= testErrors.size();
    stdev = sqrtf(stdev);
    infoText += QString("%1 clusters: %2 (+- %3)\n").arg(clusterer->NbClusters()).arg(mean, 0, 'f', 3).arg(stdev, 0, 'f', 3);

    mldemos->showStats->infoText->setText(infoText);
    mldemos->showStats->tabWidget->setCurrentIndex(1); // we show the info panel
    mldemos->statsDialog->show();

    drawTimer->clusterer= &this->clusterer;
    drawTimer->inputDims = GetInputDimensions();
    drawTimer->start(QThread::NormalPriority);
    canvas->repaint();
}

void AlgorithmManager::ClusterOptimize()
{
    if(!canvas || !canvas->data->GetCount()) return;
    QMutexLocker lock(mutex);
    if(!optionsCluster->algoList->count()) return;
    int tab = optionsCluster->algoList->currentIndex();
    if(tab >= clusterers.size() || !clusterers[tab]) return;
    tabUsedForTraining = tab;

    int startCount=optionsCluster->rangeStartSpin->value(), stopCount=optionsCluster->rangeStopSpin->value();
    if(startCount>stopCount) startCount ^= stopCount ^= startCount ^= stopCount;


    ivec inputDims = GetInputDimensions();
    vector<fvec> samples = canvas->data->GetSampleDims(inputDims);
    ivec labels = canvas->data->GetLabels();
    int f1ratioIndex = optionsCluster->trainRatioCombo->currentIndex();
    float f1ratios[] = {0.01f, 0.05f, 0.1f, 0.2f, 1.f/3.f, 0.5f, 0.75f, 1.f};
    float ratio = f1ratios[f1ratioIndex];

    float ratios [] = {.1f,.25f,1.f/3.f,.5f,2.f/3.f,.75f,.9f,1.f};
    int ratioIndex = optionsCluster->trainTestCombo->currentIndex();
    float trainRatio = ratios[ratioIndex];

    vector<bool> trainList;
    if(optionsCluster->manualTrainButton->isChecked()) {
        trainList = GetManualSelection();
    }

    float testError = 0;
    ivec kCounts;
    vector< vector<fvec> > resultList(5);
    vector<fvec> testErrors(stopCount-startCount+1);
    int crossValCount = 5;
    FOR(i, resultList.size()) resultList[i].resize(crossValCount);
    for(int k=startCount; k<=stopCount; k++) {
        Clusterer* clusterer = clusterers[tab]->GetClusterer();
        clusterer->SetClusterTestValue(k, stopCount);
        testErrors[k-startCount].resize(crossValCount);
        FOR(j, crossValCount) {
            Train(clusterer, trainRatio, trainList, &testError);
            testErrors[k-startCount][j] = testError;

            float logL = clusterer->GetLogLikelihood(samples);
            float n = samples.size();
            float k = clusterer->GetParameterCount();
            float BIC = -2*logL + log(n)*k;
            float AIC = -2*logL + 2*k;
            float AICc = AIC + 2*(k*k + k)/(n-k-1);

            vector<fvec> clusterScores(samples.size());
            FOR(i, samples.size())
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
            float f1 = ClusterFMeasure(samples, labels, clusterScores, ratio);

            fvec clusterMetrics(5);
            clusterMetrics[0] = logL;
            clusterMetrics[1] = BIC;
            clusterMetrics[2] = AIC;
            clusterMetrics[3] = AICc;
            clusterMetrics[4] = f1;

            FOR(i, clusterMetrics.size()) {
                resultList[i][j].push_back(clusterMetrics[i]);
            }
        }
        kCounts.push_back(k);
        delete clusterer;
    }
    vector<fvec> results(5);
    double value = 0;
    FOR(i, resultList.size())
    {
        results[i].resize(resultList[i][0].size());
        FOR(k, resultList[i][0].size())
        {
            value = 0;
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
    //QBitmap bitmap(w,h);
    //bitmap.clear();
    //pixmap.setMask(bitmap);
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

    vector<QColor> colors;
    colors.push_back(Qt::black);
    colors.push_back(Qt::red);
    colors.push_back(Qt::blue);
    colors.push_back(Qt::magenta);
    colors.push_back(QColor(255,128,0));
    FOR(i, results.size())
    {
        QPointF old;
        painter.setPen(QPen(colors[i%colors.size()],2));
        bests[i] = make_pair(FLT_MAX, 0);
        FOR(k, kCounts.size())
        {
            if(results[i][k] < bests[i].first)
            {
                bests[i] = make_pair(results[i][k], kCounts[k]);
            }
            float x = k/(float)(kCounts.size()-1);
            float y = (results[i][k] - mins[i])/(maxes[i]-mins[i]);
            if(std::isnan(y)){y=0;}

            //if(i == 3) y = 1.f - y; // fmeasures needs to be maximized

            QPointF point(x*(w-2*pad)+pad,(1.f-y)*(h-2*pad));
            if(k) painter.drawLine(old, point);
            old = point;
        }
    }
    optionsCluster->graphLabel->setPixmap(pixmap);
    optionsCluster->graphLabel->repaint();

    optionsCluster->resultList->clear();
    optionsCluster->resultList->addItem(QString("RSS: %1 (%2)").arg(bests[0].second).arg(bests[0].first, 0, 'f', 2));
    optionsCluster->resultList->addItem(QString("BIC: %1 (%2)").arg(bests[1].second).arg(bests[1].first, 0, 'f', 2));
    optionsCluster->resultList->addItem(QString("AIC: %1 (%2)").arg(bests[2].second).arg(bests[2].first, 0, 'f', 2));
    optionsCluster->resultList->addItem(QString("AICc: %1 (%2)").arg(bests[3].second).arg(bests[3].first, 0, 'f', 2));
    optionsCluster->resultList->addItem(QString("F1: %1 (%2)").arg(bests[4].second).arg(bests[4].first, 0, 'f', 2));
    FOR(i, results.size()) {
        optionsCluster->resultList->item(i)->setForeground(colors[i%colors.size()]);
    }
}

void AlgorithmManager::ClusterIterate()
{
    if(!canvas || !canvas->data->GetCount()) return;
    drawTimer->Stop();
    int tab = optionsCluster->algoList->currentIndex();
    if(tab >= clusterers.size() || !clusterers[tab]) return;
    QMutexLocker lock(mutex);
    if(!clusterer)
    {
        clusterer = clusterers[tab]->GetClusterer();
        tabUsedForTraining = tab;
    }
    else clusterers[tab]->SetParams(clusterer);
    clusterer->SetIterative(true);
    Train(clusterer);
    clusterers[tab]->Draw(canvas,clusterer);
    glw->clearLists();
    if(canvas->canvasType == 1)
    {
        clusterers[tab]->DrawGL(canvas, glw, clusterer);
        if(canvas->data->GetDimCount() == 3) Draw3DClusterer(glw, clusterer);
    }


    // we fill in the canvas sampleColors
    ivec inputDims = GetInputDimensions();
    vector<fvec> samples = canvas->data->GetSampleDims(inputDims);
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

    emit UpdateInfo();
}

void AlgorithmManager::Train(Clusterer *clusterer, float trainRatio, bvec trainList, float *testFMeasures, std::vector<fvec> samples, ivec labels)
{
    if(!clusterer) return;
    if(!labels.size()) labels = canvas->data->GetLabels();
    ivec inputDims = GetInputDimensions();
    if(!samples.size()) samples = canvas->data->GetSampleDims(inputDims);
    else samples = canvas->data->GetSampleDims(samples, inputDims);
    sourceDims = inputDims;
    canvas->sourceDims = inputDims;

    if(trainList.size())
    {
        vector<fvec> trainSamples;
        FOR(i, trainList.size())
        {
            if(trainList[i])
            {
                trainSamples.push_back(samples[i]);
            }
        }
        clusterer->Train(trainSamples);
    }
    else if(trainRatio < 1)
    {
        int trainCnt = samples.size()*trainRatio;
        vector<fvec> trainSamples(trainCnt);
        u32 *perm = randPerm(samples.size());
        FOR(i, trainCnt)
        {
            trainSamples[i] = samples[perm[i]];
        }
        clusterer->Train(trainSamples);
        delete [] perm;
    }
    else clusterer->Train(samples);
    // we test the clusters to see how well they classify the samples

    if(!testFMeasures) return;
    // we compute the f-measures for each class
    map<int,int> classcounts;
    int cnt = 0;
    FOR(i, labels.size()) if(!classcounts.count(labels[i])) classcounts[labels[i]] = cnt++;
    int classCount = classcounts.size();
    map<int, fvec> classScores;
    int nbClusters = clusterer->NbClusters();
    fvec clusterScores(nbClusters);
    map<int,float> labelScores;

    vector<fvec> scores(samples.size());
    FOR(i, canvas->data->GetCount())
    {
        fvec result = clusterer->Test(samples[i]);
        if(clusterer->NbClusters()==1) scores[i] = result;
        else if(result.size()>1) scores[i] = result;
        else scores[i] = fvec(nbClusters,0);
    }

    FOR(i, labels.size())
    {
        int label = labels[i];
        labelScores[label] += 1.f;
        if(!classScores.count(label)) classScores[label].resize(nbClusters);
        FOR(k, nbClusters)
        {
            float score = k < scores[i].size() ? scores[i][k] : 0;
            classScores[label][k] += score;
            clusterScores[k] += score;
        }
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

    *testFMeasures = fmeasure;
}
