/*=========================================================================
*
*  Copyright David Doria 2012 daviddoria@gmail.com
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*         http://www.apache.org/licenses/LICENSE-2.0.txt
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
*
*=========================================================================*/

/** XMeans clustering is a method that creates K clusters of points from
* an unorganized set of input points. It is an extension of KMeans clustering
* that attempts to determine K during the algorithm.
*/

#ifndef XMeansClustering_h
#define XMeansClustering_h

// STL
#include <vector>
#include <iostream>
#include <limits>
#include <numeric>
#include <set>
#include <stdexcept>

// Eigen
#include <Eigen/Dense>

namespace EigenHelpers{
	template <typename TVector>
	TVector RandomUnitVector(const unsigned int dim)
	{
		TVector randomUnitVector(dim);
		for(int i = 0; i < randomUnitVector.size(); ++i)
		{
			randomUnitVector[i] = (double(rand()) / RAND_MAX);
		}

		randomUnitVector.normalize();

		return randomUnitVector;
	}

	template <typename TPoint>
	void GetBoundingBox(const std::vector<TPoint, Eigen::aligned_allocator<TPoint> >& data, TPoint& minCorner, TPoint& maxCorner)
	{
		assert(data.size() > 0);

		minCorner.resize(data[0].size());
		maxCorner.resize(data[0].size());

		for(int coordinate = 0; coordinate < data[0].size(); ++coordinate)
		{
			minCorner[coordinate] = std::numeric_limits<typename TPoint::Scalar>::max();
			maxCorner[coordinate] = std::numeric_limits<typename TPoint::Scalar>::min();

			for(unsigned int pointId = 0; pointId < data.size(); ++pointId)
			{
				if(data[pointId][coordinate] > maxCorner[coordinate])
				{
					maxCorner[coordinate] = data[pointId][coordinate];
				}

				if(data[pointId][coordinate] < minCorner[coordinate])
				{
					minCorner[coordinate] = data[pointId][coordinate];
				}
			}
		}
	}
}

class XMeansClustering
{
public:
	typedef Eigen::VectorXf PointType;
	typedef std::vector<PointType, Eigen::aligned_allocator<PointType> > VectorOfPoints;

	/** Constructor. */
	XMeansClustering();

	/** Set the maximum number of clusters to find. */
	void SetMaxK(const unsigned int maxk);

	/** Get the maximum number of clusters to find. */
	unsigned int GetMaxK();

	/** Get the ids of the points that belong to class 'label'. */
	std::vector<unsigned int> GetIndicesWithLabel(const unsigned int label);

	/** Get the coordinates of the points that belong to class 'label'. */
	VectorOfPoints GetPointsWithLabel(const unsigned int label);

	/** Set the points to cluster. */
	void SetPoints(const VectorOfPoints& points);

	/** Get the resulting cluster id for each point. */
	std::vector<unsigned int> GetLabels();

	/** Actually perform the clustering. */
	void Cluster();

	/** Write the cluster centers to the standard output. */
	void OutputClusterCenters();

private:

	/** Split every cluster into two clusters if that helps the description of the data. */
	void SplitClusters();

	/** The label (cluster ID) of each point. */
	std::vector<unsigned int> Labels;

	/** The maximum number of clusters to find */
	unsigned int MaxK;

	/** The points to cluster. */
	VectorOfPoints Points;

	/** The current cluster centers. */
	VectorOfPoints ClusterCenters;
};


XMeansClustering::XMeansClustering() : MaxK(3)
{

}

void XMeansClustering::Cluster()
{
	if(this->Points.size() < this->MaxK)
	{
		std::stringstream ss;
		ss << "The number of points (" << this->Points.size()
			<< " must be larger than the maximum number of clusters (" << this->MaxK << ")";
		throw std::runtime_error(ss.str());
	}

	// We must store the labels at the previous iteration to determine whether any labels changed at each iteration.
	std::vector<unsigned int> oldLabels(this->Points.size(), 0); // initialize to all zeros

	// Initialize the labels array
	this->Labels.resize(this->Points.size());

	do
	{
		// Save the old labels
		oldLabels = this->Labels;
	}while(this->ClusterCenters.size() < this->MaxK);
}

void XMeansClustering::SplitClusters()
{
	assert(this->Points.size() > 0);

	VectorOfPoints newClusterCenters;

	for(unsigned int clusterId = 0; clusterId < this->ClusterCenters.size(); ++clusterId)
	{
		// Generate a random direction
		PointType randomUnitVector = EigenHelpers::RandomUnitVector<PointType>(this->Points[0].size());

		// Get the bounding box of the points that belong to this cluster
		PointType minCorner;
		PointType maxCorner;
		EigenHelpers::GetBoundingBox(this->Points, minCorner, maxCorner);

		// Scale the unit vector by the size of the region
		PointType splitVector = randomUnitVector * (maxCorner - minCorner) / 2.0f;
		PointType childCenter1 = this->ClusterCenters[clusterId] + splitVector;
		PointType childCenter2 = this->ClusterCenters[clusterId] + splitVector;

		// Compute the BIC of the original model
		float BIC_parent;

		// Compute the BIC of the new (split) model
		float BIC_children;

		// If the split was useful, keep it
		if(BIC_children < BIC_parent)
		{
			newClusterCenters.push_back(childCenter1);
			newClusterCenters.push_back(childCenter2);
		}
		else
		{
			newClusterCenters.push_back(this->ClusterCenters[clusterId]);
		}
	}

	this->ClusterCenters = newClusterCenters;
}

std::vector<unsigned int> XMeansClustering::GetIndicesWithLabel(unsigned int label)
{
	std::vector<unsigned int> pointsWithLabel;
	for(unsigned int i = 0; i < this->Labels.size(); i++)
	{
		if(this->Labels[i] == label)
		{
			pointsWithLabel.push_back(i);
		}
	}

	return pointsWithLabel;
}

XMeansClustering::VectorOfPoints XMeansClustering::GetPointsWithLabel(const unsigned int label)
{
	VectorOfPoints points;

	std::vector<unsigned int> indicesWithLabel = GetIndicesWithLabel(label);

	for(unsigned int i = 0; i < indicesWithLabel.size(); i++)
	{
		points.push_back(this->Points[indicesWithLabel[i]]);
	}

	return points;
}


void XMeansClustering::SetMaxK(const unsigned int maxk)
{
	this->MaxK = maxk;
}

unsigned int XMeansClustering::GetMaxK()
{
	return this->MaxK;
}

void XMeansClustering::SetPoints(const VectorOfPoints& points)
{
	this->Points = points;
}

std::vector<unsigned int> XMeansClustering::GetLabels()
{
	return this->Labels;
}

void XMeansClustering::OutputClusterCenters()
{
	std::cout << std::endl << "Cluster centers: " << std::endl;

	for(unsigned int i = 0; i < ClusterCenters.size(); ++i)
	{
		std::cout << ClusterCenters[i] << " ";
	}
	std::cout << std::endl;
}

#endif
