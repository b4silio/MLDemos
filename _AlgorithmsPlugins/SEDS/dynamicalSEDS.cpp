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
#include "dynamicalSEDS.h"
#include <QDebug>

using namespace std;

Gmm *DynamicalSEDS::globalGMM = 0;

DynamicalSEDS::DynamicalSEDS()
	: gmm(0),seds(0), data(0), nbClusters(2), penalty(1), bPrior(true), bMu(true), bSigma(true), objectiveType(1),
	  maxIteration(100), maxMinorIteration(2), constraintCriterion(0), resizeFactor(500.f)
{
#ifdef USEQT
    displayLabel = 0;
#endif

    type = DYN_SEDS;
	endpoint = fvec();
	endpoint.resize(4,0.f);
}

DynamicalSEDS::~DynamicalSEDS()
{
    if(gmm != globalGMM) DEL(gmm);
}

void DynamicalSEDS::Train(std::vector< std::vector<fvec> > trajectories, ivec labels)
{
	if(!trajectories.size()) return;
	int count = trajectories[0].size();
	if(!count) return;
	dim = trajectories[0][0].size();
	// we forget about time and just push in everything
	vector<fvec> samples;
	endpoint = trajectories[0][trajectories[0].size()-1];
	endpointFast = dim >= 2 ? fVec(endpoint[0], endpoint[1]) : fVec();
	FOR(d,dim/2) endpoint[d+dim/2] = 0;
	FOR(i, trajectories.size())
	{
		FOR(j, trajectories[i].size())
		{
			samples.push_back(trajectories[i][j] - endpoint);
		}
	}
	if(!samples.size()) return;

	bool bKeepInitialization = false;

	nbClusters = min((int)nbClusters, (int)samples.size());
	KILL(data);
	data = new float[samples.size()*dim];
	double *ddata = new REALTYPE[samples.size()*dim];
	FOR(i, samples.size())
	{
		FOR(j, dim) data[i*dim + j] = samples[i][j]*resizeFactor;
		FOR(j, dim) ddata[j*samples.size() + i] = samples[i][j]*resizeFactor;
	}

	if(globalGMM && bKeepInitialization && globalGMM->nstates == nbClusters)
	{
		gmm = globalGMM;
	}
	else
	{
		// first learn the model with gmm
		DEL(gmm);
		gmm = new Gmm(nbClusters, dim);
		gmm->init(data, samples.size(), 2); // kmeans initialization
		gmm->em(data, samples.size(), 1e-4, COVARIANCE_FULL);
		globalGMM = gmm;
	}

	//gmm->initRegression(dim/2);
	//return;

	/*
 // we write down the data (for debugging purposes)
 ofstream file;
 file.open("last-data.txt");
 if(file.is_open())
 {
  FOR(i, samples.size())
  {
   FOR(j, dim)
   {
	file << ddata[j*samples.size() + i] << " ";
   }
   file << std::endl;
  }
  file.close();
 }
 */
	// then optimize with seds
	DEL(seds);
	seds = new SEDS();

#ifdef USEQT
    seds->displayLabel = displayLabel;
#endif

	// fill in the data
	//seds->Data.Resize(dim, samples.size());
	seds->Data = Matrix(ddata, dim, samples.size());

	// fill in the current model
	seds->Priors.Resize(nbClusters);
	seds->Mu.Resize(dim, nbClusters);
	seds->Sigma = new Matrix[nbClusters];
	FOR(i, nbClusters)
	{
		seds->Sigma[i].Resize(dim,dim);
		seds->Priors(i) = gmm->c_gmm->gauss[i].prior;
		FOR(d, dim) seds->Mu(d, i) = gmm->c_gmm->gauss[i].mean[d];
		FOR(d1, dim)
		{
			FOR(d2, d1+1)
			{
				seds->Sigma[i](d2, d1) = seds->Sigma[i](d1, d2) = smat_get_value(gmm->c_gmm->gauss[i].covar, d2, d1);
			}
		}
	}
	seds->nData = samples.size();
	seds->d = dim/2;
	seds->K = nbClusters;
    seds->endpoint = endpoint;

	seds->Options.perior_opt = bPrior;
	seds->Options.mu_opt = bMu;
	seds->Options.sigma_x_opt = bSigma;
	seds->Options.max_iter = maxIteration;
	seds->Options.objective = objectiveType;
    //seds->Options.constraintCriterion = constraintCriterion;
    seds->Options.constraintCriterion = 0; // set to 0 until we actually fix this (broken otherwise)

    // MMA, ISRES, ORIG_DIRECT, AUGLAG, COBYLA
    switch(optimizationType)
    {
    case 0:
        seds->Options.optimizationType = nlopt::LD_MMA;
    break;
    case 1:
        seds->Options.optimizationType = nlopt::GN_ISRES;
    break;
    case 2:
        seds->Options.optimizationType = nlopt::GN_ORIG_DIRECT;
    break;
    case 3:
        seds->Options.optimizationType = nlopt::LN_AUGLAG;
    break;
    case 4:
        seds->Options.optimizationType = nlopt::LN_COBYLA;
    break;
    }

    seds->Optimize();

    // and we copy the values back to the source gmm
    float *mu = new float[dim];
	float *sigma = new float[dim*dim];
    FOR(i, nbClusters)
	{
		FOR(d, dim) mu[d] = seds->Mu(d, i);
		FOR(d1, dim)
		{
			FOR(d2, dim)
			{
				sigma[d2*dim + d1] = seds->Sigma[i](d1, d2);
			}
		}
		fgmm_set_prior(gmm->c_gmm, i, seds->Priors(i));
		fgmm_set_mean(gmm->c_gmm, i, mu);
		fgmm_set_covar(gmm->c_gmm, i, sigma);
	}
	delete [] sigma;
	delete [] mu;
	delete [] ddata;
	gmm->initRegression(dim/2);

	/*seds->Mu.Print();
 FOR(i, nbClusters) seds->Sigma[i].Print();
        */
	dim /= 2; // we prefer to have the actual dimensions of the data;
}

std::vector<fvec> DynamicalSEDS::Test( const fvec &sample, int count)
{
	dim = sample.size();
	fvec start = (sample - endpoint)*resizeFactor;
	std::vector<fvec> res;
	res.reserve(500);
	float *sigma = new float[dim*(dim+1)/2];
	if(!gmm) return res;
	fvec velocity; velocity.resize(dim,0);
	int cnt = 0;
	fvec diff;diff.resize(2,1);
	float minDiff = 1e-5;
	while(sqrtf(diff[0]*diff[0] + diff[1]*diff[1]) > minDiff)
	{
		res.push_back(start/resizeFactor + endpoint);
		start += velocity*dT;
		gmm->doRegression(&start[0], &velocity[0], sigma);
		if(cnt > 3) diff = res[cnt] - res[cnt-1];
		cnt++;
	}
	delete [] sigma;
	return res;
}

fvec DynamicalSEDS::Test( const fvec &sample)
{
	int dim = sample.size();
	if(!dim) return fvec(2,0);
	fvec res; res.resize(dim, 0);
	if(!gmm) return res;
	float *velocity = new float[dim];
	float *sigma = new float[dim*(dim+1)/2];
	fvec point; point.resize(2);
	point += (sample-endpoint)*resizeFactor;
	gmm->doRegression(&point[0], velocity, sigma);
	res[0] = velocity[0]/resizeFactor;
	res[1] = velocity[1]/resizeFactor;
	delete [] velocity;
	delete [] sigma;
	return res;
}

fVec DynamicalSEDS::Test( const fVec &sample)
{
	fVec res;
	if(!gmm) return res;
	fVec velocity;;
	float *sigma = new float[dim*(dim+1)/2];
	fVec point;
	point += (sample-endpointFast)*resizeFactor;
	gmm->doRegression(point._, velocity._, sigma);
	res = velocity/resizeFactor;
	delete [] sigma;
	return res;
}

void DynamicalSEDS::SetParams(int clusters, bool bPrior, bool bMu, bool bSigma, int objectiveType,
                              int maxIteration, int constraintCriterion, int optimizationType)
{
	this->nbClusters = clusters;
	this->bPrior = bPrior;
	this->bMu = bMu;
	this->bSigma = bSigma;
	this->objectiveType = objectiveType;
	this->maxIteration = maxIteration;
	this->constraintCriterion = constraintCriterion;
    this->optimizationType = optimizationType;
}

const char *DynamicalSEDS::GetInfoString()
{
	char *text = new char[2048];
	sprintf(text, "GMR\n");
	sprintf(text, "%sMixture Components: %d\n", text, nbClusters);
	sprintf(text, "%sObjective Function: ", text);
	switch(objectiveType)
	{
	case 0:
		sprintf(text, "%sMSE\n", text);
		break;
	case 1:
		sprintf(text, "%sLikelihood\n", text);
		break;
	}
	/*
        sprintf(text, "%Constraint Criterion: ", text);
        switch(constraintCriterion)
        {
        case 0:
                sprintf(text, "%sEigenvalue\n", text);
                break;
        case 1:
                sprintf(text, "%sPrincipal Minor\n", text);
                break;
        }*/
	return text;
}

void DynamicalSEDS::SaveModel(string filename)
{
    if(!seds) return;
    //qDebug() << "saving SEDS model" << filename.c_str();
    seds->saveModel(filename.c_str());
}

bool DynamicalSEDS::LoadModel(string filename)
{
    //qDebug() << "loading SEDS model" << filename.c_str();
    if(!seds)
    {
        seds = new SEDS();
    }
    seds->loadModel(filename.c_str());
    dim = seds->d*2;
    nbClusters = seds->K;
    endpoint = seds->endpoint;
    endpointFast = dim >= 2 ? fVec(endpoint[0], endpoint[1]) : fVec();

    gmm = new Gmm(nbClusters, dim);
    // and we copy the values back to the source
    float *mu = new float[dim];
    float *sigma = new float[dim*dim];
    FOR(i, nbClusters)
    {
        FOR(d, dim) mu[d] = seds->Mu(d, i);
        FOR(d1, dim)
        {
            FOR(d2, dim)
            {
                sigma[d2*dim + d1] = seds->Sigma[i](d1, d2);
            }
        }
        fgmm_set_prior(gmm->c_gmm, i, seds->Priors(i));
        fgmm_set_mean(gmm->c_gmm, i, mu);
        fgmm_set_covar(gmm->c_gmm, i, sigma);
    }
    delete [] sigma;
    delete [] mu;
    gmm->initRegression(dim/2);
    globalGMM = gmm;

#ifdef USEQT
    seds->displayLabel = displayLabel;
#endif
    return true;
}
