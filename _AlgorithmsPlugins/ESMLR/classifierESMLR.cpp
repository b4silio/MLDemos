/*********************************************************************
MLDemos: A User-Friendly visualization toolkit for machine learning
Copyright (C) 2010  Basilio Noris
Contact: mldemos@b4silio.com

Evolution-Strategy Mixture of Logisitics Regression
Copyright (C) 2011  Stephane Magnenat
Contact: stephane at magnenat dot net

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

#include "classifierESMLR.h"
#include "MixtureLogisticRegression.h"

#include <iostream>
#include <algorithm>
#include <nlopt/nlopt.hpp>

// classifier for mld

ClassifierESMLR::ClassifierESMLR():
	cutCount(2),
	alpha(3),
	genCount(30),
	indPerDim(50),
	classifier(0)
{
	bSingleClass = true;
	//bUseDrawTimer = false;
	type = CLASS_ESMLR;
	classThresh = 0;
	classSpan = 1;
	dim = 0;
}

ClassifierESMLR::~ClassifierESMLR()
{
	if (classifier)
		delete classifier;
}

void ClassifierESMLR::Train(std::vector< fvec > samples, ivec labels)
{
	assert(labels.size() == samples.size());
	dim = samples[0].size();
	
	// Creating data object
	MLR::Data data(samples.size(), dim, 0, cutCount);
	for (size_t i = 0; i < labels.size(); ++i)
	{
		data.y(i) = labels[i];
		for (size_t j = 0; j < samples[i].size(); ++j)
			data.x(i,j) = samples[i][j];
	}
	std::cerr << "Learning data:\ny:\n" << data.y << "\nx:\n" << data.x << std::endl;
	
	// compute stddev
	const Eigen::VectorXd avr = data.x.colwise().sum() / double(data.x.rows());
	Eigen::VectorXd stdDev(Eigen::VectorXd::Constant(avr.size(), 0));
	for (int i = 0; i < data.x.rows(); ++i)
	{
		const Eigen::VectorXd& row(data.x.row(i));
		const Eigen::VectorXd delta(row - avr);
		stdDev += delta.cwiseProduct(delta);
	}
	stdDev /= data.x.rows();
	stdDev = stdDev.cwiseSqrt();
	double dataAvrSd(stdDev.sum() / double(stdDev.size()));
	std::cerr << "stddev:\n" << stdDev << "\navr stddev: " << dataAvrSd << std::endl;
	const double beta(alpha / dataAvrSd);
	data.beta = beta;
	
	/*
	ES + gradient descent
	// ES
	ES::Population pop(cutCount, dim, dataAvrSd, beta, indPerDim);
	if (classifier)
		delete classifier;
	classifier = new MLR::Classifier(pop.optimise(data.y, data.x, dataAvrSd, genCount));
	std::cerr << "Score before local opt: " << classifier->sumSquareError(data.y, data.x) << std::endl;
	
	// local opt
	nlopt::opt localOpt(nlopt::LD_SLSQP, classifier->getSize());
	//nlopt::opt localOpt(nlopt::LD_LBFGS, classifier->getSize());
	localOpt.set_lower_bounds(classifier->lowerBounds());
	localOpt.set_upper_bounds(classifier->upperBounds());
	localOpt.set_min_objective(MLR::f, &data);
	localOpt.set_xtol_rel(1e-4);
	localOpt.set_maxeval(10000);
	typedef std::vector<MLR::Norm2ConstraintData> ConstraintVector;
	ConstraintVector wConstraints(cutCount, MLR::Norm2ConstraintData(0, dim));
	for (size_t i = 0; i < wConstraints.size(); ++i)
	{
		wConstraints[i].start = i * dim;
		localOpt.add_equality_constraint(MLR::norm2_constraint, &(wConstraints[i]));
	}
	MLR::Norm2ConstraintData vConstraint(classifier->vIdx(), cutCount);
	localOpt.add_equality_constraint(MLR::norm2_constraint, &vConstraint);
	
	double opt_f;
	std::vector<double> v(classifier->toRawVector());
	nlopt::result res = localOpt.optimize(v, opt_f);
	std::cerr << "Score after local opt: " << opt_f << std::endl;
	
	*classifier = MLR::Classifier::fromRawVector(&v[0], cutCount, dim, beta);
	std::cerr << "Classifier:" << std::endl;
	std::cerr << *classifier << std::endl;
	*/
	
	/*
		random restart + gradient descient
	*/
	double bestVal(HUGE_VAL);
	double bestInitVal(HUGE_VAL);
	for (int rrc = 0; rrc < 100; ++rrc)
	{
		MLR::Classifier testClassifier(cutCount, dim, beta);
		double newError;
		unsigned randomCount(0);
		do
		{
			testClassifier.setRandom(dataAvrSd);
			newError = testClassifier.sumSquareError(data.y, data.x);
			bestInitVal = std::min(bestInitVal, newError);
		}
		while ((bestInitVal != HUGE_VAL) && (newError > 2 * bestInitVal) && (++randomCount < 1000));
		
		// local opt
		nlopt::opt localOpt(nlopt::LD_SLSQP, testClassifier.getSize());
		//nlopt::opt localOpt(nlopt::LD_LBFGS, testClassifier.getSize());
		localOpt.set_lower_bounds(testClassifier.lowerBounds());
		localOpt.set_upper_bounds(testClassifier.upperBounds());
		localOpt.set_min_objective(MLR::f, &data);
		localOpt.set_ftol_abs(1e-6);
		localOpt.set_maxeval(1000);
		typedef std::vector<MLR::Norm2ConstraintData> ConstraintVector;
		ConstraintVector wConstraints(cutCount, MLR::Norm2ConstraintData(0, dim));
		for (size_t i = 0; i < wConstraints.size(); ++i)
		{
			wConstraints[i].start = i * dim;
			localOpt.add_equality_constraint(MLR::norm2_constraint, &(wConstraints[i]));
		}
		MLR::Norm2ConstraintData vConstraint(testClassifier.vIdx(), cutCount);
		localOpt.add_equality_constraint(MLR::norm2_constraint, &vConstraint);
		
		double opt_f;
		std::vector<double> v(testClassifier.toRawVector());
		try {
			nlopt::result res = localOpt.optimize(v, opt_f);
		} catch (nlopt::roundoff_limited e) {
			// we can safely ignore this exception, the result being still valid
		}
		
		if (opt_f < bestVal)
		{
			if (!classifier)
				classifier = new MLR::Classifier(MLR::Classifier::fromRawVector(&v[0], cutCount, dim, beta));
			else
				*classifier = MLR::Classifier::fromRawVector(&v[0], cutCount, dim, beta);
			bestVal = opt_f;
		}
		
		std::cerr << "Step " << rrc << ", value " << opt_f << ", best: " << bestVal << ", bestInit: " << bestInitVal << std::endl;
	}
	
	std::cerr << *classifier << std::endl;
}

float ClassifierESMLR::Test(const fvec &sample)
{
	assert(classifier);
	Eigen::VectorXd x(dim);
	FOR(d, dim) x(d) = sample[d];
	return classifier->eval(x);
}

char *ClassifierESMLR::GetInfoString()
{
	char *text = new char[1024];
	snprintf(text, 1024, "Evolution Strategy, Mixture of Logistic Regressions\n"
	"hyperplane count: %d\n"
	"alpha: %f\n"
	"generation count: %d\n"
	"individual per dim: %d\n",
	cutCount, alpha, genCount, indPerDim
	);
	return text;
}

void ClassifierESMLR::SetParams(u32 cutCount, float alpha, u32 genCount, u32 indPerDim)
{
	this->cutCount = cutCount;
	this->alpha = alpha;
	this->genCount = genCount;
	this->indPerDim = indPerDim;
}
