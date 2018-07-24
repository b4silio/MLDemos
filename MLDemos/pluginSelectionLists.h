#ifndef PLUGINSELECTIONLISTS_H
#define PLUGINSELECTIONLISTS_H

#include <QStringList>
#include <map>

static std::map<QString, QStringList> PLUGIN_SELECTION_LISTS;

void InitPluginSelectionLists()
{
    PLUGIN_SELECTION_LISTS.clear();

    QStringList AMLClass;
    AMLClass << "classifier:Gaussian Mixture Model"
    << "classifier:Support Vector Machine"
    << "classifier:Relevance Vector Machine"
    << "classifier:K-Nearest Neighbours"
    << "classifier:Boosting"
    << "classifier:Multi-Layer Perceptron"
    << "classifier:Linear Projections"
    << "clusterer:DBSCAN"
    << "clusterer:Gaussian Mixture Model"
    << "clusterer:K-Means"
    << "regressor:Gaussian Mixture Regression"
    << "regressor:Gaussian Process Regression"
    << "regressor:Support Vector Regression"
    << "regressor:Relevance Vector Regression"
    << "regressor:K-Nearest Neighbours"
    << "projector:Independent Component Analysis"
    << "projector:Principal Component Analysis"
    << "projector:Linear Discriminant Analysis"
    << "projector:Kernel PCA"
    << "IO:PCA Faces";

    PLUGIN_SELECTION_LISTS["AML Class"] = AMLClass;
}

#endif // PLUGINSELECTIONLISTS_H
