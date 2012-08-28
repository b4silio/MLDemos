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
	IMKILL(display);
	Clear();
}

void SampleManager::CreateSampleImage(IplImage **image, bool bShowLabels, float ratio)
{
	int cnt = samples.size();

	int gridH = (int)(sqrtf(cnt/ratio) + 0.5f);
	int gridW = (cnt / gridH) + (cnt%gridH ? 1 : 0);

	CvSize imSize = cvSize(size.width*gridW, size.height*gridH);
	if (!(*image) || (*image)->width != imSize.width || (*image)->height != imSize.height)
	{
		if((*image)) cvReleaseImage(image);
		(*image) = cvCreateImage(imSize, 8, 3);
	}
	cvZero((*image));

	FOR(i, cnt)
	{
		if (!samples.at(i)) continue;
		CvRect rect = cvRect((i%gridW) * size.width, (i/gridW) * size.height, size.width, size.height);
		cvSetImageROI((*image), rect);
		cvCopy(samples.at(i), (*image));
		cvResetImageROI((*image));
		if(bShowLabels && flags[i] == TEST)
		{
			cvSetImageROI((*image), rect);
			IplImage *black = cvCreateImage(size, 8, 3);
			cvZero(black);
			cvAddWeighted((*image),0.5,black,1,0,(*image));
			IMKILL(black);
			cvResetImageROI((*image));
		}
		if(bShowLabels && labels[i])
		{
			cvRectangle((*image), cvPoint(rect.x, rect.y), cvPoint(rect.x+rect.width-1, rect.y+rect.height-1), CV::color[labels[i]%CV::colorCnt], 2, CV_AA); 
		}
	}
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
	cvNamedWindow(name);
	cvShowImage(name, display);
	cvSetMouseCallback(name, sm_on_mouse, (void *) &params);
	bShowing = true;
}

IplImage *SampleManager::GetSampleImage()
{
	IplImage *image = NULL;
	if(!GetCount()) return image;
	CreateSampleImage(&image, true);
	return image;
}


void SampleManager::Hide()
{
	char name[255];
	sprintf(name, "collected samples %d", ID);
	cvDestroyWindow(name);
	bShowing = false;
}

int SampleManager::GetIndexAt(int x, int y)
{
	if(display)
	{
		int gridX = (display->width / size.width);
		int gridY = (display->height / size.height);
		int i = (int)(x / (float)display->width * gridX);
		int j = (int)(y / (float)display->height * gridY);
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

	if(event == CV_EVENT_LBUTTONDOWN)
    {
	}
	else if(event == CV_EVENT_LBUTTONUP)
	{
		if(flags & CV_EVENT_FLAG_CTRLKEY)
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
		else if(flags & CV_EVENT_FLAG_ALTKEY)
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
				if (flags & CV_EVENT_FLAG_SHIFTKEY)
				{
					u32 newLabel = ((*labels)[index]+1) % 256;
					for (u32 i=index; i<labels->size(); i++) (*labels)[i] = newLabel;
				}
				else (*labels)[index] = ((*labels)[index]+1) % 256;
			}
		}
    }
	else if(event == CV_EVENT_RBUTTONUP)
	{
		if(index < labels->size())
		{
			if (flags & CV_EVENT_FLAG_SHIFTKEY)
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
	FOR(i, samples.size())
	{
		IMKILL(samples[i]);
	}
	samples.clear();
	flags.clear();
	labels.clear();
	KILL(perm);
	if(display) cvZero(display);
}

void SampleManager::AddSample(IplImage *image, unsigned int label)
{
	if (!image) return;

	IplImage *img = cvCreateImage(size, 8, 3);
	if(image->nChannels == 3) cvResize(image, img, CV_INTER_CUBIC);
	else
	{
		IplImage *tmp = cvCreateImage(cvGetSize(image), 8, 3);
		cvCvtColor(image, tmp, CV_GRAY2BGR);
		cvResize(tmp, img);
		IMKILL(tmp);
	}
	samples.push_back(img);
	flags.push_back(UNUSED);
	labels.push_back(label);
	KILL(perm);
	perm = randPerm(samples.size());
}

void SampleManager::AddSample(IplImage *image, CvRect selection, unsigned int label)
{
	if (!image) return;
	if (selection.x < 0 || selection.y < 0 || !selection.width || !selection.height) return;
	if (selection.x + selection.width > image->width || selection.y + selection.height > image->height) return;

	ROI(image, selection);
	IplImage *img = cvCreateImage(size, 8, 3);
	cvResize(image, img, CV_INTER_CUBIC);
	unROI(image);
	samples.push_back(img);
	flags.push_back(UNUSED);
	labels.push_back(label);
	KILL(perm);
	perm = randPerm(samples.size());
}


void SampleManager::AddSamples(std::vector<IplImage *>images)
{
	FOR(i, images.size())
	{
		if(images[i])
		{
			IplImage *sample = cvCreateImage(size, 8, 3);
			if(images[i]->width == size.width && images[i]->height == size.height)
			{
				if(images[i]->nChannels == 3) cvCopy(images[i], sample);
				else cvCvtColor(images[i], sample, CV_GRAY2BGR);
			}
			else
			{
				if(images[i]->nChannels == 3) cvResize(images[i], sample, CV_INTER_CUBIC);
				else
				{
					IplImage *tmp = cvCreateImage(size, 8, 1);
					cvResize(images[i], tmp, CV_INTER_CUBIC);
					cvCvtColor(tmp, sample, CV_GRAY2BGR);
					IMKILL(tmp);
				}
			}
			samples.push_back(sample);
			flags.push_back(UNUSED);
			labels.push_back(0);
		}
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
	IMKILL(samples[index]);
	for (unsigned int i = index; i < samples.size()-1; i++)
	{
		samples[i] = samples[i+1];
		labels[i] = labels[i+1];
	}
	samples.pop_back();
	labels.pop_back();
}

// we compare the current sample with all the ones in the dataset
// and return the smallest distance
f32 SampleManager::Compare(IplImage *sample)
{
	if(!sample) return 1.0f;
	IplImage *s = cvCreateImage(size, 8, 3);
	if(sample->width == size.width && sample->height == size.height)
	{
		if(sample->nChannels == 3) cvCopy(sample, s);
		else cvCvtColor(sample, s, CV_GRAY2BGR);
	}
	else if(sample->nChannels == s->nChannels)
	{
		cvResize(sample, s, CV_INTER_CUBIC);
	}
	else
	{
		IplImage *tmp = cvCreateImage(cvGetSize(sample), 8, 3);
		cvCvtColor(sample, tmp, CV_GRAY2BGR);
		cvResize(tmp, s, CV_INTER_CUBIC);
		IMKILL(tmp);
	}

	// now compute the differences
	f32 minDist = 1.0f;
	u32 index = 0;
	IplImage *diff = cvCloneImage(s);
	FOR(i, samples.size())
	{
		cvAbsDiff(s, samples[i], diff);
		f32 dist = (f32)cvSum(diff).val[0] / (f32)(size.width*size.height) / 255.f;
		if(minDist > dist)
		{
			index = i;
			minDist = dist;
		}
	}
	IMKILL(diff);
	IMKILL(s);
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


std::vector<IplImage *> SampleManager::GetSamples(u32 count, smFlags flag, smFlags replaceWith)
{
	std::vector<IplImage *> selected;
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

void SampleManager::Save(const char *filename)
{
	if(!samples.size()) return;
	IplImage *image = NULL;
	u32 sampleCnt = samples.size();

	IplImage *labelImg = cvCreateImage(size, 8, 3);
	u32 passes = 1 + (sampleCnt+2) / (size.width*size.height*3);
	u32 cnt = min(size.width*size.height*3, (int)sampleCnt);
	cvZero(labelImg); // we want at least one empty label
	samples.push_back(labelImg);
	FOR(i, passes)
	{
		cnt = min(size.width*size.height*3, (int)sampleCnt - (int)i*size.width*size.height*3);
		labelImg = cvCreateImage(size, 8, 3);
		cvZero(labelImg);
		FOR(j, cnt)
		{
			labelImg->imageData[j] = labels[i*(size.width*size.height*3) + j];
		}
		samples.push_back(labelImg);
	}

	CreateSampleImage(&image);
	
	// we write down the size of the samples in the last pixel of the image
    cvSet2D(image,image->height-1,image->width-1,CV_RGB(255, size.height, size.width));


	FOR(i, passes+1)
	{
		IMKILL(samples[samples.size()-1]);
		samples.pop_back();
	}

	cvSaveImage(filename, image);
	IMKILL(image);
}

bool SampleManager::Load(const char *filename, CvSize resolution)
{
	IplImage *image = cvLoadImage(filename);
	if(!image || image->width < resolution.width || image->height < resolution.height) return false;

	Clear();

	// we try to get the resolution off the image itself
	int last = (image->height-1)*image->widthStep + (image->width-1)*3;
	if(image->imageData[last] == -1) // we have the information!
	{
		size.width = image->imageData[last-2];
		size.height = image->imageData[last-1];
	}
	else size = resolution;

	int gridW = image->width / size.width;
	int gridH = image->height / size.height;
	int cnt = gridW*gridH;
	bool bDone = false;
	FOR(i, cnt)
	{
		IplImage *sample = cvCreateImage(size, 8, 3);
		ROI(image, cvRect((i%gridW) * size.width, (i/gridW) * size.height, size.width, size.height));
		cvCopy(image, sample);
		unROI(image);

		if(bDone)
		{
			if(labels.size() == samples.size()) // we added all the labels already
			{
				IMKILL(sample);
				break;
			}
			u32 labelCnt = min((u32)size.width*size.height*3, (u32)samples.size() - (u32)labels.size());
			FOR(j, labelCnt)
			{
				labels.push_back((u8)sample->imageData[j]);
			}
			IMKILL(sample);
			continue;
		}

		bool bZero = true;
		FOR(i, sample->imageSize)
		{
			if(sample->imageData[i] != 0)
			{
				bZero = false;
				break;
			}
		}
		if (bZero)
		{
			IMKILL(sample);
			bDone = true;
		}
		else
		{
			samples.push_back(sample);
			flags.push_back(UNUSED);
			//labels.push_back(0);
		}
	}
	while(labels.size() < samples.size()) labels.push_back(0);
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
