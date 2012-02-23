// Original code by Antonio Gulli, January 2011
// http://codingplayground.blogspot.com/

#ifndef EIGEN_PCA_H
#define EIGEN_PCA_H

#include <iostream>
#include <Eigen/Core>
#include <Eigen/Eigen>
#include <vector>

// Pretty simple and straightforward implementation of Kernel - PCA

using namespace Eigen;

typedef std::pair<double, int> myPair;
typedef std::vector<myPair> PermutationIndices;	


class Kernel
{
// We assume that the original dataset is represented in m x n matrix (m dimension, n points)
// so that points are represented by columns

// in this toy example we materialize the kernel, which is not necessary

public:
	const MatrixXd & get() const { return _kernel; };

	virtual void Compute(MatrixXd &data)
	{
		_kernel = MatrixXd::Zero(data.cols(), data.cols());
		// an example of kernel
		for (int i=0; i<data.cols();i++)
			for (int j=i; j <data.cols(); j++)
			{
				// an example of kernel projection
				double value = (data.col(i).dot(data.col(j)));
				_kernel(i,j) = value * value;
				_kernel(j,i) = _kernel(i,j);
			}
	};

	virtual void Compute(MatrixXd &data, MatrixXd &source)
	{
		_kernel = MatrixXd::Zero(data.cols(), source.cols());
		// an example of kernel
		for (int i=0; i<data.cols();i++)
			for (int j=0; j <source.cols(); j++)
			{
				// an example of kernel projection
				double value = (data.col(i).dot(source.col(j)));
				_kernel(i,j) = value * value;
			}
	};

protected:
	MatrixXd _kernel;
};

class LinearKernel : public Kernel
{
public:
	void Compute(MatrixXd &data)
	{
		_kernel = MatrixXd::Zero(data.cols(), data.cols());
		for (int i=0; i<data.cols();i++)
			for (int j=i; j <data.cols(); j++)
			{
				_kernel(i,j) = (data.col(i).dot(data.col(j)));
				_kernel(j,i) = _kernel(i,j);
			}
	}

	virtual void Compute(MatrixXd &data, MatrixXd &source)
	{
		_kernel = MatrixXd::Zero(data.cols(), source.cols());
		for (int i=0; i<data.cols();i++)
			for (int j=0; j <source.cols(); j++)
			{
				double value = (data.col(i).dot(source.col(j)));
				_kernel(i,j) = value;
			}
	};

};

class PolyKernel : public Kernel
{
	double degree;
public:
	PolyKernel(int degree) : degree(degree){};
	void Compute(MatrixXd &data)
	{
		_kernel = MatrixXd::Zero(data.cols(), data.cols());
		for (int i=0; i<data.cols();i++)
			for (int j=i; j <data.cols(); j++)
			{
				double value = (data.col(i).dot(data.col(j)));
				_kernel(i,j) = pow(fabs(value), degree);
				_kernel(j,i) = _kernel(i,j);
			}
	}

	virtual void Compute(MatrixXd &data, MatrixXd &source)
	{
		_kernel = MatrixXd::Zero(data.cols(), source.cols());
		for (int i=0; i<data.cols();i++)
			for (int j=0; j <source.cols(); j++)
			{
				double value = (data.col(i).dot(source.col(j)));
				_kernel(i,j) = pow(fabs(value), degree);
			}
	};
};

class RBFKernel : public Kernel
{
	double gamma;
public:
	RBFKernel(float gamma) : gamma(gamma){};
	void Compute(MatrixXd &data)
	{
		_kernel = MatrixXd::Zero(data.cols(), data.cols());
		for (int i=0; i<data.cols();i++)
			for (int j=i; j <data.cols(); j++)
			{
				double value = (data.col(i) - data.col(j)).transpose() * (data.col(i) - data.col(j));
				_kernel(i,j) = exp(-value*gamma);
				_kernel(j,i) = _kernel(i,j);
			}
	}
	virtual void Compute(MatrixXd &data, MatrixXd &source)
	{
		_kernel = MatrixXd::Zero(data.cols(), source.cols());
		for (int i=0; i<data.cols();i++)
			for (int j=0; j <source.cols(); j++)
			{
				double value = (data.col(i) - source.col(j)).transpose() * (data.col(i) - source.col(j));
				_kernel(i,j) = exp(-value*gamma);
			}
	};
};

class PCA
{
public:
	Kernel *k;
	VectorXd eigenvalues;
	MatrixXd eigenVectors;
	PermutationIndices pi;
	int kernelType;
	int degree;
	float gamma;
	MatrixXd sourcePoints;
        PCA() : k(0), kernelType(0), degree(2), gamma(0.01f){}
        ~ PCA(){if(k) delete k;}
	//
	// compute the kernel pca
	//
	void kernel_pca(MatrixXd & dataPoints, unsigned int dimSpace);
	MatrixXd project(MatrixXd &dataPoints, unsigned int dimSpace);
	float test(VectorXd point);
	// get
        const MatrixXd & get() const { return _result; }

private:
	MatrixXd _result;
};


#endif // EIGEN_PCA_H
