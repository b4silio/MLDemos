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
#include <assert.h>

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
    connect(optionsClassify->manualTrainButton, SIGNAL(clicked()), mldemos, SLOT(ShowManualSelection()));
    connect(optionsClassify->inputDimButton, SIGNAL(clicked()), mldemos, SLOT(ShowInputDimensions()));
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
    connect(optionsRegress->manualTrainButton, SIGNAL(clicked()), mldemos, SLOT(ShowManualSelection()));
    connect(optionsRegress->inputDimButton, SIGNAL(clicked()), mldemos, SLOT(ShowInputDimensions()));
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
    connect(optionsCluster->manualTrainButton, SIGNAL(clicked()), mldemos, SLOT(ShowManualSelection()));
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
    connect(optionsProject->manualTrainButton, SIGNAL(clicked()), mldemos, SLOT(ShowManualSelection()));
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
        drawTimer->inputDims = GetInputDimensions();
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
        drawTimer->inputDims = GetInputDimensions();
        drawTimer->start(QThread::NormalPriority);
    }
}

void AlgorithmManager::ClusterChanged()
{
    if (optionsCluster->optimizeCombo->currentIndex() == 4) { // F1
        optionsCluster->trainRatioCombo->setVisible(true);
        optionsCluster->trainRatioF1->setVisible(true);
    } else {
        optionsCluster->trainRatioCombo->setVisible(false);
        optionsCluster->trainRatioF1->setVisible(false);

    }
}

void AlgorithmManager::RestrictDimChanged()
{
    optionsClassify->inputDimButton->setEnabled(!mldemos->ui.restrictDimCheck->isChecked());
    optionsRegress->inputDimButton->setEnabled(!mldemos->ui.restrictDimCheck->isChecked());
    optionsRegress->outputDimCombo->setEnabled(!mldemos->ui.restrictDimCheck->isChecked());
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
