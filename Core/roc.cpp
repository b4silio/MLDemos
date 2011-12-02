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
#include "public.h"
#include "basicMath.h"
#include "mymaths.h"
#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include "roc.h"

#define RES 512
#define PAD 16

std::vector<std::vector<std::vector<f32> > > rocCallBackAllData;
std::vector<std::vector<f32> > rocCallBackData;
u32 rocIndex = 0;

/*
CvFont *rocfont;
IplImage *roc = cvCreateImage(cvSize(RES,RES),8,3);
IplImage *rocBackup = cvCloneImage(roc);
*/

bool UDLesser(f32pair e1, f32pair e2)
{
	return e1.first < e2.first;
}

std::vector<float> GetBestFMeasures()
{
	std::vector<float> fmeasures;
	FOR(i, rocCallBackAllData.size())
	{
		float fmax = 0;
		FOR(j, rocCallBackAllData[i].size())
		{
			if(rocCallBackAllData[i][j].size() <4) continue;
			if(rocCallBackAllData[i][j][3] > fmax)
			{
				fmax = rocCallBackAllData[i][j][3];
			}
		}
		fmeasures.push_back(fmax);
	}
	return fmeasures;
}

void SaveRoc(std::vector<f32pair> data, const char *filename)
{
	std::sort(data.begin(), data.end(), UDLesser);
	std::fstream file(filename, std::ios::out|std::ios::binary);
	u32 size = data.size();
	file.write(reinterpret_cast<char *>(&size), sizeof(u32));
	FOR(i, data.size())
	{
		file.write(reinterpret_cast<char *>(&data[i]), sizeof(f32pair));
	}
	file.close();
}

std::vector<f32pair> LoadRoc(const char *filename)
{
	std::vector<f32pair> data;

	std::fstream file(filename, std::ios::in|std::ios::binary);
	u32 size;
	file.read(reinterpret_cast<char *>(&size), sizeof(u32));
	FOR(i, size)
	{
		f32pair pair;
		file.read(reinterpret_cast<char *>(&pair), sizeof(f32pair));
		data.push_back(pair);
	}
	file.close();
	return data;
}

// fixrocdata takes as input binary roc data and decides whether the two classes should be swapped
// this should be done when the binary classifier is performing worse than random
std::vector<f32pair> FixRocData(std::vector<f32pair> data)
{
    if(!data.size()) return data;
    std::vector<f32pair> invData = data;
    int rocType = 0; // -1 or 1
    FOR(i, data.size())
    {
        if(data[i].second != 1 && data[i].second != -1) rocType = 1; // 0 1 2 ...
    }
    if(rocType == 0)
    {
        FOR(i, data.size()) invData[i].second = -invData[i].second;
    }
    else
    {
        FOR(i, data.size()) invData[i].second = 1 - invData[i].second;
    }

    float fData = GetBestFMeasure(data)[0];
    float fInvData = GetBestFMeasure(invData)[0];
    return (fData >= fInvData) ? data : invData;
}

float GetBestThreshold( std::vector<f32pair> data )
{
	if(!data.size()) return 0;

	std::vector< std::vector<f32> > measures;

	std::sort(data.begin(), data.end(), UDLesser);

	fVec oldVal(1,0);
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
		fVec val(fp/float(fp+tn), 1 - tp/float(tp+fn));
		float precision = tp / float(tp+fp);
		float recall = tp /float(tp+fn);
		float fmeasure = tp == 0 ? 0 : 2 * (precision * recall) / (precision + recall);

		std::vector<f32> dat;
		dat.push_back(val.x);
		dat.push_back(val.y);
		dat.push_back(thresh);
		dat.push_back(fmeasure);
		measures.push_back(dat);

		oldVal = val;
	}

	float tmax = 0;
	float fmax = 0;
	FOR(j, measures.size())
	{
		if(measures[j][3] > fmax)
		{
			tmax = measures[j][2];
			fmax = measures[j][3];
		}
	}
	return tmax;
}

fvec GetBestFMeasure( std::vector<f32pair> data )
{
    if(!data.size()) return fvec(1,0);

	std::vector< std::vector<f32> > measures;

	std::sort(data.begin(), data.end(), UDLesser);

	fVec oldVal(1,0);
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
		fVec val(fp/float(fp+tn), 1 - tp/float(tp+fn));
		float precision = tp / float(tp+fp);
		float recall = tp /float(tp+fn);
		float fmeasure = tp == 0 ? 0 : 2 * (precision * recall) / (precision + recall);

		std::vector<f32> dat;
		dat.push_back(val.x);
		dat.push_back(val.y);
		dat.push_back(thresh);
		dat.push_back(fmeasure);
        dat.push_back(precision);
        dat.push_back(recall);
        measures.push_back(dat);

		oldVal = val;
	}

	float tmax = 0;
	float fmax = 0;
    float pmax = 0;
    float rmax = 0;
    FOR(j, measures.size())
	{
		if(measures[j][3] > fmax)
		{
			tmax = measures[j][2];
			fmax = measures[j][3];
            pmax = measures[j][4];
            rmax = measures[j][5];
        }
	}
    fvec res(3);
    res[0] = fmax;
    res[1] = pmax;
    res[2] = rmax;
    return res;
}


float GetAveragePrecision( std::vector<f32pair> data )
{
	if(!data.size()) return 0;

	std::vector< std::vector<f32> > measures;

	std::sort(data.begin(), data.end(), UDLesser);

	float averagePrecision = 0, oldRecall = 1;
	float m = 0;
	FOR(i, data.size()) m += data[i].second;
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
		float t = float(tp+fp);
		float precision = tp / float(tp+fp);
		float recall = tp / float(tp+fn);
		//averagePrecision += precision/t;
		averagePrecision += precision*(oldRecall-recall);
		oldRecall = recall;
	}
//	averagePrecision /= m
	return averagePrecision;
}

float GetRocValueAt(std::vector<f32pair> data, float threshold)
{
	if(!data.size()) return 0;

	u32 tp = 0, fp = 0;
	u32 fn = 0, tn = 0;
	FOR(j, data.size())
	{
		if(data[j].second == 1)
		{
			if(data[j].first >= threshold) tp++;
			else fn++;
		}
		else 
		{
			if(data[j].first >= threshold) fp++;
			else tn++;
		}
	}
	fVec val(fp/float(fp+tn), 1 - tp/float(tp+fn));
	float precision = tp / float(tp+fp);
	float recall = tp /float(tp+fn);
	float fmeasure = tp == 0 ? 0 : 2 * (precision * recall) / (precision + recall);

	return fmeasure;
}

/*
void SaveRocImage(const char *filename)
{
	cvSaveImage(filename, roc);
}
*/

/****************************************************/
/*     mouse listener (for filter configuration)    */
/****************************************************/
/*
void roc_on_mouse( int event, int x, int y, int flags, void* param )
{
	if(!roc || !rocCallBackAllData.size()) return;
	int w = roc->width-2*PAD;
	int h = roc->height-2*PAD;
	cvVec2 mouse((x-PAD)/(float)w,(y-PAD)/(float)h);
	cvVec2 vpad(PAD,PAD);

	rocCallBackData = rocCallBackAllData[rocIndex];

	if(!rocCallBackData.size())return;

	cvCopy(rocBackup, roc);


	// we highlight the current roc curve
	cvVec2 pt1, pt2;
	FOR(i, rocCallBackData.size()-1)
	{
		pt1 = cvVec2(rocCallBackData[i][0]*w, rocCallBackData[i][1]*h);
		pt2 = cvVec2(rocCallBackData[i+1][0]*w, rocCallBackData[i+1][1]*h);
		cvDrawLine(roc, (pt1+vpad).to2d(), (pt2+vpad).to2d(), CV_RGB(255,0,0), 2, CV_AA);
	}
	pt1 = cvVec2(0,h);
	cvDrawLine(roc, (pt1+vpad).to2d(), (pt2+vpad).to2d(), CV_RGB(255,0,0), 2, CV_AA);

	// we find the closest point in our curve
	u32 minInd = 0;
	float minDist = 2;
	FOR(i, rocCallBackData.size())
	{
		cvVec2 p(rocCallBackData[i][0],rocCallBackData[i][1]);
		if((p-mouse).length() < minDist)
		{
			minDist = (p-mouse).length();
			minInd = i;
		}
	}

	CvPoint rocPoint = cvPoint((u32)(rocCallBackData[minInd][0]*w+PAD), (u32)(rocCallBackData[minInd][1]*h+PAD));

	char text[255];
	sprintf(text,"thresh: %.3f TP: %.2f FP: %.2f",rocCallBackData[minInd][2],1-rocCallBackData[minInd][1],rocCallBackData[minInd][0]);
	CvPoint pt = cvPoint(PAD + w - strlen(text)*7, h - PAD - rocfont->line_type);
	cvPutText(roc, text, pt, rocfont, CV_RGB(255,255,255));
	if(rocCallBackData[minInd].size() >= 4)
	{
		sprintf(text,"f-measure: %.3f",rocCallBackData[minInd][3]);
		pt.y += rocfont->line_type;
		cvPutText(roc, text, pt, rocfont, CV_RGB(255,255,255));
	}

	cvDrawCircle(roc, rocPoint, 3, CV_RGB(255,255,255), 1, CV_AA);


	if(rocCallBackAllData.size() > 1)
	{
		pt = cvPoint(w*2/3, (rocIndex+4)*rocfont->line_type + 4);
		CvPoint pt2 = cvPoint(w, (rocIndex+4)*rocfont->line_type + 4);
		cvDrawLine(roc, pt, pt2, CV_RGB(255,0,0));
	}

	//cvShowImage("roc", roc);

	switch( event )
    {
    case CV_EVENT_LBUTTONUP:
		rocIndex++;
		rocIndex %= rocCallBackAllData.size();
        break;
	case CV_EVENT_RBUTTONUP:
		rocIndex = rocIndex ? rocIndex-1 : rocCallBackAllData.size()-1;
		rocIndex %= rocCallBackAllData.size();
        break;
    }
}

IplImage *GetRocImage()
{
	return roc;
}
*/
