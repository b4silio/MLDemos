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
#ifndef _CLASSIFIER_KPCA_H_
#define _CLASSIFIER_KPCA_H_

#include <vector>
#include <classifier.h>
#include "eigen_pca.h"

class ClassifierKPCA : public Classifier
{
private:
	PCA *pca;
	fvec mean;
	fvec minValues, maxValues;
	ivec labels;
	std::vector<fvec> samples;
	std::vector<fvec> results;
	int kernelType;
	int kernelDegree;
	float kernelGamma;
    float kernelOffset;
public:
	std::vector<fvec> Project(std::vector<fvec> samples);
    std::vector<fvec> GetSamples(){return samples;}
    std::vector<fvec> GetResults(){return results;}
    ivec GetLabels(){return labels;}

	ClassifierKPCA();
	void Train(std::vector< fvec > samples, ivec labels);
    float Test(const fvec &sample) const ;
    float Test(const fVec &sample) const ;
    const char *GetInfoString() const ;
    void SetParams(int kernelType, int kernelDegree, float kernelGamma, float kernelOffset);
};

#endif // _CLASSIFIER_KPCA_H_
