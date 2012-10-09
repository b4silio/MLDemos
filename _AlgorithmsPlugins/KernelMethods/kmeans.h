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
#ifndef _KMEANS_H_
#define _KMEANS_H_

#include <vector>

struct ClusterPoint{
	fvec point;
	u32 cluster;
	float *weights;
    ClusterPoint() : point(0,0), cluster(0), weights(NULL){}
	~ClusterPoint()
	{
		if(weights) delete [] weights;
		weights=0;
	};
};

inline float SquareNorm(fvec p1, fvec p2);
inline int FindSmallest(fvec values);
inline void Mean(std::vector<ClusterPoint>, std::vector<fvec> means, int nbClusters);
inline void SoftMean(std::vector<ClusterPoint> &points, std::vector<fvec> means, int nbClusters);


class KMeansCluster
{
private:
	float beta;
	u32 clusters;
	bool bSoft;
	std::vector<fvec> means;
	std::vector<ClusterPoint> points;
	ivec closestIndices;
	int dim;
	int power;
        bool plusPlus;

	bool bGMM;
	double **sigma;
	double *pi;

public:
	KMeansCluster(u32 cnt=1);
	~KMeansCluster();

    void Update(bool bFirstIteration=false);

	void Clear();
	void Test(fvec sample, fvec &res);

    void SetPoint(u32 index, fvec point){if(index<points.size()) points[index].point = point;}

	void AddPoint(fvec sample);
	void AddPoints(std::vector<fvec> points);

    fvec GetMean(u32 index=0) {return index<clusters ? means[index] : fvec();}
    std::vector<fvec> GetMeans(){return means;}
	ivec GetClosestPoints();

	void SetClusters(u32 clusters);
    u32 GetClusters(){return clusters;}
        void InitClusters();
        void InitClustersPlusPlus();

	inline float Distance(fvec a, fvec b);
	inline float Distance2(fvec a, fvec b);

    void SetSoft(bool soft){bSoft = soft;}
    void SetBeta(float b){beta = b > 0 ? b : 0.01f;}
    void SetGMM(bool gmm){bGMM = gmm;}
    void SetPower(int p){power = p;}
    void SetPlusPlus(bool p){plusPlus = p;}
    float GetBeta(){return beta;}

private:
	void Mean(std::vector<ClusterPoint> &points, std::vector<fvec> &means, int nbClusters);
	void SoftMean(std::vector<ClusterPoint> &points, std::vector<fvec> &means, int nbClusters);
    void KmeansClustering(std::vector<ClusterPoint> &points, std::vector<fvec> &oldMeans, int nbClusters);
	void SoftKmeansClustering(std::vector<ClusterPoint> &points, std::vector<fvec> &oldMeans, int nbClusters, float beta, bool bEStep);
	void GMMClustering(std::vector<ClusterPoint> &points, std::vector<fvec> &oldMeans, double **oldSigma, double*oldPi, int nbClusters, bool bEStep);

};

#endif // _KMEANS_H_
