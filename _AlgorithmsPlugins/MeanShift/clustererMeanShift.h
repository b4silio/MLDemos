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
#ifndef _CLUSTERER_MEAN_SHIFT_H_
#define _CLUSTERER_MEAN_SHIFT_H_

#include <clusterer.h>
#include "MeanShift/MeanShift.h"

class ClustererMeanShift : public Clusterer {
public:

    ClustererMeanShift();
    ~ClustererMeanShift();

    void Train(std::vector< fvec > samples);
    fvec Test(const fvec &sample);
    const char *GetInfoString();
    void SetParams(float kernelWidth, float mergeRadius);
    void SetClusterTestValue(int count, int max);

    MeanShift* meanShift;
    std::vector<MeanShiftCluster> clusters;
    std::vector<dvec> points;
    float kernelWidth;
    float mergeRadius;
    float testCount;
    float testMax;

};

#endif // _CLUSTERER_MEAN_SHIFT_H_
