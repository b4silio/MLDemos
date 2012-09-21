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
#include <nlopt/nlopt.hpp>
#include <QDebug>

using namespace std;

const char *RegressorSVR::GetInfoString()
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

struct OptData
{
    svm_model *svm;
    svm_problem *problem;
};

double getSVRObjectiveFunction(const svm_model *svm, const double *x, const svm_problem *problem)
{
    svm_parameter param = svm->param;
    switch(param.kernel_type)
    {
    case LINEAR:
        return 0.;
        break;
    case POLY:
        param.degree = x[0];
        param.gamma = 1. / x[1];
        param.coef0 = x[2];
        break;
    case RBF:
        param.gamma = 1. / x[0];
        break;
    case SIGMOID:
        param.coef0 = x[0];
        break;
    case RBFWEIGH:
    {
        param.gamma = 1. / x[0];
        FOR(i, param.kernel_dim)
        {
            param.kernel_weight[i] = x[i+1];
        }
    }
        break;
    }
    svm_model *newSVM = svm_train(problem, &param);
    double value = svm_get_dual_objective_function(newSVM);
    qDebug() << "value:" << value << "gamma:" << 1. / param.gamma;
    delete newSVM;
    return value;
}

double svrObjectiveFunction(unsigned n, const double *x, double *gradient /* NULL if not needed */, void *func_data)
{
    OptData *data = (OptData*)func_data;

    double objective = getSVRObjectiveFunction(data->svm, x, data->problem);
    if(gradient)
    {
        double *dx = new double[n];
        double delta = 1e-2;
        FOR(i, n)
        {
            memcpy(dx, x, n*sizeof(double));
            dx[i] += delta;
            double dError = getSVRObjectiveFunction(data->svm, dx, data->problem);
            gradient[i] = (dError - objective)/delta;
        }
        delete [] dx;
    }

    return objective;
}

void RegressorSVR::Optimize(svm_problem *problem)
{
    OptData *data = new OptData;
    data->svm = svm;
    data->problem = problem;

    int optDim = 1;
    switch(svm->param.kernel_type)
    {
    case POLY:
        optDim = 3;
        break;
    case RBF:
        optDim = 1;
        break;
    case RBFWEIGH:
        optDim = dim + 1;
        break;
    }

    //nlopt::opt opt(nlopt::LN_AUGLAG, optDim);
    nlopt::opt opt(nlopt::LN_COBYLA, optDim);
    //nlopt::opt opt(nlopt::LN_NELDERMEAD, optDim);
    //nlopt::opt opt(nlopt::LN_NEWUOA, optDim);
    //nlopt::opt opt(nlopt::LN_PRAXIS, optDim);
    //nlopt::opt opt(nlopt::LN_BOBYQA, optDim);
    //nlopt::opt opt(nlopt::LN_SBPLX, optDim);

    opt.set_max_objective(svrObjectiveFunction, (void*)data);

    opt.set_maxeval(100);
    vector<double> lowerBounds(optDim, 0.001);
    opt.set_xtol_abs(0.001);

    vector<double> x(optDim), xOpt;

    vector<double> steps(optDim,0.1);
    switch(svm->param.kernel_type)
    {
    case POLY:
        x[0] = svm->param.degree;
        x[1] = 1. / svm->param.gamma;
        x[2] = svm->param.coef0;
        steps[0] = 1;
        lowerBounds[0] = 1;
        break;
    case RBF:
        x[0] = 1. / svm->param.gamma;
        break;
    case SIGMOID:
        x[0] = svm->param.coef0;
        break;
    case RBFWEIGH:
    {
        x[0] = 1. / svm->param.gamma;
        FOR(i, svm->param.kernel_dim)
        {
            x[i+1] = svm->param.kernel_weight[i];
        }
    }
        break;
    }
    opt.set_initial_step(steps);
    opt.set_lower_bounds(lowerBounds);

    try
    {
        // do the actual optimization
        xOpt = opt.optimize(x);
        param = svm->param;
        switch(param.kernel_type)
        {
        case POLY:
            param.degree = xOpt[0];
            param.gamma = 1. / xOpt[1];
            param.coef0 = xOpt[2];
            break;
        case RBF:
            param.gamma = 1. / xOpt[0];
            break;
        case SIGMOID:
            param.coef0 = xOpt[0];
            break;
        case RBFWEIGH:
        {
            param.gamma = 1. / xOpt[0];
            FOR(i, param.kernel_dim)
            {
                param.kernel_weight[i] = xOpt[i+1];
            }
        }
            break;
        }
        delete svm;
        svm = svm_train(problem, &param);
    }
    catch(std::exception e)
    {
        qDebug() << "caught exception while optimizing";
    }
    delete data;
}

void RegressorSVR::Train(std::vector< fvec > samples, ivec labels)
{
    svm_problem problem;
    svm_node *x_space;

    dim = samples[0].size()-1;
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
    if(bOptimize) Optimize(&problem);

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
