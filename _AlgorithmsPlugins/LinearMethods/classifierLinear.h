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
#ifndef _CLASSIFIER_LINEAR_H_
#define _CLASSIFIER_LINEAR_H_

#include <vector>
#include <mymaths.h>
#include "classifier.h"
#include "basicMath.h"
#include "basicOpenCV.h"

class ClassifierLinear : public Classifier
{
private:
	fvec meanPos, meanNeg;
	int linearType;
	fVec W;
	int threshold;
	double* Transf;

	void GetCovariance(const std::vector<fvec> &samples, const fvec &mean, float ***covar);
	void TrainPCA(std::vector< fvec > samples, const ivec &labels);
	void TrainLDA(std::vector< fvec > samples, const ivec &labels, bool bFisher=true);
	void TrainICA(std::vector< fvec > samples, const ivec &labels);

public:
	ClassifierLinear() : threshold(0), linearType(0), Transf(0) {type = CLASS_LINEAR;};
	void Train(std::vector< fvec > samples, ivec labels);
	float Test(const fvec &sample);
	char *GetInfoString();
	fvec Project(const fvec &sample);
	fvec InvProject(const fvec &sample);
	void SetParams(u32 linearType);
	fvec GetMean(bool positive=true){return positive ? meanPos : meanNeg;};
	fVec GetW(){return W;};
	int GetType(){return linearType;};
};

#endif // _CLASSIFIER_LINEAR_H_
