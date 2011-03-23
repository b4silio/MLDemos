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
#include "basicOpenCV.h"
#include "dynamicalGMR.h"

using namespace std;

void DynamicalGMR::Train(std::vector< std::vector<fvec> > trajectories, ivec labels)
{
	if(!trajectories.size()) return;
	int count = trajectories[0].size();
	if(!count) return;
	dim = trajectories[0][0].size();
	// we forget about time and just push in everything
	vector<fvec> samples;
	FOR(i, trajectories.size())
	{
		FOR(j, trajectories[i].size())
		{
			samples.push_back(trajectories[i][j]);
		}
	}
	if(!samples.size()) return;
	DEL(gmm);
	nbClusters = min((int)nbClusters, (int)samples.size());
	gmm = new Gmm(nbClusters, dim);
	KILL(data);
	data = new float[samples.size()*dim];
	FOR(i, samples.size())
	{
		FOR(j, dim) data[i*dim + j] = samples[i][j];
	}
	gmm->init(data, samples.size(), initType);
	gmm->em(data, samples.size(), 1e-4, (COVARIANCE_TYPE)covarianceType);
	gmm->initRegression(dim-2);
}

std::vector<fvec> DynamicalGMR::Test( const fvec &sample, int count)
{
	fvec start = sample;
	dim = sample.size();
	std::vector<fvec> res;
	res.resize(count);
	FOR(i, count) res[i].resize(dim,0);
	if(!gmm) return res;
	fvec velocity; velocity.resize(dim,0);
	float *sigma = new float[dim*(dim+1)/2];
	FOR(i, count)
	{
		res[i] = start;
		start += velocity*dT;
		gmm->doRegression(&start[0], &velocity[0], sigma);
	}
	delete [] sigma;
	return res;
}

fvec DynamicalGMR::Test( const fvec &sample)
{
	int dim = sample.size();
	fvec res; res.resize(dim, 0);
	if(!gmm) return res;
	float *velocity = new float[dim];
	float *sigma = new float[dim*(dim+1)/2];
	gmm->doRegression(&sample[0], velocity, sigma);
	res[0] = velocity[0];
	res[1] = velocity[1];
	delete [] velocity;
	delete [] sigma;
	return res;
}


void DynamicalGMR::SetParams(u32 nbClusters, u32 covarianceType, u32 initType)
{
	this->nbClusters = nbClusters;
	this->covarianceType = covarianceType;
	this->initType = initType;
}

char *DynamicalGMR::GetInfoString()
{
	char *text = new char[255];
	sprintf(text, "GMR\n");
	sprintf(text, "%sMixture Components: %d\n", text, nbClusters);
	sprintf(text, "%sCovariance Type: ", text);
	switch(covarianceType)
	{
	case 0:
		sprintf(text, "%sSpherical\n", text);
		break;
	case 1:
		sprintf(text, "%sDiagonal\n", text);
		break;
	case 2:
		sprintf(text, "%sFull\n", text);
		break;
	}
	sprintf(text, "%sInitialization Type: ", text);
	switch(initType)
	{
	case 0:
		sprintf(text, "%sRandom\n", text);
		break;
	case 1:
		sprintf(text, "%sUniform\n", text);
		break;
	case 2:
		sprintf(text, "%sK-Means\n", text);
		break;
	}
	return text;
}

void DynamicalGMR::Draw(IplImage *display)
{
	IplImage *density = cvCreateImage(cvSize(256,256), 8, 3);
	cvZero(density);
	// we draw a density map for the probability
	fvec sample;
	sample.resize(dim);
	for (int i=0; i < density->width; i++)
	{
		sample[0] = i/(float)density->width;
		for (int j=0; j< density->height; j++)
		{
			sample[1] = j/(float)density->height;
			fvec res = Test(sample)*10;
			//float val = gmm->pdf(sample);
			int hue = (int)(atan2(res[1], res[0]) / (2*M_PI) * 359);
			cvSet2D(density, j, i, CV_RGB(128+res[0]*128,128+res[1]*128,0));
		}
	}
	cvResize(density, display, CV_INTER_CUBIC);
	IMKILL(density);
}
