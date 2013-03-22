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
#ifndef _CLASSIFIER_MRVM_H_
#define _CLASSIFIER_MRVM_H_

#include <vector>
#include <map>
#include <classifier.h>
#include "svm.h"

class MRVM
{
public:
    double *sv;
    double *alphas;
    double *betas;
    double bias;
    int svCount;
    int dim;

    double gamma;

    MRVM(int dim=2, double gamma=0.1) : dim(dim), gamma(gamma), sv(0), alphas(0), betas(0), svCount(0){}
    ~MRVM();
    void clear();
    void SetSVs(svm_model *svm);
    void SetSVs(const int svCount, const double *sv, const double *alphas, const double bias, const double *betas=NULL);
    double Kernel(const double *sv, const double *x, const double beta=1.f) const;
    double Test(const double *sample) const ;
    double Dual() const;
};

class ClassifierMRVM : public Classifier
{
private:
	svm_model *svm;
	svm_node *node;
	svm_node *x_space;
	int classCount;
    int type;
public:
    MRVM mrvm;
    bool bInvert;
    svm_parameter param;
    bool bOptimize;

    ClassifierMRVM();
    ~ClassifierMRVM();
	void Train(std::vector< fvec > samples, ivec labels);
    void OptimizeGradient(svm_problem *problem);
    void OptimizeRandomWalk(std::vector<fvec> samples, ivec labels);
    float Test(const fvec &sample) const ;
    const char *GetInfoString() const ;
	void SetParams(int svmType, float svmC, u32 kernelType, float kernelParam);
    svm_model *GetModel(){return svm;}
    void SaveModel(std::string filename) const ;
    bool LoadModel(std::string filename);
};

#endif // _CLASSIFIER_MRVM_H_
