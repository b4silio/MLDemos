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
