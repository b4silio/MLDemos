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
#include <QFileDialog>
#include <QProgressDialog>

using namespace std;

bool MLDemos::Train(Classifier *classifier, int positive, float trainRatio, bvec trainList)
{
    if(!classifier) return false;
    ivec labels = canvas->data->GetLabels();
    ivec newLabels;
    std::map<int,int> binaryClassMap;

    newLabels.resize(labels.size(), 1);
    bool bMulticlass = classifier->IsMultiClass();
    if(!bMulticlass)
    {
        if(positive == 0)
        {
            FOR(i, labels.size()) newLabels[i] = (!labels[i] || labels[i] == -1) ? 1 : -1;
        }
        else
        {
            FOR(i, labels.size()) newLabels[i] = (labels[i] == positive) ? 1 : -1;
        }
        bool bHasPositive = false, bHasNegative = false;
        FOR(i, newLabels.size())
        {
            if(bHasPositive && bHasNegative) break;
            bHasPositive |= newLabels[i] == 1;
            bHasNegative |= newLabels[i] == -1;
        }
        if((!bHasPositive || !bHasNegative) && !classifier->SingleClass()) return false;
    }
    else
    {
        newLabels = labels;
        //
        int cnt=0;
        FOR(i, labels.size()) if(!binaryClassMap.count(labels[i])) binaryClassMap[labels[i]] = cnt++;
        if(binaryClassMap.size() > 2) binaryClassMap.clear(); // standard multiclass, no problems
        //for(map<int,int>::iterator it=classMap.begin(); it != classMap.end(); it++);
        //positive = classMap.begin()->first;
        //binaryLabels.resize(labels.size());
        //FOR(i, labels.size()) binaryLabels[i] = labels[i] == positive ? 1 : 0;
    }

    classifier->rocdata.clear();
    classifier->roclabels.clear();

    ivec inputDims = GetInputDimensions();
    vector<fvec> samples = canvas->data->GetSampleDims(inputDims);

    vector<fvec> trainSamples, testSamples;
    ivec trainLabels, testLabels;
    u32 *perm = 0;
    int trainCnt, testCnt;
    if(trainList.size())
    {
        FOR(i, trainList.size())
        {
            if(trainList[i])
            {
                trainSamples.push_back(samples[i]);
                trainLabels.push_back(newLabels[i]);
            }
            else
            {
                testSamples.push_back(samples[i]);
                testLabels.push_back(newLabels[i]);
            }
        }
        trainCnt = trainSamples.size();
        testCnt = testSamples.size();
    }
    else
    {
        map<int,int> classCnt, trainClassCnt, testClassCnt;
        FOR(i, labels.size())
        {
            classCnt[labels[i]]++;
        }

        trainCnt = (int)(samples.size()*trainRatio);
        testCnt = samples.size() - trainCnt;
        trainSamples.resize(trainCnt);
        trainLabels.resize(trainCnt);
        testSamples.resize(testCnt);
        testLabels.resize(testCnt);
        perm = randPerm(samples.size());
        FOR(i, trainCnt)
        {
            trainSamples[i] = samples[perm[i]];
            trainLabels[i] = newLabels[perm[i]];
            trainClassCnt[trainLabels[i]]++;
        }
        for(int i=trainCnt; i<samples.size(); i++)
        {
            testSamples[i-trainCnt] = samples[perm[i]];
            testLabels[i-trainCnt] = newLabels[perm[i]];
            testClassCnt[trainLabels[i]]++;
        }
        // we need to make sure that we have at least one sample per class
        for(map<int,int>::iterator it=classCnt.begin();it!=classCnt.end();it++)
        {
            if(!trainClassCnt.count(it->first))
            {
                FOR(i, testSamples.size())
                {
                    if(testLabels[i] != it->first) continue;
                    trainSamples.push_back(testSamples[i]);
                    trainLabels.push_back(testLabels[i]);
                    testSamples.erase(testSamples.begin() + i);
                    testLabels.erase(testLabels.begin() + i);
                    trainCnt++;
                    testCnt--;
                    break;
                }
            }
        }
    }

    // do the actual training
    classifier->Train(trainSamples, trainLabels);

    // compute test results
    lastTrainingInfo = "";
    map<int, int> truePerClass;
    map<int, int> falsePerClass;
    map<int, int> countPerClass;
    map<int, map<int, int> > confusionMatrix[2];

    // we generate the roc curve for this guy
    bool bTrueMulti = bMulticlass;
    vector<f32pair> rocData;
    FOR(i, trainSamples.size())
    {
        int label = trainLabels[i];
        if(bMulticlass && binaryClassMap.size()) label = binaryClassMap[label];
        if(bMulticlass)
        {
            fvec res = classifier->TestMulti(trainSamples[i]);
            if(res.size() == 1)
            {
                rocData.push_back(f32pair(res[0], label));
                float resp = res[0];
                if(resp > 0 && label == 1) truePerClass[1]++;
                else if(resp > 0 && label != 1) falsePerClass[0]++;
                else if(label == 1) falsePerClass[1]++;
                else truePerClass[0]++;
                bTrueMulti = false;
            }
            else
            {
                int maxClass = 0;
                for(int j=1; j<res.size(); j++) if(res[maxClass] < res[j]) maxClass = j;
                int c = classifier->inverseMap[maxClass];
                rocData.push_back(f32pair(c, label));
                confusionMatrix[0][label][c]++;
                if(label != c) falsePerClass[c]++;
                else truePerClass[c]++;
            }
        }
        else
        {
            float resp = classifier->Test(trainSamples[i]);
            rocData.push_back(f32pair(resp, label));
            if(resp > 0 && label == 1) truePerClass[1]++;
            else if(resp > 0 && label != 1) falsePerClass[0]++;
            else if(label == 1) falsePerClass[1]++;
            else truePerClass[0]++;
        }
        if(bMulticlass) countPerClass[label]++;
        else countPerClass[(label==1)?1:0]++;
    }
    if(!bTrueMulti) rocData = FixRocData(rocData);
    classifier->rocdata.push_back(rocData);
    classifier->roclabels.push_back("training");
    lastTrainingInfo += QString("\nTraining Set (%1 samples):\n").arg(trainSamples.size());
    int posClass = 1;
    if(bTrueMulti)
    {
        float macroFMeasure = 0.f, microFMeasure = 0.f;
        int microTP = 0, microFP = 0, microCount = 0;
        float microPrecision = 0.f;
        float microRecall = 0.f;
        for(map<int,int>::iterator it = countPerClass.begin(); it != countPerClass.end(); it++)
        {
            int c = it->first;
            int tp = truePerClass[c];
            int fp = falsePerClass[c];
            int count = it->second;
            microTP += tp;
            microFP += fp;
            microCount += count;
            float precision = tp / float(tp + fp);
            float recall = tp / float(count);
            macroFMeasure += 2*precision*recall/(precision+recall);
            float ratio = it->second != 0 ? tp / (float)it->second : 0;
            lastTrainingInfo += QString("Class %1 (%5 samples): %2 correct (%4%)\n%3 incorrect\n").arg(c).arg(tp).arg(fp).arg((int)(ratio*100)).arg(it->second);
        }
        macroFMeasure /= countPerClass.size();
        microPrecision = microTP / float(microTP + microFP);
        microRecall = microTP / float(microCount);
        microFMeasure = 2*microPrecision*microRecall/(microPrecision + microRecall);
        lastTrainingInfo += QString("F-Measure: %1 (micro) \t %2 (macro)\n").arg(microFMeasure, 0, 'f', 3).arg(macroFMeasure, 0, 'f', 3);
    }
    else
    {
        if(truePerClass[1] / (float) countPerClass[1] < 0.25)
        {
            posClass = 0;
        }
        int tp = posClass ? truePerClass[1] : falsePerClass[1];
        int fp = posClass ? falsePerClass[1] : truePerClass[1];
        int count = countPerClass[1];
        float ratio = count != 0 ? tp/(float)count : 1;
        lastTrainingInfo += QString("Positive (%4 samples): %1 correct (%3%)\n%2 incorrect\n").arg(tp).arg(fp).arg((int)(ratio*100)).arg(count);
        tp = posClass ? truePerClass[0] : falsePerClass[0];
        fp = posClass ? falsePerClass[0] : truePerClass[0];
        count = countPerClass[0];
        ratio = count != 0 ? tp/(float)count : 1;
        lastTrainingInfo += QString("Negative (%4 samples): %1 correct (%3%)\n%2 incorrect\n").arg(tp).arg(fp).arg((int)(ratio*100)).arg(count);
    }

    truePerClass.clear();
    falsePerClass.clear();
    countPerClass.clear();
    rocData.clear();
    FOR(i, testSamples.size())
    {
        int label = testLabels[i];
        if(bMulticlass && binaryClassMap.size()) label = binaryClassMap[label];
        if(bMulticlass)
        {
            fvec res = classifier->TestMulti(testSamples[i]);
            if(res.size() == 1)
            {
                rocData.push_back(f32pair(res[0], label));
                float resp = res[0];
                if(resp > 0 && label == 1) truePerClass[1]++;
                else if(resp > 0 && label != 1) falsePerClass[0]++;
                else if(label == 1) falsePerClass[1]++;
                else truePerClass[0]++;
                bTrueMulti = false;
            }
            else
            {
                int maxClass = 0;
                for(int j=1; j<res.size(); j++) if(res[maxClass] < res[j]) maxClass = j;
                int c = classifier->inverseMap[maxClass];
                rocData.push_back(f32pair(c, label));
                if(label != c) falsePerClass[c]++;
                else truePerClass[c]++;
                confusionMatrix[1][label][c]++;
            }
        }
        else
        {
            float resp = classifier->Test(testSamples[i]);
            rocData.push_back(f32pair(resp, label));
            if(resp > 0 && label == 1) truePerClass[1]++;
            else if(resp > 0 && label != 1) falsePerClass[0]++;
            else if(label == 1) falsePerClass[1]++;
            else truePerClass[0]++;
        }
        if(bMulticlass) countPerClass[label]++;
        else countPerClass[(label==1)?1:0]++;
    }
    if(!bTrueMulti) rocData = FixRocData(rocData);
    classifier->rocdata.push_back(rocData);
    classifier->roclabels.push_back("test");
    classifier->confusionMatrix[0] = confusionMatrix[0];
    classifier->confusionMatrix[1] = confusionMatrix[1];
    lastTrainingInfo += QString("\nTesting Set (%1 samples):\n").arg(testSamples.size());
    if(bTrueMulti)
    {
        float macroFMeasure = 0.f, microFMeasure = 0.f;
        int microTP = 0, microFP = 0, microCount = 0;
        float microPrecision = 0.f;
        float microRecall = 0.f;
        for(map<int,int>::iterator it = countPerClass.begin(); it != countPerClass.end(); it++)
        {
            int c = it->first;
            int tp = truePerClass[c];
            int fp = falsePerClass[c];
            int count = it->second;
            microTP += tp;
            microFP += fp;
            microCount += count;
            float precision = tp / float(tp + fp);
            float recall = tp / float(count);
            macroFMeasure += 2*precision*recall/(precision+recall);
            float ratio = it->second != 0 ? tp / (float)it->second : 0;
            lastTrainingInfo += QString("Class %1 (%5 samples): %2 correct (%4%)\n%3 incorrect\n").arg(c).arg(tp).arg(fp).arg((int)(ratio*100)).arg(it->second);
        }
        macroFMeasure /= countPerClass.size();
        microPrecision = microTP / float(microTP + microFP);
        microRecall = microTP / float(microCount);
        microFMeasure = 2*microPrecision*microRecall/(microPrecision + microRecall);
        lastTrainingInfo += QString("F-Measure: %1 (micro) \t %2 (macro)\n").arg(microFMeasure, 0, 'f', 3).arg(macroFMeasure, 0, 'f', 3);
    }
    else
    {
        int tp = posClass ? truePerClass[1] : falsePerClass[1];
        int fp = posClass ? falsePerClass[1] : truePerClass[1];
        int count = countPerClass[1];
        float ratio = count != 0 ? tp/(float)count : 1;
        lastTrainingInfo += QString("Positive (%4 samples): %1 correct (%3%)\n%2 incorrect\n").arg(tp).arg(fp).arg((int)(ratio*100)).arg(count);
        tp = posClass ? truePerClass[0] : falsePerClass[0];
        fp = posClass ? falsePerClass[0] : truePerClass[0];
        count = countPerClass[0];
        ratio = count != 0 ? tp/(float)count : 1;
        lastTrainingInfo += QString("Negative (%4 samples): %1 correct (%3%)\n%2 incorrect\n").arg(tp).arg(fp).arg((int)(ratio*100)).arg(count);
    }
    KILL(perm);

    bIsRocNew = true;
    bIsCrossNew = true;
    SetROCInfo();
    return true;
}

void MLDemos::Train(Regressor *regressor, int outputDim, float trainRatio, bvec trainList)
{
    if(!regressor || !canvas->data->GetCount()) return;
    ivec inputDims = GetInputDimensions();
    int outputIndexInList = -1;
    if(inputDims.size()==1 && inputDims[0] == outputDim) return; // we dont have enough dimensions for training
    FOR(i, inputDims.size()) if(outputDim == inputDims[i])
    {
        outputIndexInList = i;
        break;
    }

    vector<fvec> samples = canvas->data->GetSampleDims(inputDims, outputIndexInList == -1 ? outputDim : -1);
    ivec labels = canvas->data->GetLabels();
    if(!samples.size()) return;
    int cnt = samples.size();
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
            int dim = sample.size();
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
            int dim = sample.size();
            fvec res = regressor->Test(sample);
            float error = fabs(res[0] - sample.back());
            trainErrors.push_back(error);
        }
        FOR(i, testCnt)
        {
            fvec sample = testSamples[i];
            int dim = sample.size();
            fvec res = regressor->Test(sample);
            float error = fabs(res[0] - sample.back());
            testErrors.push_back(error);
            //qDebug() << " test error: " << i << error;
        }
        regressor->trainErrors = trainErrors;
        regressor->testErrors = testErrors;
        KILL(perm);
    }
    bIsCrossNew = true;
}

// returns respectively the reconstruction error for the training points individually, per trajectory, and the error to target
fvec MLDemos::Train(Dynamical *dynamical)
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

void MLDemos::Train(Clusterer *clusterer, float trainRatio, bvec trainList, float *testFMeasures)
{
    if(!clusterer) return;
    vector<fvec> samples = canvas->data->GetSamples();
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
    ivec labels = canvas->data->GetLabels();
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
    }

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

void MLDemos::Train(Projector *projector, bvec trainList)
{
    if(!projector) return;
    if(trainList.size())
    {
        vector<fvec> trainSamples;
        ivec trainLabels;
        FOR(i, trainList.size())
        {
            if(trainList[i])
            {
                trainSamples.push_back(canvas->data->GetSample(i));
                trainLabels.push_back(canvas->data->GetLabel(i));
            }
        }
        projector->Train(trainSamples, trainLabels);
    }
    else projector->Train(canvas->data->GetSamples(), canvas->data->GetLabels());
}

void MLDemos::Train(Maximizer *maximizer)
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

void MLDemos::Test(Maximizer *maximizer)
{
    if(!maximizer) return;
    do
    {
        fvec sample = maximizer->Test(maximizer->Maximum());
        maximizer->age++;
    }
    while(maximizer->age < maximizer->maxAge && maximizer->MaximumValue() < maximizer->stopValue);
}

void MLDemos::Train(Reinforcement *reinforcement)
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

// returns respectively the reconstruction error for the training points individually, per trajectory, and the error to target
fvec MLDemos::Test(Dynamical *dynamical, vector< vector<fvec> > trajectories, ivec labels)
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

void MLDemos::Compare()
{
    if(!canvas) return;
    if(!compareOptions.size()) return;

    QMutexLocker lock(&mutex);
    drawTimer->Stop();
    DEL(clusterer);
    DEL(regressor);
    DEL(dynamical);
    DEL(classifier);
    DEL(maximizer);
    DEL(projector);
    // we start parsing the algorithm list
    int folds = optionsCompare->foldCountSpin->value();
    float ratios [] = {.1f,.25f,1.f/3.f,.5f,2.f/3.f,.75f,.9f,1.f};
    int ratioIndex = optionsCompare->traintestRatioCombo->currentIndex();
    float trainRatio = ratios[ratioIndex];
    //int positive = optionsCompare->positiveSpin->value();
    int positive = 1;

    compare->Clear();

    QProgressDialog progress("Comparing Algorithms", "cancel", 0, folds*compareOptions.size());
    progress.show();
    FOR(i, compareOptions.size())
    {
        QString string = compareOptions[i];
        QTextStream stream(&string);
        QString line = stream.readLine();
        QString paramString = stream.readAll();
        if(line.startsWith("Optimization"))
        {
            QStringList s = line.split(":");
            int tab = s[1].toInt();
            if(tab >= maximizers.size() || !maximizers[tab]) continue;
            QTextStream paramStream(&paramString);
            QString paramName;
            float paramValue;
            while(!paramStream.atEnd())
            {
                paramStream >> paramName;
                paramStream >> paramValue;
                maximizers[tab]->LoadParams(paramName, paramValue);
            }
            QString algoName = maximizers[tab]->GetAlgoString();
            fvec resultIt, resultVal, resultEval;
            FOR(f, folds)
            {
                maximizer = maximizers[tab]->GetMaximizer();
                if(!maximizer) continue;
                maximizer->maxAge = optionsMaximize->iterationsSpin->value();
                maximizer->stopValue = optionsMaximize->stoppingSpin->value();
                Train(maximizer);
                Test(maximizer);
                resultIt.push_back(maximizer->age);
                resultVal.push_back(maximizer->MaximumValue());
                resultEval.push_back(maximizer->Evaluations());
                progress.setValue(f + i*folds);
                DEL(maximizer);
                qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
                if(progress.wasCanceled())
                {
                    compare->AddResults(resultEval, "Evaluations", algoName);
                    compare->AddResults(resultVal, "Reward", algoName);
                    compare->AddResults(resultIt, "Iterations", algoName);
                    compare->Show();
                    return;
                }
            }
            compare->AddResults(resultEval, "Evaluations", algoName);
            compare->AddResults(resultVal, "Reward", algoName);
            compare->AddResults(resultIt, "Iterations", algoName);
        }
        if(line.startsWith("Classification"))
        {
            QStringList s = line.split(":");
            int tab = s[1].toInt();
            if(tab >= classifiers.size() || !classifiers[tab]) continue;
            QTextStream paramStream(&paramString);
            QString paramName;
            float paramValue;
            while(!paramStream.atEnd())
            {
                paramStream >> paramName;
                paramStream >> paramValue;
                qDebug() << "params:" << paramName << paramValue;
                classifiers[tab]->LoadParams(paramName, paramValue);
            }
            QString algoName = classifiers[tab]->GetAlgoString();
            fvec fmeasureTrain, fmeasureTest, errorTrain, errorTest, precisionTrain, precisionTest, recallTrain, recallTest;

            bvec trainList;
            if(optionsClassify->manualTrainButton->isChecked())
            {
                // we get the list of samples that are checked
                trainList = GetManualSelection();
            }

            map<int,int> classes;
            FOR(j, canvas->data->GetLabels().size()) classes[canvas->data->GetLabels()[j]]++;

            FOR(f, folds)
            {
                classifier = classifiers[tab]->GetClassifier();
                if(!classifier) continue;
                Train(classifier, positive, trainRatio, trainList);
                bool bMulti = classifier->IsMultiClass() && DatasetManager::GetClassCount(canvas->data->GetLabels());
                if(classifier->rocdata.size()>0)
                {
                    if(!bMulti || classes.size() <= 2)
                    {
                        fvec res = GetBestFMeasure(classifier->rocdata[0]);
                        fmeasureTrain.push_back(res[0]);
                        precisionTrain.push_back(res[1]);
                        recallTrain.push_back(res[2]);
                        //qDebug() << "training" << res[0] << res[1] << res[2];
                    }
                    else
                    {
                        int errors = 0;
                        std::vector<f32pair> rocdata = classifier->rocdata[0];
                        FOR(j, rocdata.size())
                        {
                            //qDebug() << "rocdata: " << j << rocdata[j].first << rocdata[j].second;
                            if(rocdata[j].first != rocdata[j].second)
                            {
                                if(classes.size() > 2) errors++;
                                else if((rocdata[j].first < 0) != rocdata[j].second) errors++;
                            }
                        }
                        if(classes.size() <= 2)
                        {
                            float e = min(errors,(int)rocdata.size()-errors)/(float)rocdata.size();
                            fmeasureTrain.push_back(1-e);
                        }
                        else
                        {
                            // compute the micro and macro f-measure
                            fpair fmeasure = GetMicroMacroFMeasure(rocdata);
                            fmeasureTrain.push_back(fmeasure.first);
                            errorTrain.push_back(errors/(float)rocdata.size());
                        }
                    }
                }
                if(classifier->rocdata.size()>1)
                {
                    if(!bMulti || classes.size() <= 2)
                    {
                        fvec res = GetBestFMeasure(classifier->rocdata[1]);
                        fmeasureTest.push_back(res[0]);
                        precisionTest.push_back(res[1]);
                        recallTest.push_back(res[2]);
                    }
                    else
                    {
                        int errors = 0;
                        std::vector<f32pair> rocdata = classifier->rocdata[1];
                        FOR(j, rocdata.size())
                        {
                            if(rocdata[j].first != rocdata[j].second)
                            {
                                if(classes.size() > 2) errors++;
                                else if((rocdata[j].first < 0) != rocdata[j].second) errors++;
                            }
                        }
                        if(classes.size() <= 2) fmeasureTest.push_back(min(errors,(int)rocdata.size()-errors)/(float)rocdata.size());
                        else
                        {
                            // compute the micro and macro f-measure
                            fpair fmeasure = GetMicroMacroFMeasure(rocdata);
                            fmeasureTest.push_back(fmeasure.first);
                            errorTest.push_back(errors/(float)rocdata.size());
                        }
                    }
                }
                DEL(classifier);

                progress.setValue(f + i*folds);
                qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
                if(progress.wasCanceled())
                {
                    compare->AddResults(fmeasureTest,   "F-Measure (Test)", algoName);
                    compare->AddResults(errorTest,      "Error (Test)", algoName);
                    compare->AddResults(precisionTest,  "Precision (Test)", algoName);
                    compare->AddResults(recallTest,     "Recall (Test)", algoName);
                    compare->AddResults(fmeasureTrain,  "F-Measure (Training)", algoName);
                    compare->AddResults(errorTrain,     "Error (Training)", algoName);
                    compare->AddResults(precisionTrain, "Precision (Training)", algoName);
                    compare->AddResults(recallTrain,    "Recall (Training)", algoName);
                    //compare->SetActiveResult(1);
                    compare->Show();
                    return;
                }
            }
            compare->AddResults(fmeasureTest,   "F-Measure (Test)", algoName);
            compare->AddResults(errorTest,      "Error (Test)", algoName);
            compare->AddResults(precisionTest,  "Precision (Test)", algoName);
            compare->AddResults(recallTest,     "Recall (Test)", algoName);
            compare->AddResults(fmeasureTrain,  "F-Measure (Training)", algoName);
            compare->AddResults(errorTrain,     "Error (Training)", algoName);
            compare->AddResults(precisionTrain, "Precision (Training)", algoName);
            compare->AddResults(recallTrain,    "Recall (Training)", algoName);
            //compare->SetActiveResult(1);
        }
        if(line.startsWith("Regression"))
        {
            QStringList s = line.split(":");
            int tab = s[1].toInt();
            if(tab >= regressors.size() || !regressors[tab]) continue;
            int outputDim = optionsCompare->outputDimCombo->currentIndex();
            QTextStream paramStream(&paramString);
            QString paramName;
            float paramValue;
            while(!paramStream.atEnd())
            {
                paramStream >> paramName;
                paramStream >> paramValue;
                regressors[tab]->LoadParams(paramName, paramValue);
            }
            bvec trainList;
            if(optionsRegress->manualTrainButton->isChecked())
            {
                // we get the list of samples that are checked
                trainList = GetManualSelection();
            }

            QString algoName = regressors[tab]->GetAlgoString();
            fvec resultTrain, resultTest;
            FOR(f, folds)
            {
                regressor = regressors[tab]->GetRegressor();
                if(!regressor) continue;
                //qDebug() << " training: " << regressors[tab]->GetName();
                Train(regressor, outputDim, trainRatio, trainList);
                if(regressor->trainErrors.size())
                {
                    float error = 0.f;
                    FOR(i, regressor->trainErrors.size()) error += regressor->trainErrors[i];
                    error /= regressor->trainErrors.size();
                    resultTrain.push_back(error);
                }
                if(regressor->testErrors.size())
                {
                    float error = 0.f;
                    FOR(i, regressor->testErrors.size()) error += regressor->testErrors[i];
                    error /= regressor->testErrors.size();
                    resultTest.push_back(error);
                }
                DEL(regressor);

                progress.setValue(f + i*folds);
                qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
                if(progress.wasCanceled())
                {
                    compare->AddResults(resultTest, "Error (Testing)", algoName);
                    compare->AddResults(resultTrain, "Error (Training)", algoName);
                    compare->Show();
                    return;
                }
            }
            compare->AddResults(resultTest, "Error (Testing)", algoName);
            compare->AddResults(resultTrain, "Error (Training)", algoName);
        }
        if(line.startsWith("Dynamical"))
        {
            QStringList s = line.split(":");
            int tab = s[1].toInt();
            if(tab >= dynamicals.size() || !dynamicals[tab]) continue;
            QTextStream paramStream(&paramString);
            QString paramName;
            float paramValue;
            while(!paramStream.atEnd())
            {
                paramStream >> paramName;
                paramStream >> paramValue;
                dynamicals[tab]->LoadParams(paramName, paramValue);
            }
            QString algoName = dynamicals[tab]->GetAlgoString();
            fvec resultReconst, resultTargetTraj, resultTarget;
            FOR(f, folds)
            {
                dynamical = dynamicals[tab]->GetDynamical();
                if(!dynamical) continue;
                fvec results = Train(dynamical);
                if(results.size())
                {
                    resultReconst.push_back(results[0]);
                    resultTargetTraj.push_back(results[1]);
                    resultTarget.push_back(results[2]);
                }
                DEL(dynamical);

                progress.setValue(f + i*folds);
                qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
                if(progress.wasCanceled())
                {
                    compare->AddResults(resultReconst, "Reconstruction Error", algoName);
                    compare->AddResults(resultTargetTraj, "Target Error (trajectories)", algoName);
                    compare->AddResults(resultTarget, "Target Error (random points)", algoName);
                    compare->Show();
                    return;
                }
            }
            compare->AddResults(resultReconst, "Reconstruction Error", algoName);
            compare->AddResults(resultTargetTraj, "Target Error (trajectories)", algoName);
            compare->AddResults(resultTarget, "Target Error (random points)", algoName);
        }
        compare->Show();
    }
}
