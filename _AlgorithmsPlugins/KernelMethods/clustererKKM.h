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
#ifndef _CLUSTERER_KKM_H_
#define _CLUSTERER_KKM_H_

#include <vector>
#include <clusterer.h>
#include "dlib/svm.h"
#include "dlib/rand.h"
#include "dlibTypes.h"

class ClustererKKM : public Clusterer
{
private:
	int kernelType;
	float kernelGamma;
    float kernelDegree;
    float kernelOffset;

	int maxVectors;

    int kernelTypeTrained;
    void *decFunction;

public:

    ClustererKKM() : decFunction(NULL), kernelType(2), kernelGamma(0.01), kernelDegree(2), maxVectors(8) {}
    ~ClustererKKM();

	void Train(std::vector< fvec > samples);

    template <int N> void KillDim();
    template <int N> void TrainDim(std::vector< fvec > _samples);
    template <int N> fvec TestDim(const fvec &sample);
    template <int N> double TestScoreDim(const fvec &sample, int index);
    template <int N> fvec TestUnnormalizedDim(const fvec &sample);
    fvec Test( const fvec &sample);
    double TestScore(const fvec &_sample, const int index);
    fvec TestUnnormalized( const fvec &sample);
    const char *GetInfoString();

    void SetParams(int clusters, int kernelType, float kernelGamma, int kernelDegree, float kernelOffset)
    {this->nbClusters=clusters;this->kernelType=kernelType;this->kernelGamma=kernelGamma;this->kernelDegree=kernelDegree; this->kernelOffset=kernelOffset;}
};

#endif // _CLUSTERER_KKM_H_
