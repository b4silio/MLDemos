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

ProjectorKPCA::ProjectorKPCA(int targetDims)
    : pca(0), targetDims(targetDims)
{}

ProjectorKPCA::~ProjectorKPCA()
{
    DEL(pca);
}

std::vector<fvec> ProjectorKPCA::Project(std::vector<fvec> samples)
{
    if(!samples.size() || !pca) return samples;
    vector<fvec> projected;
    int dim = samples[0].size();

    FOR(i, samples.size()) samples[i] -= mean;

    // we dump the data in a matrix
    MatrixXd data(dim, samples.size());
    FOR(i, samples.size())
    {
        FOR(d, dim) data(d,i) = samples[i][d];
    }

    MatrixXd projections = pca->project(data, targetDims);
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

    if(maxValues.size())
    {
        fvec diffValues = maxValues - minValues;
        // we renormalize the results in a 0-1 space
        FOR(i, projected.size())
        {
            FOR(d, projected[i].size())
            {
                projected[i][d] = ((projected[i][d]-minValues[d])/diffValues[d])*0.9f + 0.05f;
            }
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
    int count = samples.size();
    if(targetDims > count) targetDims = count;

    this->labels = labels;

    // we center the data
    mean = samples[0];
    FOR(i, count-1) mean += samples[i+1];
    mean /= (float)count;
    FOR(i, count) samples[i] -= mean;

    // we dump the data in a matrix
    MatrixXd data(dim, count);
    FOR(i, count)
    {
        FOR(d, dim) data(d,i) = samples[i][d];
    }

    // and we run the algorithm
    DEL(pca);
    pca = new PCA();
    pca->kernelType = kernelType;
    pca->degree = kernelDegree;
    pca->gamma = 1.f/kernelGamma;
    pca->offset = kernelGamma;

    pca->kernel_pca(data, targetDims);

    projected = Project(source);
    minValues.clear();
    maxValues.clear();
    minValues.resize(projected.size(), FLT_MAX);
    maxValues.resize(projected.size(), -FLT_MAX);
    FOR(i, projected.size())
    {
        FOR(j, projected[i].size())
        {
            if(projected[i][j] < minValues[j]) minValues[j] = projected[i][j];
            if(projected[i][j] > maxValues[j]) maxValues[j] = projected[i][j];
        }
    }
    /*
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
            sample[d] = projections(i,d);
            //sample[d] = projections(i,d) + 0.5; // we recenter the data
            if(sample[d] < minValues[d]) minValues[d] = sample[d];
            if(sample[d] > maxValues[d]) maxValues[d] = sample[d];
        }
        projected[i] = sample;
    }
    */

    fvec diffValues = maxValues - minValues;
    FOR(d, diffValues.size()) if(diffValues[d] == 0) diffValues[d] = 1.f;
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

void ProjectorKPCA::SetParams(int kernelType, float kernelDegree, float kernelGamma)
{
    this->kernelType = kernelType;
    this->kernelDegree = kernelDegree;
    this->kernelGamma = kernelGamma;
}

const char *ProjectorKPCA::GetInfoString()
{
    char *text = new char[1024];

    sprintf(text, "%sKernel Principal Component Analysis: ", text);
    switch(kernelType)
    {
    case 0:
        sprintf(text, "%s linear\n", text);
        break;
    case 1:
        sprintf(text, "%s polynomial (deg: %f offset: %f)\n", text, kernelDegree, kernelGamma);
        break;
    case 2:
        sprintf(text, "%s rbf (gamma: %f)\n", text, kernelGamma);
        break;
    case 3:
        sprintf(text, "%s sigmoid (scale: %f offset: %f)\n", text, kernelDegree, kernelGamma);
        break;
    }
    return text;
}
