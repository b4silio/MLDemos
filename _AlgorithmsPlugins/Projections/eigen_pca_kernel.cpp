// eigen_pca_kernel.cpp 

#include "eigen_pca.h"
#include <algorithm>
#include <QDebug>

void PCA::kernel_pca(MatrixXd & dataPoints, unsigned int dimSpace)
{
    int m = dataPoints.rows();
    int n = dataPoints.cols();

    sourcePoints = dataPoints;

    // ugly hack to ensure that we don't have empty polynomial or linear kernels
    if(n==1 && kernelType<=1)
    {
        bool bIsZero = true;
        for(int i=0; i<m; i++)
        {
            if(sourcePoints(i,0) != 0)
            {
                bIsZero = false;
                break;
            }
        }
        if(bIsZero)
            for(int i=0; i<m; i++)
                sourcePoints(i,0) = 1.f;
    }

    if(k) delete k; k=0;
    switch(kernelType)
    {
    case 0:
        k = new LinearKernel();
        break;
    case 1:
        k = new PolyKernel(degree, offset);
        break;
    case 2:
        k = new RBFKernel(gamma);
        break;
    case 3:
        k = new TANHKernel(degree, offset);
        break;
    default:
        k = new Kernel();
    }
    k->Compute(dataPoints);

    //std::cout << "K:\n" << k->get() << "\n";

    // ''centralize''
    MatrixXd K_Centralized = k->get()
            - MatrixXd::Ones(n, n)*k->get()
            - k->get()*MatrixXd::Ones(n, n)
            + MatrixXd::Ones(n, n)*k->get()*MatrixXd::Ones(n, n);
    //std::cout << "Centralized" << "\n";

    //std::cout << "K:\n" << k->get() << "\n";

    // compute the eigenvalue on the K_Centralized Matrix
    EigenSolver<MatrixXd> m_solve(K_Centralized);
    //std::cout << "got the eigenvalues, eigenvectors" << "\n";
    eigenvalues = m_solve.eigenvalues().real();
    // we need to check that the eigenvalues are ok
    bool bAllNans = true;
    bool bTooSmall = true;
    for (int i=0; i<n; i++)
    {
        if(eigenvalues(i) == eigenvalues(i))
        {
            bAllNans = false;
        }
        if(fabs(eigenvalues(i) > 1e-300))
        {
            bTooSmall = false;
        }
        if(!bAllNans && !bTooSmall) break;
    }
    /*
    for (int i=0; i<n; i++)
    {
        qDebug() << "eigenvalues" << i << eigenvalues(i);
    }
    */
    if(bAllNans || bTooSmall)
    {
        for (int i=0; i<n; i++) eigenvalues(i) = 1;
        eigenVectors = MatrixXd::Identity(n,n);
    }
    else
    {
        eigenVectors = m_solve.eigenvectors().real();
    }

    //std::cout << "eigv:\n" << eigenvalues << "\n";
    //std::cout << "eigs:\n" << eigenVectors << "\n";

    // sort and get the permutation indices
    pi.clear();
    for (int i = 0 ; i < n; i++)
        pi.push_back(std::make_pair(-eigenvalues(i), i));

    std::sort(pi.begin(), pi.end());

    // get top eigenvectors
    _result = MatrixXd::Zero(n, dimSpace);
    for (unsigned int i = 0; i < dimSpace; i++)
    {
        _result.col(i) = eigenVectors.col(pi[i].second); // permutation indices
    }

    /*
    MatrixXd sqrtE = MatrixXd::Zero(dimSpace, dimSpace);
    for (unsigned int i = 0; i < dimSpace; i++)
    {
        //sqrtE(i, i) = sqrt(-pi[i].first);
        sqrtE(i, i) = 1.0;
    }
    */

    // get the final data projection
    //_result = (sqrtE * _result.transpose()).transpose();
}

float PCA::test(VectorXd point, int dim, double multiplier)
{
    if(dim >= eigenVectors.cols()) return 0;

    int n = 1;
    int dimSpace = 1;
    int m = point.rows();
    if(k) delete k; k=0;

    switch(kernelType)
    {
    case 0:
        k = new LinearKernel();
        break;
    case 1:
        k = new PolyKernel(degree, offset);
        break;
    case 2:
        k = new RBFKernel(gamma);
        break;
    case 3:
        k = new TANHKernel(degree, offset);
        break;
    default:
        k = new Kernel();
    }

    MatrixXd onePoint = MatrixXd::Zero(m,1);
    for(int i=0; i<m; i++) onePoint(i,0) = point(i);
    k->Compute(onePoint, sourcePoints);

    //std::cout << "K:\n" << k->get() << "\n";
    //	std::cout << eigenvalues << "\n";

    double result = 0;
    for (int w=0; w<eigenVectors.rows(); w++)
    {
        //result += K(0,w) * eigenVectors(w,pi[dim].second); // permutation indices
        double kw = k->get()(0,w);
        result += kw * eigenVectors(w,pi[dim].second); // permutation indices
    }
    result = result * multiplier;
    //result = (result * 0.25f - 1)*2;
    //result *= eigenvalues(pi[dim].second);
    //std::cout << result << "\n";
    return result;
}

VectorXd PCA::project(VectorXd &point)
{
    int n = eigenVectors.cols();
    int m = point.rows();
    MatrixXd onePoint = MatrixXd::Zero(m,1);
    for(int i=0; i<m; i++) onePoint(i,0) = point(i);
    MatrixXd oneResult = project(onePoint, n);
    VectorXd result(m);
    for(int i=0; i<m; i++) result(i) = oneResult(0,i);
    return result;
}

MatrixXd PCA::project(MatrixXd &dataPoints, unsigned int dimSpace)
{
    int m = dataPoints.rows();
    int n = dataPoints.cols();

    if(k)
    {
        delete k; k=0;
    }
    switch(kernelType)
    {
    case 0:
        k = new LinearKernel();
        break;
    case 1:
        k = new PolyKernel(degree, offset);
        break;
    case 2:
        k = new RBFKernel(gamma);
        break;
    case 3:
        k = new TANHKernel(degree, offset);
        break;
    default:
        k = new Kernel();
    }
    k->Compute(dataPoints, sourcePoints);

    //std::cout << "K:\n" << k->get() << "\n";

    // ''centralize''
    MatrixXd K = k->get()
            - MatrixXd::Ones(k->get().rows(), k->get().rows())*k->get()
            - k->get()*MatrixXd::Ones(k->get().cols(), k->get().cols())
            + MatrixXd::Ones(k->get().rows(), k->get().rows())*k->get()*MatrixXd::Ones(k->get().cols(), k->get().cols());

    MatrixXd results = MatrixXd::Zero(n, dimSpace);
    for (unsigned int i = 0; i < dimSpace; i++)
    {
        for (int j=0; j<n; j++)
            for (int w=0; w<eigenVectors.rows(); w++)
                results(j,i) += K(j,w) * eigenVectors(w,pi[i].second); // permutation indices
    }

    /*
    MatrixXd sqrtE = MatrixXd::Zero(dimSpace, dimSpace);
    for (unsigned int i = 0; i < dimSpace; i++)
    {
        //		sqrtE(i, i) = 0.9;
    }

    for(int i=0; i < results.rows(); i++)
    {
        for(int j=0; j < results.cols(); j++)
        {
            //            results(i,j) *= 1.0;
        }
    }
    */

    // get the final data projection
    //results = (sqrtE * results.transpose()).transpose();

    return results;
}


