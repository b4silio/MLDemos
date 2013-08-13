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
using namespace dlib;

ClustererKKM::~ClustererKKM()
{
    if(decFunction)
    {
#define KILLCASE(a) case a:{KillDim<a>();return;}
        switch(dim)
        {
        KILLCASE(2);
        KILLCASE(3);
        KILLCASE(4);
        KILLCASE(5);
        KILLCASE(6);
        KILLCASE(7);
        KILLCASE(8);
        KILLCASE(9);
        KILLCASE(10);
        KILLCASE(11);
        KILLCASE(12);
        default:
            KillDim<2>();
            return;
        }
    }
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
		sprintf(text, "%s polynomial (deg: %f width: %f)", text, kernelDegree, kernelGamma);
		break;
	case 2:
		sprintf(text, "%s rbf (gamma: %f)", text, kernelGamma);
		break;
	}
	return text;
}

void ClustererKKM::Train(std::vector< fvec > _samples)
{
    if(!_samples.size()) return;
    dim = _samples[0].size();

    if(dim > 12) dim = 12;

    maxVectors = 30;

#define TRAINCASE(a) case a:{TrainDim<a>(_samples);return;}
    switch(dim)
    {
    TRAINCASE(2);
    TRAINCASE(3);
    TRAINCASE(4);
    TRAINCASE(5);
    TRAINCASE(6);
    TRAINCASE(7);
    TRAINCASE(8);
    TRAINCASE(9);
    TRAINCASE(10);
    TRAINCASE(11);
    TRAINCASE(12);
    default:
        TrainDim<2>(_samples);
        return;
    }
}

template <int N>
void ClustererKKM::KillDim()
{
    if(!decFunction) return;
    switch(kernelTypeTrained)
    {
    case 0:
        if(decFunction) delete (kkmeans<linkernel>*)decFunction;
        break;
    case 1:
        if(decFunction) delete (kkmeans<polkernel>*)decFunction;
        break;
    case 2:
        if(decFunction) delete (kkmeans<rbfkernel>*)decFunction;
        break;
    }
    decFunction = 0;
}

template <int N>
void ClustererKKM::TrainDim(std::vector< fvec > _samples)
{
//    if(nbClusters < 2) nbClusters = 2; // we can't have less than 2 clusters
    std::vector<sampletype> samples;
    sampletype samp;
    FOR(i, _samples.size()) { FOR(d, dim) samp(d) = _samples[i][d]; samples.push_back(samp); }
    std::vector<sampletype> initial_centers;
    KillDim<N>();

    switch(kernelType)
    {
    case 0:
    {
        kcentroid<linkernel> cen = kcentroid<linkernel>(linkernel(),0.001, maxVectors);
        kkmeans<linkernel> *fun = new kkmeans<linkernel>(cen);
        fun->set_number_of_centers(nbClusters);
        pick_initial_centers(nbClusters, initial_centers, samples, fun->get_kernel());
        fun->train(samples,initial_centers);
        decFunction = (void *)fun;
        kernelTypeTrained = 0;
    }
        break;
    case 1:
    {
        kcentroid<polkernel> cen = kcentroid<polkernel>(polkernel(1,1,kernelDegree),0.001, maxVectors);
        kkmeans<polkernel> *fun = new kkmeans<polkernel>(cen);
        fun->set_number_of_centers(nbClusters);
        pick_initial_centers(nbClusters, initial_centers, samples, fun->get_kernel());
        fun->train(samples,initial_centers);
        decFunction = (void *)fun;
        kernelTypeTrained = 1;
    }
        break;
    case 2:
    {
        kcentroid<rbfkernel> cen = kcentroid<rbfkernel>(rbfkernel(1./kernelGamma),0.001, maxVectors);
        kkmeans<rbfkernel> *fun = new kkmeans<rbfkernel>(cen);
        fun->set_number_of_centers(nbClusters);
        pick_initial_centers(nbClusters, initial_centers, samples, fun->get_kernel());
        fun->train(samples,initial_centers);
        decFunction = (void *)fun;
        kernelTypeTrained = 2;
    }
        break;
    }
}

template <int N>
fvec ClustererKKM::TestDim(const fvec &_sample)
{
    sampletype sample;
    FOR(d,dim) sample(d) = _sample[d];
    fvec res;
    res.resize(nbClusters, 0);
    if(!decFunction) return res;
    float sum = 0;
    float vmax = -FLT_MAX;
    int index=0;
    switch(kernelType)
    {
    case 0:
    {
        FOR(i, nbClusters)
        {
            double value = (*((kkmeans<linkernel>*)decFunction)).getDistance(sample, i);
            res[i] = exp(-value);
            if(vmax < res[i])
            {
                vmax = res[i];
                index = i;
            }
            sum += res[i];
        }
    }
        break;
    case 1:
    {
        FOR(i, nbClusters)
        {
            double value = (*((kkmeans<polkernel>*)decFunction)).getDistance(sample, i);
            res[i] = exp(-value);
            if(vmax < res[i])
            {
                vmax = res[i];
                index = i;
            }
            sum += res[i];
        }
    }
        break;
    case 2:
    {
        FOR(i, nbClusters)
        {
            double value = (*((kkmeans<rbfkernel>*)decFunction)).getDistance(sample, i);
            res[i] = exp(-value);
            if(vmax < res[i])
            {
                vmax = res[i];
                index = i;
            }
            sum += res[i];
        }
    }
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

template <int N>
double ClustererKKM::TestScoreDim(const fvec &_sample, const int index)
{
    sampletype sample;
    FOR(d,dim) sample(d) = _sample[d];
    fvec res;
    res.resize(nbClusters, 0);
    double value = 0;
    switch(kernelType)
    {
    case 0:
        value = (*((kkmeans<linkernel>*)decFunction)).getDistance(sample, index);
        break;
    case 1:
        value = (*((kkmeans<polkernel>*)decFunction)).getDistance(sample, index);
        break;
    case 2:
        value = (*((kkmeans<rbfkernel>*)decFunction)).getDistance(sample, index);
    }
    return -value;
}

template <int N>
fvec ClustererKKM::TestUnnormalizedDim(const fvec &_sample)
{
    sampletype sample;
    FOR(d,dim) sample(d) = _sample[d];
    fvec res;
    res.resize(nbClusters, 0);
    switch(kernelType)
    {
    case 0:
    {
        FOR(i, nbClusters)
        {
            res[i] = (*((kkmeans<linkernel>*)decFunction)).getDistance(sample, i);
        }
    }
        break;
    case 1:
    {
        FOR(i, nbClusters)
        {
            res[i] = (*((kkmeans<polkernel>*)decFunction)).getDistance(sample, i);
        }
    }
        break;
    case 2:
    {
        FOR(i, nbClusters)
        {
            res[i] = (*((kkmeans<rbfkernel>*)decFunction)).getDistance(sample, i);
        }
    }
        break;
    }
    return res;
}

fvec ClustererKKM::Test( const fvec &_sample )
{
#define TESTCASE(a) case a:{return TestDim<a>(_sample);}
    switch(dim)
    {
    TESTCASE(2);
    TESTCASE(3);
    TESTCASE(4);
    TESTCASE(5);
    TESTCASE(6);
    TESTCASE(7);
    TESTCASE(8);
    TESTCASE(9);
    TESTCASE(10);
    TESTCASE(11);
    TESTCASE(12);
    default:
        return TestDim<2>(_sample);
    }
}

double ClustererKKM::TestScore(const fvec &_sample, const int index)
{
#define SCORECASE(a) case a:{return TestScoreDim<a>(_sample, index);}
    if(index < 0 || index > nbClusters) return 0;
    switch(dim)
    {
    SCORECASE(2);
    SCORECASE(3);
    SCORECASE(4);
    SCORECASE(5);
    SCORECASE(6);
    SCORECASE(7);
    SCORECASE(8);
    SCORECASE(9);
    SCORECASE(10);
    SCORECASE(11);
    SCORECASE(12);
    default:
        return TestScoreDim<2>(_sample, index);
    }
}

fvec ClustererKKM::TestUnnormalized(const fvec &_sample )
{
#define TESTUNCASE(a) case a:{return TestUnnormalizedDim<a>(_sample);}
    switch(dim)
    {
    TESTUNCASE(2);
    TESTUNCASE(3);
    TESTUNCASE(4);
    TESTUNCASE(5);
    TESTUNCASE(6);
    TESTUNCASE(7);
    TESTUNCASE(8);
    TESTUNCASE(9);
    TESTUNCASE(10);
    TESTUNCASE(11);
    TESTUNCASE(12);
    default:
        return TestUnnormalizedDim<2>(_sample);
    }
}

#undef TRAINCASE
#undef TESTCASE
#undef SCORECASE
#undef TESTUNCASE
#undef rbfkernel
#undef linkernel
#undef polkernel
