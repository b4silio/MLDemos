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
#ifndef _REGRESSOR_SVR_H_
#define _REGRESSOR_SVR_H_

#include <vector>
#include <regressor.h>
#include "svm.h"

class RegressorSVR : public Regressor
{
private:
	svm_model *svm;
	svm_node *node;
public:
	svm_parameter param;
    bool bOptimize;

	RegressorSVR();
	~RegressorSVR();
	void Train(std::vector< fvec > samples, ivec labels);
	fvec Test( const fvec &sample);
	fVec Test(const fVec &sample);
    void Optimize(svm_problem *problem);
    const char *GetInfoString();

	void SetParams(int svmType, float svmC, float svmP, u32 kernelType, float kernelParam);
    svm_model *GetModel(){return svm;}
};

#endif // _REGRESSOR_SVR_H_
