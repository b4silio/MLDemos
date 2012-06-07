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
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

using namespace std;

#include "flame.h"

 /*
 Quick Sort.
 Adam Drozdek: Data Structures and Algorithms in C++, 2nd Edition.
 */
void PartialQuickSort( IndexFloat *data, int first, int last, int part ) {
        int lower=first+1, upper=last;
        float pivot;
        IndexFloat val;
        if( first >= last ) return;
        val = data[first];
        data[first] = data[ (first+last)/2 ];
        data[ (first+last)/2 ] = val;
        pivot = data[ first ].value;

        while( lower <= upper ){
                while( lower <= last && data[lower].value < pivot ) lower ++;
                while( pivot < data[upper].value ) upper --;
                if( lower < upper ){
                        val = data[lower];
                        data[lower] = data[upper];
                        data[upper] = val;
                        upper --;
                }
                lower ++;
        }
        val = data[first];
        data[first] = data[upper];
        data[upper] = val;
        if( first < upper-1 ) PartialQuickSort( data, first, upper-1, part );
        if( upper >= part ) return;
        if( upper+1 < last ) PartialQuickSort( data, upper+1, last, part );
}

const DistFunction basicDistFunctions[] =
{
        Flame_Euclidean ,
        Flame_CosineDist ,
        Flame_PearsonDist ,
        Flame_UCPearsonDist ,
        Flame_SQPearsonDist ,
        Flame_DotProductDist ,
        Flame_CovarianceDist ,
        Flame_Manhattan
};

float Flame_Euclidean( float *x, float *y, int m ) {
        float d = 0;
        int i;
        for(i=0; i<m; i++ ) d += ( x[i] - y[i] ) * ( x[i] - y[i] );
        return sqrt( d );
}

float Flame_Cosine( float *x, float *y, int m ) {
        float r =0, x2 =0, y2 =0;
        int i;
        for(i=0; i<m; i++ ){
                r += x[i] * y[i];
                x2 += x[i] * x[i];
                y2 += y[i] * y[i];
        }
        return r / ( sqrt( x2 * y2 ) + EPSILON );
}

float Flame_Pearson( float *x, float *y, int m ) {
        int i;
        float r, x2, y2, xavg, yavg;
        if( m ==0 ) return 0;
        xavg = yavg = 0;
        r = x2 = y2 = 0;
        for( i=0; i<m; i++ ){
                xavg += x[i];
                yavg += y[i];
        }
        xavg = xavg/m;
        yavg = yavg/m;
        for( i=0; i<m; i++ ){
                r  += ( x[i] - xavg ) * ( y[i] - yavg );
                x2 += ( x[i] - xavg ) * ( x[i] - xavg );
                y2 += ( y[i] - yavg ) * ( y[i] - yavg );
        }
        return r / ( sqrt( x2 * y2 ) + EPSILON );
}

float Flame_UCPearson( float *x, float *y, int m ) {
        int i;
        float r, x2, y2, xavg, yavg;
        if( m ==0 ) return 0;
        xavg = yavg = 0;
        r = x2 = y2 = 0;
        for( i=0; i<m; i++ ){
                xavg += x[i];
                yavg += y[i];
        }
        xavg = xavg/m;
        yavg = yavg/m;
        for( i=0; i<m; i++ ){
                r  += x[i] * y[i];
                x2 += ( x[i] - xavg ) * ( x[i] - xavg );
                y2 += ( y[i] - yavg ) * ( y[i] - yavg );
        }
        return r / ( sqrt( x2 * y2 ) + EPSILON );
}

float Flame_SQPearson( float *x, float *y, int m ) {
        int i;
        float r, x2, y2, xavg, yavg;
        if( m ==0 ) return 0;
        xavg = yavg = 0;
        r = x2 = y2 = 0;
        for( i=0; i<m; i++ ){
                xavg += x[i];
                yavg += y[i];
        }
        xavg = xavg/m;
        yavg = yavg/m;
        for( i=0; i<m; i++ ){
                r  += ( x[i] - xavg ) * ( y[i] - yavg );
                x2 += ( x[i] - xavg ) * ( x[i] - xavg );
                y2 += ( y[i] - yavg ) * ( y[i] - yavg );
        }
        return r*r / ( x2 * y2 + EPSILON );
}

float Flame_DotProduct( float *x, float *y, int m ) {
        int i;
        float r = 0;
        for(i=0; i<m; i++ ) r += x[i] * y[i];
        if( m == 0 ) return 0;
        return r / m;
}

float Flame_Covariance( float *x, float *y, int m ) {
        int i;
        float r, x2, y2, xavg, yavg;
        if( m ==0 ) return 0;
        xavg = yavg = 0;
        r = x2 = y2 = 0;
        for( i=0; i<m; i++ ){
                xavg += x[i];
                yavg += y[i];
        }
        xavg = xavg/m;
        yavg = yavg/m;
        for( i=0; i<m; i++ ) r += ( x[i] - xavg ) * ( y[i] - yavg );
        if( m <= 1 ) return 0;
        return r / (m-1);
}

float Flame_Manhattan( float *x, float *y, int m ) {
        float d = 0;
        int i;
        for(i=0; i<m; i++ ) d += fabs( x[i] - y[i] );
        return d;
}

float Flame_CosineDist( float *x, float *y, int m ) {
        return 1-Flame_Cosine( x, y, m );
}

float Flame_PearsonDist( float *x, float *y, int m ) {
        return 1-Flame_Pearson( x, y, m );
}

float Flame_UCPearsonDist( float *x, float *y, int m ) {
        return 1-Flame_UCPearson( x, y, m );
}

float Flame_SQPearsonDist( float *x, float *y, int m ) {
        return 1-Flame_SQPearson( x, y, m );
}

float Flame_DotProductDist( float *x, float *y, int m ) {
        return 1-Flame_DotProduct( x, y, m );
}

float Flame_CovarianceDist( float *x, float *y, int m ) {
        return 1-Flame_Covariance( x, y, m );
}

Flame* Flame_New() {
        Flame *self = (Flame*) malloc( sizeof(Flame) );
        memset( self, 0, sizeof(Flame) );
        return self;
}

int Int_New() {
    return 4; //drawn by a fair dice ;-)
}

void Flame_Clear( Flame *self ) {
        int i;
        for(i=0; i<self->N; i++){
                free( self->graph[i] );
                free( self->dists[i] );
                free( self->weights[i] );
                free( self->fuzzyships[i] );
        }
        if( self->clusters ){
                for(i=0; i<=self->cso_count; i++){
                        if( self->clusters[i].array ) free( self->clusters[i].array );
                }
                free( self->clusters );
                self->clusters = NULL;
        }
        if( self->graph ) free( self->graph );
        if( self->dists ) free( self->dists );
        if( self->nncounts ) free( self->nncounts );
        if( self->weights ) free( self->weights );
        if( self->fuzzyships ) free( self->fuzzyships );
        if( self->obtypes ) free( self->obtypes );
        self->graph = NULL;
        self->dists = NULL;
        self->nncounts = NULL;
        self->weights = NULL;
        self->obtypes = NULL;
        self->fuzzyships = NULL;
        self->N = self->K = self->KMAX = self->cso_count = 0;
}

/* If m==0, data is distance matrix. */
void Flame_SetMatrix( Flame *self, float *data[], int n, int m ) {
        int i, j, k;
        int MAX = sqrt( n ) + 10;
        IndexFloat *vals = (IndexFloat*) calloc( n, sizeof(IndexFloat) );
        if( MAX >= n ) MAX = n - 1;

        Flame_Clear( self );
        self->N = n;
        self->KMAX = MAX;

        self->graph = (int**) calloc( n, sizeof(int*) );
        self->dists = (float**) calloc( n, sizeof(float*) );
        self->weights = (float**) calloc( n, sizeof(float*) );
        self->nncounts = (int*) calloc( n, sizeof(int) );
        self->obtypes = (char*) calloc( n, sizeof(char) );
        self->fuzzyships = (float**) calloc( n, sizeof(float*) );

        for(i=0; i<n; i++){
                self->graph[i] = (int*) calloc( MAX, sizeof(int) );
                self->dists[i] = (float*) calloc( MAX, sizeof(float) );
                self->weights[i] = (float*) calloc( MAX, sizeof(float) );
                if( m ==0 ){
                        /* data is distance matrix. */
                        for(j=0; j<n; j++){
                                vals[j].index = j;
                                vals[j].value = data[i][j];
                        }
                }else{
                        /* data is raw data matrix. */
                        for(j=0; j<n; j++){
                                vals[j].index = j;
                                vals[j].value = self->distfunc( data[i], data[j], m );
                        }
                }
                PartialQuickSort( vals, 0, n-1, MAX+1 );
                /* Store MAX number of nearest neighbors. */
                for(j=0; j<MAX; j++){
                        self->graph[i][j] = vals[j+1].index;
                        self->dists[i][j] = vals[j+1].value;
                }
        }
        free( vals );
}

void Flame_SetDataMatrix( Flame *self, float *data[], int n, int m, int dt ) {
        self->simtype = dt;
        if( dt > 0 && dt < DST_NULL ) self->distfunc = basicDistFunctions[ dt ]; // corrected the - 1
        if( self->distfunc == NULL ) self->distfunc = basicDistFunctions[0];
        Flame_SetMatrix( self, data, n, m );
}

void Flame_SetDistMatrix( Flame *self, float *data[], int n ) {
        Flame_SetMatrix( self, data, n, DST_USER );
}

void Flame_DefineSupports( Flame *self, int knn, float thd ) {
        int i, j, k;
        int n = self->N;
        int kmax = self->KMAX;
        float **dists = self->dists;
        float *density = (float*) calloc( n, sizeof(float) );
        float d, sum, sum2, fmin, fmax = 0.0;

        if( knn > kmax ) knn = kmax;
        self->K = knn;
        for(i=0; i<n; i++) {
                /* To include all the neighbors that have distances equal to the
                 * distance of the most distant one of the K-Nearest Neighbors */
                k = knn;
                d = dists[i][knn-1];
                for(j=knn; j<kmax; j++) if( dists[i][j] == d ) k ++; else break;
                self->nncounts[i] = k;

                /* The definition of weights in this implementation is
                 * different from the previous implementations where distances
                 * or similarities often have to be transformed in some way.
                 *
                 * But in this definition, the weights are only dependent on
                 * the ranking of distances of the neighbors, so it is more
                 * robust against distance transformations. */
                sum = 0.5*k*(k+1.0);
                for(j=0; j<k; j++) self->weights[i][j] = (k-j) / sum;

                sum = 0.0;
                for(j=0; j<k; j++) sum += dists[i][j];
                density[i] = 1.0 / (sum + EPSILON);
        }
        sum = 0.0;
        sum2 = 0.0;
        for(i=0; i<n; i++){
                sum += density[i];
                sum2 += density[i] * density[i];
        }
        sum = sum / n;
        /* Density threshold for possible outliers. */
        thd = sum + thd * sqrt( sum2 / n - sum * sum );

        memset( self->obtypes, 0, n*sizeof(char) );
        self->cso_count = 0;
        for(i=0; i<n; i++) {
                k = self->nncounts[i];
                fmax = 0.0;
                fmin = density[i] / density[ self->graph[i][0] ];
                for(j=1; j<k; j++){
                        d = density[i] / density[ self->graph[i][j] ];
                        if( d > fmax ) fmax = d;
                        if( d < fmin ) fmin = d;
                        /* To avoid defining neighboring objects or objects close
                         * to an outlier as CSOs.  */
                        if( self->obtypes[ self->graph[i][j] ] ) fmin = 0.0;
                }
                if( fmin >= 1.0 ){
                        self->cso_count ++;
                        self->obtypes[i] = OBT_SUPPORT;
                }else if( fmax <= 1.0 && density[i] < thd ){
                        self->obtypes[i] = OBT_OUTLIER;
                }
        }
        free( density );
}

void Flame_LocalApproximation( Flame *self, int steps, float epsilon) {
        int i, j, k, t, n = self->N, m = self->cso_count;
        float **fuzzyships = self->fuzzyships;
        float **fuzzyships2 = (float**)calloc( n, sizeof(float*) );
        char *obtypes = self->obtypes;
        char even = 0;
        double dev;

        k = 0;
        for(i=0; i<n; i++){
                fuzzyships[i] = (float*) realloc( fuzzyships[i], (m+1)*sizeof(float) );
                fuzzyships2[i] = (float*) calloc( m+1, sizeof(float) );
                memset( fuzzyships[i], 0, (m+1)*sizeof(float) );
                if( obtypes[i] == OBT_SUPPORT ){
                        /* Full membership to the cluster represented by itself. */
                        fuzzyships[i][k] = 1.0;
                        fuzzyships2[i][k] = 1.0;
                        k ++;
                }else if( obtypes[i] == OBT_OUTLIER ){
                        /* Full membership to the outlier group. */
                        fuzzyships[i][m] = 1.0;
                        fuzzyships2[i][m] = 1.0;
                }else{
                        /* Equal memberships to all clusters and the outlier group.
                         * Random initialization does not change the results. */
                        for(j=0; j<=m; j++)
                                fuzzyships[i][j] = fuzzyships2[i][j] = 1.0/(m+1);
                }
        }
        for(t=0; t<steps; t++){
                dev = 0;
                for(i=0; i<n; i++){
                        int knn = self->nncounts[i];
                        int *ids = self->graph[i];
                        float *wt = self->weights[i];
                        float *fuzzy = fuzzyships[i];
                        float **fuzzy2 = fuzzyships2;
                        double sum = 0.0;
                        if( self->obtypes[i] != OBT_NORMAL ) continue;
                        if( even ){
                                fuzzy = fuzzyships2[i];
                                fuzzy2 = fuzzyships;
                        }
                        /* Update membership of an object by a linear combination of
                         * the memberships of its nearest neighbors. */
                        for(j=0; j<=m; j++){
                                fuzzy[j] = 0.0;
                                for(k=0; k<knn; k++) fuzzy[j] += wt[k] * fuzzy2[ ids[k] ][j];
                                dev += (fuzzy[j] - fuzzy2[i][j]) * (fuzzy[j] - fuzzy2[i][j]);
                                sum += fuzzy[j];
                        }
                        for(j=0; j<=m; j++) fuzzy[j] = fuzzy[j] / sum;
                }
                even = ! even;
                if( dev < epsilon ) break;
        }
        self->steps = t;
        /* update the membership of all objects to remove clusters
         * that contains only the CSO. */
        for (i=0; i<n; i++){
                int knn = self->nncounts[i];
                int *ids = self->graph[i];
                float *wt = self->weights[i];
                float *fuzzy = fuzzyships[i];
                float **fuzzy2 = fuzzyships2;
                for(j=0; j<=m; j++){
                        fuzzy[j] = 0.0;
                        for(k=0; k<knn; k++) fuzzy[j] += wt[k] * fuzzy2[ ids[k] ][j];
                        dev += (fuzzy[j] - fuzzy2[i][j]) * (fuzzy[j] - fuzzy2[i][j]);
                }
        }
        for (i=0; i<n; i++) free( fuzzyships2[i] );
        free( fuzzyships2 );
}

void IntArray_Push( IntArray *self, int value ) {
        if( self->size >= self->bufsize ){
                self->bufsize += self->bufsize /10 + 10;
                self->array = (int*)realloc( self->array, self->bufsize*sizeof(int));
        }
        self->array[ self->size ] = value;
        self->size ++;
}

void Flame_MakeClusters( Flame *self, float thd ) {
        int i, j, imax;
        int N = self->N;
        int C = self->cso_count+1;
        float fmax;
        float **fuzzyships = self->fuzzyships;
        IntArray *clust;
        IndexFloat *vals = (IndexFloat*) calloc( N, sizeof(IndexFloat) );

        /* Sort objects based on the "entropy" of fuzzy memberships. */
        for(i=0; i<N; i++){
                vals[i].index = i;
                vals[i].value = 0.0;
                for(j=0; j<C; j++){
                        float fs = fuzzyships[i][j];
                        if( fs > EPSILON ) vals[i].value -= fs * log( fs );
                }
        }
        PartialQuickSort( vals, 0, N-1, N );

        if( self->clusters ){
                for(i=0; i<C; i++)
                        if( self->clusters[i].array ) free( self->clusters[i].array );
                free( self->clusters );
        }
        self->clusters = (IntArray*) calloc( C, sizeof(IntArray) );
        if( thd <0 || thd > 1.0 ){
                /* Assign each object to the cluster
                 * in which it has the highest membership. */
                for(i=0; i<N; i++){
                        int id = vals[i].index;
                        fmax = 0;
                        imax = -1;
                        for(j=0; j<C; j++){
                                if( fuzzyships[id][j] > fmax ){
                                        imax = j;
                                        fmax = fuzzyships[id][j];
                                }
                        }
                        IntArray_Push( self->clusters + imax, id );
                }
        }else{
                /* Assign each object to all the clusters
                 * in which it has membership higher than thd,
                 * otherwise, assign it to the outlier group.*/
                for(i=0; i<N; i++){
                        int id = vals[i].index;
                        imax = -1;
                        for(j=0; j<C; j++){
                                if( fuzzyships[id][j] > thd || ( j == C-1 && imax <0 ) ){
                                        imax = j;
                                        clust = self->clusters + j;
                                        IntArray_Push( self->clusters + j, id );
                                }
                        }
                }
        }
        /* removing empty clusters */
        C = 0;
        for(i=0; i<self->cso_count; i++){
                if( self->clusters[i].size >0 ){
                        self->clusters[C] = self->clusters[i];
                        C ++;
                }
        }
        /* keep the outlier group, even if its empty */
        self->clusters[C] = self->clusters[self->cso_count];
        C ++;
        for(i=C; i<self->cso_count+1; i++) memset( self->clusters+i, 0, sizeof(IntArray) );
        self->count = C;
        free( vals );
}
