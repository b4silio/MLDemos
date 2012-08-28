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
#include "public.h"
#include "clustererDBSCAN.h"
#include <boost/foreach.hpp>

using namespace std;


void ClustererDBSCAN::Train(std::vector< fvec > samples)
{
    if(!samples.size()) return; // no sample :(

    // prepare the different array storing information about the clusters
    _noise.resize(samples.size(), false);
    _visited.resize(samples.size(), false);
    _core.resize(samples.size(), false);
    _pointId_to_clusterId.resize(samples.size(), 0);
    _reachability.resize(samples.size(), -1);

    // convert from fvec to Point
    for (int j = 0; j < samples.size(); ++j)
    {
        Point v (samples[j].size());
        for (int i = 0; i < samples[j].size(); ++i)
        {
            v(i)=samples[j][i];
        }
        pts.push_back(v);
    }

    // build the similarity matrix according to the selected metric
    if(_metric == 0)
    {
        Metrics::Distance<Metrics::Cosine<Point> > d;
        computeSimilarity(d,pts);
    }
    else
    {
        Metrics::Distance<Metrics::Euclidean<Point> > d;
        computeSimilarity(d,pts);
    }

    // run clustering

    if (_type>0) //OPTICS
    {
        run_optics(pts);

        if(_type>1) //OPTICS WP
        {
            find_clusters_WF(); // old name was for "water-filling"
        }
        else //OPTICS default
        {
            find_clusters();
        }
    }
    else // DBSCAN
    {
        run_cluster(pts);
    }
}

fvec ClustererDBSCAN::Test( const fvec &sample)
{
    fvec res;
    res.resize(nbClusters+1,0);

    //convert input to Point
    Point v (sample.size());
    for (int i = 0; i < sample.size(); ++i)
    {
        v(i)=sample[i];
    }

    // find the nearest point in our samples
    int nearest = -1;
    double dist = INFINITY;
    double temp_d = 0;

    if (_type==0) //if DBSCAN we set _depth like _eps
    {
        _depth=_eps;
    }
    for (int j = 0; j < pts.size(); ++j)
    {
        // according to the selected metric
        if(_metric == 0)
        {
            Metrics::Distance<Metrics::Cosine<Point> > d;
            temp_d = d.distance(v, pts[j]);
        }
        else
        {
            Metrics::Distance<Metrics::Euclidean<Point> > d;
            temp_d = d.distance(v, pts[j]);
        }
        if (temp_d < dist && temp_d < _eps && _pointId_to_clusterId[j] > 0 && _core[j]){
            dist = temp_d;
            nearest = j;
        }
    }

    // did we find something?
    if (nearest > -1){
        if (dist < _depth){ // is it near enough?
            res[_pointId_to_clusterId[nearest]-1] = 1; //take the color of that cluster
        }
        else if (abs(dist - _eps) < _eps*0.01) //in OPTICS, we are at the border of _eps : draw a thin line, darker
        {
            res[_pointId_to_clusterId[nearest]-1] = 0.5;
        }
    }

    return res;
}

const char *ClustererDBSCAN::GetInfoString()
{
    char *text = new char[1024];
    if (_type==0)
    {
        sprintf(text, "DBSCAN\n\nTraining informations: minPts : %d, eps: %f, metric: %d\n",_minPts,_eps,_metric);
    }
    else if (_type==1)
    {
        sprintf(text, "OPTICS\n\nTraining informations: minPts : %d, eps: %f, depth: %f, metric: %d\n",_minPts,_eps,_depth,_metric);
    }
    else
    {
        sprintf(text, "OPTICS WP\n\nTraining informations: minPts : %d, eps: %f, depth: %f, metric: %d\n",_minPts,_eps,_depth,_metric);
    }

    sprintf(text, "%sNumber of clusters: %d\n",text,_clusters.size());

    int countN = 0;
    int countC = 0;

    for (int i=0;i<_noise.size();i++)
    {
       if (_noise[i]) countN++;
       if (_core[i]) countC++;
    }
    sprintf(text, "%sNumber of core points: %d\nNumber of noise points: %d\n",text,countC,countN);

    return text;
}

void ClustererDBSCAN::SetParams(float minpts, float eps, int metric, float depth, int type)
{
    _eps = eps;
    _metric = metric;
    _minPts = minpts;
    _depth = depth;
    _type = type;
}

void ClustererDBSCAN::run_cluster(Points samples)
{

        ClusterId cid = 1;
        // foreach pid
        for (PointId pid = 0; pid < samples.size(); pid++)
        {
                // not already visited
                if (!_visited[pid]){

                        _visited[pid] = true;

                        // get the neighbors
                        Neighbors ne = findNeighbors(pid, _eps);

                        // not enough support -> mark as noise
                        if (ne.size() < _minPts)
                        {
                                _noise[pid] = true;
                        }
                        else
                        {
                            //else it's a core point
                                _core[pid] = true;

                                // Add p to current cluster

                                CCluster c;              // a new cluster
                                c.push_back(pid);   	// assign pid to cluster
                                _pointId_to_clusterId[pid]=cid;

                                // go to neighbors
                                for (unsigned int i = 0; i < ne.size(); i++)
                                {
                                        PointId nPid = ne[i];

                                        // not already visited
                                        if (!_visited[nPid])
                                        {
                                                _visited[nPid] = true;

                                                // go to neighbors
                                                Neighbors ne1 = findNeighbors(nPid, _eps);

                                                // enough support
                                                if (ne1.size() >= _minPts)
                                                {
                                                        _core[nPid] = true;

                                                        // join
                                                        BOOST_FOREACH(Neighbors::value_type n1, ne1)
                                                        {
                                                                // join neighbord
                                                                ne.push_back(n1);    
                                                        }
                                                }
                                        }

                                        // not already assigned to a cluster
                                        if (!_pointId_to_clusterId[nPid])
                                        {
                                            // add it to the current cluster
                                                c.push_back(nPid);
                                                _pointId_to_clusterId[nPid]=cid;
                                        }
                                }
                                // start a new cluster
                                _clusters.push_back(c);
                                cid++;
                        }
                } // if (!visited
        } // for

        nbClusters = cid;
}


void ClustererDBSCAN::run_optics(Points samples)
{
        // foreach pid
        for (PointId pid = 0; pid < samples.size(); pid++)
        {
                // not already visited
                if (!_visited[pid]){

                        _visited[pid] = true;

                        // get the neighbors
                        Neighbors ne = findNeighbors(pid, _eps);
                        // add it to the ordered list
                        _optics_list.push_back(pid);
                        // use the multiMap as priority queue
                        QMultiMap<double,PointId> queue;

                        double d = this->core_distance(pid,_eps);
                        // not enough support -> mark as noise
                        if (d < 0)
                        {
                                _noise[pid] = true;
                        }
                        else
                        {
                            //else it is a core point
                                _core[pid] = true;
                                this->update_reachability(ne,pid,d,queue);

                                // go to neighbors in the good order
                                while(queue.size()>0)
                                {
                                    //take element with lowest distance from the queue
                                        PointId nPid = queue.begin().value();
                                        queue.erase(queue.begin());

                                        // not already visited
                                        if (!_visited[nPid])
                                        {
                                                _visited[nPid] = true;

                                                // go to neighbors
                                                Neighbors ne1 = findNeighbors(nPid, _eps);

                                                _optics_list.push_back(nPid);

                                                double dd = this->core_distance(nPid,_eps);
                                                // enough support
                                                if (dd >= 0)
                                                {
                                                        _core[nPid] = true;
                                                        this->update_reachability(ne1,nPid,dd,queue);

                                                }
                                        }
                                }

                        }
                } // if (!visited
        } // for

}

void ClustererDBSCAN::update_reachability(Neighbors ne,PointId pid,double core_dist,QMultiMap<double,PointId> &queue)
{
    BOOST_FOREACH(Neighbors::value_type n, ne)
    {
        if(!_visited[n])
        {
            double ndist = max(core_dist,_sim(pid,n));
            if(_reachability[n]< 0)
            {
                _reachability[n] = ndist;
                queue.insert(ndist,n);
            }
            else{
                if(_reachability[n]>ndist)
                {
                    queue.remove(_reachability[n],n);
                    _reachability[n] = ndist;
                    queue.insert(ndist,n);
                }

            }
        }
    }
}

void ClustererDBSCAN::find_clusters()
{

    ClusterId cid = 1;
    CCluster c;   // a new cluster

    if (!_optics_list.empty()) _reachability[_optics_list[0]] = 0; //first element is anyway reachable

    // find pits below the threshold
    BOOST_FOREACH(PointId pid, _optics_list)
    {

        if (_reachability[pid]== -1) _reachability[pid] = _eps * 1.1;// undefined values are set to 10% more than Eps
        if (_reachability[pid] > _depth && !c.empty()) //we pass the threshold up
        {
            if(c.size() >= _minPts) // cluster is valid
            {
                _clusters.push_back(c);
                cid++;
                c = CCluster();
            }
            else     // cluster is too small -> noise
            {
                BOOST_FOREACH(PointId cpid, c)
                {
                    _noise[cpid] = true;
                    _pointId_to_clusterId[cpid] = 0;
                }
                c.clear(); //restart
            }
        }
        if (!_noise[pid])
        {
            c.push_back(pid);   	// assign pid to cluster
            _pointId_to_clusterId[pid]=cid;
        }
    }
    // do it once again after the loop to finalize the last cluster
    if(c.size() >= _minPts) // cluster is valid
    {
        _clusters.push_back(c);
    }
    else     // cluster is too small -> noise
    {
        BOOST_FOREACH(PointId cpid, c)
        {
            _noise[cpid] = true;
            _pointId_to_clusterId[cpid] = 0;
        }
    }


    nbClusters = cid;
}


void ClustererDBSCAN::find_clusters_WF()
{

    ClusterId cid = 1;
    CCluster c;   // a new cluster
    double bottom = _eps;
    double top = _eps;


    if (!_optics_list.empty()) {
        _reachability[_optics_list[0]] = 0; //first element is anyway reachable
        }

    for (int i = 0; i < _optics_list.size(); i++)
    {
        PointId pid = _optics_list[i];

        if (_reachability[pid]== -1) _reachability[pid] = _eps * 1.1;// undefined values are set to 10% more than Eps

        if (i>0 && _reachability[pid] < _reachability[_optics_list[i-1]]) // going down
        {
            if (_reachability[pid] < bottom)
            {
                bottom = _reachability[pid]; //keep track of the minima
            }
            if (_reachability[_optics_list[i-1]] > top)
            {
                top = _reachability[_optics_list[i-1]];  // previous maxima
            }
        }
        else   // going up
        {

            if (top > bottom + _depth && _reachability[pid] > bottom + _depth) // we detected a pit with enough depth
            {
                int j = i;
                for(; j>0 && _reachability[_optics_list[j-1]] < bottom + _depth ; j--) // let's fill it backwards
                {
                    if (!_noise[_optics_list[j-1]])
                    {
                        _pointId_to_clusterId[_optics_list[j-1]]=cid;
                        c.push_back(_optics_list[j-1]);
                    }

                }
                if (j>0){
                    _pointId_to_clusterId[_optics_list[j-1]]=cid;//add first point
                    c.push_back(_optics_list[j-1]);
                }
                if (!c.empty())
                {
                    if(c.size() >= _minPts) // cluster is valid
                    {
                        _clusters.push_back(c);
                        cid++;
                        c = CCluster();
                    }
                    else     // cluster is too small -> noise
                    {
                        BOOST_FOREACH(PointId cpid, c)
                        {
                            _noise[cpid] = true;
                            _pointId_to_clusterId[cpid] = 0;
                        }
                        c.clear();
                    }
                }
                top = _reachability[pid]; // restart from here
                bottom = _eps;

            }
        }


    }
    // and check if there is a last cluster

    if (top > bottom + _depth) // there is a last cluster here
    {
        int j = _optics_list.size();
        for(; j>0 && _reachability[_optics_list[j-1]] < bottom + _depth && _reachability[_optics_list[j-1]] < _eps ; j--)
        {
            if (!_noise[_optics_list[j-1]])
            {
                _pointId_to_clusterId[_optics_list[j-1]]=cid;
                c.push_back(_optics_list[j-1]);
            }

        }
        if (j>0){
            _pointId_to_clusterId[_optics_list[j-1]]=cid;//add first point
            c.push_back(_optics_list[j-1]);
        }
        if (!c.empty())
        {
            if(c.size() >= _minPts) // cluster is valid
            {
                _clusters.push_back(c);
                cid++;
                c = CCluster();
            }
            else     // cluster is too small -> noise
            {
                BOOST_FOREACH(PointId cpid, c)
                {
                    _noise[cpid] = true;
                    _pointId_to_clusterId[cpid] = 0;
                }
                c.clear();
            }
        }
    }

    nbClusters = cid;
}


// compute the core-distance
double ClustererDBSCAN::core_distance(PointId pid,double threshold)
{
        QMultiMap<double,int> p;
        for (unsigned int j=0; j < _sim.size1(); j++)
        {
                if 	((pid != j ) && (_sim(pid, j)) < threshold)
                {
                    p.insert(_sim(pid, j),j);
                }
        }
        QList<double> k = p.keys();
        if (k.size()<_minPts)
        {
            return -1;
        }
        else
        {
            return k[_minPts-1];
        }

}


Neighbors ClustererDBSCAN::findNeighbors(PointId pid, double threshold)
{
        Neighbors ne;

        for (unsigned int j=0; j < _sim.size1(); j++)
        {
                if 	((pid != j ) && (_sim(pid, j)) < threshold)
                {
                        ne.push_back(j);
                }
        }
        return ne;
}

template <typename Distance_type>
void ClustererDBSCAN::computeSimilarity(Distance_type & d,Points samples)
{
        unsigned int size = samples.size();
        _sim.resize(size, size, false);
        for (unsigned int i=0; i < size; i++)
        {
                for (unsigned int j=i+1; j < size; j++)
                {
                        _sim(j, i) = _sim(i, j) = d.distance(samples[i], samples[j]);

                }
        }
}

