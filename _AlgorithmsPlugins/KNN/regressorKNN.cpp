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
#include "regressorKNN.h"
using namespace std;

void RegressorKNN::Train( std::vector< fvec > samples, ivec labels )
{
	if(!samples.size()) return;
    dim = samples[0].size()-1;
	DEL(kdTree);
	annClose();
	ANN::MetricType = (ANN_METRIC)metricType;
	ANN::MetricPower = metricP;
	this->samples = samples;
	this->labels = labels;

	dataPts = annAllocPts(samples.size(), dim);			// allocate data points
	FOR(i, samples.size())
	{
		FOR(j, dim) dataPts[i][j] = samples[i][j];
        if(outputDim != -1 && outputDim < dim)
        {
            dataPts[i][outputDim] = samples[i][dim];
        }
	}
	kdTree = new ANNkd_tree(dataPts, samples.size(), dim);
}

RegressorKNN::~RegressorKNN()
{
	annClose();
	DEL(kdTree);
}

fvec RegressorKNN::Test( const fvec &sample )
{
	fvec res;
	res.resize(2,0);
	if(!samples.size()) return res;
	int dim = sample.size()-1;
    int oDim = outputDim == -1 || outputDim > dim ? dim : outputDim;
	double eps = 0; // error bound
    ANNpoint queryPt; // query point
	queryPt = annAllocPt(dim); // allocate query point
	ANNidxArray nnIdx = new ANNidx[k]; // allocate near neigh indices
	ANNdistArray dists = new ANNdist[k]; // allocate near neighbor dists
    FOR(i, dim) queryPt[i] = sample[i];
    if(outputDim != -1 && outputDim < dim)
    {
        queryPt[outputDim] = sample[dim];
    }
    if(k > samples.size()) k = samples.size();
	kdTree->annkSearch(queryPt, k, nnIdx, dists, eps);

    float dsum = 0;
	fvec scores;
        scores.resize(k,0);
        FOR(i, k)
	{
                if(nnIdx[i] >= samples.size()) continue;
                if(dists[i] == 0) dsum += 0;
                else dsum += 1./dists[i];
                scores[i] = samples[nnIdx[i]][oDim];
	}
        FOR(i, k)
        {
               if(nnIdx[i] >= samples.size()) continue;
               if(dists[i] == 0) continue;
               dists[i] = 1./(dists[i])/dsum;
        }

	float mean = 0, stdev = 0;
        int cnt = 0;
        FOR(i, k)
	{
                if(nnIdx[i] >= samples.size()) continue;
                //mean += scores[i] / (scores.size());
		mean += scores[i] * dists[i];
                cnt++;
	}
        FOR(i, k)
	{
                if(nnIdx[i] >= samples.size()) continue;
                stdev += (scores[i] - mean)*(scores[i] - mean);
	}
        if(cnt) stdev /= cnt;
        else stdev = 0;
	stdev = sqrtf(stdev);

	delete [] nnIdx; // clean things up
	delete [] dists;

	res[0] = mean;
	res[1] = stdev;

	return res;
}


fVec RegressorKNN::Test( const fVec &sample )
{
	fVec res;
	if(!samples.size()) return res;
	int dim = 1;
	double eps = 0; // error bound
	ANNpoint queryPt; // query point
	queryPt = annAllocPt(dim); // allocate query point
	ANNidxArray nnIdx = new ANNidx[k]; // allocate near neigh indices
	ANNdistArray dists = new ANNdist[k]; // allocate near neighbor dists
	FOR(i, dim) queryPt[i] = sample._[i];
	if(k > samples.size()) k = samples.size();
	kdTree->annkSearch(queryPt, k, nnIdx, dists, eps);

	float dsum = 0;
	fvec scores;
		scores.resize(k,0);
		FOR(i, k)
	{
				if(nnIdx[i] >= samples.size()) continue;
				if(dists[i] == 0) dsum += 0;
				else dsum += 1./dists[i];
				scores[i] = samples[nnIdx[i]][dim];
	}
		FOR(i, k)
		{
			   if(nnIdx[i] >= samples.size()) continue;
			   if(dists[i] == 0) continue;
			   dists[i] = 1./(dists[i])/dsum;
		}

	float mean = 0, stdev = 0;
		int cnt = 0;
		FOR(i, k)
	{
				if(nnIdx[i] >= samples.size()) continue;
				//mean += scores[i] / (scores.size());
		mean += scores[i] * dists[i];
				cnt++;
	}
		FOR(i, k)
	{
				if(nnIdx[i] >= samples.size()) continue;
				stdev += (scores[i] - mean)*(scores[i] - mean);
	}
		if(cnt) stdev /= cnt;
		else stdev = 0;
	stdev = sqrtf(stdev);

	delete [] nnIdx; // clean things up
	delete [] dists;

	res[0] = mean;
	res[1] = stdev;

	return res;
}

void RegressorKNN::SetParams( u32 k, int metricType, u32 metricP )
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

const char *RegressorKNN::GetInfoString()
{
	char *text = new char[1024];
	sprintf(text, "KNN\n");
	sprintf(text, "%sK: %d\n", text, k);
	sprintf(text, "%sMetric: ", text);
	switch(metricType)
	{
	case 0:
		sprintf(text, "%sinfinite norm\n", text);
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
