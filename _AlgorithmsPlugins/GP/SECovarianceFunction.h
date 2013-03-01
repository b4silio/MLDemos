
#ifndef _SECOVARIANCEFUNCTION_H_
#define _SECOVARIANCEFUNCTION_H_

#include <vector>
#include <map>
#include "basicMath.h"
#include <mymaths.h>

// the data type that should be used.
#define SECOVFT float
/**
A simple class for computing the squared exponential covariance function. The idea is that this should be very portable (no dependencies) and hopefully
reasonably fast.
  */


class SECovarianceFunction
{
private:
    //SECOVFT * inData;
    int dim; //dimensionality of data
    SECOVFT lengthscales[]; // float array containing the lengthscale for each dimension
    //SECOVFT * outData;
    SECOVFT sigma_n; // noise var
    SECOVFT sigma_f; //signal var


public:  
    SECovarianceFunction(){}
     ~SECovarianceFunction(){}
    /**
    Set the params of the Squared Exponential Covariance Function: dimension of data, lengthscales, noise variance and a scaling constant sf.
      */
    void SetParams(int d, SECOVFT l[], SECOVFT sn, SECOVFT sf);

    /**
    Compute the covaraince between vector a and b, which must both be of size dim.
     */
    SECOVFT ComputeCovariance(const SECOVFT * a, const SECOVFT * b) const ;
    /**
    Compute the covaraince between vectors in a and vector b. The array a must have the shape [N][dim], where N is the number of vectors.
    The array b must have shape [dim]. res is a pointer to the resulting array, which has shape [N].
     */
    SECOVFT * ComputeCovarianceVector(const SECOVFT * a, const int N, const SECOVFT * b) const ;
    void ComputeCovarianceVector(const SECOVFT * a, const int N, const SECOVFT * b, SECOVFT * res) const ;
    /**
    Compute the covaraince matrix between vectors in a. The array a must have the shape [N][dim], where N is the number of vectors and dim is the dimenstionality
    previously defined when setting up the covaraince function. The resulting array has shape [N][N] and is pointed by res.
     */
    SECOVFT * ComputeCovarianceMatrix(const SECOVFT *a , const int N) const ;
    void ComputeCovarianceMatrix(const SECOVFT *a, const int N,SECOVFT *res) const ;

};

#endif
