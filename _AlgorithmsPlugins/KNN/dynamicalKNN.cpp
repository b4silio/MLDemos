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
#include "dynamicalKNN.h"
using namespace std;

void DynamicalKNN::Train(std::vector< std::vector<fvec> > trajectories, ivec labels)
{
	if(!trajectories.size()) return;
	int count = trajectories[0].size();
	if(!count) return;
	dim = trajectories[0][0].size()/2;
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
	int sampleCount = samples.size();
	points.resize(sampleCount);
	velocities.resize(sampleCount);
	FOR(i, sampleCount)
	{
		points[i].resize(dim);
		velocities[i].resize(dim);
		FOR(d,dim)
		{
			points[i][d] = samples[i][d];
			velocities[i][d] = samples[i][dim+d];
		}
	}

	DEL(kdTree);
	annClose();
	ANN::MetricType = (ANN_METRIC)metricType;
	ANN::MetricPower = metricP;

	dataPts = annAllocPts(sampleCount, dim);			// allocate data points
	FOR(i, sampleCount)
	{
		FOR(d, dim) dataPts[i][d] = points[i][d];
	}
	kdTree = new ANNkd_tree(dataPts, sampleCount, dim);
}

DynamicalKNN::~DynamicalKNN()
{
	annClose();
	DEL(kdTree);
}
std::vector<fvec> DynamicalKNN::Test( const fvec &sample, const int count)
{
	fvec start = sample;
	dim = sample.size();
	std::vector<fvec> res;
	res.resize(count);
	FOR(i, count) res[i].resize(dim,0);
	if(!points.size()) return res;
	fvec velocity; velocity.resize(dim,0);
	FOR(i, count)
	{
		res[i] = start;
		start += velocity*dT;
		velocity = Test(start);
	}
	return res;
}

fvec DynamicalKNN::Test( const fvec &sample )
{
	fvec res;
	res.resize(2,0);
	int dim = sample.size();
	if(!points.size()) return res;
	double eps = 0; // error bound
	ANNpoint queryPt; // query point
	queryPt = annAllocPt(dim); // allocate query point
	ANNidxArray nnIdx = new ANNidx[k]; // allocate near neigh indices
	ANNdistArray dists = new ANNdist[k]; // allocate near neighbor dists
	FOR(i, dim) queryPt[i] = sample[i];
	kdTree->annkSearch(queryPt, k, nnIdx, dists, eps);

	float dsum = 0;
	vector<fvec> scores;
	scores.resize(k);
	FOR(i, k)
	{
		if(nnIdx[i] >= points.size()) continue;
		if(dists[i] == 0) dsum += 0;
		else dsum += 1./dists[i];
		scores[i].resize(dim);
		FOR(d,dim) scores[i][d] = velocities[nnIdx[i]][d];
	}
	FOR(i, k)
	{
		if(nnIdx[i] >= points.size()) continue;
		if(dists[i] == 0) continue;
		dists[i] = 1./(dists[i])/dsum;
	}

	fvec mean, stdev;
	mean.resize(dim,0);
	stdev.resize(dim,0);
	int cnt = 0;
	FOR(i, k)
	{
		if(nnIdx[i] >= points.size()) continue;
		//mean += scores[i] / (scores.size());
		mean += scores[i] * dists[i];
		cnt++;
	}
	FOR(i, k)
	{
		if(nnIdx[i] >= points.size()) continue;
		FOR(d,dim) stdev[d] += (scores[i] - mean)[d]*(scores[i] - mean)[d];
	}
	FOR(d,dim)
	{
		if(cnt) stdev[d] /= cnt;
		else stdev[d] = 0;
		stdev[d] = sqrtf(stdev[d]);
	}

	delete [] nnIdx; // clean things up
	delete [] dists;

	res = mean;
	//res[1] = stdev;

	return res;
}


fVec DynamicalKNN::Test( const fVec &sample )
{
	fVec res;
	int dim = 2;
	if(!points.size()) return res;
	double eps = 0; // error bound
	ANNpoint queryPt; // query point
	queryPt = annAllocPt(dim); // allocate query point
	ANNidxArray nnIdx = new ANNidx[k]; // allocate near neigh indices
	ANNdistArray dists = new ANNdist[k]; // allocate near neighbor dists
	FOR(i, dim) queryPt[i] = sample._[i];
	kdTree->annkSearch(queryPt, k, nnIdx, dists, eps);

	float dsum = 0;
	vector<fvec> scores;
	scores.resize(k);
	FOR(i, k)
	{
		if(nnIdx[i] >= points.size()) continue;
		if(dists[i] == 0) dsum += 0;
		else dsum += 1./dists[i];
		scores[i].resize(dim);
		FOR(d,dim) scores[i][d] = velocities[nnIdx[i]][d];
	}
	FOR(i, k)
	{
		if(nnIdx[i] >= points.size()) continue;
		if(dists[i] == 0) continue;
		dists[i] = 1./(dists[i])/dsum;
	}

	fVec mean, stdev;
	int cnt = 0;
	FOR(i, k)
	{
		if(nnIdx[i] >= points.size()) continue;
		//mean += scores[i] / (scores.size());
		mean += scores[i] * dists[i];
		cnt++;
	}
	FOR(i, k)
	{
		if(nnIdx[i] >= points.size()) continue;
		FOR(d,dim) stdev[d] += (scores[i] - mean)[d]*(scores[i] - mean)[d];
	}
	FOR(d,dim)
	{
		if(cnt) stdev[d] /= cnt;
		else stdev[d] = 0;
		stdev[d] = sqrtf(stdev[d]);
	}

	delete [] nnIdx; // clean things up
	delete [] dists;

	res = mean;
	//res[1] = stdev;

	return res;
}

void DynamicalKNN::SetParams( u32 k, int metricType, u32 metricP )
{
	this->k = k;
	switch(metricType)
	{
	case 0:
		this->metricType = ANN_METRIC1;
		this->metricP = 1;
		break;
	case 1:
		this->metricType = ANN_METRIC2;
		this->metricP = 2;
		break;
	case 2:
		this->metricType = ANN_METRICP;
		this->metricP = metricP;
		break;
	case 3:
		this->metricType = ANN_METRIC0;
		this->metricP = 0;
		break;
	}
}

const char *DynamicalKNN::GetInfoString()
{
	char *text = new char[1024];
	sprintf(text, "KNN\n");
	sprintf(text, "%sK: %d\n", text, k);
	sprintf(text, "%sMetric: ", text);
	switch(metricType)
	{
	case 0:
		sprintf(text, "%infinite norm\n", text);
		break;
	case 1:
		sprintf(text, "%s1-norm (Manhattan)\n", text);
		break;
	case 2:
		sprintf(text, "%s2-norm (Euclidean)\n", text);
		break;
	case 3:
		sprintf(text, "%s%d-norm\n", text, metricP);
		break;
	}
	return text;
}
