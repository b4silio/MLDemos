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
#include "classifierGMM.h"

using namespace std;

ClassifierGMM::ClassifierGMM()
: gmmPos(0), gmmNeg(0), dataPos(0), dataNeg(0), nbClusters(2), covarianceType(2), initType(1)
{
	type = CLASS_GMM;
	bSingleClass = false;
}


void ClassifierGMM::Train(std::vector< fvec > samples, ivec labels)
{
	if(!samples.size()) return;
	vector< fvec > positives, negatives;
	FOR(i, samples.size())
	{
		if(labels[i] == 1) positives.push_back(samples[i]);
		else negatives.push_back(samples[i]);
	}
	int dim = samples[0].size();
	DEL(gmmPos);
	DEL(gmmNeg);
        nbClusters = min(nbClusters, (u32)samples.size());
	gmmPos = new Gmm(nbClusters, dim);
	gmmNeg = new Gmm(nbClusters, dim);
	KILL(dataPos);
	KILL(dataNeg);
	dataPos = new float[positives.size()*dim];
	dataNeg = new float[negatives.size()*dim];
	FOR(i, positives.size())
	{
		FOR(j, dim) dataPos[i*dim + j] = positives[i][j];
	}
	FOR(i, negatives.size())
	{
		FOR(j, dim) dataNeg[i*dim + j] = negatives[i][j];
	}
	gmmPos->init(dataPos, positives.size(), initType);
	gmmNeg->init(dataNeg, negatives.size(), initType);
	gmmPos->em(dataPos, positives.size(), 1e-4, (COVARIANCE_TYPE)covarianceType);
	gmmNeg->em(dataNeg, negatives.size(), 1e-4, (COVARIANCE_TYPE)covarianceType);
	bFixedThreshold = false;
}

float ClassifierGMM::Test( const fvec &sample)
{
	if(!gmmPos || !gmmNeg) return 0.f;
	float pos = gmmPos->pdf((float *)&sample[0]);
	float neg = gmmNeg->pdf((float *)&sample[0]);
	return log(pos) - log(neg);
}

float ClassifierGMM::Test( const fVec &_sample)
{
	if(!gmmPos || !gmmNeg) return 0.f;
	fVec sample = _sample;
	float pos = gmmPos->pdf(sample._);
	float neg = gmmNeg->pdf(sample._);
	return log(pos) - log(neg);
}

void ClassifierGMM::SetParams(u32 nbClusters, u32 covarianceType, u32 initType)
{
	this->nbClusters = nbClusters;
	this->covarianceType = covarianceType;
	this->initType = initType;
}

char *ClassifierGMM::GetInfoString()
{
	char *text = new char[1024];
	sprintf(text, "GMM\n");
	sprintf(text, "%sMixture Components: %d\n", text, nbClusters);
	sprintf(text, "%sCovariance Type: ", text);
	switch(covarianceType)
	{
        case 2:
		sprintf(text, "%sSpherical\n", text);
		break;
	case 1:
		sprintf(text, "%sDiagonal\n", text);
		break;
        case 0:
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

void ClassifierGMM::Update()
{

}
