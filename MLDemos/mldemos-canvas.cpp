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
#include "mldemos.h"
#include "optimization_test_functions.h"

void MLDemos::FitToData()
{
    canvas->FitToData();
    float zoom = canvas->GetZoom();
    if (zoom >= 1) zoom -= 1;
    else zoom = 1/(-zoom) - 1;
    if (zoom == displayOptions->spinZoom->value()) {
        DisplayOptionsChanged();
        return;
    }
    displayOptions->spinZoom->blockSignals(true);
    displayOptions->spinZoom->setValue(zoom);
    displayOptions->spinZoom->blockSignals(false);
    drawTimer->Stop();
    drawTimer->Clear();
    drawTimer->inputDims = algo->GetInputDimensions();
    if (!canvas->canvasType) {
        QMutexLocker lock(&mutex);
        if (algo->classifier) {
            algo->classifiers[algo->tabUsedForTraining]->Draw(canvas, algo->classifier);
            algo->DrawClassifiedSamples(canvas, algo->classifier, algo->classifierMulti);
            if (algo->classifier->UsesDrawTimer()) drawTimer->start(QThread::NormalPriority);
        } else if (algo->regressor) {
            algo->regressors[algo->tabUsedForTraining]->Draw(canvas, algo->regressor);
        } else if (algo->clusterer) {
            algo->clusterers[algo->tabUsedForTraining]->Draw(canvas, algo->clusterer);
            drawTimer->start(QThread::NormalPriority);
        } else if (algo->dynamical) {
            algo->dynamicals[algo->tabUsedForTraining]->Draw(canvas, algo->dynamical);
            if (algo->dynamicals[algo->tabUsedForTraining]->UsesDrawTimer()) drawTimer->start(QThread::NormalPriority);
        } else if (algo->projector) {
            algo->projectors[algo->tabUsedForTraining]->Draw(canvas, algo->projector);
        }
    }
    DisplayOptionsChanged();
}

void MLDemos::ZoomChanged(float d)
{
    displayOptions->spinZoom->setValue(displayOptions->spinZoom->value()+d/4);
}

void MLDemos::CanvasMoveEvent()
{
    if (canvas->canvasType) return;
    drawTimer->Stop();
    drawTimer->Clear();
    algo->UpdateLearnedModel();
    drawTimer->inputDims = algo->GetInputDimensions();
    QMutexLocker lock(&mutex);
    if (canvas->canvasType != 1) {
        if (algo->classifier) {
            algo->classifiers[algo->tabUsedForTraining]->Draw(canvas, algo->classifier);
            algo->DrawClassifiedSamples(canvas, algo->classifier, algo->classifierMulti);
            if (algo->classifier->UsesDrawTimer()) {
                drawTimer->start(QThread::NormalPriority);
            }
        } else if (algo->regressor) {
            algo->regressors[algo->tabUsedForTraining]->Draw(canvas, algo->regressor);
        } else if (algo->clusterer) {
            algo->clusterers[algo->tabUsedForTraining]->Draw(canvas, algo->clusterer);
            drawTimer->start(QThread::NormalPriority);
        } else if (algo->dynamical) {
            algo->dynamicals[algo->tabUsedForTraining]->Draw(canvas, algo->dynamical);
            if (algo->dynamicals[algo->tabUsedForTraining]->UsesDrawTimer()) drawTimer->start(QThread::NormalPriority);
        } else if (algo->projector) {
            algo->projectors[algo->tabUsedForTraining]->Draw(canvas, algo->projector);
        }
    }
    canvas->repaint();
}

void MLDemos::CanvasTypeChanged()
{
    bool bProjected = canvas->data->bProjected;
    int type = ui.canvasTypeCombo->currentIndex();

    if (!canvas->data->GetCount()) {
        if (type > 1 && !canvas->rewardPixmap().isNull()) { // we only have rewards
            ui.canvasTypeCombo->setCurrentIndex(0);
            return;
        }
    }

    ui.canvasAxesWidget->hide();
    switch (type)
    {
    case 0: // standard
        ui.canvasAxesWidget->show();
        ui.canvasX3Spin->setEnabled(false);
        ui.canvasX1Label->setText(bProjected ? "e1" : "x1");
        ui.canvasX2Label->setText(bProjected ? "e2" : "x2");
        break;
    case 1: // 3D viewport
        {
            ui.canvasAxesWidget->show();
            ui.canvasX3Spin->setEnabled(true);
            ui.canvasX1Label->setText(bProjected ? "e1" : "x1");
            ui.canvasX2Label->setText(bProjected ? "e2" : "x2");
            ui.canvasX3Label->setText(bProjected ? "e3" : "x3");
            // if we haven't really set the third dimension, we pick the one manually
            if (canvas->data->GetDimCount() > 2 &&
                    ui.canvasX3Spin->value() == 0 ||
                    ui.canvasX3Spin->value() == ui.canvasX2Spin->value()) {
                ui.canvasX3Spin->setValue(ui.canvasX2Spin->value() == canvas->data->GetDimCount() ?
                                              canvas->data->GetDimCount()-1 : ui.canvasX2Spin->value()+1);
            }
            if (algo->regressor && canvas->data->GetDimCount() > 2 &&
                    ui.canvasX2Spin->value() == algo->optionsRegress->outputDimCombo->currentIndex()+1) {
                ui.canvasX2Spin->setValue(ui.canvasX1Spin->value()+1);
                ui.canvasX3Spin->setValue(ui.canvasX2Spin->value()+1);
            }
            if (canvas->data->GetDimCount() <= 2) ui.canvasX3Spin->setValue(0);
        }
        break;
    case 2: // Visualizations
        break;
    }
    if ((!glw || !glw->isVisible()) && (!vis || !vis->isVisible()) && canvas->canvasType == type) return;
    if (type == 1) { // 3D viewport
        vis->hide();
        displayOptions->tabWidget->setCurrentIndex(1);
        canvas->Clear();
        canvas->repaint();
        ui.canvasWidget->repaint();
        //        ui.canvasWidget->hide();
        if (!ui.canvasArea->layout()) {
            // this is an ugly hack that forces the layout behind to be drawn as a cleared image
            // for some reason otherwise, the canvas will only be repainted AFTER it is re-shown
            // and it will flicker with the last image shown beforehand
            QBoxLayout *layout = new QBoxLayout(QBoxLayout::LeftToRight);
            layout->setContentsMargins(1,1,1,1);
            layout->setMargin(1);
            ui.canvasArea->setLayout(layout);
            ui.canvasArea->layout()->addWidget(glw);
            ui.canvasArea->layout()->addWidget(vis);
        }
        QSizePolicy policy = glw->sizePolicy();
        policy.setHorizontalPolicy(QSizePolicy::Preferred);
        policy.setVerticalPolicy(QSizePolicy::Preferred);
        canvas->SetCanvasType(type);
        glw->setSizePolicy(policy);
        glw->setMinimumSize(ui.canvasArea->size());
        glw->resize(ui.canvasArea->size());
        FOR (i, glw->objects.size()) {
            if (!glw->objectAlive[i]) continue;
            if (glw->objects[i].objectType.contains("Reward")) glw->killList.push_back(i);
        }
        glw->show();
        glw->repaint();
    } else if (type==2) { // visualizations
        glw->hide();
        displayOptions->tabWidget->setCurrentIndex(0);
        canvas->Clear();
        canvas->repaint();
        ui.canvasWidget->repaint();
        //        ui.canvasWidget->hide();
        if (!ui.canvasArea->layout()) {
            // this is an ugly hack that forces the layout behind to be drawn as a cleared image
            // for some reason otherwise, the canvas will only be repainted AFTER it is re-shown
            // and it will flicker with the last image shown beforehand
            QBoxLayout *layout = new QBoxLayout(QBoxLayout::LeftToRight);
            layout->setContentsMargins(1,1,1,1);
            layout->setMargin(1);
            ui.canvasArea->setLayout(layout);
            ui.canvasArea->layout()->addWidget(glw);
            ui.canvasArea->layout()->addWidget(vis);
        }
        QSizePolicy policy = vis->sizePolicy();
        policy.setHorizontalPolicy(QSizePolicy::Preferred);
        policy.setVerticalPolicy(QSizePolicy::Preferred);
        canvas->SetCanvasType(type);
        vis->setSizePolicy(policy);
        vis->setMinimumSize(ui.canvasArea->size());
        vis->resize(ui.canvasArea->size());
        vis->show();
        vis->Update();
    } else {
        displayOptions->tabWidget->setCurrentIndex(0);
        canvas->SetCanvasType(type);
        CanvasOptionsChanged();
        canvas->ResetSamples();
        glw->hide();
        vis->hide();
        ui.canvasWidget->show();
        ui.canvasWidget->repaint();
    }
    algo->UpdateLearnedModel();
    canvas->repaint();
}

void MLDemos::CanvasOptionsChanged()
{
    QSizePolicy policy = ui.canvasWidget->sizePolicy();
    int dims = canvas ? (canvas->data->GetCount() ? canvas->data->GetSample(0).size() : 2) : 2;
    int w = ui.canvasArea->width();
    int h = ui.canvasArea->height();
    bool bNeedsZoom = false;
    if (canvas->canvasType == 1) {
        if (h/dims < 100) {
            h = 100*dims;
            bNeedsZoom = true;
        }
        if (w/dims < 100) {
            w = 100*dims;
            bNeedsZoom = true;
        }
    }

    if (canvas->canvasType == 0 || !bNeedsZoom) {
        policy.setHorizontalPolicy(QSizePolicy::Preferred);
        policy.setVerticalPolicy(QSizePolicy::Preferred);
        ui.canvasWidget->setSizePolicy(policy);
        ui.canvasWidget->setMinimumSize(ui.canvasArea->size());
        ui.canvasWidget->resize(ui.canvasArea->size());
        canvas->resize(ui.canvasWidget->size());
        ui.canvasArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        ui.canvasArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        canvas->SetCanvasType(canvas->canvasType);
        canvas->ResizeEvent();
        if (mutex.tryLock()) {
            if (algo->clusterer) {
                algo->clusterers[algo->tabUsedForTraining]->Draw(canvas, algo->clusterer);
                drawTimer->inputDims = algo->GetInputDimensions();
                drawTimer->start(QThread::NormalPriority);
            } else if (algo->dynamical) {
                algo->dynamicals[algo->tabUsedForTraining]->Draw(canvas, algo->dynamical);
                drawTimer->start(QThread::NormalPriority);
            } else if (algo->projector) {
                algo->projectors[algo->tabUsedForTraining]->Draw(canvas, algo->projector);
            }
            mutex.unlock();
        }
        return;
    }
    policy.setHorizontalPolicy(QSizePolicy::Fixed);
    policy.setVerticalPolicy(QSizePolicy::Fixed);
    ui.canvasWidget->setSizePolicy(policy);
    ui.canvasWidget->setMinimumSize(w,h);
    ui.canvasWidget->resize(w,h);
    canvas->resize(ui.canvasWidget->size());
    ui.canvasArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui.canvasArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    canvas->SetCanvasType(canvas->canvasType);

    if (mutex.tryLock()) {
        if (algo->classifier) {
            algo->classifiers[algo->tabUsedForTraining]->Draw(canvas, algo->classifier);
            algo->DrawClassifiedSamples(canvas, algo->classifier, algo->classifierMulti);
        } else if (algo->regressor) {
            algo->regressors[algo->tabUsedForTraining]->Draw(canvas, algo->regressor);
        } else if (algo->dynamical) {
            algo->dynamicals[algo->tabUsedForTraining]->Draw(canvas, algo->dynamical);
            drawTimer->start(QThread::NormalPriority);
        } else if (algo->maximizer) {
        } else if (algo->clusterer) {
            algo->clusterers[algo->tabUsedForTraining]->Draw(canvas, algo->clusterer);
            drawTimer->inputDims = algo->GetInputDimensions();
            drawTimer->start(QThread::NormalPriority);
        } else if (algo->projector) {
            algo->projectors[algo->tabUsedForTraining]->Draw(canvas, algo->projector);
        }
        mutex.unlock();
    }
    canvas->ResizeEvent();
}

void MLDemos::Navigation(fvec sample)
{
    if (sample[0]==-1) {
        ZoomChanged(sample[1]);
        return;
    }
    if (!mutex.tryLock(20)) return;
    QString information;
    char string[255];
    int count = canvas->data->GetCount();
    int pcount = 0, ncount = 0;
    ivec labels = canvas->data->GetLabels();
    FOR (i, labels.size()) {
        if (labels[i] == 0) ++pcount;
        else ++ncount;
    }
    sprintf(string, "samples: %d (o:%.3d|x:%.3d)", count, pcount, ncount);
    information += QString(string);
    sprintf(string, " | x%d: %.3f x%d: %.3f", canvas->xIndex+1, sample[canvas->xIndex], canvas->yIndex+1, sample[canvas->yIndex]);
    information += QString(string);
    if (algo->classifier) {
        float score;
        if (algo->sourceDims.size()) {
            fvec newSample(algo->sourceDims.size());
            FOR (d, algo->sourceDims.size()) newSample[d] = sample[algo->sourceDims[d]];
            sample = newSample;
        }
        if (algo->classifier->IsMultiClass()) {
            fvec res = algo->classifier->TestMulti(sample);
            int max = 0;
            FOR (i, res.size()) if (res[max] < res[i]) max = i;
            score = algo->classifier->inverseMap[max];
        } else {
            score = algo->classifier->Test(sample);
        }
        drawTimer->bPaused = false;
        sprintf(string, " | value: %.4f", score);
        information += QString(string);
    } else if (algo->dynamical) {
        // we build the trajectory(by hand)
        int count = 1000;
        std::vector<fvec> trajectory;
        fvec position = sample;
        if (algo->dynamical->avoid) algo->dynamical->avoid->SetObstacles(canvas->data->GetObstacles());
        FOR (i, count) {
            trajectory.push_back(position);
            fvec velocity = algo->dynamical->Test(position);
            if (algo->dynamical->avoid) {
                fvec newVelocity = algo->dynamical->avoid->Avoid(position, velocity);
                velocity = newVelocity;
            }
            position += velocity*algo->dynamical->dT;
            if (velocity == 0) break;
        }
        canvas->liveTrajectory = trajectory;
        canvas->repaint();
    }
    mutex.unlock();
    ui.statusBar->showMessage(information);
}

void MLDemos::BenchmarkButton()
{
    int w = canvas->width(), h = canvas->height();
    int type = algo->optionsMaximize->benchmarkCombo->currentIndex();
    QImage image(w, h, QImage::Format_ARGB32);
    image.fill(qRgb(255,255,255));

    int dim = 2;
    float minSpace = 0.f;
    float maxSpace = 1.f;
    float minVal = FLT_MAX;
    float maxVal = -FLT_MAX;
    switch (type)
    {
    case 0: // griewangk
        minSpace = -60.f;
        maxSpace = 60.f;
        minVal = 0;
        maxVal = 2;
        break;
    case 1: // rastragin
        minSpace = -5.12f;
        maxSpace = 5.12f;
        minVal = 0;
        maxVal = 82;
        break;
    case 2: // schwefel
        minSpace = -500.f;
        maxSpace = 500.f;
        minVal = -838;
        maxVal = 838;
        break;
    case 3: // ackley
        minSpace = -2.f;
        maxSpace = 2.f;
        minVal = 0;
        maxVal = 2.3504;
    case 4: // michalewicz
        minSpace = -2;
        maxSpace = 2;
        minVal = -1.03159;
        maxVal = 5.74;
        //		minVal = -1.03159;
        //		maxVal = 55.74;
    }

    bool bSetValues = minVal == FLT_MAX;
    Eigen::VectorXd x(2);
    fVec point;
    float value = 0;
    FOR (i, w) {
        x[0] = i/(float)w*(maxSpace - minSpace) + minSpace;
        FOR (j, h) {
            x[1] = j/(float)h*(maxSpace - minSpace) + minSpace;
            switch (type)
            {
            case 0:
                value = griewangk(x)(0);
                break;
            case 1:
                value = rastragin(x)(0);
                break;
            case 2:
                value = schwefel(x)(0);
                break;
            case 3:
                value = ackley(x)(0);
                break;
            case 4:
                value = sixhump(x)(0);
                break;
            }
            if (bSetValues) {
                if (value < minVal) minVal = value;
                if (value > maxVal) maxVal = value;
            } else {
                value = (value-minVal)/(maxVal-minVal);
            }
            int color = 255.f*max(0.f,min(1.f,value));
            image.setPixel(i,j,qRgba(255, color, color, 255));
        }
    }
    canvas->maps.reward = QPixmap::fromImage(image);
    canvas->repaint();
}
