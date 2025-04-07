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
#include "basicOpenCV.h"
#include "sampleManager.h"
#include <QDebug>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>

using namespace std;

u32 SampleManager::IDCount = 0;

SampleManager::SampleManager(CvSize resolution)
	: size(resolution)
{
	ID = IDCount++;
	display = NULL;
	perm = NULL;
	bShowing = false;
}

SampleManager::~SampleManager()
{
	Clear();
}

void SampleManager::CreateSampleImage(cv::Mat *image, bool bShowLabels, float ratio)
{
	int cnt = samples.size();

	int gridH = (int)(sqrtf(cnt/ratio) + 0.5f);
	int gridW = (cnt / gridH) + (cnt%gridH ? 1 : 0);

	CvSize imSize = cvSize(size.width*gridW, size.height*gridH);
    cv::Mat img = cv::Mat::zeros(imSize.height, imSize.width, CV_8UC3);

    FOR(i, samples.size()) {
        if (samples.at(i).empty()) continue;
        cv::Rect rect((i%gridW) * size.width, (i/gridW) * size.height, size.width, size.height);
        samples.at(i).copyTo(img(rect));
        if(bShowLabels && flags[i] == TEST) {
            cv::Mat black = cv::Mat::zeros(size.height, size.width, CV_8UC3);
            cv::addWeighted(img(rect),0.5,black,1,0,img(rect));
		}
        if(bShowLabels && labels[i]) {
            cv::Scalar color = CV::color[labels[i]%CV::colorCnt];
#ifndef MACX
            color = cv::Scalar(color.val[2],color.val[1],color.val[0]);
#endif
            cv::rectangle(img, cv::Point(rect.x, rect.y), cv::Point(rect.x+rect.width-1, rect.y+rect.height-1), color, 2, cv::LINE_AA);
		}
	}
    *image = img;
}

void sm_on_mouse( int event, int x, int y, int flags, void* param );
int params[5];
void SampleManager::Show()
{
	int cnt = samples.size();
	if(!cnt) return;

	CreateSampleImage(&display, true);

	params[0] = (intptr_t)((void *)&display);
	params[1] = (intptr_t)((void *)&samples);
	params[2] = (intptr_t)((void *)&labels);
	params[3] = (intptr_t)((void *)&flags);
	params[4] = (intptr_t)((void *)&size);
	char name[255];
	sprintf(name, "collected samples %d", ID);
    cv::namedWindow(name);
    cv::imshow(name, display);
    cv::setMouseCallback(name, sm_on_mouse, (void *) &params);
	bShowing = true;
}

cv::Mat SampleManager::GetSampleImage()
{
    cv::Mat image;
	if(!GetCount()) return image;
    CreateSampleImage(&image, true);
    return image;
}


void SampleManager::Hide()
{
	char name[255];
	sprintf(name, "collected samples %d", ID);
    cv::destroyWindow(name);
	bShowing = false;
}

int SampleManager::GetIndexAt(int x, int y)
{
    if(!display.empty())
	{
        int gridX = (display.cols / size.width);
        int gridY = (display.rows / size.height);
        int i = (int)(x / (float)display.cols * gridX);
        int j = (int)(y / (float)display.rows * gridY);
		int index = j*gridX + i;
		return index;
	}
	else
	{
		float ratio = 1.f;
		int cnt = samples.size();
		int gridH = (int)(sqrtf(cnt/ratio) + 0.5f);
		int gridW = (cnt / gridH) + (cnt%gridH ? 1 : 0);
		CvSize imSize = cvSize(size.width*gridW, size.height*gridH);

		int gridX = (imSize.width / size.width);
		int gridY = (imSize.height / size.height);
		int i = (int)(x / (float)imSize.width * gridX);
		int j = (int)(y / (float)imSize.height * gridY);
		int index = j*gridW + i;
		return index;
	}
}

void sm_on_mouse( int event, int x, int y, int flags, void* param )
{
	IplImage *image = (*(IplImage **)(((int *)param)[0]));
	std::vector<IplImage *> *samples = (std::vector<IplImage *> *)(((int *)param)[1]);
	ivec *labels = (ivec *)(((int *)param)[2]);
	std::vector<smFlags> *smflags = (std::vector<smFlags> *)(((int *)param)[3]);
	CvSize size = (*(CvSize *)(((int *)param)[4]));
    if( !image )
        return;

    if( image->origin )
        y = image->height - y;

	x = x >= (0xffff>>1) ? -(0xffff - x + 1) : x;
	y = y >= (0xffff>>1) ? -(0xffff - y + 1) : y;

	if (x < 0) x = 0;
	if (y < 0) y = 0;

	unsigned int gridX = (image->width / size.width);
	unsigned int gridY = (image->height / size.height);
	unsigned int i = (int)(x / (float)image->width * gridX);
	unsigned int j = (int)(y / (float)image->height * gridY);
	unsigned int index = j*gridX + i;

    if(event == cv::EVENT_LBUTTONDOWN)
    {
	}
    else if(event == cv::EVENT_LBUTTONUP)
	{
        if(flags & cv::EVENT_FLAG_CTRLKEY)
		{
			if(samples->size() == 1)
			{
				IMKILL((*samples)[0]);
				samples->clear();
				labels->clear();
				cvZero(image);
			}
			else if(index < samples->size())
			{
				IMKILL((*samples)[index]);
				while(index < samples->size()-1)
				{
					(*samples)[index] = (*samples)[index+1];
					(*labels)[index] = (*labels)[index+1];
					index++;
				}
				samples->pop_back();
				labels->pop_back();
			}
		}
        else if(flags & cv::EVENT_FLAG_ALTKEY)
		{
			if(index < smflags->size())
			{
				if((*smflags)[index] == UNUSED)
				{
					(*smflags)[index] = TEST;
				}
				else if ((*smflags)[index] == TEST)
				{
					(*smflags)[index] = UNUSED;
				}
			}
		}
		else
		{
			if(index < labels->size())
			{
                if (flags & cv::EVENT_FLAG_SHIFTKEY)
				{
					u32 newLabel = ((*labels)[index]+1) % 256;
					for (u32 i=index; i<labels->size(); i++) (*labels)[i] = newLabel;
				}
				else (*labels)[index] = ((*labels)[index]+1) % 256;
			}
		}
    }
    else if(event == cv::EVENT_RBUTTONUP)
	{
		if(index < labels->size())
		{
            if (flags & cv::EVENT_FLAG_SHIFTKEY)
			{
				u32 newLabel = (*labels)[index] ? (*labels)[index]-1 : 255;
				for (u32 i=index; i<labels->size(); i++) (*labels)[i] = newLabel;
			}
			else (*labels)[index] = (*labels)[index] ? (*labels)[index]-1 : 255;
		}
	}
}


void SampleManager::Clear()
{
	samples.clear();
	flags.clear();
	labels.clear();
	KILL(perm);
    display = cv::Mat();
}

void SampleManager::AddSample(cv::Mat& image, unsigned int label)
{
    if (image.empty()) return;

    cv::Mat img(size.height, size.width, CV_8UC3);
    if(image.channels() == 3) cv::resize(image, img, cv::Size(img.cols, img.rows), cv::INTER_CUBIC);
	else
	{
        cv::Mat tmp(cv::Size(image.cols, image.rows), image.type());
        cv::cvtColor(image, tmp, cv::COLOR_GRAY2BGR);
        cv::resize(tmp, img, cv::Size(img.cols, img.rows));
	}
	samples.push_back(img);
	flags.push_back(UNUSED);
	labels.push_back(label);
	KILL(perm);
	perm = randPerm(samples.size());
}

void SampleManager::AddSample(cv::Mat& image, CvRect selection, unsigned int label)
{
    if (image.empty()) return;
	if (selection.x < 0 || selection.y < 0 || !selection.width || !selection.height) return;
    if (selection.x + selection.width > image.cols || selection.y + selection.height > image.rows) return;

    cv::Mat selectionImage = image(selection);
    cv::Mat img(size.height, size.width, CV_8UC3);
    cv::resize(selectionImage, img, cv::Size(img.cols, img.rows), cv::INTER_CUBIC);
	samples.push_back(img);
	flags.push_back(UNUSED);
	labels.push_back(label);
	KILL(perm);
	perm = randPerm(samples.size());
}

void SampleManager::AddSamples(std::vector<cv::Mat> images)
{
    FOR(i, images.size())
    {
        if(images[i].empty()) continue;

        cv::Mat sample(size.height, size.width, CV_8UC3);
        if(images[i].cols == size.width && images[i].rows == size.height)
        {
            if(images[i].channels() == 3) images[i].copyTo(sample);
            else cv::cvtColor(images[i], sample, cv::COLOR_GRAY2BGR);
        }
        else
        {
            if(images[i].channels() == 3) cv::resize(images[i], sample, cv::Size(sample.cols, sample.rows), cv::INTER_CUBIC);
            else
            {
                cv::Mat tmp(size.height, size.width, CV_8UC3);
                cv::resize(images[i], tmp, cv::Size(size.width, size.height), cv::INTER_CUBIC);
                cv::cvtColor(tmp, sample, cv::COLOR_GRAY2BGR);
            }
        }
        samples.push_back(sample);
        flags.push_back(UNUSED);
        labels.push_back(0);
    }
    KILL(perm);
    perm = randPerm(samples.size());
}

void SampleManager::AddSamples(SampleManager newSamples)
{
	FOR(i, newSamples.GetSamples().size())
	{
		samples.push_back(newSamples.GetSample(i));
		flags.push_back(newSamples.GetFlag(i));
		labels.push_back(newSamples.GetLabel(i));
	}
	KILL(perm);
	perm = randPerm(samples.size());
}

void SampleManager::RemoveSample(unsigned int index)
{
	if(index >= samples.size()) return;
	if(samples.size() == 1)
	{
		Clear();
		return;
	}
    samples.erase(samples.begin() + index);
    labels.erase(labels.begin() + index);
    flags.erase(flags.begin() + index);
}

// we compare the current sample with all the ones in the dataset
// and return the smallest distance
f32 SampleManager::Compare(cv::Mat sample)
{
    if(sample.empty()) return 1.0f;
    cv::Mat s(size.height, size.width, CV_8UC3);
    if(sample.cols == size.width && sample.rows == size.height) {
        if(sample.channels() == 3) sample.copyTo(s);
        else cv::cvtColor(sample, s, cv::COLOR_GRAY2BGR);
    } else if(sample.channels() == s.channels()) {
        cv::resize(sample, s, cv::Size(s.cols, s.rows), cv::INTER_CUBIC);
    } else {
        cv::Mat tmp(sample.rows, sample.cols, CV_8UC3);
        cv::cvtColor(sample, tmp, cv::COLOR_GRAY2BGR);
        cv::resize(tmp, s, cv::Size(s.cols, s.rows), cv::INTER_CUBIC);
	}

	// now compute the differences
	f32 minDist = 1.0f;
    cv::Mat diff = s.clone();
	FOR(i, samples.size())
	{
        cv::absdiff(s, samples[i], diff);
        f32 dist = (f32)cv::sum(diff).val[0] / (f32)(size.width*size.height) / 255.f;
		if(minDist > dist)
		{
			minDist = dist;
		}
	}
	return minDist;
}

void SampleManager::Randomize(int seed)
{
	KILL(perm);
	if(samples.size()) perm = randPerm(samples.size(), seed);
}

void SampleManager::ResetFlags()
{
	FOR(i, samples.size()) flags[i] = UNUSED;
}


std::vector<cv::Mat> SampleManager::GetSamples(u32 count, smFlags flag, smFlags replaceWith)
{
    std::vector<cv::Mat> selected;
	if (!samples.size() || !perm) return selected;

	if (!count)
	{
		FOR(i, samples.size())
		{
			if ( flags[perm[i]] == flag)
			{
				selected.push_back(samples[perm[i]]);
				flags[perm[i]] = replaceWith;
			}
		}
		return selected;
	}

	for ( u32 i=0, cnt=0; i < samples.size() && cnt < count; i++ )
	{
		if ( flags[perm[i]] == flag )
		{
			selected.push_back(samples[perm[i]]);
			flags[perm[i]] = replaceWith;
			cnt++;
		}
	}

    return selected;
}

std::vector<cv::Mat> SampleManager::GetSampleMats()
{
    std::vector<cv::Mat> mats;
    FOR(i, samples.size()) {
        mats.push_back(samples.at(i));
    }
    return mats;
}

void SampleManager::Save(const char *filename)
{
	if(!samples.size()) return;
    cv::Mat image;
	u32 sampleCnt = samples.size();

    cv::Mat labelImg = cv::Mat::zeros(size, CV_8UC3);
    u32 passes = 1 + ((sampleCnt+2) / (size.width*size.height*3));
	samples.push_back(labelImg);
    flags.push_back(UNUSED);
	FOR(i, passes)
	{
        u32 cnt = min(size.width*size.height*3, (int)sampleCnt - (int)i*size.width*size.height*3);
        labelImg = cv::Mat::zeros(size, CV_8UC3);
		FOR(j, cnt)
		{
            labelImg.data[j] = labels[i*(size.width*size.height*3) + j];
		}
		samples.push_back(labelImg);
        flags.push_back(UNUSED);
    }

	CreateSampleImage(&image);
	
	// we write down the size of the samples in the last pixel of the image
    image.at<cv::Vec3b>(image.rows-1, image.cols-1) = cv::Vec3b(255, size.height, size.width);
    FOR(i, passes+1) samples.pop_back();
    cv::imwrite(filename, image);
}

bool SampleManager::Load(const char *filename, CvSize resolution)
{
    cv::Mat image = cv::imread(filename);
    if(image.empty() || image.cols < resolution.width || image.rows < resolution.height) return false;

	Clear();

	// we try to get the resolution off the image itself
    int last = (image.rows-1)*image.step + (image.cols-1)*3;
    if((s8)image.data[last] == -1) { // we have the information!
        size.width = image.data[last-2];
        size.height = image.data[last-1];
    } else size = resolution;

    int gridW = image.cols / size.width;
    int gridH = image.rows / size.height;
	int cnt = gridW*gridH;
	bool bDone = false;
	FOR(i, cnt)
	{
        cv::Mat sample(size, CV_8UC3);
        cv::Rect rect((i%gridW) * size.width, (i/gridW) * size.height, size.width, size.height);
        image(rect).copyTo(sample);

		if(bDone)
		{
            if(labels.size() == samples.size()) break; // we added all the labels already
            u32 labelCnt = min((u32)size.width*size.height*3, (u32)samples.size() - (u32)labels.size());
			FOR(j, labelCnt)
			{
                labels.push_back((u8)sample.data[j]);
			}
			continue;
		}

		bool bZero = true;
        FOR(i, sample.cols*sample.rows*sample.channels()) {
            if(sample.data[i] != 0) {
				bZero = false;
				break;
			}
		}
        if (bZero) {
			bDone = true;
        } else {
			samples.push_back(sample);
			flags.push_back(UNUSED);
			//labels.push_back(0);
		}
	}
    while(labels.size() < samples.size()) {
        labels.push_back(0);
        flags.push_back(UNUSED);
    }
	KILL(perm);
	perm = randPerm(samples.size());
	return samples.size() > 0;
}

u32 SampleManager::GetClassCount(ivec classes)
{
	u32 *counts = new u32[256];
	memset(counts, 0, 256*sizeof(u32));
	FOR(i, classes.size()) counts[classes[i]]++;
	u32 result = 0;
	for (u32 i=1; i<256; i++) result += counts[i] > 0 ? 1 : 0;
	return result;
}

std::vector<bool> SampleManager::GetFreeFlags()
{
	std::vector<bool> res;
	FOR(i, flags.size()) res.push_back(flags[i] == UNUSED);
	return res;
}

float SampleManager::GetTestRatio()
{
	float ratio = 0;
	FOR(i, flags.size())
	{
		ratio += flags[i] == TEST ? 1 : 0;
	}
	return ratio /= flags.size();
}

void SampleManager::RandomTestSet(float ratio, bool bEnsureOnePerClass)
{
	float minSamples = 5;
	// we want to have at least minSamples samples in order not to get weird results
	ratio = min(ratio, (GetCount()-minSamples)/GetCount());
	u32 *perm = randPerm(GetCount());
	FOR(i, GetCount())
	{
		SetFlag(perm[i], i < GetCount()*ratio ? TEST : UNUSED);
	}
	delete [] perm;

	if(!bEnsureOnePerClass) return;
	// we count how many samples of the positive and negative classes we have
	vector< pair<u32, u32> > counts;
	FOR(i, GetCount())
	{
		u32 label = GetLabel(i);
		bool bExists = false;
		u32 index = 0;
		FOR(j, counts.size())
		{
			if(label == counts[j].first)
			{
				index = j;
				bExists = true;
				break;
			}
		}
		if(!bExists)
		{
			counts.push_back(pair<u32,u32>(label, 0));
		}
		else
		{
			if(GetFlag(i) == UNUSED)
				counts[index].second++;
		}
	}
	// if we don't have any, we set at least one sample for ever class
	perm = randPerm(GetCount());
	FOR(j, counts.size())
	{
		if(counts[j].second) continue;
		FOR(i, GetCount())
		{
			if(GetLabel(perm[i]) == counts[j].first)
			{
				SetFlag(perm[i], UNUSED);
				break;
			}
		}
	}
	delete [] perm;
}
