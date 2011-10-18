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
#include "basicMath.h"
#include "classifier.h"
#include "regressor.h"
#include "dynamical.h"
#include "clusterer.h"
#include "maximize.h"
#include "roc.h"
#include <QDebug>
#include <fstream>
#include <QPixmap>
#include <QBitmap>
#include <QSettings>
#include <QFileDialog>
#include <QProgressDialog>

using namespace std;

void MLDemos::Classify()
{
    if(!canvas || !canvas->data->GetCount()) return;
    drawTimer->Stop();
	drawTimer->Clear();
	mutex.lock();
    DEL(clusterer);
    DEL(regressor);
    DEL(dynamical);
    DEL(classifier);
	DEL(maximizer);
    DEL(projector);
    int tab = optionsClassify->tabWidget->currentIndex();
    if(tab >= classifiers.size() || !classifiers[tab]) return;
    classifier = classifiers[tab]->GetClassifier();
    tabUsedForTraining = tab;
    float ratios [] = {.1f,.25f,1.f/3.f,.5f,2.f/3.f,.75f,.9f,1.f};
    int ratioIndex = optionsClassify->traintestRatioCombo->currentIndex();
    float trainRatio = ratios[ratioIndex];
    int positive = optionsClassify->positiveSpin->value();

    bool trained = Train(classifier, positive, trainRatio);
    if(trained)
    {
		classifiers[tab]->Draw(canvas, classifier);
		UpdateInfo();
		if(drawTimer && classifier->UsesDrawTimer())
        {
            drawTimer->classifier = &this->classifier;
            drawTimer->start(QThread::NormalPriority);
        }
        if(canvas->canvasType) CanvasZoomChanged();
        // we fill in the canvas sampleColors
        vector<fvec> samples = canvas->data->GetSamples();
        canvas->sampleColors.resize(samples.size());
        FOR(i, samples.size())
        {
            canvas->sampleColors[i] = DrawTimer::GetColor(classifier, samples[i]);
        }
        if(canvas->canvasType)
        {
            canvas->maps.model = QPixmap();
            CanvasZoomChanged();
        }
    }
    else
    {
        mutex.unlock();
        Clear();
		mutex.lock();
		UpdateInfo();
	}
	mutex.unlock();
}


void MLDemos::ClassifyCross()
{
    if(!canvas || !canvas->data->GetCount()) return;
    drawTimer->Stop();
    QMutexLocker lock(&mutex);
    DEL(clusterer);
    DEL(regressor);
    DEL(dynamical);
    DEL(classifier);
	DEL(maximizer);
    DEL(projector);
    int tab = optionsClassify->tabWidget->currentIndex();
    if(tab >= classifiers.size() || !classifiers[tab]) return;
    tabUsedForTraining = tab;

    float ratios [] = {.1f,.25f,1.f/3.f,.5f,2.f/3.f,.75f,.9f,1.f};
    int ratioIndex = optionsClassify->traintestRatioCombo->currentIndex();
    float trainRatio = ratios[ratioIndex];
    int positive = optionsClassify->positiveSpin->value();
    int foldCount = optionsClassify->foldCountSpin->value();

    vector<fvec> fmeasures;
    fmeasures.resize(2);
    bool trained = false;
    FOR(f,foldCount)
    {
        DEL(classifier);
        classifier = classifiers[tab]->GetClassifier();
        trained = Train(classifier, positive, trainRatio);
        if(!trained) break;
        if(classifier->rocdata.size()>0)
        {
            fmeasures[0].push_back(GetBestFMeasure(classifier->rocdata[0]));
        }
        if(classifier->rocdata.size()>1)
        {
            fmeasures[1].push_back(GetBestFMeasure(classifier->rocdata[1]));
        }
    }
    classifier->crossval = fmeasures;
    ShowCross();
    if(trained) classifiers[tab]->Draw(canvas, classifier);
    UpdateInfo();
}

void MLDemos::Regression()
{
    if(!canvas || !canvas->data->GetCount()) return;
    drawTimer->Stop();
	drawTimer->Clear();

    QMutexLocker lock(&mutex);
    DEL(clusterer);
    DEL(regressor);
    DEL(dynamical);
    DEL(classifier);
	DEL(maximizer);
    DEL(projector);
    int tab = optionsRegress->tabWidget->currentIndex();
    if(tab >= regressors.size() || !regressors[tab]) return;
    regressor = regressors[tab]->GetRegressor();
    tabUsedForTraining = tab;

    float ratios [] = {.1f,.25f,1.f/3.f,.5f,2.f/3.f,.75f,.9f,1.f};
    int ratioIndex = optionsRegress->traintestRatioCombo->currentIndex();
    float trainRatio = ratios[ratioIndex];

    Train(regressor, trainRatio);
    regressors[tab]->Draw(canvas, regressor);
    UpdateInfo();
}

void MLDemos::RegressionCross()
{
    if(!canvas || !canvas->data->GetCount()) return;
    drawTimer->Stop();
	drawTimer->Clear();
	QMutexLocker lock(&mutex);
    DEL(clusterer);
    DEL(regressor);
    DEL(dynamical);
    DEL(classifier);
	DEL(maximizer);
    DEL(projector);
    int tab = optionsRegress->tabWidget->currentIndex();
    if(tab >= regressors.size() || !regressors[tab]) return;
    regressor = regressors[tab]->GetRegressor();
    tabUsedForTraining = tab;

    float ratios [] = {.1f,.25f,1.f/3.f,.5f,2.f/3.f,.75f,.9f,1.f};
    int ratioIndex = optionsRegress->traintestRatioCombo->currentIndex();
    float trainRatio = ratios[ratioIndex];
    int foldCount = optionsRegress->foldCountSpin->value();

    vector<fvec> errors;
    errors.resize(2);
    bool trained = false;
    FOR(f,foldCount)
    {
        DEL(regressor);
        regressor = regressors[tab]->GetRegressor();
        Train(regressor, trainRatio);
        if(regressor->trainErrors.size())
        {
            errors[0] = regressor->trainErrors;
        }
        if(regressor->testErrors.size())
        {
            errors[1] = regressor->testErrors;
        }
    }
    regressor->crossval = errors;
    ShowCross();

	Train(regressor, trainRatio);
    regressors[tab]->Draw(canvas, regressor);
	UpdateInfo();
}

void MLDemos::Dynamize()
{
    if(!canvas || !canvas->data->GetCount() || !canvas->data->GetSequences().size()) return;
    drawTimer->Stop();
	drawTimer->Clear();
    QMutexLocker lock(&mutex);
    DEL(clusterer);
    DEL(regressor);
    DEL(dynamical);
    DEL(classifier);
	DEL(maximizer);
    DEL(projector);
    int tab = optionsDynamic->tabWidget->currentIndex();
    if(tab >= dynamicals.size() || !dynamicals[tab]) return;
    dynamical = dynamicals[tab]->GetDynamical();
    tabUsedForTraining = tab;

    Train(dynamical);
    dynamicals[tab]->Draw(canvas,dynamical);

	int w = canvas->width(), h = canvas->height();

	int resampleType = optionsDynamic->resampleCombo->currentIndex();
	int resampleCount = optionsDynamic->resampleSpin->value();
	int centerType = optionsDynamic->centerCombo->currentIndex();
	float dT = optionsDynamic->dtSpin->value();
	int zeroEnding = optionsDynamic->zeroCheck->isChecked();
	bool bColorMap = optionsDynamic->colorCheck->isChecked();

	vector< vector<fvec> > trajectories = canvas->data->GetTrajectories(resampleType, resampleCount, centerType, dT, zeroEnding);
	if(trajectories.size())
	{
		canvas->maps.model = QPixmap(w,h);
		QBitmap bitmap(w,h);
		bitmap.clear();
		canvas->maps.model.setMask(bitmap);
		canvas->maps.model.fill(Qt::transparent);
		QPainter painter(&canvas->maps.model);
		int dim = trajectories[0][0].size() / 2;
		fvec start(dim,0);
		FOR(i, trajectories.size())
		{
			FOR(d, dim) start[d] = trajectories[i][0][d];
			vector<fvec> result = dynamical->Test(start, 1000);
			fvec oldPt = result[0];
			int count = result.size();
			FOR(j, count-1)
			{
				fvec pt = result[j+1];
				painter.setPen(QPen(Qt::green, 2));
				painter.drawLine(canvas->toCanvasCoords(pt), canvas->toCanvasCoords(oldPt));
				//if(j<count-2) Canvas::drawSample(painter, canvas->toCanvasCoords(pt), 5, 2);
				oldPt = pt;
			}
			painter.setBrush(Qt::NoBrush);
			painter.setPen(Qt::green);
			painter.drawEllipse(canvas->toCanvasCoords(result[0]), 5, 5);
			painter.setPen(Qt::red);
			painter.drawEllipse(canvas->toCanvasCoords(result[count-1]), 5, 5);
		}
	}

	// the first index is "none", so we subtract 1
    int avoidIndex = optionsDynamic->obstacleCombo->currentIndex()-1;
    if(avoidIndex >=0 && avoidIndex < avoiders.size() && avoiders[avoidIndex])
    {
        DEL(dynamical->avoid);
		dynamical->avoid = avoiders[avoidIndex]->GetObstacleAvoidance();
    }
	UpdateInfo();
	if(dynamicals[tab]->UsesDrawTimer())
	{
		drawTimer->bColorMap = bColorMap;
		drawTimer->start(QThread::NormalPriority);
	}
}

void MLDemos::Avoidance()
{
    if(!canvas || !dynamical) return;
    drawTimer->Stop();
    QMutexLocker lock(&mutex);
	// the first index is "none", so we subtract 1
	int index = optionsDynamic->obstacleCombo->currentIndex()-1;
    if(index >=0 && index >= avoiders.size() || !avoiders[index]) return;
    DEL(dynamical->avoid);
    dynamical->avoid = avoiders[index]->GetObstacleAvoidance();
	UpdateInfo();
	drawTimer->Clear();
    drawTimer->start(QThread::NormalPriority);
}

void MLDemos::Cluster()
{
    if(!canvas || !canvas->data->GetCount()) return;
    drawTimer->Stop();
    QMutexLocker lock(&mutex);
    DEL(clusterer);
    DEL(regressor);
    DEL(dynamical);
    DEL(classifier);
	DEL(maximizer);
    DEL(projector);
    int tab = optionsCluster->tabWidget->currentIndex();
    if(tab >= clusterers.size() || !clusterers[tab]) return;
    clusterer = clusterers[tab]->GetClusterer();
    tabUsedForTraining = tab;
    Train(clusterer);
	drawTimer->Stop();
	drawTimer->Clear();
    clusterers[tab]->Draw(canvas,clusterer);

    // we fill in the canvas sampleColors
    vector<fvec> samples = canvas->data->GetSamples();
    canvas->sampleColors.resize(samples.size());
    FOR(i, samples.size())
    {
        fvec res = clusterer->Test(samples[i]);
        float r=0,g=0,b=0;
        if(res.size() > 1)
        {
            FOR(j, res.size())
            {
                r += SampleColor[(j+1)%SampleColorCnt].red()*res[j];
                g += SampleColor[(j+1)%SampleColorCnt].green()*res[j];
                b += SampleColor[(j+1)%SampleColorCnt].blue()*res[j];
            }
        }
        else if(res.size())
        {
            r = (1-res[0])*255 + res[0]* 255;
            g = (1-res[0])*255;
            b = (1-res[0])*255;
        }
        canvas->sampleColors[i] = QColor(r,g,b);
    }
    canvas->maps.model = QPixmap();

	UpdateInfo();
	drawTimer->clusterer= &this->clusterer;
	drawTimer->start(QThread::NormalPriority);
}

void MLDemos::ClusterIterate()
{
    if(!canvas || !canvas->data->GetCount()) return;
    drawTimer->Stop();
    int tab = optionsCluster->tabWidget->currentIndex();
    if(tab >= clusterers.size() || !clusterers[tab]) return;
    QMutexLocker lock(&mutex);
    if(!clusterer)
    {
        clusterer = clusterers[tab]->GetClusterer();
        tabUsedForTraining = tab;
    }
    else clusterers[tab]->SetParams(clusterer);
    clusterer->SetIterative(true);
    Train(clusterer);
    clusterers[tab]->Draw(canvas,clusterer);
	UpdateInfo();
}

void MLDemos::Maximize()
{
	if(!canvas) return;
	if(canvas->maps.reward.isNull()) return;
	QMutexLocker lock(&mutex);
	drawTimer->Stop();
	DEL(clusterer);
	DEL(regressor);
	DEL(dynamical);
	DEL(classifier);
	DEL(maximizer);
    DEL(projector);
    int tab = optionsMaximize->tabWidget->currentIndex();
	if(tab >= maximizers.size() || !maximizers[tab]) return;
	maximizer = maximizers[tab]->GetMaximizer();
	maximizer->maxAge = optionsMaximize->iterationsSpin->value();
	maximizer->stopValue = optionsMaximize->stoppingSpin->value();
	tabUsedForTraining = tab;
	Train(maximizer);

	UpdateInfo();
	drawTimer->Stop();
	drawTimer->Clear();
	drawTimer->start(QThread::NormalPriority);
}

void MLDemos::MaximizeContinue()
{
	if(!canvas || !maximizer) return;
	QMutexLocker lock(&mutex);
	if(drawTimer)
	{
		drawTimer->Stop();
	}
	maximizer->SetConverged(!maximizer->hasConverged());

	UpdateInfo();
	if(drawTimer)
	{
		drawTimer->start(QThread::NormalPriority);
	}
}

void MLDemos::Project()
{
    if(!canvas) return;
    QMutexLocker lock(&mutex);
    drawTimer->Stop();
    DEL(clusterer);
    DEL(regressor);
    DEL(dynamical);
    DEL(classifier);
    DEL(maximizer);
    DEL(projector);
    int tab = optionsProject->tabWidget->currentIndex();
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
    Train(projector);
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
    QPixmap infoPixmap;
    QPainter painter(&infoPixmap);
    projectors[tab]->DrawInfo(canvas, painter, projector);
    canvas->FitToData();
    CanvasTypeChanged();
    CanvasZoomChanged();
    canvas->repaint();
    UpdateInfo();
    if(drawTimer->isRunning())
    {
        drawTimer->Stop();
        drawTimer->Clear();
    }
}

void MLDemos::ProjectRevert()
{
    if(!sourceData.size()) return;
    canvas->data->SetSamples(sourceData);
    canvas->data->SetLabels(sourceLabels);
    canvas->data->bProjected = false;
    canvas->FitToData();
    CanvasTypeChanged();
    CanvasZoomChanged();
    canvas->repaint();
    UpdateInfo();
    if(drawTimer->isRunning())
    {
        drawTimer->Stop();
        drawTimer->Clear();
    }
    sourceData.clear();
    sourceLabels.clear();
}

void MLDemos::ProjectReproject()
{
    if(!canvas) return;
    QMutexLocker lock(&mutex);
    drawTimer->Stop();
    DEL(clusterer);
    DEL(regressor);
    DEL(dynamical);
    DEL(classifier);
    DEL(maximizer);
    DEL(projector);
    int tab = optionsProject->tabWidget->currentIndex();
    if(tab >= projectors.size() || !projectors[tab]) return;
    projector = projectors[tab]->GetProjector();
    projectors[tab]->SetParams(projector);
    tabUsedForTraining = tab;
    Train(projector);
    sourceData = canvas->data->GetSamples();
    sourceLabels = canvas->data->GetLabels();
    projectedData = projector->GetProjected();
    if(projectedData.size())
    {
        canvas->data->SetSamples(projectedData);
        canvas->data->bProjected = true;
    }
    QPixmap infoPixmap;
    QPainter painter(&infoPixmap);
    projectors[tab]->DrawInfo(canvas, painter, projector);
    canvas->FitToData();
    CanvasTypeChanged();
    CanvasZoomChanged();
    canvas->repaint();

    UpdateInfo();
    if(drawTimer->isRunning())
    {
        drawTimer->Stop();
        drawTimer->Clear();
    }
}

void MLDemos::ExportOutput()
{
    if(!classifier && !regressor && !clusterer && !dynamical && !maximizer) return;
    // get a file
}

void MLDemos::ExportAnimation()
{
    if(!canvas->data->GetSamples().size()) return;
}

