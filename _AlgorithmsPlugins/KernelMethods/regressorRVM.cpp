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
#include "regressorRVM.h"

using namespace std;

char *RegressorRVM::GetInfoString()
{
	char *text = new char[255];
	sprintf(text, "Relevance Vector Regression\n");
	sprintf(text, "%sKernel: ", text);
	switch(kernelType)
	{
	case 0:
		sprintf(text, "%s linear", text);
		break;
	case 1:
		sprintf(text, "%s polynomial (deg: %f %f width: %f)", text, kernelDegree, kernelParam);
		break;
	case 2:
		sprintf(text, "%s rbf (gamma: %f)", text, kernelParam);
		break;
	}
	sprintf(text, "%seps: %f\n", text, epsilon);
	sprintf(text, "%sRelevant Vectors: %d\n", text, GetSVs().size());
	return text;
}

void RegressorRVM::Train(std::vector< fvec > _samples, ivec _labels)
{
	samples.clear();
	labels.clear();
    if(!_samples.size()) return;
    if(_samples[0].size() > 2) return; // no multi-dim for now...

	FOR(i, _samples.size())
	{
		reg_sample_type samp;
		samp(0) = _samples[i][0];
		samples.push_back(samp);
		labels.push_back(_samples[i][1]);
	}
	randomize_samples(samples, labels);

	switch(kernelType)
	{
	case 0:
		linTrainer = dlib::rvm_regression_trainer<reg_lin_kernel>();
		linTrainer.set_epsilon(epsilon);
		linTrainer.set_kernel(reg_lin_kernel());
		linFunc = linTrainer.train(samples, labels);
		break;
	case 1:
		polTrainer = dlib::rvm_regression_trainer<reg_pol_kernel>();
		polTrainer.set_epsilon(epsilon);
		polTrainer.set_kernel(reg_pol_kernel(1./kernelParam, 0, kernelDegree));
		polFunc = polTrainer.train(samples, labels);
		break;
	case 2:
		rbfTrainer = dlib::rvm_regression_trainer<reg_rbf_kernel>();
		rbfTrainer.set_epsilon(epsilon);
		rbfTrainer.set_kernel(reg_rbf_kernel(1./kernelParam));
		rbfFunc = rbfTrainer.train(samples, labels);
		break;
	}
}

fvec  RegressorRVM::Test( const fvec &_sample )
{
	fvec res;
	res.resize(2,0);
    reg_sample_type sample;
	sample(0) = _sample[0];
	switch(kernelType)
	{
	case 0:
		res[0] = linFunc(sample);
		break;
	case 1:
		res[0] = polFunc(sample);
		break;
	case 2:
		res[0] = rbfFunc(sample);
		break;
	}
	return res;
}

fVec  RegressorRVM::Test( const fVec &_sample )
{
	fVec res;
	reg_sample_type sample;
	sample(0) = _sample._[0];
	switch(kernelType)
	{
	case 0:
		res[0] = linFunc(sample);
		break;
	case 1:
		res[0] = polFunc(sample);
		break;
	case 2:
		res[0] = rbfFunc(sample);
		break;
	}
	return res;
}

std::vector<fvec> RegressorRVM::GetSVs()
{
	vector<fvec> SVs;
	if(kernelType == 0)
	{
		FOR(i, linFunc.basis_vectors.nr())
		{
			fvec sv;
			sv.resize(2,0);
			sv[0] = linFunc.basis_vectors(i)(0);
			SVs.push_back(sv);
		}
	}
	else if(kernelType == 1)
	{
		FOR(i, polFunc.basis_vectors.nr())
		{
			fvec sv;
			sv.resize(2,0);
			sv[0] = polFunc.basis_vectors(i)(0);
			SVs.push_back(sv);
		}
	}
	else if(kernelType == 2)
	{
		FOR(i, rbfFunc.basis_vectors.nr())
		{
			fvec sv;
			sv.resize(2,0.5);
			sv[0] = rbfFunc.basis_vectors(i)(0);
			SVs.push_back(sv);
		}
	}

	FOR(i, SVs.size())
	{
		int closest = 0;
		double dist = DBL_MAX;
		FOR(j, samples.size())
		{
			double d = abs(samples[j](0)-SVs[i][0]);
			if(d < dist)
			{
				dist = d;
				closest = j;
			}
		}
		SVs[i][1] = labels[closest];
	}
	return SVs;
}
