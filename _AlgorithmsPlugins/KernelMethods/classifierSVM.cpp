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
#include <public.h>
#include "classifierSVM.h"
#include <QDebug>

using namespace std;

ClassifierSVM::ClassifierSVM()
    : svm(0), node(0), x_space(0)
{
    bMultiClass = true;
    classCount = 0;
    // default values
    param.svm_type = C_SVC;
    //param.svm_type = EPSILON_SVR;
    param.kernel_type = RBF;
    param.gamma = 0.1;
    param.C = 100;
    param.nu = 0.1;
    param.p = 0.3;

    param.degree = 1;
    param.coef0 = 0;
    param.shrinking = 1;
    param.probability = 0;
    param.eps = 1e-6;
    param.cache_size = 400;
    param.nr_weight = 0;
    param.weight_label = NULL;
    param.weight = NULL;
    param.kernel_weight = NULL;
    param.kernel_dim = 0;
    param.kernel_norm = 1.;
    param.normalizeKernel = false;
}

ClassifierSVM::~ClassifierSVM()
{
    DEL(node);
    DEL(svm);
    DEL(x_space);
}

void ClassifierSVM::SetParams(int svmType, float svmC, u32 kernelType, float kernelParam)
{
    // default values
    param.svm_type = svmType;
    param.C = svmC;
    param.nu = svmC;
    param.eps = 0.01;

    param.coef0 = 0;
    param.gamma = 1;

    switch(kernelType)
    {
    case 0:
        param.kernel_type = LINEAR;
        param.degree = 1;
        break;
    case 1:
        param.kernel_type = POLY;
        param.degree = (u32)kernelParam;
        break;
    case 2:
        param.kernel_type = RBF;
        param.gamma = kernelParam;
        break;
    case 3:
        param.kernel_type = SIGMOID;
        param.gamma = kernelParam;
        break;
    }
}

void ClassifierSVM::Train(std::vector< fvec > samples, ivec labels)
{
    svm_problem problem;

    int data_dimension = samples[0].size();
    problem.l = samples.size();
    problem.y = new double[problem.l];
    problem.x = new svm_node *[problem.l];
    KILL(x_space);
    x_space = new svm_node[(data_dimension+1)*problem.l];

    int cnt=0;
    FOR(i, labels.size()) if(!classMap.count(labels[i])) classMap[labels[i]] = cnt++;
    for(map<int,int>::iterator it=classMap.begin(); it != classMap.end(); it++) inverseMap[it->second] = it->first;
    ivec newLabels(labels.size());
    FOR(i, labels.size()) newLabels[i] = classMap[labels[i]];


    FOR(i, problem.l)
    {
        FOR(j, data_dimension)
        {
            x_space[(data_dimension+1)*i + j].index = j+1;
            x_space[(data_dimension+1)*i + j].value = samples[i][j];
        }
        x_space[(data_dimension+1)*i + data_dimension].index = -1;
        problem.x[i] = &x_space[(data_dimension+1)*i];
        problem.y[i] = newLabels[i];
    }

    DEL(svm);
    DEL(node);
    svm = svm_train(&problem, &param);
    delete [] problem.x;
    delete [] problem.y;

    int maxClass = 0;
    FOR(j, newLabels.size()) maxClass = max(maxClass, newLabels[j]);

    classCount = svm->nr_class;
    //classCount = maxClass;
    FOR(i, classCount)
    {
        classes[i] = svm->label[i];
        //qDebug() << "classes: " << i << classes[i];
    }
    //FOR(j, labels.size()) qDebug() << "label:" << j << labels[j];

}

float ClassifierSVM::Test( const fvec &sample )
{
    int data_dimension = sample.size();
    if(!svm) return 0;
    float estimate;
    if(!node) node = new svm_node[data_dimension+1];
    FOR(i, data_dimension)
    {
        node[i].index = i+1;
        node[i].value = sample[i];
    }
    node[data_dimension].index = -1;
    estimate = (float)svm_predict(svm, node);
    return estimate;
}

float ClassifierSVM::Test( const fVec &sample )
{
    int data_dimension = 2;
    if(!svm) return 0;
    float estimate;
    if(!node)
    {
        node = new svm_node[data_dimension+1];
        node[data_dimension].index = -1;
    }
    FOR(i, data_dimension)
    {
        node[i].index = i+1;
        node[i].value = sample._[i];
    }
    estimate = (float)svm_predict(svm, node);
    return estimate;
}

fvec ClassifierSVM::TestMulti(const fvec &sample)
{
    if(classCount == 2)
    {
        fvec res(1);
        res[0] = Test(sample);
        return res;
    }
    int maxClass = classCount;
    FOR(i, classCount) maxClass = max(maxClass, classes[i]);
    fvec resp(maxClass,0);
    int data_dimension = sample.size();
    if(!svm) return resp;
    if(!node)
    {
        node = new svm_node[data_dimension+1];
        node[data_dimension].index = -1;
    }
    FOR(i, data_dimension)
    {
        node[i].index = i+1;
        node[i].value = sample[i];
    }
    double *decisions = new double[classCount];
    svm_predict_votes(svm, node, decisions);
    //int max = 0;
    FOR(i, classCount)
    {
        resp[classes[i]] = decisions[i];
        //if(resp[max] < resp[classes[i]]) max = i;
    }
    //resp[max] += classCount;
    delete [] decisions;
    return resp;
}

char *ClassifierSVM::GetInfoString()
{
    if(!svm) return NULL;
    char *text = new char[1024];
    sprintf(text, "%s\n", param.svm_type == NU_SVC ? "nu-SVM" : "C-SVM");
    sprintf(text, "%sKernel: ", text);
    switch(param.kernel_type)
    {
    case LINEAR:
        sprintf(text, "%s linear\n", text);
        break;
    case POLY:
        sprintf(text, "%s polynomial (deg: %d bias: %.3f width: %f)\n", text, param.degree, param.coef0, param.gamma);
        break;
    case RBF:
        sprintf(text, "%s rbf (gamma: %f)\n", text, param.gamma);
        break;
    case SIGMOID:
        sprintf(text, "%s sigmoid (%f %f)\n", text, param.gamma, param.coef0);
        break;
    }
    sprintf(text, "%sC: %f \t nu: %f\n", text, param.C, param.nu);
    sprintf(text, "%sSupport Vectors: %d\n", text, svm->l);
    return text;
}
