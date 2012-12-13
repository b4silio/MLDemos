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
#ifndef _CLASSIFIER_TREES_H_
#define _CLASSIFIER_TREES_H_

#include <vector>
#include "classifier.h"
#include "basicOpenCV.h"
#include <QPixmap>
#include <QPainter>

class ClassifierTrees : public Classifier
{
private:
    bool bBalanceClasses;
    int minSampleCount;
    bool bComputeImportance;
    int maxDepth;
    int maxTrees;
    float accuracyTolerance;

    CvRTrees *tree;

    int negativeClass;
    int maxClass;

public:
    std::vector<fvec> samples;
    ivec labels;

    QPixmap treePixmap;
    QPainter *treePainter;
    int treeDepth;
    int treeCount;

public:
    ClassifierTrees();
    ~ClassifierTrees();
	void Train(std::vector< fvec > samples, ivec labels);
    float Test(const fvec &sample);
    fvec TestMulti(const fvec &sample);
    const char *GetInfoString();
    fvec GetImportance();
    void PrintTree(CvForestTree *tree, int count);
    void PrintNode(const CvDTreeNode *node, int rootX=0, bool bLeft=true);
    int GetTreeDepth(const CvDTreeNode *node);
    void SetParams(bool bBalanceClasses,
                   int minSampleCount, int maxDepth, int maxTrees,
                   float accuracyTolerance);
};

#endif // _CLASSIFIER_BOOST_H_
