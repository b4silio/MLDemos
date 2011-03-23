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

#include "GradientDescent.h"

#ifdef USE_MATHLIB_NAMESPACE
using namespace MathLib;
#endif

GradientFunction::GradientFunction(){
}
GradientFunction::~GradientFunction(){
}
float GradientFunction::GetErrorValue(float *params){ 
  return GDESCENT_ERROR;
}



GradientDescent::GradientDescent(){
    mFunc             = NULL;
    mNbParams         = 0;
    mCurrParams       = NULL;
    mDeltaParams      = NULL;
    mMomentumParams   = NULL;
    mParamsStep       = NULL;
    mWorkParams       = NULL;
    mCurrError        = 0;
    mParamsRangeMin   = NULL;
    mParamsRangeMax   = NULL;
    mParamsRate       = NULL;
    mParamsMomentum   = NULL;
}

GradientDescent::~GradientDescent(){
    Clear();
}

void GradientDescent::Init(int nbParams){

    Clear();
    if(nbParams<=0)
        return;
  
    mNbParams           = nbParams;
    mCurrParams         = new float[mNbParams];
    mDeltaParams        = new float[mNbParams];
    mMomentumParams     = new float[mNbParams];
    mWorkParams         = new float[mNbParams];
    mParamsStep         = new float[mNbParams];
    mParamsRangeMin     = new float[mNbParams];
    mParamsRangeMax     = new float[mNbParams];
    mParamsRate         = new float[mNbParams];
    mParamsMomentum     = new float[mNbParams];
    //mErrorValues        = new float[mNbParams*2];
    
    for(int i=0;i<mNbParams;i++){
        mParamsStep[i]      = 0;
        mParamsRangeMin[i]  = 0;
        mParamsRangeMax[i]  = 0;
        mParamsRate[i]      = 0;
        mParamsMomentum[i]  = 0;
    }

    mInDataPtsInverse.Resize(mNbParams*2+1,mNbParams*2+1,false);
    mOutDataPts.Resize(mNbParams*2+1,false);
    mCoefs.Resize(mNbParams*2+1,false);
    mNewInDataPt.Resize(mNbParams*2+1,false);
    mNewOutDataPt = R_ZERO;

    mInDataPtsInverse.Zero();
    mOutDataPts.Zero();
    mCoefs.Zero();
}

#define SAFE_DELETEARRAY(x) if((x)!=NULL) delete [] (x); (x) = NULL;
#define SAFE_DELETE(x)      if((x)!=NULL) delete    (x); (x) = NULL;

void GradientDescent::Clear(){
    SAFE_DELETEARRAY(mCurrParams);
    SAFE_DELETEARRAY(mDeltaParams);
    SAFE_DELETEARRAY(mMomentumParams);
    SAFE_DELETEARRAY(mParamsStep);
    SAFE_DELETEARRAY(mWorkParams);
    SAFE_DELETEARRAY(mParamsRangeMin);
    SAFE_DELETEARRAY(mParamsRangeMax);
    SAFE_DELETEARRAY(mParamsRate);
    SAFE_DELETEARRAY(mParamsMomentum);
    //SAFE_DELETEARRAY(mErrorValues);
    mFunc       = NULL;
    mNbParams   = 0;
    mCurrError  = 0;
    mMethod     = 1;
}

void  GradientDescent::SetFunction(GradientFunction * func){
    mFunc = func;
}
void  GradientDescent::SetSearchStep(float *step){
    for(int i=0;i<mNbParams;i++)
        mParamsStep[i] = step[i];
}
void  GradientDescent::SetSearchRange(float *rangeMin,float *rangeMax){
    for(int i=0;i<mNbParams;i++){
        mParamsRangeMin[i] = rangeMin[i];
        mParamsRangeMax[i] = rangeMax[i];
    }
}

void  GradientDescent::SetLearningRate(float *rate, float *momentum){
    for(int i=0;i<mNbParams;i++){
        mParamsRate[i]      = rate[i];
        mParamsMomentum[i]  = 0;
    }
    if(momentum){
        for(int i=0;i<mNbParams;i++)
            mParamsMomentum[i]  = momentum[i];
    }
}

void  GradientDescent::CheckRange(){
    for(int i=0;i<mNbParams;i++){
        if(mParamsRangeMin[i]<=mParamsRangeMax[i]){
                 if(mCurrParams[i]<mParamsRangeMin[i]) mCurrParams[i] = mParamsRangeMin[i];
            else if(mCurrParams[i]>mParamsRangeMax[i]) mCurrParams[i] = mParamsRangeMax[i];
        }
    }
}

float GradientDescent::Start(float *params){
    if(mFunc==NULL)
        return GDESCENT_ERROR;
    for(int i=0;i<mNbParams;i++){
        mCurrParams[i]      = params[i];
        mDeltaParams[i]     = 0;
        mMomentumParams[i]  = 0;
    }
    mCurrError = mFunc->GetErrorValue(mCurrParams);
    
    if(mMethod==1){
        Matrix A(2*mNbParams+1,2*mNbParams+1,false);
        Vector y(2*mNbParams+1,false);
        for(int i=0;i<mNbParams;i++){
            mWorkParams[i] = mCurrParams[i];
        }
        for(int i=0;i<mNbParams;i++){
            A(2*mNbParams, 2*i  ) = mWorkParams[i]*mWorkParams[i];
            A(2*mNbParams, 2*i+1) = mWorkParams[i];
        }
        A(2*mNbParams, 2*mNbParams) = 1.0;
        y(2*mNbParams) = mCurrError;
        
        for(int i=0;i<mNbParams;i++){
            mWorkParams[i]  += mParamsStep[i];
            for(int j=0;j<mNbParams;j++){
                A(2*i,   2*j  ) = mWorkParams[j]*mWorkParams[j];
                A(2*i,   2*j+1) = mWorkParams[j];
            }
            A(2*i,   2*mNbParams) = 1.0;
            y(2*i  ) = mFunc->GetErrorValue(mWorkParams);
            mWorkParams[i]  -= 2.0*mParamsStep[i];
            for(int j=0;j<mNbParams;j++){
                A(2*i+1, 2*j  ) = mWorkParams[j]*mWorkParams[j];
                A(2*i+1, 2*j+1) = mWorkParams[j];
            }
            A(2*i+1, 2*mNbParams) = 1.0;
            y(2*i+1) = mFunc->GetErrorValue(mWorkParams);
            mWorkParams[i]  += mParamsStep[i];
        }
        //A.Print();
        //y.Print();
        A.TransposeMult(A,mInDataPtsInverse);
        mInDataPtsInverse.SInverseSymmetric();
        if(Matrix::IsInverseOk())
            cout << "inversion ok"<<endl;
        else
            cout << "WARNING: inversion fail"<<endl;
        
        A.Transpose().Mult(y,mOutDataPts);
        mInDataPtsInverse.Mult(mOutDataPts,mCoefs);

        A.Print();
        y.Print();
        //(B*mInDataPtsInverse).Print();
        /*
        mInDataPtsInverse.Print();
        if(Matrix::IsInverseOk())
            cout << "ok"<<endl;
        //mOutDataPts
         */
    }
    
    return mCurrError;
}

float GradientDescent::Step(int cnt){
    if(mFunc==NULL)
        return GDESCENT_ERROR;

    //if(mCurrError == 0.0) return mCurrError;
    for(int j=0;j<cnt;j++){
        for(int i=0;i<mNbParams;i++){
            mWorkParams[i] = mCurrParams[i];
        }
        for(int i=0;i<mNbParams;i++){
            if(mParamsStep[i]==0.0){
                mDeltaParams[i] = 0;
            }else{
                switch(mMethod){
                case 0:
                    mWorkParams[i]  += mParamsStep[i];
                    mDeltaParams[i]  = mFunc->GetErrorValue(mWorkParams);
                    mWorkParams[i]  -= 2.0*mParamsStep[i];
                    mDeltaParams[i] -= mFunc->GetErrorValue(mWorkParams);
                    mDeltaParams[i] /= 2*mParamsStep[i];
                    mWorkParams[i]  += mParamsStep[i];
                    break;
                case 1:
                    
                    break;
                }
            }
        }
        // |  |  |  |  |
        //       x
        switch(mMethod){
        case 0:
            for(int i=0;i<mNbParams;i++){
                mCurrParams[i]     += -mParamsRate[i]*mDeltaParams[i] -mParamsMomentum[i]*mMomentumParams[i];
                mMomentumParams[i]  = mDeltaParams[i];
            }
            break;
        case 1:
            for(int i=0;i<mNbParams;i++){
                mCurrParams[i] = -mCoefs(2*i+1) / (2.0*mCoefs(2*i));
            }
            CheckRange();
            
            mCurrError = mFunc->GetErrorValue(mCurrParams);
            
            for(int i=0;i<mNbParams;i++){
                mNewInDataPt(2*i)   = mCurrParams[i]*mCurrParams[i];
                mNewInDataPt(2*i+1) = mCurrParams[i];
            }
            mNewInDataPt(2*mNbParams) = 1.0;
            
            mOutDataPts += mNewInDataPt * mCurrError;

            Matrix A;
            mNewInDataPt.MultTranspose(mNewInDataPt,A);
            //mNewInDataPt.Print();

            mInDataPtsInverse -= (mInDataPtsInverse * A * mInDataPtsInverse) /
                                 (1.0+ (mInDataPtsInverse * mNewInDataPt).Dot(mNewInDataPt));
            
            //A.Transpose().Mult(y,mOutDataPts);
            mInDataPtsInverse.Mult(mOutDataPts,mCoefs);

            //A.Print();
            
            
            break;
        }
        CheckRange();
    }
    mCurrError = mFunc->GetErrorValue(mCurrParams);
    return mCurrError;
}
