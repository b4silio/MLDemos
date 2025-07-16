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
#include <iostream>
#include "algorithmmanager.h"

using namespace std;

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
