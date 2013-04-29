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

RegressorMLP::RegressorMLP()
: functionType(1), neuronCount(2), mlp(0), alpha(0), beta(0), trainingType(1)
{
	type = REGR_MLP;
}

RegressorMLP::~RegressorMLP()
{
	DEL(mlp);
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
    DEL(mlp);
    dim = samples[0].size()-1;

	CvMat *layers;
	//	if(neuronCount == 3) neuronCount = 2; // don't ask me why but 3 neurons mess up everything...

	if(!layerCount || neuronCount < 2)
	{
		layers = cvCreateMat(2,1,CV_32SC1);
		cvSet1D(layers, 0, cvScalar(dim));
		cvSet1D(layers, 1, cvScalar(1));
	}
	else
	{
		layers = cvCreateMat(2+layerCount,1,CV_32SC1);
		cvSet1D(layers, 0, cvScalar(dim));
		cvSet1D(layers, layerCount+1, cvScalar(1));
		FOR(i, layerCount) cvSet1D(layers, i+1, cvScalar(neuronCount));
	}

	u32 *perm = randPerm(sampleCnt);

	CvMat *trainSamples = cvCreateMat(sampleCnt, dim, CV_32FC1);
	CvMat *trainOutputs = cvCreateMat(sampleCnt, 1, CV_32FC1);
	CvMat *sampleWeights = cvCreateMat(samples.size(), 1, CV_32FC1);
	FOR(i, sampleCnt)
	{
		FOR(j, dim) cvSetReal2D(trainSamples, i, j, samples[perm[i]][j]);
		cvSet1D(trainOutputs, i, cvScalar(samples[perm[i]][dim]));
		cvSet1D(sampleWeights, i, cvScalar(1));
	}

	delete [] perm;

	int activationFunction = functionType == 2 ? CvANN_MLP::GAUSSIAN : functionType ? CvANN_MLP::SIGMOID_SYM : CvANN_MLP::IDENTITY;

	mlp = new CvANN_MLP();
	mlp->create(layers, activationFunction, alpha, beta);

	CvANN_MLP_TrainParams params;
    params.term_crit = cvTermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 1000, 0.0001);
    params.train_method = trainingType ? CvANN_MLP_TrainParams::RPROP : CvANN_MLP_TrainParams::BACKPROP;
    mlp->train(trainSamples, trainOutputs, sampleWeights, 0, params);
	cvReleaseMat(&trainSamples);
	cvReleaseMat(&trainOutputs);
	cvReleaseMat(&sampleWeights);
	cvReleaseMat(&layers);
}

fvec RegressorMLP::Test( const fvec &sample)
{
	fvec res;
	res.resize(2);
	if(!mlp) return res;
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
	float _output[1];
	CvMat output = cvMat(1,1,CV_32FC1, _output);
	mlp->predict(&input, &output);
	res[0] = _output[0];
	delete [] _input;
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
