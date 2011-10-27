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

/*
void RocCurve(std::vector<f32pair> data)
{
	if(!data.size()) return;
	if (!rocfont) 
	{
		rocfont = new CvFont;
		cvInitFont(rocfont, CV_FONT_HERSHEY_PLAIN, 0.75, 0.75, 0, 1, CV_AA);
	}
	u32 res = RES;

	if(!roc) roc = cvCreateImage(cvSize(res,res), 8, 3);
	
	cvSet(roc, CV_RGB(40,40,40));

	rocCallBackAllData.clear();
	rocCallBackData.clear();

	std::sort(data.begin(), data.end(), UDLesser);

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
		cvVec2 val(fp/float(fp+tn), 1 - tp/float(tp+fn));
		float precision = tp / float(tp+fp);
		float recall = tp /float(tp+fn);
		float fmeasure = tp == 0 ? 0 : 2 * (precision * recall) / (precision + recall);
		cvDrawLine(roc, (oldVal*(float)res).to2d(), (val*(float)res).to2d(), CV_RGB(255,255,255));
		//draw_cross(roc, (val*(float)res).to2d(),CV_RGB(255,255,255), 2);

		std::vector<f32> dat;
		dat.push_back(val.x);
		dat.push_back(val.y);
		dat.push_back(data[i].first);
		dat.push_back(fmeasure);
		rocCallBackData.push_back(dat);

		oldVal = val;
	}
	rocCallBackAllData.push_back(rocCallBackData);
	rocIndex = rocCallBackAllData.size() - 1;
	cvDrawLine(roc, cvPoint(0,res), (oldVal*(float)res).to2d(), CV_RGB(255,255,255));
	cvNamedWindow("roc");
	cvShowImage("roc", roc);
	cvSetMouseCallback("roc", roc_on_mouse);
	roc_on_mouse(0,0,0,0,0);
}

void RocCurves(std::vector<std::vector<f32pair> > dataVector, std::vector<char *> legend)
{
	IplImage *roc = RocImage(dataVector, legend);
	cvNamedWindow("roc");
	cvShowImage("roc", roc);
	cvSetMouseCallback("roc", roc_on_mouse);
	roc_on_mouse(0,0,0,0,0);
}

IplImage *RocImage(std::vector<std::vector<f32pair> > dataVector, std::vector<char *> legend, CvSize resolution)
{
	if (!rocfont) 
	{
		rocfont = new CvFont;
		cvInitFont(rocfont, CV_FONT_HERSHEY_PLAIN, 1, 1, 0, 1, CV_AA);
	}

	//u32 res = RES;
	cvVec2 vpad(PAD,PAD);
	if(!roc || roc->width != resolution.width+2*PAD || roc->height != resolution.height + 2*PAD)
	{
		IMKILL(roc);
		roc = cvCreateImage(cvSize(resolution.width+2*PAD,resolution.height+2*PAD), 8, 3);
		IMKILL(rocBackup);
		rocBackup = cvCloneImage(roc);
	}
	cvSet(roc, CV_RGB(40,40,40));

	CvScalar colors [] = {
		cvScalarAll(255),
		cvScalarAll(225),
		cvScalarAll(205),
		cvScalarAll(185),
		cvScalarAll(165),
		cvScalarAll(150),
		cvScalarAll(135),
		cvScalarAll(120),
		cvScalarAll(105),
		cvScalarAll( 90),
		cvScalarAll( 75),
		cvScalarAll( 55),
	};
	int colorCnt = 12;

	rocCallBackAllData.clear();
	FOR(d, dataVector.size())
	{
		int minCol = 70;
		CvScalar color = (dataVector.size() == 1) ? cvScalarAll(255) : cvScalarAll((255 - minCol)*(dataVector.size() - d -1)/(dataVector.size()-1) + minCol);
		//CvScalar color = colors[d%colorCnt];

		std::vector<f32pair> data = dataVector[d];
		if(!data.size()) continue;
		std::sort(data.begin(), data.end(), UDLesser);

		rocCallBackData.clear();

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

			std::vector<f32> dat;
			dat.push_back(val.x);
			dat.push_back(val.y);
			dat.push_back(data[i].first);
			dat.push_back(fmeasure);
			rocCallBackData.push_back(dat);

			oldVal = val;
		}

		// we highlight the current roc curve
		cvVec2 pt1, pt2;
		FOR(i, rocCallBackData.size()-1)
		{
			pt1 = cvVec2(rocCallBackData[i][0]*resolution.width, rocCallBackData[i][1]*resolution.height);
			pt2 = cvVec2(rocCallBackData[i+1][0]*resolution.width, rocCallBackData[i+1][1]*resolution.height);
			cvDrawLine(roc, (pt1+vpad).to2d(), (pt2+vpad).to2d(), color, 2, CV_AA);
		}
		pt1 = cvVec2(0,resolution.width);
		cvDrawLine(roc, (pt1+vpad).to2d(), (pt2+vpad).to2d(), color, 2, CV_AA);

		rocCallBackAllData.push_back(rocCallBackData);
		rocIndex = rocCallBackAllData.size() - 1;
		cvDrawLine(roc, cvPoint(0 + PAD,resolution.height + PAD), cvPoint(oldVal.x*resolution.width+vpad.x,oldVal.y*resolution.height+vpad.y), color);
		if(d < legend.size())
		{
			CvPoint point = cvPoint(2*resolution.width/3,(d+4)*rocfont->line_type);
			//CvPoint point = cvPoint(res - strlen(legend[d])*8,(d+2)*rocfont->line_type);
			cvPutText(roc, legend[d], point, rocfont, color);
		}
	}
	cvCopy(roc, rocBackup);
	return roc;
}
*/

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
