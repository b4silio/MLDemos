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
#include <public.h>
#include <interfaces.h>
#include <classifier.h>
#include <regressor.h>
#include <datasetManager.h>

#include <QtGui/QApplication>
#include <QtPlugin>

using namespace std;

map<QString,ClassifierInterface*> classifierInterfaces;
vector<QPluginLoader*> pluginLoaders;
void LoadPlugins();

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

    // we start by loading all plugins
    LoadPlugins();

    if(argc == 1) // we just list the algorithms
    {
        printf("Usage: .... \n");

        printf("Algorithms \n");
        printf("---------- \n");
        printf("name \t:\t[param1] [param2] ... \n");
        printf("\n");
        for (map<QString,ClassifierInterface*>::iterator it = classifierInterfaces.begin(); it != classifierInterfaces.end(); it++)
        {
            printf("%s\t:\t", it->first.toLatin1().data());
            std::vector<QString> pNames;
            std::vector<QString> pTypes;
            std::vector< std::vector<QString> > pValues;
            it->second->GetParameterList(pNames, pTypes, pValues);
            FOR(i, pNames.size()) printf("[%s] ", pNames[i].toLatin1().data());
            printf("\n");
        }
        fflush(stdout);
        //return -1;
    }

    // we get the datasets filenames
    QString filename1, filename2;


    // we get the data (training and testing)
    DatasetManager dataset1, dataset2;
    dataset1.Load(filename1.toLatin1().data());
    dataset2.Load(filename2.toLatin1().data());
    vector<fvec> trainSamples = dataset1.GetSamples();
    ivec trainLabels = dataset1.GetLabels();
    vector<fvec> testSamples = dataset2.GetSamples();
    ivec testLabels = dataset2.GetLabels();

    if(!trainSamples.size() && !testSamples.size())
    {
        qDebug() << "no samples to train or test!";
        return -1;
    }

    // toy example!
    QString name = "GMM";
    int param1 = 4; // 4 gaussiennes

    // we get the algorithm and the parameters
    if(!classifierInterfaces.count(name))
    {
        qDebug() << "cannot find algorithm, sorry.";
        return -1;
    }
    ClassifierInterface *interface = classifierInterfaces[name];
    Classifier *classifier = interface->GetClassifier();

    fvec parameters;
    parameters.push_back(param1);
    interface->SetParams(classifier, parameters);

    // we do the training
    classifier->Train(trainSamples, trainLabels);

    int errors=0;
    FOR(i, testSamples.size())
    {
        int label = testLabels[i];
        if(classifier->IsMultiClass())
        {
            fvec res = classifier->TestMulti(testSamples[i]);
            if(res.size() == 1) // it's actually a binary classification
            {
                if (res[0] > 0)
                {
                    if (label < 1) errors++;
                }
                else
                {
                    if (label > 0) errors++;
                }
            }
            else
            {
                int maxClass = 0;
                FOR(j, res.size()) if (res[maxClass] < res[j]) maxClass = j;
                if (maxClass != label) errors++;
            }
        } else {
            float res = classifier->Test(testSamples[i]);
            if (res > 0)
            {
                if (label < 1) errors++;
            }
            else
            {
                if (label > 0) errors++;
            }
        }
    }
    printf("error: %.2f%", errors / (float)testSamples.size()*100);
    fflush(stdout);

}

void LoadPlugins()
{
    //qDebug() << "Importing plugins";
    QDir pluginsDir = QDir(qApp->applicationDirPath());
    QDir alternativeDir = pluginsDir;

#if defined(Q_OS_WIN)
    if (pluginsDir.dirName().toLower() == "debug" || pluginsDir.dirName().toLower() == "release") pluginsDir.cdUp();
#elif defined(Q_OS_MAC)
    if (pluginsDir.dirName() == "MacOS") {
        if (!pluginsDir.cd("plugins")) {
            //qDebug() << "looking for alternative directory";
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
    //qDebug() << "looking for debug plugins";
    bFoundPlugins = pluginsDir.cd("pluginsDebug");
#else
    //qDebug() << "looking for release plugins";
    bFoundPlugins = pluginsDir.cd("plugins");
#endif
    if (!bFoundPlugins) {
        //qDebug() << "plugins not found on: " << pluginsDir.absolutePath();
        //qDebug() << "using alternative directory: " << alternativeDir.absolutePath();
        pluginsDir = alternativeDir;
    }
    foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
        QPluginLoader *pluginLoader = new QPluginLoader(pluginsDir.absoluteFilePath(fileName));
        QObject *plugin = pluginLoader->instance();
        if (plugin) {
            pluginLoaders.push_back(pluginLoader);
            //qDebug() << "loading " << fileName;
            // check type of plugin
            CollectionInterface *iCollection = qobject_cast<CollectionInterface *>(plugin);
            if (iCollection) {
                std::vector<ClassifierInterface*> classifierList = iCollection->GetClassifiers();
                FOR (i, classifierList.size())
                {
                    QString name = classifierList[i]->GetAlgoString();
                    name = name.split(" ").at(0);
                    classifierInterfaces[name] = classifierList[i];
                }
                continue;
            }
            ClassifierInterface *iClassifier = qobject_cast<ClassifierInterface *>(plugin);
            if (iClassifier) {
                QString name = iClassifier->GetAlgoString();
                name = name.split(" ").at(0);
                classifierInterfaces[name] = iClassifier;
                continue;
            }
        } else {
            qDebug() << pluginLoader->errorString();
            delete pluginLoader;
        }
    }
}
