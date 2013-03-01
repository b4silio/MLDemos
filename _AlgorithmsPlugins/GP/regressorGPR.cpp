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
#include "regressorGPR.h"
#include <QDebug>
#include <nlopt/nlopt.hpp>

void RegressorGPR::Train( std::vector<fvec> input, ivec labels)
{
    if(!input.size()) return;
    dim = input[0].size()-1;
    Matrix inputs(dim, input.size());
    RowVector outputs(input.size());
    int oDim = outputDim != -1 ? min(outputDim, dim) : dim;
    FOR(n, input.size())
    {
        FOR(d, dim) inputs(d+1, n+1) = input[n][d];
        outputs(n+1) = input[n][oDim];
        if(outputDim != -1 && outputDim < dim) inputs(outputDim+1, n+1) = input[n][dim];
    }
    
    if(sogp) delete sogp;
    if(kernelType == kerPOL)
    {
        if(!degree) degree = 1;
        RowVector deg(degree);
        for (int i=0; i<degree; i++) deg(i+1) = param1*(1+i*0.3f);
        POLKernel kern(deg);
        SOGPParams params(&kern);
        params.s20=param2;
        params.capacity = capacity;
        sogp = new SOGP(params);
    }
    else
    {
        RowVector widths(dim);
        FOR(d, dim) widths(d+1) = param1;
        RBFKernel kern(widths);
        SOGPParams params(&kern);
        params.s20=param2;
        params.capacity = capacity;
        sogp = new SOGP(params);
    }
    sogp->addM(inputs,outputs);
    double logLikelihood = 0;
    for(int i=1;i<=inputs.Ncols();i++)
    {
        float loglik = sogp->log_prob(inputs.Column(i), outputs.Column(i));
        logLikelihood += loglik;
    }

    if(bOptimize) Optimize(inputs, outputs);
    bTrained = true;
}

double GetLikelihood(const double *x, const Matrix &inputs, const Matrix &outputs, SOGP *sogp, bool bLikelihood)
{
    int dim = inputs.Nrows();
    SOGPParams &oldParams = sogp->getParams();
    int kernelType = oldParams.m_kernel->m_type;

    SOGPParams *params=0;
    if(kernelType == kerRBF)
    {
        RowVector widths(dim);
        FOR(d, dim) widths(d+1) = x[d];
        RBFKernel kern(widths);
        kern.setA(x[dim]);
        params = new SOGPParams(&kern);
        params->s20 = oldParams.s20;
    }
    else if(oldParams.m_kernel->m_type == kerPOL)
    {
        POLKernel kern(x[0]);
        params = new SOGPParams(&kern);
        params->s20 = oldParams.s20;
    }
    params->capacity = oldParams.capacity;
    sogp->setParams(*params);
    sogp->addM(inputs, outputs);
    delete params;

    if(bLikelihood)
    {
        double logLikelihood = 0;
        for(int i=1;i<=inputs.Ncols();i++)
        {
            float loglik = sogp->log_prob(inputs.Column(i), outputs.Column(i));
            logLikelihood += loglik;
        }
        QString list;
        if(kernelType == kerRBF)
        {
            FOR(d, dim) list += QString("%1 ").arg(x[d]);
            list += QString("A: %1").arg(x[dim]);
        }
        else list += QString("%1 ").arg(x[0]);
        qDebug() << "loglik" << logLikelihood << list;
        if(logLikelihood != logLikelihood) return 0; // better than NAN!
        return logLikelihood;
    }
    else
    {
        double mse = 0;
        for(int i=1; i<=inputs.Ncols();i++)
        {
            double confidence;
            Matrix out = sogp->predict(inputs.Column(i),confidence);
            if(out.Ncols())
            {
                double diff = out(1,1) - outputs(1,i);
                mse += diff*diff;
            }
            else mse = FLT_MAX;
        }
        mse /= inputs.Ncols();
        QString list;
        if(kernelType == kerRBF)
        {
            FOR(d, dim) list += QString("%1 ").arg(x[d]);
            list += QString("A: %1").arg(x[dim]);
        }
        else list += QString("%1 ").arg(x[0]);
        qDebug() << "mse" << mse << list;
        return mse;
    }
}

struct OptData
{
    const Matrix *inputs, *outputs;
    SOGP *sogp;
    bool bLikelihood;
};

double objectiveFunction(unsigned n, const double *x, double *gradient /* NULL if not needed */, void *func_data)
{
    OptData *data = (OptData*)func_data;

    double objective = GetLikelihood(x, *data->inputs, *data->outputs, data->sogp, data->bLikelihood);
    if(gradient)
    {
        double *dx = new double[n];
        double delta = 1e-2;
        FOR(i, n)
        {
            memcpy(dx, x, n*sizeof(double));
            dx[i] += delta;
            double dError = GetLikelihood(dx, *data->inputs, *data->outputs, data->sogp, data->bLikelihood);
            gradient[i] = (dError - objective)/delta;
        }
        delete [] dx;
    }

    return objective;
}

void RegressorGPR::Optimize(const Matrix &inputs, const Matrix &outputs)
{
    int dim = inputs.Nrows();

    OptData *data = new OptData;

    data->inputs = &inputs;
    data->outputs = &outputs;
    data->sogp = sogp;
    data->bLikelihood = bOptimizeLikelihood;

    int optDim = dim + 1;
    SOGPParams &oldParams = sogp->getParams();
    switch(oldParams.m_kernel->m_type)
    {
    case kerRBF:
        optDim = dim+1;
        break;
    case kerPOL:
        optDim = 1;
        break;
    }

    //nlopt::opt opt(nlopt::LN_AUGLAG, optDim);
    //nlopt::opt opt(nlopt::LN_COBYLA, optDim);
    //nlopt::opt opt(nlopt::LN_NELDERMEAD, optDim);
    //nlopt::opt opt(nlopt::LN_NEWUOA, optDim);
    //nlopt::opt opt(nlopt::LN_PRAXIS, optDim);
    nlopt::opt opt(nlopt::LN_BOBYQA, optDim);
    //nlopt::opt opt(nlopt::LN_SBPLX, optDim);

    if(bOptimizeLikelihood) opt.set_max_objective(objectiveFunction, (void*)data);
    else opt.set_min_objective(objectiveFunction, (void*)data);
    opt.set_maxeval(200);
    vector<double> lowerBounds(optDim, 0);
    if(bOptimizeLikelihood) lowerBounds.back() = 1e-4; // s20 noise
    opt.set_lower_bounds(lowerBounds);
    vector<double> steps(optDim,0.1);
    opt.set_initial_step(steps);
    opt.set_xtol_abs(0.01);

    vector<double> x(optDim), xOpt;

    switch(oldParams.m_kernel->m_type)
    {
    case kerRBF:
    {
        RowVector widths = ((RBFKernel*)oldParams.m_kernel)->getWidths();
        FOR(d, dim) x[d] = 1./widths(d+1);
        x[dim] = ((RBFKernel*)oldParams.m_kernel)->getA();
    }
        break;
    case kerPOL:
    {
        RowVector scales = ((POLKernel*)oldParams.m_kernel)->getScales();
        x[0] = scales(1);
    }
        break;
    }
    try
    {
        // do the actual optimization
        xOpt = opt.optimize(x);

        // use the best parameters to retrain
        int dim = inputs.Nrows();
        int kernelType = oldParams.m_kernel->m_type;
        SOGPParams *params=0;
        if(kernelType == kerRBF)
        {
            RowVector widths(dim);
            FOR(d, dim) widths(d+1) = xOpt[d];
            RBFKernel kern(widths);
            kern.setA(xOpt[dim]);
            params = new SOGPParams(&kern);
            params->s20 = oldParams.s20;
        }
        else if(oldParams.m_kernel->m_type == kerPOL)
        {
            POLKernel kern(xOpt[0]);
            params = new SOGPParams(&kern);
            params->s20 = oldParams.s20;
        }
        params->capacity = oldParams.capacity;
        sogp->setParams(*params);
        sogp->addM(inputs, outputs);
    }
    catch(std::exception e)
    {
        qDebug() << "caught exception while optimizing";
    }
    delete data;
}


fvec RegressorGPR::Test( const fvec &sample )
{
    fvec res;
    res.resize(2,0);
    if(!sogp) return res;
    double confidence;
    Matrix _testout;
    int dim = sogp->dim();
    ColumnVector _testin(dim);
    FOR(i,dim)
    {
        _testin(1+i) = sample[i];
    }
    if(outputDim != -1 && outputDim < dim) _testin(1+outputDim) = sample[dim];
    _testout = sogp->predict(_testin,confidence);
    if(_testout.Ncols()) res[0] = _testout(1,1);
    res[1] = confidence*confidence;
    return res;
}

fVec RegressorGPR::Test( const fVec &sample )
{
    fVec res;
    if(!sogp) return res;
    double confidence;
    Matrix _testout;
    ColumnVector _testin(2);
    FOR(i,2)
    {
        _testin(1+i) = sample._[i];
    }
    _testout = sogp->predict(_testin,confidence);
    if(_testout.Ncols()) res[0] = _testout(1,1);
    res[1] = confidence*confidence;
    return res;
}

float RegressorGPR::GetLikelihood(float mean, float sigma, float point)
{
    const float sqrpi = 1.f/sqrtf(2.f*PIf);
    const float divider = sqrpi/sigma;
    const float exponent = -powf((point-mean)/sigma,2.f)*0.5;
    return expf(exponent)*divider;
}

void RegressorGPR::Clear()
{
    bTrained = false;
    delete sogp;
    sogp = 0;
}

int RegressorGPR::GetBasisCount()
{
    return sogp ? sogp->size() : 0;
}

fvec RegressorGPR::GetBasisVector( int index )
{
    if(!sogp) return fvec();
    if(index > sogp->size()) return fvec();
    int dim = sogp->dim();
    fvec res(dim*2,0);
    FOR(d, dim)
    {
        res[d] = sogp->BVloc(index, d);
        res[dim + d] = sogp->alpha_acc(index, d);
    }
    return res;
}

const char *RegressorGPR::GetInfoString()
{
    char *text = new char[2048];
    sprintf(text, "Sparse Optimized Gaussian Processes\n");
    sprintf(text, "%sKernel: ", text);
    switch(kernelType)
    {
    case 0:
        sprintf(text, "%s rbf (gamma: %f)\n", text, param1);
        break;
    case 1:
        sprintf(text, "%s polynomial (deg: %f %f width: %f)\n", text, degree, param1);
        break;
    case 2:
        sprintf(text, "%s rbf (gamma: %f)\n", text, param1);
        break;
    }
    sprintf(text, "%sNoise: %.3f\n", text, param2);
    sprintf(text, "%sBasis Functions: %d\n", text, GetBasisCount());
    return text;
}
