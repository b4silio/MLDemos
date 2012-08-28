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
    const MatrixXd & get() const { return _kernel; }
    virtual ~Kernel(){}

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
    }

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
    }
    /*
    virtual Kernel& operator= (const Kernel &k) {
        if (this != &k) {
            _kernel = k._kernel;
        }
        return *this;
    }
    */

protected:
    MatrixXd _kernel;
};

class LinearKernel : public Kernel
{
public:
    virtual ~LinearKernel(){}
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
    }
};

class PolyKernel : public Kernel
{
    double degree;
    double offset;
public:
    PolyKernel(int degree, double offset=0.) : degree(degree), offset(offset){}
    virtual ~PolyKernel(){}
    void Compute(MatrixXd &data)
    {
        _kernel = MatrixXd::Zero(data.cols(), data.cols());
        for (int i=0; i<data.cols();i++)
            for (int j=i; j <data.cols(); j++)
            {
                double value = (data.col(i).dot(data.col(j))) + offset;
                _kernel(i,j) = pow(value, degree);
                _kernel(j,i) = _kernel(i,j);
            }
    }

    virtual void Compute(MatrixXd &data, MatrixXd &source)
    {
        _kernel = MatrixXd::Zero(data.cols(), source.cols());
        for (int i=0; i<data.cols();i++)
            for (int j=0; j <source.cols(); j++)
            {
                double value = (data.col(i).dot(source.col(j))) + offset;
                _kernel(i,j) = pow(value, degree);
            }
    }
    /*
    virtual PolyKernel& operator= (const PolyKernel &k) {
        if (this != &k) {
            _kernel = k._kernel;
            degree = k.degree;
        }
        return *this;
    }
    */
};

class RBFKernel : public Kernel
{
    double gamma;
public:
    RBFKernel(float gamma) : gamma(gamma){}
    virtual ~RBFKernel(){}
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
    }
    /*
    virtual RBFKernel& operator= (const RBFKernel &k) {
        if (this != &k) {
            _kernel = k._kernel;
            gamma = k.gamma;
        }
        return *this;
    }
    */
};

class TANHKernel : public Kernel
{
    double alpha;
    double offset;
public:
    TANHKernel(double alpha, double offset=0.) : alpha(alpha), offset(offset){}
    virtual ~TANHKernel(){}
    void Compute(MatrixXd &data)
    {
        _kernel = MatrixXd::Zero(data.cols(), data.cols());
        for (int i=0; i<data.cols();i++)
            for (int j=i; j <data.cols(); j++)
            {
                double value = alpha * (data.col(i).dot(data.col(j))) + offset;
                _kernel(i,j) = tanh(value);
                _kernel(j,i) = _kernel(i,j);
            }
    }

    virtual void Compute(MatrixXd &data, MatrixXd &source)
    {
        _kernel = MatrixXd::Zero(data.cols(), source.cols());
        for (int i=0; i<data.cols();i++)
            for (int j=0; j <source.cols(); j++)
            {
                double value = alpha * (data.col(i).dot(source.col(j))) + offset;
                _kernel(i,j) = tanh(value);
            }
    }
};

class PCA
{
public:
    Kernel *k;
    VectorXd eigenvalues;
    MatrixXd eigenVectors;
    PermutationIndices pi;
    int kernelType;
    float degree;
    double gamma;
    double offset;
    MatrixXd sourcePoints;
    PCA() : k(0), kernelType(0), degree(2), gamma(0.1), offset(0.){}
    ~ PCA(){if(k) delete k;}
    //
    // compute the kernel pca
    //
    void kernel_pca(MatrixXd & dataPoints, unsigned int dimSpace);
    VectorXd project(VectorXd &point);
    MatrixXd project(MatrixXd &dataPoints, unsigned int dimSpace);
    float test(VectorXd point, int dim=0, double multiplier=1.);
    // get
    const MatrixXd & get() const { return _result; }
    PCA& operator= (const PCA &p) {
        if (this != &p) {
            eigenvalues = p.eigenvalues;
            eigenVectors = p.eigenVectors;
            kernelType = p.kernelType;
            degree = p.degree;
            gamma = p.gamma;
            sourcePoints = p.sourcePoints;
            pi = p.pi;
            _result = p._result;
            if(k)
            {
                delete k; k=0;
            }
            /*
            if(p.k)
            {
                if(!k) k = new Kernel();
                *k = *p.k;
            }
            */

        }
        return *this;
    }
private:
    MatrixXd _result;
};


#endif // EIGEN_PCA_H
