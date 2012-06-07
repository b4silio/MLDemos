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
#ifndef _CLUSTERER_DBSCAN_H_
#define _CLUSTERER_DBSCAN_H_

#include <vector>
#include <cmath>
#include <clusterer.h>
#include "distance.h"
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/foreach.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <QMultiMap>


// a single point is made up of vector of float
typedef boost::numeric::ublas::vector<float>  Point;
typedef std::vector<Point> Points;

typedef unsigned int ClusterId;
typedef unsigned int PointId;

// a cluster is a vector of pointid
typedef std::vector<PointId> CCluster;
// a set of Neighbors is a vector of pointid
typedef std::vector<PointId> Neighbors;


/**
  Clusterer DBSCAN implementing all the necessary functions from the interface
  */
class ClustererDBSCAN : public Clusterer
{
public:

    /**
      Constructor, instanciating everything that will be used
      */
    ClustererDBSCAN(){
    }
    /**
      Deconstructor, deinstanciating everything that has been instanciated
      */
    ~ClustererDBSCAN(){}

    /**
      The training function, called by the main program, all training should go here
      */
    void Train(std::vector< fvec > samples);

    /**
      The testing function, returns a vector of size nbClusters, with the contribution/weight of the point for each cluster
      */
	fvec Test( const fvec &sample);

    /**
      Information string for the Algorithm Information and Statistics panel in the main program interface.
      Here you probably will put the number of parameters, the training time or anything else
      */
    const char *GetInfoString();

    /**
      Function to set the algorithm hyper-parameters, called prior to the training itself
      */
    void SetParams(float minpts, float eps, int metric, float depth,int type);

    /**
      Function to compute the similarity matrix used as cache for the distances between points
      */
    template <typename Distance_type>
    void computeSimilarity(Distance_type & d,Points samples);

    /**
      Function to get all the points within a distance given by the threshold
      */
    Neighbors findNeighbors(PointId pid, double threshold);

    /**
      Run DBSCAN
      */
    void run_cluster(Points samples) ;

    /**
      Function to update the reachability of a given point according to its core-distance
      */
    void update_reachability(Neighbors ne,PointId pid,double core_dist,QMultiMap<double,PointId> &queue);

    /**
      Function to compute the core-distance of a points
      */
    double core_distance(PointId pid,double threshold);

    /**
      Run OPTICS
      */
    void run_optics(Points samples);

    /**
      Run the default cluster identification method for OPTICS, must be run after run_optics.
      */
    void find_clusters();
    /**
      Run the water-pouring (or water-filling) cluster identification method for OPTICS, must be run after run_optics.
      */
    void find_clusters_WF();

    //keep the list of samples
    Points pts;

    // mapping point_id -> clusterId
    std::vector<ClusterId> _pointId_to_clusterId;

    // core points
    std::vector<bool> _core;

   // ordering of the points
   std::vector<double> _optics_list;

   //reachability distance of the points
   std::vector<double> _reachability;

   // type of clustering (0:DBSCAN,1:OPTICS,2:OPTICS WP)
   int _type;

   // which point is noise
   std::vector<bool> _noise;   

private:

    // the collection of clusters
    std::vector<CCluster> _clusters;

    // simarity_matrix
    boost::numeric::ublas::matrix<double> _sim;

    // eps radiuus
    // Two points are neighbors if the distance
    // between them does not exceed threshold value.
    float _eps;

    //minimum number of points
    int _minPts;

    // visited vector
    std::vector<bool> _visited;



    // depth of the pits to identify on the plot
    float _depth;

    // metric to be used for compèuting the distances (0:Cosine,1:Euclidean)
    int _metric;
};

#endif // _CLUSTERER_DBSCAN_H_
