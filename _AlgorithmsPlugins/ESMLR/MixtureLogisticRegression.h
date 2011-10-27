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

#ifndef _MLR_MIXTURE_LOGISTIC_REGRESSION_H
#define _MLR_MIXTURE_LOGISTIC_REGRESSION_H

#include <Eigen/Core>
#include <Eigen/Eigen>
#include <vector>

namespace MLR
{
	using namespace Eigen;
	using namespace std;
	
	double uniformRand(double min, double max);
	double gaussianRand(double mean, double sigm);
	
	double sigm(double v);
	double sgn(double v);
	
	struct Classifier
	{
		MatrixXd w;
		VectorXd b;
		VectorXd v;
		double v_b;
		double beta;
		
		Classifier(unsigned cutCount, unsigned dataSize, double beta);
		
		unsigned getSize() const;
		unsigned bIdx() const;
		unsigned vIdx() const;
		unsigned vbIdx() const;
		
		vector<double> lowerBounds() const;
		vector<double> upperBounds() const;
		vector<double> toRawVector() const;
		static Classifier fromRawVector(const double *data, int count, int dim, double beta);
		void setRandom(double dataAVrStd);
		
		double evalCut(const VectorXd& x, int i) const;
		double eval(const VectorXd& x) const;
		double sumSquareError(const VectorXd& y, const MatrixXd& x) const;
		
		friend std::ostream& operator<< (std::ostream& stream, const Classifier& that);
	};
	
	struct Data
	{
		VectorXd y;
		MatrixXd x;
		double beta;
		int cutCount;
		
		Data(int count, int dim, double beta, int cutCount):
			y(count),
			x(count, dim),
			beta(beta),
			cutCount(cutCount)
		{}
	};
	
	// objective function
	double f(unsigned n, const double* t, double* grad, void* f_data);
	
	struct Norm2ConstraintData
	{
		unsigned start;
		unsigned len;
		Norm2ConstraintData(unsigned start, unsigned len):
			start(start),
			len(len)
		{}
	};
	
	// constraint function
	double norm2_constraint(unsigned n, const double *t, double *grad, void* c_data);
	
} // MLR

#endif // _MIXTURE_LOGISTIC_REGRESSION_H