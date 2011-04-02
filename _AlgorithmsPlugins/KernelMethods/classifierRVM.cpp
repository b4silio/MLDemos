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
#include "basicOpenCV.h"
#include "classifierRVM.h"

using namespace std;

char *ClassifierRVM::GetInfoString()
{
	char *text = new char[1024];
	sprintf(text, "Relevance Vector Machine\n");
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

void ClassifierRVM::Train(std::vector< fvec > _samples, ivec _labels)
{
	vector<sample_type> samples;
	vector<double> labels;

	FOR(i, _samples.size())
	{
		sample_type samp;
		samp(0) = _samples[i][0];
		samp(1) = _samples[i][1];
		samples.push_back(samp);
		labels.push_back(_labels[i] == 1 ? 1 : -1);
	}
	randomize_samples(samples, labels);


	switch(kernelType)
	{
	case 0:
		linTrainer = dlib::rvm_trainer<lin_kernel>();
		linTrainer.set_epsilon(epsilon);
		linTrainer.set_kernel(lin_kernel());
		linFunc = linTrainer.train(samples, labels);
		break;
	case 1:
		polTrainer = dlib::rvm_trainer<pol_kernel>();
		polTrainer.set_epsilon(epsilon);
		polTrainer.set_kernel(pol_kernel(1./kernelParam, 0, kernelDegree));
		polFunc = polTrainer.train(samples, labels);
		break;
	case 2:
		rbfTrainer = dlib::rvm_trainer<rbf_kernel>();
		rbfTrainer.set_epsilon(epsilon);
		rbfTrainer.set_kernel(rbf_kernel(1./kernelParam));
		rbfFunc = rbfTrainer.train(samples, labels);
		break;
	}
}

float ClassifierRVM::Test( const fvec &_sample )
{
	sample_type sample;
	sample(0) = _sample[0];
	sample(1) = _sample[1];
	float estimate;
	switch(kernelType)
	{
	case 0:
		estimate = linFunc(sample);
		break;
	case 1:
		estimate = polFunc(sample);
		break;
	case 2:
		estimate = rbfFunc(sample);
		break;
	}
	return estimate;
}

float ClassifierRVM::Test( const fVec &_sample )
{
	sample_type sample;
	sample(0) = _sample._[0];
	sample(1) = _sample._[1];
	float estimate;
	switch(kernelType)
	{
	case 0:
		estimate = linFunc(sample);
		break;
	case 1:
		estimate = polFunc(sample);
		break;
	case 2:
		estimate = rbfFunc(sample);
		break;
	}
	return estimate;
}

std::vector<fvec> ClassifierRVM::GetSVs()
{
	vector<fvec> SVs;
	if(kernelType == 0)
	{
		FOR(i, linFunc.basis_vectors.nr())
		{
			fvec sv;
			sv.push_back(linFunc.basis_vectors(i)(0));
			sv.push_back(linFunc.basis_vectors(i)(1));
			SVs.push_back(sv);
		}
	}
	else if(kernelType == 1)
	{
		FOR(i, polFunc.basis_vectors.nr())
		{
			fvec sv;
			sv.push_back(polFunc.basis_vectors(i)(0));
			sv.push_back(polFunc.basis_vectors(i)(1));
			SVs.push_back(sv);
		}
	}
	else if(kernelType == 2)
	{
		FOR(i, rbfFunc.basis_vectors.nr())
		{
			fvec sv;
			sv.push_back(rbfFunc.basis_vectors(i)(0));
			sv.push_back(rbfFunc.basis_vectors(i)(1));
			SVs.push_back(sv);
		}
	}
	return SVs;
}
