/*********************************************************************
MLDemos: A User-Friendly visualization toolkit for machine learning
Copyright (C) 2010  Basilio Noris
Contact: mldemos@b4silio.com

Evolution-Strategy Mixture of Logisitics Regression
Copyright (C) 2011  Stephane Magnenat
Contact: stephane at magnenat dot net

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
#ifndef _CLASSIFIER_ES_MLR_H_
#define _CLASSIFIER_ES_MLR_H_

#include "classifier.h"

namespace MLR
{
	struct Classifier;
}

class ClassifierESMLR : public Classifier
{
	u32 cutCount;
	float alpha;
	u32 genCount;
	u32 indPerDim;
	MLR::Classifier* classifier;
	
public:
	ClassifierESMLR();
	~ClassifierESMLR();
	virtual void Train(std::vector< fvec > samples, ivec labels);
	virtual float Test(const fvec &sample);
	virtual char *GetInfoString();
	void SetParams(u32 cutCount, float alpha, u32 genCount, u32 indPerDim);
};

#endif // _CLASSIFIER_ES_MLR_H_
