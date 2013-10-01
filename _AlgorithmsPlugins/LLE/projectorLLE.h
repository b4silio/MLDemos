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
#ifndef _PROJECTOR_LLE_H_
#define _PROJECTOR_LLE_H_

#include <vector>
#include <projector.h>
#include "ANN/ANN.h"
#include <Eigen/Core>
#include <Eigen/Eigen>

typedef std::pair<double, int> myPair;
typedef std::vector<myPair> PermutationIndices;

class ProjectorLLE : public Projector
{
private:
    int knn;
    ANNpointArray		dataPts;				// data points
    ANNkd_tree*			kdTree;					// search structure
    Eigen::MatrixXd data;
    Eigen::MatrixXd y;
    PermutationIndices pi;


    void computeReconstructionWeights(Eigen::MatrixXd& W, Eigen::MatrixXd& points);
    void computeEmbedding(Eigen::MatrixXd& W, Eigen::MatrixXd& Y);

public:
    int targetDims;
    Eigen::VectorXd eigenvalues;
    Eigen::MatrixXd eigenVectors;

    ProjectorLLE(int targetDims=-1);
    ~ProjectorLLE();
    void Train(std::vector< fvec > samples, ivec labels);
    fvec Project(const fvec &sample);

    const char *GetInfoString();
    void SetParams(int knn);
};

#endif // _PROJECTOR_LLE_H_
