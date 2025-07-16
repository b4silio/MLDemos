#ifndef REGRESSORRGPR_H
#define REGRESSORRGPR_H

#include <vector>
#include <regressor.h>
#include "basicMath.h"
#include <mymaths.h>
#include <newmat11/newmat.h>
#include <randomKernelUtils.h>

struct lr_model
{
    Matrix covariance;
    Matrix inverseA;
    float noise;
    ColumnVector Xy;
    ColumnVector W;
};

class RegressorRGPR : public Regressor
{
private:
    int dim;
    bool bTrained;
    std::vector<fVec> bv;
    int eRandType;
    int nRandRank;
    double param1, param2;
    int kernelType;

    std::vector<fvec>   randFourierW;
    fvec                randFourierb;

public:
    lr_model* _model;
    bool bShowBasis;
    RegressorRGPR() : _model(0), dim(1), kernelType(RAND_KERNEL_RBF), bTrained(false), param1(1), param2(0.1), bShowBasis(false){type = REGR_GPR;}
    void Train(std::vector<fvec> inputs, ivec labels);
    fvec Test(const fvec &sample);
    fVec Test(const fVec &sample);
    const char *GetInfoString();

    void SetParams(int rt, int rr, int kType, double p1, double p2){eRandType = rt; nRandRank = rr;param1=p1; param2=p2; kernelType=kType;}
    lr_model *GetModel(){return _model;}
    void Clear();
    fvec GetBasisVector(int index);
    int GetBasisCount();
    float GetLikelihood(float mean, float sigma, float point);
};

#endif // REGRESSORRGPR_H
