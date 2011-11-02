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
#ifndef _CLASSIFIER_LINEAR_H_
#define _CLASSIFIER_LINEAR_H_

#include <vector>
#include <mymaths.h>
#include "classifier.h"
#include "basicMath.h"

/**
 * @brief Linear Projections with Naive Bayes Classification
 *
 */
class ClassifierLinear : public Classifier
{
private:
	fvec meanAll, meanPos, meanNeg; /**< TODO */
	int linearType; /**< TODO */
	fVec W; /**< TODO */
	int threshold; /**< TODO */
	double* Transf; /**< TODO */
    float minResponse, maxResponse, midResponse;
    std::vector<fvec> projected;

	/**
	 * @brief Take the input samples and return its mean and covariance
	 *
	 * @param samples
	 * @param mean
	 * @param covar
	 */
	void GetCovariance(const std::vector<fvec> &samples, const fvec &mean, float ***covar);
	/**
	 * @brief Perform Principal Component Analysis on the input samples, and store the obtained components in W
	 *
	 * @param samples
	 * @param labels
	 */
	void TrainPCA(std::vector< fvec > samples, const ivec &labels);
	/**
	 * @brief Perform Linear Discriminant Analysis on the input samples, and store the obtained components in W
	 *
	 * @param samples
	 * @param labels
	 * @param bFisher Use Fisher-LDA instead of standard LDA
	 */
	void TrainLDA(std::vector< fvec > samples, const ivec &labels, bool bFisher=true);
	/**
	 * @brief Perform Independent Component Analysis on the input samples, and store the obtained components in Transf
	 *
	 * @param samples
	 * @param labels
	 */
	void TrainICA(std::vector< fvec > samples, const ivec &labels);

public:
	/**
	 * @brief Default Constructor
	 *
	 */
    ClassifierLinear() : threshold(0), linearType(0), Transf(0) {bUsesDrawTimer = false;}
    ~ClassifierLinear();
	/**
	 * @brief Perform the training, by gather the training parameters from the ui, and then training the corresponding classifier
	 *
	 * @param samples
	 * @param labels
	 */
	void Train(std::vector< fvec > samples, ivec labels);
	/**
	 * @brief Test a single sample using the current method selected
	 *
	 * @param sample
	 */
	float Test(const fvec &sample);
	/**
	 * @brief Get the algorithm information and statistics to be displayed in the main interface
	 *
	 */
	char *GetInfoString();
	/**
	 * @brief Project the input sample into local space
	 *
	 * @param sample
	 */
	fvec Project(const fvec &sample);
	/**
	 * @brief Project the input sample from local space back to world space
	 *
	 * @param sample
	 */
	fvec InvProject(const fvec &sample);
	/**
	 * @brief Set the algorithm parameters from the ui
	 *
	 * @param linearType
	 */
	void SetParams(u32 linearType);
	/**
	 * @brief Get the current mean for the positive or negative class
	 *
	 * @param positive
	 * @return fvec
	 */
    fvec GetMean(bool positive=true){return positive ? meanPos : meanNeg;}
	/**
	 * @brief
	 *
	 * @return fVec
	 */
    fVec GetW(){return W;}
	/**
	 * @brief
	 *
	 * @return int
	 */
    int GetType(){return linearType;}
};

#endif // _CLASSIFIER_LINEAR_H_
