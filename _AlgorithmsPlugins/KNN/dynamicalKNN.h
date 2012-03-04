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
#ifndef _DYNAMICAL_KNN_H_
#define _DYNAMICAL_KNN_H_

#include <vector>
#include "dynamical.h"
#include "ANN/ANN.h"

class DynamicalKNN : public Dynamical
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
	std::vector<fvec> points;
	std::vector<fvec> velocities;
public:
    DynamicalKNN(): k(1), nPts(0), dataPts(0), nnIdx(0), dists(0), kdTree(0), metricType(2), metricP(2){type = DYN_KNN;}
	~DynamicalKNN();
	void Train(std::vector< std::vector<fvec> > trajectories, ivec labels);
	std::vector<fvec> Test( const fvec &sample, const int count);
	fvec Test( const fvec &sample);
	fVec Test( const fVec &sample);
    const char *GetInfoString();

	void SetParams(u32 k, int metricType, u32 metricP);
};

#endif // _DYNAMICAL_KNN_H_
