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
#include <public.h>
#include "clustererKKM.h"

using namespace std;

ClustererKKM::~ClustererKKM()
{
    DEL(linKmeans);
    DEL(polKmeans);
    DEL(rbfKmeans);
}

const char *ClustererKKM::GetInfoString()
{
	char *text = new char[1024];
	sprintf(text, "Kernel K-Means\n");
    sprintf(text, "%sClusters: %d\n", text, nbClusters);
	sprintf(text, "%sKernel: ", text);
	switch(kernelType)
	{
	case 0:
		sprintf(text, "%s linear", text);
		break;
	case 1:
		sprintf(text, "%s polynomial (deg: %f %f width: %f)", text, kernelDegree, kernelGamma);
		break;
	case 2:
		sprintf(text, "%s rbf (gamma: %f)", text, kernelGamma);
		break;
	}
	return text;
}

void ClustererKKM::Train(std::vector< fvec > _samples)
{
	vector<sample_type> samples;
	vector<double> labels;

	FOR(i, _samples.size())
	{
		sample_type samp;
		samp(0) = _samples[i][0];
		samp(1) = _samples[i][1];
		samples.push_back(samp);
	}

	maxVectors = 30;

	dlib::kcentroid<lin_kernel>linKc = dlib::kcentroid<lin_kernel>(lin_kernel(),0.001, maxVectors);
    dlib::kcentroid<pol_kernel>polKc = dlib::kcentroid<pol_kernel>(pol_kernel(1,1,kernelDegree),0.001, maxVectors);
	dlib::kcentroid<rbf_kernel>rbfKc = dlib::kcentroid<rbf_kernel>(rbf_kernel(1./kernelGamma),0.01, maxVectors);

	std::vector<sample_type> initial_centers;

	switch(kernelType)
	{
	case 0:
		DEL(linKmeans);
		linKmeans = new dlib::kkmeans<lin_kernel>(linKc);
        linKmeans->set_number_of_centers(nbClusters);
        pick_initial_centers(nbClusters, initial_centers, samples, linKmeans->get_kernel());
		linKmeans->train(samples,initial_centers);
		break;
	case 1:
		DEL(polKmeans);
		polKmeans = new dlib::kkmeans<pol_kernel>(polKc);
        polKmeans->set_number_of_centers(nbClusters);
        pick_initial_centers(nbClusters, initial_centers, samples, polKmeans->get_kernel());
		polKmeans->train(samples,initial_centers);
		break;
	case 2:
		DEL(rbfKmeans);
		rbfKmeans = new dlib::kkmeans<rbf_kernel>(rbfKc);
        rbfKmeans->set_number_of_centers(nbClusters);
        pick_initial_centers(nbClusters, initial_centers, samples, rbfKmeans->get_kernel());
		rbfKmeans->train(samples,initial_centers);
		break;
	}

}

fvec ClustererKKM::Test( const fvec &_sample )
{
	sample_type sample;
	sample(0) = _sample[0];
	sample(1) = _sample[1];
	fvec res;
    res.resize(nbClusters, 0);
    float sum = 0;
    float vmax = -FLT_MAX;
    int index=0;
	switch(kernelType)
	{
	case 0:
    {
        FOR(i, nbClusters)
        {

            double value = (*linKmeans).getDistance(sample, i);
            res[i] = exp(-value);
            if(vmax < res[i])
            {
                vmax = res[i];
                index = i;
            }
            sum += res[i];
        }
    }
//		index = (*linKmeans)(sample);
		break;
	case 1:
    {
        FOR(i, nbClusters)
        {
            double value = (*polKmeans).getDistance(sample, i);
            res[i] = exp(-value);
            if(vmax < res[i])
            {
                vmax = res[i];
                index = i;
            }
            sum += res[i];
        }
    }
//		index = (*polKmeans)(sample);
		break;
	case 2:
    {
        FOR(i, nbClusters)
        {
            double value = (*rbfKmeans).getDistance(sample, i);
            res[i] = exp(-value);
            if(vmax < res[i])
            {
                vmax = res[i];
                index = i;
            }
            sum += res[i];
        }
    }
//		index = (*rbfKmeans)(sample);
		break;
	}
    /*
    FOR(i, nbClusters)
    {
        res[i] /= sum;
    }
    */
//    res[index] *= 1.2f;
    FOR(i, nbClusters) res[i] = 0;
    res[index] = 1;
	return res;
}

double ClustererKKM::TestScore(const fvec &_sample, const int index)
{
    if(index < 0 || index > nbClusters) return 0;
    sample_type sample;
    sample(0) = _sample[0];
    sample(1) = _sample[1];
    double value = 0;
    switch(kernelType)
    {
    case 0:
        value = (*linKmeans).getDistance(sample, index);
        break;
    case 1:
        value = (*polKmeans).getDistance(sample, index);
        break;
    case 2:
        value = (*rbfKmeans).getDistance(sample, index);
        break;
    }
    return -value;
//    return exp(-value);
}

fvec ClustererKKM::TestUnnormalized(const fvec &_sample )
{
    sample_type sample;
    sample(0) = _sample[0];
    sample(1) = _sample[1];
    fvec res;
    res.resize(nbClusters, 0);
    int index=0;
    switch(kernelType)
    {
    case 0:
    {
        FOR(i, nbClusters)
        {
            res[i] = (*linKmeans).getDistance(sample, i);
        }
    }
        break;
    case 1:
    {
        FOR(i, nbClusters)
        {
            res[i] = (*polKmeans).getDistance(sample, i);
        }
    }
        break;
    case 2:
    {
        FOR(i, nbClusters)
        {
            res[i] = (*rbfKmeans).getDistance(sample, i);
        }
    }
        break;
    }
    return res;
}

fvec ClustererKKM::Test( const fVec &_sample )
{
    sample_type sample;
    sample(0) = _sample._[0];
    sample(1) = _sample._[1];
    fvec res;
    res.resize(nbClusters, 0);
    float sum = 0;
    float vmax = -FLT_MAX;
    int index=0;
    switch(kernelType)
    {
    case 0:
    {
        FOR(i, nbClusters)
        {
            res[i] = (*linKmeans).getDistance(sample, i);
            if(vmax < res[i])
            {
                vmax = res[i];
                index = i;
            }
            sum += res[i];
        }
    }
//		index = (*linKmeans)(sample);
        break;
    case 1:
    {
        FOR(i, nbClusters)
        {
            res[i] = (*polKmeans).getDistance(sample, i);
            if(vmax < res[i])
            {
                vmax = res[i];
                index = i;
            }
            sum += res[i];
        }
    }
//		index = (*polKmeans)(sample);
        break;
    case 2:
    {
        FOR(i, nbClusters)
        {
            res[i] = (*rbfKmeans).getDistance(sample, i);
            if(vmax < res[i])
            {
                vmax = res[i];
                index = i;
            }
            sum += res[i];
        }
    }
//		index = (*rbfKmeans)(sample);
        break;
    }
    /*
    FOR(i, nbClusters)
    {
        res[i] /= sum;
    }
    res[index] *= 1.2f;
    */
    FOR(i, nbClusters) res[i] = 0;
    res[index] = 1;
    return res;
}
