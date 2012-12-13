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
#ifndef _REGRESSOR_GMM_H_
#define _REGRESSOR_GMM_H_

#include <vector>
#include "regressor.h"
#include "fgmm/fgmm++.hpp"

class RegressorGMR : public Regressor
{
public:
	Gmm *gmm;
private:
	u32 nbClusters;
	u32 covarianceType;
	u32 initType;
	float *data;
public:
    RegressorGMR() : gmm(0), data(0), nbClusters(2), covarianceType(2), initType(1){type = REGR_GMR;}
	void Train(std::vector< fvec > samples, ivec labels);
	fvec Test( const fvec &sample);
	fVec Test( const fVec &sample);
    const char *GetInfoString();
    void SaveModel(std::string filename);
    bool LoadModel(std::string filename);

	void SetParams(u32 nbClusters, u32 covarianceType, u32 initType);
};

#endif // _REGRESSOR_GMM_H_
