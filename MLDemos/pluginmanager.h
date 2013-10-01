#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <public.h>
#include <algorithmmanager.h>

class MLDemos;

class PluginManager : public QObject
{
    Q_OBJECT
public:
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

    //QMenu *menuInput_Output, *menuImport;

    QList<QPluginLoader*> pluginLoaders;
    AlgorithmManager *algo;
    MLDemos *mldemos;

public:
    PluginManager(MLDemos *mldemos, AlgorithmManager *algo);
    ~PluginManager();

    void LoadPlugins();
    void AddPlugin(ClassifierInterface *iClassifier, const char *method);
    void AddPlugin(ClustererInterface *iCluster, const char *method);
    void AddPlugin(RegressorInterface *iRegress, const char *method);
    void AddPlugin(DynamicalInterface *iDynamical, const char *method);
    void AddPlugin(AvoidanceInterface *iAvoid, const char *method);
    void AddPlugin(MaximizeInterface *iMaximize, const char *method);
    void AddPlugin(ReinforcementInterface *iReinforcement, const char *method);
    void AddPlugin(ProjectorInterface *iProject, const char *method);
    void AddPlugin(InputOutputInterface *iIO);

public slots:
    // input/output plugins
    void ActivateIO();
    void ActivateImport();
    void DisactivateIO(QObject *);
};

#endif // PLUGINMANAGER_H
