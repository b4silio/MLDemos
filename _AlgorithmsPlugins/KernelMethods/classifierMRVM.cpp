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
#include "classifierMRVM.h"
#include <nlopt/nlopt.hpp>
#include <QDebug>
#include <iostream>
#include <fstream>
//#include "sse_mathfun.h"
#include <QElapsedTimer>

using namespace std;

ClassifierMRVM::ClassifierMRVM()
    : svm(0), node(0), x_space(0)
{
    dim = 2;
    bMultiClass = false;
    bInvert=false;
    classCount = 0;
    // default values
    param.svm_type = C_SVC;
    //param.svm_type = EPSILON_SVR;
    param.kernel_type = RBF;
    param.gamma = 0.1;
    param.C = 100;
    param.nu = 0.1;
    param.p = 0.3;
    bOptimize = false;

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

ClassifierMRVM::~ClassifierMRVM()
{
    DEL(node);
    DEL(svm);
    DEL(x_space);
}

void ClassifierMRVM::SetParams(int svmType, float svmC, u32 kernelType, float kernelParam)
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

double kernelFunctionMRVM(svm_model *svm, int d)
{
    // 1 - 0.5 * sum_i sum_j (a_i*a_j*y_i*y_j*k(x_i,x_j)*(x_i-x_j)*(x_i-x_j)) / C
    double accumulator = 0;
    int nsv = svm->l;
    int nclass = svm->nr_class-1;
    for(int c=0; c<nclass; c++)
    {
        double *sv_coef = svm->sv_coef[c];
        double sum = 0;
        for(int i=0; i<nsv; i++)
        {
            for(int j=0; j<=i; j++)
            {
                double diff = svm->SV[i][d].value-svm->SV[j][d].value;
                double value = - sv_coef[i] * sv_coef[j] * Kernel::k_function(svm->SV[i],svm->SV[j],svm->param);
                value *= diff * diff;
                sum += j==i ? value : 2*value;
            }
        }
        accumulator += 0.5*sum;
    }
    return 1 - accumulator/svm->param.C;
}

void ClassifierMRVM::OptimizeGradient(svm_problem *problem)
{
    if( param.kernel_type == LINEAR || param.kernel_type == SIGMOID || param.kernel_type == RBFWMATRIX) return;
    qDebug() << "optimizing kernel weights";

    double gamma = param.gamma;
    double C = param.C;
    if( param.kernel_type == RBF)
    {
        param.kernel_type = RBFWEIGH;
        param.kernel_dim = dim;
        param.kernel_weight = new double[dim];
        FOR(d, dim) param.kernel_weight[d] = param.gamma;
        param.gamma = 1.;
        param.C = 1.;
        svm->param = param;
    }
    delete svm;
    svm = svm_train(problem, &param);

    QString s;
    /*
    int count = problem->l;
    FOR(d, dim)
    {
        float mean = 0, sigma = 0;
        FOR(i, count)
        {
            qDebug() << "d" << d << "i" << i << problem->x[i][d].index;
            mean += problem->x[i][d].value;
            count++;
        }
        mean /= count;
        FOR(i, count)
        {
            float diff = problem->x[i][d].value - mean;
            sigma += diff*diff;
        }
        sigma /= count;
        sigma = sqrtf(sigma);
        param.kernel_weight[d] = sigma*pow(4./3.,0.2)*pow(count,-0.2);
        s += QString("%1 ").arg(param.kernel_weight[d]);
    }
    */

    // we begin by computing the kernel dimension
    dvec gammas(dim, gamma), newGammas(dim, gamma);
    dvec deltas(dim, 0);

    double firstObj = svm_get_dual_objective_function(svm);
    double oldObj = 0;
    qDebug() << "Initial objective function" << oldObj << "gamma" << s;
    double gammaStep = 1;
    int iterations = 30;
    bool bAllZeros = true;
    int restarts = 0;
    FOR(it, iterations)
    {
        //QString s1, s2;
        double norm=0;
        FOR(d, dim)
        {
            double delta = kernelFunctionMRVM(svm, d);
            deltas[d] = delta;
            //s1 += QString("%1 ").arg(delta);
            norm += deltas[d]*deltas[d];
        }
        norm = sqrt(norm);
        // find the step size
        //gammaStep = 2;
        double obj;

        // armijo's rule
        double beta = 0.5; // the decay factor for the step
        double scale = 0.5; // the scaling for the acceptance of a step
        int gammaIter = 10;
        FOR(i, gammaIter)
        {
            QString s, s1, ds;
            FOR(d, dim)
            {
                //if(fabs(deltas[d]) <= 1e-4) continue;
                newGammas[d] = gammas[d] - deltas[d]*gammaStep;
                newGammas[d] = max(0.,newGammas[d]);
                s += QString("%1 ").arg(1./newGammas[d],0,'f',3);
                ds += QString("%1 ").arg(deltas[d],0,'f',3);
                s1 += QString("%1 ").arg(newGammas[d],0,'f',3);
            }
            FOR(d, dim) param.kernel_weight[d] = newGammas[d];
            DEL(svm);
            svm = svm_train(problem, &param);
            obj = svm_get_dual_objective_function(svm);
            qDebug() << "it" << it << i << "obj" << obj << "(" << oldObj << ")" << "gamma step" << gammaStep << "delta" << ds << "gamma" << s1 << "(" << s << ")";
            if(obj < oldObj)
            //if(obj - oldObj <= scale * gammaStep * norm)
            {
                break;
            }
            gammaStep *= beta;
        }
        bAllZeros = true;
        if(obj < oldObj)
        {
            FOR(d, dim)
            {
                gammas[d] = newGammas[d];
                if(gammas[d] != 0) bAllZeros = false;
            }
        }
        if(obj > oldObj)
        {
            DEL(svm);
            FOR(d, dim)
            {
                param.kernel_weight[d] = gammas[d];
            }
            svm = svm_train(problem, &param);
            break;
        }

        if(bAllZeros)
        {
            FOR(d, dim)
            {
                gammas[d] = gamma;
                param.kernel_weight[d] = gammas[d];
            }
            param.C = param.C*2;
            it = 0;
            oldObj = firstObj;
            restarts++;
            if(restarts > 10) break;
            qDebug() << "starting again!" << "C" << param.C << "gamma" << gammas[0];
        }
        else
        {
            if(fabs(oldObj-obj) < 1e-5) break;
            oldObj = obj;
        }
    }
    s = "";
    FOR(d, dim)
    {
        s += QString("%1 ").arg(1. / gammas[d]);
    }
    qDebug() << "gamma" << s;

    param.C = C;
    DEL(svm);
    svm = svm_train(problem, &param);
}

void ClassifierMRVM::Train(std::vector< fvec > samples, ivec labels)
{
    svm_problem problem;

    dim = samples[0].size();
    problem.l = samples.size();
    problem.y = new double[problem.l];
    problem.x = new svm_node *[problem.l];
    KILL(x_space);
    x_space = new svm_node[(dim +1)*problem.l];

    classes.clear();
    classMap.clear();
    inverseMap.clear();
    int cnt=0;
    FOR(i, labels.size()) if(!classMap.count(labels[i])) classMap[labels[i]] = cnt++;
    bool bBinary = classMap.size() == 2;
    if(bBinary)
    {
        int positive = INT_MIN;
        int negative;
        FOR(i, labels.size()) positive = max(positive, labels[i]);
        FORIT(classMap, int, int)
        {
            if(it->first != positive)
            {
                negative = it->first;
                break;
            }
        }
        classMap[negative] = -1;
    }
    FORIT(classMap, int, int) inverseMap[it->second] = it->first;
    ivec newLabels(labels.size());
    FOR(i, labels.size()) newLabels[i] = classMap[labels[i]];

    bInvert = !(!classMap.count(1) || classMap.at(1) != 1);

    FOR(i, problem.l)
    {
        FOR(j, dim )
        {
            x_space[(dim +1)*i + j].index = j+1;
            x_space[(dim +1)*i + j].value = samples[i][j];
        }
        x_space[(dim +1)*i + dim ].index = -1;
        problem.x[i] = &x_space[(dim +1)*i];
        problem.y[i] = newLabels[i];
    }

    delete(svm);
    DEL(node);
    svm = svm_train(&problem, &param);

    //if(bOptimize) OptimizeGradient(&problem);

    delete [] problem.x;
    delete [] problem.y;

    int maxClass = 0;
    FOR(j, newLabels.size()) maxClass = max(maxClass, newLabels[j]);

    classCount = svm->nr_class;
    //classCount = maxClass;
    FOR(i, classCount)
    {
        classes[i] = svm->label[i];
    }

    // now we create our own svm model with the betas
    mrvm.dim = dim;
    mrvm.SetSVs(svm);

    OptimizeRandomWalk(samples, newLabels);
}

float GetScore(MRVM *mrvm, vector<fvec> &samples, ivec &labels, bool bInvert)
{
    double dual = mrvm->Dual();
    double error = 0;
    double *s = new double[mrvm->dim];
    FOR(i, samples.size())
    {
        FOR(d, mrvm->dim)
        {
            s[d] = (double)samples[i][d];
        }
        double estimate = mrvm->Test(s);
        if(bInvert) estimate *= -1;
        int label = labels[i] == -1 ? -1 : 1;
        if(estimate * label < 0)
        {
            //error += fabs(estimate);
            error += 1.;
        }
    }
    error /= samples.size();

    KILL(s);
    return (1. - error);
    //return (1. - error) + dual;
}

void ClassifierMRVM::OptimizeRandomWalk(vector<fvec> samples, ivec labels)
{
    int iterations = 5000;
    double step = 0.05;
    double alphaStep = 0.1*svm->param.C;
    int count = mrvm.svCount;

    double *betas = new double[count];
    double *alphas = new double[count];
    FOR(i, count) betas[i] = 1.f;
    memcpy(alphas, mrvm.alphas, count*sizeof(double));

    qDebug() << "optimizing betas";
    float bestScore = -FLT_MAX;
    float initScore = 0;
    int posCount = 0;
    for(int i=1; i<count; i++)
    {
        qDebug() << i << mrvm.alphas[i-1] << mrvm.alphas[i];
        if(mrvm.alphas[i-1]*mrvm.alphas[i] < 0)
        {
            posCount = i;
            break;
        }
    }

    FOR(it, iterations)
    {
        memcpy(mrvm.betas, betas, count*sizeof(double));
        memcpy(mrvm.alphas, alphas, count*sizeof(double));
        if(it)
        {
            int i = rand()%posCount;
            int j = rand()%(count-posCount) + posCount;
            //FOR(i, count)
            {
                float newBeta = mrvm.betas[i] += (drand48()-0.5)*step;
                if(newBeta < 0.01) newBeta = 0.01;
                mrvm.betas[i] = newBeta;
                newBeta = mrvm.betas[j] += (drand48()-0.5)*step;
                if(newBeta < 0.01) newBeta = 0.01;
                mrvm.betas[j] = newBeta;

                double alphaDiff = (drand48()-0.5)*alphaStep;
                double sign = mrvm.alphas[i] < 0 ? -1 : 1;
                mrvm.alphas[i] += alphaDiff;
                mrvm.alphas[i] = sign*max(0., min(svm->param.C, fabs(mrvm.alphas[i])));
                sign = mrvm.alphas[j] < 0 ? -1 : 1;
                mrvm.alphas[j] -= alphaDiff;
                mrvm.alphas[j] = sign*max(0., min(svm->param.C, fabs(mrvm.alphas[j])));
            }
        }
        float score = GetScore(&mrvm, samples, labels, bInvert);
        if(!it) initScore = score;
        if(score > bestScore)
        {
            memcpy(betas, mrvm.betas, count*sizeof(double));
            memcpy(alphas, mrvm.alphas, count*sizeof(double));
            bestScore = score;
        }
        double minB=FLT_MAX, maxB=-FLT_MAX;
        FOR(i, count)
        {
            minB = min(minB, betas[i]);
            maxB = max(maxB, betas[i]);
        }
        qDebug() << "init" << initScore <<  "best" << bestScore << "current" << score << "--> " << minB << maxB;
    }
    memcpy(mrvm.betas, betas, count*sizeof(double));
    QString text;
    FOR(i, count)
    {
        text += QString("%1 ").arg(betas[i]);
    }
    qDebug() << "betas" << text;
    delete [] betas;
    delete [] alphas;
}

MRVM::~MRVM()
{
    clear();
}

void MRVM::clear()
{
    KILL(sv);
    KILL(alphas);
    KILL(betas);
    svCount = 0;
}

void MRVM::SetSVs(svm_model *svm)
{
    clear();
    svCount = svm->l;
    sv = new double[svm->l*dim];
    alphas = new double[svm->l];
    betas = new double[svm->l];
    bias = svm->rho[0];
    FOR(i, svm->l)
    {
        FOR(d, dim) sv[i*dim + d] = svm->SV[i][d].value;
        alphas[i] = svm->sv_coef[0][i];
        betas[i] = 1.0;
    }
    gamma = svm->param.gamma;
}

void MRVM::SetSVs(const int svCount, const double *sv, const double *alphas, const double bias, const double *betas)
{
    if(this->svCount != svCount)
    {
        clear();
        this->sv = new double[svCount*dim];
        this->alphas = new double[svCount];
        this->betas = new double[svCount];
    }
    this->svCount = svCount;
    this->bias = bias;
    memcpy(this->sv, sv, svCount*dim*sizeof(double));
    memcpy(this->alphas, alphas, svCount*sizeof(double));
    memcpy(this->betas, betas, svCount*sizeof(double));
}

double MRVM::Kernel(const double *sv, const double *x, const double beta)const
{
    double k=0;
    FOR(d, dim)
    {
        double dist = sv[d] - x[d];
        k += dist * dist;
    }
    return exp(-k*gamma*beta);
}

double MRVM::Test(const double *sample) const
{
    double k = 0;
    FOR(i, svCount)
    {
        k += alphas[i] * Kernel(sv + i*dim, sample, betas[i]);
    }
    k -= bias;
    return k;
}

double MRVM::Dual() const
{
    double k = 0;
    FOR(i, svCount)
    {
        FOR(j, svCount)
        {
            k += alphas[i]*alphas[j] * Kernel(sv + i*dim, sv + j*dim, betas[i]);
        }
    }
    k = -0.5*k;
    FOR(i, svCount) k += alphas[i];
    return k;
}

float ClassifierMRVM::Test( const fvec &sample ) const
{
    if(!svm) return 0;
    double *s = new double[dim];
    FOR(d, dim)
    {
        s[d] = (double)sample[d];
    }
    double estimate = mrvm.Test(s);
    KILL(s);
    if(bInvert) estimate *= -1;
    return estimate;
}

const char *ClassifierMRVM::GetInfoString() const
{
    if(!svm) return NULL;
    char *text = new char[1024];
    char text2[1024];
    sprintf(text, "%s\n", param.svm_type == NU_SVC ? "nu-SVM" : "C-SVM");
    sprintf(text2, "%sKernel: ", text);
    switch(param.kernel_type)
    {
    case LINEAR:
        sprintf(text, "%s linear\n", text2);
        break;
    case POLY:
        sprintf(text, "%s polynomial (deg: %d bias: %.3f width: %f)\n", text2, param.degree, param.coef0, param.gamma);
        break;
    case RBF:
    case RBFWEIGH:
        sprintf(text, "%s rbf (gamma: %f)\n", text2, param.gamma);
        break;
    case SIGMOID:
        sprintf(text, "%s sigmoid (%f %f)\n", text2, param.gamma, param.coef0);
        break;
    default:
        strcpy(text, text2);
        break;
    }
    sprintf(text2, "%sC: %f \t nu: %f\n", text, param.C, param.nu);
    sprintf(text, "%sSupport Vectors: %d\n", text2, svm->l);
    return text;
}

void ClassifierMRVM::SaveModel(std::string filename) const
{
    std::cout << "saving SVM model";
    if(!svm)
    {
        std::cout << "Error: Nothing to save!" << std::endl;
        return; // nothing to save!
    }

    // Save the dataset to a file
    std::ofstream file(filename.c_str());

    if(!file){
        std::cout << "Error: Could not open the file!" << std::endl;
        return;
    }

    file << dim << " " << classCount << endl;
    file << param.svm_type << " " << param.kernel_type << endl;
    file << (param.kernel_weight==0 ? 0 : param.kernel_dim) << " ";
    if(param.kernel_weight)
    {
        FOR(i, param.kernel_dim) file << param.kernel_weight[i] << " ";
        file << endl;
    }
    file << param.eps << " " << param.C << " " << param.nu << " " << param.p << endl;
    file << (param.weight_label==0 ? 0 : param.nr_weight);
    FOR(i, param.nr_weight) file << " " << param.weight_label[i];
    FOR(i, param.nr_weight) file << " " << param.weight[i];
    file << endl;
    file << param.normalizeKernel << " " << param.kernel_norm << " " << param.cache_size << " " << param.shrinking << " " << param.probability << endl;

    if(param.kernel_type == POLY) file << param.degree << " ";
    if(param.kernel_type == POLY || param.kernel_type == RBF || param.kernel_type == SIGMOID) file << param.gamma << " ";
    if(param.kernel_type == POLY || param.kernel_type == SIGMOID) file << param.coef0;
    file << endl;

    file << svm->nr_class << " " << svm->l << endl;
    FOR(i, svm->nr_class*(svm->nr_class-1)/2) file << svm->rho[i] << " ";
    file << endl;

    file << (svm->label!=0) << " " << (svm->probA!=0) << " " << (svm->probB!=0) << " " << (svm->nSV!=0) << endl;
    if(svm->label)
    {
        FOR(i, svm->nr_class) file << svm->label[i] << " ";
        file << endl;
    }
    if(svm->probA)
    {
        FOR(i, svm->nr_class*(svm->nr_class-1)/2) file << svm->probA[i] << " ";
        file << endl;
    }
    if(svm->probB)
    {
        FOR(i, svm->nr_class*(svm->nr_class-1)/2) file << svm->probB[i] << " ";
        file << endl;
    }
    if(svm->nSV)
    {
        FOR(i, svm->nr_class) file << svm->nSV[i] << " ";
        file << endl;
    }

    file << (param.kernel_type == PRECOMPUTED) << endl;
    FOR(i, svm->l)
    {
        FOR(j, svm->nr_class-1) file << svm->sv_coef[j][i] << " ";

        const svm_node *p = svm->SV[i];

        if(param.kernel_type == PRECOMPUTED) file << (int)(p->value) << " ";
        else
        {
            while(p->index != -1)
            {
                file << p->index << " " << p->value << " ";
                p++;
            }
        }
        file << endl;
    }

    file << type << " " << bOptimize << endl;

    for(map<int,int>::const_iterator it=inverseMap.begin(); it != inverseMap.end(); it++)
    {
        file << it->first << " " << it->second << " ";
    }
    file << endl;
    for(map<int,int>::const_iterator it=classMap.begin(); it != classMap.end(); it++)
    {
        file << it->first << " " << it->second << " ";
    }
    file << endl;

    file.close();
}

//#define Malloc(type,n) (type *)malloc((n)*sizeof(type))
#define Malloc(type,n) new type[n]
bool ClassifierMRVM::LoadModel(std::string filename)
{
    std::cout << "Loading SVM model" << std::endl;
    if(svm) DEL(svm);
    if(node) DEL(node);
    if(x_space) DEL(x_space);

    std::ifstream file(filename.c_str());
    if(!file.is_open()){
        std::cout << "Error: Could not open the file!" << std::endl;
        return false;
    }

    file >> dim >> classCount;

    svm = new svm_model;
    file >> param.svm_type >> param.kernel_type;
    file >> param.kernel_dim;
    if(param.kernel_dim)
    {
        param.kernel_weight= Malloc(double, param.kernel_dim);
        FOR(i, param.kernel_dim) file >> param.kernel_weight[i];
    }
    file >> param.eps >> param.C >> param.nu >> param.p;
    file >> param.nr_weight;
    if(param.nr_weight)
    {
        param.weight_label = Malloc(int, param.nr_weight);
        param.weight = Malloc(double, param.nr_weight);
        FOR(i, param.nr_weight) file >> param.weight_label[i];
        FOR(i, param.nr_weight) file >> param.weight[i];
    }
    file >> param.normalizeKernel >> param.kernel_norm >> param.cache_size >> param.shrinking >> param.probability;

    if(param.kernel_type == POLY) file >> param.degree;
    if(param.kernel_type == POLY || param.kernel_type == RBF || param.kernel_type == SIGMOID) file >> param.gamma;
    if(param.kernel_type == POLY || param.kernel_type == SIGMOID) file >> param.coef0;

    file >> svm->nr_class >> svm->l;
    svm->rho = Malloc(double, svm->nr_class*(svm->nr_class-1)/2);
    FOR(i, svm->nr_class*(svm->nr_class-1)/2) file >> svm->rho[i];

    int label, probA, probB, nSV;
    file >> label >> probA >> probB >> nSV;
    if(label)
    {
        svm->label = Malloc(int, svm->nr_class);
        FOR(i, svm->nr_class) file >> svm->label[i];
    }
    if(probA)
    {
        svm->probA = Malloc(double, svm->nr_class*(svm->nr_class-1)/2);
        FOR(i, svm->nr_class*(svm->nr_class-1)/2) file >> svm->probA[i];
    }
    if(probB)
    {
        svm->probB = Malloc(double, svm->nr_class*(svm->nr_class-1)/2);
        FOR(i, svm->nr_class*(svm->nr_class-1)/2) file >> svm->probB[i];
    }
    if(nSV)
    {
        svm->nSV = Malloc(int, svm->nr_class);
        FOR(i, svm->nr_class) file >> svm->nSV[i];
    }

    bool bPrecomputed;
    file >> bPrecomputed;

    svm->sv_coef = Malloc(double *, svm->nr_class-1);
    svm->SV = Malloc(svm_node*, svm->l);
    FOR(i, svm->nr_class-1) svm->sv_coef[i] = Malloc(double, svm->l);
    ivec indices;
    dvec values;
    int index = 0;
    double value = 0;
    FOR(i, svm->l)
    {
        FOR(j, svm->nr_class-1) file >> svm->sv_coef[j][i];

        if(bPrecomputed)
        {
            param.kernel_type = PRECOMPUTED;
            file >> value;
        }
        else
        {
            svm->SV[i] = Malloc(svm_node, dim + 1);
            FOR(j, dim)
            {
                file >> svm->SV[i][j].index;
                file >> svm->SV[i][j].value;
            }
            svm->SV[i][dim].index = -1;
            svm->SV[i][dim].value = 0;
        }
    }

    file >> type >> bOptimize;
    inverseMap.clear();
    classMap.clear();
    FOR(i, classCount)
    {
        int first, second;
        file >> first >> second;
        inverseMap[first] = second;
    }
    FOR(i, classCount)
    {
        int first, second;
        file >> first >> second;
        classMap[first] = second;
    }

    classCount = svm->nr_class;
    FOR(i, classCount)
    {
        classes[i] = svm->label[i];
    }

    file.close();
    svm->param = param;

    return true;
}
