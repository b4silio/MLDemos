/*********************************************************************
MLDemos: A User-Friendly visualization toolkit for machine learning
Copyright (C) 2010  Basilio Noris
Contact: mldemos@b4silio.com

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public License,
version 3 as published by the Free Software Foundation.

This library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free
Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*********************************************************************/
#ifndef _CLASSIFIER_GP_H_
#define _CLASSIFIER_GP_H_

#include <vector>
#include <map>
#include "classifier.h"
#include "basicMath.h"
#include <mymaths.h>
#include "SECovarianceFunction.h"
#include <QDebug>
#include "newmat11/newmatap.h"
/**
  GP Classifier using the Laplace approximation for the joint posterior over latent function
  */
#ifndef PI
#define PI 3.14159265358979323846
#endif

// this is an upper bound on the allowed number of training points. The code can be easily changed to accomodate data sets of arbitrary size (will do this at some point)
#define MAX_N_TRAIN 10000
// upper bound for allowed dimensionality. Can and should be removed...soon
#define MAX_DIM 100
// the maximum allowed number of iterations for the training phase.
#define MAX_TRAIN_ITERATION 1000


class ClassifierGP : public Classifier
{
private:
    std::map<int,fvec> centers; // used as a simple examples in which we compute the distance to the center of each class
    SECovarianceFunction mSECovFunc; // an object which will be responsible for computing all covariance function evaluations that we will need
    ColumnVector f_mode; // This is the result of the training phase, the estimate of the mode of the joint posterior over the latent function values
    Matrix K; // Covariance matrix of trainning data

    ColumnVector g_logprob_yf; // grad log p(y|f)
    ColumnVector gg_logprob_yf; // grad grad log p(y|f)

    float ConvergenceObjective; // this is a vairable which is used for checking convergence in the trainig phase
    int Niter; // keeps track of the number of iterations in training phase

    // these are some intermediary matrices and vectors that are used to speed up computation.
    DiagonalMatrix W;
    DiagonalMatrix sqrtW;
    Matrix L;
    Matrix Linv;
    Matrix LinvXsqrtW;
    SymmetricMatrix B;


    int Ntrain; // number of tranining points
    int dim; //dimensionality of training data
    std::vector< fvec > training_data;
    float * training_data_raw_array; // an array of floats that contains the training data in 'vector-free' form.

    bool bMonteCarlo; //a bool for determining if the class probability should be integrated numerically (bMonteCarlo = false) or using MC (bMonteCarlo = true)
    int Neval; // an integer determining the number of samples/ integration steps for the evaluation of the class probability

public:

    /**
      Constructor, instanciating everything that will be used
      */
    ClassifierGP(){
        // set up some standard values for the covariance function

        float params[2] = {0.1,0.1}; //lengthscales for the two input dimensions
        mSECovFunc.SetParams(2,params,0.1,1.0);
    }
    /**
      Deconstructor, deinstanciating everything that has been in            interfaceGPRRegress.cpp \
            interfaceGPRDynamic.cpp \stanciated
      */
    ~ClassifierGP(){}

    /**
      The training function, called by the main program, all training should go here
      */
    void Train(std::vector< fvec > samples, ivec labels);

    /**
      Binary classification function, takes a sample, returns a real value, negative for class 0 and positive for class 1
      */
    float Test(const fvec &sample) const ;


    /**
      Information string for the Algorithm Information and Statistics panel in the main program interface.
      Here you probably will put the number of parameters, the training time or anything else
      */
    const char *GetInfoString() const ;

    /**
      Function to set the algorithm hyper-parameters, called prior to the training itself
      */
    void SetParams(double l,int method, int Ns);
};

#endif // _CLASSIFIER_EXAMPLE_H_
