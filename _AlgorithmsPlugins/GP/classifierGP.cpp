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
#include "classifierGP.h"
#include <map>
#include <QDebug>


using namespace std;


// this is a little function to convert training and testing data to 'raw' float arrays.
float * ConvertToRawArray(std::vector< fvec > v){
    int Nsamp = (int)v.size();
    int dim = (int)v[0].size();
    float * ret = new float[Nsamp*dim];
    int Pind = 0;
    int Wind = 0;
    while(Pind<Nsamp){
        ret[Wind] = v[Pind][0];
        Wind++;
        ret[Wind] = v[Pind][1];
        Wind++;
        Pind++;
    }
    return ret;
}

// a little function which copies content from the fvec to a classic array of floats.
// this requires that ret is properly allocated!
void ConvertToRawArray(const fvec &a,float * ret){
    int dim = a.size();
    FOR(i,dim){
        ret[i] = a[i];
    }
}

// this is the sigmoid function. Can be replaced by any other function mapping R -> [0,1]
float LogisticResponseFunction(float z){
    return 1.f/(1.f+expf(-z));
}
// evaluate the unidimensional Gaussian with mean and var at point x.
float gausspdf(float mean, float var,float x){
    return (1.f/sqrtf(2.f*PIf*var))*expf(-0.5f*(x-mean)*(x-mean)/var);
}

//numerical integration of class probabilities, with E(q) = mean, Var(q) = var. Nsteps controls the speed/accuracy tradeoff.
float IntegrateLogisticGaussian(float mean,float var,int Nstep){
    float prob=0;
    //start integrating from 3 stdev (>99% of data)
    float x_up = mean+3*sqrtf(var);
    float x_low = mean-3*sqrtf(var);
    // the integration step length
    float steplen = (x_up - x_low)/((float)Nstep);
    float x = x_low;
    while(x<x_up){
        prob+=(steplen*LogisticResponseFunction(x)*gausspdf(mean,var,x));
        x+=steplen;
    }
    //sanity check, this should normally never happen. But if p >1 then make sure it is still a probability by truncating.
    if(prob>1) prob = 1.0;

    return prob;
}

// evalutate class probability using MC sampling. Speed/accuracy tradeoff controller with Nsamp
float MonteCarloLogisticGaussian(float mean,float var,int Nsamp){
    float sample;
    float sigma = sqrtf(var);
    float prob = 0;
    //Take Nsamp samples from N(mean, var)
    FOR(i,Nsamp){
        sample = RandN(mean,sigma); //get the sample
        prob  += LogisticResponseFunction(sample); //evaluate the sigmoid and sum
    }
    prob /= Nsamp; //compute average
    //Note that is mathematically impossible to get prob>1 here, so no sanity check needed.
    return prob;

}
// this function for printing the contents of a newmat Matrix to qDebug()
void newmatPrint(Matrix v){
    qDebug()<<"printing matrix with size"<<v.size();
    FOR(i,v.size()){
        qDebug()<<v.element(i,1);
    }
}

// this function for printing the contents of a newmat Vector to qDebug()
void newmatPrint(ColumnVector v){
    qDebug()<<"printing vector with size"<<v.size();
    FOR(i,v.size()){
        qDebug()<<v.element(i);
    }
}

void ClassifierGP::Train(std::vector< fvec > samples, ivec labels)
{
    qDebug() <<"GPC started training with "<<samples.size()<<" training points.";
    // we don't want to train anything if we don't have data
    if(!samples.size()) return;

    int mylabels[MAX_N_TRAIN];
    FOR(i,labels.size()){
        if(labels[i]>0)
            mylabels[i] = 1;
        else
            mylabels[i] = -1;
    }
    //Compute Covaraince K
    training_data = samples;
    Ntrain = (int)training_data.size();
    dim = (int)training_data[0].size();
    float Kf[Ntrain*Ntrain]; // float arrray that will hold cov matrix data
    training_data_raw_array = ConvertToRawArray(training_data); //convert to format wanted by mSECovFunc
    mSECovFunc.ComputeCovarianceMatrix(training_data_raw_array,Ntrain,Kf); //compute the NxN matrix to float array Kf.
    K.resize(Ntrain,Ntrain); //resize the newmat covaraince matrix
    K << Kf; //load the cov data into the newmat covariance matrix

    // initialize f mode
    f_mode.resize(Ntrain);
    f_mode = 0.0;
    //initialize g_logprob_yf and gg_logprob_yf
    g_logprob_yf.resize(Ntrain);
    gg_logprob_yf.resize(Ntrain);
    //initialize W
    W.resize(Ntrain);
    sqrtW.resize(Ntrain);
    //initialize B
    B.resize(Ntrain);
    //initialize L
    L.resize(Ntrain,Ntrain);
    Linv.resize(Ntrain,Ntrain);
    //initialize b
    ColumnVector c_b(Ntrain);
    //initialize a
    ColumnVector c_a(Ntrain);
    bool bTrain = true;
    Niter = 0;
    while(bTrain){
        //compute g_logprob_yf and gg_logprob_yf, update W and sqrtW
        float c_pi,c_t;
        for(int i=0;i<Ntrain;i++){
            c_pi = LogisticResponseFunction(f_mode.element(i));
    
            c_t = (float)(mylabels[i]+1)/2;
            g_logprob_yf.element(i) = c_t - c_pi;
    
            gg_logprob_yf.element(i) = c_pi*(c_pi-1);
            W.element(i,i) = -gg_logprob_yf.element(i);
            sqrtW.element(i,i) = sqrt(-gg_logprob_yf.element(i));
        }
        //update B
        B << IdentityMatrix(Ntrain) + sqrtW*K*sqrtW;

        //update L
        L << Cholesky(B);
        Linv = L.i();
        LinvXsqrtW = Linv*sqrtW;
        //update b
        c_b = W*f_mode + g_logprob_yf;
        //newmatPrint(c_b);
        c_a = c_b -  sqrtW*(L.t()).i()*(Linv*(sqrtW*K*c_b));
        //newmatPrint(c_a);

        //update mode of latent posterior
        f_mode = K*c_a;

        Niter++;

        //Check convergence
        float new_ConvObj;
        ColumnVector tmpCO;
        tmpCO = c_a.t()*f_mode;
        new_ConvObj = -tmpCO.as_scalar();
        FOR(i,Ntrain){
            new_ConvObj += log(LogisticResponseFunction(f_mode.element(i)));
        }

        // make the training stop if we have coverged, or if the maximum number of allowed iterations has ben exceeded.
        if(fabs(new_ConvObj - ConvergenceObjective) < 0.000001 || Niter > MAX_TRAIN_ITERATION){
            bTrain = false;
        }
        ConvergenceObjective = new_ConvObj;

    }

    qDebug() <<"GPC finished training in "<<Niter<<" iterations.";

}


float ClassifierGP::Test(const fvec &sample) const
{
    float smp_raw_array[MAX_DIM]; // float array for testing data
    float  k_star_raw_array[MAX_N_TRAIN]; //float array for covariance k(x*,X)

    ConvertToRawArray(sample,smp_raw_array);
    mSECovFunc.ComputeCovarianceVector(training_data_raw_array,Ntrain,smp_raw_array,k_star_raw_array); //compute k(x*,X)

    ColumnVector k_star(Ntrain); // create a newmat vector for k(x*,X)
    k_star<<k_star_raw_array; //load it with the data

    if(k_star.size() != Ntrain)
        qDebug()<<"k_star is wrong size!";

    ColumnVector posterior_mean_v;
    posterior_mean_v = k_star.t()*g_logprob_yf;

    if(posterior_mean_v.size() != 1)
        qDebug()<<"posterior_mean_v is wrong size!";

    ColumnVector v; //intermediary vector needed for computation

    v = LinvXsqrtW*k_star;
    if(v.size() != Ntrain)
        qDebug()<<"v is wrong size!"<<Ntrain<<v.size();

    ColumnVector posterior_var_v(1); //variance of q
    float kss = mSECovFunc.ComputeCovariance(smp_raw_array,smp_raw_array);
    posterior_var_v << kss;
    posterior_var_v = posterior_var_v - (v.t())*v;
    if(posterior_var_v.size() != 1)
        qDebug()<<"posterior_var_v is wrong size!";

    float p_pos; //float for storing p(y*=1|X,y,x*)

    //this is a hack to deal with numerical instabilities that may result in negative variance for the posterior distribution
    if(posterior_var_v.element(0)<FLT_MIN)
        posterior_var_v.element(0) = FLT_MIN;

    //evaluate using either MC or numerical integration
    if(!bMonteCarlo)
        p_pos = IntegrateLogisticGaussian(posterior_mean_v.element(0),posterior_var_v.element(0),Neval);
    else
        p_pos = MonteCarloLogisticGaussian(posterior_mean_v.element(0),posterior_var_v.element(0),Neval);

    if(isnan(p_pos)){
        qDebug()<<"post mean"<<posterior_mean_v.element(0)<<"post var"<<posterior_var_v.element(0);
        qDebug()<<"k**"<< kss<<"v.t()*v"<<(posterior_var_v.element(0)-kss)*(-1);
    }

    float p_neg = 1-p_pos;
    return 3*(p_pos - p_neg);


}

const char *ClassifierGP::GetInfoString() const
{
    char *text = new char[1024];
    sprintf(text, "My Classifier Example\n");
    sprintf(text, "%s\n", text);
    sprintf(text, "%sTraining information:\n", text);

    // here you can fill in whatever information you want
    for(map<int,fvec>::const_iterator it=centers.begin(); it != centers.end(); it++)
    {
        sprintf(text, "%sCenter for class %d\n", text, it->first);
        FOR(d, it->second.size())
        {
            // write down the dimension as floats with 3 decimals
            sprintf(text,"%s %.3f", text, it->second[d]);
        }
        sprintf(text, "%s\n", text);
    }

    return text;
}

void ClassifierGP::SetParams(double l,int method,int Ns){
    float params[2] = {(float)l,(float)l};
    mSECovFunc.SetParams(2,params,0.1,1.0);
    Neval = Ns;
    bMonteCarlo = method;
}
