#include "pluginmanager.h"
#include "mldemos.h"

PluginManager::PluginManager(MLDemos *mldemos, AlgorithmManager *algo)
    : mldemos(mldemos), algo(algo)
{
}

PluginManager::~PluginManager()
{
    FOR (i, inputoutputs.size()) {
        if (inputoutputs[i] && bInputRunning[i]) inputoutputs[i]->Stop();
        DEL(inputoutputs[i]);
    }
    FOR (i, pluginLoaders.size()) {
        pluginLoaders.at(i)->unload();
        DEL(pluginLoaders[i]);
    }
}

void PluginManager::LoadPlugins()
{
    qDebug() << "Importing plugins";
    QDir pluginsDir = QDir(qApp->applicationDirPath());
    QDir alternativeDir = pluginsDir;

#if defined(Q_OS_WIN)
    if (pluginsDir.dirName().toLower() == "debug" || pluginsDir.dirName().toLower() == "release") pluginsDir.cdUp();
#elif defined(Q_OS_MAC)
    if (pluginsDir.dirName() == "MacOS") {
        if (!pluginsDir.cd("plugins")) {
            qDebug() << "looking for alternative directory";
            pluginsDir.cdUp();
            pluginsDir.cdUp();
            alternativeDir = pluginsDir;
            alternativeDir.cd("plugins");
        }
        pluginsDir.cdUp();
    }
#endif
    bool bFoundPlugins = false;
#if defined(DEBUG)
    qDebug() << "looking for debug plugins";
    bFoundPlugins = pluginsDir.cd("pluginsDebug");
#else
    qDebug() << "looking for release plugins";
    bFoundPlugins = pluginsDir.cd("plugins");
#endif
    if (!bFoundPlugins) {
        qDebug() << "plugins not found on: " << pluginsDir.absolutePath();
        qDebug() << "using alternative directory: " << alternativeDir.absolutePath();
        pluginsDir = alternativeDir;
    }
    foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
        QPluginLoader *pluginLoader = new QPluginLoader(pluginsDir.absoluteFilePath(fileName));
        QObject *plugin = pluginLoader->instance();
        if (plugin) {
            pluginLoaders.push_back(pluginLoader);
            qDebug() << "loading " << fileName;
            // check type of plugin
            CollectionInterface *iCollection = qobject_cast<CollectionInterface *>(plugin);
            if (iCollection) {
                std::vector<ClassifierInterface*> classifierList = iCollection->GetClassifiers();
                std::vector<ClustererInterface*> clustererList = iCollection->GetClusterers();
                std::vector<RegressorInterface*> regressorList = iCollection->GetRegressors();
                std::vector<DynamicalInterface*> dynamicalList = iCollection->GetDynamicals();
                std::vector<MaximizeInterface*> maximizerList = iCollection->GetMaximizers();
                std::vector<ReinforcementInterface*> reinforcementList = iCollection->GetReinforcements();
                std::vector<ProjectorInterface*> projectorList = iCollection->GetProjectors();
                FOR (i, classifierList.size()) AddPlugin(classifierList[i], SLOT(ChangeActiveOptions));
                FOR (i, clustererList.size()) AddPlugin(clustererList[i], SLOT(ChangeActiveOptions));
                FOR (i, regressorList.size()) AddPlugin(regressorList[i], SLOT(ChangeActiveOptions));
                FOR (i, dynamicalList.size()) AddPlugin(dynamicalList[i], SLOT(ChangeActiveOptions));
                FOR (i, maximizerList.size()) AddPlugin(maximizerList[i], SLOT(ChangeActiveOptions));
                FOR (i, reinforcementList.size()) AddPlugin(reinforcementList[i], SLOT(ChangeActiveOptions));
                FOR (i, projectorList.size()) AddPlugin(projectorList[i], SLOT(ChangeActiveOptions));
                continue;
            }
            ClassifierInterface *iClassifier = qobject_cast<ClassifierInterface *>(plugin);
            if (iClassifier) {
                AddPlugin(iClassifier, SLOT(ChangeActiveOptions()));
                continue;
            }
            ClustererInterface *iClusterer = qobject_cast<ClustererInterface *>(plugin);
            if (iClusterer) {
                AddPlugin(iClusterer, SLOT(ChangeActiveOptions()));
                continue;
            }
            RegressorInterface *iRegressor = qobject_cast<RegressorInterface *>(plugin);
            if (iRegressor) {
                AddPlugin(iRegressor, SLOT(ChangeActiveOptions()));
                continue;
            }
            DynamicalInterface *iDynamical = qobject_cast<DynamicalInterface *>(plugin);
            if (iDynamical) {
                AddPlugin(iDynamical, SLOT(ChangeActiveOptions()));
                continue;
            }
            MaximizeInterface *iMaximize = qobject_cast<MaximizeInterface *>(plugin);
            if (iMaximize) {
                AddPlugin(iMaximize, SLOT(ChangeActiveOptions()));
                continue;
            }
            ReinforcementInterface *iReinforcement = qobject_cast<ReinforcementInterface *>(plugin);
            if (iReinforcement) {
                AddPlugin(iReinforcement, SLOT(ChangeActiveOptions()));
                continue;
            }
            ProjectorInterface *iProject = qobject_cast<ProjectorInterface *>(plugin);
            if (iProject) {
                AddPlugin(iProject, SLOT(ChangeActiveOptions()));
                continue;
            }
            InputOutputInterface *iIO = qobject_cast<InputOutputInterface *>(plugin);
            if (iIO) {
                AddPlugin(iIO);
                continue;
            }
            AvoidanceInterface *iAvoid = qobject_cast<AvoidanceInterface *>(plugin);
            if (iAvoid) {
                AddPlugin(iAvoid, SLOT(ChangeActiveOptions()));
                continue;
            }
        } else {
            qDebug() << pluginLoader->errorString();
            delete pluginLoader;
        }
    }
    algo->SetAlgorithms(classifiers, clusterers, regressors, dynamicals, avoiders, maximizers, reinforcements, projectors, inputoutputs);
}

void PluginManager::AddPlugin(InputOutputInterface *iIO)
{
    inputoutputs.push_back(iIO);
    bInputRunning.push_back(false);
    connect(algo, SIGNAL(SendResults(std::vector<fvec>)), iIO->object(), iIO->FetchResultsSlot());
    connect(iIO->object(), iIO->SetDataSignal(), mldemos, SLOT(SetData(std::vector<fvec>, ivec, std::vector<ipair>, bool)));
    connect(iIO->object(), iIO->SetTimeseriesSignal(), mldemos, SLOT(SetTimeseries(std::vector<TimeSerie>)));
    connect(iIO->object(), iIO->QueryClassifierSignal(), algo, SLOT(QueryClassifier(std::vector<fvec>)));
    connect(iIO->object(), iIO->QueryRegressorSignal(), algo, SLOT(QueryRegressor(std::vector<fvec>)));
    connect(iIO->object(), iIO->QueryDynamicalSignal(), algo, SLOT(QueryDynamical(std::vector<fvec>)));
    connect(iIO->object(), iIO->QueryClustererSignal(), algo, SLOT(QueryClusterer(std::vector<fvec>)));
    connect(iIO->object(), iIO->QueryMaximizerSignal(), algo, SLOT(QueryMaximizer(std::vector<fvec>)));
    connect(iIO->object(), iIO->DoneSignal(), this, SLOT(DisactivateIO(QObject *)));
    QString name = iIO->GetName();
    if (mldemos->ui.menuInput_Output) {
        QAction *pluginAction = mldemos->ui.menuInput_Output->addAction(name);
        pluginAction->setCheckable(true);
        pluginAction->setChecked(false);
        connect(pluginAction,SIGNAL(toggled(bool)), this, SLOT(ActivateIO()));
    }
    if (mldemos->ui.menuImport) {
        QAction *importAction = mldemos->ui.menuImport->addAction(name);
        importAction->setCheckable(true);
        importAction->setChecked(false);
        connect(importAction,SIGNAL(toggled(bool)), this, SLOT(ActivateImport()));
    }
}

void PluginManager::AddPlugin(ClassifierInterface *iClassifier, const char *method)
{
    if (!iClassifier) return;
    // we add the interface so we can use it to produce classifiers
    classifiers.push_back(iClassifier);
    // we add the classifier parameters to the gui
    algo->optionsClassify->algoList->addItem(iClassifier->GetName());
    QWidget *widget = iClassifier->GetParameterWidget();
    widget->setParent(algo->optionsClassify->algoWidget);
    algo->optionsClassify->algoWidget->layout()->addWidget(widget);
    widget->hide();
    algo->algoWidgets["classifiers"].push_back(widget);
}

void PluginManager::AddPlugin(ClustererInterface *iCluster, const char *method)
{
    if (!iCluster) return;
    clusterers.push_back(iCluster);
    algo->optionsCluster->algoList->addItem(iCluster->GetName());
    QWidget *widget = iCluster->GetParameterWidget();
    widget->setParent(algo->optionsCluster->algoWidget);
    algo->optionsCluster->algoWidget->layout()->addWidget(widget);
    widget->hide();
    algo->algoWidgets["clusterers"].push_back(widget);
}

void PluginManager::AddPlugin(RegressorInterface *iRegress, const char *method)
{
    if (!iRegress) return;
    regressors.push_back(iRegress);
    algo->optionsRegress->algoList->addItem(iRegress->GetName());
    QWidget *widget = iRegress->GetParameterWidget();
    widget->setParent(algo->optionsRegress->algoWidget);
    algo->optionsRegress->algoWidget->layout()->addWidget(widget);
    widget->hide();
    algo->algoWidgets["regressors"].push_back(widget);
}

void PluginManager::AddPlugin(DynamicalInterface *iDynamical, const char *method)
{
    if (!iDynamical) return;
    dynamicals.push_back(iDynamical);
    algo->optionsDynamic->algoList->addItem(iDynamical->GetName());
    QWidget *widget = iDynamical->GetParameterWidget();
    widget->setParent(algo->optionsDynamic->algoWidget);
    algo->optionsDynamic->algoWidget->layout()->addWidget(widget);
    widget->hide();
    algo->algoWidgets["dynamicals"].push_back(widget);
}

void PluginManager::AddPlugin(AvoidanceInterface *iAvoid, const char *method)
{
    if (!iAvoid) return;
    avoiders.push_back(iAvoid);
    algo->optionsDynamic->obstacleCombo->addItem(iAvoid->GetName());
}

void PluginManager::AddPlugin(MaximizeInterface *iMaximizer, const char *method)
{
    if (!iMaximizer) return;
    maximizers.push_back(iMaximizer);
    algo->optionsMaximize->algoList->addItem(iMaximizer->GetName());
    QWidget *widget = iMaximizer->GetParameterWidget();
    widget->setParent(algo->optionsMaximize->algoWidget);
    algo->optionsMaximize->algoWidget->layout()->addWidget(widget);
    widget->hide();
    algo->algoWidgets["maximizers"].push_back(widget);
}

void PluginManager::AddPlugin(ReinforcementInterface *iReinforcement, const char *method)
{
    if (!iReinforcement) return;
    reinforcements.push_back(iReinforcement);
    algo->optionsReinforcement->algoList->addItem(iReinforcement->GetName());
    QWidget *widget = iReinforcement->GetParameterWidget();
    widget->setParent(algo->optionsReinforcement->algoWidget);
    algo->optionsReinforcement->algoWidget->layout()->addWidget(widget);
    widget->hide();
    algo->algoWidgets["reinforcements"].push_back(widget);
}

void PluginManager::AddPlugin(ProjectorInterface *iProject, const char *method)
{
    if (!iProject) return;
    projectors.push_back(iProject);
    algo->optionsProject->algoList->addItem(iProject->GetName());
    QWidget *widget = iProject->GetParameterWidget();
    widget->setParent(algo->optionsProject->algoWidget);
    algo->optionsProject->algoWidget->layout()->addWidget(widget);
    widget->hide();
    algo->algoWidgets["projectors"].push_back(widget);
}

void PluginManager::ActivateIO()
{
    QList<QAction *> pluginActions = mldemos->ui.menuInput_Output->actions();
    FOR (i, inputoutputs.size())
    {
        if (i<pluginActions.size() && inputoutputs[i] && pluginActions[i])
        {
            if (pluginActions[i]->isChecked())
            {
                bInputRunning[i] = true;
                inputoutputs[i]->Start();
            }
            else if (bInputRunning[i])
            {
                bInputRunning[i] = false;
                inputoutputs[i]->Stop();
            }
        }
    }
}

void PluginManager::ActivateImport()
{
    QList<QAction *> pluginActions = mldemos->ui.menuInput_Output->actions();
    FOR (i, inputoutputs.size()) {
        if (i<pluginActions.size() && inputoutputs[i] && pluginActions[i]) {
            if (pluginActions[i]->isChecked()) {
                bInputRunning[i] = true;
                inputoutputs[i]->Start();
            } else if (bInputRunning[i]) {
                bInputRunning[i] = false;
                inputoutputs[i]->Stop();
            }
        }
    }
}

void PluginManager::DisactivateIO(QObject *io)
{
    if (!io) return;
    // first we find the right plugin
    int pluginIndex = -1;
    FOR (i, inputoutputs.size()) {
        if (inputoutputs[i]->object() == io) {
            pluginIndex = i;
            break;
        }
    }
    if (pluginIndex == -1) {
        qDebug() << "Unable to unload plugin: ";
        return; // something weird is going on!
    }
    QList<QAction *> pluginActions = mldemos->ui.menuInput_Output->actions();
    if (pluginIndex < pluginActions.size() && pluginActions[pluginIndex]) {
        pluginActions[pluginIndex]->setChecked(false);
        if (bInputRunning[pluginIndex]) inputoutputs[pluginIndex]->Stop();
        bInputRunning[pluginIndex] = false;
    }
    pluginActions = mldemos->ui.menuImport->actions();
    if (pluginIndex < pluginActions.size() && pluginActions[pluginIndex]) {
        pluginActions[pluginIndex]->setChecked(false);
        if (bInputRunning[pluginIndex]) inputoutputs[pluginIndex]->Stop();
        bInputRunning[pluginIndex] = false;
    }
}
