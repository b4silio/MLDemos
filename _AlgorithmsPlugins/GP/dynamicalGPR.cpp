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
#include "mymaths.h"
#include "dynamicalGPR.h"

void DynamicalGPR::Train(std::vector< std::vector<fvec> > trajectories, ivec labels)
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
	Matrix inputs(dim, samples.size());
	Matrix outputs(dim, samples.size());
	FOR(n, samples.size())
	{
		FOR(d, dim) inputs(d+1,n+1) = samples[n][d];
		//outputs(n+1,1) = samples[n][dim];
		FOR(d, dim) outputs(d+1,n+1) = samples[n][dim+d];
	}

	if(sogp) delete sogp;
	if(kernelType == kerPOL)
	{
		if(!degree) degree = 1;
		RowVector deg(degree);
		for (int i=0; i<degree; i++) deg(i+1) = param1*(1+i*0.3f);
		POLKernel kern(deg);
		SOGPParams params(&kern);
		params.s20=param2;
		params.capacity = capacity;
		sogp = new SOGP(params);
	}
	else
	{
		RBFKernel kern(param1);
		SOGPParams params(&kern);
		params.s20=param2;
		params.capacity = capacity;
		sogp = new SOGP(params);
	}
	sogp->addM(inputs,outputs);

	bTrained = true;
}


std::vector<fvec> DynamicalGPR::Test( const fvec &sample, const int count)
{
	fvec start = sample;
	dim = sample.size();
	std::vector<fvec> res;
	res.resize(count);
	FOR(i, count) res[i].resize(dim,0);
	if(!sogp) return res;

	Matrix _testout;
	ColumnVector _testin(dim);
	fvec velocity; velocity.resize(dim,0);
	FOR(i, count)
	{
		res[i] = start;
		start += velocity*dT;

		FOR(i,dim) _testin(1+i) = start[i];
		_testout = sogp->predict(_testin);
		FOR(d, dim) velocity[d] = _testout(d+1,1);
	}
	return res;
}

fvec DynamicalGPR::Test( const fvec &sample )
{
    fvec res(dim);
	if(!sogp) return res;
	if(sample.size() < dim) return res;
	Matrix _testout;
	ColumnVector _testin(dim);
	FOR(i,dim)
	{
		_testin(1+i) = sample[i];
	}
	_testout = sogp->predict(_testin);
    FOR(d,dim) res[d] = _testout(d+1,1);
	return res;
}

fVec DynamicalGPR::Test( const fVec &sample )
{
	fVec res;
	if(!sogp) return res;
	Matrix _testout;
	ColumnVector _testin(dim);
	FOR(i,dim)
	{
		_testin(1+i) = sample._[i];
	}
	_testout = sogp->predict(_testin);
	res[0] = _testout(1,1);
	res[1] = _testout(2,1);
	return res;
}

float DynamicalGPR::GetLikelihood(float mean, float sigma, float point)
{
	const float sqrpi = 1.f/sqrtf(2.f*PIf);
	const float divider = sqrpi/sigma;
	const float exponent = -powf((point-mean)/sigma,2.f)*0.5;
	return expf(exponent)*divider;
}

void DynamicalGPR::Clear()
{
	bTrained = false;
	delete sogp;
	sogp = 0;
}

int DynamicalGPR::GetBasisCount()
{
	return sogp ? sogp->size() : 0;
}

fvec DynamicalGPR::GetBasisVector( int index )
{
	if(!sogp) return fvec();
	if(index > sogp->size()) return fvec();
	fvec res;
	res.resize(4,0);
	res[0] = sogp->BVloc(index, 0);
	res[1] = sogp->BVloc(index, 1);
	res[2] = sogp->alpha_acc(index, 0);
	res[3] = sogp->alpha_acc(index, 1);
	return res;
}

const char *DynamicalGPR::GetInfoString()
{
	char *text = new char[2048];
	sprintf(text, "Sparse Optimized Gaussian Processes\n");
	sprintf(text, "%sKernel: ", text);
	switch(kernelType)
	{
	case 0:
		sprintf(text, "%s rbf (gamma: %f)\n", text, param1);
		break;
	case 1:
		sprintf(text, "%s polynomial (deg: %f %f width: %f)\n", text, degree, param1);
		break;
	case 2:
		sprintf(text, "%s rbf (gamma: %f)\n", text, param1);
		break;
	}
	return text;
	sprintf(text, "%sNoise: %.3f\n", param2);
	sprintf(text, "%sBasis Functions: %d\n", text, GetBasisCount());
	return text;
}
