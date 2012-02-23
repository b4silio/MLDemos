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
#include "basicOpenCV.h"
#include <mymaths.h>
#include "kmeans.h"

using namespace std;


KMeansCluster::KMeansCluster(u32 cnt)
    : clusters(cnt), sigma(NULL), pi(NULL), bGMM(false), bSoft(false), beta(1), dim(2), power(2)
{
    ResetClusters();
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

void KMeansCluster::Update(bool bEStep)
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
    if(bSuperposed) ResetClusters();

    if(bGMM)
    {
        GMMClustering(points, means, sigma, pi, clusters, bEStep);
    }
    else
    {
        if(!bSoft) KmeansClustering(points, means, clusters);
        else SoftKmeansClustering(points, means, clusters, beta, bEStep);
    }
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

void KMeansCluster::Draw(IplImage *image)
{
    if(!points.size()) return;
    if (!bSoft && !bGMM) {
        // used for overlay colors
        float over[6][3] = {{0,0,1},{1,0,0},{0,1,0},{0.5f,0,0.5f},{0,0.5f,0.5f},{0.5f,0.5f,0}};
        // and now lets draw them points!
        FOR(i, points.size()){
            u32 cluster = points[i].cluster;

            register u32 index = ((u32)(points[i].point[0]*image->width) + (u32)(points[i].point[1]*image->height)*image->width)*3;
            FOR(c,3){
                //u8 color = RGB(image, index + c);
                u8 color = 255;
                //				RGB(image, index + c) = (u8)(color);
                //RGB(image, index + c) = (u8)(color*over[cluster%6][c]);
            }
        }
    }
    else {
        // used for overlay colors
        float over[6][3] = {{0,0,1},{1,0,0},{0,1,0},{0.5f,0,0.5f},{0,0.5f,0.5f},{0.5f,0.5f,0}};
        // and now lets draw them points!
        FOR(i, points.size()){
            register u32 index = ((u32)(points[i].point[0]*image->width) + (u32)(points[i].point[1]*image->height)*image->width)*3;
            FOR(c,3){
                //u8 color = RGB(image, index + c);
                u8 color = 255;
                float colorSum = 0;
                FOR(cl, clusters){
                    colorSum += points[i].weights[cl]*over[cl%6][c];
                }
                //				RGB(image, index + c) = u8(color*colorSum);
            }
        }
    }

    FOR(i, clusters){
        if(i<points.size()) {draw_cross(image, cvPoint((u32)(means[i][0]*image->width), (u32)(means[i][1]*image->height)),CV_RGB(255,255,255),4);}
        //		cvCircle(image, cvPoint((u32)(points[closestIndices[i]].point.x*image->width), (u32)(points[closestIndices[i]].point.y*image->height)), 4, CV_RGB(0,255,0), 1, CV_AA);
    }
}

void KMeansCluster::DrawMap(IplImage *image)
{
    if(!points.size()) return;
    if (!bSoft && !bGMM) {
        // and now lets draw them points!
        fvec dists;
        dists.resize(clusters);
        FOR(i, image->width*image->height)
        {
            fvec point;
            point.push_back((i%image->width)/(float)image->width);
            point.push_back((i/image->width)/(float)image->height);
            FOR(j, clusters) dists[j] = SquareNorm(point, means[j]);
            u32 cluster = FindSmallest(dists);

            register u32 index = ((u32)(point[0]*image->width) + (u32)(point[1]*image->height)*image->width)*3;

            CvScalar color = CV::color[(cluster+1)% CV::colorCnt];
            cvSet1D(image, i, color);
        }
    }
    else {
        // and now lets draw them points!
        f32 *dists = new f32[clusters];
        f32 *weights = new f32[clusters];
        FOR(i, image->width*image->height)
        {
            fvec point;
            point.push_back((i%image->width)/(float)image->width);
            point.push_back((i/image->width)/(float)image->height);
            f32 sum = 0;
            FOR(j, clusters)
            {
                dists[j] = expf(-beta * sqrt((means[j] - point)*(means[j] - point)));
                sum += dists[j];
            }
            FOR(j, clusters) weights[j] = dists[j] / sum;

            register u32 index = ((u32)(point[0]*image->width) + (u32)(point[1]*image->height)*image->width)*3;
            CvScalar color = cvScalarAll(0);
            FOR(j, clusters)
            {
                CvScalar col = CV::color[(j+1)% CV::colorCnt];
                FOR(c,3) color.val[c] += col.val[c]*weights[j];
            }
            cvSet1D(image, i, color);
        }
    }
    /*
 FOR(i, clusters){
  if(i<points.size()) {draw_cross(image, cvPoint((u32)(means[i].x*image->width), (u32)(means[i].y*image->height)),CV_RGB(255,255,255),4);}
  cvCircle(image, cvPoint((u32)(points[closestIndices[i]].point.x*image->width), (u32)(points[closestIndices[i]].point.y*image->height)), 4, CV_RGB(0,255,0), 1, CV_AA);
 }
 */
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
    ResetClusters();
}

void KMeansCluster::ResetClusters()
{
    srand((u32)cvGetTickCount());

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
        if(!points.size())
        {
            FOR(d,dim)
            {
                means[i][d] = float(rand())/RAND_MAX;
            }
            closestIndices[i] = 0;
        }
        else
        {
            int index = rand()%points.size();
            means[i] = points[index].point;
            closestIndices[i] = index;
        }
        pi[i] = 1.f/clusters;
        sigma[i] = new double[4];
        sigma[i][0] = sigma[i][3] = 0.1;
        sigma[i][1] = sigma[i][2] = 0.05;
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
    while(bSomethingChanged){
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
    }

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
    srand((u32)cvGetTickCount());

    means = oldMeans;

    int nbPoints = points.size();

    // initialize the points weights
    for (int i=0; i < nbPoints; i++)
    {
        KILL(points[i].weights);
        points[i].weights = new float[nbClusters];
        FOR(j, nbClusters) points[i].weights[j] = 0;
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
    srand((u32)cvGetTickCount());

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
            KILL(points[i].weights);
            points[i].weights = new float[nbClusters];
            FOR(j, nbClusters) points[i].weights[j] = 0;
            points[i].weights[cnt++%nbClusters] = 1.f;
        }
    }

    if(!bEStep)
    {
        if(points[0].weights == NULL)
        {
            // initialize the points weights
            for (int i=0; i < nbPoints; i++)
            {
                KILL(points[i].weights);
                points[i].weights = new float[nbClusters];
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
