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

/************************************/
/*                                  */
/*      Input Output functions      */
/*                                  */
/************************************/

void MLDemos::DataEdited()
{
    canvas->ResetSamples();
    FitToData();
    ResetPositiveClass();
    UpdateInfo();
    canvas->repaint();
}

void MLDemos::Clear()
{
    drawTimer->Stop();
    drawTimer->Clear();
    QMutexLocker lock(&mutex);
    qApp->processEvents();
    algo->Clear();
    glw->clearLists();
    if (canvas) {
        canvas->maps.confidence = QPixmap();
        canvas->maps.model = QPixmap();
        canvas->maps.info = QPixmap();
        canvas->liveTrajectory.clear();
        canvas->sampleColors.clear();
        canvas->maps.animation = QPixmap();
    }
    UpdateInfo();
    canvas->repaint();
}

void MLDemos::ClearData()
{
    algo->ClearData();
    if (canvas) {
        canvas->sampleColors.clear();
        canvas->data->Clear();
        canvas->maps.model = QPixmap();
    }
    UpdateInfo();
    canvas->repaint();
}

void MLDemos::ClearAll()
{
    algo->ClearData();
    if (canvas)
    {
        canvas->dimNames.clear();
        canvas->classNames.clear();
        canvas->sampleColors.clear();
        canvas->data->Clear();
        canvas->targets.clear();
        canvas->targetAge.clear();
        canvas->maps.animation = QPixmap();
        canvas->maps.reward = QPixmap();
        canvas->maps.samples = QPixmap();
        canvas->maps.trajectories = QPixmap();
        canvas->maps.grid = QPixmap();
    }
    Clear();
    ResetPositiveClass();
    FitToData();
    UpdateInfo();
}

void MLDemos::AddDataFromGenerator()
{
    ClearData();
    pair<vector<fvec>,ivec> newData = generator->Generate();
    canvas->data->AddSamples(newData.first, newData.second);
    ResetPositiveClass();
    FitToData();
}

void MLDemos::ResetPositiveClass()
{
    int labMin = INT_MAX, labMax = INT_MIN;
    if (!canvas->data->GetCount())
    {
        labMin = 0;
        labMax = 1;
        algo->optionsClassify->positiveSpin->setValue(1);
    }
    else
    {
        ivec labels = canvas->data->GetLabels();
        FOR (i, labels.size())
        {
            if (labels[i] > labMax) labMax = labels[i];
            if (labels[i] < labMin) labMin = labels[i];
        }
    }
    if (labMin == labMax) labMax = labMin+1;
    int dimCount = max(2,canvas->data->GetDimCount());
    int currentOutputDim = compare->params->outputDimCombo->currentIndex();

    compare->params->outputDimCombo->clear();
    algo->optionsRegress->outputDimCombo->clear();
    FOR (i, dimCount)
    {
        if (i < canvas->dimNames.size())
        {
            compare->params->outputDimCombo->addItem(QString("%1) %2").arg(i+1).arg(canvas->dimNames.at(i)));
            algo->optionsRegress->outputDimCombo->addItem(QString("%1) %2").arg(i+1).arg(canvas->dimNames.at(i)));
        }
        else
        {
            compare->params->outputDimCombo->addItem(QString("%1").arg(i+1));
            algo->optionsRegress->outputDimCombo->addItem(QString("%1").arg(i+1));
        }
    }
    if (currentOutputDim < dimCount) compare->params->outputDimCombo->setCurrentIndex(currentOutputDim);

    int currentPositive = algo->optionsClassify->positiveSpin->value();
    algo->optionsClassify->positiveSpin->setRange(labMin,labMax);
    if (currentPositive < labMin)
        algo->optionsClassify->positiveSpin->setValue(labMin);
    else if (currentPositive > labMax)
        algo->optionsClassify->positiveSpin->setValue(labMax);
    ui.canvasX1Spin->setRange(1,dimCount);
    ui.canvasX2Spin->setRange(1,dimCount);
    ui.canvasX3Spin->setRange(0,dimCount);
    canvas->SetDim(ui.canvasX1Spin->value()-1,ui.canvasX2Spin->value()-1, ui.canvasX3Spin->value()-1);
    dataEdit->Update();
    ManualSelectionUpdated();
    InputDimensionsUpdated();
}

void MLDemos::DimPlus()
{
    if (!canvas->data->GetCount()) return;
    vector<fvec> samples = canvas->data->GetSamples();
    int dim = samples[0].size()+1;
    FOR (i, samples.size()) samples[i].push_back(0);
    canvas->data->SetSamples(samples);
    canvas->zooms.push_back(1.f);
    canvas->center.push_back(0.f);
    canvas->ResetSamples();
    ResetPositiveClass();
    Clear();
    if (canvas->canvasType == 0) ui.canvasX2Spin->setValue(dim);
    canvas->repaint();
}

void MLDemos::DimLess()
{
    if (!canvas->data->GetCount()) return;
    vector<fvec> samples = canvas->data->GetSamples();
    int dim = samples[0].size();
    switch (dim)
    {
    case 1:
        break;
    case 2:
        {
            FOR (i, samples.size()) samples[i][1] = 0;
            canvas->data->SetSamples(samples);
            canvas->center[1] = 0;
            canvas->zooms[1] = 1.f;
        }
        break;
    default:
        {
            FOR (i, samples.size()) samples[i].pop_back();
            canvas->data->SetSamples(samples);
            canvas->zooms.pop_back();
            canvas->center.pop_back();
            if (ui.canvasX2Spin->value() == dim) ui.canvasX2Spin->setValue(dim-1);
        }
        break;
    }
    canvas->ResetSamples();
    ResetPositiveClass();
    Clear();
    canvas->repaint();
}

void MLDemos::SetData(std::vector<fvec> samples, ivec labels, std::vector<ipair> trajectories, bool bProjected)
{
    algo->sourceData.clear();
    algo->sourceLabels.clear();
    algo->projectedData.clear();
    if (!canvas) return;
    canvas->dimNames.clear();
    canvas->sampleColors.clear();
    canvas->data->Clear();
    canvas->data->AddSamples(samples, labels);
    canvas->data->bProjected = bProjected;
    if (bProjected) ui.status->setText("Projected Data");
    else ui.status->setText("Raw Data");
    if (trajectories.size()) canvas->data->AddSequences(trajectories);
    FitToData();
    ResetPositiveClass();
    ManualSelectionUpdated();
    algo->optionsRegress->outputDimCombo->setCurrentIndex(algo->optionsRegress->outputDimCombo->count()-1);
    CanvasOptionsChanged();
    canvas->ResetSamples();
    canvas->repaint();
}

void MLDemos::SetDimensionNames(QStringList headers)
{
    //qDebug() << "setting dimension names" << headers;
    canvas->dimNames = headers;
    ResetPositiveClass();
    CanvasOptionsChanged();
    dataEdit->Update();
    canvas->ResetSamples();
    canvas->repaint();
}

void MLDemos::SetClassNames(std::map<int,QString> classNames)
{
    canvas->classNames = classNames;
    ResetPositiveClass();
    CanvasOptionsChanged();
    dataEdit->Update();
    canvas->ResetSamples();
    canvas->repaint();
}

void MLDemos::SetCategorical(std::map<int,std::vector<std::string> > categorical)
{
    canvas->data->categorical = categorical;
    ResetPositiveClass();
    CanvasOptionsChanged();
    dataEdit->Update();
    canvas->ResetSamples();
    canvas->repaint();
}

void MLDemos::SetTimeseries(std::vector<TimeSerie> timeseries)
{
    //	qDebug() << "importing " << timeseries.size() << " timeseries";
    algo->sourceData.clear();
    algo->sourceLabels.clear();
    algo->projectedData.clear();
    if (!canvas) return;
    canvas->dimNames.clear();
    canvas->sampleColors.clear();
    canvas->data->Clear();
    canvas->data->AddTimeSeries(timeseries);
    FitToData();
    ResetPositiveClass();
    ManualSelectionUpdated();
    canvas->ResetSamples();
    canvas->repaint();
    qDebug() << "added " << canvas->data->GetTimeSeries().size() << " timeseries";
    //	qDebug() << " dim: " << dim << " count: " << count << " frames: " << frames;

    /*
 vector<TimeSerie> series = canvas->data->GetTimeSeries();
 FOR (i, series.size())
 {
  FOR (j, series[i].size())
  {
   qDebug() << i << " " << j << ": " << series[i][j][0];
   FOR (d, series[i][j].size())
   {
//				qDebug() << i << " " << j << " " << d << ": " << series[i][j][d];
   }
  }
 }
 */
}

