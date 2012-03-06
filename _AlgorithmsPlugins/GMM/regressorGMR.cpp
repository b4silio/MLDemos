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
#include "regressorGMR.h"

using namespace std;

void RegressorGMR::Train(std::vector< fvec > samples, ivec labels)
{
	if(!samples.size()) return;
	int dim = samples[0].size();

    if(outputDim != -1 && outputDim < dim-1)
    {
        // we need to swap the current last dimension with the desired output
        FOR(i, samples.size())
        {
            float val = samples[i][dim-1];
            samples[i][dim-1] = samples[i][outputDim];
            samples[i][outputDim] = val;
        }
    }

    DEL(gmm);
	nbClusters = min((int)nbClusters, (int)samples.size());
	gmm = new Gmm(nbClusters, dim);
	KILL(data);
	data = new float[samples.size()*dim];

    FOR(i, samples.size())
	{
        FOR(d, dim) data[i*dim + d] = samples[i][d];
	}

	gmm->init(data, samples.size(), initType);
	gmm->em(data, samples.size(), 1e-4, (COVARIANCE_TYPE)covarianceType);
	bFixedThreshold = false;
	gmm->initRegression(dim-1);
}

fvec RegressorGMR::Test( const fvec &sample)
{
    fvec res;
	res.resize(2,0);
	if(!gmm) return res;
	float estimate;
	float sigma;
    int dim = sample.size();
    if(outputDim != -1 && outputDim < dim-1)
    {
        // we need to swap the current last dimension with the desired output
        fvec newSample = sample;
        float val = newSample[dim-1];
        newSample[dim-1] = newSample[outputDim];
        newSample[outputDim] = val;
        gmm->doRegression(&newSample[0], &estimate, &sigma);
    }
    else
    {
        gmm->doRegression(&sample[0], &estimate, &sigma);
    }
	res[0] = estimate;
    res[1] = sqrt(sigma);
	return res;
}

fVec RegressorGMR::Test( const fVec &sample)
{
	fVec res;
	if(!gmm) return res;
	float estimate;
	float sigma;
	gmm->doRegression(sample._, &estimate, &sigma);
	res[0] = estimate;
	res[1] = sqrt(sigma);
	return res;
}

void RegressorGMR::SetParams(u32 nbClusters, u32 covarianceType, u32 initType)
{
	this->nbClusters = nbClusters;
	this->covarianceType = covarianceType;
	this->initType = initType;
}

const char *RegressorGMR::GetInfoString()
{
	char *text = new char[1024];
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
