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
#include "dynamicalSVR.h"

using namespace std;

const char *DynamicalSVR::GetInfoString()
{
    if(!svms.size()) return NULL;
	char *text = new char[1024];
	sprintf(text, "%s\n", param.svm_type == NU_SVR ? "nu-SVR" : "eps-SVR");
	sprintf(text, "%sKernel: ", text);
	switch(param.kernel_type)
	{
	case LINEAR:
		sprintf(text, "%s linear\n", text);
		break;
	case POLY:
		sprintf(text, "%s polynomial (deg: %d bias: %f width: %f)\n", text, param.degree, param.coef0, param.gamma);
		break;
	case RBF:
		sprintf(text, "%s rbf (gamma: %f)\n", text, param.gamma);
		break;
	case SIGMOID:
		sprintf(text, "%s sigmoid (%f %f)\n", text, param.gamma, param.coef0);
		break;
	}
	sprintf(text, "%seps: %f \t nu: %f\n", text, param.eps, param.nu);
    sprintf(text, "%sSupport Vectors: %d\n", text, svms[0]->l);
	return text;
}

DynamicalSVR::DynamicalSVR()
: node(0)
{
	type = DYN_SVR;
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

DynamicalSVR::~DynamicalSVR()
{
    FOR(i, svms.size()) DEL(svms[i]);
    svms.clear();
	DEL(node);
}

void DynamicalSVR::Train(std::vector< std::vector<fvec> > trajectories, ivec labels)
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
	if(!samples.size()) return;
    FOR(i, svms.size()) DEL(svms[i]);
    svms.clear();
	DEL(node);

	svm_problem problem;
	svm_node *x_space;

	problem.l = samples.size();
    problem.x = new svm_node *[problem.l];
    problem.y = new double[problem.l];
    x_space = new svm_node[(dim+1)*problem.l];

	FOR(i, problem.l)
	{
		FOR(j, dim)
		{
			x_space[(dim+1)*i + j].index = j+1;
			x_space[(dim+1)*i + j].value = samples[i][j];
		}
		x_space[(dim+1)*i + dim].index = -1;
		problem.x[i] = &x_space[(dim+1)*i];
//        problem.y[i] = samples[i][dim];
	}
//    svms.push_back(svm_train(&problem, &param));
    FOR(d, dim)
    {
        FOR(i, problem.l) problem.y[i] = samples[i][dim + d];
        svms.push_back(svm_train(&problem, &param));
    }

    delete [] problem.x;
    delete [] problem.y;
}

std::vector<fvec> DynamicalSVR::Test( const fvec &sample, const int count)
{
	fvec start = sample;
	dim = sample.size();
    std::vector<fvec> res(count);
	FOR(i, count) res[i].resize(dim,0);
    if(svms.size() < dim) return res;
    fvec velocity(dim,0);

	if(!node) node = new svm_node[dim+1];
	FOR(i, dim)
	{
		node[i].index = i+1;
		node[i].value = start[i];
	}
	node[dim].index = -1;

	FOR(i, count)
	{
		res[i] = start;
		start += velocity*dT;

		FOR(d, dim) node[d].value = start[d];
        FOR(d, dim) velocity[d] = (float)svm_predict(svms[d], node);
	}
	return res;
}

fvec DynamicalSVR::Test( const fvec &sample )
{
	int dim = sample.size();
    if(svms.size() != dim) return sample;
	if(!node) node = new svm_node[dim+1];
	FOR(i, dim)
	{
		node[i].index = i+1;
		node[i].value = sample[i];
	}
	node[dim].index = -1;
    fvec res(dim);
    FOR(d, dim) res[d] = (float)svm_predict(svms[d], node);
	return res;
}

fVec DynamicalSVR::Test( const fVec &sample )
{
	int dim = 2;
	if(!node) node = new svm_node[dim+1];
	FOR(i, dim)
	{
		node[i].index = i+1;
		node[i].value = sample._[i];
	}
	node[dim].index = -1;
	fVec res;
    res[0] = (float)svm_predict(svms[0], node);
    res[1] = (float)svm_predict(svms[1], node);
	return res;
}

void DynamicalSVR::SetParams(int svmType, float svmC, float svmP, u32 kernelType, float kernelParam)
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
