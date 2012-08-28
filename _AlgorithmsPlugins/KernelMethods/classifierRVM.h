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
#ifndef _CLASSIFIER_RVM_H_
#define _CLASSIFIER_RVM_H_

#include <vector>
#include <classifier.h>
#include "dlib/svm.h"
#include "dlibTypes.h"

class ClassifierRVM : public Classifier
{
private:
	float epsilon;
	int kernelType; // 0: linear, 1: poly, 2: rbf
	float kernelParam;
	int kernelDegree;

    int kernelTypeTrained;
    void *decFunction;

public:

    ClassifierRVM():epsilon(0.001), kernelType(2), decFunction(0){}
    ~ClassifierRVM();

	void Train(std::vector< fvec > samples, ivec labels);
	float Test(const fvec &sample);
    const char *GetInfoString();
	void SetParams(float epsilon, int kernelType, float kernelParam, int kernelDegree)
        {this->epsilon=epsilon;this->kernelType=kernelType;this->kernelParam=kernelParam;this->kernelDegree=kernelDegree;}
	std::vector<fvec> GetSVs();

    template <int N> void KillDim();
    template <int N> void TrainDim(std::vector< fvec > _samples, ivec _labels);
    template <int N> float TestDim(const fvec &sample);
    template <int N> std::vector<fvec> GetSVsDim();
};

#endif // _CLASSIFIER_RVM_H_
