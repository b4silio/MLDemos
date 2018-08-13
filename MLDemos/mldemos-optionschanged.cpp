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
    canvas->bDisplayInfo = displayOptions->showModel->isChecked();
    canvas->bDisplayLearned = displayOptions->showOutput->isChecked();
    canvas->bDisplayMap = displayOptions->showBackground->isChecked();
    canvas->bDisplaySamples = displayOptions->showSamples->isChecked();
    canvas->bDisplayTrajectories = displayOptions->showSamples->isChecked();
    canvas->bDisplayTimeSeries = displayOptions->showSamples->isChecked();
    canvas->bDisplayGrid = displayOptions->showGrid->isChecked();
    canvas->bDisplayLegend = displayOptions->showLegend->isChecked();

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

    float zoom = viewOptions->spinZoom->value();
    // we want to find a smooth function to make zooming more "linear" when zooming further
    if (zoom >= 0.f) {
        //zoom += 1.f;
        zoom = powf(1.5,zoom);
    } else {
        zoom = 1.f / (powf(1.5,fabs(zoom)));
    }

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
        if (canvas->bDisplayGrid != displayOptions->showGrid->isChecked())
        {
            canvas->bDisplayGrid = displayOptions->showGrid->isChecked();
            canvas->repaint();
        }
    }
    else canvas->bDisplayGrid = displayOptions->showGrid->isChecked();
    if (!glw) return;
    glw->bDisplaySamples = viewOptions->check3DSamples->isChecked();
    glw->bDisplayLines = viewOptions->check3DWireframe->isChecked();
    glw->bDisplaySurfaces = viewOptions->check3DSurfaces->isChecked();
    glw->bDisplayTransparency = viewOptions->check3DTransparency->isChecked();
    glw->bDisplayBlurry = viewOptions->check3DBlurry->isChecked();
    glw->bRotateCamera = viewOptions->check3DRotate->isChecked();
    glw->update();
}

void MLDemos::DrawToolsChanged()
{
    QLabel* rLabel = drawToolbar->radiusLabel;
    QDoubleSpinBox * rSpin = drawToolbar->radiusSpin;
    QLabel* cLabel = drawToolbar->classLabel;
    QSpinBox * cSpin = drawToolbar->classSpin;

    if     (drawToolbar->singleButton->isChecked() ||
            drawToolbar->trajectoryButton->isChecked()) {
        rLabel->hide();
        rSpin->hide();
        cLabel->show();
        cSpin->show();
    }
    if     (drawToolbar->sprayButton->isChecked() ||
            drawToolbar->spray3DButton->isChecked() ||
            drawToolbar->sprayClassButton->isChecked() ||
            drawToolbar->paintButton->isChecked())  {
        rLabel->show();
        rSpin->show();
        cLabel->show();
        cSpin->show();
    }
    if(drawToolbar->eraseButton->isChecked()){
        rLabel->show();
        rSpin->show();
        cLabel->hide();
        cSpin->hide();
    }
    if     (drawToolbar->obstacleButton->isChecked() ||
            drawToolbar->dragButton->isChecked() ||
            drawToolbar->moveButton->isChecked() ||
            drawToolbar->moveClassButton->isChecked() ||
            drawToolbar->lineButton->isChecked() ||
            drawToolbar->ellipseButton->isChecked() ||
            drawToolbar->extrudeButton->isChecked() ||
            drawToolbar->dimPlusButton->isChecked() ||
            drawToolbar->dimLessButton->isChecked()) {
        rLabel->hide();
        rSpin->hide();
        cLabel->hide();
        cSpin->hide();
    }
    drawContext1Widget->hide();
    drawContext2Widget->hide();
    drawContext3Widget->hide();
    drawContext4Widget->hide();

    if(drawToolbar->sprayButton->isChecked()) drawContext1Widget->show();
    if(drawToolbar->ellipseButton->isChecked()) drawContext2Widget->show();
    if(drawToolbar->lineButton->isChecked()) drawContext2Widget->show();
    if(drawToolbar->eraseButton->isChecked()) drawContext1Widget->show();
    if(drawToolbar->obstacleButton->isChecked()) drawContext3Widget->show();
    if(drawToolbar->paintButton->isChecked()) drawContext4Widget->show();
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    if (rLabel->isHidden() && cLabel->isHidden()
            && drawContext1Widget->isHidden()
            && drawContext2Widget->isHidden()
            && drawContext3Widget->isHidden()
            && drawContext4Widget->isHidden()) {
        drawToolbar->toolOptionWidget->hide();
        ui.leftPaneWidget->setMinimumWidth(70);
    } else {
        drawToolbar->toolOptionWidget->show();
        ui.leftPaneWidget->setMinimumWidth(68 + 82);
    }
    ResetMinimumWidth();
}

void MLDemos::ResetMinimumWidth()
{
    int minW = 520;
    if(ui.leftPaneWidget->isVisible()) minW += ui.leftPaneWidget->width();
    if(ui.rightPaneWidget->isVisible()) minW += 310;
    setMinimumWidth(minW);
    repaint();
}


void MLDemos::ClearPluginSelectionText()
{
    setWindowTitle("MLDemos");
}

void MLDemos::AddPluginSelectionText(QString text)
{
    QString currentTitle = windowTitle();
    QString newTitle;
    if(currentTitle == "MLDemos") newTitle = "";
    else {
        newTitle = currentTitle.right(currentTitle.length() - QString("MLDemos [").length());
        newTitle.chop(QString("]").length());
    }

    newTitle = "MLDemos [" + (newTitle.isEmpty() ? "" : newTitle + "|") + text + "]";
    setWindowTitle(newTitle);
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
    if(algo->options->tabClass->isVisible()) ui.algoTypeLabel->setText("C L A S S I F I C A T I O N");
    if(algo->options->tabClust->isVisible()) ui.algoTypeLabel->setText("C L U S T E R I N G");
    if(algo->options->tabRegr->isVisible()) ui.algoTypeLabel->setText("R E G R E S S I O N");
    if(algo->options->tabProj->isVisible()) ui.algoTypeLabel->setText("P R O J E C T I O N");
    if(algo->options->tabDyn->isVisible()) ui.algoTypeLabel->setText("D Y N A M I C A L   S Y S T E M S");
    if(algo->options->tabMax->isVisible()) ui.algoTypeLabel->setText("M A X I M I Z A T I O N");
    if(algo->options->tabReinf->isVisible()) ui.algoTypeLabel->setText("R E I N F O R C E M E N T   L E A R N I N G");
}
