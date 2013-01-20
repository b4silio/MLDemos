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
#include "basicMath.h"
#include "regressorGB.h"


RegressorGB::RegressorGB()
    : boostIters(0), boostLossType(0), boostTreeDepths(0), gbt(0)
{
}

RegressorGB::~RegressorGB()
{
    DEL(gbt);
}
void RegressorGB::Train(std::vector< fvec > samples, ivec labels)
{
    u32 sampleCnt = samples.size();
    if(!sampleCnt) return;
    dim = samples[0].size();
    if(outputDim != -1 && outputDim < dim -1)
    {
        // we need to swap the current last dimension with the desired output
        FOR(i, samples.size())
        {
            float val = samples[i][dim-1];
            samples[i][dim-1] = samples[i][outputDim];
            samples[i][outputDim] = val;
        }
    }

    DEL(gbt);
    dim = samples[0].size()-1;
    // convert the samples into CvMat* format
    CvMat *trainSamples = cvCreateMat(sampleCnt, dim, CV_32FC1);
    CvMat *trainOutputs = cvCreateMat(sampleCnt, 1, CV_32FC1);
    const CvMat *sampleIdx = 0;//cvCreateMat(samples[0].size(), 1, CV_32FC1);//dim;//samples[0].size();// should be dim
    const CvMat *varIdx = 0;// 0 if the sused ubset if not set
    const CvMat *varType = 0;
    const CvMat *missingDataMask = 0;
    int tflag = CV_ROW_SAMPLE;

    //loss_function_type – Type of the loss function used for training (see Training the GBT model). It must be one of the following types: CvGBTrees::SQUARED_LOSS, CvGBTrees::ABSOLUTE_LOSS, CvGBTrees::HUBER_LOSS, CvGBTrees::DEVIANCE_LOSS. The first three types are used for regression problems, and the last one for classification.
    int activationFunction;
    switch(boostLossType==1)
    {
    case 1:
        activationFunction = CvGBTrees::SQUARED_LOSS;
        break;
    case 2:
        activationFunction = CvGBTrees::ABSOLUTE_LOSS;
        break;
    case 3:
        activationFunction = CvGBTrees::HUBER_LOSS;
        break;
    }
    //weak_count – Count of boosting algorithm iterations. weak_count*K is the total count of trees in the GBT model, where K is the output classes count (equal to one in case of a regression).
    int weak_count = boostIters;
    //shrinkage – Regularization parameter (see Training the GBT model).
    float shrinkage = 0.1f;
    //subsample_portion – Portion of the whole training set used for each algorithm iteration. Subset is generated randomly. For more information see http://www.salfordsystems.com/doc/StochasticBoostingSS.pdf.
    float subsample_portion = 0.5f;
    //max_depth – Maximal depth of each decision tree in the ensemble (see CvDTree).
    int max_depth = boostTreeDepths;
    //use_surrogates – If true, surrogate splits are built (see CvDTree).
    bool use_surrogates = false;
    CvGBTreesParams params=  CvGBTreesParams(activationFunction, weak_count, shrinkage, subsample_portion, max_depth, use_surrogates);
    u32 *perm = randPerm(sampleCnt);

    FOR(i, sampleCnt)
    {
        FOR(j, dim) cvSetReal2D(trainSamples, i, j, samples[perm[i]][j]);
        cvSet1D(trainOutputs, i, cvScalar(samples[perm[i]][dim]));
    }

    delete [] perm;
    gbt = new CvGBTrees();
    gbt->train(trainSamples, tflag,trainOutputs,varIdx,sampleIdx,varType,missingDataMask,params);
    cvReleaseMat(&trainSamples);
    cvReleaseMat(&trainOutputs);
}

fvec RegressorGB::Test( const fvec &sample)
{
    fvec res;
    res.resize(2);
    if(!gbt) return res;
    float *_input = new float[dim];
    if(outputDim != -1 & outputDim < sample.size())
    {
        fvec newSample = sample;
        newSample[outputDim] = sample[sample.size()-1];
        newSample[sample.size()-1] = sample[outputDim];
        FOR(d, min(dim,(u32)sample.size())) _input[d] = newSample[d];
        for(int d=min(dim,(u32)sample.size()); d<dim; d++) _input[d] = 0;
    }
    else
    {
        FOR(d, min(dim,(u32)sample.size())) _input[d] = sample[d];
        for(int d=min(dim,(u32)sample.size()); d<dim; d++) _input[d] = 0;
    }
    CvMat input = cvMat(1,dim,CV_32FC1, _input);
    float output;
    output = gbt->predict(&input);
    res[0] = output;
    res[1] = 0;
    delete [] _input;
    return res;
}

void RegressorGB::SetParams(int boostIters, int boostLossType, int boostTreeDepths)
{
    this->boostIters = boostIters;
    this->boostLossType = boostLossType;
    this->boostTreeDepths = boostTreeDepths;
}

const char *RegressorGB::GetInfoString()
{
    char *text = new char[1024];
    sprintf(text, "Gradient Boosting Tree\n");
    sprintf(text, "\n");
    return text;
}
