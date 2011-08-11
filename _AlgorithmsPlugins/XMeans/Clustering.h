#ifndef CLUSTERING_H
#define CLUSTERING_H

#include "vectorSpace.hpp"
#include <set> 

namespace AG
{
	namespace Clustering
	{
		using namespace AG::Data;

		typedef unsigned int index;
		typedef std::vector<index> Clusters;

		Clusters qt_clustering(VectorSpace & vs, double max_diameter, int minCount);
	};
};
#endif

