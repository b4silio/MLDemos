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
#include "drawSVG.h"

void MLDemos::SaveData()
{
    if (!canvas) return;
    QString filename = QFileDialog::getSaveFileName(this, tr("Save Data"), "", tr("ML Files (*.ml)"));
    if (filename.isEmpty()) return;
    if (!filename.endsWith(".ml")) filename += ".ml";
    Save(filename);
}
void MLDemos::Save(QString filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        ui.statusBar->showMessage("WARNING: Unable to save file");
        return;
    }
    file.close();
    if (!canvas->maps.reward.isNull()) RewardFromMap(canvas->maps.reward.toImage());
    canvas->data->Save(filename.toLatin1());
    SaveParams(filename);
    ui.statusBar->showMessage("Data saved successfully");
}

void MLDemos::LoadData()
{
    if (!canvas) return;
    QString filename = QFileDialog::getOpenFileName(this, tr("Load Data"), "", tr("ML Files (*.ml)"));
    if (filename.isEmpty()) return;
    if (!filename.endsWith(".ml")) filename += ".ml";
    Load(filename);
}

void MLDemos::Load(QString filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        ui.statusBar->showMessage("WARNING: Unable to open file");
        return;
    }
    file.close();
    ClearData();
    canvas->data->Load(filename.toLatin1());
    MapFromReward();
    LoadParams(filename);
    //    QImage reward(filename + "-reward.png");
    //    if (!reward.isNull()) canvas->maps.reward = QPixmap::fromImage(reward);
    ui.statusBar->showMessage("Data loaded successfully");
    ResetPositiveClass();
    algo->optionsRegress->outputDimCombo->setCurrentIndex(algo->optionsRegress->outputDimCombo->count()-1);
    UpdateInfo();
    canvas->repaint();
}

void MLDemos::ImportData()
{
    if (!canvas || !import) return;
    QString filename = QFileDialog::getOpenFileName(this, tr("Import Data Data"), "", tr("Dataset Files (*.csv *.data *.txt)"));
    if (filename.isEmpty()) return;
    import->Start();
    import->Parse(filename);
    import->SendData();
}

void MLDemos::ImportData(QString filename)
{
    import->Start();
    import->Parse(filename);
    import->SendData();
    if (import->GetHeaders().size()) canvas->dimNames = import->GetHeaders();
    ResetPositiveClass();
    ui.statusBar->showMessage("Data loaded successfully");
}

void MLDemos::dragEnterEvent(QDragEnterEvent *event)
{
    QList<QUrl> dragUrl;
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls = event->mimeData()->urls();
        QStringList dataType;
        dataType << ".ml" << ".csv" << ".data";
        for (int i=0; i<urls.size(); i++) {
            QString filename = urls[i].path();
            for (int j=0; j < dataType.size(); j++) {
                if (filename.toLower().endsWith(dataType[j])) {
                    dragUrl.push_back(urls[i]);
                    break;
                }
            }
        }
        if (dragUrl.size()) {
            event->acceptProposedAction();
        }
    }
}

void MLDemos::dropEvent(QDropEvent *event)
{
    if (!event->mimeData()->hasUrls()) return;
    FOR (i, event->mimeData()->urls().length())
    {
        QString filename = event->mimeData()->urls()[i].toLocalFile();
        qDebug() << "accepted drop file:" << filename;
        if (filename.toLower().endsWith(".ml"))
        {
            ClearData();
            canvas->data->Load(filename.toLatin1());
            MapFromReward();
            LoadParams(filename);
            ui.statusBar->showMessage("Data loaded successfully");
            ResetPositiveClass();
            UpdateInfo();
            canvas->repaint();
        }
        else if (filename.toLower().endsWith(".csv") || filename.toLower().endsWith(".data"))
        {
            ClearData();
            ImportData(filename);
            ResetPositiveClass();
            UpdateInfo();
            canvas->repaint();
        }
    }
    event->acceptProposedAction();
}

void MLDemos::ExportSVG()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Save Vector Image"), "", tr("Images (*.svg)"));
    if (filename.isEmpty()) return;
    if (!filename.endsWith(".svg")) filename += ".svg";

    DrawSVG svg(canvas, &mutex);
    svg.classifier = algo->classifier;
    svg.classifierMulti = algo->classifierMulti;
    svg.regressor = algo->regressor;
    svg.clusterer = algo->clusterer;
    svg.dynamical = algo->dynamical;
    svg.maximizer = algo->maximizer;
    svg.projector = algo->projector;
    if (algo->classifier) svg.drawClass = algo->classifiers[algo->tabUsedForTraining];
    if (algo->regressor) svg.drawRegr = algo->regressors[algo->tabUsedForTraining];
    if (algo->dynamical) svg.drawDyn = algo->dynamicals[algo->tabUsedForTraining];
    if (algo->clusterer) svg.drawClust = algo->clusterers[algo->tabUsedForTraining];
    if (algo->projector) svg.drawProj = algo->projectors[algo->tabUsedForTraining];
    svg.Write(filename);
    ui.statusBar->showMessage("Vector Image saved successfully");
}

void MLDemos::Screenshot()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Save Screenshot"), "", tr("Images (*.png *.jpg)"));
    if (filename.isEmpty()) return;
    if (!filename.endsWith(".jpg") && !filename.endsWith(".png")) filename += ".png";
    if (canvas->canvasType == 1) {
        QImage img = glw->grabFrameBuffer();
        if (!img.save(filename)) ui.statusBar->showMessage("WARNING: Unable to save image");
        else ui.statusBar->showMessage("Image saved successfully");
    }
    else if(canvas->canvasType == 2) {
        if (!vis->SaveScreenshot(filename)) ui.statusBar->showMessage("WARNING: Unable to save image");
        else ui.statusBar->showMessage("Image saved successfully");
    }
    else
    {
        if (!canvas->SaveScreenshot(filename)) ui.statusBar->showMessage("WARNING: Unable to save image");
        else ui.statusBar->showMessage("Image saved successfully");
    }
}

void MLDemos::ToClipboard()
{
    QClipboard *clipboard = QApplication::clipboard();
    if (canvas->canvasType == 1) {
        QImage img = glw->grabFrameBuffer();
        clipboard->setImage(img);
        clipboard->setPixmap(QPixmap::fromImage(img));
    } else if (canvas->canvasType == 2) {
        QPixmap pixmap = vis->GetDisplayPixmap();
        clipboard->setPixmap(pixmap);
        clipboard->setImage(pixmap.toImage());
    } else {
        QPixmap screenshot = canvas->GetScreenshot();
        if (screenshot.isNull()) {
            ui.statusBar->showMessage("WARNING: Nothing to copy to clipboard");
            return;
        }
        clipboard->setImage(screenshot.toImage());
        clipboard->setPixmap(screenshot);
    }
    ui.statusBar->showMessage("Image copied successfully to clipboard");
}
