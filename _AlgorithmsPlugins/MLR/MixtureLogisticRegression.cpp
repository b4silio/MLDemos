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

#include "MixtureLogisticRegression.h"

namespace MLR
{
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
	
	/*double sigm(double v)
	{
		return (2. / (1. + exp(-v))) - 1.;
	}*/
	
	double sigm(double v)
	{
		return tanh(v);
	}
	
	double sgn(double v)
	{
		if (v > 0) return 1;
		if (v < 0) return -1;
		return 0;
	}
	
	
	Classifier::Classifier(unsigned cutCount, unsigned dataSize, double beta):
		w(cutCount, dataSize),
		b(cutCount),
		v(cutCount),
		beta(beta)
	{}
	
	unsigned Classifier::getSize() const
	{
		return w.size() + b.size() + v.size() + 1;
	}
	
	unsigned Classifier::bIdx() const
	{
		return w.size();
	}
	
	unsigned Classifier::vIdx() const
	{
		return w.size() + b.size();
	}
	
	unsigned Classifier::vbIdx() const
	{
		return w.size() + b.size() + v.size();
	}
	
	vector<double> Classifier::lowerBounds() const
	{
		vector<double> ret;
		// write w
		for (int i = 0; i < w.rows(); ++i)
			for (int j = 0; j < w.cols(); ++j)
				ret.push_back(-1);
		// write b
		for (int i = 0; i < b.size(); ++i)
			ret.push_back(-HUGE_VAL);
		// write v
		for (int i = 0; i < v.size(); ++i)
			ret.push_back(-1);
		// write v_b
		ret.push_back(-HUGE_VAL);
		return ret;
	}
	
	vector<double> Classifier::upperBounds() const
	{
		vector<double> ret;
		// write w
		for (int i = 0; i < w.rows(); ++i)
			for (int j = 0; j < w.cols(); ++j)
				ret.push_back(1);
		// write b
		for (int i = 0; i < b.size(); ++i)
			ret.push_back(HUGE_VAL);
		// write v
		for (int i = 0; i < v.size(); ++i)
			ret.push_back(1);
		// write v_b
		ret.push_back(HUGE_VAL);
		return ret;
	}
	
	Classifier Classifier::fromRawVector(const double *data, int count, int dim, double beta)
	{
		Classifier c(count, dim, beta);
		// write w
		for (int i = 0; i < count; ++i)
			for (int j = 0; j < dim; ++j)
				c.w(i,j) = *data++;
		// write b
		for (int i = 0; i < count; ++i)
			c.b[i] = *data++;
		// write v
		for (int i = 0; i < count; ++i)
			c.v[i] = *data++;
		// write v_b
		c.v_b = *data;
		return c;
	}
	
	vector<double> Classifier::toRawVector() const
	{
		vector<double> ret;
		// write w
		for (int i = 0; i < w.rows(); ++i)
			for (int j = 0; j < w.cols(); ++j)
				ret.push_back(w(i,j));
		// write b
		for (int i = 0; i < b.size(); ++i)
			ret.push_back(b[i]);
		// write v
		for (int i = 0; i < v.size(); ++i)
			ret.push_back(v[i]);
		// write v_b
		ret.push_back(v_b);
		return ret;
	}
	
	void Classifier::setRandom(double dataAvrSd)
	{
		// w
		for (int i = 0; i < w.rows(); ++i)
		{
			for (int j = 0; j < w.cols(); ++j)
				w(i,j) = uniformRand(-1, 1);
			w.row(i) /= w.row(i).norm();
		}
		// b
		for (int i = 0; i < b.size(); ++i)
			b(i) = gaussianRand(0, dataAvrSd);
		// v
		for (int i = 0; i < v.size(); ++i)
			v(i) = uniformRand(-1, 1);
		v /= v.norm();
		// v_b
		v_b = uniformRand(-double(v.size()), double(v.size()));
	}
	
	double Classifier::evalCut(const VectorXd& x, int i) const
	{
		return sigm(beta * (w.row(i).dot(x) + b(i)));
	}
	
	double Classifier::eval(const VectorXd& x) const
	{
		assert(w.rows() == b.size());
		assert(b.size() == v.size());
		assert(w.cols() == x.size());
		double sum(0);
		for (int i = 0; i < w.rows(); ++i)
			sum += v(i) * evalCut(x, i);
		sum += v_b;
		const double gamma(2 * w.rows());
		return sigm(gamma * sum);
		//return sum > 0 ? 1 : -1;
	}
	
	double Classifier::sumSquareError(const VectorXd& y, const MatrixXd& x) const
	{
		double error(0);
		for (int sample = 0; sample < y.size(); ++sample)
		{
			const double v(eval(x.row(sample)));
			const double delta(y(sample) - v);
			error += delta*delta;
		}
		return error;
	}
	
	std::ostream& operator<< (std::ostream& stream, const Classifier& that)
	{
		stream << "Classifier on " << that.w.cols() << " dimensions with " << that.w.rows() << " hyperplanes" << std::endl;
		stream << "w:\n" << that.w << std::endl;
		stream << "b:\n" << that.b << std::endl;
		stream << "v:\n" << that.v << std::endl;
		stream << "v_b:\n" << that.v_b << std::endl;
		stream << "beta:\n" << that.beta << std::endl;
		return stream;
	}
	
	double f(unsigned n, const double* t, double* grad, void* f_data)
	{
		Data* data(reinterpret_cast<Data*>(f_data));
		const int dim(data->x.cols());
		const int cutCount(data->cutCount);
		assert(data->x.rows() == data->y.size());
		const int sampleCount(data->x.rows());
		const double beta(data->beta);
		const double gamma(2 * cutCount);
		
		Classifier classifier(Classifier::fromRawVector(t, cutCount, dim, beta));
		assert(classifier.getSize() == n);
		const unsigned bIdx(classifier.bIdx());
		const unsigned vIdx(classifier.vIdx());
		const unsigned vbIdx(classifier.vbIdx());
		
		double err(0.);
		if (grad)
			fill(grad, grad+n, 0.);
		for (int d = 0; d < sampleCount; ++d)
		{
			const double y(data->y[d]);
			const VectorXd& x(data->x.row(d));
			const double Sx = classifier.eval(x);
			const double Sx2m1 = Sx * Sx - 1;
			const double ymSx = y - Sx;
			if (grad)
			{
				for (int i = 0; i < cutCount; ++i)
				{
					const double vi = classifier.v[i];
					const double Sxi = classifier.evalCut(x, i);
					const double DfWi = -2 * (Sxi * Sxi - 1) * Sx2m1 * ymSx * gamma * beta * vi;
					for (int j = 0; j < dim; ++j)
						grad[i*dim+j] += DfWi * x[j];
					grad[bIdx + i] += DfWi;
					grad[vIdx + i] += 2. * Sx2m1 * ymSx * Sxi * gamma;
				}
				grad[vbIdx] += 2. * Sx2m1 * ymSx * gamma;
			}
			err += ymSx*ymSx;
		}
		//cerr << "NLOPT f called, cur err: " << err << " grad: " << (grad ? 1 : 0) << endl;
		//std::cerr << ESMLR::Classifier::fromRawVector(t, cutCount, dim, beta) << std::endl;
		return err;
	}
	
	double norm2_constraint(unsigned n, const double *t, double *grad, void* c_data)
	{
		Norm2ConstraintData *data(reinterpret_cast<Norm2ConstraintData*>(c_data));
		Eigen::Map<const Eigen::VectorXd> wi(t+data->start, data->len);
		if (grad)
		{
			fill(grad, grad+n, 0.);
			for (unsigned j = data->start; j < data->start + data->len; ++j)
				grad[j] = 2 * t[j];
		}
		//cerr << "NLOPT norm2_constraint " << data->start << " called, grad: " << (grad ? 1 : 0) << endl;
		return wi.dot(wi)-1;
	}
} // MLR
