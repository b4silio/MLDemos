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
#include "clustererGMM.h"

using namespace std;

ClustererGMM::~ClustererGMM()
{
    DEL(gmm);
}

void ClustererGMM::Train(std::vector< fvec > samples)
{
	if(!samples.size()) return;
	int dim = samples[0].size();
	DEL(gmm);
	gmm = new Gmm(nbClusters, dim);
	KILL(data);
	data = new float[samples.size()*dim];
	FOR(i, samples.size())
	{
		FOR(j, dim) data[i*dim + j] = samples[i][j];
	}
	gmm->init(data, samples.size(), initType);
	gmm->em(data, samples.size(),-1e4,(COVARIANCE_TYPE)covarianceType);
//	FOR(i, nbClusters) gmm->SetPrior(i, 1.f/nbClusters);
}

fvec ClustererGMM::Test( const fvec &sample)
{
	fvec res;
	res.resize(nbClusters,0);
	if(!gmm) return res;
	float estimate;
	float sigma;
	FOR(i, nbClusters) res[i] = gmm->pdf(&sample[0], i);
	float sum = 0;
	FOR(i, nbClusters) sum += res[i];
	if(sum > FLT_MIN*3) FOR(i, nbClusters) res[i] /= sum;
	return res;
}

fvec ClustererGMM::Test( const fVec &sample)
{
	fvec res;
	res.resize(nbClusters,0);
	if(!gmm) return res;
	float estimate;
	float sigma;
	FOR(i, nbClusters) res[i] = gmm->pdf(sample._, i);
	float sum = 0;
	FOR(i, nbClusters) sum += res[i];
	if(sum > FLT_MIN*3) FOR(i, nbClusters) res[i] /= sum;
	return res;
}


void ClustererGMM::SetParams(u32 nbClusters, u32 covarianceType, u32 initType)
{
	this->nbClusters = nbClusters;
	this->covarianceType = covarianceType;
	this->initType = initType;
}

const char *ClustererGMM::GetInfoString()
{
	char *text = new char[1024];
	sprintf(text, "GMM\n");
	sprintf(text, "%sClusters: %d\n", text, nbClusters);
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
