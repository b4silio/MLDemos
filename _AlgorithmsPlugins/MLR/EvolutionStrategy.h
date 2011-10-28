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

#ifndef _MLR_EVOLUTION_STRATEGY_H
#define _MLR_EVOLUTION_STRATEGY_H

#include <map>

#include "MixtureLogisticRegression.h"

namespace ES
{
	using namespace MLR;
	typedef MLR::Classifier Classifier;
	
	struct Individual
	{
		Classifier classifier;
		
		double r_w;
		double r_b;
		double r_v;
		double r_v_b;
		
		Individual(unsigned cutCount = 0, unsigned dataSize = 0, double beta = 1);
		void mutate(double dataAvrSd);
		Individual createChild(double dataAvrSd) const;
		static Individual createRandom(unsigned cutCount, unsigned dataSize, double dataAvrSd, double beta);
	};
	
	struct Population: protected std::vector<Individual>
	{
		typedef std::pair<double, double> ErrorPair;
		
		Population(unsigned cutCount, unsigned dataSize, double dataAvrSd, double beta, unsigned indPerDim);
		ErrorPair evolveOneGen(const VectorXd& y, const MatrixXd& x, double dataAvrSd);
		Classifier optimise(const VectorXd& y, const MatrixXd& x, double dataAvrSd, size_t genCount);
	};
	
} // namespace ES

#endif // _EVOLUTION_STRATEGY_H