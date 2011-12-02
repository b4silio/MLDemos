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

using namespace std;

ClassifierBoost::ClassifierBoost()
: model(0), weakCount(0), scoreMultiplier(1.f)
{
	bSingleClass = false;
}

ClassifierBoost::~ClassifierBoost()
{
	if(model) model->clear();
	DEL(model);
}

vector<fvec> learners;
int currentLearnerType = -1;

void ClassifierBoost::Train( std::vector< fvec > samples, ivec labels )
{
	if(model)model->clear();
	u32 sampleCnt = samples.size();
	if(!sampleCnt) return;
	DEL(model);
	dim = samples[0].size();
	u32 *perm = randPerm(sampleCnt);

	int learnerCount = max((!weakType?360 : 1000), (int)weakCount);
    if(dim > 2) learnerCount = 1000;
	if(currentLearnerType != weakType)
	{
		srand(1); // so we always generate the same weak learner
		learners.clear();
		learners.resize(learnerCount);
		// we generate a bunch of random directions as learners
	//	srand(1);
        if(weakType != 1) // random projection or random circle
		{
            float minV = -1, maxV = 1;
            if(weakType)
            {
                FOR(i, samples.size())
                {
                    FOR(d, dim)
                    {
                        minV = min(minV, samples[i][d]);
                        maxV = max(maxV, samples[i][d]);
                    }
                }
            }

			if(dim==2)
			{
				FOR(i, learnerCount)
				{
						learners[i].resize(dim);
						if(!weakType)
						{
							float theta = i / (float)learnerCount * PIf;
							//			float theta = rand()/(float)RAND_MAX*PIf;
							learners[i][0] = cosf(theta);
							learners[i][1] = sinf(theta);
						}
						else
						{
                            learners[i][0] =  (rand()/(float)RAND_MAX)*(maxV-minV) + minV;
                            learners[i][1] =  (rand()/(float)RAND_MAX)*(maxV-minV) + minV;
						}
				}
			}
			else
			{
				FOR(i, learnerCount)
				{
					learners[i].resize(dim);
                    if(!weakType) // random projection
					{
                        fvec projection(dim,0);
                        float norm = 0;
                        FOR(d, dim)
                        {
                            projection[d] = drand48();
                            norm += projection[d];
                        }
                        FOR(d, dim) learners[i][d] = projection[d] / norm;
                    }
                    else // random circles
					{
                        FOR(d, dim) learners[i][d] = (rand()/(float)RAND_MAX)*(maxV-minV) + minV;
					}
				}
			}
		}
		else // random rectangle
		{
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

			FOR(i, learnerCount)
			{
				learners[i].resize(dim*2);
				FOR(d, dim)
				{
                    float x = (rand() / (float)RAND_MAX)*(xMax[d] - xMin[d]) + xMin[d]; // rectangle center
                    float l = (rand() / (float)RAND_MAX)*(xMax[d] - xMin[d]); // width
                    //float l = ((rand()+1) / (float)RAND_MAX); // width ratio
                    learners[i][2*d] = x;
					learners[i][2*d+1] = l;
				}
			}
		}
		currentLearnerType = weakType;
	}

	CvMat *trainSamples = cvCreateMat(sampleCnt, learnerCount, CV_32FC1);
	CvMat *trainLabels = cvCreateMat(labels.size(), 1, CV_32FC1);
	CvMat *sampleWeights = cvCreateMat(samples.size(), 1, CV_32FC1);

    if(weakType != 1) // random projection or random circle
	{
		if(dim == 2)
		{
			FOR(i, sampleCnt)
			{
                fvec sample = samples[perm[i]];
                FOR(j, learnerCount)
				{
                    float val = 0;
					if(!weakType)
					{
                        val = sample[0]* learners[j][0] + sample[1]* learners[j][1];
					}
					else
					{
                        val = sqrtf((sample[0] - learners[j][0])*(sample[0] - learners[j][0])+
                            (sample[1] - learners[j][1])*(sample[1] - learners[j][1]));
					}
					cvSetReal2D(trainSamples, i, j, val);
				}
				cvSet1D(trainLabels, i, cvScalar((float)labels[perm[i]]));
				cvSet1D(sampleWeights, i, cvScalar(1));
			}

		}
		else
		{
			FOR(i, sampleCnt)
			{
				// project the sample in the direction of the learner 
				fvec sample = samples[perm[i]];
				FOR(j, learnerCount)
				{
					float val = 0;
                    if(!weakType)
                    {
                        FOR(d, dim) val += sample[d] * learners[j][d];
                    }
					else
					{
						FOR(d,dim) val += (sample[d] - learners[j][d])*(sample[d] - learners[j][d]);
						val = sqrtf(val);
					}
					cvSetReal2D(trainSamples, i, j, val);
				}
				cvSet1D(trainLabels, i, cvScalar((float)labels[perm[i]]));
				cvSet1D(sampleWeights, i, cvScalar(1));
			}
		}
	}
	else
	{
		FOR(i, sampleCnt)
		{
			// check if the sample is inside the recangle generated by the classifier
			const fvec sample = samples[perm[i]];
			FOR(j, learnerCount)
			{
                float val = 1;
				FOR(d, dim)
				{
                    //float v = fabs(sample[d] - learners[j][2*d]);
                    //float v = fabs(sample[d] - learners[j][2*d]) / learners[j][2*d+1];
                    //val += v;

                    if(sample[d] < learners[j][2*d] || sample[d] > learners[j][2*d]+learners[j][2*d+1])
                    {
                        val = 0;
                        break;
                    }
                }
                cvSetReal2D(trainSamples, i, j, val + rand()/(float)RAND_MAX*0.1);
			}
			cvSet1D(trainLabels, i, cvScalar((float)labels[perm[i]]));
			cvSet1D(sampleWeights, i, cvScalar(1));
		}
	}
	delete [] perm;

	CvMat *varType = cvCreateMat(trainSamples->width+1, 1, CV_8UC1);
	FOR(i, trainSamples->width)
	{
		CV_MAT_ELEM(*varType, u8, i, 0) = CV_VAR_NUMERICAL;
	}
	CV_MAT_ELEM(*varType, u8, trainSamples->width, 0) = CV_VAR_CATEGORICAL;

	int maxSplit = 1;
	CvBoostParams params(CvBoost::GENTLE, weakCount, 0.95, maxSplit, false, NULL);
	params.split_criteria = CvBoost::DEFAULT;
	model = new CvBoost();
	model->train(trainSamples, CV_ROW_SAMPLE, trainLabels, NULL, NULL, varType, NULL, params);

	scoreMultiplier = 1.f;
	float maxScore=-FLT_MAX, minScore=FLT_MAX;
	FOR(i, samples.size())
	{
		float score = Test(samples[i]);
		if(score > maxScore) maxScore = score;
		if(score < minScore) minScore = score;
	}
	if(minScore != maxScore)
	{
		scoreMultiplier = 1.f/(max(abs((double)maxScore),abs((double)minScore)))*5.f;
	}

	CvSeq *predictors = model->get_weak_predictors();
	int length = cvSliceLength(CV_WHOLE_SEQ, predictors);
	features.clear();
	FOR(i, length)
	{
		CvBoostTree *predictor = *CV_SEQ_ELEM(predictors, CvBoostTree*, i);
		CvDTreeSplit *split = predictor->get_root()->split;
        if(!split) continue;
		features.push_back(split->var_idx);
	}

	cvReleaseMat(&trainSamples);
	cvReleaseMat(&trainLabels);
	cvReleaseMat(&sampleWeights);
	cvReleaseMat(&varType);
	trainSamples = 0;
	trainLabels = 0;
	sampleWeights = 0;
	varType = 0;
}

float ClassifierBoost::Test( const fvec &sample )
{
	if(!model) return 0;
	if(!learners.size()) return 0;

	CvMat *x = cvCreateMat(1, learners.size(), CV_32FC1);
	if(weakType != 1)
	{
		if(dim == 2)
		{
			FOR(i, features.size())
			{
				float val = 0;
				if(!weakType)
				{
					val = sample[0] * learners[features[i]][0] + sample[1] * learners[features[i]][1];
				}
				else
				{
					val = sqrtf((sample[0] - learners[features[i]][0])*(sample[0] - learners[features[i]][0])+
						(sample[1] - learners[features[i]][1])*(sample[1] - learners[features[i]][1]));
				}
				cvSetReal2D(x, 0, features[i], val);
			}
		}
		else
		{
			FOR(i, features.size())
			{
				float val = 0;
                if(!weakType) sample * learners[features[i]];
				else
				{
					FOR(d,dim) val += (sample[d] - learners[features[i]][d])*(sample[d] - learners[features[i]][d]);
					val = sqrtf(val);
				}
				cvSetReal2D(x, 0, features[i], val);
			}
		}
	}
	else
	{
		FOR(i, features.size())
		{
            int val = 1;
			FOR(d, dim)
			{
                //float v = fabs(sample[d] - learners[features[i]][2*d]);
                //float v = fabs(sample[d] - learners[features[i]][2*d]) / learners[features[i]][2*d+1];
                //val += v;

                if(sample[d] < learners[features[i]][2*d] ||
                    sample[d] > learners[features[i]][2*d]+learners[features[i]][2*d+1])
                {
                    val = 0;
                    break;
                }
			}
            cvSetReal2D(x, 0, features[i], val + rand()/(float)RAND_MAX*0.1);
		}
	}

	// allocate memory for weak learner output
	int length = cvSliceLength(CV_WHOLE_SEQ, model->get_weak_predictors());
	CvMat *weakResponses = cvCreateMat(length, 1, CV_32FC1);
	float y = model->predict(x, NULL, weakResponses, CV_WHOLE_SEQ);
	double score = cvSum(weakResponses).val[0] * scoreMultiplier;

	cvReleaseMat(&weakResponses);
	cvReleaseMat(&x);
	return score;
}

void ClassifierBoost::SetParams( u32 weakCount, int weakType )
{
	this->weakCount = weakCount;
	this->weakType = weakType ;
}

char *ClassifierBoost::GetInfoString()
{
	char *text = new char[1024];
	sprintf(text, "Boosting\n");
	sprintf(text, "%sLearners Count: %d\n", text, weakCount);
	sprintf(text, "%sLearners Type: ", text);
	switch(weakType)
	{
	case 0:
		sprintf(text, "%sRandom Projections\n", text);
		break;
	case 1:
		sprintf(text, "%sRandom Rectangles\n", text);
		break;
	case 2:
		sprintf(text, "%sRandom Circles\n", text);
		break;
	}
	return text;
}
