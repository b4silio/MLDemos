/*********************************************************************
MLDemos: A User-Friendly visualization toolkit for machine learning
Copyright (C) 2010  Basilio Noris
Contact: mldemos@b4silio.com

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free
Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*********************************************************************/
#include "public.h"
#include "Clustering.h"
#include "vectorSpace.hpp"
#include "clustererQTClust.h"

using namespace std;
using namespace AG::Clustering;
using namespace AG::Data;

void ClustererQTClust::Train(std::vector< fvec > samples)
{
	if(!samples.size()) return;
	int dim = samples[0].size();

	this->samples = samples;
	centers.clear();
	centerCnt.clear();

	VectorSpace vs;
	Clusters clusters;

	FOR(s, samples.size())
	{
		Vect v(dim);
		FOR(i, dim) v(i) = samples[s][i];
		vs.push_back(v);
	}

	clusters = qt_clustering(vs, distance, minCount);
	this->clusters.resize(samples.size());
	FOR(i, clusters.size())
	{
		this->clusters[i] = clusters[i];
		if(centerCnt.count(clusters[i]))
		{
			centers[clusters[i]] += samples[i];
			centerCnt[clusters[i]]++;
		}
		else
		{
			centers[clusters[i]] = samples[i];
			centerCnt[clusters[i]] = 1;
		}
	}
	for(map<int,int>::iterator it = centerCnt.begin(); it != centerCnt.end(); it++)
	{
		int key = it->first;
		centers[key] /= it->second;
	}
}

fvec ClustererQTClust::Test( const fvec &sample)
{
	fvec res;
	res.resize(centers.size(),0);
	return res;
}

fvec ClustererQTClust::Test( const fVec &sample)
{
	fvec res;
	res.resize(centers.size(),0);
	return res;
}

void ClustererQTClust::SetParams(double distance, int minCount)
{
	this->distance = distance;
	this->minCount = minCount;
}

char *ClustererQTClust::GetInfoString()
{
	char *text = new char[1024];
	sprintf(text, "QT Clustering\n");
	sprintf(text, "%sMaximum Cluster Diameter: %f\n", text, distance);
	sprintf(text, "%sMinimum Samples per Cluster: %f\n", text, minCount);
	return text;
}
