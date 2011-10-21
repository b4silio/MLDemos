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
        if(canvas->canvasType) CanvasOptionsChanged();
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
            CanvasOptionsChanged();
        }
        canvas->repaint();
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

fvec ClusterMetrics(std::vector<fvec> samples, ivec labels, std::vector<fvec> scores, float ratio = 1.f)
{
    fvec results(4, 0);
    results[0] = drand48();
    if(!samples.size() || !scores.size()) return results;
    int dim = samples[0].size();
    int nbClusters = scores[0].size();
    int count = samples.size();
    // compute bic
    double loglik = 0;

    vector<fvec> means(nbClusters);
    FOR(k, nbClusters)
    {
        means[k] = fvec(dim, 0);
        float contrib = 0;
        FOR(i, count)
        {
            contrib += scores[i][k];
            means[k] += samples[i]*scores[i][k];
        }
        means[k] /= contrib;
    }

    float log_lik=0;
    float like;
    float *pxi = new float[nbClusters];
    int data_i=0;
    int state_i;

    fvec loglikes(nbClusters);
    FOR(k, nbClusters)
    {
        float rss = 0;
        double contrib = 0;
        FOR(i, count)
        {
            contrib += scores[i][k];
            if(contrib==0) continue;
            fvec diff = samples[i]-means[k];
            rss += diff*diff*scores[i][k];
        }
        loglikes[k] = rss;
    }
    FOR(k, nbClusters) loglik += loglikes[k];
    //loglik /= nbClusters;

    results[0] = loglik; // RSS
    results[1] = log(count)*nbClusters + loglik; // BIC
    results[2] = 2*nbClusters + loglik; // AIC


    // we compute the f-measures for each class
    map<int,int> classcounts;
    int cnt = 0;
    FOR(i, labels.size()) if(!classcounts.count(labels[i])) classcounts[labels[i]] = cnt++;
    int classCount = classcounts.size();
    map<int, fvec> classScores;
    fvec clusterScores(nbClusters);
    map<int,float> labelScores;

    if(ratio == 1.f)
    {
        FOR(i, labels.size())
        {
            labelScores[labels[i]] += 1.f;
            if(!classScores.count(labels[i]))classScores[labels[i]].resize(nbClusters);
            FOR(k, nbClusters)
            {
                classScores[labels[i]][k] += scores[i][k];
                clusterScores[k] += scores[i][k];
            }
        }
    }
    else
    {
        u32 *perm = randPerm(labels.size());
        map<int, ivec> indices;
        FOR(i, labels.size()) indices[labels[perm[i]]].push_back(perm[i]);
        for(map<int,ivec>::iterator it = indices.begin(); it != indices.end(); it++)
        {
            int labelCount = max(1,int(it->second.size()*ratio));
            FOR(i, labelCount)
            {
                labelScores[labels[it->second[i]]] += 1.f;
                if(!classScores.count(labels[it->second[i]]))classScores[labels[it->second[i]]].resize(nbClusters);
                FOR(k, nbClusters)
                {
                    classScores[labels[it->second[i]]][k] += scores[it->second[i]][k];
                    clusterScores[k] += scores[it->second[i]][k];
                }
            }
        }
        delete [] perm;
    }

    float fmeasure = 0;
    map<int,float>::iterator it2 = labelScores.begin();
    for(map<int,fvec>::iterator it = classScores.begin(); it != classScores.end(); it++, it2++)
    {
        float maxScore = -FLT_MAX;
        FOR(k, nbClusters)
        {
            float precision = it->second[k] / it2->second;
            float recall = it->second[k] / clusterScores[k];
            float f1 = 2*precision*recall/(precision+recall);
            maxScore = max(maxScore,f1);
        }
        fmeasure += maxScore;
    }
    int classAndClusterCount = classCount;
    // we penalize empty clusters
    FOR(k, nbClusters) if(clusterScores[k] == 0) classAndClusterCount++; // we have an empty cluster!
    fmeasure /= classAndClusterCount;

    results[3] = -fmeasure; // F-Measure

    return results;
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

    // we compute the stats on the clusters (f-measure, bic etc)

    vector<fvec> samples = canvas->data->GetSamples();
    ivec labels = canvas->data->GetLabels();
    vector<fvec> clusterScores(samples.size());
    FOR(i, canvas->data->GetCount())
    {
        fvec result = clusterer->Test(samples[i]);
        if(clusterer->NbClusters()==1) clusterScores[i] = result;
        else if(result.size()>1) clusterScores[i] = result;
        else if(result.size())
        {
            fvec res(clusterer->NbClusters(),0);
            res[result[0]] = 1.f;
        }
    }

    int ratioIndex = optionsCluster->trainRatioCombo->currentIndex();
    float ratios[] = {0.01f, 0.05f, 0.1f, 0.2f, 1.f/3.f, 0.5f, 0.75f, 1.f};
    float ratio = ratios[ratioIndex];

    fvec clusterMetrics = ClusterMetrics(samples, labels, clusterScores, ratio);

    optionsCluster->resultList->clear();
    optionsCluster->resultList->addItem(QString("rss: %1").arg(clusterMetrics[0], 0, 'f', 2));
    optionsCluster->resultList->addItem(QString("bic: %1").arg(clusterMetrics[1], 0, 'f', 2));
    optionsCluster->resultList->addItem(QString("aic: %1").arg(clusterMetrics[2], 0, 'f', 2));
    optionsCluster->resultList->addItem(QString("f1: %1").arg(clusterMetrics[3], 0, 'f', 2));
    FOR(i, clusterMetrics.size())
    {
        optionsCluster->resultList->item(i)->setForeground(i ? SampleColor[i%SampleColorCnt] : Qt::gray);
    }


    // we fill in the canvas sampleColors for the alternative display types
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
    canvas->repaint();

	UpdateInfo();
	drawTimer->clusterer= &this->clusterer;
	drawTimer->start(QThread::NormalPriority);
}

void MLDemos::ClusterOptimize()
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

    int tab = optionsCluster->tabWidget->currentIndex();
    if(tab >= clusterers.size() || !clusterers[tab]) return;
    clusterer = clusterers[tab]->GetClusterer();
    tabUsedForTraining = tab;

    int startCount=1, stopCount=11;

    vector<fvec> samples = canvas->data->GetSamples();
    ivec labels = canvas->data->GetLabels();
    int ratioIndex = optionsCluster->trainRatioCombo->currentIndex();
    float ratios[] = {0.01f, 0.05f, 0.1f, 0.2f, 1.f/3.f, 0.5f, 0.75f, 1.f};
    float ratio = ratios[ratioIndex];

    ivec kCounts;
    vector<fvec> results(4);
    for(int k=startCount; k<stopCount; k++)
    {
        clusterer->SetNbClusters(k);
        Train(clusterer);

        int folds = 10;
        fvec metricMeans(results.size());
        ivec foldCount(results.size());
        FOR(f, folds)
        {
            vector<fvec> clusterScores(samples.size());
            FOR(i, canvas->data->GetCount())
            {
                fvec result = clusterer->Test(samples[i]);
                if(clusterer->NbClusters()==1) clusterScores[i] = result;
                else if(result.size()>1) clusterScores[i] = result;
                else if(result.size())
                {
                    fvec res(clusterer->NbClusters(),0);
                    res[result[0]] = 1.f;
                }
            }
            fvec clusterMetrics = ClusterMetrics(samples, labels, clusterScores, ratio);
            FOR(d, clusterMetrics.size())
            {
                if(clusterMetrics[d] != clusterMetrics[d]) continue;
                metricMeans[d] += clusterMetrics[d];
                foldCount[d]++;
            }
        }
        FOR(d, metricMeans.size()) metricMeans[d] /= foldCount[d];
        kCounts.push_back(k);
        FOR(i, metricMeans.size()) results[i].push_back(metricMeans[i]);
    }

    int w = optionsCluster->graphLabel->width();
    int h = optionsCluster->graphLabel->height();
    int pad = 6;
    QPixmap pixmap(w,h);
    QBitmap bitmap(w,h);
    bitmap.clear();
    pixmap.setMask(bitmap);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);

    painter.setPen(QPen(Qt::black, 1.f));
    painter.drawLine(pad, h - 2*pad, w-pad, h-2*pad);
    painter.drawLine(pad, 0, pad, h-2*pad);
    QFont font = painter.font();
    font.setPointSize(9);
    painter.setFont(font);
    FOR(k, kCounts.size())
    {
        float x = k/(float)(kCounts.size()-1);
        painter.drawLine(x*(w-2*pad)+pad, h-2*pad-1, x*(w-2*pad)+pad, h-2*pad+1);
        if(k == kCounts.size()-1) x -= 0.05;
        painter.drawText(x*(w-2*pad)-2+pad, h-1, QString("%1").arg(kCounts[k]));
    }

    painter.setRenderHint(QPainter::Antialiasing);
    fvec mins(results.size(), FLT_MAX), maxes(results.size(), -FLT_MAX);
    FOR(i, results.size())
    {
        FOR(j, results[i].size())
        {
            mins[i] = min(mins[i], results[i][j]);
            maxes[i] = max(maxes[i], results[i][j]);
        }
    }
    vector< pair<float,int> > bests(results.size());
    FOR(i, results.size())
    {
        QPointF old;
        painter.setPen(QPen(i ? SampleColor[i%SampleColorCnt] : Qt::gray,2));
        bests[i] = make_pair(FLT_MAX, 0);
        FOR(k, kCounts.size())
        {
            if(results[i][k] < bests[i].first)
            {
                bests[i] = make_pair(results[i][k], kCounts[k]);
            }
            float x = k/(float)(kCounts.size()-1);
            float y = (results[i][k] - mins[i])/(maxes[i]-mins[i]);
            if(i == 3) y = 1.f - y; // fmeasures needs to be maximized
            QPointF point(x*(w-2*pad)+pad, (1.f-y)*(h-2*pad));
            if(k) painter.drawLine(old, point);
            old = point;
        }
    }
    optionsCluster->graphLabel->setPixmap(pixmap);

    optionsCluster->resultList->clear();
    optionsCluster->resultList->addItem(QString("rss: %1 (%2)").arg(bests[0].second).arg(bests[0].first, 0, 'f', 2));
    optionsCluster->resultList->addItem(QString("bic: %1 (%2)").arg(bests[1].second).arg(bests[1].first, 0, 'f', 2));
    optionsCluster->resultList->addItem(QString("aic: %1 (%2)").arg(bests[2].second).arg(bests[2].first, 0, 'f', 2));
    optionsCluster->resultList->addItem(QString("f1: %1 (%2)").arg(bests[3].second).arg(-bests[3].first, 0, 'f', 2));
    FOR(i, results.size())
    {
        optionsCluster->resultList->item(i)->setForeground(i ? SampleColor[i%SampleColorCnt] : Qt::gray);
    }

    int bestIndex = optionsCluster->optimizeCombo->currentIndex();
    clusterer->SetNbClusters(bests[bestIndex].second);
    Train(clusterer);

    // we fill in the canvas sampleColors for the alternative display types
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

    clusterers[tab]->Draw(canvas, clusterer);
    drawTimer->Clear();
    UpdateInfo();
    drawTimer->clusterer= &this->clusterer;
    drawTimer->start(QThread::NormalPriority);
    canvas->repaint();

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
    canvas->repaint();

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
    drawTimer->Clear();
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
    //canvas->FitToData();
    CanvasTypeChanged();
    CanvasOptionsChanged();
    if(!canvas->canvasType)
    {
        projectors[tab]->Draw(canvas, projector);
    }
    canvas->repaint();
    UpdateInfo();
}

void MLDemos::ProjectRevert()
{
    QMutexLocker lock(&mutex);
    drawTimer->Stop();
    drawTimer->Clear();
    DEL(clusterer);
    DEL(regressor);
    DEL(dynamical);
    DEL(classifier);
    DEL(maximizer);
    DEL(projector);
    if(!sourceData.size()) return;
    canvas->data->SetSamples(sourceData);
    canvas->data->SetLabels(sourceLabels);
    canvas->data->bProjected = false;
    canvas->maps.info = QPixmap();
    canvas->maps.model = QPixmap();
    canvas->FitToData();
    CanvasTypeChanged();
    CanvasOptionsChanged();
    canvas->repaint();
    UpdateInfo();
    sourceData.clear();
    sourceLabels.clear();
}

void MLDemos::ProjectReproject()
{
    if(!canvas) return;
    mutex.lock();
    sourceData = canvas->data->GetSamples();
    sourceLabels = canvas->data->GetLabels();
    mutex.unlock();
    Project();
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

