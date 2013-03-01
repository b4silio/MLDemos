/*********************************************************************
MLDemos: A User-Friendly visualization toolkit for machine learning
Copyright (C) 2010  Basilio Noris
Contact: mldemos@b4silio.com

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public License,
version 3 as published by the Free Software Foundation.

This library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free
Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*********************************************************************/
#ifndef _REGRESSOR_GPR_H_
#define _REGRESSOR_GPR_H_

#include <vector>
#include <regressor.h>
#include "SOGP.h"
#include "basicMath.h"
#include <mymaths.h>

class RegressorGPR : public Regressor
{
private:
	int dim;
	bool bTrained;
	std::vector<fVec> bv;
	double param1, param2;
	int kernelType;
	int degree;
	int capacity;
    bool bOptimize;
    bool bOptimizeLikelihood;

public:
	SOGP *sogp;
	bool bShowBasis;
    RegressorGPR() : sogp(0), dim(1), capacity(0), kernelType(kerRBF), bTrained(false), param1(1), param2(0.1), bShowBasis(false), degree(1), bOptimize(false){type = REGR_GPR;}
	void Train(std::vector<fvec> inputs, ivec labels);
	fvec Test(const fvec &sample);
	fVec Test(const fVec &sample);
    const char *GetInfoString();

    void SetParams(double p1, double p2, int capacity, int kType, int d=1, bool bOptimize=false, bool bOptimizeLikelihood=true){param1=p1; param2=p2; kernelType=kType; degree = d;this->capacity=capacity;this->bOptimize=bOptimize;this->bOptimizeLikelihood=bOptimizeLikelihood;}
    SOGP *GetModel(){return sogp;}
	void Clear();
	fvec GetBasisVector(int index);
	int GetBasisCount();
	float GetLikelihood(float mean, float sigma, float point);
    void Optimize(const Matrix &inputs, const Matrix &outputs);
};

#endif // _REGRESSOR_SVR_H_
