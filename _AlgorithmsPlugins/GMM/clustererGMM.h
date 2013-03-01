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
#ifndef _CLUSTERER_GMM_H_
#define _CLUSTERER_GMM_H_

#include <vector>
#include <clusterer.h>
#include "fgmm/fgmm++.hpp"

class ClustererGMM : public Clusterer
{
public:
	Gmm *gmm;
private:
	u32 covarianceType;
	u32 initType;
	float *data;
public:
    ClustererGMM() : gmm(0), data(0), covarianceType(2), initType(1){}
    ~ClustererGMM();
	void Train(std::vector< fvec > samples);
	fvec Test( const fvec &sample);
	fvec Test( const fVec &sample);
    const char *GetInfoString();
    float GetLogLikelihood(std::vector<fvec> samples);
    float GetParameterCount();
	void SetParams(u32 nbClusters, u32 covarianceType, u32 initType);
};

#endif // _CLUSTERER_GMM_H_
