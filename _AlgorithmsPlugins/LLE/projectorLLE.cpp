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
#include <public.h>
#include "projectorLLE.h"
#include <QDebug>
#include <assert.h>

using namespace std;
using namespace Eigen;

ProjectorLLE::ProjectorLLE(int targetDims)
    : targetDims(targetDims), knn(0), dataPts(0), kdTree(0)
{}

ProjectorLLE::~ProjectorLLE()
{
    annClose();
    DEL(kdTree);    
    if (dataPts != 0) annDeallocPts(dataPts);
}

void ProjectorLLE::computeReconstructionWeights(MatrixXd &W, MatrixXd &p)
{
    ANNidxArray nnIdxQ = new ANNidx[knn+1]; // allocate near neighbour indices
    ANNidxArray nnIdx = new ANNidx[knn];
    ANNdistArray dists = new ANNdist[knn+1]; // allocate near neighbour dists

    assert(W.cols() == data.cols() && W.rows() == p.cols());
    assert(p.rows() == dim);

    W.setZero();
    MatrixXd nn(dim, knn);

    ANNpoint queryPt; // query point
    queryPt = annAllocPt(dim); // allocate query point
    qDebug() << "LLE: Computing reconstruction weights .. hold on";
    FOR(i, p.cols())
    {        
        //qDebug() << endl << i << ": ";
        //
        // compute the k-nearest neighbours of i in the data
        //
        double eps = 0; // error bound
        FOR(j, dim) queryPt[j] = p(j,i);
        kdTree->annkSearch(queryPt, knn+1, nnIdxQ, dists, eps);
        FOR(j, knn) nnIdx[j] = nnIdxQ[j+1];  // skip the first one, as it is same as the query point.

        // collect the k-nn centered on the query point
        nn.setZero();
        FOR(j, knn)
        {
            //qDebug() << nnIdx[j] << " ";
            FOR(k, dim) nn(k,j) = data(k,nnIdx[j]) - p(k,i);
        }

        //
        // find the weights that minimize the rss of reconstructing i from linear combination its k-nn
        //
        MatrixXd G = nn.transpose() * nn;
        MatrixXd eye = MatrixXd::Identity(knn, knn);
        // regularization on w in case C is singular. This will happen if knn > #input dimensions
        double tol;
        tol = (knn>dim) ? 0.001 : 0;
        G = G + eye*tol*G.trace();

        MatrixXd w = MatrixXd::Zero(knn, 1);
        MatrixXd b = MatrixXd::Ones(knn, 1);
        w = G.lu().solve(b);
        w = w/w.sum();

        // record the weights
        FOR(j, knn) W(i, nnIdx[j]) = w(j,0);
    }
    qDebug() << "done";

    annDeallocPt(queryPt);
    delete [] nnIdx;
    delete [] nnIdxQ;
    delete [] dists;
}

void ProjectorLLE::computeEmbedding(MatrixXd& W, MatrixXd& Y)
{
    assert(W.rows() == data.cols() && W.cols() == data.cols());

    MatrixXd eye = MatrixXd::Identity(W.rows(), W.cols());
    MatrixXd T(W.rows(), W.cols()), M(W.rows(), W.cols());

    T = eye-W;
    M = T.transpose() * T;

    qDebug() << "LLE: Finding eigen vectors .. hold on";
    EigenSolver<MatrixXd> eigM(M);
    qDebug() << "done";   
    eigenvalues = eigM.eigenvalues().real();
    eigenVectors = eigM.eigenvectors().real();

    // FOR(i, M.cols()) qDebug() << eigenvalues(i);

    // sort and get the permutation indices
    pi.clear();
    for (int i = 0 ; i < M.cols(); i++)
        pi.push_back(std::make_pair(eigenvalues(i), i));

    std::sort(pi.begin(), pi.end());

    // get bottom targetDims eigenvectors leaving out the smallest which corresponds to evec of all 1s.
    Y.setZero();
    // FOR(j, M.cols()) qDebug() << eigenVectors(j, pi[0].second);
    for (unsigned int i = 1; i < targetDims+1; i++)
    {
        Y.row(i-1) = eigenVectors.col(pi[i].second)*sqrt(M.cols());
    }
}

void ProjectorLLE::Train(std::vector< fvec > samples, ivec labels)
{
    projected.clear();
    source.clear();
    if(!samples.size()) return;
    source = samples;
    dim = samples[0].size();
    if(!dim) return;
    int count = samples.size();
    if(targetDims > count) targetDims = count;

    // we dump the data in a matrix
    data.resize(dim, count);
    data.setZero();
    FOR(i, count)
    {
        FOR(d, dim) data(d,i) = samples[i][d];
    }

    // initialize k-nearest neighbours structure
    annClose();
    DEL(kdTree);    
    if (dataPts != NULL) annDeallocPts(dataPts);
    dataPts = annAllocPts(count, dim);			// allocate data points
    FOR(i, count)
    {
        FOR(j, dim) dataPts[i][j] = samples[i][j];
    }
    kdTree = new ANNkd_tree(dataPts, count, dim);

    // compute reconstruction weights
    MatrixXd W(count, count);
    computeReconstructionWeights(W, data);

    // compute the embedding
    y.resize(targetDims, count);
    y.setZero();
    computeEmbedding(W, y);

    projected.resize(y.cols());
    fvec sample(y.rows());
    FOR(i, y.cols())
    {
        FOR(d, y.rows()) sample[d] = y(d,i);
        projected[i] = sample;
    }
}

fvec ProjectorLLE::Project(const fvec &sample)
{
    fvec estimate;
    estimate = sample;
    return estimate;

//    assert(sample.size() == dim);

//    MatrixXd W(1, data.cols());
//    MatrixXd p(dim, 1);
//    FOR(i, sample.size()) p(i, 0) = sample[i];
//    computeReconstructionWeights(W, p);

//    MatrixXd res = MatrixXd::Zero(targetDims, 1);
//    res = y * W.transpose();

//    fvec estimate;
//    estimate.resize(targetDims);
//    FOR(i, targetDims) estimate[i] = res(i,0);

//    return estimate;
}

void ProjectorLLE::SetParams(int knn)
{
    this->knn = knn;
}

const char *ProjectorLLE::GetInfoString()
{
    char *text = new char[1024];

    sprintf(text, "%sLocally Linear Embedding : %d neighbours", text);
    return text;
}
