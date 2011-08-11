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
#ifndef _CLUSTERER_XMeans_H_
#define _CLUSTERER_XMeans_H_

#include <vector>
#include <clusterer.h>
#include <map>

class ClustererQTClust : public Clusterer
{
public:
	std::vector<fvec> samples;
	ivec clusters;
	std::map<int,int> centerCnt;
	std::map<int,fvec> centers;

private:
	double distance;
	int minCount;
	float *data;
public:
	ClustererQTClust() : data(0), distance(0.6), minCount(2){};
	void Train(std::vector< fvec > samples);
	fvec Test( const fvec &sample);
	fvec Test( const fVec &sample);
	char *GetInfoString();

	void SetParams(double distance, int minCount);
};

#endif // _CLUSTERER_XMeans_H_
