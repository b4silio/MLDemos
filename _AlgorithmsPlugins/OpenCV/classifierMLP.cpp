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
#include "classifierMLP.h"

ClassifierMLP::~ClassifierMLP()
{
	DEL(mlp);
}

void ClassifierMLP::Train(std::vector< fvec > samples, ivec labels)
{
	u32 sampleCnt = samples.size();
	if(!sampleCnt) return;
	DEL(mlp);
	dim = samples[0].size();

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
	CvMat *trainLabels = cvCreateMat(labels.size(), 1, CV_32FC1);
	CvMat *sampleWeights = cvCreateMat(samples.size(), 1, CV_32FC1);
	FOR(i, sampleCnt)
	{
		FOR(d, dim) cvSetReal2D(trainSamples, i, d, samples[perm[i]][d]);
		cvSet1D(trainLabels, i, cvScalar(labels[perm[i]]));
		cvSet1D(sampleWeights, i, cvScalar(1));
	}

	delete [] perm;

	int activationFunction = functionType == 2 ? CvANN_MLP::GAUSSIAN : functionType ? CvANN_MLP::SIGMOID_SYM : CvANN_MLP::IDENTITY;


	mlp = new CvANN_MLP();
	mlp->create(layers, activationFunction, alpha, beta);

	CvANN_MLP_TrainParams params;
	params.term_crit = cvTermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 1000, 0.001);
	mlp->train(trainSamples, trainLabels, sampleWeights, 0, params);
	cvReleaseMat(&trainSamples);
	cvReleaseMat(&trainLabels);
	cvReleaseMat(&sampleWeights);
	cvReleaseMat(&layers);
}

float ClassifierMLP::Test( const fvec &sample)
{
	if(!mlp) return 0;
	float *_input = new float[dim];
	FOR(d, dim) _input[d] = sample[d];
	CvMat input = cvMat(1,dim,CV_32FC1, _input);
	float _output[1];
	CvMat output = cvMat(1,1,CV_32FC1, _output);
	mlp->predict(&input, &output);
	delete [] _input;
	return _output[0];
}

void ClassifierMLP::SetParams(u32 functionType, u32 neuronCount, u32 layerCount, f32 alpha, f32 beta)
{
	this->functionType = functionType;
	this->neuronCount = neuronCount;
	this->layerCount = layerCount;
	this->alpha = alpha;
	this->beta = beta; 
}

const char *ClassifierMLP::GetInfoString()
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
