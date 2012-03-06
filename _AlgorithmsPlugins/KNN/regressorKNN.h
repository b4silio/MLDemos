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
#ifndef _REGRESSOR_KNN_H_
#define _REGRESSOR_KNN_H_

#include <vector>
#include "regressor.h"
#include "ANN/ANN.h"

class RegressorKNN : public Regressor
{
private:
	int					nPts;					// actual number of data points
	ANNpointArray		dataPts;				// data points
	ANNidxArray			nnIdx;					// near neighbor indices
	ANNdistArray		dists;					// near neighbor distances
	ANNkd_tree*			kdTree;					// search structure
	int metricType;
	int metricP;
	int k;
public:
    RegressorKNN(): k(1), nPts(0), dataPts(0), nnIdx(0), dists(0), kdTree(0), metricType(2), metricP(2){type = REGR_KNN;}
	~RegressorKNN();
	void Train(std::vector< fvec > samples, ivec labels);
	fvec Test( const fvec &sample);
	fVec Test( const fVec &sample);
    const char *GetInfoString();

	void SetParams(u32 k, int metricType, u32 metricP);
};

#endif // _REGRESSOR_KNN_H_
