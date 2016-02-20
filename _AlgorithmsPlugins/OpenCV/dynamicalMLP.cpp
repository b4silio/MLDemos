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
#include "dynamicalMLP.h"

using namespace std;
using namespace cv;
using namespace cv::ml;

DynamicalMLP::DynamicalMLP()
    : functionType(1), neuronCount(2), alpha(0), beta(0), trainingType(1)
{
	type = DYN_MLP;
}

DynamicalMLP::~DynamicalMLP()
{
}

void DynamicalMLP::Train(std::vector< std::vector<fvec> > trajectories, ivec labels)
{
	if(!trajectories.size()) return;
	int count = trajectories[0].size();
	if(!count) return;
	dim = trajectories[0][0].size()/2;
	// we forget about time and just push in everything
	vector<fvec> samples;
	FOR(i, trajectories.size())
	{
		FOR(j, trajectories[i].size())
		{
			samples.push_back(trajectories[i][j]);
		}
	}
	u32 sampleCnt = samples.size();
	if(!sampleCnt) return;

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
        layers.at<int>(2+layerCount-1) = dim; // outputs
    }

	u32 *perm = randPerm(sampleCnt);

    Mat trainSamples(sampleCnt, dim, CV_32FC1);
    Mat trainOutputs(sampleCnt, dim, CV_32FC1);
	FOR(i, sampleCnt)
	{
        FOR(d, dim) trainSamples.at<float>(i,d) = samples[perm[i]][d];
        FOR(d, dim) trainOutputs.at<float>(i,d) = samples[perm[i]][dim+d];
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

std::vector<fvec> DynamicalMLP::Test( const fvec &sample, const int count)
{
	fvec start = sample;
	dim = sample.size();
	std::vector<fvec> res(count);
	FOR(i, count) res[i].resize(dim,0);
	if(!mlp) return res;
    Mat input(1, dim, CV_32FC1);
    Mat output(1,dim,CV_32FC1);

    fvec velocity; velocity.resize(dim,0);
	FOR(i, count)
	{
		res[i] = start;
		start += velocity*dT;
        FOR(d, dim) input.at<float>(d) = start[d];
        mlp->predict(input, output);
        FOR(d, dim) velocity[d] = output.at<float>(d);
	}
	return res;
}

fvec DynamicalMLP::Test( const fvec &sample)
{
	int dim = sample.size();
	fvec res(2);
	if(!mlp) return res;
    Mat input(1,dim, CV_32FC1);
    Mat output(1,dim, CV_32FC1);
    FOR(d, dim) input.at<float>(d) = sample[d];
    mlp->predict(input, output);
    FOR(d,dim) res[d] = output.at<float>(d);
	return res;
}

void DynamicalMLP::SetParams(u32 functionType, u32 neuronCount, u32 layerCount, f32 alpha, f32 beta, u32 trainingType)
{
	this->functionType = functionType;
	this->neuronCount = neuronCount;
	this->layerCount = layerCount;
	this->alpha = alpha;
	this->beta = beta; 
    this->trainingType = trainingType;
}


const char *DynamicalMLP::GetInfoString()
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
