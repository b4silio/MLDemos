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
#include "classifierPegasos.h"

using namespace std;
using namespace dlib;

ClassifierPegasos::~ClassifierPegasos()
{
    if(!decFunction) return;
    switch(kernelTypeTrained)
    {
    case 0:
        if(decFunction) delete [] (lin_func*)decFunction;
        break;
    case 1:
        if(decFunction) delete [] (pol_func*)decFunction;
        break;
    case 2:
        if(decFunction) delete [] (rbf_func*)decFunction;
        break;
    }
    decFunction = 0;
}

const char *ClassifierPegasos::GetInfoString() const
{
	char *text = new char[1024];
	sprintf(text, "pegasos SVM\n");
	sprintf(text, "%sKernel: ", text);
	switch(kernelType)
	{
	case 0:
		sprintf(text, "%s linear", text);
		break;
	case 1:
		sprintf(text, "%s polynomial (deg: %d width: %f)", text, kernelDegree, kernelParam);
		break;
	case 2:
		sprintf(text, "%s rbf (gamma: %f)", text, kernelParam);
		break;
	}
	sprintf(text, "%slambda: %f\n", text, lambda);
	sprintf(text, "%sSupport Vectors: %lu\n", text, (unsigned long)GetSVs().size());
	return text;
}

void ClassifierPegasos::Train(std::vector< fvec > _samples, ivec _labels)
{
    if(!_samples.size()) return;
    dim = _samples[0].size();

    classMap.clear();
    int cnt=0;
    FOR(i, _labels.size()) if(!classMap.count(_labels[i])) classMap[_labels[i]] = cnt++;
    for(map<int,int>::iterator it=classMap.begin(); it != classMap.end(); it++) inverseMap[it->second] = it->first;

    std::vector<sample_type> samples;
    std::vector<double> labels;
    sample_type samp(dim);
    FOR(i, _samples.size()) { FOR(d, dim) samp(d) = _samples[i][d]; samples.push_back(samp); }
    if(!decFunction) return;
    switch(kernelTypeTrained)
    {
    case 0:
        if(decFunction) delete [] (lin_func*)decFunction;
        break;
    case 1:
        if(decFunction) delete [] (pol_func*)decFunction;
        break;
    case 2:
        if(decFunction) delete [] (rbf_func*)decFunction;
        break;
    }
    decFunction = 0;

    FOR(i, _samples.size()) labels.push_back(_labels[i] == 1 ? 1 : -1);

    randomize_samples(samples, labels);

    switch(kernelType)
    {
    case 0:
    {
        svm_pegasos<lin_kernel> train = svm_pegasos<lin_kernel>();
        train.set_lambda(lambda);
        train.set_kernel(lin_kernel());
        train.set_max_num_sv(maxSV);
        lin_func *fun = new lin_func[1];
        *fun = batch_cached(train).train(samples, labels);
        decFunction = (void *)fun;
        kernelTypeTrained = 0;
    }
        break;
    case 1:
    {
        svm_pegasos<pol_kernel> train = svm_pegasos<pol_kernel>();
        train.set_lambda(lambda);
        train.set_kernel(pol_kernel(1./kernelParam, 0, kernelDegree));
        train.set_max_num_sv(maxSV);
        pol_func *fun = new pol_func[1];
        *fun = batch_cached(train).train(samples, labels);
        decFunction = (void *)fun;
        kernelTypeTrained = 1;
    }
        break;
    case 2:
    {
        svm_pegasos<rbf_kernel> train = svm_pegasos<rbf_kernel>();
        train.set_lambda(lambda);
        train.set_kernel(rbf_kernel(1./kernelParam));
        train.set_max_num_sv(maxSV);
        rbf_func *fun = new rbf_func[1];
        *fun = batch_cached(train).train(samples, labels);
        decFunction = (void *)fun;
        kernelTypeTrained = 2;
    }
        break;
    }
}

float ClassifierPegasos::Test( const fvec &_sample ) const
{
    float estimate = 0.f;

    sample_type sample(dim);
    FOR(d,dim) sample(d) = _sample[d];
    if(!decFunction) return estimate;
    switch(kernelTypeTrained)
    {
    case 0:
    {
        lin_func fun = *(lin_func*)decFunction;
        estimate = fun(sample);
    }
        break;
    case 1:
    {
        pol_func fun = *(pol_func*)decFunction;
        estimate = fun(sample);
    }
        break;
    case 2:
    {
        rbf_func fun = *(rbf_func*)decFunction;
        estimate = fun(sample);
    }
        break;
    }
    return estimate;
}

std::vector<fvec> ClassifierPegasos::GetSVs() const
{
    std::vector<fvec> SVs;
    switch(kernelTypeTrained)
    {
    case 0:
    {

        FOR(i, (*(lin_func*)decFunction).basis_vectors.nr())
        {
            fvec sv(dim);
            FOR(d, dim) sv[d] = (*(lin_func*)decFunction).basis_vectors(i)(d);
            SVs.push_back(sv);
        }
    }
        break;
    case 1:
    {

        FOR(i, (*(pol_func*)decFunction).basis_vectors.nr())
        {
            fvec sv(dim);
            FOR(d, dim) sv[d] = (*(pol_func*)decFunction).basis_vectors(i)(d);
            SVs.push_back(sv);
        }
    }
        break;
    case 2:
    {

        FOR(i, (*(rbf_func*)decFunction).basis_vectors.nr())
        {
            fvec sv(dim);
            FOR(d, dim) sv[d] = (*(rbf_func*)decFunction).basis_vectors(i)(d);
            SVs.push_back(sv);
        }
    }
        break;
    }
    return SVs;
}

