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
#ifndef _CLASSIFIER_GMM_H_
#define _CLASSIFIER_GMM_H_

#include <vector>
#include <map>
#include "classifier.h"
#include <fgmm/fgmm++.hpp>

class ClassifierGMM : public Classifier
{
public:
	std::vector<Gmm*> gmms;
	std::vector<float*> data;
private:
	u32 nbClusters;
	u32 covarianceType;
	u32 initType;
public:
	ClassifierGMM();
	~ClassifierGMM();
	void Train(std::vector< fvec > samples, ivec labels);
	float Test(const fvec &sample);
	float Test(const fVec &sample);
	fvec TestMulti(const fvec &sample);
    const char *GetInfoString();
    void SaveModel(std::string filename);
    bool LoadModel(std::string filename);

	void SetParams(u32 nbClusters, u32 covarianceType, u32 initType);
	void Update();
};

#endif // _CLASSIFIER_GMM_H_
