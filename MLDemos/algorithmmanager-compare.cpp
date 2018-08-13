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
void AlgorithmManager::Compare()
{
    if(!canvas) return;
    if(!compare->compareOptions.size()) return;

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
    DEL(projector);
    // we start parsing the algorithm list
    int folds = compare->params->foldCountSpin->value();
    float ratios [] = {.1f,.25f,1.f/3.f,.5f,2.f/3.f,.75f,.9f,1.f};
    int ratioIndex = compare->params->traintestRatioCombo->currentIndex();
    float trainRatio = ratios[ratioIndex];
    bvec trainList;
    // we get the list of samples that are checked

    vector<fvec> samples;
    ivec labels;
    int datasetOption = compare->params->datasetCombo->currentIndex();
    switch(datasetOption)
    {
    default:
    case 0:
        break;
    case 1:
        samples = compare->datasetA;
        labels = compare->labelsA;
        samples.insert(samples.end(), compare->datasetB.begin(), compare->datasetB.end());
        labels.insert(labels.end(), compare->labelsB.begin(), compare->labelsB.end());
        trainList.clear();
        break;
    case 2:
        samples = compare->datasetA;
        labels = compare->labelsA;
        trainList.clear();
        if(!samples.size()) return;
        break;
    case 3:
        samples = compare->datasetB;
        labels = compare->labelsB;
        trainList.clear();
        if(!samples.size()) return;
        break;
    case 4:
        samples = compare->datasetA;
        labels = compare->labelsA;
        samples.insert(samples.end(), compare->datasetB.begin(), compare->datasetB.end());
        labels.insert(labels.end(), compare->labelsB.begin(), compare->labelsB.end());
        trainList = bvec(samples.size(), false);
        FOR (i, compare->datasetA.size()) trainList[i] = true;
        break;
    case 5:
        samples = compare->datasetB;
        labels = compare->labelsB;
        samples.insert(samples.end(), compare->datasetA.begin(), compare->datasetA.end());
        labels.insert(labels.end(), compare->labelsA.begin(), compare->labelsA.end());
        trainList = bvec(samples.size(), false);
        FOR (i, compare->datasetB.size()) trainList[i] = true;
        break;
    }

    compare->Clear();

    QProgressDialog progress("Comparing Algorithms", "cancel", 0, folds*compare->compareOptions.size());
    progress.show();
    FOR(i, compare->compareOptions.size())
    {
        QString string = compare->compareOptions[i];
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
                classifiers[tab]->LoadParams(paramName, paramValue);
            }
            QString algoName = classifiers[tab]->GetAlgoString();
            fvec fmeasureTrain, fmeasureTest, errorTrain, errorTest, precisionTrain, precisionTest, recallTrain, recallTest;

            map<int,int> classes;
            FOR(j, canvas->data->GetLabels().size()) classes[canvas->data->GetLabels()[j]]++;

            if (!samples.size() && optionsClassify->manualTrainButton->isChecked()) {
                trainList = GetManualSelection();
            }

            FOR(f, folds)
            {
                classifier = classifiers[tab]->GetClassifier();
                if(!classifier) continue;
                Train(classifier, trainRatio, trainList, -1, samples, labels);
                bool bMulti = classifier->IsMultiClass() && DatasetManager::GetClassCount(canvas->data->GetLabels()) > 2;
                if(classifier->rocdata.size()>0)
                {
                    if(!bMulti || classes.size() <= 2)
                    {
                        fvec res = GetBestFMeasure(classifier->rocdata[0]);
                        fmeasureTrain.push_back(res[0]);
                        precisionTrain.push_back(res[1]);
                        recallTrain.push_back(res[2]);
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
                        errorTrain.push_back(errors/(float)rocdata.size());
                    }
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
                if(!classifierMulti.size()) DEL(classifier);
                classifier = 0;
                sourceDims.clear();
                FOR(c,classifierMulti.size()) DEL(classifierMulti[c]); classifierMulti.clear();

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
            int outputDim = compare->params->outputDimCombo->currentIndex();
            QTextStream paramStream(&paramString);
            QString paramName;
            float paramValue;
            while(!paramStream.atEnd())
            {
                paramStream >> paramName;
                paramStream >> paramValue;
                regressors[tab]->LoadParams(paramName, paramValue);
            }

            if (!samples.size() && optionsRegress->manualTrainButton->isChecked()) {
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
                sourceDims.clear();

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

void AlgorithmManager::CompareAdd()
{
    if (options->tabClass->isVisible())
    {
        int tab = optionsClassify->algoList->currentIndex();
        QString name = classifiers[tab]->GetAlgoString();
        QString parameterData;
        QTextStream stream(&parameterData, QIODevice::WriteOnly);
        stream << "Classification" << ":" << tab << "\n";
        classifiers[tab]->SaveParams(stream);
        compare->Add(parameterData, name);
    }
    if (options->tabRegr->isVisible())
    {
        int tab = optionsRegress->algoList->currentIndex();
        QString name = regressors[tab]->GetAlgoString();
        QString parameterData;
        QTextStream stream(&parameterData, QIODevice::WriteOnly);
        stream << "Regression" << ":" << tab << "\n";
        regressors[tab]->SaveParams(stream);
        compare->Add(parameterData, name);
    }
    if (options->tabDyn->isVisible())
    {
        int tab = optionsDynamic->algoList->currentIndex();
        QString name = dynamicals[tab]->GetAlgoString();
        QString parameterData;
        QTextStream stream(&parameterData, QIODevice::WriteOnly);
        stream << "Dynamical" << ":" << tab << "\n";
        dynamicals[tab]->SaveParams(stream);
        compare->Add(parameterData, name);
    }
    if (options->tabMax->isVisible())
    {
        int tab = optionsMaximize->algoList->currentIndex();
        QString name = maximizers[tab]->GetAlgoString();
        QString parameterData;
        QTextStream stream(&parameterData, QIODevice::WriteOnly);
        stream << "Optimization" << ":" << tab << "\n";
        maximizers[tab]->SaveParams(stream);
        compare->Add(parameterData, name);
    }
    mldemos->drawToolbar->compareButton->setChecked(true);
    compare->Show();
}
