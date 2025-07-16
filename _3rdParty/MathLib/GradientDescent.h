/*
 * Copyright (C) 2010 Learning Algorithms and Systems Laboratory, EPFL, Switzerland
 * Author: Eric Sauser
 * email:   eric.sauser@a3.epf.ch
 * website: lasa.epfl.ch
 *
 * Permission is granted to copy, distribute, and/or modify this program
 * under the terms of the GNU General Public License, version 2 or any
 * later version published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
 */

#ifndef __GDCLASS_H__
#define __GDCLASS_H__

#include <stdlib.h>

#define GDESCENT_ERROR 1e10
#include "MathLib.h"

#ifdef USE_MATHLIB_NAMESPACE
namespace MathLib {
#endif

class GradientFunction
{
public:
            GradientFunction();
    virtual ~GradientFunction();
public:
    virtual float GetErrorValue(float *params);
};


class GradientDescent
{
public:
    GradientFunction   *mFunc;
    int                 mNbParams;
    
    float              *mCurrParams;
    float              *mDeltaParams;
    float              *mMomentumParams;
    float              *mParamsStep;
    float              *mWorkParams;
    float              *mParamsRangeMin;
    float              *mParamsRangeMax;
    float              *mParamsRate;
    float              *mParamsMomentum;
    //float              *mErrorValues;
  
  
    float               mCurrError;

    
    int                 mMethod;

    Matrix              mInDataPtsInverse;
    Vector              mOutDataPts;
    Vector              mNewInDataPt;
    REALTYPE            mNewOutDataPt;
    Vector              mCoefs;
        
public:
    GradientDescent();
    ~GradientDescent();

    void    Init(int nbParams);
    void    Clear();

    void    SetFunction(GradientFunction * func);
    
    void    SetSearchStep(float *step);
    void    SetSearchRange(float *rangeMin,float *rangeMax);

    void    SetLearningRate(float *rate, float *momentum = NULL);

    
    float   Start(float *params);
    float   Step(int cnt = 1);

protected:
    void  CheckRange();
};

#ifdef USE_MATHLIB_NAMESPACE
}
#endif
#endif
