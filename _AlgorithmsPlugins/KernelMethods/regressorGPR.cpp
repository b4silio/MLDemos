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
#include "regressorGPR.h"

void RegressorGPR::Train( std::vector<fvec> input, ivec labels)
{
	if(!input.size()) return;
    int dim = input[0].size()-1;
    Matrix inputs(dim, input.size());
    RowVector outputs(input.size());
    int oDim = outputDim != -1 ? min(outputDim, dim) : dim;
	FOR(n, input.size())
	{
        FOR(d, dim) inputs(d+1, n+1) = input[n][d];
        outputs(n+1) = input[n][oDim];
        if(outputDim != -1 && outputDim < dim) inputs(outputDim+1, n+1) = input[n][dim];
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
	//sogp = new SOGP(param1, param2);
	sogp->addM(inputs,outputs);

	bTrained = true;
}

fvec RegressorGPR::Test( const fvec &sample )
{
	fvec res;
	res.resize(2,0);
	if(!sogp) return res;
	double confidence;
	Matrix _testout;
    int dim = sogp->dim();
	ColumnVector _testin(dim);
	FOR(i,dim)
	{
		_testin(1+i) = sample[i];
	}
    if(outputDim != -1 && outputDim < dim) _testin(1+outputDim) = sample[dim];
	_testout = sogp->predict(_testin,confidence);
	res[0] = _testout(1,1);
	res[1] = confidence*confidence;
	return res;
}

fVec RegressorGPR::Test( const fVec &sample )
{
	fVec res;
	if(!sogp) return res;
	double confidence;
	Matrix _testout;
	ColumnVector _testin(2);
	FOR(i,2)
	{
		_testin(1+i) = sample._[i];
	}
	_testout = sogp->predict(_testin,confidence);
	res[0] = _testout(1,1);
	res[1] = confidence*confidence;
	return res;
}

float RegressorGPR::GetLikelihood(float mean, float sigma, float point)
{
	const float sqrpi = 1.f/sqrtf(2.f*PIf);
	const float divider = sqrpi/sigma;
	const float exponent = -powf((point-mean)/sigma,2.f)*0.5;
	return expf(exponent)*divider;
}

void RegressorGPR::Clear()
{
	bTrained = false;
	delete sogp;
	sogp = 0;
}

int RegressorGPR::GetBasisCount()
{
	return sogp ? sogp->size() : 0;
}

fvec RegressorGPR::GetBasisVector( int index )
{
	if(!sogp) return fvec();
	if(index > sogp->size()) return fvec();
    int dim = sogp->dim();
    fvec res(dim*2,0);
    FOR(d, dim)
    {
        res[d] = sogp->BVloc(index, d);
        res[dim + d] = sogp->alpha_acc(index, d);
    }
	return res;
}

char *RegressorGPR::GetInfoString()
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
	sprintf(text, "%sNoise: %.3f\n", text, param2);
	sprintf(text, "%sBasis Functions: %d\n", text, GetBasisCount());
	return text;
}
