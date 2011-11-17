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
#include "regressorSVR.h"

using namespace std;

char *RegressorSVR::GetInfoString()
{
	if(!svm) return NULL;
	char *text = new char[255];
	sprintf(text, "%s\n", param.svm_type == NU_SVR ? "nu-SVR" : "eps-SVR");
	sprintf(text, "%sKernel: ", text);
	switch(param.kernel_type)
	{
	case LINEAR:
		sprintf(text, "%s linear\n", text);
		break;
	case POLY:
		sprintf(text, "%s polynomial (deg: %f bias: %f width: %f)\n", text, param.degree, param.coef0, param.gamma);
		break;
	case RBF:
		sprintf(text, "%s rbf (gamma: %f)\n", text, param.gamma);
		break;
	case SIGMOID:
		sprintf(text, "%s sigmoid (%f %f)\n", text, param.gamma, param.coef0);
		break;
	}
	sprintf(text, "%seps: %f \t nu: %f\n", text, param.eps, param.nu);
	sprintf(text, "%sSupport Vectors: %d\n", text, svm->l);
	return text;
}

RegressorSVR::RegressorSVR()
: svm(0), node(0)
{
	type = REGR_SVR;
	// default values
	param.svm_type = EPSILON_SVR;
	//param.svm_type = NU_SVR;
	param.kernel_type = RBF;
	param.gamma = 0.1;
	param.C = 100;
	param.nu = 0.1;
	param.p = 0.3;

	param.degree = 1;
	param.coef0 = 0;
	param.shrinking = 1;
	param.probability = 0;
	param.eps = 1e-6;
	param.cache_size = 400;
	param.nr_weight = 0;
	param.weight_label = NULL;
	param.weight = NULL;
	param.kernel_weight = NULL;
	param.kernel_dim = 0;
	param.kernel_norm = 1.;
	param.normalizeKernel = false;
}

RegressorSVR::~RegressorSVR()
{
	DEL(node);
}

void RegressorSVR::Train(std::vector< fvec > samples, ivec labels)
{
	svm_problem problem;
	svm_node *x_space;

    int dim = samples[0].size()-1;
    int oDim = outputDim != -1 && outputDim < dim ? outputDim : dim;
	problem.l = samples.size();
	problem.y = new double[problem.l];
	problem.x = new svm_node *[problem.l];
    x_space = new svm_node[(dim+1)*problem.l];

	FOR(i, problem.l)
	{
        FOR(j, dim)
		{
            x_space[(dim+1)*i + j].index = j+1;
            x_space[(dim+1)*i + j].value = samples[i][j];
		}
        x_space[(dim+1)*i + dim].index = -1;
        if(outputDim != -1 && outputDim < dim) x_space[(dim+1)*i + outputDim].value = samples[i][dim];
        problem.x[i] = &x_space[(dim+1)*i];
        problem.y[i] = samples[i][oDim];
	}

	DEL(svm);
	DEL(node);
	svm = svm_train(&problem, &param);

	delete [] problem.x;
	delete [] problem.y;

	bFixedThreshold = true;
	classThresh = 0.5f;
}

fvec RegressorSVR::Test( const fvec &sample )
{
    int dim = sample.size()-1;
	float estimate;
	if(!node)
	{
        node = new svm_node[dim+1];
        node[dim].index = -1;
	}
    FOR(i, dim)
	{
		node[i].index = i+1;
		node[i].value = sample[i];
	}
    if(outputDim != -1 && outputDim < dim) node[outputDim].value = sample[dim];
	estimate = (float)svm_predict(svm, node);
	fvec res;
	res.push_back(estimate);
	res.push_back(1);
	return res;
}

fVec RegressorSVR::Test( const fVec &sample )
{
    int dim = 1;
	float estimate;
    if(!node) node = new svm_node[dim+1];
    FOR(i, dim)
	{
		node[i].index = i+1;
		node[i].value = sample._[i];
	}
    node[dim].index = -1;
	estimate = (float)svm_predict(svm, node);
	return fVec(estimate,1);
}

void RegressorSVR::SetParams(int svmType, float svmC, float svmP, u32 kernelType, float kernelParam)
{
	// default values
	param.svm_type = svmType;
	param.C = svmC;
	param.nu = svmC;
	param.eps = 0.01;
	param.p = svmP;

	param.coef0 = 0;
	param.gamma = 1;

	switch(kernelType)
	{
	case 0:
		param.kernel_type = LINEAR;
		param.degree = 1;
		break;
	case 1:
		param.kernel_type = POLY;
		param.degree = (u32)kernelParam;
		break;
	case 2:
		param.kernel_type = RBF;
		param.gamma = kernelParam;
		break;
	case 3:
		param.kernel_type = SIGMOID;
		param.gamma = kernelParam;
		break;
	}
}
