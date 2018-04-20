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

void AlgorithmManager::Clear()
{
    if (!classifierMulti.size()) DEL(classifier);
    classifier = 0;
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

    if(mldemos->ui.restrictDimCheck->isChecked()) {
        if(canvas->data->GetDimCount() == 2) return ivec();
        ivec dimList(2);
        dimList.front() = canvas->xIndex;
        dimList.back() = canvas->yIndex;
        return dimList;
    }

    QList<QListWidgetItem*> selected = inputDimensions->dimList->selectedItems();
    if(!selected.size() || selected.size() == inputDimensions->dimList->count()) return ivec(); // if nothing is selected we use all dimensions for training
    ivec dimList(selected.size());
    FOR(i, selected.size())
    {
        dimList[i] = inputDimensions->dimList->row(selected[i]);
    }
    return dimList;
}

void AlgorithmManager::UpdateLearnedModel()
{
    if(!canvas) return;
    if(!clusterer && !regressor && !dynamical && !classifier && !projector) return;
    if(glw) glw->clearLists();

    if(classifier) UpdateClassifier();
    if(clusterer) UpdateClusterer();
    if(regressor) UpdateRegressor();
    if(dynamical) UpdateDynamical();
    if(projector) UpdateProjector();

    emit UpdateInfo();
}

void AlgorithmManager::UpdateClassifier()
{
    QMutexLocker lock(mutex);
    if(glw->isVisible()) {
        glw->clearLists();
        if(canvas->canvasType == 1) {
            classifiers[tabUsedForTraining]->DrawGL(canvas, glw, classifier);
            if(canvas->data->GetDimCount() == 3 && (sourceDims.size()==0 || sourceDims.size()==3)) Draw3DClassifier(glw, classifier);
        }
    } else {
        classifiers[tabUsedForTraining]->Draw(canvas, classifier);
        DrawClassifiedSamples(canvas, classifier, classifierMulti);
        if(classifier->UsesDrawTimer() && !drawTimer->isRunning()) {
            drawTimer->inputDims = GetInputDimensions();
            drawTimer->start(QThread::NormalPriority);
        }
    }
}

void AlgorithmManager::UpdateClusterer()
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

void AlgorithmManager::UpdateDynamical()
{
    QMutexLocker lock(mutex);
    if(glw->isVisible()) {
        glw->clearLists();
        if(canvas->canvasType == 1) {
            dynamicals[tabUsedForTraining]->DrawGL(canvas, glw, dynamical);
            if(canvas->data->GetDimCount() == 3) {
                int displayStyle = optionsDynamic->displayCombo->currentIndex();
                if(displayStyle < 3) Draw3DDynamical(glw, dynamical,displayStyle);
            }
        }
    } else {
        dynamicals[tabUsedForTraining]->Draw(canvas, dynamical);
        bool bColorMap = optionsDynamic->colorCheck->isChecked();

        // the first index is "none", so we subtract 1
        int avoidIndex = optionsDynamic->obstacleCombo->currentIndex()-1;
        if(avoidIndex >=0 && avoidIndex < avoiders.size() && avoiders[avoidIndex]) {
            DEL(dynamical->avoid);
            dynamical->avoid = avoiders[avoidIndex]->GetObstacleAvoidance();
        }
        emit UpdateInfo();

        if(dynamicals[tabUsedForTraining]->UsesDrawTimer()) {
            drawTimer->bColorMap = bColorMap;
            drawTimer->start(QThread::NormalPriority);
        }
    }
}

void AlgorithmManager::UpdateRegressor()
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
    if(mldemos->ui.restrictDimCheck->isChecked()) outputDim = inputDims.back();
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

void AlgorithmManager::UpdateProjector()
{
    if(canvas->canvasType != 0) return;
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

