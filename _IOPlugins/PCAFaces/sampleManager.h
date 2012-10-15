/*********************************************************************
MLDemos: A User-Friendly visualization toolkit for machine learning
Copyright (C) 2010  Basilio Noris
Contact: mldemos@b4silio.com

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public License,
version 3 as published by the Free Software Foundation.

This library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free
Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*********************************************************************/
#ifndef _SAMPLE_MANAGER_H_
#define _SAMPLE_MANAGER_H_

#include <vector>
#include "basicOpenCV.h"

enum SampleManagerFlags
{
	UNUSED = 0x0000,
	TRAIN  = 0x0001,
	VALID  = 0x0010,
	TEST   = 0x0100
};
typedef SampleManagerFlags smFlags;

class SampleManager
{
protected:
	static u32 IDCount;

	u32 ID;

	CvSize size; // the samples size (resolution)

	std::vector<IplImage *> samples;

	std::vector<smFlags> flags;

	ivec labels;

	u32 *perm;

	IplImage *display;

	void CreateSampleImage(IplImage **image, bool bShowLabels = false, float ratio = 1.f);

	bool bShowing;

public:
	SampleManager(CvSize resolution = cvSize(48,48));

    virtual ~SampleManager();
	
	void Show();
	void Hide();
    bool IsShowing(){return bShowing;}
	void Clear();


	IplImage *GetSampleImage();
    IplImage *GetDisplay(){return display;}

	static u32 GetClassCount(ivec classes);
	float GetTestRatio();
	void Randomize(int seed=-1);
	void RandomTestSet(float ratio = 0.66f, bool bEnsureOnePerClass=true);

	void AddSample(IplImage *image, unsigned int label = 0);
	void AddSample(IplImage *image, CvRect selection, unsigned int label = 0);
	void AddSamples(std::vector<IplImage *>images);
	void AddSamples(SampleManager newSamples);
	void RemoveSample(unsigned int index);

	int GetIndexAt(int x, int y);
	f32 Compare(IplImage *sample);

    CvSize GetSize(){return size;}
    int GetCount(){return samples.size();}

    IplImage *GetSample(unsigned int index=0){return index<samples.size() ? samples[index] : NULL;}
    std::vector<IplImage *> GetSamples(){return samples;}
	std::vector<IplImage *> GetSamples(u32 count, smFlags flag=UNUSED, smFlags replaceWith=TRAIN);

	void ResetFlags();
    void SetFlag(unsigned int index, smFlags flag){if(index < flags.size()) flags[index] = flag;}
    smFlags GetFlag(unsigned int index){return index < flags.size() ? flags[index] : UNUSED;}
    std::vector<smFlags > GetFlags(){return flags;}
	std::vector<bool> GetFreeFlags();

    u32 GetLabel(unsigned int index){return index < labels.size() ? labels[index] : 0;}
    ivec GetLabels(){return labels;}
    void SetLabel(unsigned int index, unsigned int label){if(index<labels.size())labels[index] = label;}

	void Save(const char *filename);
	bool Load(const char *filename, CvSize resolution = cvSize(48,48));

    virtual void Config(IplImage *image, CvRect selection, IplImage * =0){AddSample(image, selection);}
};

#endif // _SAMPLE_MANAGER_H_
