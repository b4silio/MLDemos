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
#ifndef _CLASSIFIER_BOOST_H_
#define _CLASSIFIER_BOOST_H_

#include <vector>
#include "classifier.h"
#include "basicOpenCV.h"

class ClassifierBoost : public Classifier
{
private:
	CvBoost *model;
	u32 weakCount;
	int weakType; // 0: random projection, 1: random rectangle
	float scoreMultiplier;
	ivec features;
public:
	ClassifierBoost();
	~ClassifierBoost();
	void Train(std::vector< fvec > samples, ivec labels);
	float Test(const fvec &sample);
    const char *GetInfoString();
	void SetParams(u32 weakCount, int weakType);
};

#endif // _CLASSIFIER_BOOST_H_
