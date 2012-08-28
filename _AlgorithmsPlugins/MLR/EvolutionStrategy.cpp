/*********************************************************************
MLDemos: A User-Friendly visualization toolkit for machine learning
Copyright (C) 2010  Basilio Noris
Contact: mldemos@b4silio.com

Mixture of Logisitics Regression
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

#include <iostream>

#include "EvolutionStrategy.h"

namespace ES
{
	Individual::Individual(unsigned cutCount, unsigned dataSize, double beta):
		classifier(cutCount, dataSize, beta),
		r_w(1),
		r_b(1),
		r_v(1),
		r_v_b(1)
	{}
	
	void Individual::mutate(double dataAvrSd)
	{
		assert(classifier.w.rows() == classifier.b.size());
		assert(classifier.w.rows() == classifier.v.size());
		// mutate rate
		r_w *= (rand()%2 == 0) ? 1.25 : 0.8;
		r_b *= (rand()%2 == 0) ? 1.25 : 0.8;
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
		// b
		for (int i = 0; i < classifier.b.size(); ++i)
			classifier.b(i) += gaussianRand(0, dataAvrSd*aprioriRate*r_b);
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
		classifier.setRandom(dataAvrSd);
		return ind;
	}
	
	Population::Population(unsigned cutCount, unsigned dataSize, double dataAvrSd, double beta, unsigned indPerDim):
		vector<Individual>((((cutCount*(dataSize+1)+1)*indPerDim)/4)*4)
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
			const double error(ind.classifier.sumSquareError(y, x));
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
} // namespace ES