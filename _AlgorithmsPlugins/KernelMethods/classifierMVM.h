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
#ifndef _CLASSIFIER_MVM_H_
#define _CLASSIFIER_MVM_H_

#include <vector>
#include <map>
#include <classifier.h>
#include "svm.h"

class ClassifierMVM : public Classifier
{
private:
    float **SVs;
    float *alpha;
    float b;
    int svCount;

public:
    std::vector< fvec > manualSamples;
    ivec manualLabels;
    int kernel_type;
    int degree;				/* for poly */
    double gamma;			/* for poly/rbf/sigmoid */
    double coef0;			/* for poly/sigmoid */
    ivec indices;
    fvec alphas;

    ClassifierMVM();
    ~ClassifierMVM();
	void Train(std::vector< fvec > samples, ivec labels);
    float Test(const fvec &sample);
    const char *GetInfoString();
    void SetParams(u32 kernelType, float kernelParam, ivec indices, fvec alphas);
};

#endif // _CLASSIFIER_SVM_H_
