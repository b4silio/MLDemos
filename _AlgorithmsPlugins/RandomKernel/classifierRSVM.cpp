#include <nlopt/nlopt.hpp>
#include <QDebug>
#include <iostream>
#include <fstream>
#include "classifierRSVM.h"
#include <QTime>

using namespace std;

ClassifierRSVM::ClassifierRSVM()
    :svm(0)
{
    //cout << "initializing classifier RSVM" << endl;
    dim = 2;
    bMultiClass = true;
    classCount = 0;

    //default solver type for L2 regularized and L2 loss primal problem for Support Vector Classification
    param.solver_type = L2R_L2LOSS_SVC;
    param.eps = 0.0001;
    param.C = 1.0;
    param.nr_weight = 0;
    param.weight_label = NULL;
    param.weight = NULL;
    svm = NULL;
}

ClassifierRSVM::~ClassifierRSVM()
{
    DEL(svm);
}

void ClassifierRSVM::SetParams(int eRandKernelType, float svmC, int kernelRank, float kernelWidth)
{
    param.C = svmC;
    kernelParms.eRandKernelType = eRandKernelType;
    kernelParms.nFeatureDim = kernelRank;
    kernelParms.fGamma = kernelWidth;
    return;
}

void ClassifierRSVM::Train(std::vector< fvec > samples, ivec labels)
{
    QTime startTime(QTime::currentTime());
    //map the samples to feature space
    std::vector<fvec>   G;

    switch(kernelParms.eRandFeatureType)
    {
    case RANDOM_FOURIER:
        RandFourierFactorize(kernelParms.eRandKernelType,
                                 kernelParms.nFeatureDim,
                                 kernelParms.fGamma,
                                 samples,
                                 G,
                                 W,
                                 b);
        break;
    default:
        return;
    }

    //construct primal optimization problem with linear model
    problem prob;
    prob.l = G.size();
    prob.n = G[0].size();
    prob.y = new int[prob.l];
    prob.x = new feature_node* [prob.l];
    prob.bias = 1.0;
    FOR(i, prob.l)
    {
        prob.x[i] = NULL;
    }

    int feat_dim = G[0].size();

    //map labels, copied from standard svm code
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

    FOR(i, prob.l)
    {
        //warning: need an last element with index of -1 to indicate the end of the list
        prob.x[i] = new feature_node[feat_dim + 1];
        FOR(j, feat_dim)
        {
            prob.x[i][j].index = j + 1;
            prob.x[i][j].value = G[i][j];
        }
        prob.y[i] = newLabels[i];
        //cout << newLabels[i] << " ";
        prob.x[i][feat_dim].index = -1;
    }
    cout << endl;
    //use liblinear to solve the problem
    delete svm;
    svm = train(&prob, &param);
    //cout << "Label[0]:" << svm->label[0] << "Label[1]:" << svm->label[1] << endl;
    FOR(i, prob.l)
    {
        delete[] prob.x[i];
    }

    delete[] prob.x;
    delete[] prob.y;

    int maxClass = 0;
    FOR(j, newLabels.size()) maxClass = max(maxClass, newLabels[j]);

    classCount = svm->nr_class;
    //classCount = maxClass;
    FOR(i, classCount)
    {
        classes[i] = svm->label[i];
    }
    QTime endTime(QTime::currentTime());
    printf("Training time cost: %d millisecs\n", startTime.msecsTo(endTime));
    return;
}

float ClassifierRSVM::Test( const fvec &sample ) const
{
    //QTime startTime(QTime::currentTime());
    if(!svm) return 0;
    int data_dimension = sample.size();
    //evaluate given sample, first need to convert it to feature space
    if(W.size() != data_dimension)
    {
        cout << "Inconsistent size of Omega for dimension of sample" << endl;
        return 0;
    }
    fvec mapped_sample;
    switch(kernelParms.eRandFeatureType)
    {
    case RANDOM_FOURIER:
        RandFourierMap(kernelParms.eRandKernelType, sample, W, b, mapped_sample);
        break;
    default:
        return 0;
    }
    int feat_dimension = W[0].size();
    feature_node* node = Malloc(feature_node, feat_dimension + 1);

    FOR(i, feat_dimension)
    {
        node[i].index = i+1;
        node[i].value = mapped_sample[i];
    }
    node[feat_dimension].index = -1;
    float estimate = svm_predict(svm, node);
    free(node);
    //printf("input fvec, return float\n");
    //printf("estimate: %f\n", estimate);
    // if we have a binary class in which the negative class is not the first
    if(svm->label[0] == -1) estimate *= -1;
    //QTime endTime(QTime::currentTime());
    //printf("Test time cost: %d millisecs\n", startTime.msecsTo(endTime));
    return estimate;
}

float ClassifierRSVM::Test( const fVec &sample ) const
{
    int data_dimension = 2;
    if(!svm) return 0;
    float estimate;
    //map to feature space
    fvec original_sample;
    FOR(j, data_dimension)
    {
        original_sample.push_back( sample._[j] );
    }
    fvec mapped_sample;
    switch(kernelParms.eRandFeatureType)
    {
    case RANDOM_FOURIER:
        RandFourierMap(kernelParms.eRandKernelType, sample, W, b, mapped_sample);
        break;
    default:
        return 0;
    }
    //fill the node
    int feat_dimension = W[0].size();
    feature_node *node = Malloc(feature_node, feat_dimension+1);
    node[feat_dimension].index = -1;

    FOR(i, feat_dimension)
    {
        node[i].index = i+1;
        node[i].value = mapped_sample[i];
    }
    estimate = (float)predict(svm, node);
    free(node);
    // if we have a binary class in which the negative class is not the first
    if(svm->label[0] == -1) estimate *= -1;
    //printf("input fVec, return float\n");
    //printf("estimate: %f\n", estimate);
    return estimate;
}

fvec ClassifierRSVM::TestMulti(const fvec &sample) const
{
    if(classCount == 2)
    {
        fvec res(1);
        res[0] = Test(sample);
        return res;
    }
    int maxClass = classCount;
    FOR(i, classCount) maxClass = max(maxClass, classes.at(i));
    fvec resp(maxClass,0);

    if(!svm) return resp;

    //mapped to feature space
    fvec mapped_sample;
    switch(kernelParms.eRandFeatureType)
    {
    case RANDOM_FOURIER:
        RandFourierMap(kernelParms.eRandKernelType, sample, W, b, mapped_sample);
        break;
    default:
        fvec res(1);
        return res;
    }

    int feat_dimension = W[0].size();
    feature_node *node = Malloc(feature_node, feat_dimension+1);
    node[feat_dimension].index = -1;

    FOR(i, feat_dimension)
    {
        node[i].index = i+1;
        node[i].value = mapped_sample[i];
    }
    //printf("class count: %d\n", classCount);
    double *decisions = Malloc(double, classCount);
    //printf("Before predict values\n");
    svm_predict_values(svm, node, decisions);
    //printf("After predict values\n");
    //int max = 0;
    FOR(i, classCount)
    {
        resp[classes.at(i)] = decisions[i];
        //if(resp[max] < resp[classes[i]]) max = i;
    }
    //resp[max] += classCount;

    free(decisions);
    //seems we could not free node... but why?
    free(node);

    return resp;
}

const char *ClassifierRSVM::GetInfoString() const
{
    char* text = new char[1024];
    snprintf(text, 1024, "%s", "test info string\n");
    return text;
}

void ClassifierRSVM::SaveModel(std::string filename) const
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

    return;
}

bool ClassifierRSVM::LoadModel(std::string filename)
{
    std::cout << "Loading RSVM model" << std::endl;
    if(svm) DEL(svm);

    return true;
}


