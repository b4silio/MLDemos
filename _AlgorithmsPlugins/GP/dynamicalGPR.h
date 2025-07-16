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
#ifndef _DYNAMICAL_GPR_H_
#define _DYNAMICAL_GPR_H_

#include <vector>
#include "dynamical.h"
#include "SOGP.h"
#include <basicMath.h>
#include <mymaths.h>

class DynamicalGPR : public Dynamical
{
private:
	int dim;
	SOGP *sogp;
	bool bTrained;
	std::vector<fVec> bv;
	double param1, param2;
	int kernelType;
	int degree;
	int capacity;
	float GetLikelihood(float mean, float sigma, float point);

public:
	bool bShowBasis;
    DynamicalGPR() : sogp(0), dim(1), capacity(0), kernelType(kerRBF), bTrained(false), param1(1), param2(0.1), bShowBasis(false), degree(1){type = DYN_GPR;}
	void Train(std::vector< std::vector<fvec> > trajectories, ivec labels);
	std::vector<fvec> Test( const fvec &sample, const int count);
	fvec Test(const fvec &sample);
	fVec Test(const fVec &sample);
    const char *GetInfoString();

    void SetParams(double p1, double p2, int capacity, int kType, int d=1){param1=p1; param2=p2; kernelType=kType; degree = d;this->capacity=capacity;}
    SOGP *GetModel(){return sogp;}
	void Clear();
	fvec GetBasisVector(int index);
	int GetBasisCount();
};

#endif // _DYNAMICAL_GPR_H_
