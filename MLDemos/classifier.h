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
#ifndef _CLASSIFIER_H_
#define _CLASSIFIER_H_

#include <vector>
#include "public.h"
#include "types.h"
#include "roc.h"

extern "C" enum {CLASS_SVM, CLASS_RVM, CLASS_PEG, CLASS_EM, CLASS_GMM, CLASS_KNN, CLASS_BOOST, CLASS_MLP, CLASS_LINEAR, CLASS_PERC, CLASS_NONE} classifierType;

class Classifier
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
        bool bSingleClass;
        bool bUsesDrawTimer;

public:
	std::vector<fvec> crossval;
	fvec fmeasures;
	int type;
	std::vector< std::vector<f32pair> > rocdata;
        std::vector<const char *> roclabels;

        Classifier(): posClass(0), bFixedThreshold(true), classThresh(0.5f), classSpan(0.1f), bSingleClass(true), bUsesDrawTimer(true),type(CLASS_NONE)
	{
		rocdata.push_back(std::vector<f32pair>());
		rocdata.push_back(std::vector<f32pair>());
		roclabels.push_back("training");
		roclabels.push_back("testing");
	};

	virtual ~Classifier(){};
	std::vector <fvec> GetSamples(){return samples;};

	virtual void Train(std::vector< fvec > samples, ivec labels){};
	virtual float Test(const fvec &sample){ return 0; };
	virtual float Test(const fVec &sample){ return Test((fvec)sample); };
	virtual char *GetInfoString(){return NULL;};
	bool SingleClass(){return bSingleClass;};
        bool UsesDrawTimer(){return bUsesDrawTimer;};
};

#endif // _CLASSIFIER_H_
