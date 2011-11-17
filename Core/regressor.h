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
#ifndef _REGRESSOR_H_
#define _REGRESSOR_H_

#include <vector>
#include "mymaths.h"

extern "C" enum {REGR_SVR, REGR_RVM, REGR_GMR, REGR_GPR, REGR_KNN, REGR_MLP, REGR_LINEAR, REGR_LWPR, REGR_KRLS, REGR_NONE} regressorType;

class Regressor
{
protected:
	std::vector< fvec > samples;
	ivec classes;
	ivec labels;
	std::vector< fvec >testSamples;
	ivec testClasses;
	ivec testLabels;
	u32 dim;
	u32 posClass;
	f32 classThresh;
	f32 classSpan;
	s32 class2labels[255];
	ivec labels2class;
	bool bFixedThreshold;

public:
	std::vector<fvec> crossval;
	fvec fmeasures;
	fvec trainErrors, testErrors;
	int type;
    int outputDim;

    Regressor() : posClass(0), bFixedThreshold(true), classThresh(0.5f), classSpan(0.1f), outputDim(-1), type(REGR_NONE){}
    std::vector <fvec> GetSamples(){return samples;}
    void SetOutputDim(int outputDim){this->outputDim = outputDim;}

    virtual void Train(std::vector< fvec > samples, ivec labels){}
    virtual fvec Test( const fvec &sample){ return fvec(); }
    virtual fVec Test(const fVec &sample){ if (dim==2) return fVec(Test((fvec)sample)); fvec s = (fvec)sample; s.resize(dim,0); return Test(s);}
    virtual char *GetInfoString(){return NULL;}
};

#endif // _REGRESSOR_H_
