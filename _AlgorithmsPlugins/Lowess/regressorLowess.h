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
#ifndef _REGRESSOR_LOWESS_H_
#define _REGRESSOR_LOWESS_H_

#include <vector>
#include "regressor.h"
#include "gsl/multifit/gsl_multifit.h"
#include "gsl/matrix/gsl_matrix.h"

enum lowessWeightFunc
{
    kLowessWeightFunc_Tricube = 0,
    kLowessWeightFunc_Hann,
    kLowessWeightFunc_Uniform
};
enum lowessFitType
{
    kLowessFitType_Linear = 0,
    kLowessFitType_Quadratic
};
enum lowessNormType
{
    kLowessNormType_None = 0,
    kLowessNormType_StDev,
    kLowessNormType_IQR
};

class RegressorLowess : public Regressor
{
public:
    RegressorLowess();
    ~RegressorLowess();
    void Train(std::vector< fvec > trainSamples, ivec labels);
    fvec Test(const fvec &sample);
    const char *GetInfoString();

    void SetParams(double param1, lowessWeightFunc param2, lowessFitType param3, lowessNormType param4);
    bool Ready() { return !(tooFewPoints || zeroSpread); }
    void StoreLastRadius();
    fvec const &GetRadiusVec() { return radiusVec; }

private:
    //Model parameters
    double           smoothingFac;
    lowessWeightFunc weightingFunc;
    lowessFitType    fitType;
    lowessNormType   normType;
    bool             zeroSpread;
    bool             tooFewPoints;

    //Internal variables
    fvec means;
    fvec stdevs;
    fvec iqrs;
    int numNearestNeighbors;
    int numFitParams;
    int numCrossProds;
    float radius;
    fvec radiusVec;

    //GSL work arrays
    gsl_multifit_linear_workspace *workspace;
    gsl_matrix *X;
    gsl_vector *y;
    gsl_matrix *cov;
    gsl_vector *weights;
    gsl_vector *c;
    gsl_vector *x;

    void  calcDistances(const fvec &sample, fvec &distances);
    float calcWeighting(float distance, float radius, float minWeight);
    void  showErrorMsg_zeroSpread  ();
    void  showErrorMsg_tooFewPoints();
};

#endif // _REGRESSOR_LOWESS_H_
