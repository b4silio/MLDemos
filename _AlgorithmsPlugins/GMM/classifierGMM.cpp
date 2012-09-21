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

    for(map<int,int>::iterator it=inverseMap.begin(); it != inverseMap.end(); it++) qDebug() << "inverse: " << it->first << it->second;
    for(map<int,int>::iterator it=classMap.begin(); it != classMap.end(); it++) qDebug() << "class: " << it->first << it->second;

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
		res[0] = log(pdf[1]) - log(pdf[0]);
		return res;
	}

    float xmin=-100.f, xmax=100.f;
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
