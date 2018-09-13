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
#include <clustererMeanShift.h>
#include <iostream>
#include <math.h>
#include <ostream>
#include <public.h>
#include <string>
#include <sstream>

using namespace std;

ClustererMeanShift::ClustererMeanShift()
    :meanShift(0), kernelWidth(0.1), mergeRadius(0.05), testMax(1), testCount(0)
{}

ClustererMeanShift::~ClustererMeanShift()
{
    DEL(meanShift);
}

void ClustererMeanShift::SetParams(float kernelWidth, float mergeRadius) {
    this->kernelWidth = kernelWidth;
    this->mergeRadius = mergeRadius;
}

bool ClustererMeanShift::SetClusterTestValue(int count, int max)
{
    testCount = count;
    testMax = max;
    return true;
}

void ClustererMeanShift::Train(std::vector<fvec> samples) {
    if(!samples.size()) return;
    dim = samples.front().size();
    float kernelW = kernelWidth;
    if(testMax > 1) {
        float ratio = (testCount) / (float)testMax;
        kernelW = kernelW*(1-ratio);
    }

    DEL(meanShift);
    meanShift = new MeanShift();
    points = vector<dvec>(samples.size());
    FOR(i, samples.size()) {
        points[i].resize(dim,0);
        FOR(d, dim) points[i][d] = samples[i][d];
    }
    clusters = meanShift->cluster(points, kernelW, mergeRadius);
    nbClusters = clusters.size();
}

fvec ClustererMeanShift::Test( const fvec &sample) {
    fvec res;
    res.resize(nbClusters, 0);
    if(clusters.empty()) return res;
    dvec point(sample.size());
    FOR(d, point.size()) point[d] = sample[d];
    float kernelW = kernelWidth;
    if(testMax > 1) {
        float ratio = (testCount) / (float)testMax;
        kernelW = kernelW*(1-ratio);
    }
    dvec outputPoint;
    meanShift->shift_point(point, points, kernelW, outputPoint);
    int closest = 0;
    float closestDistance = FLT_MAX;
    FOR(c, clusters.size()) {

        float distance = meanShift->distance(outputPoint, clusters[c].mode);
        if(distance < closestDistance) {
            closest = c;
            closestDistance = distance;
        }
    }
    res[closest] = 1;
    return res;
}

const char *ClustererMeanShift::GetInfoString() {
    stringstream s;
    s << "MeanShift\n\n";
    const char *result = s.str().c_str();
    return result;
}
