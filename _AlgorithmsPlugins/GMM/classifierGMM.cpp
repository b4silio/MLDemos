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
#include <map>
#include <QDebug>
#include <iostream>
#include <fstream>

using namespace std;

ClassifierGMM::ClassifierGMM()
	: nbClusters(2), covarianceType(2), initType(1)
{
	bSingleClass = false;
	bMultiClass = true;
}

ClassifierGMM::~ClassifierGMM()
{
	FOR(i, gmms.size()) DEL(gmms[i]);
	FOR(i, data.size()) KILL(data[i]);
}

void ClassifierGMM::Train(std::vector< fvec > samples, ivec labels)
{
	if(!samples.size()) return;
    vector< fvec > positives, negatives;
	classes.clear();
    classMap.clear();
    inverseMap.clear();

    int cnt=0;
    FOR(i, labels.size()) if(!classMap.count(labels[i])) classMap[labels[i]] = cnt++;
    for(map<int,int>::iterator it=classMap.begin(); it != classMap.end(); it++) inverseMap[it->second] = it->first;
    ivec newLabels(labels.size());
    FOR(i, labels.size()) newLabels[i] = classMap[labels[i]];

//    for(map<int,int>::iterator it=inverseMap.begin(); it != inverseMap.end(); it++) qDebug() << "inverse: " << it->first << it->second;
//    for(map<int,int>::iterator it=classMap.begin(); it != classMap.end(); it++) qDebug() << "class: " << it->first << it->second;

    std::map<int, vector<fvec> > sampleMap;

    FOR(i, samples.size())
	{
        sampleMap[newLabels[i]].push_back(samples[i]);
        if(newLabels[i] == 1) positives.push_back(samples[i]);
		else negatives.push_back(samples[i]);
	}
	int dim = samples[0].size();
	nbClusters = min(nbClusters, (u32)samples.size());

	FOR(i, gmms.size()) DEL(gmms[i]);
	FOR(i, data.size()) KILL(data[i]);
	gmms.clear();
	data.clear();
    int i=0;
    for(map<int,vector<fvec> >::iterator it=sampleMap.begin(); it != sampleMap.end(); it++, i++)
	{
        vector<fvec> &s = it->second;
		gmms.push_back(new Gmm(nbClusters, dim));
		data.push_back(new float[dim*s.size()]);
		FOR(j, s.size())
		{
			FOR(d, dim) data[i][j*dim + d] = s[j][d];
		}
		gmms[i]->init(data[i], s.size(), initType);
		gmms[i]->em(data[i], s.size(), 1e-4, (COVARIANCE_TYPE)covarianceType);
	}
}

fvec ClassifierGMM::TestMulti(const fvec &sample)
{
	fvec pdf(gmms.size());
	FOR(i, gmms.size()) pdf[i] = gmms[i]->pdf((float*)&sample[0]);
	if(gmms.size()==2)
	{
		fvec res(1);
//        res[0] = pdf[1] - pdf[0];
        double p1 = log((double)pdf[1]);
        double p0 = log((double)pdf[0]);
//        if(fabs(p1) < 1e-6) p1 = 0;
//        if(fabs(p0) < 1e-6) p0 = 0;
        res[0] = (float)(p1 - p0);
        return res;
	}

    float xmin=-100.f, xmax=100.f; // we clamp the value between these two
	float sum = 0;
	FOR(i, pdf.size())
	{
        float value = log(pdf[i]);
        value = (min(xmax,max(xmin, value)) - xmin) / (xmax);
        pdf[i] = value;
	}
	return pdf;
}

float ClassifierGMM::Test( const fvec &sample)
{
	fvec pdf = TestMulti(sample);
	if(pdf.size() < 2) return 0;
	float res = log(pdf[1]) - log(pdf[0]);
	return res;
}

float ClassifierGMM::Test( const fVec &_sample)
{
	if(!gmms.size()) return 0;
	fVec sample = _sample;
	float v0 = gmms[0]->pdf(sample._);
	float v1 = 0;
	if(gmms.size() > 1)
	{
		v1 = gmms[1]->pdf(sample._);
	}
	float res = log(v1) - log(v0);
	return res;
}

void ClassifierGMM::SetParams(u32 nbClusters, u32 covarianceType, u32 initType)
{
	this->nbClusters = nbClusters;
	this->covarianceType = covarianceType;
	this->initType = initType;
}

const char *ClassifierGMM::GetInfoString()
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

void ClassifierGMM::SaveModel(std::string filename)
{
    std::cout << "saving GMM model";
    if(!gmms.size())
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

    int dim = gmms[0]->dim;
    int classCount = gmms.size();
    file << dim << " " << classCount << endl;

    for(map<int,int>::iterator it=inverseMap.begin(); it != inverseMap.end(); it++)
    {
        file << it->first << " " << it->second << " ";
    }
    file << endl;
    for(map<int,int>::iterator it=classMap.begin(); it != classMap.end(); it++)
    {
        file << it->first << " " << it->second << " ";
    }
    file << endl;

    file.precision(10); //setting the precision of writing
    FOR(g, gmms.size())
    {
        Gmm* gmm = gmms[g];
        file << gmm->dim << endl;
        file << gmm->nstates << endl;
        FOR(i, gmm->nstates)
        {
            float prior = gmm->getPrior(i);
            file << prior << " ";
        }
        file << endl;
        float *mu = new float[dim];
        FOR(i, gmm->nstates)
        {
            gmm->getMean(i, mu);
            FOR(d, dim)
            {
                file << mu[d] << " ";
            }
            file << endl;
        }
        KILL(mu);
        float *sigma = new float[dim*dim];
        FOR(i, gmm->nstates)
        {
            gmm->getCovariance(i, sigma, false);
            FOR(d, dim*dim)
            {
                file << sigma[d] << " ";
            }
            file << endl;
        }
        KILL(sigma);
    }

    file.close();
}

bool ClassifierGMM::LoadModel(std::string filename)
{
    std::cout << "loading GMM model: " << filename;

    std::ifstream file(filename.c_str());

    if(!file.is_open()){
        std::cout << "Error: Could not open the file!" << std::endl;
        return false;
    }

    FOR(i, gmms.size()) DEL(gmms[i]);
    gmms.clear();

    int dim, classCount;
    file >> dim >> classCount;
    inverseMap.clear();
    classMap.clear();

    FOR(i, classCount)
    {
        int first, second;
        file >> first >> second;
        inverseMap[first] = second;
    }

    FOR(i, classCount)
    {
        int first, second;
        file >> first >> second;
        classMap[first] = second;
    }

    FOR(g, classCount)
    {
        int dim, nstates;
        file >> dim >> nstates;
        Gmm *gmm = new Gmm(nstates, dim);
        if(!g) nbClusters = gmm->nstates;
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
        gmms.push_back(gmm);
    }

    file.close();
    return true;
}
