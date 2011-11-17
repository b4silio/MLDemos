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
#include "regressorKRLS.h"

using namespace std;

char *RegressorKRLS::GetInfoString()
{
	char *text = new char[255];
	sprintf(text, "Kernel Ridge Least Squares\n");
	sprintf(text, "%sCapacity: %d", text, capacity);
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
	sprintf(text, "%Basis Functions: %d\n", text, GetSVs().size());
	return text;
}

RegressorKRLS::~RegressorKRLS()
{
	DEL(linTrainer);
	DEL(polTrainer);
	DEL(rbfTrainer);
}

void RegressorKRLS::Train(std::vector< fvec > _samples, ivec _labels)
{
	if(capacity == 1) capacity = 2;
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

    DEL(linTrainer);
    DEL(polTrainer);
    DEL(rbfTrainer);
    switch(kernelType)
	{
	case 0:
		{
			linTrainer = new dlib::krls<reg_lin_kernel>(reg_lin_kernel(),epsilon,capacity ? capacity : 1000000);
			FOR(i, samples.size())
			{
				linTrainer->train(samples[i], labels[i]);
			}
			linFunc = linTrainer->get_decision_function();
		}
		break;
	case 1:
		{
			polTrainer = new dlib::krls<reg_pol_kernel>(reg_pol_kernel(1./kernelParam,0,kernelDegree),epsilon,capacity ? capacity : 1000000);
			FOR(i, samples.size())
			{
				polTrainer->train(samples[i], labels[i]);
			}
			polFunc = polTrainer->get_decision_function();
		}
		break;
	case 2:
		{
			rbfTrainer = new dlib::krls<reg_rbf_kernel>(reg_rbf_kernel(1./kernelParam),epsilon,capacity ? capacity : 1000000);
			FOR(i, samples.size())
			{
				rbfTrainer->train(samples[i], labels[i]);
			}
			rbfFunc = rbfTrainer->get_decision_function();
		}
		break;
	}
}

fvec  RegressorKRLS::Test( const fvec &_sample )
{
	fvec res;
	res.resize(2,0);
    if(!linTrainer && !polTrainer && !rbfTrainer) return res;
    reg_sample_type sample;
	sample(0) = _sample[0];
	switch(kernelType)
	{
	case 0:
		res[0] = (*linTrainer)(sample);
		break;
	case 1:
		res[0] = (*polTrainer)(sample);
		break;
	case 2:
		res[0] = (*rbfTrainer)(sample);
		break;
	}
	return res;
}

fVec  RegressorKRLS::Test( const fVec &_sample )
{
	fVec res;
	reg_sample_type sample;
	sample(0) = _sample._[0];
	switch(kernelType)
	{
	case 0:
		res[0] = (*linTrainer)(sample);
		break;
	case 1:
		res[0] = (*polTrainer)(sample);
		break;
	case 2:
		res[0] = (*rbfTrainer)(sample);
		break;
	}
	return res;
}

std::vector<fvec> RegressorKRLS::GetSVs()
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
			sv.resize(2,0);
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
