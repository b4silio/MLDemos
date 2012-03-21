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
    if(!_samples.size()) return;
    dim = _samples[0].size();

    if(dim > 10) dim = 10;

    maxVectors = 30;

    switch(dim)
    {
    case 2:
        TrainDim<2>(_samples);
        return;
    case 3:
        TrainDim<3>(_samples);
        return;
    case 4:
        TrainDim<4>(_samples);
        return;
    case 5:
        TrainDim<5>(_samples);
        return;
    case 6:
        TrainDim<6>(_samples);
        return;
    case 7:
        TrainDim<7>(_samples);
        return;
    case 8:
        TrainDim<8>(_samples);
        return;
    case 9:
        TrainDim<9>(_samples);
        return;
    case 10:
        TrainDim<10>(_samples);
        return;
    case 11:
        TrainDim<11>(_samples);
        return;
    default:
        TrainDim<12>(_samples);
        return;
    }

    std::vector<sample_type> samples;

	FOR(i, _samples.size())
	{
		sample_type samp;
		samp(0) = _samples[i][0];
		samp(1) = _samples[i][1];
		samples.push_back(samp);
	}

	maxVectors = 30;

    dlib::kcentroid<lin_kernel> linKc = dlib::kcentroid<lin_kernel>(lin_kernel(),0.001, maxVectors);
    dlib::kcentroid<pol_kernel> polKc = dlib::kcentroid<pol_kernel>(pol_kernel(1,1,kernelDegree),0.001, maxVectors);
    dlib::kcentroid<rbf_kernel> rbfKc = dlib::kcentroid<rbf_kernel>(rbf_kernel(1./kernelGamma),0.01, maxVectors);

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

template <int N>
void ClustererKKM::TrainDim(std::vector< fvec > _samples)
{
#define sampletype matrix<double,N,1>
#define rbfkernel radial_basis_kernel< sampletype >
#define linkernel linear_kernel< sampletype >
#define polkernel polynomial_kernel< sampletype >

    std::vector<sampletype> samples;
    sampletype samp;
    FOR(i, _samples.size()) { FOR(d, dim) samp(d) = _samples[i][d]; samples.push_back(samp); }
    std::vector<sampletype> initial_centers;
    switch(kernelType)
    {
    case 0:
    {
        if(decFunction) delete (kkmeans<linkernel>*)decFunction;
        kcentroid<linkernel> cen = kcentroid<linkernel>(linkernel(),0.001, maxVectors);
        kkmeans<linkernel> *fun = new kkmeans<linkernel>(cen);
        fun->set_number_of_centers(nbClusters);
        pick_initial_centers(nbClusters, initial_centers, samples, fun->get_kernel());
        fun->train(samples,initial_centers);
        decFunction = (void *)fun;
    }
        break;
    case 1:
    {
        if(decFunction) delete (kkmeans<polkernel>*)decFunction;
        kcentroid<polkernel> cen = kcentroid<polkernel>(polkernel(1,1,kernelDegree),0.001, maxVectors);
        kkmeans<polkernel> *fun = new kkmeans<polkernel>(cen);
        fun->set_number_of_centers(nbClusters);
        pick_initial_centers(nbClusters, initial_centers, samples, fun->get_kernel());
        fun->train(samples,initial_centers);
        decFunction = (void *)fun;
    }
        break;
    case 2:
    {
        if(decFunction) delete (kkmeans<rbfkernel>*)decFunction;
        kcentroid<rbfkernel> cen = kcentroid<rbfkernel>(rbfkernel(1./kernelGamma),0.001, maxVectors);
        kkmeans<rbfkernel> *fun = new kkmeans<rbfkernel>(cen);
        fun->set_number_of_centers(nbClusters);
        pick_initial_centers(nbClusters, initial_centers, samples, fun->get_kernel());
        fun->train(samples,initial_centers);
        decFunction = (void *)fun;
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

#undef rbfkernel
#undef linkernel
#undef polkernel

fvec ClustererKKM::Test( const fvec &_sample )
{
    switch(dim)
    {
    case 2:
        return TestDim<2>(_sample);
    case 3:
        return TestDim<3>(_sample);
    case 4:
        return TestDim<4>(_sample);
    case 5:
        return TestDim<5>(_sample);
    case 6:
        return TestDim<6>(_sample);
    case 7:
        return TestDim<7>(_sample);
    case 8:
        return TestDim<8>(_sample);
    case 9:
        return TestDim<9>(_sample);
    case 10:
        return TestDim<10>(_sample);
    case 11:
        return TestDim<11>(_sample);
    default:
        return TestDim<12>(_sample);
    }
}

double ClustererKKM::TestScore(const fvec &_sample, const int index)
{
    if(index < 0 || index > nbClusters) return 0;
    switch(dim)
    {
    case 2:
        return TestScoreDim<2>(_sample, index);
    case 3:
        return TestScoreDim<3>(_sample, index);
    case 4:
        return TestScoreDim<4>(_sample, index);
    case 5:
        return TestScoreDim<5>(_sample, index);
    case 6:
        return TestScoreDim<6>(_sample, index);
    case 7:
        return TestScoreDim<7>(_sample, index);
    case 8:
        return TestScoreDim<8>(_sample, index);
    case 9:
        return TestScoreDim<9>(_sample, index);
    case 10:
        return TestScoreDim<10>(_sample, index);
    case 11:
        return TestScoreDim<11>(_sample, index);
    default:
        return TestScoreDim<12>(_sample, index);
    }
}

fvec ClustererKKM::TestUnnormalized(const fvec &_sample )
{
    switch(dim)
    {
    case 2:
        return TestUnnormalizedDim<2>(_sample);
    case 3:
        return TestUnnormalizedDim<3>(_sample);
    case 4:
        return TestUnnormalizedDim<4>(_sample);
    case 5:
        return TestUnnormalizedDim<5>(_sample);
    case 6:
        return TestUnnormalizedDim<6>(_sample);
    case 7:
        return TestUnnormalizedDim<7>(_sample);
    case 8:
        return TestUnnormalizedDim<8>(_sample);
    case 9:
        return TestUnnormalizedDim<9>(_sample);
    case 10:
        return TestUnnormalizedDim<10>(_sample);
    case 11:
        return TestUnnormalizedDim<11>(_sample);
    default:
        return TestUnnormalizedDim<12>(_sample);
    }
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
