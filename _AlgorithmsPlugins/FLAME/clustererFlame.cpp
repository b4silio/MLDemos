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
#include <clustererFlame.h>
#include <iostream>
#include <math.h>
#include <ostream>
#include <public.h>
#include <QHash>
#include <string>
#include <sstream>
#include "flame.h"
#include "assert.h"

using namespace std;

void ClustererFlame::SetParams(
    int knnParameter,
    int knnMetric,
    int maxIterationsParameter,
    bool isSeveralClasses,
    float thresholdParameter) {
    this->knnParameter = knnParameter;
    this->knnMetric = knnMetric;
    this->maxIterationsParameter = maxIterationsParameter;
    this->isSeveralClasses = isSeveralClasses;
    this->thresholdParameter = thresholdParameter;
}

void ClustererFlame::PrintDone() {
    printf("done.\n");
    fflush(stdout);
}

void ClustererFlame::Train(std::vector<fvec> samples) {
    if(!samples.size()) {
        return;
    }

    resultMap.clear();

    int N = samples.size();
    int M = samples[0].size();

    // Preconditions. Fail-fast on errors.
    // All datapoints are of same dimention.
    for (int i = 0; i < N; i++) {
        assert(samples[i].size() == M);
    }

    printf("Making a defensive deepcopy of the data.");
    fflush(stdout);

    data = (float**) malloc( N * sizeof(float*) );

    for (int i = 0; i < N; i++) {
        data[i] = (float*) malloc( M * sizeof(float) );
        for (int j = 0; j < M; j++) {
            data[i][j] = samples[i][j];
        }
    }
    PrintDone();

    printf("Initialize Flame data structure: ");
    fflush(stdout);
    flameStructure = Flame_New();
    PrintDone();

    printf("Send data to Flame structure: ");
    fflush(stdout);
    Flame_SetDataMatrix(flameStructure, data, N, M, knnMetric);
    PrintDone();

    free(data);

    printf("Detecting Cluster Supporting Objects: ");
    fflush(stdout);
    Flame_DefineSupports(flameStructure, knnParameter, -2.0);
    nbClusters = flameStructure->cso_count + 1; //Adding 1 for outlier class.
    printf("done, found %i.\n", flameStructure->cso_count);

    printf("Propagating fuzzy memberships: ");
    fflush(stdout);
    Flame_LocalApproximation(flameStructure, maxIterationsParameter, 1e-6);
    PrintDone();

    printf("Defining clusters from fuzzy memberships: ");
    fflush(stdout);
    if (isSeveralClasses) {
        Flame_MakeClusters(flameStructure, thresholdParameter);
    } else {
        Flame_MakeClusters(flameStructure, -1);
    }
    PrintDone();

    printf( "Displaying results: " );

    for (int i = 0; i <= flameStructure->cso_count; i++) {
        if (i == flameStructure->cso_count) {
            printf("\nCluster outliers, with %6i members:\n",
                flameStructure->clusters[i].size);
        } else {
            printf("\nCluster %3i, with %6i members:\n",
                i+1,
                flameStructure->clusters[i].size );
        }

        for (int j = 0; j<flameStructure->clusters[i].size; j++) {
            printf( "%5i", flameStructure->clusters[i].array[j] );
            resultMap[(samples[flameStructure->clusters[i].array[j]])].push_back(i);
        }
        printf( "\n" );
    }

    for (int i = 0; i < flameStructure->N; i++) {
        if (flameStructure->obtypes[i] == OBT_SUPPORT) {
            supports.push_back(samples[i]);
            std::cout <<  i << ",";
        }
    }
    std::cout << "\n";
    fflush(stdout);
}

fvec ClustererFlame::Test( const fvec &sample) {
    if (resultMap.count(sample) < 1) {
        return fvec(1,0);
    }

    fvec res;
    res.resize(nbClusters, 0); // set everyone to 0
    for (int i = 0; i < resultMap[sample].size(); i++) {
        int index = resultMap[sample][i];
        // set to 1/(nb affected class) for classes to which sample belong
        res[index] = 1/resultMap[sample].size();
    }
    return res;
}

// Not always called properly by the main program :-(
const char *ClustererFlame::GetInfoString() {
    stringstream s;

    s << "Flame\n\n";
    s << "Support definition" << "\n";
    s << "KNN: " << knnParameter << "\n";

    s << "Cluster making" << "\n";
    s << "Iterations: " << flameStructure->steps << " over " << maxIterationsParameter << "\n";

    s << "# clusters/supports found: " << flameStructure->cso_count << " \n\n";

    for( int i=0; i<=flameStructure->cso_count; i++) {
        if( i == flameStructure->cso_count ) {
            s << "# outliers elements: " << flameStructure->clusters[i].size << "\n";
        } else {
            s << "# elements in cluster: " << (i + 1) << ": " << flameStructure->clusters[i].size << "\n";
        }
    }

    const char *result = s.str().c_str();
    return result;
}

vector<fvec> ClustererFlame::GetSupports() {
    return supports;
}
