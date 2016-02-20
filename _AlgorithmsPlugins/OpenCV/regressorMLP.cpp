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
#include "regressorMLP.h"

using namespace cv;
using namespace cv::ml;

RegressorMLP::RegressorMLP()
: functionType(1), neuronCount(2), alpha(0), beta(0), trainingType(1)
{
	type = REGR_MLP;
}

RegressorMLP::~RegressorMLP()
{
}

void RegressorMLP::Train(std::vector< fvec > samples, ivec labels)
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
    dim = samples[0].size()-1;

    Mat layers;

    if(!layerCount || neuronCount < 2)
    {
        layers = Mat(1, 2, CV_32SC1 );
        layers.at<int>(0) = dim; // input size
        layers.at<int>(1) = 1; // outputs
    }
    else
    {
        layers = Mat(1, 2+layerCount, CV_32SC1 );
        layers.at<int>(0) = dim; // input size
        FOR(i, layerCount) layers.at<int>(i+1) = neuronCount;
        layers.at<int>(2+layerCount-1) = 1; // outputs
    }

	u32 *perm = randPerm(sampleCnt);
    Mat trainSamples(sampleCnt, dim, CV_32FC1);
    Mat trainOutputs(sampleCnt, 1, CV_32FC1);
	FOR(i, sampleCnt)
	{
        FOR(d, dim) trainSamples.at<float>(i,d) = samples[perm[i]][d];
        trainOutputs.at<float>(i) = samples[perm[i]][dim];
	}
    delete [] perm;

    int activationFunction = functionType == 2 ? ANN_MLP::GAUSSIAN : functionType ? ANN_MLP::SIGMOID_SYM : ANN_MLP::IDENTITY;

    mlp = ANN_MLP::create();
    mlp->setLayerSizes(layers);
    mlp->setActivationFunction(activationFunction, alpha, beta);
    mlp->setTermCriteria(cv::TermCriteria(cv::TermCriteria::COUNT + cv::TermCriteria::EPS, 1000, 0.0001));
    mlp->setTrainMethod(trainingType ? ANN_MLP::RPROP : ANN_MLP::BACKPROP);
    if(trainingType) {
        mlp->setRpropDWMin(0.0001);
        mlp->setRpropDWMax(1000);
        mlp->setRpropDW0(0.1);
        mlp->setRpropDWPlus(1.2);
        mlp->setRpropDWMinus(0.8);
    }
    Ptr<ml::TrainData> trainData = ml::TrainData::create(trainSamples, ROW_SAMPLE, trainOutputs);
    mlp->train(trainData);
}

fvec RegressorMLP::Test( const fvec &sample)
{
    fvec res(2,0);
	if(!mlp) return res;
    Mat input = Mat(1, dim, CV_32FC1);
    Mat output = Mat(1,1,CV_32FC1);
    if(outputDim != -1 & outputDim < sample.size())
    {
        fvec newSample = sample;
        newSample[outputDim] = sample[sample.size()-1];
        newSample[sample.size()-1] = sample[outputDim];
        FOR(d, min(dim,(u32)sample.size())) input.at<float>(d) = newSample[d];
        for(int d=min(dim,(u32)sample.size()); d<dim; d++) input.at<float>(d) = 0;
    }
    else
    {
        FOR(d, min(dim,(u32)sample.size())) input.at<float>(d) = sample[d];
        for(int d=min(dim,(u32)sample.size()); d<dim; d++) input.at<float>(d) = 0;
    }
    float result = mlp->predict(input, output);
    res[0] = output.at<float>(0);
    return res;
}

void RegressorMLP::SetParams(u32 functionType, u32 neuronCount, u32 layerCount, f32 alpha, f32 beta, u32 trainingType)
{
	this->functionType = functionType;
	this->neuronCount = neuronCount;
	this->layerCount = layerCount;
    this->trainingType = trainingType;
    this->alpha = alpha;
	this->beta = beta; 
}

const char *RegressorMLP::GetInfoString()
{
	char *text = new char[1024];
	sprintf(text, "Multi-Layer Perceptron\n");
	sprintf(text, "%sLayers: %d\n", text, layerCount);
	sprintf(text, "%sNeurons: %d\n", text, neuronCount);
	sprintf(text, "%sActivation Function: ", text);
	switch(functionType)
	{
	case 0:
		sprintf(text, "%s identity\n", text);
		break;
	case 1:
		sprintf(text, "%s sigmoid (alpha: %f beta: %f)\n\t%s\n", text, alpha, beta, "beta*(1-exp(-alpha*x)) / (1 + exp(-alpha*x))");
		break;
	case 2:
		sprintf(text, "%s gaussian (alpha: %f beta: %f)\n\t%s\n", text, alpha, beta, "beta*exp(-alpha*x*x)");
		break;
	}
	return text;
}
