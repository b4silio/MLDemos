/*********************************************************************
FLAME Implementation in MLDemos
Copyright (C) Pierre-Antoine Sondag (pasondag@gmail.com) 2012

Based on the standard implementation of FLAME data clustering algorithm.
Copyright (C) 2007, Fu Limin (phoolimin@gmail.com).
All rights reserved.

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
#ifndef _CLUSTERER_FLAME_H_
#define _CLUSTERER_FLAME_H_

#include <boost/functional/hash.hpp>
#include <clusterer.h>
#include <flame.h>
#include <boost/unordered_map.hpp>
//#include <unordered_map>
#include <vector>

// generic hashing function
template <typename Container>
struct container_hash {
    std::size_t operator()(Container const& c) const {
        return boost::hash_range(c.begin(), c.end());
    }
};


class ClustererFlame : public Clusterer {
private:

    float **data;
    Flame *flameStructure;

    int knnParameter; // used by Flame_DefineSupports
    int knnMetric; //passed to Flame_SetDataMatrix

    int maxIterationsParameter; //used by Flame_MakeCluster
    float epsilonParameter; //used by Flame_MakeCluster

    bool isSeveralClasses; //used by Flame_MakeClusters
    float thresholdParameter;  //used by Flame_MakeClusters

    boost::unordered_map<fvec, vector<int>, container_hash<fvec> > resultMap;
    vector<fvec> supports;

    void PrintDone();


public:

    /** Constructor, instanciating everything that will be used */
    ClustererFlame(){
        data = NULL;
        flameStructure = NULL;
        knnParameter = 10;
        maxIterationsParameter = 100;
        epsilonParameter = 1e-6;
    }

    /** Deconstructor, deinstanciating everything that has been instanciated */
    ~ClustererFlame(){ }

    /** The training function, called by the main program, all training is here. */
    void Train(std::vector< fvec > samples);

    /**
    The testing function, returns a vector of size nbClusters,
    with the contribution/weight of the point for each cluster */
    fvec Test(const fvec &sample);

    /**
    Information string for the Algorithm Information and Statistics
    panel in the main program interface. Here you probably will put
    the number of parameters, the training time or anything else
    */
    const char *GetInfoString();

    /**
    Function to set the algorithm hyper-parameters, called prior to the
    training itself
    */
    void SetParams(
        int knnParameter,
        int knnMetric,
        int maxIterationsParameter,
        bool isSeveralClasses,
        float thresholdParameter);

    /* Once the treining is done, returns the vectors choosen to be supports. */
    vector<fvec> GetSupports();
};

#endif
