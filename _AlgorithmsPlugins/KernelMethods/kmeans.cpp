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
#include <stdio.h>
#include <vector>
#include "public.h"
#include "basicMath.h"
#include <mymaths.h>
#include "kmeans.h"
#include <QTime>
#include <QDebug>

using namespace std;


KMeansCluster::KMeansCluster(u32 cnt)
    : clusters(cnt), sigma(NULL), pi(NULL), bGMM(false), bSoft(false), beta(1), dim(2), power(2), plusPlus(true)
{
    InitClusters();
}

KMeansCluster::~KMeansCluster()
{
    Clear();
}

float KMeansCluster::Distance( fvec a, fvec b )
{
    fvec dif = a-b;
    float d = 0;
    if(power == 0) // infinite distance
    {
        FOR(i, dif.size()) d = max(d, abs(dif[i]));
    }
    else if(power == 1) // manhattan distance
    {
        FOR(i, dif.size()) d += abs(dif[i]);
    }
    else if(power == 2)
    {
        d = dif*dif;
    }
    else if(power > 2)
    {
        FOR(i, dif.size())
        {
            float p = abs(dif[i]);
            float p2 = 1;
            FOR(j, power) p2 *= p;
            d += p2;
        }
    }
    return d;
}

float KMeansCluster::Distance2( fvec a, fvec b )
{
    float d = 0;
    if(power == 0) // infinite distance
    {
        d = max(abs(a[0]-b[0]), abs(a[1]-b[1]));
    }
    else if(power == 1) // manhattan distance
    {
        d = abs(a[0]-b[0]) + abs(a[1]-b[1]);
    }
    else if(power == 2)
    {
        float d0 = a[0]-b[0];
        float d1 = a[1]-b[1];
        d = d0*d0 + d1*d1;
    }
    else if(power > 2)
    {
        float d0 = abs(a[0]-b[0]);
        float d1 = abs(a[1]-b[1]);
        float p0 = 1;
        float p1 = 1;
        FOR(j, power)
        {
            p0 *= d0;
            p1 *= d1;
        }
        d = p0 + p1;
    }
    return d;
}

void KMeansCluster::Update(bool bFirstIteration)
{
    bool bSuperposed = false;
    FOR(i, clusters)
    {
        FOR(j,i)
        {
            if(means[i] == means[j]) // we have 2 superposed clusters
            {
                // we replace it with a new one
                FOR(d,means[i].size()) means[i][d] = rand()/(float)RAND_MAX;
                //bSuperposed = true;
                break;
            }
        }
    }
    if(bSuperposed) InitClusters();

    if(bGMM) GMMClustering(points, means, sigma, pi, clusters, bFirstIteration);
    else if (bSoft) SoftKmeansClustering(points, means, clusters, beta, bFirstIteration);
    else if(!bFirstIteration) KmeansClustering(points, means, clusters);
    FOR(i, clusters)
    {
        float mindist = 1;
        u32 closest = 0;
        FOR(p, points.size())
        {
            float d = (points[p].point - means[i])*(points[p].point - means[i]);
            if (d < mindist)
            {
                mindist = d;
                closest = p;
            }
        }
        closestIndices[i] = closest;
    }
}

void KMeansCluster::AddPoint(fvec point)
{
    if(point.size() != dim) dim = point.size();
    ClusterPoint cpoint;
    cpoint.point = point;
    points.push_back(cpoint);
}

void KMeansCluster::AddPoints(std::vector<fvec> points)
{
    FOR(i, points.size()) AddPoint(points[i]);
}

void KMeansCluster::Clear()
{
    points.clear();
}

void KMeansCluster::SetClusters(u32 clusters)
{
    this->clusters = max((u32)0,clusters);
    InitClusters();
}

void KMeansCluster::InitClusters()
{
    srand(QTime::currentTime().msec());

    KILL(pi);
    if(sigma) FOR(i, clusters) KILL(sigma[i]);
    KILL(sigma);
    if(!clusters) return;
    means.resize(clusters);
    pi = new double[clusters];
    sigma = new double *[clusters];
    closestIndices.resize(clusters);
    FOR(i,clusters){
        means[i].resize(dim);
        pi[i] = 1.f/clusters;
        sigma[i] = new double[4];
        sigma[i][0] = sigma[i][3] = 0.1;
        sigma[i][1] = sigma[i][2] = 0.05;
    }
    if(!points.size()){
        // no points, just choose random centers
        FOR(i,clusters)
        {
            FOR(d,dim)
            {
                means[i][d] = float(rand())/RAND_MAX;
            }
            closestIndices[i] = 0;
        }
    }
    else if (plusPlus){
        InitClustersPlusPlus();
    }
    else // choose a point at random
    {
        FOR(i,clusters)
        {
            int index = rand()%points.size();
            means[i] = points[index].point;
            closestIndices[i] = index;
        }
    }
}

/** Use K-means++ to set the initial cluster centers, see
* <a href="http://en.wikipedia.org/wiki/K-means%2B%2B">K-means++ (wikipedia)</a>
* This code is based on Apache Commons Maths' KMeansPlusPlusClusterer.java
*/
void KMeansCluster::InitClustersPlusPlus()
{
    // Set the corresponding element in this array to indicate when points are no longer available.
    bvec pointTaken(points.size(),false);

    // Choose first cluster center uniformly at random from among the data points.
    int firstPointIndex = rand() % points.size(); // not uniform, but fair?
    means[0] = points[firstPointIndex].point;
    closestIndices[0] = firstPointIndex;
    pointTaken[firstPointIndex] = true; // must mark it as taken
    qDebug("first point at rand = %d[%f;%f]", firstPointIndex, points[firstPointIndex].point[0], points[firstPointIndex].point[1]);

    // Stores the squared minimum distance of each point to its nearest cluster center
    fvec minDistSquared(points.size(), 0.0f);

    // Initialize the distances. Easy, since the only cluster is the first point
    FOR(i, points.size())
    {
        if (i != firstPointIndex) // first point isn't considered
        {
            float d = Distance(points[firstPointIndex].point, points[i].point);
            qDebug("initial minDistSquared to %i[%f;%f] = %f", i, points[i].point[0], points[i].point[1], d);
            minDistSquared[i] = d * d;
        }
    }

    for(u32 centerCount = 1; centerCount < clusters; ++centerCount) // start at 1!
    {
        qDebug("picking cluster center %i----------------------------", centerCount);

        // Sum up the squared distances for the points not already taken.
        float distSqSum = 0.0f;
        FOR(j,points.size())
        {
            if (!pointTaken[j]) {
                distSqSum += minDistSquared[j];
            }
        }
        qDebug(" distSqSum=%f",distSqSum);

        // Choose one new point at random as a new center, using a weighted
        // probability distribution where a point x is chosen with probability proportional to D(x)^2
        float r = (rand() / float(RAND_MAX)) * distSqSum;
        qDebug(" random index %f",r);
        // The index of the next point to be added to the resultSet.
        bool nextPointFound= false;
        u32 nextPointIndex = 0;

        // Sum through the squared min distances again, stopping when sum >= r.
        float sum = 0.0f;
        for(size_t j=0; j < points.size() && !nextPointFound; ++j)
        {
            if (!pointTaken[j])
            {
                sum += minDistSquared[j];
                if (sum >= r)
                {
                    nextPointIndex = (u32) j;
                    nextPointFound = true;
                }
            }
        }

        // If no point was found in the previous for loop, probably because distances are extremely small.
        // Just pick the last available point.
        if (!nextPointFound)
        {
            qDebug("loop empty, pick one at rand");
            for(size_t j=0; j < points.size() && !nextPointFound; ++j)
            {
                if (!pointTaken[j])
                {
                    nextPointIndex = j;
                    nextPointFound = true;
                }
            }
        }

        // assert(nextPointFound); // if wanted

        // Set the new cluster point
        means[centerCount] = points[nextPointIndex].point;
        closestIndices[centerCount] = nextPointIndex;
        pointTaken[nextPointIndex] = true;
        qDebug(" new point %d[%f;%f]", nextPointIndex, points[nextPointIndex].point[0], points[nextPointIndex].point[1]);

        // Update minDistSquared. We only have to compute the distance to the new center, and update it if it is shorter
        for(size_t j=0; j < points.size(); ++j)
        {
            if (!pointTaken[j])
            {
                float d = Distance(points[nextPointIndex].point, points[j].point);
                float dSqr = d * d;
                if (dSqr < minDistSquared[j]) {
                    minDistSquared[j] = dSqr;
                }
            }
        }
    }
}



ivec KMeansCluster::GetClosestPoints()
{
    return closestIndices;
}

inline float fastExp(const float x)
{
    if(-x>90) return 0;
    else return expf(x);
}

void KMeansCluster::Test( fvec sample, fvec &res )
{
    if(res.size() != clusters) res.resize(clusters);
    if(bSoft)
    {
        fvec distances;
        distances.resize(clusters);
        // compute the distance to each clusters
        float distanceSum = 0;
        if(dim==2)
        {
            for (int j=0;j<clusters;j++){
                float d0 = means[j][0] - sample[0];
                float d1 = means[j][1] - sample[1];

                distances[j] = fastExp(-beta * sqrtf(d0*d0 + d1*d1));
                distanceSum += distances[j];
            }
        }
        else
        {
            for (int j=0;j<clusters;j++){
                distances[j] = fastExp(-beta * sqrtf((means[j] - sample)*(means[j] - sample)));
                distanceSum += distances[j];
            }
        }

        // compute the weights for each cluster
        for (int j=0;j<clusters;j++){
            res[j] = distances[j] / float(distanceSum);
        }
    }
    else
    {
        FOR(d, res.size()) res[d] = 0;
        int minIndex = 0;
        float minDist = FLT_MAX;
        if(dim==2)
        {
            FOR(j, clusters)
            {
                float distance = Distance2(sample, means[j]);
                if(distance < minDist)
                {
                    minIndex = j;
                    minDist = distance;
                }
            }
        }
        else
        {
            FOR(j, clusters)
            {
                float distance = Distance(sample, means[j]);
                if(distance < minDist)
                {
                    minIndex = j;
                    minDist = distance;
                }
            }
        }
        res[minIndex] = 1;
    }
}


/**
* performs the K-mean clustering algorithm
*
* @param points[]  : each element of this array contains two elements
*                    point, a fvec with the coordinates of the points
*                    cluster, the number of the cluster the point belongs to
* @param nbPoints  : number of point in the array
* @param oldMeans  : array of fvec containing the old positions of the cluster centers
* @param nbCluster : number of clusters
* @param limits    : boundaries of the values for the points coordinates (default is the image size [320x240])
*
*/

void KMeansCluster::KmeansClustering(std::vector<ClusterPoint> &points, vector<fvec> &oldMeans, int nbClusters)
{
    // check that we didnt try to use zero clusters
    nbClusters = !nbClusters ? 1 : nbClusters;

    if((u32)nbClusters > points.size()) nbClusters = points.size();

    // new means (centers) for the clusters
    vector<fvec>means;
    means.resize(nbClusters);

    // used to check all the distances from the current point to each cluster
    fvec distances;
    distances.resize(nbClusters);

    means = oldMeans;

    int nbPoints = points.size();

    // did at least one point move from one cluster to another ?
    bool bSomethingChanged = true;

    // the kmeans loop
    //while(bSomethingChanged){
        bSomethingChanged = false;

        //classify the points into clusters
        for (register int i=0; i<nbPoints; i++){
            // compute the distance to each clusters
            for (register int j=0;j<nbClusters;j++){
                distances[j] = Distance(points[i].point, means[j]);
            }

            // find the closest cluster
            if (points[i].cluster != FindSmallest(distances)){
                bSomethingChanged = true;
                points[i].cluster = FindSmallest(distances);
            }
        }

        //compute the new means for each cluster
        Mean(points, means, nbClusters);
    //}

    oldMeans = means;
}


/**
* returns the square distance between two points
*
* @param p1 : the first point
* @param p2 : the second one
* @return   : the distance
*
*       note: if this function is more than a couple of lines long,
*             you're doing something wrong!
*/
float SquareNorm(fvec p1, fvec p2)
{
    return p1*p2;
}


/**
* returns the index of the smallest value in the array
*
* @param values   : the array containing the distances
* @param nbValues : the length of the array
* @return   : the index of the smallest distance in the array
*
*/
int FindSmallest(fvec values)
{

    // initialize the minimum value and index to the first values in the array
    int minIndex = 0;
    float minValue = values[0];

    // go through the array and get the smallest value
    for (int i=0; i<values.size(); i++){
        if (values[i]<minValue){
            minIndex = i;
            minValue = values[i];
        }
    }

    return minIndex;
}

/**
* computes the means for each cluster
*
* @param points     : the array containing the cluster points
* @param nbPoints   : the length of the points array
* @param means      : the array containing the means for the clusters, will be modified by the function
* @param nbClusters : the number of clusters (also = length of the means array)
*
*/
void KMeansCluster::Mean(std::vector<ClusterPoint> &points, vector<fvec> &means, int nbClusters)
{
    // counters to know how many points went into each cluster
    int *nbPointInCluster = new int[nbClusters];

    // reinitialize the center for each cluster
    for(int k = 0; k < nbClusters; k++){
        FOR(d,dim) means[k][d] = 0;
        nbPointInCluster[k] = 0;
    }

    // sum the points
    for (register int i = 0; (u32)i<points.size(); i++){
        means[points[i].cluster] += points[i].point;
        nbPointInCluster[points[i].cluster] += 1;
    }

    // normalize by the number of points added to each cluster
    for(int k = 0; k < nbClusters; k++){
        if (nbPointInCluster[k]){
            means[k] /= (float)nbPointInCluster[k];
        }
    }

    // delete the unused memory
    delete [] nbPointInCluster;
}


// computes the means for each cluster
void KMeansCluster::SoftMean(std::vector<ClusterPoint> &points, vector<fvec> &means, int nbClusters)
{
    // counters to know how many points went into each cluster
    float *weightsOfPointsInCluster = new float[nbClusters];

    // reinitialize the center for each cluster
    for(int k = 0; k < nbClusters; k++){
        FOR(d,dim) means[k][d] = 0;
        weightsOfPointsInCluster[k] = 0;
    }

    // sum the points, for each cluster use the point's weight
    for (unsigned int i = 0; i<points.size(); i++){
        for(int k = 0; k<nbClusters; k++){
            means[k] += points[i].point * points[i].weights[k];
            weightsOfPointsInCluster[k] += points[i].weights[k];
        }
    }

    // normalize by the weights of the points added to each cluster
    for(int k = 0; k < nbClusters; k++){
        if (weightsOfPointsInCluster[k] != 0){
            means[k] /= weightsOfPointsInCluster[k];
        }
    }

    // delete the unused memory
    delete [] weightsOfPointsInCluster;
}


/**
* performs the Soft K-mean clustering algorithm
*
* @param points[]  : each element of this array contains two elements
*                    point, a fvec with the coordinates of the points
*                    weights, the weights of influence of the point on each cluster
* @param nbPoints  : number of point in the array
* @param oldMeans  : array of fvec containing the old positions of the cluster centers
* @param nbCluster : number of clusters
* @param limits    : boundaries of the values for the points coordinates (default is the image size [320x240])
* @param beta      : soft boundary stiffness (sigma = 1 / sqrt(beta))
*
*/

void KMeansCluster::SoftKmeansClustering(std::vector<ClusterPoint> &points, vector<fvec> &oldMeans, int nbClusters, float beta, bool bEStep)
{
    // check that we didnt try to use zero clusters
    nbClusters = !nbClusters ? 1 : nbClusters;

    if((unsigned int)nbClusters > points.size()) nbClusters = points.size();

    // contains the means for each cluster
    vector<fvec>means;
    means.resize(nbClusters);

    // used to check all the distances from the current point to each cluster
    fvec distances;
    distances.resize(nbClusters);

    // Random number generation for initial means of clusters
    // initialize the random seed with the current cpu time
    srand(QTime::currentTime().msec());

    means = oldMeans;

    int nbPoints = points.size();

    // initialize the points weights
    for (int i=0; i < nbPoints; i++)
    {
        points[i].weights = fvec(nbClusters,0);
        //FOR(j, nbClusters) points[i].weights[j] = 0;
    }

    //classify the points into clusters
    for (int i=0; i<nbPoints; i++){
        fvec point = points[i].point;
        // compute the distance to each clusters
        float distanceSum = 0;
        for (int j=0;j<nbClusters;j++){
            distances[j] = expf(-beta * sqrtf((means[j] - point)*(means[j] - point)));
            distanceSum += distances[j];
        }

        // compute the weights for each cluster
        for (int j=0;j<nbClusters;j++){
            points[i].weights[j] = distances[j] / float(distanceSum);
        }
    }

    if(!bEStep)
    {
        //compute the new means for each cluster
        SoftMean(points, means, nbClusters);
    }

    oldMeans = means;
}

void KMeansCluster::GMMClustering(std::vector<ClusterPoint> &points, vector<fvec> &oldMeans, double **oldSigma, double*oldPi, int nbClusters, bool bEStep)
{
    // check that we didnt try to use zero clusters
    nbClusters = !nbClusters ? 1 : nbClusters;

    if((unsigned int)nbClusters > points.size()) nbClusters = points.size();

    // contains the means for each cluster
    vector<fvec>means;
    means.resize(nbClusters);
    double *pi = new double[nbClusters];
    double **sigma = new double *[nbClusters];
    FOR(i, nbClusters)
    {
        sigma[i] = new double[4];
        sigma[i][0] = sigma[i][3] = 0.1f;
        sigma[i][1] = sigma[i][2] = 0;
    }

    // used to check all the distances from the current point to each cluster
    double *distances = new double[nbClusters];

    // Random number generation for initial means of clusters
    // initialize the random seed with the current cpu time
    srand(QTime::currentTime().msec());

    // initialize the means as the old values
    // divide by [320x240] to avoid numerical precision problems
    // WARNING: from now on the values will go from 0 to 1
    for (int i=0; i<nbClusters; i++){
        means[i] = oldMeans[i];
    }
    for (int i=0; i<nbClusters; i++){
        pi[i] = oldPi[i];
    }
    for (int i=0; i<nbClusters; i++){
        for (int j=0; j<4; j++) sigma[i][j] = oldSigma[i][j];
    }

    int nbPoints = points.size();


    if(bEStep)
    {
        // initialize the points weights
        u32 cnt = 0;
        for (int i=0; i < nbPoints; i++)
        {
            points[i].weights.resize(nbClusters);
            FOR(j, nbClusters) points[i].weights[j] = 0;
            points[i].weights[cnt++%nbClusters] = 1.f;
        }
    }

    if(!bEStep)
    {
        if(points[0].weights.size() == 0)
        {
            // initialize the points weights
            for (int i=0; i < nbPoints; i++)
            {
                points[i].weights.resize(nbClusters);
                FOR(j, nbClusters) points[i].weights[j] = 0;
            }
        }

        //classify the points into clusters
        u32 flipper=0;
        for (int i=0; i<nbPoints; i++){
            fvec point = points[i].point;
            // compute the distance to each clusters
            double distanceSum = 0;
            for (int j=0;j<nbClusters;j++){
                fvec a = (point - means[j]);
                double *s = sigma[j];
                double sdet = s[0]*s[3] - s[1]*s[2];
                double sinv[4] = {s[3], -s[1], -s[2], s[0]};
                FOR(k,4) sinv[k] /= sdet;

                double b = a[0]*a[0]*sinv[0] + a[0]*a[1]*(sinv[1]+sinv[2]) + a[1]*a[1]*sinv[3];
                b *= -0.5;
                double dist = exp(b);
                dist /= sqrt(sdet);
                distances[j] = pi[j]*dist;
                distances[j] /= 2*(double)PIf;
                distanceSum += distances[j];
            }

            // compute the weights for each cluster
            if(distanceSum != distanceSum)
            {
                FOR(j, nbClusters) points[i].weights[j] = 0;
                points[i].weights[flipper++%nbClusters] = 1;
            }
            else
                for (int j=0;j<nbClusters;j++){
                    points[i].weights[j] = (float)(distances[j] / distanceSum);
                }
        }
    }

    //compute the new means for each cluster
    for (int i=0; i<nbClusters; i++)
    {
        fvec mean;
        mean.resize(2,0);
        float respTotal = 0;
        for (int j=0; j<nbPoints; j++)
        {
            mean += points[j].point * points[j].weights[i];
            respTotal += points[j].weights[i];
        }
        means[i] = mean / respTotal;
    }

    //compute the new prior for each cluster
    float respTotal = 0;
    for (int i=0; i<nbClusters; i++)
    {
        float resp = 0;
        for (int j=0; j<nbPoints; j++)
        {
            resp += points[j].weights[i];
        }
        respTotal += resp;
        pi[i] = resp;
    }
    for (int i=0; i<nbClusters; i++)
    {
        pi[i] /= respTotal;
    }

    //compute the new sigma for each cluster
    for (int i=0; i<nbClusters; i++)
    {
        float sums[3];
        float resp = 0;
        for (int j=0; j<3; j++) sums[j] = 0;
        for (int j=0; j<nbPoints; j++)
        {
            float r = points[j].weights[i];
            if(r==0) continue;
            fvec diff = points[j].point - means[i];
            sums[0] += r*(diff[0]*diff[0]);
            sums[1] += r*(diff[0]*diff[1]);
            sums[2] += r*(diff[1]*diff[1]);
            resp += r;
        }
        for (int j=0; j<3; j++) sums[j] /= resp;
        sigma[i][0] = sums[0];
        sigma[i][1] = sigma[i][2] = sums[1];
        sigma[i][3] = sums[2];
    }

    // we copy the new values into the old ones
    for (int i=0; i<nbClusters; i++){
        oldMeans[i] = means[i];
    }
    for (int i=0; i<nbClusters; i++){
        oldPi[i] = pi[i];
    }
    for (int i=0; i<nbClusters; i++){
        for (int j=0; j<4; j++) oldSigma[i][j] = sigma[i][j];
    }

    // we delete the memory we used
    delete [] distances;
    delete [] pi;
    for (int i=0; i<nbClusters; i++) delete [] sigma[i];
    delete [] sigma;
}
