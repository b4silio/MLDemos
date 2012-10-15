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
#ifndef __FLAME_H
#define __FLAME_H

/*
 Since data for clustering are usually noisy,
 it is not very necessary to have EPSILON extremely small.
 */
#define EPSILON 1E-9

using namespace std;

typedef struct IndexFloat IndexFloat;
typedef struct Flame Flame;
typedef struct IntArray IntArray;

struct IntArray
{
	int *array;
	int  size;
	int  bufsize;
};

/* For sorting and storing the orignal indices. */
struct IndexFloat
{
	int   index;
	float value;
};

float L1_Euclidean_Norm( float *x, float *y, int m ); // Added
float Flame_Euclidean( float *x, float *y, int m );
float Flame_Cosine( float *x, float *y, int m );
float Flame_Pearson( float *x, float *y, int m );
float Flame_UCPearson( float *x, float *y, int m );
float Flame_SQPearson( float *x, float *y, int m );
float Flame_DotProduct( float *x, float *y, int m );
float Flame_Covariance( float *x, float *y, int m );
float Flame_Manhattan( float *x, float *y, int m );
float Flame_CosineDist( float *x, float *y, int m );
float Flame_PearsonDist( float *x, float *y, int m );
float Flame_UCPearsonDist( float *x, float *y, int m );
float Flame_SQPearsonDist( float *x, float *y, int m );
float Flame_DotProductDist( float *x, float *y, int m );
float Flame_CovarianceDist( float *x, float *y, int m );

enum DistSimTypes
{
	DST_USER = 0,
	DST_EUCLID ,
	DST_COSINE ,
	DST_PEARSON ,
	DST_UC_PEARSON ,
	DST_SQ_PEARSON ,
	DST_DOT_PROD ,
	DST_COVARIANCE ,
	DST_MANHATTAN ,
	DST_NULL
};
typedef float (*DistFunction)( float *x, float *y, int m );

extern const DistFunction basicDistFunctions[];

enum FlameObjectTypes
{
	OBT_NORMAL ,
	OBT_SUPPORT ,
	OBT_OUTLIER
};

struct Flame
{
	int simtype;

	/* Number of objects */
	int N;

	/* Number of K-Nearest Neighbors */
	int K;

	/* Upper bound for K defined as: sqrt(N)+10 */
	int KMAX;

        /* Number of used steps*/
        int steps;

	/* Stores the KMAX nearest neighbors instead of K nearest neighbors
	 * for each objects, so that when K is changed, weights and CSOs can be
	 * re-computed without referring to the original data.
	 */
	int   **graph;
	/* Distances to the KMAX nearest neighbors. */
	float **dists;

	/* Nearest neighbor count.
	 * it can be different from K if an object has nearest neighbors with
	 * equal distance. */
	int    *nncounts;
	float **weights;

	/* Number of identified Cluster Supporting Objects */
	int cso_count;
	char *obtypes;

	float **fuzzyships;
	
	/* Number of clusters including the outlier group */
	int count;
	/* The last one is the outlier group. */
	IntArray *clusters;
	
	DistFunction distfunc;
};

/* Create a structure for FLAME clustering, and set all fields to zero. */
Flame* Flame_New();

int Int_New();

/* Free allocated memory, and set all fields to zero. */
void Flame_Clear( Flame *self );

/* Set a NxM data matrix, and compute distances of type T.
 * 
 * If T==DST_USER or T>=DST_NULL, and Flame::distfunc member is set,
 * then Flame::distfunc is used to compute the distances;
 * Otherwise, Flame_Euclidean() is used. */
void Flame_SetDataMatrix( Flame *self, float *data[], int N, int M, int T );

/* Set a pre-computed NxN distance matrix. */
void Flame_SetDistMatrix( Flame *self, float *data[], int N );

/* Define knn-nearest neighbors for each object 
 * and the Cluster Supporting Objects (CSO). 
 * 
 * The actual number of nearest neighbors could be large than knn,
 * if an object has neighbors of the same distances.
 *
 * Based on the distances of the neighbors, a density can be computed
 * for each object. Objects with local maximum density are defined as
 * CSOs. The initial outliers are defined as objects with local minimum
 * density which is less than mean( density ) + thd * stdev( density );
 */
void Flame_DefineSupports( Flame *self, int knn, float thd );

/* Local Approximation of fuzzy memberships.
 * Stopped after the maximum steps of iterations;
 * Or stopped when the overall membership difference between
 * two iterations become less than epsilon. */
void Flame_LocalApproximation( Flame *self, int steps, float epsilon );

/* Construct clusters.
 * If 0<thd<1:
 *   each object is assigned to all clusters in which
 *   it has membership higher than thd; if it can not be assigned
 *   to any clusters, it is then assigned to the outlier group.
 * Else:
 *   each object is assigned to the group (clusters/outlier group)
 *   in which it has the highest membership. */
void Flame_MakeClusters( Flame *self, float thd );

#endif
