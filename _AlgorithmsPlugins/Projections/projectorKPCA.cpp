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
#include "projectorKPCA.h"
#include <QDebug>

using namespace std;

ProjectorKPCA::ProjectorKPCA()
    : pca(0)
{}


std::vector<fvec> ProjectorKPCA::Project(std::vector<fvec> samples)
{
    if(!samples.size() || !pca) return samples;
    vector<fvec> projected;
    int dim = samples[0].size();

    FOR(i, samples.size()) samples[i] -= mean;

    // we dump the data in a matrix
    MatrixXd data(samples[0].size(), samples.size());
    FOR(i, samples.size())
    {
        FOR(d, dim) data(d,i) = samples[i][d];
    }

    MatrixXd projections = pca->project(data, dim);
    projected.clear();
    projected.resize(projections.rows());
    fvec sample;
    sample.resize(projections.cols());
    FOR(i, projections.rows())
    {
        FOR(d, projections.cols())
        {
            sample[d] = projections(i,d); // we recenter the data
        }
        projected[i] = sample;
    }
    fvec diffValues = maxValues - minValues;
    // we renormalize the results in a 0-1 space
    FOR(i, projected.size())
    {
        FOR(d, projected[0].size())
        {
            projected[i][d] = ((projected[i][d]-minValues[d])/diffValues[d])*0.9f + 0.05f;
        }
    }

    return projected;
}

void ProjectorKPCA::Train(std::vector< fvec > samples, ivec labels)
{
    projected.clear();
    source.clear();
    if(!samples.size()) return;
    source = samples;
    dim = samples[0].size();
    if(!dim) return;

    this->labels = labels;
    int count = samples.size();

    // we center the data
    mean = samples[0];
    FOR(i, count-1) mean += samples[i+1];
    mean /= count;
    FOR(i, count) samples[i] -= mean;

    // we dump the data in a matrix
    MatrixXd data(samples[0].size(), samples.size());
    FOR(i, count)
    {
        FOR(d, dim) data(d,i) = samples[i][d];
    }

    // and we run the algorithm
    DEL(pca);
    pca = new PCA();
    pca->kernelType = kernelType;
    pca->degree = kernelDegree;
    pca->gamma = kernelGamma;

    pca->kernel_pca(data, dim);
    MatrixXd projections = pca->get();
    projected.resize(projections.rows());
    fvec sample;
    sample.resize(projections.cols());
    minValues.clear();
    maxValues.clear();
    minValues.resize(projections.cols(), FLT_MAX);
    maxValues.resize(projections.cols(), -FLT_MAX);
    FOR(i, projections.rows())
    {
        FOR(d, projections.cols())
        {
            sample[d] = projections(i,d) + 0.5; // we recenter the data
            if(sample[d] < minValues[d]) minValues[d] = sample[d];
            if(sample[d] > maxValues[d]) maxValues[d] = sample[d];
        }
        projected[i] = sample;
    }
    fvec diffValues = maxValues - minValues;
    // we renormalize the results in a -0.5 - 0.5 space
    FOR(i, projected.size())
    {
        FOR(d, projected[0].size())
        {
            projected[i][d] = ((projected[i][d]-minValues[d])/diffValues[d])*0.9f + 0.05f - 0.5f;
        }
    }
}

fvec ProjectorKPCA::Project(const fvec &sample)
{
    fvec estimate(1);
    if(!pca) return estimate;

    VectorXd point(sample.size());
    FOR(i, sample.size()) point(i) = sample[i];
    estimate[0] = pca->test(point);
    return estimate;
}

void ProjectorKPCA::SetParams(int kernelType, int kernelDegree, float kernelGamma)
{
    this->kernelType = kernelType;
    this->kernelDegree = kernelDegree;
    this->kernelGamma = kernelGamma;
}

char *ProjectorKPCA::GetInfoString()
{
    char *text = new char[1024];

    sprintf(text, "%sKernel Principal Component Analysis: ", text);
    switch(kernelType)
    {
    case 0:
        sprintf(text, "%s linear\n", text);
        break;
    case 1:
        sprintf(text, "%s polynomial (deg: %f)\n", text, kernelDegree);
        break;
    case 2:
        sprintf(text, "%s rbf (gamma: %f)\n", text, kernelGamma);
        break;
    }
    return text;
}
