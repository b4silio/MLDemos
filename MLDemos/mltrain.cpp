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

bool MLDemos::Train(Classifier *classifier, int positive, float trainRatio)
{
    if(!classifier) return false;
    ivec labels = canvas->data->GetLabels();
    ivec newLabels;
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
    }

    classifier->rocdata.clear();
    classifier->roclabels.clear();

    vector<fvec> samples = canvas->data->GetSamples();
    if(trainRatio == 1)
    {
        classifier->Train(samples, newLabels);
        // we generate the roc curve for this guy
        vector<f32pair> rocData;
        FOR(i, samples.size())
        {
            if(bMulticlass)
            {
                fvec res = classifier->TestMulti(samples[i]);
                if(res.size() == 1)
                {
                    rocData.push_back(f32pair(res[0], newLabels[i]));
                }
                else
                {
                    int maxClass = 0;
                    for(int j=1; j<res.size(); j++) if(res[maxClass] < res[j]) maxClass = j;
                    rocData.push_back(f32pair(maxClass, newLabels[i]));
                }
            }
            else
            {
                float resp = classifier->Test(samples[i]);
                rocData.push_back(f32pair(resp, newLabels[i]));
            }
        }
        classifier->rocdata.push_back(rocData);
        classifier->roclabels.push_back("training");
    }
    else
    {
        int trainCnt = (int)(samples.size()*trainRatio);
        u32 *perm = randPerm(samples.size());
        vector<fvec> trainSamples;
        ivec trainLabels;
        trainSamples.resize(trainCnt);
        trainLabels.resize(trainCnt);
        FOR(i, trainCnt)
        {
            trainSamples[i] = samples[perm[i]];
            trainLabels[i] = newLabels[perm[i]];
        }
        classifier->Train(trainSamples, trainLabels);

        // we generate the roc curve for this guy
        vector<f32pair> rocData;
        FOR(i, trainCnt)
        {
            if(bMulticlass)
            {
                fvec res = classifier->TestMulti(samples[perm[i]]);
                if(res.size() == 1)
                {
                    rocData.push_back(f32pair(res[0], newLabels[perm[i]]));
                }
                else
                {
                    int maxClass = 0;
                    for(int j=1; j<res.size(); j++) if(res[maxClass] < res[j]) maxClass = j;
                    rocData.push_back(f32pair(maxClass, newLabels[perm[i]]));
                }
            }
            else
            {
                float resp = classifier->Test(samples[perm[i]]);
                rocData.push_back(f32pair(resp, newLabels[perm[i]]));
            }
        }
        classifier->rocdata.push_back(rocData);
        classifier->roclabels.push_back("training");
        rocData.clear();
        for(int i=trainCnt; i<samples.size(); i++)
        {
            if(bMulticlass)
            {
                fvec res = classifier->TestMulti(samples[perm[i]]);
                if(res.size() == 1)
                {
                    rocData.push_back(f32pair(res[0], newLabels[perm[i]]));
                }
                else
                {
                    int maxClass = 0;
                    for(int j=1; j<res.size(); j++) if(res[maxClass] < res[j]) maxClass = j;
                    rocData.push_back(f32pair(maxClass, newLabels[perm[i]]));
                }
            }
            else
            {
                float resp = classifier->Test(samples[perm[i]]);
                rocData.push_back(f32pair(resp, newLabels[perm[i]]));
            }
        }
        classifier->rocdata.push_back(rocData);
        classifier->roclabels.push_back("test");
        KILL(perm);
    }
    bIsRocNew = true;
    bIsCrossNew = true;
    SetROCInfo();
    return true;
}

void MLDemos::Train(Regressor *regressor, float trainRatio)
{
    if(!regressor) return;
    vector<fvec> samples = canvas->data->GetSamples();
    ivec labels = canvas->data->GetLabels();
    fvec trainErrors, testErrors;
    if(trainRatio == 1.f)
    {
        regressor->Train(samples, labels);
        trainErrors.clear();
        FOR(i, samples.size())
        {
            fvec sample = samples[i];
            int dim = sample.size();
            fvec res = regressor->Test(sample);
            float error = fabs(res[0] - sample[dim-1]);
            trainErrors.push_back(error);
        }
        regressor->trainErrors = trainErrors;
        regressor->testErrors.clear();
    }
    else
    {
        int trainCnt = (int)(samples.size()*trainRatio);
        u32 *perm = randPerm(samples.size());
        vector<fvec> trainSamples;
        ivec trainLabels;
        trainSamples.resize(trainCnt);
        trainLabels.resize(trainCnt);
        FOR(i, trainCnt)
        {
            trainSamples[i] = samples[perm[i]];
            trainLabels[i] = labels[perm[i]];
        }
        regressor->Train(trainSamples, trainLabels);

        FOR(i, trainCnt)
        {
            fvec sample = samples[perm[i]];
            int dim = sample.size();
            fvec res = regressor->Test(sample);
            float error = fabs(res[0] - sample[dim-1]);
            trainErrors.push_back(error);
        }
        for(int i=trainCnt; i<samples.size(); i++)
        {
            fvec sample = samples[perm[i]];
            int dim = sample.size();
            fvec res = regressor->Test(sample);
            float error = fabs(res[0] - sample[dim-1]);
            testErrors.push_back(error);
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

    dynamical->Train(trajectories, labels);
    return Test(dynamical, trajectories, labels);
}

void MLDemos::Train(Clusterer *clusterer)
{
    if(!clusterer) return;
    clusterer->Train(canvas->data->GetSamples());
}

void MLDemos::Train(Projector *projector)
{
    if(!projector) return;
    projector->Train(canvas->data->GetSamples(), projector->startIndex, projector->stopIndex);
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
    fvec startingPoint;
    if(canvas->targets.size())
    {
        startingPoint = canvas->targets[canvas->targets.size()-1];
        QPointF starting = canvas->toCanvasCoords(startingPoint);
        startingPoint[0] = starting.x()/w;
        startingPoint[1] = starting.y()/h;
    }
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

// returns respectively the reconstruction error for the training points individually, per trajectory, and the error to target
fvec MLDemos::Test(Dynamical *dynamical, vector< vector<fvec> > trajectories, ivec labels)
{
    if(!dynamical || !trajectories.size()) return fvec();
    int dim = trajectories[0][0].size()/2;
    //(int dim = dynamical->Dim();
    float dT = dynamical->dT;
    fvec sample; sample.resize(dim,0);
    fvec vTrue; vTrue.resize(dim, 0);
    fvec xMin, xMax;
    xMin.resize(dim, FLT_MAX);
    xMax.resize(dim, -FLT_MAX);

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
    FOR(i, trajectories.size())
    {
        fvec pos = trajectories[i][0];
        fvec end = trajectories[i][trajectories[i].size()-1];
        FOR(d, dim)
        {
            pos.pop_back();
            end.pop_back();
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
    fvec pos; pos.resize(dim);
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
    int positive = optionsCompare->positiveSpin->value();

    compare->Clear();

    QProgressDialog progress("Comparing Algorithms", "cancel", 0, folds*compareOptions.size());
    progress.show();
    FOR(i, compareOptions.size())
    {
        QString string = compareOptions[i];
        QTextStream stream(&string);
        QString line = stream.readLine();
        QString paramString = stream.readAll();
        if(line.startsWith("Maximization"))
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
                classifiers[tab]->LoadParams(paramName, paramValue);
            }
            QString algoName = classifiers[tab]->GetAlgoString();
            fvec resultTrain, resultTest, errorTrain, errorTest;

            map<int,int> classes;
            FOR(j, canvas->data->GetLabels().size()) classes[canvas->data->GetLabels()[j]]++;

            FOR(f, folds)
            {
                classifier = classifiers[tab]->GetClassifier();
                if(!classifier) continue;
                Train(classifier, positive, trainRatio);
                bool bMulti = classifier->IsMultiClass() && DatasetManager::GetClassCount(canvas->data->GetLabels());
                if(classifier->rocdata.size()>0)
                {
                    if(!bMulti || classes.size() <= 2) resultTrain.push_back(GetBestFMeasure(classifier->rocdata[0]));
                    else
                    {
                        int errors = 0;
                        std::vector<f32pair> rocdata = classifier->rocdata[0];
                        FOR(j, rocdata.size())
                        {
                            if(rocdata[j].first != rocdata[j].second)
                            {
                                if(classes.size() > 2) errors++;
                                else if((rocdata[j].first < 0) != rocdata[j].second) errors++;
                            }
                        }
                        if(classes.size() <= 2)
                        {
                            float e = min(errors,(int)rocdata.size()-errors)/(float)rocdata.size();
                            resultTrain.push_back(1-e);
                            resultTrain.push_back(1-e);
                        }
                        else
                        {
                            errorTrain.push_back(errors/(float)rocdata.size());
                            errorTest.push_back(errors/(float)rocdata.size());
                        }
                    }
                }
                if(classifier->rocdata.size()>1)
                {
                    if(!bMulti || classes.size() <= 2) resultTest.push_back(GetBestFMeasure(classifier->rocdata[1]));
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
                        if(classes.size() <= 2) errorTest.push_back(min(errors,(int)rocdata.size()-errors)/(float)rocdata.size());
                        else errorTest.push_back(errors/(float)rocdata.size());
                    }
                }
                DEL(classifier);

                progress.setValue(f + i*folds);
                qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
                if(progress.wasCanceled())
                {
                    compare->AddResults(resultTrain, "f-Measure (Training)", algoName);
                    compare->AddResults(resultTest, "f-Measure (Test)", algoName);
                    compare->AddResults(errorTrain, "Error (Training)", algoName);
                    compare->AddResults(errorTest, "Error (Test)", algoName);
                    compare->Show();
                    return;
                }
            }
            compare->AddResults(resultTrain, "f-Measure (Training)", algoName);
            compare->AddResults(resultTest, "f-Measure (Test)", algoName);
            compare->AddResults(errorTrain, "Error (Training)", algoName);
            compare->AddResults(errorTest, "Error (Test)", algoName);
        }
        if(line.startsWith("Regression"))
        {
            QStringList s = line.split(":");
            int tab = s[1].toInt();
            if(tab >= regressors.size() || !regressors[tab]) continue;
            QTextStream paramStream(&paramString);
            QString paramName;
            float paramValue;
            while(!paramStream.atEnd())
            {
                paramStream >> paramName;
                paramStream >> paramValue;
                regressors[tab]->LoadParams(paramName, paramValue);
            }
            QString algoName = regressors[tab]->GetAlgoString();
            fvec resultTrain, resultTest;
            FOR(f, folds)
            {
                regressor = regressors[tab]->GetRegressor();
                if(!regressor) continue;
                Train(regressor, trainRatio);
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
                    resultTest = regressor->testErrors;
                }
                DEL(regressor);

                progress.setValue(f + i*folds);
                qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
                if(progress.wasCanceled())
                {
                    compare->AddResults(resultTrain, "Error (Training)", algoName);
                    compare->AddResults(resultTest, "Error (Testing)", algoName);
                    compare->Show();
                    return;
                }
            }
            compare->AddResults(resultTrain, "Error (Training)", algoName);
            compare->AddResults(resultTest, "Error (Testing)", algoName);
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
