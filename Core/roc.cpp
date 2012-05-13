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
#include <QDebug>
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
        if(data[i].second != 1 && data[i].second != -1)
        {
            rocType = 1; // 0 1 2 ...
            break;
        }
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

std::pair<float,float> GetMicroMacroFMeasure(std::vector<f32pair> data)
{
    if(!data.size()) return std::make_pair(0.f,0.f);
    std::map<int,ivec> perClass; // count, tp, fp
    FOR(i, data.size())
    {
        int trueClass = data[i].second;
        int predicted = data[i].first;
        if(!perClass.count(trueClass)) perClass[trueClass] = ivec(3,0);
        perClass[trueClass][0]++;
        if(predicted == trueClass) perClass[trueClass][1]++; // tp
        else perClass[trueClass][2]++; // fp
    }
    int microTP = 0, microFP = 0, microCount=0;
    float macroFMeasure = 0.f;
    for(std::map<int,ivec>::iterator it = perClass.begin(); it != perClass.end(); it++)
    {
        int c = it->first;
        int count = it->second[0];
        int tp = it->second[1];
        int fp = it->second[2];
        float precision = tp/float(tp+fp);
        float recall = tp/float(count);
        macroFMeasure += 2*precision*recall/(precision+recall);
        microTP += tp;
        microFP += fp;
        microCount += count;
    }
    macroFMeasure /= perClass.size();
    float precision = microTP/float(microTP+microFP);
    float recall = microTP/float(microCount);
    float microFMeasure = 2*precision*recall/(precision+recall);
    return std::make_pair(microFMeasure, macroFMeasure);
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

