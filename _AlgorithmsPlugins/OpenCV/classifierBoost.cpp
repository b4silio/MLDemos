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
#include "classifierBoost.h"
#include <QDebug>

using namespace std;
using namespace cv;
using namespace cv::ml;

CvMat *weakResponses=0;
CvMat *x=0;
int predictorsLength;

ClassifierBoost::ClassifierBoost()
    : weakCount(0), scoreMultiplier(1.f), boostType(Boost::GENTLE)
{
	bSingleClass = false;
}

ClassifierBoost::~ClassifierBoost()
{
	if(model) model->clear();
    if(weakResponses) cvReleaseMat(&weakResponses);
}

vector<fvec> ClassifierBoost::learners;
int ClassifierBoost::currentLearnerType = -1;
int ClassifierBoost::learnerCount=1000;
int ClassifierBoost::svmCount=2;

void ClassifierBoost::InitLearners(fvec xMin, fvec xMax)
{
    srand(1); // so we always generate the same weak learner
    switch(weakType)
    {
    case 0: // stumps
        learnerCount = dim;
        break;
    case 1: // projections
        learnerCount = dim>2?1000:360;
        break;
    case 2: // rectangles
    case 3: // circles
    case 4: // gmm
    case 5: // svm
        learnerCount = 3000;
        break;
    }
    learnerCount = max(learnerCount, (int)weakCount);

    learners.clear();
    learners.resize(learnerCount);
    // we generate a bunch of random directions as learners
//	srand(1);
    switch(weakType)
    {
    case 0:// stumps
    {
        FOR(i, learnerCount)
        {
            learners[i].resize(1);
            learners[i][0] = i % dim; // we choose a single dimension
        }
    }
        break;
    case 1:// random projection
    {
        if(dim==2)
        {
            FOR(i, learnerCount)
            {
                learners[i].resize(dim);
                float theta = i / (float)learnerCount * PIf;
                learners[i][0] = cosf(theta);
                learners[i][1] = sinf(theta);
            }
        }
        else
        {
            FOR(i, learnerCount)
            {
                learners[i].resize(dim);
                fvec projection(dim,0);
                float norm = 0;
                FOR(d, dim)
                {
                    projection[d] = drand48();
                    norm += projection[d];
                }
                FOR(d, dim) learners[i][d] = projection[d] / norm;
            }
        }
    }
        break;
    case 2: // random rectangle
    {
        FOR(i, learnerCount)
        {
            learners[i].resize(dim*2);
            FOR(d, dim)
            {
                float x = drand48()*(xMax[d] - xMin[d]) + xMin[d]; // rectangle center
                //float x = (drand48()*2-0.5)*(xMax[d] - xMin[d]) + xMin[d]; // rectangle center
                float l = drand48()*(xMax[d] - xMin[d]); // width
                //float x = drand48()*(xMax[d] - xMin[d]) + xMin[d]; // rectangle center
                //float l = drand48()*(xMax[d] - xMin[d]); // width
                learners[i][2*d] = x;
                learners[i][2*d+1] = l;
            }
        }
    }
        break;
    case 3: // random circle
    {
        if(dim==2)
        {
            FOR(i, learnerCount)
            {
                learners[i].resize(dim);
                learners[i][0] =  drand48()*(xMax[0]-xMin[0]) + xMin[0];
                learners[i][1] =  drand48()*(xMax[1]-xMin[1]) + xMin[1];
            }
        }
        else
        {
            FOR(i, learnerCount)
            {
                learners[i].resize(dim);
                FOR(d, dim) learners[i][d] = drand48()*(xMax[d]-xMin[d]) + xMin[d];
            }
        }
    }
        break;
    case 4: // random GMM
    {
        FOR(i, learnerCount)
        {
            learners[i].resize(dim + dim*(dim+1)/2); // a center plus a covariance matrix
            // we generate a random center
            FOR(d, dim)
            {
                learners[i][d] = drand48()*(xMax[d] - xMin[d]) + xMin[d];
            }
            // we generate a random covariance matrix
            float minLambda = (xMax[0]-xMin[0])*0.01f; // we set the minimum covariance lambda to 1% of the data span
            fvec C = RandCovMatrix(dim, minLambda);
            FOR(d1, dim)
            {
                FOR(d2,d1+1)
                {
                    int index = d1*(d1+1)/2 + d2; // index in triangular matrix form
                    learners[i][dim + index] = C[d1*dim + d2];
                }
            }
        }
    }
        break;
    case 5: // random SVM
    {
        FOR(i, learnerCount)
        {
            learners[i].resize(1 + svmCount*(dim+1)); // a kernel width plus svmCount points plus svmCount alphas
            learners[i][0] = 1.f / drand48()*(xMax[0]-xMin[0]); // kernel width proportional to the data
            float sumAlpha=0;
            FOR(j, svmCount)
            {
                // we generate a random alpha
                if(j<svmCount-1) sumAlpha += (learners[i][1+(dim+1)*j] = drand48()*2.f - 1.f);
                else learners[i][1+(dim+1)*j] = -sumAlpha; // we ensure that the sum of all alphas is zero
                // and the coordinates of the SV
                FOR(d, dim)
                {
                    learners[i][1+(dim+1)*j+1 + d] = drand48()*(xMax[d]-xMin[d])+xMin[d];
                }
            }
        }
    }
        break;
    }
    currentLearnerType = weakType;
    if(x) cvReleaseMat(&x);
    x = cvCreateMat(1, learners.size(), CV_32FC1);
}

fvec ClassifierBoost::GetFeatures(const fvec sample, const int weakType, const ivec features=ivec()) const
{
    fvec res(learnerCount,0);
    if(features.size()) {
        FOR(i, features.size()) {
            fvec& learner = learners[features[i]];
            float val=0;
            switch(weakType) {
            case 0:// stumps
            {
                int index = learner[0];
                val = index < dim ? sample[index] : 0;
            }
                break;
            case 1:// random projection
            {
                if(dim == 2) val = sample[0]* learner[0] + sample[1]* learner[1];
                else {
                    FOR(d, dim) val += sample[d] * learner[d];
                }
            }
                break;
            case 2:// random rectangles
            {
                // check if the sample is inside the recangle generated by the classifier
                val = 1;
                FOR(d, dim)
                {
                    if(sample[d] < learner[2*d] || sample[d] > learner[2*d]+learner[2*d+1])
                    {
                        val = 0;
                        break;
                    }
                }
                val += drand48()*0.01; // we add a small noise to the value just to not have only 0s and 1s
            }
                break;
            case 3: // random circle
            {
                if(dim==2) val = sqrtf((sample[0] - learner[0])*(sample[0] - learner[0]) + (sample[1] - learner[1])*(sample[1] - learner[1]));
                else {
                    FOR(d,dim) val += (sample[d] - learner[d])*(sample[d] - learner[d]);
                    val = sqrtf(val);
                }
            }
                break;
            case 4: // random GMM
            {
                fvec &gmm = learner;
                fvec x(dim);
                FOR(d, dim) x[d] = sample[d]-gmm[d];
                FOR(d, dim)
                {
                    float xC = 0;
                    FOR(d1,dim)
                    {
                        int index = d1>d? d1*(d1+1)/2 + d : d*(d+1)/2 + d1;
                        xC += x[d1]*gmm[dim+index];
                    }
                    val += xC*x[d];
                }
            }
                break;
            case 5: // random SVM
            {
                // compute the svm function
                fvec &svm = learner;
                float gamma = svm[0];
                FOR(k, svmCount)
                {
                    float alpha = svm[1+k*(dim+1)];
                    // we compute the rbf kernel;
                    float K = 0;
                    int index = 1+k*(dim+1)+1;
                    FOR(d, dim)
                    {
                        float dist = sample[d]-svm[index+d];
                        K += dist*dist;
                    }
                    K *= gamma;
                    val += alpha*expf(-K);
                }
            }
                break;
            }
            res[features[i]] = val;
        }
        return res;
    }


    switch(weakType)
    {
    case 0:// stumps
    {
        FOR(j, learnerCount)
        {
            int index = learners[j][0];
            float val = index < dim ? sample[index] : 0;
            res[j] = val;
        }
    }
        break;
    case 1:// random projection
    {
        FOR(j, learnerCount)
        {
            if(dim == 2) res[j] = sample[0]* learners[j][0] + sample[1]* learners[j][1];
            else {
                float val = 0;
                FOR(d, dim) val += sample[d] * learners[j][d];
                res[j] = val;
            }
        }
    }
        break;
    case 2:// random rectangles
    {
        // check if the sample is inside the recangle generated by the classifier
        FOR(j, learnerCount)
        {
            float val = 1;
            FOR(d, dim)
            {
                if(sample[d] < learners[j][2*d] || sample[d] > learners[j][2*d]+learners[j][2*d+1])
                {
                    val = 0;
                    break;
                }
            }
            res[j] = val + drand48()*0.01; // we add a small noise to the value just to not have only 0s and 1s
        }
    }
        break;
    case 3: // random circle
    {
        FOR(j, learnerCount)
        {
            float val = 0;
            if(dim==2) val = sqrtf((sample[0] - learners[j][0])*(sample[0] - learners[j][0]) + (sample[1] - learners[j][1])*(sample[1] - learners[j][1]));
            else {
                FOR(d,dim) val += (sample[d] - learners[j][d])*(sample[d] - learners[j][d]);
                val = sqrtf(val);
            }
            res[j] = val;
        }
    }
        break;
    case 4: // random GMM
    {
        FOR(j, learnerCount)
        {
            fvec &gmm = learners[j];
            float val = 0;
            fvec x(dim);
            FOR(d, dim) x[d] = sample[d]-gmm[d];
            FOR(d, dim)
            {
                float xC = 0;
                FOR(d1,dim)
                {
                    int index = d1>d? d1*(d1+1)/2 + d : d*(d+1)/2 + d1;
                    xC += x[d1]*gmm[dim+index];
                }
                val += xC*x[d];
            }
            res[j] = val;
        }
    }
        break;
    case 5: // random SVM
    {
        // compute the svm function
        FOR(j, learnerCount)
        {
            fvec &svm = learners[j];
            float val = 0;
            float gamma = svm[0];
            FOR(k, svmCount)
            {
                float alpha = svm[1+k*(dim+1)];
                // we compute the rbf kernel;
                float K = 0;
                int index = 1+k*(dim+1)+1;
                FOR(d, dim)
                {
                    float dist = sample[d]-svm[index+d];
                    K += dist*dist;
                }
                K *= gamma;
                val += alpha*expf(-K);
            }
            res[j] = val;
        }
    }
        break;
    }
    return res;
}

void ClassifierBoost::Train( std::vector< fvec > samples, ivec labels )
{
	if(model)model->clear();
	u32 sampleCnt = samples.size();
    if(!sampleCnt) return;
    if(sampleCnt < 16)
    {
        vector<fvec> copy = samples;
        ivec lcopy = labels;
        while(sampleCnt < 16)
        {
            samples.insert(samples.end(), copy.begin(), copy.end());
            labels.insert(labels.end(), lcopy.begin(), lcopy.end());
            sampleCnt = samples.size();
        }
    }
	dim = samples[0].size();
	u32 *perm = randPerm(sampleCnt);
    this->samples = samples;
    this->labels = labels;

    // we need to find the boundaries
    fvec xMin(dim, FLT_MAX), xMax(dim, -FLT_MAX);
    FOR(i, samples.size())
    {
        FOR(d,dim)
        {
            if(xMin[d] > samples[i][d]) xMin[d] = samples[i][d];
            if(xMax[d] < samples[i][d]) xMax[d] = samples[i][d];
        }
    }

    // we need to regenerate the learners
    if(currentLearnerType != weakType || learners.size() != learnerCount) InitLearners(xMin, xMax);

    Mat trainSamples(sampleCnt, learnerCount, CV_32FC1);
    Mat trainLabels(sampleCnt, 1, CV_32FC1);
    Mat sampleWeights(sampleCnt, 1, CV_32FC1);

    FOR(i, sampleCnt)
    {
        fvec sample = samples[perm[i]];
        fvec trainSample = GetFeatures(sample, weakType);
        FOR(j, learnerCount) trainSamples.at<float>(i,j) = trainSample[j];
        trainLabels.at<float>(i) = (float)labels[perm[i]];
        sampleWeights.at<float>(i) = 1.f;
    }

    int maxSplit = 1;
    model = Boost::create();
    model->setBoostType(boostType);
    model->setWeakCount(weakCount);
    model->setMaxDepth(maxSplit);
    model->setWeightTrimRate(0.95);

    model->train(trainSamples, ROW_SAMPLE, trainLabels);
    vector<Boost::Split> splits = model->getSplits();
    features.clear();
    FOR(i, splits.size()) {
        Boost::Split& s = splits.at(i);
        features.push_back(s.varIdx);
    }

    scoreMultiplier = 1.f;
    float maxScore=-FLT_MAX, minScore=FLT_MAX;
    FOR(i, samples.size())
    {
        float score = Test(samples[i]);
        if(score > maxScore) maxScore = score;
        if(score < minScore) minScore = score;
        qDebug() << "score" << i << score;
    }
    if(minScore != maxScore)
    {
        scoreMultiplier = 1.f/(max(abs((double)maxScore),abs((double)minScore)))*5.f;
    }

    // we want to compute the error weight for each training sample
    vector<fvec> responses(predictorsLength);
    FOR(i, predictorsLength) responses[i].resize(sampleCnt);
    // first we compute all the errors, for each learner
    FOR(i, sampleCnt)
    {
        fvec response(predictorsLength);
        Test(samples[i], &response);
        FOR(j, predictorsLength) responses[j][i] = response[j];
    }
    // then we iterate through the learners
    errorWeights = fvec(sampleCnt,1.f);
    FOR(i, responses.size())
    {
        double sum = 0;
        // we compute the current weighted error
        FOR(j, sampleCnt)
        {
            double response = responses[i][j];
            //debugString += QString("%1(%2) ").arg(response,0,'f',2).arg(labels[perm[j]]);
            if((response < 0 && labels[j] == 1) || (response >= 0 && labels[j]!=1)) sum += fabs(response)*errorWeights[j]/sampleCnt;
        }
        //qDebug() << debugString;
        double c = sqrtf(fabs((1-sum)/sum));

        // we update the individual weights
        sum = 0;
        FOR(j, sampleCnt)
        {
            double response = responses[i][j];
            if((response < 0 && labels[j] == 1) || (response >= 0 && labels[j]!=1)) errorWeights[j] *= c;
            else errorWeights[j] *= 1.f/c;
            sum += errorWeights[j];
        }
        sum /= sampleCnt;
        // and we renormalize them
        FOR(j, sampleCnt) errorWeights[j] /= sum;
    }

    //QString debugString;
    //FOR(i, sampleCnt) debugString += QString("%1 ").arg(errorWeights[i],0,'f',3);
    //qDebug() << "errorWeights" << debugString;

    delete [] perm;
}


float ClassifierBoost::Test( const fvec &sample ) const
{
    return Test(sample, 0);
}

float ClassifierBoost::Test( const fvec &sample, fvec *responses) const
{
    if(!model) return 0;
    if(!learners.size()) return 0;
    if(!features.size()) return 0;

    fvec testSample = GetFeatures(sample, weakType, features);
    Mat input(1,learnerCount,CV_32FC1);
    FOR(d, learnerCount)  input.at<float>(d) = testSample[d];

    Mat res;//(1,features.size(),CV_32FC1);
    float result = model->predict(input, res, StatModel::RAW_OUTPUT);
    return (result-0.5)*3;
    //res.at<float>(0);
    //y = res.at<float>(0);
    //float y = model->predict(x, NULL, weakResponses, CV_WHOLE_SEQ);
    /*
    if(responses != NULL)
    {
        (*responses).resize(predictorsLength);
        FOR(i, predictorsLength) (*responses)[i] = cvGet1D(weakResponses, i).val[0];
    }
    double score = cvSum(weakResponses).val[0] * scoreMultiplier;
    return score;
    */
}

void ClassifierBoost::SetParams( u32 weakCount, int weakType, int boostType, int svmCount)
{
	this->weakCount = weakCount;
    this->weakType = weakType;
    this->boostType = boostType;
    if(this->svmCount != svmCount)
    {
        // if we changed the number of svms we need to regenerate the learners
        this->svmCount = svmCount;
        if(weakType == 5) currentLearnerType = -1;
    }
}

const char *ClassifierBoost::GetInfoString() const
{
	char *text = new char[1024];
	sprintf(text, "Boosting\n");
	sprintf(text, "%sLearners Count: %d\n", text, weakCount);
	sprintf(text, "%sLearners Type: ", text);
	switch(weakType)
	{
    case 0:
        sprintf(text, "%sDecision Stumps\n", text);
        break;
    case 1:
        sprintf(text, "%sRandom Projections\n", text);
        break;
    case 2:
		sprintf(text, "%sRandom Rectangles\n", text);
		break;
    case 3:
        sprintf(text, "%sRandom Circles\n", text);
        break;
    case 4:
        sprintf(text, "%sRandom GMM\n", text);
        break;
    case 5:
        sprintf(text, "%sRandom SVM %d\n", text, svmCount);
        break;
    }
	return text;
}
