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

void AlgorithmManager::Classify()
{
    if(!canvas || !canvas->data->GetCount()) return;
    drawTimer->Stop();
    drawTimer->Clear();
    mutex->lock();
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
    if(!optionsClassify->algoList->count()) return;
    int tab = optionsClassify->algoList->currentIndex();
    if(tab >= classifiers.size() || !classifiers[tab]) return;

    classifier = classifiers[tab]->GetClassifier();
    tabUsedForTraining = tab;
    float ratios [] = {.1f,.25f,1.f/3.f,.5f,2.f/3.f,.75f,.9f,1.f};
    int ratioIndex = optionsClassify->traintestRatioCombo->currentIndex();
    float trainRatio = ratios[ratioIndex];
    vector<bool> trainList;
    if(optionsClassify->manualTrainButton->isChecked())
    {
        // we get the list of samples that are checked
        trainList = GetManualSelection();
    }


    int positiveIndex = optionsClassify->binaryCheck->isChecked() ? optionsClassify->positiveSpin->value() : -1;
    bool trained = Train(classifier, trainRatio, trainList, positiveIndex);
    if(trained)
    {
        classifiers[tab]->Draw(canvas, classifier);
        DrawClassifiedSamples(canvas, classifier, classifierMulti);
        glw->clearLists();
        if(canvas->canvasType == 1)
        {
            classifiers[tab]->DrawGL(canvas, glw, classifier);
            if(canvas->data->GetDimCount() == 3 && sourceDims.size() <= 3) Draw3DClassifier(glw, classifier);
        }

        emit UpdateInfo();
        if(drawTimer && classifier->UsesDrawTimer())
        {
            drawTimer->inputDims = GetInputDimensions();
            drawTimer->start(QThread::NormalPriority);
        }
        if(canvas->canvasType) emit CanvasOptionsChanged();
        // we fill in the canvas sampleColors
        ivec inputDims = GetInputDimensions();
        vector<fvec> samples = canvas->data->GetSampleDims(inputDims);
        canvas->sampleColors.resize(samples.size());
        FOR(i, samples.size())
        {
            canvas->sampleColors[i] = DrawTimer::GetColor(classifier, samples[i], &classifierMulti, inputDims);
        }
        if(canvas->canvasType)
        {
            canvas->maps.model = QPixmap();
            emit CanvasOptionsChanged();
        }
        canvas->repaint();
    }
    else
    {
        mutex->unlock();
        mldemos->Clear();;
        mutex->lock();
        emit UpdateInfo();
    }
    mutex->unlock();
}

void AlgorithmManager::DrawClassifiedSamples(Canvas *canvas, Classifier *classifier, std::vector<Classifier *> classifierMulti)
{
    if(!canvas || !classifier) return;
    int w = canvas->width(), h = canvas->height();
    canvas->maps.model = QPixmap(w,h);
    canvas->maps.model.fill(Qt::transparent);
    QPainter painter(&canvas->maps.model);
    int posClass = INT_MIN;
    FORIT(classifier->classMap, int, int) {
        posClass = max(posClass, it->first);
    }

    QString s;
    FOR(d, sourceDims.size()) s += QString("%1 ").arg(sourceDims[d]);

    int forcedPositive = classifier->inverseMap[-1] < 0 ? -classifier->inverseMap[-1]-1 : -1;

    // we draw the samples
    painter.setRenderHint(QPainter::Antialiasing, true);
    FOR(i, canvas->data->GetCount()) {
        fvec sample = sourceDims.size() ? canvas->data->GetSampleDim(i, sourceDims) : canvas->data->GetSample(i);
        int label = canvas->data->GetLabel(i);
        QPointF point = canvas->toCanvasCoords(canvas->data->GetSample(i));
        fvec res;
        if(classifier->IsMultiClass()) {
            res = classifier->TestMulti(sample);
        }
        else if(classifierMulti.size()) {
            FOR(c, classifierMulti.size()) {
                res.push_back(classifierMulti[c]->Test(sample));
            }
        }
        else res.push_back(classifier->Test(sample));
        if(res.size()==1) {
            float response = res[0];
            if(forcedPositive != -1) {// we forced binary classification
                if(response > 0) {
                    if(label == forcedPositive) Canvas::drawSample(painter, point, 9, 1);
                    else Canvas::drawCross(painter, point, 6, 2);
                } else {
                    if(label != forcedPositive) Canvas::drawSample(painter, point, 9, 0);
                    else Canvas::drawCross(painter, point, 6, 0);
                }
            } else {
                if(response > 0) {
                    if(label != classifier->inverseMap[-1]) Canvas::drawSample(painter, point, 9, 1);
                    else Canvas::drawCross(painter, point, 6, 2);
                } else {
                    if(label == classifier->inverseMap[-1]) Canvas::drawSample(painter, point, 9, 0);
                    else Canvas::drawCross(painter, point, 6, 0);
                }
            }
        } else {
            int max = 0;
            for(int i=1; i<(int)res.size(); i++) if(res[max] < res[i]) max = i;
            int resp = classifier->inverseMap[max];
            if(label == resp) Canvas::drawSample(painter, point, 9, label);
            else Canvas::drawCross(painter, point, 6, label);
        }
    }
}

bool AlgorithmManager::Train(Classifier *classifier, float trainRatio, bvec trainList, int positiveIndex, std::vector<fvec> samples, ivec labels)
{
    if(!classifier) return false;
    if(!labels.size()) labels = canvas->data->GetLabels();
    ivec inputDims = GetInputDimensions();
    if(!samples.size()) samples = canvas->data->GetSampleDims(inputDims);
    else samples = canvas->data->GetSampleDims(samples, inputDims);
    sourceDims = inputDims;
    canvas->sourceDims = inputDims;

    ivec newLabels;
    std::map<int,int> binaryClassMap, binaryInverseMap;
    int classCount = DatasetManager::GetClassCount(labels);
    int positive = INT_MIN; // positive class is always the largest class index
    int negative = 0;
    FOR(i, labels.size()) positive = max(positive, labels[i]);
    int cnt=0;
    FOR(i, labels.size()) if(!binaryClassMap.count(labels[i])) binaryClassMap[labels[i]] = cnt++;
    FORIT(binaryClassMap, int, int)
    {
        binaryInverseMap[it->second] = it->first;
        if(it->first != positive) negative = it->first;
    }
    if(positiveIndex != -1)
    {
        positive = positiveIndex;
        negative = -positiveIndex-1;
        classCount = 2;
    }

    newLabels.resize(labels.size(), 1);
    bool bMulticlass = classifier->IsMultiClass() || classCount > 2;
    if(!bMulticlass || positiveIndex != -1)
    {
        FOR(i, labels.size()) newLabels[i] = (labels[i] == positive) ? 1 : -1;
        bool bHasPositive = false, bHasNegative = false;
        FOR(i, newLabels.size())
        {
            if(bHasPositive && bHasNegative) break;
            bHasPositive |= newLabels[i] == 1;
            bHasNegative |= newLabels[i] == -1;
        }
        if((!bHasPositive || !bHasNegative) && !classifier->SingleClass()) return false;
        binaryClassMap[negative] = -1;
        binaryInverseMap[-1] = negative;
    }
    else
    {
        newLabels = labels;
        if(binaryClassMap.size() > 2) binaryClassMap.clear(); // standard multiclass, no problems
    }
    classifier->rocdata.clear();
    classifier->roclabels.clear();

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
        //trainCnt = trainSamples.size();
        //testCnt = testSamples.size();
    }
    else
    {
        map<int,int> classCnt, trainClassCnt, testClassCnt;
        FOR(i, newLabels.size())
        {
            classCnt[newLabels[i]]++;
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
            testClassCnt[testLabels[i-trainCnt]]++;
        }
        // we need to make sure that we have at least one sample per class
        FORIT(classCnt, int, int)
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
    if(classifier->IsMultiClass() || !bMulticlass)
    {
        classifier->Train(trainSamples, trainLabels);
        // fix the labels for binary classification
        if(classCount == 2)
        {
            if(!classifier->IsMultiClass())
            {
                classifier->classMap = binaryClassMap;
                classifier->inverseMap = binaryInverseMap;
            }
            else
            {
                // if we forced binary classification on multi-class
                if(positiveIndex != -1)
                {
                    classifier->inverseMap[-1] = negative;
                }
            }
        }
    }
    else
    {
        qDebug() << "we're going one-vs-all multiclass! (" << classCount << ")";
        // if we are going multiclass on a single-class classifier, we need to train N one-vs-all models
        FOR(c, classCount)
        {
            int realClass = binaryInverseMap[c];
            ivec trainLabelsBinary(trainLabels.size());
            FOR(i, trainLabels.size())
            {
                if(trainLabels[i] == realClass) trainLabelsBinary[i] = +1;
                else trainLabelsBinary[i] = -1;
            }
            if(c==0) classifierMulti.push_back(classifier);
            else classifierMulti.push_back(classifiers[tabUsedForTraining]->GetClassifier());
            classifierMulti.back()->Train(trainSamples, trainLabelsBinary);
        }
        classifier->classMap = binaryClassMap;
        classifier->inverseMap = binaryInverseMap;
    }

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
        if(classifier->IsMultiClass())
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
                QString text;
                for(int j=1; j<res.size(); j++) {
                    text += QString("%1").arg(res[j]) + "\t";
                    if(res[maxClass] < res[j]) maxClass = j;
                }
                int c = classifier->inverseMap[maxClass];
                rocData.push_back(f32pair(c, label));
                confusionMatrix[0][label][c]++;
                if(label != c) falsePerClass[c]++;
                else truePerClass[c]++;
            }
        }
        else
        {
            if(bMulticlass)
            {
                // we get all the responses for the on vs all
                int maxClass;
                float maxResp = -FLT_MAX;
                FOR(c, classifierMulti.size())
                {
                    float res = classifierMulti[c]->Test(trainSamples[i]);
                    if(res > maxResp)
                    {
                        maxResp = res;
                        maxClass = c;
                    }
                }
                int c = classifier->inverseMap[maxClass];
                rocData.push_back(f32pair(c, label));
                confusionMatrix[0][label][c]++;
                if(label != c) falsePerClass[c]++;
                else truePerClass[c]++;
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
        FORIT(countPerClass, int, int)
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
        if(classifier->IsMultiClass())
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
            if(bMulticlass)
            {
                // we get all the responses for the on vs all
                int maxClass;
                float maxResp = -FLT_MAX;
                FOR(c, classifierMulti.size())
                {
                    float res = classifierMulti[c]->Test(testSamples[i]);
                    if(res > maxResp)
                    {
                        maxResp = res;
                        maxClass = c;
                    }
                }
                int c = classifier->inverseMap[maxClass];
                rocData.push_back(f32pair(c, label));
                confusionMatrix[0][label][c]++;
                if(label != c) falsePerClass[c]++;
                else truePerClass[c]++;
            }            else
            {
                float resp = classifier->Test(testSamples[i]);
                rocData.push_back(f32pair(resp, label));
                if(resp > 0 && label == 1) truePerClass[1]++;
                else if(resp > 0 && label != 1) falsePerClass[0]++;
                else if(label == 1) falsePerClass[1]++;
                else truePerClass[0]++;
            }
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

    emit Trained();
    //bIsRocNew = true;
    //bIsCrossNew = true;
    //SetROCInfo();
    return true;
}
