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

void MLDemos::ChangeActiveOptions()
{
    DisplayOptionsChanged();
}

void MLDemos::DisplayOptionsChanged()
{
    if (!canvas) return;

    canvas->bDisplayInfo = displayOptions->infoCheck->isChecked();
    canvas->bDisplayLearned = displayOptions->modelCheck->isChecked();
    canvas->bDisplayMap = displayOptions->mapCheck->isChecked();
    canvas->bDisplaySamples = displayOptions->samplesCheck->isChecked();
    canvas->bDisplayTrajectories = displayOptions->samplesCheck->isChecked();
    canvas->bDisplayTimeSeries = displayOptions->samplesCheck->isChecked();
    canvas->bDisplayGrid = displayOptions->gridCheck->isChecked();
    canvas->bDisplayLegend = displayOptions->legendCheck->isChecked();

    int xIndex = ui.canvasX1Spin->value()-1;
    int yIndex = ui.canvasX2Spin->value()-1;
    int zIndex = ui.canvasX3Spin->value()-1;
    bool bCanvasIndicesChanged = canvas->SetDim(xIndex, yIndex, zIndex);
    if(bCanvasIndicesChanged) {
        if(ui.restrictDimCheck->isChecked()) {
            Clear();
        }
        canvas->FitToData();
    }

    float zoom = displayOptions->spinZoom->value();
    if (zoom >= 0.f) zoom += 1.f;
    else zoom = 1.f / (fabs(zoom)+1.f);
    if (zoom != canvas->GetZoom()) {
        drawTimer->Stop();
        drawTimer->Clear();
        drawTimer->inputDims = algo->GetInputDimensions();
        canvas->SetZoom(zoom);
        if (mutex.tryLock()) {
            if (!canvas->canvasType) {
                if (algo->classifier) {
                    algo->classifiers[algo->tabUsedForTraining]->Draw(canvas, algo->classifier);
                    algo->DrawClassifiedSamples(canvas, algo->classifier, algo->classifierMulti);
                    if (algo->classifier->UsesDrawTimer()) {
                        drawTimer->start(QThread::NormalPriority);
                    }
                } else if (algo->regressor) {
                    algo->regressors[algo->tabUsedForTraining]->Draw(canvas, algo->regressor);
                    //drawTimer->start(QThread::NormalPriority);
                } else if (algo->clusterer) {
                    algo->clusterers[algo->tabUsedForTraining]->Draw(canvas, algo->clusterer);
                } else if (algo->dynamical) {
                    algo->dynamicals[algo->tabUsedForTraining]->Draw(canvas, algo->dynamical);
                    if (algo->dynamicals[algo->tabUsedForTraining]->UsesDrawTimer()) drawTimer->start(QThread::NormalPriority);
                } else if (algo->maximizer) {
                    drawTimer->start(QThread::NormalPriority);
                } else if (algo->projector) {
                    algo->projectors[algo->tabUsedForTraining]->Draw(canvas, algo->projector);
                }
            }
            mutex.unlock();
        }
        canvas->repaint();
    }
    //	canvas->bDisplayTrajectories = displayOptions->trajectoriesCheck->isChecked();
    if (algo->optionsDynamic)
    {
        canvas->trajectoryCenterType = algo->optionsDynamic->centerCombo->currentIndex();
        canvas->trajectoryResampleType = algo->optionsDynamic->resampleCombo->currentIndex();
        canvas->trajectoryResampleCount = algo->optionsDynamic->resampleSpin->value();
    }
    CanvasTypeChanged();
    CanvasOptionsChanged();
    canvas->ResetSamples();
    canvas->repaint();
}

void MLDemos::Display3DOptionsChanged()
{
    if (canvas->canvasType == 0)
    {
        if (canvas->bDisplayGrid != displayOptions->gridCheck->isChecked())
        {
            canvas->bDisplayGrid = displayOptions->gridCheck->isChecked();
            canvas->repaint();
        }
    }
    else canvas->bDisplayGrid = displayOptions->gridCheck->isChecked();
    if (!glw) return;
    glw->bDisplaySamples = displayOptions->check3DSamples->isChecked();
    glw->bDisplayLines = displayOptions->check3DWireframe->isChecked();
    glw->bDisplaySurfaces = displayOptions->check3DSurfaces->isChecked();
    glw->bDisplayTransparency = displayOptions->check3DTransparency->isChecked();
    glw->bDisplayBlurry = displayOptions->check3DBlurry->isChecked();
    glw->bRotateCamera = displayOptions->check3DRotate->isChecked();
    glw->update();
}

void MLDemos::ChangeInfoFile()
{
    QStringList infoFiles = algo->GetInfoFiles();
    QString infoFile = infoFiles.at(0);
    QString mainFile = infoFiles.at(1);
    if (mainFile == "") mainFile = "mldemos.html"; // we want the main information page

    QDir helpDir = QDir(qApp->applicationDirPath());
    QDir alternativeDir = helpDir;
#if defined(Q_OS_WIN)
    if (helpDir.dirName().toLower() == "debug" || helpDir.dirName().toLower() == "release") helpDir.cdUp();
#elif defined(Q_OS_MAC)
    if (helpDir.dirName() == "MacOS") {
        if (!helpDir.cd("help"))
        {
            helpDir.cdUp();
            helpDir.cdUp();
            helpDir.cdUp();
            alternativeDir = helpDir;
        }
        else helpDir.cdUp();
    }
#endif
    if (!helpDir.cd("help")) {
        //qDebug() << "using alternative directory: " << alternativeDir.absolutePath();
        helpDir = alternativeDir;
        if (!helpDir.cd("help")) return;
    }
    //qDebug() << "using help directory: " << helpDir.absolutePath();

    showStats->helpAlgoText->clear();
    if (!infoFile.isEmpty()) {
        QString filePath(helpDir.absolutePath() + "/" + infoFile);
        if (QFile::exists(filePath)) {
            showStats->helpAlgoText->setSource(QUrl::fromLocalFile(filePath));
        } else {
            showStats->helpAlgoText->setText("No information available.");
        }
    }

    showStats->helpMainText->clear();
    QString filePath2(helpDir.absolutePath() + "/" + mainFile);
    if (QFile::exists(filePath2)) {
        showStats->helpMainText->setSource(QUrl::fromLocalFile(filePath2));
    } else {
        showStats->helpMainText->setText("No information available.");
    }
}

void MLDemos::AlgoChanged()
{
    if(!algo) return;

    algo->SetAlgorithmWidget();

    ChangeInfoFile();
    actionAlgorithms->setChecked(algo->algorithmWidget->isVisible());
    if (algo->options->tabMax->isVisible() ||
            algo->options->tabReinf->isVisible()) {
        drawToolbar->paintButton->setChecked(true);
    }
    if (algo->options->tabDyn->isVisible()) {
        drawToolbar->trajectoryButton->setChecked(true);
    }
    if (algo->options->tabRegr->isVisible() ||
            algo->options->tabClass->isVisible() ||
            algo->options->tabClust->isVisible() ||
            algo->options->tabProj->isVisible()) {
        if (!drawToolbar->sprayButton->isChecked() && !drawToolbar->spray3DButton->isChecked()) {
            drawToolbar->sprayButton->setChecked(true);
        }
    }
    if (!algo->options->tabClust->isVisible()) {
        algo->options->tabWidget->resize(635,193);
    }
}
