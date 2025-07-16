#ifndef CLASSIFIERRSVM_H
#define CLASSIFIERRSVM_H

#include <vector>
#include <map>
#include <classifier.h>
#include "liblinear/linear.h"
#include "randomKernelUtils.h"


typedef struct rand_kernel_parms_s
{
    int eRandFeatureType;
    int eRandKernelType;
    int nFeatureDim;
    float fGamma;
} rand_kernel_parms_t, *rand_kernel_parms_pt;


class ClassifierRSVM : public Classifier
{
private:
    model           *svm;

    int classCount;
    int type;

    //for RBF fourier kernel
    std::vector<fvec>   W;
    fvec                b;

public:
    parameter            param;
    rand_kernel_parms_t  kernelParms;

    ClassifierRSVM();
    ~ClassifierRSVM();
    void Train(std::vector< fvec > samples, ivec lables);
    float Test(const fvec &sample) const ;
    float Test(const fVec &sample) const ;
    fvec TestMulti(const fvec &sample) const ;
    const char *GetInfoString() const ;
    void SetParams(int eRandKernelType, float svmC, int kernelDim, float fGamma);
    model *GetModel(){return svm;}
    void SaveModel(std::string filename) const ;
    bool LoadModel(std::string filename);
};
#endif // CLASSIFIERRSVM_H
