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
#ifndef _REGRESSOR_KRLS_H_
#define _REGRESSOR_KRLS_H_

#include <vector>
#include <regressor.h>
#include "dlib/svm.h"
#include "dlibTypes.h"

class RegressorKRLS : public Regressor
{
private:
	dlib::krls<reg_lin_kernel> *linTrainer;
	dlib::krls<reg_pol_kernel> *polTrainer;
	dlib::krls<reg_rbf_kernel> *rbfTrainer;
    reg_lin_func linFunc;
	reg_pol_func polFunc;
	reg_rbf_func rbfFunc;
	std::vector<reg_sample_type> samples;
	std::vector<double> labels;

	float epsilon;
	int kernelType; // 0: linear, 1: poly, 2: rbf
	float kernelParam;
	int kernelDegree;
	int capacity;

public:

    RegressorKRLS(): linTrainer(0), polTrainer(0), rbfTrainer(0), capacity(0), epsilon(0.001), kernelType(2){type = REGR_KRLS;}
	~RegressorKRLS();
	void Train(std::vector< fvec > samples, ivec labels);
	fvec Test( const fvec &sample);
	fVec Test(const fVec &sample);
    const char *GetInfoString();

	void SetParams(float epsilon, int capacity, int kernelType, float kernelParam, int kernelDegree)
    {this->epsilon=epsilon;this->capacity=capacity;this->kernelType=kernelType;this->kernelParam=kernelParam;this->kernelDegree=kernelDegree;}
	std::vector<fvec> GetSVs();
};


#endif // _REGRESSOR_KRLS_H_
