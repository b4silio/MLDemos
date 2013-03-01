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
#include "public.h"
#include "basicMath.h"
#include "classifierTrees.h"
#include <QDebug>
#include <QLabel>
#include <QPixmap>
#include <QPainter>

using namespace std;
using namespace cv;

ClassifierTrees::ClassifierTrees()
{
    negativeClass = 0;
    maxClass = 2;
    bComputeImportance = true;
    minSampleCount = 2;

    // define the parameters for training the random forest (trees)
    bBalanceClasses = true;
    minSampleCount = 1;
    maxDepth = 25;
    maxTrees = 100;
    accuracyTolerance = 0.001f;

    tree = 0;

    bSingleClass = false;
    bMultiClass = true;

    treePainter = 0;
    treeDepth = 1;
    treeCount = 1;
}

ClassifierTrees::~ClassifierTrees()
{
    DEL(tree);
}

void ClassifierTrees::Train( std::vector< fvec > samples, ivec labels )
{
	u32 sampleCnt = samples.size();
    if(!sampleCnt) return;

    classes.clear();
    classMap.clear();
    inverseMap.clear();

    int cnt=0;
    FOR(i, labels.size()) if(!classMap.count(labels[i])) classMap[labels[i]] = cnt++;
    for(map<int,int>::iterator it=classMap.begin(); it != classMap.end(); it++) inverseMap[it->second] = it->first;
    ivec newLabels(labels.size());
    FOR(i, labels.size()) newLabels[i] = classMap[labels[i]];
    labels = newLabels;
//    for(map<int,int>::iterator it=inverseMap.begin(); it != inverseMap.end(); it++) qDebug() << "inverse: " << it->first << it->second;
//    for(map<int,int>::iterator it=classMap.begin(); it != classMap.end(); it++) qDebug() << "class: " << it->first << it->second;

    int classCount = classMap.size();
    if(classMap.count(-1)) negativeClass = classMap[-1];
    else negativeClass = 0;
    maxClass = classMap.size();
    for(std::map<int,int>::iterator it=inverseMap.begin(); it!=inverseMap.end(); it++)
    {
        maxClass = max(maxClass, it->second);
    }

    dim = samples[0].size();
    this->samples = samples;
    this->labels = labels;

    int trainCount = samples.size();

    // creating training data array
    float *trainingData = new float[trainCount*dim];
    //FOR(i, trainCount) trainingData[i] = new float[dim];
    float *trainingLabels = new float[trainCount];

    int *classCounts = new int[classCount];
    FOR(c, classCount) classCounts[c] = 0;
    FOR(i, samples.size())
    {
        FOR(d, dim)
        {
            trainingData[i*dim + d] = samples[i][d];
        }
        trainingLabels[i] = labels[i];
        classCounts[labels[i]]++;
    }

    // Creating Mat data structures out of the arrays
    Mat trainingData_ = Mat(trainCount, dim, CV_32FC1, trainingData);
    Mat trainingLabels_ = Mat(trainCount, 1, CV_32FC1, trainingLabels);

    // This is a classification problem (i.e. predict a discrete number of classes),
    // So we define all the attributes as numerical and the class as categorical.
    Mat var_type = Mat(dim + 1, 1, CV_8UC1 );
    var_type.setTo(Scalar(CV_VAR_NUMERICAL) ); // all inputs are numerical
    var_type.at<uchar>(dim, 0) = CV_VAR_CATEGORICAL;

    //printf( "AI learning : creating tree.\n" ); fflush(stdout);


    // array of a priori class probabilities. can be used to tune the decision tree preferences toward a certain class.
    float *priors = new float[classCount];
    FOR(c, classCount) priors[c] = bBalanceClasses ? classCounts[c] : 1.f / labels.size();
    //qDebug() << "classCounts" << classCounts[0] << classCounts[1];

    CvRTParams params = CvRTParams(maxDepth, //25,      // max depth (is that used ?)
                                   minSampleCount, //4, // min sample count
                                   0,                   // regression accuracy: N/A here
                                   bComputeImportance,  //false, // compute surrogate split, no missing data
                                   classCount, //15,    // max number of categories (use sub-optimal algorithm for larger numbers)
                                   priors,              // the array of priors
                                   bComputeImportance,  //false, // calculate variable importance
                                   0,                   // number of variables randomly selected at node and used to find the best split(s).
                                   maxTrees, //100,     // max number of trees in the forest
                                   accuracyTolerance,   // forrest accuracy
                                   CV_TERMCRIT_ITER | CV_TERMCRIT_EPS // termination criteria : max nb trees OR accuracy = CV_TERMCRIT_ITER |	CV_TERMCRIT_EPS
                                   );
    //printf( "AI learning : Forest parameters: %i max trees, %i max depth, %i nb of variables for node split.\n", maxTrees, maxDepth, minSampleCount); fflush(stdout);

    // train random forest classifier (using training data)
    DEL(tree);
    tree = new CvRTrees;
    tree->train(trainingData_, CV_ROW_SAMPLE, trainingLabels_, Mat(), Mat(), var_type, Mat(), params);

    //printf( "AI learning : %i trees created.\n", tree->get_tree_count() ); fflush(stdout);

    if (bComputeImportance)
    {
        Mat varImportance = tree->getVarImportance();
        printf( "Random Forest - Variable importance : [ ");
        FOR(i, varImportance.cols)
        {
            printf("%f ", varImportance.at<float>(i));
        }
        printf("]\n");fflush(stdout);
    }

    treeCount = tree->get_tree_count();
    treeDepth = 0;
    FOR(i, tree->get_tree_count())
    {
        treeDepth = max(treeDepth, GetTreeDepth(tree->get_tree(i)->get_root()));
    }
    treePixmap = QPixmap(min(100*(treeCount+1), 1024), 200 + (treeDepth > 5 ? (treeDepth-5)*20 : 0));
    treePixmap.fill(Qt::white);
    DEL(treePainter);
    treePainter = new QPainter(&treePixmap);
    treePainter->setRenderHint(QPainter::Antialiasing);
    QFont font = treePainter->font();
    font.setPointSize(9);
    font.setWeight(QFont::Bold);
    treePainter->setFont(font);
    FOR(i, tree->get_tree_count())
    {
        CvForestTree *myTree = tree->get_tree(i);
        PrintTree(myTree, i);
    }
}

int ClassifierTrees::GetTreeDepth(const CvDTreeNode *node) const
{
    if(!node) return -1;
    if(!node->left && !node->right) return node->depth;
    int left = node->left ? GetTreeDepth(node->left) : node->depth;
    int right = node->right ? GetTreeDepth(node->right) : node->depth;
    return max(right, left);
}

void ClassifierTrees::PrintNode(const CvDTreeNode *node, int rootX, bool bLeft) const
{
    if(node == NULL)
    {
        return;
    }
    int depth = node->depth+1;
    int y = depth * treePixmap.height() / (treeDepth+2);
    int deltaY = treePixmap.height()/(treeDepth+2);
    int W = treePixmap.width() / treeCount;
    int w = W/(depth*2);
    int shift = w/(depth+1);
    int x = rootX;
    int radius = 5;
    const CvDTreeNode *nodeL = node->left;
    const CvDTreeNode *nodeR = node->right;
    int classId = inverseMap.at(node->class_idx);

    treePainter->setPen(QPen(Qt::black,2));
    treePainter->setBrush(SampleColor[classId%SampleColorCnt]);
    if(nodeL)
    {
        treePainter->drawLine(x, y, x - shift, y+deltaY);
        treePainter->setBrush(Qt::black);
    }
    if(nodeR)
    {
        treePainter->drawLine(x, y, x + shift, y+deltaY);
        treePainter->setBrush(Qt::black);
    }
    treePainter->drawEllipse(x-radius, y-radius, radius*2, radius*2);
    if(node->split)
    {
        treePainter->drawText(x+6, y, QString("[%1]").arg(node->split->var_idx+1));
    }
    else
    {
        treePainter->drawText(x-2, y+16, QString("%1").arg(classId));
    }
    PrintNode(nodeL, x-shift, true);
    PrintNode(nodeR, x+shift, false);
}

void ClassifierTrees::PrintTree(CvForestTree *tree, int count=0) const
{
    int W = treePixmap.width() / treeCount;
    int rootX = W*(count + 0.5f);
    const CvDTreeNode *root = tree->get_root();
    PrintNode(root, rootX);
    fflush(stdout);
}

fvec ClassifierTrees::GetImportance() const
{
    Mat varImportance = tree->getVarImportance();
    fvec importance(varImportance.cols);
    FOR(i, varImportance.cols)
    {
        importance[i] = varImportance.at<float>(i);
    }
    return importance;
}

fvec ClassifierTrees::TestMulti(const fvec &sample) const
{
    float c = Test(sample);
    if(classMap.size() == 2)
    {
        float value = (c-0.5)*3;
        fvec res(1,value);
        return res;
    }
    fvec res(maxClass, 0);
    res[c] = 1.0f;
    return res;
}

float ClassifierTrees::Test( const fvec &sample) const
{
    if (tree == NULL){
        printf( "Classification error: no classifier learned. \n" ); fflush(stdout);
        return 0.0;
    }
    float *sample_ = new float[dim];
    FOR(d, dim) sample_[d] = sample[d];
    Mat test_sample = Mat(1, dim, CV_32FC1, sample_);
    // predict_prob() returns a fuzzy-predicted class label for binary classification problems.
    float res = 0;
    if(classMap.size() == 2) res = tree->predict_prob(test_sample);
    else res = tree->predict(test_sample);
    return res;
}

void ClassifierTrees::SetParams(bool bBalanceClasses,
                                int minSampleCount, int maxDepth, int maxTrees,
                                float accuracyTolerance)
{
    this->bBalanceClasses = bBalanceClasses;
    this->minSampleCount = minSampleCount;
    this->maxDepth = maxDepth;
    this->maxTrees = maxTrees;
    this->accuracyTolerance = accuracyTolerance;
}

const char *ClassifierTrees::GetInfoString() const
{
	char *text = new char[1024];
    sprintf(text, "Decision Trees\n");
	return text;
}
