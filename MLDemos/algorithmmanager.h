#ifndef ALGORITHMMANAGER_H
#define ALGORITHMMANAGER_H

#include <QList>
#include "canvas.h"
#include "classifier.h"
#include "regressor.h"
#include "dynamical.h"
#include "clusterer.h"
#include "maximize.h"
#include "reinforcement.h"
#include "reinforcementProblem.h"
#include "interfaces.h"
#include "compare.h"
#include "widget.h"
#include "drawTimer.h"
#include "gridsearch.h"
#include "basewidget.h"

#include "ui_algorithmOptions.h"
#include "ui_optsClassify.h"
#include "ui_optsCluster.h"
#include "ui_optsRegress.h"
#include "ui_optsMaximize.h"
#include "ui_optsReinforcement.h"
#include "ui_optsDynamic.h"
#include "ui_optsProject.h"
#include "ui_manualSelection.h"
#include "ui_inputDimensions.h"

class MLDemos;

class AlgorithmManager : public QObject
{
    Q_OBJECT

public:
    BaseWidget *algorithmWidget;
    QWidget *regressWidget;
    QWidget *dynamicWidget;
    QWidget *classifyWidget;
    QWidget *clusterWidget;
    QWidget *maximizeWidget;
    QWidget *reinforcementWidget;
    QWidget *projectWidget;

    Ui::algorithmOptions *options;
    Ui::optionsClassifyWidget *optionsClassify;
    Ui::optionsClusterWidget *optionsCluster;
    Ui::optionsRegressWidget *optionsRegress;
    Ui::optionsMaximizeWidget *optionsMaximize;
    Ui::optionsDynamicWidget *optionsDynamic;
    Ui::optionsProjectWidget *optionsProject;
    Ui::optionsReinforcementWidget *optionsReinforcement;
    Ui::ManualSelection* manualSelection;
    Ui::InputDimensions* inputDimensions;

    QList<ClassifierInterface *> classifiers;
    QList<ClustererInterface *> clusterers;
    QList<RegressorInterface *> regressors;
    QList<DynamicalInterface *> dynamicals;
    QList<AvoidanceInterface *> avoiders;
    QList<MaximizeInterface*> maximizers;
    QList<ReinforcementInterface*> reinforcements;
    QList<ProjectorInterface*> projectors;
    QList<InputOutputInterface *> inputoutputs;
    QList<bool> bInputRunning;

    std::map< QString , std::vector<QWidget*> > algoWidgets;
    QList<QPluginLoader*> pluginLoaders;

    QString lastTrainingInfo;
    int tabUsedForTraining;
    Classifier *classifier;
    Regressor *regressor;
    Dynamical *dynamical;
    Clusterer *clusterer;
    Maximizer *maximizer;
    Reinforcement *reinforcement;
    ReinforcementProblem reinforcementProblem;
    Projector *projector;
    std::vector<Classifier *> classifierMulti;
    std::vector<fvec> sourceData;
    std::vector<fvec> projectedData;
    ivec sourceLabels;
    ivec sourceDims;

    Canvas *canvas;
    GLWidget *glw;
    QMutex *mutex;
    DrawTimer *drawTimer;
    CompareAlgorithms *compare;
    GridSearch *gridSearch;
    MLDemos *mldemos;

public:
    AlgorithmManager(MLDemos *mldemos,
                     Canvas *canvas,
                     GLWidget *glw,
                     QMutex *mutex,
                     DrawTimer *drawTimer,
                     CompareAlgorithms *compare,
                     GridSearch *gridSearch);
    ~AlgorithmManager();

    void SetAlgorithms(QList<ClassifierInterface *> classifiers,
                      QList<ClustererInterface *> clusterers,
                      QList<RegressorInterface *> regressors,
                      QList<DynamicalInterface *> dynamicals,
                      QList<AvoidanceInterface *> avoiders,
                      QList<MaximizeInterface *> maximizers,
                      QList<ReinforcementInterface *> reinforcements,
                      QList<ProjectorInterface *> projectors,
                      QList<InputOutputInterface *> inputoutputs);

    bool Train(Classifier *classifier, float trainRatio=1, bvec trainList = bvec(), int positiveIndex=-1,
               std::vector<fvec> samples=std::vector<fvec>(), ivec labels=ivec());
    void Train(Regressor *regressor, int outputDim=-1, float trainRatio=1, bvec trainList = bvec(), std::vector<fvec> samples=std::vector<fvec>(), ivec labels=ivec());
    fvec Train(Dynamical *dynamical);
    void Train(Clusterer *clusterer, float trainRatio=1, bvec trainList = bvec(), float *testFMeasures=0, std::vector<fvec> samples=std::vector<fvec>(), ivec labels=ivec());
    void Train(Maximizer *maximizer);
    void Train(Reinforcement *reinforcement);
    void Train(Projector *projector, bvec trainList = bvec());
    fvec Test(Dynamical *dynamical, std::vector< std::vector<fvec> > trajectories, ivec labels);
    void Test(Maximizer *maximizer);
    float ClusterFMeasure(std::vector<fvec> samples, ivec labels, std::vector<fvec> scores, float ratio = 1.f);
    void DrawClassifiedSamples(Canvas *canvas, Classifier *classifier, std::vector<Classifier *> classifierMulti);
    void UpdateLearnedModel();

    std::vector<bool> GetManualSelection();
    ivec GetInputDimensions();
    QStringList GetInfoFiles();

signals:
    void Trained();
    void UpdateInfo();
    void CanvasOptionsChanged();
    void CanvasTypeChanged();
    void DisplayOptionsChanged();
    void ResetPositiveClass();
    void SendResults(std::vector<fvec> results);

public slots:
    // running the algorithms
    void Clear();
    void ClearData();
    void Classify();
    void Regression();
    void Maximize();
    void MaximizeContinue();
    void Reinforce();
    void ReinforceContinue();
    void Dynamize();
    void Cluster();
    void ClusterIterate();
    void ClusterOptimize();
    void ClusterTest();
    void Project();
    void ProjectManifold();
    void ProjectRevert();
    void ProjectReproject();
    void Avoidance();
    void Compare();
    void CompareAdd();

    // saving/loading the algorithms
    void LoadClassifier();
    void SaveClassifier();
    void LoadRegressor();
    void SaveRegressor();
    void LoadDynamical();
    void SaveDynamical();

    // interface updates
    void AvoidOptionChanged();
    void ColorMapChanged();
    void ClusterChanged();
    void SetAlgorithmWidget();

    void QueryClassifier(std::vector<fvec> samples);
    void QueryRegressor(std::vector<fvec> samples);
    void QueryDynamical(std::vector<fvec> samples);
    void QueryClusterer(std::vector<fvec> samples);
    void QueryMaximizer(std::vector<fvec> samples);
    void QueryProjector(std::vector<fvec> samples);
};

#endif // ALGORITHMMANAGER_H
