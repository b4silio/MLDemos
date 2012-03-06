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
#ifndef _CLUSTERING_H_
#define _CLUSTERING_H_

#include <vector>
#include "mymaths.h"

class Clusterer
{
protected:
	u32 dim;
    u32 nbClusters;
	bool bIterative;

public:
    Clusterer() : dim(2), bIterative(false), nbClusters(1) {}
    virtual ~Clusterer(){}
    void Cluster(std::vector< fvec > allsamples) {Train(allsamples);}
    void SetIterative(bool iterative){bIterative = iterative;}
    int NbClusters(){return nbClusters;}

    virtual void Train(std::vector< fvec > samples){}
    virtual fvec Test( const fvec &sample){ return fvec(); }
    virtual fvec Test(const fVec &sample){ return Test((fvec)sample); }
    virtual const char *GetInfoString(){ return NULL; }
    virtual void SetNbClusters(int count){ nbClusters = count; }
};

#endif // _CLUSTERING_H_
