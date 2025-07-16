#include "Clustering.h"
#include <limits>
#include <boost/foreach.hpp>
#include <map>
using namespace std;
namespace AG
{
namespace Clustering{

Vect operator+ (Vect &a, Vect &b)
{
	Vect c = a;
	for (int i=0; i<c.size(); i++) c[i] += b[i];
	return c;
}

void operator+= (Vect &a, const Vect b)
{
	for(int i=0; i<a.size(); i++) a[i] += b[i];
}

void operator *= (Vect &a, const float b)
{
	for(int i=0; i<a.size(); i++) a[i] *= b;
}

void operator /= (Vect &a, const float b)
{
	for(int i=0; i<a.size(); i++) a[i] /= b;
}

bool diameterInf(Vect x, double maxDist, double *limits)
{
	int dim = x.size();
	double *newLimits = new double[dim*2];
	memcpy(newLimits, limits, sizeof(double)*dim*2);

	bool bInside = true;
	for (int d=0; d < dim; d++)
	{
		double dLow = limits[2*d];
		double dHigh = limits[2*d+1];
		if(dLow > x[d] || dHigh < x[d])
		{
			bInside = false;
			break;
		}
	}
	if(bInside) return true;

	bool bOk = true;
	for (int d=0; d < dim; d++)
	{
		double xd = x[d];
		double dLow = min(xd, limits[d*2]);
		double dHigh = max(xd, limits[d*2+1]);
		if(dHigh - dLow > maxDist)
		{
			bOk = false;
			break;
		}
		newLimits[2*d] = dLow;
		newLimits[2*d+1] = dHigh;
	}

	if(bOk) memcpy(limits, newLimits, sizeof(double)*dim*2);
	delete [] newLimits;
	return bOk;
}

bool diameter2(Vect x, double maxDist, std::vector<Vect> clust, Vect sum, int count)
{
	int dim = x.size();
	maxDist = maxDist*maxDist;
	// we keep the center of the cluster
	Vect center = sum + x;
	center /= count + 1;

	double dist = 0;
	for (int d=0; d < dim; d++) dist += (x[d]-center[d])*(x[d]-center[d]);
	if(dist > maxDist) return false;

	for (int i=0; i<clust.size(); i++)
	{
		double dist = 0;
		for (int d=0; d<dim; d++) dist += (clust[i][d]-center[d])*(clust[i][d]-center[d]);
		if(dist > maxDist) return false;
	}
	return true;
}

Clusters qt_clustering(VectorSpace & vs, double max_diameter, int minCount)
{
	Clusters clusters(vs.size(), std::numeric_limits<index>::max());  // assign all the vectors to no cluster
	int assignedCount = 0;
	int clusterId = 0;
	int count = vs.size();
	int dim = vs[0].size();
	while(assignedCount < vs.size())
	{
		std::vector < std::vector<Vect> > clusts;
		std::vector < std::vector<int> > clustsIdx;
		clusts.resize(vs.size());
		clustsIdx.resize(vs.size());
		double *limits = new double[dim*2];
		int skipped = 0;
		int docId_i = 0;
		BOOST_FOREACH(Vect d_i, vs)         // foreach document
		{
			if (clusters[docId_i] != std::numeric_limits<index>::max())
			{
				skipped++;
				docId_i++;
				continue; // sample taken already
			}
			for(int d=0; d<dim; d++)
			{
				limits[2*d] = d_i[d];
				limits[2*d+1] = d_i[d];
			}
			std::vector<Vect> &clust = clusts[docId_i];
			std::vector<int> &clustIdx = clustsIdx[docId_i];
			clust.push_back(d_i);
			clustIdx.push_back(docId_i);

			for(index docId_j = 0; docId_j < vs.size(); docId_j++)
			{
				if(docId_j == docId_i) continue;
				if (clusters[docId_j] != std::numeric_limits<index>::max()) continue; // sample taken already
				if(diameterInf(vs[docId_j], max_diameter, limits))
				{
					clust.push_back(vs[docId_j]);
					clustIdx.push_back(docId_j);
				}
			}
			docId_i++;
		} // foreach document
		delete [] limits;
		// find the cluster with the maximum count
		int maxIndex = 0, maxCnt = 0;
		for(int i=0; i<clustsIdx.size(); i++)
		{
			if(maxCnt < clustsIdx[i].size())
			{
				maxIndex = i;
				maxCnt = clustsIdx[i].size();
			}
		}
		std::cout << "maximum cluster: " << maxIndex << " with " << maxCnt << " samples"<< std::endl;
		if(maxCnt < minCount) break;
		for(int i=0; i<clustsIdx[maxIndex].size(); i++)
		{
			int index = clustsIdx[maxIndex][i];
			clusters[index] = clusterId;
		}
		clusterId++;
		assignedCount += maxCnt;
	}
	return clusters;
};
}; /* Clustering */
};
