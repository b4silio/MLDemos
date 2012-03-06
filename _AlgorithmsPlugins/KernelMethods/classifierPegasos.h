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
#ifndef _CLASSIFIER_PEGASOS_H_
#define _CLASSIFIER_PEGASOS_H_

#include <vector>
#include "classifier.h"
#include "dlib/svm.h"
#include "dlibTypes.h"

class ClassifierPegasos : public Classifier
{
private:
	dlib::svm_pegasos<lin_kernel> linTrainer;
	dlib::svm_pegasos<pol_kernel> polTrainer;
	dlib::svm_pegasos<rbf_kernel> rbfTrainer;
	lin_func linFunc;
	pol_func polFunc;
	rbf_func rbfFunc;

	float lambda;
	int kernelType; // 0: linear, 1: poly, 2: rbf
	float kernelParam;
	int kernelDegree;
	int maxSV;

public:

    ClassifierPegasos():maxSV(10),lambda(0.001), kernelType(2){}
	void Train(std::vector< fvec > samples, ivec labels);
	float Test(const fvec &sample);
	float Test(const fVec &sample);
    const char *GetInfoString();
	void SetParams(float lambda, int maxSV, int kernelType, float kernelParam, int kernelDegree)
        {this->lambda = lambda; this->maxSV = maxSV; this->kernelType=kernelType;this->kernelParam=kernelParam;this->kernelDegree=kernelDegree;}
	std::vector<fvec> GetSVs();
};

#endif // _CLASSIFIER_PEGASOS_H_
