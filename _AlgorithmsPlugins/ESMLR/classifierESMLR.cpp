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

#include <Eigen/Core>
#include <Eigen/Eigen>
#include <vector>
#include <map>
#include <iostream>

// ES-related stuff

namespace ESMLR
{
	using namespace Eigen;
	
	double uniformRand(double min, double max)
	{
		const double v = double(rand())/double(RAND_MAX);
		return (min + v * (max-min));
	}

	double gaussianRand(double mean, double sigm)
	{
		// Generation using the Polar (Box-Mueller) method.
		// Code inspired by GSL, which is a really great math lib.
		// http://sources.redhat.com/gsl/
		// C++ wrapper available.
		// http://gslwrap.sourceforge.net/
		double r, x, y;

		// Generate random number in unity circle.
		do
		{
			x = uniformRand(-1, 1);
			y = uniformRand(-1, 1);
			r = x*x + y*y;
		}
		while (r > 1.0 || r == 0);

		// Box-Muller transform.
		return sigm * y * sqrt (-2.0 * log(r) / r) + mean;
	}
	
	double sigm(double v)
	{
		return 1. / (1. + exp(-v));
	}
	
	double sgn(double v)
	{
		if (v > 0) return 1;
		if (v < 0) return -1;
		return 0;
	}
	
	struct Classifier
	{
		MatrixXd w;
		VectorXd w_b;
		VectorXd v;
		double v_b;
		double beta;
		
		Classifier(unsigned cutCount, unsigned dataSize, double beta):
			w(cutCount, dataSize),
			w_b(cutCount),
			v(cutCount),
			beta(beta)
		{}
		double eval(const VectorXd& x) const;
		friend std::ostream& operator<< (std::ostream& stream, const Classifier& that);
	};
	
	double Classifier::eval(const VectorXd& x) const
	{
		assert(w.rows() == w_b.size());
		assert(w_b.size() == v.size());
		assert(w.cols() == x.size());
		double sum(0);
		for (int i = 0; i < w.rows(); ++i)
			sum += v(i) * (2. * sigm(beta * (w.row(i).dot(x) + w_b(i))) - 1.);
		sum += v_b;
		return 2. * sigm(w.rows() * 2 * sum) - 1.;
		//return sum > 0 ? 1 : -1;
	}
	
	std::ostream& operator<< (std::ostream& stream, const Classifier& that)
	{
		stream << "Classifier on " << that.w.cols() << " dimensions with " << that.w.rows() << " hyperplanes" << std::endl;
		stream << "w:\n" << that.w << std::endl;
		stream << "w_b:\n" << that.w_b << std::endl;
		stream << "v:\n" << that.v << std::endl;
		stream << "v_b:\n" << that.v_b << std::endl;
		stream << "beta:\n" << that.beta << std::endl;
		return stream;
	}
	
	struct Individual
	{
		Classifier classifier;
		
		double r_w;
		double r_w_b;
		double r_v;
		double r_v_b;
		
		Individual(unsigned cutCount = 0, unsigned dataSize = 0, double beta = 1);
		void mutate(double dataAvrSd);
		Individual createChild(double dataAvrSd) const;
		static Individual createRandom(unsigned cutCount, unsigned dataSize, double dataAvrSd, double beta);
	};
	
	Individual::Individual(unsigned cutCount, unsigned dataSize, double beta):
		classifier(cutCount, dataSize, beta),
		r_w(1),
		r_w_b(1),
		r_v(1),
		r_v_b(1)
	{}
	
	void Individual::mutate(double dataAvrSd)
	{
		assert(classifier.w.rows() == classifier.w_b.size());
		assert(classifier.w.rows() == classifier.v.size());
		// mutate rate
		r_w *= (rand()%2 == 0) ? 1.25 : 0.8;
		r_w_b *= (rand()%2 == 0) ? 1.25 : 0.8;
		r_v *= (rand()%2 == 0) ? 1.25 : 0.8;
		r_v_b *= (rand()%2 == 0) ? 1.25 : 0.8;
		// mutate using rate
		const double aprioriRate(0.05);
		// w
		for (int i = 0; i < classifier.w.rows(); ++i)
		{
			for (int j = 0; j < classifier.w.cols(); ++j)
				classifier.w(i,j) += gaussianRand(0, aprioriRate*r_w);
			classifier.w.row(i) /= classifier.w.row(i).norm();
		}
		// w_b
		for (int i = 0; i < classifier.w_b.size(); ++i)
			classifier.w_b(i) += gaussianRand(0, dataAvrSd*aprioriRate*r_w_b);
		// v
		for (int i = 0; i < classifier.v.size(); ++i)
			classifier.v(i) += gaussianRand(0, aprioriRate*r_v);
		classifier.v /= classifier.v.norm();
		// v_b
		classifier.v_b += gaussianRand(0, double(classifier.v.size())*aprioriRate*r_v_b);
	}
	
	Individual Individual::createChild(double dataAvrSd) const
	{
		Individual child(*this);
		child.mutate(dataAvrSd);
		return child;
	}
	
	Individual Individual::createRandom(unsigned cutCount, unsigned dataSize, double dataAvrSd, double beta)
	{
		Individual ind(cutCount, dataSize, beta);
		Classifier& classifier(ind.classifier);
		// w
		for (int i = 0; i < classifier.w.rows(); ++i)
		{
			for (int j = 0; j < classifier.w.cols(); ++j)
				classifier.w(i,j) = uniformRand(-1, 1);
			classifier.w.row(i) /= classifier.w.row(i).norm();
		}
		// w_b
		for (int i = 0; i < classifier.w_b.size(); ++i)
			classifier.w_b(i) = gaussianRand(0, dataAvrSd);
		// v
		for (int i = 0; i < classifier.v.size(); ++i)
			classifier.v(i) = uniformRand(-1, 1);
		classifier.v /= classifier.v.norm();
		// v_b
		classifier.v_b = uniformRand(-double(classifier.v.size()), double(classifier.v.size()));
		return ind;
	}
	
	struct Population: protected std::vector<Individual>
	{
		typedef std::pair<double, double> ErrorPair;
		
		Population(unsigned cutCount, unsigned dataSize, double dataAvrSd, double beta, unsigned indPerDim);
		ErrorPair evolveOneGen(const VectorXd& y, const MatrixXd& x, double dataAvrSd);
		Classifier optimise(const VectorXd& y, const MatrixXd& x, double dataAvrSd, size_t genCount);
	};
	
	Population::Population(unsigned cutCount, unsigned dataSize, double dataAvrSd, double beta, unsigned indPerDim):
        std::vector<Individual>((cutCount*(dataSize+1)+1)*indPerDim)
	{
		// create initial population
		for (iterator it(begin()); it != end(); ++it)
			*it = Individual::createRandom(cutCount, dataSize, dataAvrSd, beta);
	}
	
	Population::ErrorPair Population::evolveOneGen(const VectorXd& y, const MatrixXd& x, double dataAvrSd)
	{
		assert(y.size() == x.rows());
		typedef std::multimap<double, Individual> EvaluationMap;
		typedef EvaluationMap::iterator EvaluationMapIterator;
		EvaluationMap evalutationMap;
		
		// evaluation
		double totalError(0);
		for (const_iterator it(begin()); it != end(); ++it)
		{
			const Individual& ind(*it);
			double error(0);
			for (int sample = 0; sample < y.size(); ++sample)
			{
				const double v(ind.classifier.eval(x.row(sample)));
				const double delta(y(sample) - v);
				error += delta*delta;
			}
			totalError += error;
			evalutationMap.insert(std::make_pair(error, ind));
		}
		const double averageError(totalError / double(size()));
		
        // selection
		assert((size() / 4) * 4 == size());
		size_t ind = 0;
		for (EvaluationMapIterator it = evalutationMap.begin(); ind < size() / 4; ++it, ++ind)
		{
			//cout << "S " << it->first << "\n";
			(*this)[ind * 4] = it->second;
			(*this)[ind * 4 + 1] = it->second.createChild(dataAvrSd);
			(*this)[ind * 4 + 2] = it->second.createChild(dataAvrSd);
			(*this)[ind * 4 + 3] = it->second.createChild(dataAvrSd);
		}
		
		// return statistics
		return ErrorPair(evalutationMap.begin()->first, averageError);
	}
	
	Classifier Population::optimise(const VectorXd& y, const MatrixXd& x, double dataAvrSd, size_t genCount)
	{
		// optimise
		for (size_t g = 0; g < genCount; ++g)
		{
			const ErrorPair e = evolveOneGen(y, x, dataAvrSd);
			std::cout << g << " : " << e.first << ", " << e.second << ", ";
			// compute number of missclassified
			unsigned missClassified(0);
			for (int sample = 0; sample < y.size(); ++sample)
				missClassified += fabs(sgn((*this)[0].classifier.eval(x.row(sample))) - y(sample)) / 2;
			std::cout << missClassified << std::endl;
		}
		return (*this)[0].classifier;
	}
}

// classifier for mld

ClassifierESMLR::ClassifierESMLR():
	cutCount(2),
	alpha(3),
	genCount(30),
	indPerDim(50),
	classifier(0)
{
	bSingleClass = true;
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
	Eigen::VectorXd y(labels.size());
	Eigen::MatrixXd x(samples.size(), dim);
	for (size_t i = 0; i < labels.size(); ++i)
	{
		y(i) = labels[i];
		for (size_t j = 0; j < samples[i].size(); ++j)
			x(i,j) = samples[i][j];
	}
	std::cerr << "Learning data:\ny:\n" << y << "\nx:\n" << x << std::endl;
	
	// compute stddev
	const Eigen::VectorXd avr = x.colwise().sum() / double(x.rows());
	Eigen::VectorXd stdDev(Eigen::VectorXd::Constant(avr.size(), 0));
	for (int i = 0; i < x.rows(); ++i)
	{
		const Eigen::VectorXd& row(x.row(i));
		const Eigen::VectorXd delta(row - avr);
		stdDev += delta.cwiseProduct(delta);
	}
	stdDev /= x.rows();
	stdDev = stdDev.cwiseSqrt();
	double dataAvrSd(stdDev.sum() / double(stdDev.size()));
	std::cerr << "stddev:\n" << stdDev << "\navr stddev: " << dataAvrSd << std::endl;
	
	const double beta(alpha / dataAvrSd);
	ESMLR::Population pop(cutCount, dim, dataAvrSd, beta, indPerDim);
	if (classifier)
		delete classifier;
	classifier = new ESMLR::Classifier(pop.optimise(y, x, dataAvrSd, genCount));
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
