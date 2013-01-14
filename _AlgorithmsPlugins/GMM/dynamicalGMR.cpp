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
#include "dynamicalGMR.h"
#include <iostream>
#include <fstream>
#include <QDebug>

using namespace std;

void DynamicalGMR::Train(std::vector< std::vector<fvec> > trajectories, ivec labels)
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
	DEL(gmm);
	nbClusters = min((int)nbClusters, (int)samples.size());
	gmm = new Gmm(nbClusters, dim*2);
	KILL(data);
	data = new float[samples.size()*dim*2];
	FOR(i, samples.size())
	{
		FOR(j, dim*2) data[i*dim*2 + j] = samples[i][j];
	}
	gmm->init(data, samples.size(), initType);
	gmm->em(data, samples.size(), 1e-4, (COVARIANCE_TYPE)covarianceType);
	gmm->initRegression(dim);
}

std::vector<fvec> DynamicalGMR::Test( const fvec &sample, int count)
{
	fvec start = sample;
	dim = sample.size();
    std::vector<fvec> res(count);
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
	dim = sample.size();
	fvec res; res.resize(dim, 0);
	if(!gmm) return res;
	float *velocity = new float[dim];
	float *sigma = new float[dim*(dim+1)/2];
	gmm->doRegression(&sample[0], velocity, sigma);
    FOR(d, dim) res[d] = velocity[d];
	delete [] velocity;
	delete [] sigma;
	return res;
}


fVec DynamicalGMR::Test( const fVec &sample)
{
	fVec res;
	if(!gmm) return res;
	fVec velocity;
	float *sigma = new float[(dim*2)*((dim*2)+1)/2];
	gmm->doRegression(sample._, velocity._, sigma);
	res = velocity;
	delete [] sigma;
	return res;
}

void DynamicalGMR::SetParams(u32 nbClusters, u32 covarianceType, u32 initType)
{
	this->nbClusters = nbClusters;
	this->covarianceType = covarianceType;
	this->initType = initType;
}

const char *DynamicalGMR::GetInfoString()
{
	char *text = new char[1024];
	sprintf(text, "GMR\n");
	sprintf(text, "%sMixture Components: %d\n", text, nbClusters);
	sprintf(text, "%sCovariance Type: ", text);
	switch(covarianceType)
	{
        case 0:
                sprintf(text, "%sFull\n", text);
                break;
	case 1:
		sprintf(text, "%sDiagonal\n", text);
		break;
        case 2:
                sprintf(text, "%sSpherical\n", text);
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


void DynamicalGMR::SaveModel(std::string filename)
{
    std::cout << "saving GMM model";
    if(!gmm)
    {
        std::cout << "Error: Nothing to save!" << std::endl;
        return; // nothing to save!
    }

    // Save the dataset to a file
    std::ofstream file(filename.c_str());

    if(!file){
        std::cout << "Error: Could not open the file!" << std::endl;
        return;
    }

    int dim = gmm->dim;
    file << gmm->dim << endl; //dimension
    file << gmm->nstates << endl; //number of Gaussian
    file << gmm->ninput << endl; //number of regression inputs (usually dim-1)

    file.precision(10); //setting the precision of writing


    FOR(i, gmm->nstates)
    {
        float prior = fgmm_get_prior(gmm->c_gmm, i);
        file << prior << " ";
    }
    file << endl;
    FOR(i, gmm->nstates)
    {
        float *mu = fgmm_get_mean(gmm->c_gmm, i);
        FOR(d, dim)
        {
            file << mu[d] << " ";
        }
        file << endl;
    }
    float *sigma = new float[dim*dim];
    FOR(i, gmm->nstates)
    {
        fgmm_get_covar(gmm->c_gmm, i, sigma);
        FOR(d, dim*dim)
        {
            file << sigma[d] << " ";
        }
        file << endl;
    }
    KILL(sigma);

    file.close();
}

bool DynamicalGMR::LoadModel(std::string filename)
{
    std::cout << "loading GMM model: " << filename;

    std::ifstream file(filename.c_str());

    if(!file.is_open()){
        std::cout << "Error: Could not open the file!" << std::endl;
        return false;
    }

    int dim, nstates, ninput;
    file >> dim >> nstates >> ninput;
    nbClusters = nstates;
    if(gmm) DEL(gmm);
    gmm = new Gmm(nstates, dim);
    FOR(i, nstates)
    {
        float prior;
        file >> prior;
        gmm->setPrior(i, prior);
    }

    float *mu = new float[dim];
    FOR(i, nstates)
    {
        FOR(d, dim) file >> mu[d];
        gmm->setMean(i, mu);
    }
    KILL(mu);

    float *sigma = new float[dim*dim];
    FOR(i, nstates)
    {
        FOR(d, dim*dim) file >> sigma[d];
        gmm->setCovariance(i, sigma, false);
    }
    KILL(sigma);
    gmm->initRegression(ninput);
    file.close();
    return true;
}
