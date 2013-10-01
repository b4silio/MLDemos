#include <iostream>
#include <public.h>
#include "regressorRGPR.h"
#include <QDebug>
#include <nlopt/nlopt.hpp>

using namespace std;

void RegressorRGPR::Train( std::vector<fvec> input, ivec labels)
{
    //cout << "Entered Train function" << endl;
    //extract input X and Y
    std::vector<fvec> inputX;
    fvec inputY;
    if(!input.size()) return;
    dim = input[0].size() - 1;

    int oDim = outputDim != -1 ? min(outputDim, dim) : dim;
    FOR(n, input.size())
    {
        fvec tmpInputX;
        //first fill dim dimensions
        FOR(d, dim) tmpInputX.push_back(input[n][d]);
        //fill output dimension
        inputY.push_back(input[n][oDim]);
        //if output dimension is actually not the last one, re-write corresponding input dimension with last dimension...
        if(outputDim != -1 && outputDim < dim) tmpInputX[outputDim] = input[n][dim];
        inputX.push_back(tmpInputX);
    }
    //cout << "Filled Matrix & Vector" << endl;
    //map input to feature space
    std::vector<fvec> mappedX;
    //cout << "RandType " << eRandType << "; Kernel Type" << kernelType << endl;
    switch(eRandType)
    {
    case RANDOM_FOURIER:
        RandFourierFactorize(kernelType, nRandRank,
                             param1,
                             inputX,
                             mappedX,
                             randFourierW,
                             randFourierb);
       break;
    default:
        return;
    }
    //cout << "Converted from original space to feature space" << endl;
    //construct matrix X & column vector y
    //cout << "mapped X: " << mappedX.size()<< endl;
    int mappedDim = mappedX[0].size();
    //cout << "mapped X dim: " << mappedDim << endl;
    //cout << "inputY dim:" << inputY.size() << endl;
    //cout << "input dim:" << input.size() << endl;
    Matrix matX(mappedDim, input.size());
    ColumnVector colVecY(input.size());
    FOR(j, input.size())
    {
        FOR(i, mappedDim)
        {
            matX(i+1, j+1) = mappedX[j][i];
        }
        colVecY(j+1) = inputY[j];
    }
    //cout << "Try to delete model" << endl;
    if(_model)
    {
        delete _model;
        _model = NULL;
    }
    //cout << "deleted model" << endl;
    _model = new lr_model();
    TransposedMatrix matX_trans = matX.t();
    //cout << "MatX:" << " row:" << matX.nrows() << " col:" << matX.ncols() << endl;
    _model->covariance = matX * matX_trans;
    //cout << "Got covariance matrix" << endl;
    _model->Xy = matX * colVecY;
    //cout << "Got vector Xy" << endl;
    IdentityMatrix omega(mappedDim);
    Matrix matA = _model->covariance + omega * param2;
    //cout << "Got matrix A" << endl;
    _model->inverseA = matA.i();
    //cout << "Got inverse of A" << endl;
    _model->W = _model->inverseA * _model->Xy;
    //cout << "Finish calculate model parameters" << endl;
}

double GetLikelihood(const double *x, const Matrix &inputs, const Matrix &outputs, lr_model *_model, bool bLikelihood)
{
    /*
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
    */
    return 0.0;
}

struct OptData
{
    /*
    const Matrix *inputs, *outputs;
    SOGP *sogp;
    bool bLikelihood;
    */
};

double objectiveFunction(unsigned n, const double *x, double *gradient /* NULL if not needed */, void *func_data)
{
    double objective = 0.0;
    /*
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
    */

    return objective;
}



fvec RegressorRGPR::Test( const fvec &sample )
{
    fvec res;
    res.resize(2);
    if(!_model) return res;

    //extract input fields
    int dim = sample.size() -1;
    fvec inputX(dim);
    FOR(i, inputX.size())
    {
        inputX[i] = sample[i];
    }
    if(outputDim != -1 && outputDim < dim) inputX[outputDim] = sample[dim];

    fvec mapped_sample;
    switch(eRandType)
    {
    case RANDOM_FOURIER:
        RandFourierMap(kernelType, inputX, randFourierW, randFourierb, mapped_sample);
                break;
    default:
        return res;
    };
    //cout << "Finish mapping sample" << endl;

    //calculate estimation
    //(RowVector * ColumnVector).AsScalar() seems not work...
    ColumnVector mapped_x_star(mapped_sample.size());
    FOR(i, mapped_sample.size())
    {
        mapped_x_star(i+1) = mapped_sample[i];
    }

    res[0] = (mapped_x_star.t() * _model->W).AsScalar();
    res[1] = (mapped_x_star.t() * _model->inverseA * mapped_x_star).AsScalar();
    //printf("estimation: %f, variance: %f\n", res[0], res[1]);
    return res;
}

fVec RegressorRGPR::Test( const fVec &sample )
{

    fVec res;
    /*
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
    */
    return res;
}

float RegressorRGPR::GetLikelihood(float mean, float sigma, float point)
{
    const float sqrpi = 1.f/sqrtf(2.f*PIf);
    const float divider = sqrpi/sigma;
    const float exponent = -powf((point-mean)/sigma,2.f)*0.5;
    return expf(exponent)*divider;
}

void RegressorRGPR::Clear()
{
    /*
    bTrained = false;
    delete sogp;
    sogp = 0;
    */
}

int RegressorRGPR::GetBasisCount()
{
    //return sogp ? sogp->size() : 0;
    return 0;
}

fvec RegressorRGPR::GetBasisVector( int index )
{
    fvec res;
    /*
    if(!sogp) return fvec();
    if(index > sogp->size()) return fvec();
    int dim = sogp->dim();
    fvec res(dim*2,0);
    FOR(d, dim)
    {
        res[d] = sogp->BVloc(index, d);
        res[dim + d] = sogp->alpha_acc(index, d);
    }
    */
    return res;
}

const char *RegressorRGPR::GetInfoString()
{
    char *text = new char[2048];
    switch(eRandType)
    {
    case RANDOM_FOURIER:
        sprintf(text, "%d rank %s", nRandRank, "Fourier");
    }

    sprintf(text, "Random %s Gaussian Processes\n", text);
    sprintf(text, "%sKernel: ", text);
    switch(kernelType)
    {
    case RAND_KERNEL_RBF:
        sprintf(text, "%s rbf (gamma: %f)\n", text, param1);
        break;
    }
    sprintf(text, "%sNoise: %.3f\n", text, param2);
    return text;
}
