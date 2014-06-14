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
#include "basicMath.h"
#include "roc.h"
#include <QDebug>
#include <fstream>
#include <QPixmap>
#include <QBitmap>
#include <QSettings>
#include <QFileDialog>
#include <QProgressDialog>
#include <qcontour.h>
#include <assert.h>
#include "mldemos.h"

using namespace std;

AlgorithmManager::AlgorithmManager(MLDemos *mldemos, Canvas *canvas, GLWidget *glw, QMutex *mutex, DrawTimer *drawTimer, CompareAlgorithms *compare, GridSearch *gridSearch)
    : mldemos(mldemos),
      canvas(canvas),
      glw(glw),
      mutex(mutex),
      drawTimer(drawTimer),
      compare(compare),
      gridSearch(gridSearch),
      classifier(0),
      regressor(0),
      dynamical(0),
      clusterer(0),
      maximizer(0),
      reinforcement(0),
      projector(0),
      tabUsedForTraining(0),
      inputDimensions(0),
      manualSelection(0)
{
    options = new Ui::algorithmOptions();
    optionsClassify = new Ui::optionsClassifyWidget();
    optionsCluster = new Ui::optionsClusterWidget();
    optionsRegress = new Ui::optionsRegressWidget();
    optionsDynamic = new Ui::optionsDynamicWidget();
    optionsMaximize = new Ui::optionsMaximizeWidget();
    optionsReinforcement = new Ui::optionsReinforcementWidget();
    optionsProject = new Ui::optionsProjectWidget();

    algorithmWidget = new BaseWidget();
    options->setupUi(algorithmWidget);

    classifyWidget = new QWidget(options->tabClass);
    clusterWidget = new QWidget(options->tabClust);
    regressWidget = new QWidget(options->tabRegr);
    dynamicWidget = new QWidget(options->tabDyn);
    maximizeWidget = new QWidget(options->tabMax);
    reinforcementWidget = new QWidget(options->tabReinf);
    projectWidget = new QWidget(options->tabProj);
    optionsClassify->setupUi(classifyWidget);
    optionsCluster->setupUi(clusterWidget);
    optionsRegress->setupUi(regressWidget);
    optionsDynamic->setupUi(dynamicWidget);
    optionsMaximize->setupUi(maximizeWidget);
    optionsReinforcement->setupUi(reinforcementWidget);
    optionsProject->setupUi(projectWidget);
    if(options->tabClass->layout() == NULL) options->tabClass->setLayout(new QHBoxLayout());
    if(options->tabClust->layout() == NULL) options->tabClust->setLayout(new QHBoxLayout());
    if(options->tabRegr->layout() == NULL) options->tabRegr->setLayout(new QHBoxLayout());
    if(options->tabDyn->layout() == NULL) options->tabDyn->setLayout(new QHBoxLayout());
    if(options->tabMax->layout() == NULL) options->tabMax->setLayout(new QHBoxLayout());
    if(options->tabReinf->layout() == NULL) options->tabReinf->setLayout(new QHBoxLayout());
    if(options->tabProj->layout() == NULL) options->tabProj->setLayout(new QHBoxLayout());
    options->tabClass->layout()->setContentsMargins(0,0,0,0);
    options->tabClust->layout()->setContentsMargins(0,0,0,0);
    options->tabRegr->layout()->setContentsMargins(0,0,0,0);
    options->tabDyn->layout()->setContentsMargins(0,0,0,0);
    options->tabMax->layout()->setContentsMargins(0,0,0,0);
    options->tabReinf->layout()->setContentsMargins(0,0,0,0);
    options->tabProj->layout()->setContentsMargins(0,0,0,0);
    options->tabClass->layout()->addWidget(classifyWidget);
    options->tabClust->layout()->addWidget(clusterWidget);
    options->tabRegr->layout()->addWidget(regressWidget);
    options->tabDyn->layout()->addWidget(dynamicWidget);
    options->tabMax->layout()->addWidget(maximizeWidget);
    options->tabReinf->layout()->addWidget(reinforcementWidget);
    options->tabProj->layout()->addWidget(projectWidget);

    connect(gridSearch,SIGNAL(closed()),mldemos,SLOT(ResetGridSearchButton()));
    connect(algorithmWidget,SIGNAL(closed()),mldemos,SLOT(RestAlgorithmOptionsButton()));
    connect(optionsClassify->classifyButton, SIGNAL(clicked()), this, SLOT(Classify()));
    connect(optionsClassify->loadButton, SIGNAL(clicked()), this, SLOT(LoadClassifier()));
    connect(optionsClassify->saveButton, SIGNAL(clicked()), this, SLOT(SaveClassifier()));
    connect(optionsClassify->compareButton, SIGNAL(clicked()), this, SLOT(CompareAdd()));
    connect(compare->paramsWidget,SIGNAL(closed()),mldemos,SLOT(HideOptionCompare()));
    connect(optionsClassify->clearButton, SIGNAL(clicked()), mldemos, SLOT(Clear()));
    connect(optionsClassify->rocButton, SIGNAL(clicked()), mldemos, SLOT(ShowRoc()));
    connect(optionsClassify->manualTrainButton, SIGNAL(clicked()), mldemos, SLOT(ManualSelection()));
    connect(optionsClassify->inputDimButton, SIGNAL(clicked()), mldemos, SLOT(InputDimensions()));
    connect(optionsClassify->algoList, SIGNAL(currentIndexChanged(int)), mldemos, SLOT(AlgoChanged()));
    if (!optionsClassify->algoWidget->layout()) {
        QHBoxLayout *horizontalLayout = new QHBoxLayout(optionsClassify->algoWidget);
        horizontalLayout->setSpacing(0);
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
    }

    connect(optionsRegress->regressionButton, SIGNAL(clicked()), this, SLOT(Regression()));
    connect(optionsRegress->loadButton, SIGNAL(clicked()), this, SLOT(LoadRegressor()));
    connect(optionsRegress->saveButton, SIGNAL(clicked()), this, SLOT(SaveRegressor()));
    connect(optionsRegress->compareButton, SIGNAL(clicked()), this, SLOT(CompareAdd()));
    connect(optionsRegress->clearButton, SIGNAL(clicked()), mldemos, SLOT(Clear()));
    connect(optionsRegress->manualTrainButton, SIGNAL(clicked()), mldemos, SLOT(ManualSelection()));
    connect(optionsRegress->inputDimButton, SIGNAL(clicked()), mldemos, SLOT(InputDimensions()));
    connect(optionsRegress->algoList, SIGNAL(currentIndexChanged(int)), mldemos, SLOT(AlgoChanged()));
    if (!optionsRegress->algoWidget->layout()) {
        QHBoxLayout *horizontalLayout = new QHBoxLayout(optionsRegress->algoWidget);
        horizontalLayout->setSpacing(0);
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
    }

    connect(optionsCluster->clusterButton, SIGNAL(clicked()), this, SLOT(Cluster()));
    connect(optionsCluster->iterationButton, SIGNAL(clicked()), this, SLOT(ClusterIterate()));
    connect(optionsCluster->optimizeButton, SIGNAL(clicked()), this, SLOT(ClusterOptimize()));
    connect(optionsCluster->testButton, SIGNAL(clicked()), this, SLOT(ClusterTest()));
    connect(optionsCluster->optimizeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(ClusterChanged()));
    connect(optionsCluster->clearButton, SIGNAL(clicked()), mldemos, SLOT(Clear()));
    connect(optionsCluster->manualTrainButton, SIGNAL(clicked()), mldemos, SLOT(ManualSelection()));
    connect(optionsCluster->algoList, SIGNAL(currentIndexChanged(int)), mldemos, SLOT(AlgoChanged()));
    if (!optionsCluster->algoWidget->layout()) {
        QHBoxLayout *horizontalLayout = new QHBoxLayout(optionsCluster->algoWidget);
        horizontalLayout->setSpacing(0);
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
    }

    connect(optionsDynamic->regressionButton, SIGNAL(clicked()), this, SLOT(Dynamize()));
    connect(optionsDynamic->loadButton, SIGNAL(clicked()), this, SLOT(LoadDynamical()));
    connect(optionsDynamic->saveButton, SIGNAL(clicked()), this, SLOT(SaveDynamical()));
    connect(optionsDynamic->obstacleCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(AvoidOptionChanged()));
    connect(optionsDynamic->colorCheck, SIGNAL(clicked()), this, SLOT(ColorMapChanged()));
    connect(optionsDynamic->compareButton, SIGNAL(clicked()), this, SLOT(CompareAdd()));
    connect(optionsDynamic->clearButton, SIGNAL(clicked()), mldemos, SLOT(Clear()));
    connect(optionsDynamic->targetButton, SIGNAL(pressed()), mldemos, SLOT(TargetButton()));
    connect(optionsDynamic->clearTargetButton, SIGNAL(clicked()), mldemos, SLOT(ClearTargets()));
    connect(optionsDynamic->centerCombo, SIGNAL(currentIndexChanged(int)), mldemos, SLOT(ChangeActiveOptions()));
    connect(optionsDynamic->resampleCombo, SIGNAL(currentIndexChanged(int)), mldemos, SLOT(ChangeActiveOptions()));
    connect(optionsDynamic->resampleSpin, SIGNAL(valueChanged(int)), mldemos, SLOT(ChangeActiveOptions()));
    connect(optionsDynamic->resampleCombo, SIGNAL(currentIndexChanged(int)), mldemos, SLOT(ChangeActiveOptions()));
    connect(optionsDynamic->dtSpin, SIGNAL(valueChanged(double)), mldemos, SLOT(ChangeActiveOptions()));
    connect(optionsDynamic->algoList, SIGNAL(currentIndexChanged(int)), mldemos, SLOT(AlgoChanged()));
    optionsDynamic->targetButton->setAcceptDrops(true);
    if (!optionsDynamic->algoWidget->layout()) {
        QHBoxLayout *horizontalLayout = new QHBoxLayout(optionsDynamic->algoWidget);
        horizontalLayout->setSpacing(0);
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
    }

    connect(optionsMaximize->maximizeButton, SIGNAL(clicked()), this, SLOT(Maximize()));
    connect(optionsMaximize->pauseButton, SIGNAL(clicked()), this, SLOT(MaximizeContinue()));
    connect(optionsMaximize->compareButton, SIGNAL(clicked()), this, SLOT(CompareAdd()));
    connect(optionsMaximize->clearButton, SIGNAL(clicked()), mldemos, SLOT(Clear()));
    connect(optionsMaximize->targetButton, SIGNAL(pressed()), mldemos, SLOT(TargetButton()));
    connect(optionsMaximize->gaussianButton, SIGNAL(pressed()), mldemos, SLOT(GaussianButton()));
    connect(optionsMaximize->gradientButton, SIGNAL(pressed()), mldemos, SLOT(GradientButton()));
    connect(optionsMaximize->benchmarkButton, SIGNAL(clicked()), mldemos, SLOT(BenchmarkButton()));
    connect(optionsMaximize->algoList, SIGNAL(currentIndexChanged(int)), mldemos, SLOT(AlgoChanged()));
    if (!optionsMaximize->algoWidget->layout()) {
        QHBoxLayout *horizontalLayout = new QHBoxLayout(optionsMaximize->algoWidget);
        horizontalLayout->setSpacing(0);
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
    }

    connect(optionsReinforcement->maximizeButton, SIGNAL(clicked()), this, SLOT(Reinforce()));
    connect(optionsReinforcement->pauseButton, SIGNAL(clicked()), this, SLOT(ReinforceContinue()));
    connect(optionsReinforcement->clearButton, SIGNAL(clicked()), mldemos, SLOT(Clear()));
    connect(optionsReinforcement->targetButton, SIGNAL(pressed()), mldemos, SLOT(TargetButton()));
    connect(optionsReinforcement->clearTargetButton, SIGNAL(clicked()), mldemos, SLOT(ClearTargets()));
    connect(optionsReinforcement->gaussianButton, SIGNAL(pressed()), mldemos, SLOT(GaussianButton()));
    connect(optionsReinforcement->gradientButton, SIGNAL(pressed()), mldemos, SLOT(GradientButton()));
    connect(optionsReinforcement->benchmarkButton, SIGNAL(clicked()), mldemos, SLOT(BenchmarkButton()));
    connect(optionsReinforcement->algoList, SIGNAL(currentIndexChanged(int)), mldemos, SLOT(AlgoChanged()));
    if (!optionsReinforcement->algoWidget->layout()) {
        QHBoxLayout *horizontalLayout = new QHBoxLayout(optionsReinforcement->algoWidget);
        horizontalLayout->setSpacing(0);
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
    }

    connect(optionsProject->projectButton, SIGNAL(clicked()), this, SLOT(Project()));
    connect(optionsProject->manifoldButton, SIGNAL(clicked()), this, SLOT(ProjectManifold()));
    connect(optionsProject->revertButton, SIGNAL(clicked()), this, SLOT(ProjectRevert()));
    connect(optionsProject->reprojectButton, SIGNAL(clicked()), this, SLOT(ProjectReproject()));
    connect(optionsProject->manualTrainButton, SIGNAL(clicked()), mldemos, SLOT(ManualSelection()));
    connect(optionsProject->algoList, SIGNAL(currentIndexChanged(int)), mldemos, SLOT(AlgoChanged()));
    optionsProject->reprojectButton->setEnabled(false);
    optionsProject->revertButton->setEnabled(false);
    if (!optionsProject->algoWidget->layout()) {
        QHBoxLayout *horizontalLayout = new QHBoxLayout(optionsProject->algoWidget);
        horizontalLayout->setSpacing(0);
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
    }

    optionsClassify->algoList->clear();
    optionsCluster->algoList->clear();
    optionsRegress->algoList->clear();
    optionsDynamic->algoList->clear();
    optionsMaximize->algoList->clear();
    optionsReinforcement->algoList->clear();
    optionsProject->algoList->clear();

    connect(options->tabWidget, SIGNAL(currentChanged(int)), mldemos, SLOT(AlgoChanged()));

    //algorithmWidget->setWindowFlags(Qt::Tool); // disappears when unfocused on the mac
    //algorithmWidget->setFixedSize(800,430);

    drawTimer->classifier = &classifier;
    drawTimer->regressor = &regressor;
    drawTimer->dynamical = &dynamical;
    drawTimer->clusterer = &clusterer;
    drawTimer->maximizer = &maximizer;
    drawTimer->reinforcement = &reinforcement;
    drawTimer->reinforcementProblem = &reinforcementProblem;
    drawTimer->classifierMulti = &classifierMulti;
}

AlgorithmManager::~AlgorithmManager()
{
    mutex->lock();
    DEL(clusterer);
    DEL(regressor);
    DEL(dynamical);
    if (!classifierMulti.size()) DEL(classifier);
    classifier = 0;
    sourceDims.clear();
    FOR (i,classifierMulti.size()) DEL(classifierMulti[i]); classifierMulti.clear();
    DEL(maximizer);
    DEL(reinforcement);
    DEL(projector);
    mutex->unlock();

    DEL(optionsClassify);
    DEL(optionsRegress);
    DEL(optionsCluster);
    DEL(optionsDynamic);
    DEL(optionsMaximize);
    DEL(optionsReinforcement);
    DEL(optionsProject);
    DEL(options);
    DEL(algorithmWidget);
}

void AlgorithmManager::SetAlgorithms(QList<ClassifierInterface *> classifiers,
                  QList<ClustererInterface *> clusterers,
                  QList<RegressorInterface *> regressors,
                  QList<DynamicalInterface *> dynamicals,
                  QList<AvoidanceInterface *> avoiders,
                  QList<MaximizeInterface *> maximizers,
                  QList<ReinforcementInterface *> reinforcements,
                  QList<ProjectorInterface *> projectors,
                  QList<InputOutputInterface *> inputoutputs)
{
    this->classifiers = classifiers;
    this->clusterers = clusterers;
    this->regressors = regressors;
    this->dynamicals = dynamicals;
    this->avoiders = avoiders;
    this->maximizers = maximizers;
    this->reinforcements = reinforcements;
    this->projectors = projectors;
    this->inputoutputs = inputoutputs;
    if (!classifiers.size()) options->tabWidget->setTabEnabled(0,false);
    if (!clusterers.size()) options->tabWidget->setTabEnabled(1,false);
    if (!regressors.size()) options->tabWidget->setTabEnabled(2,false);
    if (!projectors.size()) options->tabWidget->setTabEnabled(3,false);
    if (!dynamicals.size()) options->tabWidget->setTabEnabled(4,false);
    if (!maximizers.size()) options->tabWidget->setTabEnabled(5,false);
    if (!reinforcements.size()) options->tabWidget->setTabEnabled(6,false);
}

void AlgorithmManager::QueryClassifier(std::vector<fvec> samples)
{
    std::vector<fvec> results;
    QMutexLocker lock(mutex);
    fvec result;
    result.resize(1);
    if (classifier && samples.size()) {
        results.resize(samples.size());
        FOR (i, samples.size()) {
            fvec sample = samples[i];
            if (sourceDims.size()) {
                fvec newSample(sourceDims.size());
                FOR (d, sourceDims.size()) newSample[d] = sample[sourceDims[d]];
                sample = newSample;
            }
            result[0] = classifier->Test(sample);
            results[i] = result;
        }
    }
    emit SendResults(results);
}

void AlgorithmManager::QueryRegressor(std::vector<fvec> samples)
{
    std::vector<fvec> results;
    QMutexLocker lock(mutex);
    if (regressor && samples.size()) {
        results.resize(samples.size());
        FOR (i, samples.size()) {
            results[i] = regressor->Test(samples[i]);
        }
    }
    emit SendResults(results);
}

void AlgorithmManager::QueryDynamical(std::vector<fvec> samples)
{
    std::vector<fvec> results;
    QMutexLocker lock(mutex);
    if (dynamical && samples.size()) {
        results.resize(samples.size());
        FOR (i, samples.size()) {
            results[i] = dynamical->Test(samples[i]);
        }
    }
    emit SendResults(results);
}

void AlgorithmManager::QueryClusterer(std::vector<fvec> samples)
{
    std::vector<fvec> results;
    QMutexLocker lock(mutex);
    if (clusterer && samples.size()) {
        results.resize(samples.size());
        FOR (i, samples.size()) {
            results[i] = clusterer->Test(samples[i]);
        }
    }
    emit SendResults(results);
}

void AlgorithmManager::QueryMaximizer(std::vector<fvec> samples)
{
    std::vector<fvec> results;
    QMutexLocker lock(mutex);
    if (maximizer && samples.size()) {
        results.resize(samples.size());
        FOR (i, samples.size()) {
            results[i] = maximizer->Test(samples[i]);
        }
    }
    emit SendResults(results);
}

void AlgorithmManager::QueryProjector(std::vector<fvec> samples)
{
    std::vector<fvec> results;
    QMutexLocker lock(mutex);
    if (projector && samples.size()) {
        results.resize(samples.size());
        FOR (i, samples.size()) {
            results[i] = projector->Project(samples[i]);
        }
    }
    emit SendResults(results);
}

bool AlgorithmManager::Train(Classifier *classifier, float trainRatio, bvec trainList, int positiveIndex, std::vector<fvec> samples, ivec labels)
{
    if(!classifier) return false;
    if(!labels.size()) labels = canvas->data->GetLabels();
    ivec inputDims = GetInputDimensions();
    if(!samples.size()) samples = canvas->data->GetSampleDims(inputDims);
    else samples = canvas->data->GetSampleDims(samples, inputDims);
    sourceDims = inputDims;

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
        trainCnt = trainSamples.size();
        testCnt = testSamples.size();
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

void AlgorithmManager::Train(Clusterer *clusterer, float trainRatio, bvec trainList, float *testFMeasures, std::vector<fvec> samples, ivec labels)
{
    if(!clusterer) return;
    if(!samples.size()) samples = canvas->data->GetSamples();
    if(!labels.size()) labels = canvas->data->GetLabels();
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

void AlgorithmManager::Train(Projector *projector, bvec trainList)
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


vector<bool> AlgorithmManager::GetManualSelection()
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

ivec AlgorithmManager::GetInputDimensions()
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

QStringList AlgorithmManager::GetInfoFiles()
{
    QStringList infoFiles;
    infoFiles << QString() << QString();
    QString infoFile, mainFile;
    if (options->tabClass->isVisible())
    {
        mainFile = "classification.html";
        int tab = optionsClassify->algoList->currentIndex();
        if (tab < 0 || tab >= (int)classifiers.size() || !classifiers[tab]) return infoFiles;
        infoFile = classifiers[tab]->GetInfoFile();
    }
    if (options->tabClust->isVisible())
    {
        mainFile = "clustering.html";
        int tab = optionsCluster->algoList->currentIndex();
        if (tab < 0 || tab >= (int)clusterers.size() || !clusterers[tab]) return infoFiles;
        infoFile = clusterers[tab]->GetInfoFile();
    }
    if (options->tabRegr->isVisible())
    {
        mainFile = "regression.html";
        int tab = optionsRegress->algoList->currentIndex();
        if (tab < 0 || tab >= (int)regressors.size() || !regressors[tab]) return infoFiles;
        infoFile = regressors[tab]->GetInfoFile();
    }
    if (options->tabDyn->isVisible())
    {
        mainFile = "dynamical.html";
        int tab = optionsDynamic->algoList->currentIndex();
        if (tab < 0 || tab >= (int)dynamicals.size() || !dynamicals[tab]) return infoFiles;
        infoFile = dynamicals[tab]->GetInfoFile();
    }
    if (options->tabMax->isVisible())
    {
        mainFile = "maximization.html";
        int tab = optionsMaximize->algoList->currentIndex();
        if (tab < 0 || tab >= (int)maximizers.size() || !maximizers[tab]) return infoFiles;
        infoFile = maximizers[tab]->GetInfoFile();
    }
    if (options->tabProj->isVisible())
    {
        mainFile = "projection.html";
        int tab = optionsProject->algoList->currentIndex();
        if (tab < 0 || tab >= (int)projectors.size() || !projectors[tab]) return infoFiles;
        infoFile = projectors[tab]->GetInfoFile();
    }
    if (options->tabReinf->isVisible())
    {
        mainFile = "reinforcement.html";
        int tab = optionsReinforcement->algoList->currentIndex();
        if (tab < 0 || tab >= (int)reinforcements.size() || !reinforcements[tab]) return infoFiles;
        infoFile = reinforcements[tab]->GetInfoFile();
    }
    infoFiles[0] = infoFile;
    infoFiles[1] = mainFile;
    return infoFiles;
}


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
            if(canvas->data->GetDimCount() == 3 && (sourceDims.size()==0 || sourceDims.size()==3)) Draw3DClassifier(glw, classifier);
        }

        emit UpdateInfo();
        if(drawTimer && classifier->UsesDrawTimer())
        {
            drawTimer->start(QThread::NormalPriority);
        }
        if(canvas->canvasType) emit CanvasOptionsChanged();
        // we fill in the canvas sampleColors
        ivec inputDims = GetInputDimensions();
        vector<fvec> samples = canvas->data->GetSampleDims(inputDims);
        canvas->sampleColors.resize(samples.size());
        FOR(i, samples.size())
        {
            canvas->sampleColors[i] = DrawTimer::GetColor(classifier, samples[i], &classifierMulti);
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

    return -fmeasure;
}


void AlgorithmManager::Cluster()
{
    if(!canvas || !canvas->data->GetCount()) return;
    drawTimer->Stop();
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
    glw->clearLists();
    if(canvas->canvasType == 1)
    {
        clusterers[tab]->DrawGL(canvas, glw, clusterer);
        if(canvas->data->GetDimCount() == 3) Draw3DClusterer(glw, clusterer);
    }

    // we compute the stats on the clusters (f-measure, bic etc)

    vector<fvec> samples = canvas->data->GetSamples();
    ivec labels = canvas->data->GetLabels();
    float logLikelihood = clusterer->GetLogLikelihood(samples);
    float BIC = -2*logLikelihood + logf(samples.size())*clusterer->GetParameterCount(); // BIC
    float AIC = -2*logLikelihood + 2*clusterer->GetParameterCount(); // AIC

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
    optionsCluster->resultList->addItem(QString("lik: %1").arg(logLikelihood, 0, 'f', 2));
    optionsCluster->resultList->addItem(QString("bic: %1").arg(BIC, 0, 'f', 2));
    optionsCluster->resultList->addItem(QString("aic: %1").arg(AIC, 0, 'f', 2));
    optionsCluster->resultList->addItem(QString("f1: %1").arg(F1, 0, 'f', 2));

    optionsCluster->resultList->item(0)->setForeground(Qt::gray);
    optionsCluster->resultList->item(1)->setForeground(SampleColor[1]);
    optionsCluster->resultList->item(2)->setForeground(SampleColor[2]);
    optionsCluster->resultList->item(3)->setForeground(SampleColor[3]);

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

    emit UpdateInfo();
    /*
    QString infoText = showStats->infoText->text();
    infoText += "\nClustering as Classifier\n";
    infoText += QString("F-Measure: %1\n").arg(testError, 0, 'f', 3);
    showStats->infoText->setText(infoText);
    */

    drawTimer->clusterer= &this->clusterer;
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
    drawTimer->start(QThread::NormalPriority);
    canvas->repaint();
}

void AlgorithmManager::ClusterOptimize()
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

    vector<bool> trainList; // oh
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

            vector<fvec> clusterScores(samples.size());
            FOR(i, samples.size())
            {
                fvec result = clusterer->Test(samples[i]);
                if(clusterer->NbClusters()==1) clusterScores[i] = result;
                else if(result.size()>1) clusterScores[i] = result;
            }

            fvec clusterMetrics(4);

            float logLikelihood = clusterer->GetLogLikelihood(samples);
            float BIC = -2*logLikelihood + k*logf(samples.size())*clusterer->GetParameterCount(); // BIC
            float AIC = -2*logLikelihood + 2*clusterer->GetParameterCount(); // AIC
            clusterMetrics[0] = logLikelihood;
            clusterMetrics[1] = BIC;
            clusterMetrics[2] = AIC;
            clusterMetrics[3] = ClusterFMeasure(samples, labels, clusterScores, ratio);

            FOR(i, clusterMetrics.size())
            {
                resultList[i][j].push_back(clusterMetrics[i]);
            }
        }
        kCounts.push_back(k);
    }
    vector<fvec> results(4);
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

    qreal xpos, ypos;
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
            if(std::isnan(y)){y=0;}

            //if(i == 3) y = 1.f - y; // fmeasures needs to be maximized


            QPointF point(x*(w-2*pad)+pad,(1.f-y)*(h-2*pad));
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
    glw->clearLists();
    if(canvas->canvasType == 1)
    {
        clusterers[tab]->DrawGL(canvas, glw, clusterer);
        if(canvas->data->GetDimCount() == 3) Draw3DClusterer(glw, clusterer);
    }

    emit UpdateInfo();
    /*
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
    */

    drawTimer->clusterer= &this->clusterer;
    drawTimer->start(QThread::NormalPriority);
    canvas->repaint();
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

    emit UpdateInfo();

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

void AlgorithmManager::Project()
{
    std::cout<< "AlgorithmManager::Project()" << std::endl;
    if(!canvas || !canvas->data->GetCount()) return;
    QMutexLocker lock(mutex);
    drawTimer->Stop();
    drawTimer->Clear();
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
    emit CanvasTypeChanged();
    emit CanvasOptionsChanged();
    emit ResetPositiveClass();
    projectors[tab]->Draw(canvas, projector);
    glw->clearLists();
    if(canvas->canvasType == 1)
    {
        projectors[tab]->DrawGL(canvas, glw, projector);
        if(canvas->data->GetDimCount() == 3) Draw3DProjector(glw, projector);
    }
    optionsProject->reprojectButton->setEnabled(true);
    optionsProject->revertButton->setEnabled(true);
    canvas->repaint();
    emit UpdateInfo();

}

void AlgorithmManager::ProjectManifold()
{
    if(!canvas || !canvas->data->GetCount()) return;
    QMutexLocker lock(mutex);
    drawTimer->Stop();
    drawTimer->Clear();
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

    emit CanvasTypeChanged();
    emit CanvasOptionsChanged();
    emit ResetPositiveClass();
    projectors[tab]->Draw(canvas, projector);
    glw->clearLists();
    if(canvas->canvasType == 1)
    {
        projectors[tab]->DrawGL(canvas, glw, projector);
        if(canvas->data->GetDimCount() == 3) Draw3DProjector(glw, projector);
    }
    canvas->repaint();
    emit UpdateInfo();
}

void AlgorithmManager::ProjectRevert()
{
    QMutexLocker lock(mutex);
    drawTimer->Stop();
    drawTimer->Clear();
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
    if(!sourceData.size()) return;
    canvas->data->SetSamples(sourceData);
    canvas->data->SetLabels(sourceLabels);
    canvas->data->bProjected = false;
    canvas->maps.info = QPixmap();
    canvas->maps.model = QPixmap();
    canvas->maps.confidence = QPixmap();
    if(optionsProject->fitCheck->isChecked()) canvas->FitToData();
    optionsProject->reprojectButton->setEnabled(false);
    optionsProject->revertButton->setEnabled(false);
    emit CanvasTypeChanged();
    emit CanvasOptionsChanged();
    emit ResetPositiveClass();
    canvas->repaint();
    glw->clearLists();
    emit UpdateInfo();

    sourceData.clear();
    sourceLabels.clear();
}

void AlgorithmManager::ProjectReproject()
{
    if(!canvas || !canvas->data->GetCount()) return;
    mutex->lock();
    sourceData = canvas->data->GetSamples();
    sourceLabels = canvas->data->GetLabels();
    mutex->unlock();
    Project();
}

void AlgorithmManager::UpdateLearnedModel()
{
    if(!canvas) return;
    if(!clusterer && !regressor && !dynamical && !classifier && !projector) return;
    if(glw) glw->clearLists();
    if(classifier)
    {
        QMutexLocker lock(mutex);
        if(glw->isVisible())
        {
            glw->clearLists();
            if(canvas->canvasType == 1)
            {
                classifiers[tabUsedForTraining]->DrawGL(canvas, glw, classifier);
                if(canvas->data->GetDimCount() == 3 && (sourceDims.size()==0 || sourceDims.size()==3)) Draw3DClassifier(glw, classifier);
            }
        }
        else
        {
            classifiers[tabUsedForTraining]->Draw(canvas, classifier);
            DrawClassifiedSamples(canvas, classifier, classifierMulti);
            if(classifier->UsesDrawTimer() && !drawTimer->isRunning())
            {
                drawTimer->start(QThread::NormalPriority);
            }
        }
    }
    if(clusterer)
    {
        QMutexLocker lock(mutex);
        if(glw->isVisible())
        {
            glw->clearLists();
            if(canvas->canvasType == 1)
            {
                clusterers[tabUsedForTraining]->DrawGL(canvas, glw, clusterer);
                if(canvas->data->GetDimCount() == 3) Draw3DClusterer(glw, clusterer);
            }
        }
        else clusterers[tabUsedForTraining]->Draw(canvas, clusterer);
    }
    if(regressor)
    {
        QMutexLocker lock(mutex);
        glw->clearLists();
        if(canvas->canvasType == 1)
        {
            regressors[tabUsedForTraining]->DrawGL(canvas, glw, regressor);
            if(canvas->data->GetDimCount() == 3) Draw3DRegressor(glw, regressor);
        }

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
        QMutexLocker lock(mutex);
        if(glw->isVisible())
        {
            glw->clearLists();
            if(canvas->canvasType == 1)
            {
                dynamicals[tabUsedForTraining]->DrawGL(canvas, glw, dynamical);
                if(canvas->data->GetDimCount() == 3)
                {
                    int displayStyle = optionsDynamic->displayCombo->currentIndex();
                    if(displayStyle < 3) Draw3DDynamical(glw, dynamical,displayStyle);
                }
            }
        }
        else
        {
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

            if(dynamicals[tabUsedForTraining]->UsesDrawTimer())
            {
                drawTimer->bColorMap = bColorMap;
                drawTimer->start(QThread::NormalPriority);
            }
        }
    }
    if(!canvas->canvasType && projector)
    {
        if(glw->isVisible())
        {
            glw->clearLists();
            if(canvas->canvasType == 1)
            {
                projectors[tabUsedForTraining]->DrawGL(canvas, glw, projector);
                if(canvas->data->GetDimCount() == 3) Draw3DProjector(glw, projector);
            }
        }
        else projectors[tabUsedForTraining]->Draw(canvas, projector);
    }
    emit UpdateInfo();

}

void AlgorithmManager::DrawClassifiedSamples(Canvas *canvas, Classifier *classifier, std::vector<Classifier *> classifierMulti)
{
    if(!canvas || !classifier) return;
    int w = canvas->width(), h = canvas->height();
    canvas->maps.model = QPixmap(w,h);
    //QBitmap bitmap(w,h);
    //bitmap.clear();
    //canvas->maps.model.setMask(bitmap);
    canvas->maps.model.fill(Qt::transparent);
    QPainter painter(&canvas->maps.model);
    int posClass = INT_MIN;
    FORIT(classifier->classMap, int, int)
    {
        posClass = max(posClass, it->first);
    }

    QString s;
    FOR(d, sourceDims.size()) s += QString("%1 ").arg(sourceDims[d]);

    int forcedPositive = classifier->inverseMap[-1] < 0 ? -classifier->inverseMap[-1]-1 : -1;

    // we draw the samples
    painter.setRenderHint(QPainter::Antialiasing, true);
    FOR(i, canvas->data->GetCount())
    {
        fvec sample = sourceDims.size() ? canvas->data->GetSampleDim(i, sourceDims) : canvas->data->GetSample(i);
        int label = canvas->data->GetLabel(i);
        QPointF point = canvas->toCanvasCoords(canvas->data->GetSample(i));
        fvec res;
        if(classifier->IsMultiClass()) res = classifier->TestMulti(sample);
        else if(classifierMulti.size())
        {
            FOR(c, classifierMulti.size())
            {
                res.push_back(classifierMulti[c]->Test(sample));
            }
        }
        else res.push_back(classifier->Test(sample));
        if(res.size()==1)
        {
            float response = res[0];
            if(forcedPositive != -1) // we forced binary classification
            {
                if(response > 0)
                {
                    if(label == forcedPositive) Canvas::drawSample(painter, point, 9, 1);
                    else Canvas::drawCross(painter, point, 6, 2);
                }
                else
                {
                    if(label != forcedPositive) Canvas::drawSample(painter, point, 9, 0);
                    else Canvas::drawCross(painter, point, 6, 0);
                }
            }
            else
            {
                if(response > 0)
                {
                    if(label != classifier->inverseMap[-1]) Canvas::drawSample(painter, point, 9, 1);
                    else Canvas::drawCross(painter, point, 6, 2);
                }
                else
                {
                    if(label == classifier->inverseMap[-1]) Canvas::drawSample(painter, point, 9, 0);
                    else Canvas::drawCross(painter, point, 6, 0);
                }
            }
        }
        else
        {
            int max = 0;
            for(int i=1; i<res.size(); i++) if(res[max] < res[i]) max = i;
            int resp = classifier->inverseMap[max];
            if(label == resp) Canvas::drawSample(painter, point, 9, label);
            else Canvas::drawCross(painter, point, 6, label);
        }
    }
}

void AlgorithmManager::AvoidOptionChanged()
{
    if (dynamical)
    {
        int avoidIndex = optionsDynamic->obstacleCombo->currentIndex();
        mutex->lock();
        if (dynamical->avoid) delete dynamical->avoid;
        if (!avoidIndex) dynamical->avoid = 0;
        else dynamical->avoid = avoiders[avoidIndex-1]->GetObstacleAvoidance();
        mutex->unlock();
        drawTimer->Stop();
        drawTimer->Clear();
        drawTimer->start(QThread::NormalPriority);
    }
}

void AlgorithmManager::ColorMapChanged()
{
    if (dynamical)
    {
        drawTimer->Stop();
        drawTimer->Clear();
        drawTimer->bColorMap = optionsDynamic->colorCheck->isChecked();
        drawTimer->start(QThread::NormalPriority);
    }
}

void AlgorithmManager::ClusterChanged()
{
    if (optionsCluster->optimizeCombo->currentIndex() == 3) { // F1
        optionsCluster->trainRatioCombo->setVisible(true);
        optionsCluster->trainRatioF1->setVisible(true);
    } else {
        optionsCluster->trainRatioCombo->setVisible(false);
        optionsCluster->trainRatioF1->setVisible(false);

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
    mldemos->actionCompare->setChecked(true);
    compare->Show();
}

void AlgorithmManager::SetAlgorithmWidget()
{
    if (options->tabClass->isVisible()) {
        int index = optionsClassify->algoList->currentIndex();
        FOR (i, algoWidgets["classifiers"].size()) {
            if (i==index) algoWidgets["classifiers"][i]->show();
            else algoWidgets["classifiers"][i]->hide();
        }
        if (index != -1 && index < classifiers.size()) gridSearch->SetClassifier(classifiers[index]);
    }
    if (options->tabClust->isVisible()) {
        int index = optionsCluster->algoList->currentIndex();
        FOR (i, algoWidgets["clusterers"].size()) {
            if (i==index){
                algoWidgets["clusterers"][i]->show();
            }
            else algoWidgets["clusterers"][i]->hide();
        }
        if (index != -1 && index < clusterers.size()) gridSearch->SetClusterer(clusterers[index]);
    }
    if (options->tabDyn->isVisible()) {
        int index = optionsDynamic->algoList->currentIndex();
        FOR (i, algoWidgets["dynamicals"].size()) {
            if (i==index) algoWidgets["dynamicals"][i]->show();
            else algoWidgets["dynamicals"][i]->hide();
        }
        if (index != -1 && index < dynamicals.size()) gridSearch->SetDynamical(dynamicals[index]);
    }
    if (options->tabMax->isVisible()) {
        int index = optionsMaximize->algoList->currentIndex();
        FOR (i, algoWidgets["maximizers"].size()) {
            if (i==index) algoWidgets["maximizers"][i]->show();
            else algoWidgets["maximizers"][i]->hide();
        }
        if (index != -1 && index < maximizers.size()) gridSearch->SetMaximizer(maximizers[index]);
    }
    if (options->tabProj->isVisible()) {
        int index = optionsProject->algoList->currentIndex();
        QWidget *projection_widget=NULL;
        QCheckBox* qCheckBox=NULL;
        FOR (i, algoWidgets["projectors"].size()) {
            if (i==index){
                projection_widget = algoWidgets["projectors"][i];
                projection_widget->show();
                qCheckBox = projection_widget->findChild<QCheckBox*>("useRangeCheck");
                if(qCheckBox != NULL){
                    QSpinBox* startRangeSpin = projection_widget->findChild<QSpinBox*>("startRangeSpin");
                    QSpinBox* stopRangeSpin = projection_widget->findChild<QSpinBox*>("stopRangeSpin");
                    assert(startRangeSpin != NULL);
                    assert(stopRangeSpin != NULL);
                    unsigned int dim = canvas->data->GetDimCount();
                    startRangeSpin->setMaximum(dim);
                    stopRangeSpin->setMaximum(dim);
                }
            }
            else algoWidgets["projectors"][i]->hide();
        }
        if (index != -1 && index < projectors.size()) gridSearch->SetProjector(projectors[index]);
    }
    if (options->tabRegr->isVisible()) {
        int index = optionsRegress->algoList->currentIndex();
        FOR (i, algoWidgets["regressors"].size()) {
            if (i==index) algoWidgets["regressors"][i]->show();
            else algoWidgets["regressors"][i]->hide();
        }
        if (index != -1 && index < regressors.size()) gridSearch->SetRegressor(regressors[index]);
    }
    if (options->tabReinf->isVisible()) {
        int index = optionsReinforcement->algoList->currentIndex();
        FOR (i, algoWidgets["reinforcements"].size()) {
            if (i==index) algoWidgets["reinforcements"][i]->show();
            else algoWidgets["reinforcements"][i]->hide();
        }
        if (index != -1 && index < reinforcements.size()) gridSearch->SetReinforcement(reinforcements[index]);
    }
}


void AlgorithmManager::LoadClassifier()
{
    QString filename = QFileDialog::getOpenFileName(mldemos, tr("Load Model"), "", tr("Model (*.model)"));
    if(filename.isEmpty()) return;
    int tab = optionsClassify->algoList->currentIndex();
    if(tab >= classifiers.size() || !classifiers[tab]) return;
    Classifier *classifier = classifiers[tab]->GetClassifier();
    bool ok = classifier->LoadModel(filename.toStdString());
    if(ok)
    {
        if(!classifierMulti.size()) DEL(this->classifier);
        this->classifier = 0;
        FOR(i,classifierMulti.size()) DEL(classifierMulti[i]); classifierMulti.clear();
        this->classifier = classifier;
        tabUsedForTraining = tab;
        classifiers[tab]->Draw(canvas, classifier);
        DrawClassifiedSamples(canvas, classifier, classifierMulti);
        if(drawTimer->isRunning()) drawTimer->Stop();
        drawTimer->Clear();
        drawTimer->start(QThread::NormalPriority);
    }
    else DEL(classifier);
}

void AlgorithmManager::SaveClassifier()
{
    if(!classifier) return;
    QString filename = QFileDialog::getSaveFileName(mldemos, tr("Save Model"), "", tr("Model (*.model)"));
    if(filename.isEmpty()) return;
    if(!filename.endsWith(".model")) filename += ".model";
    classifier->SaveModel(filename.toStdString());
}

void AlgorithmManager::LoadRegressor()
{
    QString filename = QFileDialog::getOpenFileName(mldemos, tr("Load Model"), "", tr("Model (*.model)"));
    if(filename.isEmpty()) return;
    int tab = optionsRegress->algoList->currentIndex();
    if(tab >= regressors.size() || !regressors[tab]) return;
    Regressor *regressor = regressors[tab]->GetRegressor();
    bool ok = regressor->LoadModel(filename.toStdString());
    if(ok)
    {
        DEL(this->regressor);
        this->regressor = regressor;
        tabUsedForTraining = tab;
        regressors[tab]->Draw(canvas, regressor);
        if(drawTimer->isRunning()) drawTimer->Stop();
        drawTimer->Clear();
        drawTimer->start(QThread::NormalPriority);
    }
    else DEL(regressor);
}

void AlgorithmManager::SaveRegressor()
{
    if(!regressor) return;
    QString filename = QFileDialog::getSaveFileName(mldemos, tr("Save Model"), "", tr("Model (*.model)"));
    if(filename.isEmpty()) return;
    if(!filename.endsWith(".model")) filename += ".model";
    regressor->SaveModel(filename.toStdString());
}

void AlgorithmManager::LoadDynamical()
{
    QString filename = QFileDialog::getOpenFileName(mldemos, tr("Load Model"), "", tr("Model (*.model)"));
    if(filename.isEmpty()) return;
    int tab = optionsDynamic->algoList->currentIndex();
    if(tab >= dynamicals.size() || !dynamicals[tab]) return;
    Dynamical *dynamical = dynamicals[tab]->GetDynamical();
    bool ok = dynamical->LoadModel(filename.toStdString());
    if(ok)
    {
        DEL(this->dynamical);
        this->dynamical = dynamical;
        tabUsedForTraining = tab;
        dynamicals[tab]->Draw(canvas, dynamical);
        if(dynamicals[tab]->UsesDrawTimer())
        {
            if(drawTimer->isRunning()) drawTimer->Stop();
            drawTimer->Clear();
            drawTimer->bColorMap = optionsDynamic->colorCheck->isChecked();
            drawTimer->start(QThread::NormalPriority);
        }
    }
    else DEL(dynamical);
}

void AlgorithmManager::SaveDynamical()
{
    if(!dynamical) return;
    QString filename = QFileDialog::getSaveFileName(mldemos, tr("Save Model"), "", tr("Model (*.model)"));
    if(filename.isEmpty()) return;
    if(!filename.endsWith(".model")) filename += ".model";
    dynamical->SaveModel(filename.toStdString());
}

void AlgorithmManager::Clear()
{
    if (!classifierMulti.size()) DEL(classifier);
    classifier = 0;
    sourceDims.clear();
    FOR (i,classifierMulti.size()) DEL(classifierMulti[i]); classifierMulti.clear();
    DEL(regressor);
    DEL(dynamical);
    DEL(clusterer);
    DEL(maximizer);
    DEL(reinforcement);
    DEL(projector);
    optionsProject->reprojectButton->setEnabled(false);
    optionsProject->revertButton->setEnabled(false);
    sourceDims.clear();
}

void AlgorithmManager::ClearData()
{
    sourceData.clear();
    sourceLabels.clear();
    projectedData.clear();
    optionsProject->reprojectButton->setEnabled(false);
    optionsProject->revertButton->setEnabled(false);
}
