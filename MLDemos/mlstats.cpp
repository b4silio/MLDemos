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
#include "basicOpenCV.h"
#include "classifier.h"
#include "regressor.h"
#include "clusterer.h"
#include "roc.h"
#include <QDebug>
#include <fstream>
#include <QPixmap>
#include <QBitmap>
#include <QSettings>
#include <QFileDialog>
#include <vector>
#include <algorithm>

using namespace std;

void MLDemos::MouseOnRoc(QMouseEvent *event)
{
	int e;
	switch( event->button())
	{
	case Qt::LeftButton:
		e = CV_EVENT_LBUTTONUP;
		break;
	case Qt::RightButton:
		e = CV_EVENT_RBUTTONUP;
		break;
	}
	roc_on_mouse(e, event->x(), event->y(), 0, 0);
	rocWidget->ShowImage(GetRocImage());
	statsDialog->repaint();
}

QPixmap RocImage(std::vector< std::vector<f32pair> > rocdata, std::vector<const char *> roclabels, QSize size)
{
	QPixmap pixmap(size);
	QPainter painter(&pixmap);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setBackgroundMode(Qt::OpaqueMode);
	painter.setBackground(Qt::white);
	painter.fillRect(QRect(0,0,size.width(),size.height()), Qt::white);

	int PAD = 16;

	FOR(d, rocdata.size())
	{
		int minCol = 70;
		int color = (rocdata.size() == 1) ? 255 : (255 - minCol)*(rocdata.size() - d -1)/(rocdata.size()-1) + minCol;
		color = 255 - color;

		std::vector<f32pair> data = rocdata[d];
		if(!data.size()) continue;
		std::sort(data.begin(), data.end());

		std::vector<fvec> allData;
		cvVec2 oldVal(1,0);
		FOR(i, data.size())
		{
			float thresh = data[i].first;
			u32 tp = 0, fp = 0;
			u32 fn = 0, tn = 0;

			FOR(j, data.size())
			{
				if(data[j].second == 1)
				{
					if(data[j].first >= thresh) tp++;
					else fn++;
				}
				else 
				{
					if(data[j].first >= thresh) fp++;
					else tn++;
				}
			}
			cvVec2 val;
			float fmeasure = 0;
			if((fp+tn)>0 && (tp+fn)>0 && (tp+fp)>0)
			{
				val=cvVec2(fp/float(fp+tn), 1 - tp/float(tp+fn));
				float precision = tp / float(tp+fp);
				float recall = tp /float(tp+fn);
				fmeasure = tp == 0 ? 0 : 2 * (precision * recall) / (precision + recall);
			}

			fvec dat;
			dat.push_back(val.x);
			dat.push_back(val.y);
			dat.push_back(data[i].first);
			dat.push_back(fmeasure);
			allData.push_back(dat);

			oldVal = val;
		}

		painter.setPen(QColor(color,color,color));
		// we highlight the current roc curve
		cvVec2 pt1, pt2;
		FOR(i, allData.size()-1)
		{
			pt1 = cvVec2(allData[i][0]*size.width(), allData[i][1]*size.height());
			pt2 = cvVec2(allData[i+1][0]*size.width(), allData[i+1][1]*size.height());
			painter.drawLine(QPointF(pt1.x+PAD, pt1.y+PAD),QPointF(pt2.x+PAD, pt2.y+PAD));
		}
		pt1 = cvVec2(0,size.width());
		painter.drawLine(QPointF(pt1.x+PAD, pt1.y+PAD),QPointF(pt2.x+PAD, pt2.y+PAD));

		//cvDrawLine(roc, cvPoint(0 + PAD,resolution.height + PAD), cvPoint(oldVal.x*resolution.width+vpad.x,oldVal.y*resolution.height+vpad.y), color);
		if(d < roclabels.size())
		{
			QPointF pos(3*size.width()/4,size.height() - (d+1)*16);
			painter.drawText(pos,QString(roclabels[roclabels.size()-1-d]));
			//cvPutText(roc, roclabels[d], point, rocfont, color);
		}
	}
	return pixmap;
}

QPixmap BoxPlot(std::vector<fvec> allData, QSize size, float maxVal=-FLT_MAX, float minVal=FLT_MAX)
{
	QPixmap boxplot(size);
	if(!allData.size()) return boxplot;
	QPainter painter(&boxplot);

	//	painter.setRenderHint(QPainter::Antialiasing);

	painter.setBackgroundMode(Qt::OpaqueMode);
	painter.setBackground(Qt::white);
	painter.fillRect(QRect(0,0,size.width(),size.height()), Qt::white);


	FOR(d,allData.size())
	{
		fvec data = allData[d];
		if(!data.size()) continue;
		FOR(i, data.size()) maxVal = max(maxVal, data[i]);
		FOR(i, data.size()) minVal = min(minVal, data[i]);
	}
	if(minVal == maxVal)
	{
		minVal = minVal/2;
		minVal = minVal*3/2;
	}

	FOR(d,allData.size())
	{
		int minCol = 70;
		int color = (allData.size() == 1) ? minCol : (255-minCol) * d / allData.size() + minCol;

		fvec data = allData[d];
		if(!data.size()) continue;
		int hpad = 15 + (d*size.width()/(allData.size()));
		int pad = -16;
		int res = size.height()+2*pad;

		float mean = 0;
		float sigma = 0;
		FOR(i, data.size()) mean += data[i] / data.size();
		FOR(i, data.size()) sigma += powf(data[i]-mean,2);
		sigma = sqrtf(sigma/data.size());

		float edge = minVal;
		float delta = maxVal - minVal;

		float top, bottom, median, quartLow, quartHi;
		vector<float> outliers;
		vector<float> sorted;

		if(data.size() > 1)
		{
			if(sigma==0)
			{
				sorted = data;
			}
			else
			{
				// we look for outliers using the 3*sigma rule
				FOR(i, data.size())
				{
					if (data[i] - mean < 3*sigma)
						sorted.push_back(data[i]);
					else outliers.push_back(data[i]);
				}
			}
			if(!sorted.size()) return boxplot;
			sort(sorted.begin(), sorted.end());
			int count = sorted.size();
			int half = count/2;
			bottom = sorted[0];
			top = sorted[sorted.size()-1];

			median = count%2 ? sorted[half] : (sorted[half] + sorted[half - 1])/2;
			quartLow, quartHi;
			if(count < 4)
			{
				quartLow = bottom;
				quartHi = top;
			}
			else
			{
				quartLow = half%2 ? sorted[half/2] : (sorted[half/2] + sorted[half/2 - 1])/2;
				quartHi = half%2 ? sorted[half*3/2] : (sorted[half*3/2] + sorted[half*3/2 - 1])/2;
			}
		}
		else
		{
			top = bottom = median = quartLow = quartHi = data[0];
		}

		QPointF bottomPoint = QPointF(0, size.height() - (int)((bottom-edge)/delta*res) + pad);
		QPointF topPoint = QPointF(0, size.height() - (int)((top-edge)/delta*res) + pad);
		QPointF medPoint = QPointF(0, size.height() - (int)((median-edge)/delta*res) + pad);
		QPointF lowPoint = QPointF(0, size.height() - (int)((quartLow-edge)/delta*res) + pad);
		QPointF highPoint = QPointF(0, size.height() - (int)((quartHi-edge)/delta*res) + pad);

		painter.setPen(Qt::black);
		painter.drawLine(QPointF(hpad+35, bottomPoint.y()),	QPointF(hpad+65, bottomPoint.y()));

		painter.setPen(Qt::black);
		painter.drawLine(QPointF(hpad+35, topPoint.y()), QPointF(hpad+65, topPoint.y()));

		painter.setPen(Qt::black);
		painter.drawLine(QPointF(hpad+50, bottomPoint.y()),	QPointF(hpad+50, topPoint.y()));

		painter.setBrush(QColor(color,color,color));
		painter.drawRect(hpad+30, lowPoint.y(), 40, highPoint.y() - lowPoint.y());

		painter.setPen(Qt::black);
		painter.drawLine(QPointF(hpad+30, medPoint.y()),	QPointF(hpad+70, medPoint.y()));

		painter.setPen(Qt::black);
		char text[255];
		sprintf(text, "%.3f", median);
		painter.drawText(QPointF(hpad,medPoint.y()+6), QString(text));
		sprintf(text, "%.3f", top);
		painter.drawText(QPointF(hpad+36,topPoint.y()-6), QString(text));
		sprintf(text, "%.3f", bottom);
		painter.drawText(QPointF(hpad+36,bottomPoint.y()+12), QString(text));
	}

	return boxplot;
}

void MLDemos::ShowRoc()
{
	if(!classifier) return;
	SetROCInfo();
	actionShowStats->setChecked(true);
	showStats->tabWidget->setCurrentWidget(showStats->rocTab);
	ShowStatsDialog();
}

void MLDemos::ShowCross()
{
	if(!classifier && !regressor) return;
	SetCrossValidationInfo();
	actionShowStats->setChecked(true);
	showStats->tabWidget->setCurrentWidget(showStats->crossvalidTab);
	ShowStatsDialog();
}

void MLDemos::StatsChanged()
{
	int tab = showStats->tabWidget->currentIndex();
	switch(tab)
	{
	case 0:
		UpdateInfo();
		break;
	case 1:
		SetROCInfo();
		break;
	case 2:
		SetCrossValidationInfo();
		break;
	}
}

void MLDemos::SetROCInfo()
{
	if(!classifier || !bIsRocNew) return;
	QPixmap rocImage = RocImage(classifier->rocdata, classifier->roclabels, QSize(showStats->rocWidget->width(),showStats->rocWidget->height()));
	bIsRocNew = false;
//	rocImage.save("roc.png");
	rocWidget->ShowImage(rocImage);
}

void MLDemos::SetCrossValidationInfo()
{
	if(!bIsCrossNew) return;
	std::vector<fvec> fmeasures;
	if(classifier) fmeasures = classifier->crossval;
	else if(regressor) fmeasures = regressor->crossval;

	if(!fmeasures.size()) return;
	char txt[255];
	QString text;
	text += "Cross-Validation\n";
	float ratios [] = {.1f,.25f,1.f/3.f,.5f,2.f/3.f,.75f,.9f,1.f};
	int ratioIndex = classifier ? optionsClassify->traintestRatioCombo->currentIndex() : optionsRegress->traintestRatioCombo->currentIndex();
	float trainRatio = ratios[ratioIndex];
	if(classifier) sprintf(txt, "%d folds\n", optionsClassify->foldCountSpin->value());
	else sprintf(txt, "%d folds\n", optionsRegress->foldCountSpin->value());
	text += txt;
	sprintf(txt,"%d train, %d test samples", (int)(canvas->data->GetCount()*trainRatio), canvas->data->GetCount() - (int)(canvas->data->GetCount()*trainRatio));
	text += txt + QString("\n\n");
	text += classifier ? QString("Classification Performance:\n\n") : QString("Regression Error:\n\n");
	FOR(i, fmeasures.size())
	{
		fvec meanStd = MeanStd(fmeasures[i]);
		fvec quartiles = Quartiles(fmeasures[i]);
		text += !i ? "Training\n" : "Testing\n";
		sprintf(txt,"%.3f ± %.3f", meanStd[0], meanStd[1]);
		text += txt + QString(" (mean±std)\n");
		sprintf(txt,"%.3f %.3f %.3f %.3f %.3f", quartiles[0], quartiles[1], quartiles[2], quartiles[3], quartiles[4]);
		text += txt + QString(" (quartiles)\n");
		text += "\n\n";
	}
	showStats->crossvalidText->setText(text);
	QPixmap boxplot = BoxPlot(fmeasures, QSize(showStats->crossvalidWidget->width(),showStats->crossvalidWidget->height()));
//	boxplot.save("boxplot.png");
	bIsCrossNew = false;
	crossvalidWidget->ShowImage(boxplot);
}


void MLDemos::UpdateInfo()
{
	// dataset information
	int count = canvas->data->GetCount();
	int pcount = 0, ncount = 0;
	ivec labels = canvas->data->GetLabels();
	int posClass = optionsClassify->positiveSpin->value();
	FOR(i, labels.size())
	{
		if(labels[i] == posClass) ++pcount;
		else ++ncount;
	}

	// min/max, mean/variance
	vector<fvec> samples = canvas->data->GetSamples();
	fvec sMin,sMax,sMean,sSigma;
	sMin.resize(2,FLT_MAX);
	sMax.resize(2,-FLT_MAX);
	sMean.resize(2,0);
	sSigma.resize(4,0);
	if(samples.size())
	{
		FOR(i,samples.size())
		{
			sMin[0] = min(sMin[0],samples[i][0]);
			sMin[1] = min(sMin[1],samples[i][1]);
			sMax[0] = max(sMax[0],samples[i][0]);
			sMax[1] = max(sMax[1],samples[i][1]);
			sMean += samples[i];
		}
		sMean /= samples.size();
		FOR(i, samples.size())
		{
			sSigma[0] += (samples[i][0]-sMean[0])*(samples[i][0]-sMean[0]);
			sSigma[1] += (samples[i][0]-sMean[0])*(samples[i][1]-sMean[1]);
			sSigma[3] += (samples[i][1]-sMean[1])*(samples[i][1]-sMean[1]);
		}
		sSigma[0] = sqrtf(sSigma[0]/samples.size());
		sSigma[1] = sqrtf(sSigma[1]/samples.size());
		if(sSigma[1] != sSigma[1]) sSigma[1] = 0;
		sSigma[2] = sSigma[1];
		sSigma[3] = sqrtf(sSigma[3]/samples.size());
	}
	else
	{
		sMin.clear();sMax.clear();
		sMin.resize(2,0);
		sMax.resize(2,0);
	}

	QString information;
	information += "Current Dataset:\n";
	char string[255];
	sprintf(string, "    %d Samples\n    %d Positives\n    %d Negatives\n\n", count, pcount, ncount);
	information += QString(string);
	information +=    "       Min - Max          Mean  ,    Var\n";
	sprintf(string,   "    %.3f    %.3f      %.3f   ,   %.3f  %.3f\n", sMin[0], sMax[0], sMean[0], sSigma[0], sSigma[1]);
	sprintf(string, "%s    %.3f    %.3f      %.3f   ,   %.3f  %.3f\n", string, sMin[1], sMax[1], sMean[1], sSigma[2], sSigma[3]);
	information += string;

	if(classifier) information += "\nClassifier: " + QString(classifier->GetInfoString());
	if(regressor)  information += "\nRegressor: "  + QString(regressor->GetInfoString());
	if(clusterer)  information += "\nClusterer: "  + QString(clusterer->GetInfoString());

	showStats->infoText->setText(information);
}
