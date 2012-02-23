// eigen_pca_kernel.cpp 

#include "eigen_pca.h"
#include <algorithm>

void PCA::kernel_pca(MatrixXd & dataPoints, unsigned int dimSpace)
{
    int m = dataPoints.rows();
    int n = dataPoints.cols();

    sourcePoints = dataPoints;

    if(k) delete k;
    switch(kernelType)
    {
    case 0:
        k = new LinearKernel();
        break;
    case 1:
        k = new PolyKernel(degree);
        break;
    case 2:
        k = new RBFKernel(gamma);
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
    eigenVectors = m_solve.eigenvectors().real();

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

    MatrixXd sqrtE = MatrixXd::Zero(dimSpace, dimSpace);
    for (unsigned int i = 0; i < dimSpace; i++)
    {
        //sqrtE(i, i) = sqrt(-pi[i].first);
        sqrtE(i, i) = 0.9;
    }

    // get the final data projection
    _result = (sqrtE * _result.transpose()).transpose();
}

float PCA::test(VectorXd point)
{
    if(!k) return 0;

    int n = 1;
    int dimSpace = 1;
    int m = point.rows();

    switch(kernelType)
    {
    case 0:
        k = new LinearKernel();
        break;
    case 1:
        k = new PolyKernel(degree);
        break;
    case 2:
        k = new RBFKernel(gamma);
        break;
    default:
        k = new Kernel();
    }
    MatrixXd onePoint = MatrixXd::Zero(m,1);
    for(int i=0; i<m; i++) onePoint(i,0) = point(i);
    k->Compute(onePoint, sourcePoints);

    //std::cout << "K:\n" << k->get() << "\n";

    // ''centralize''
    MatrixXd K = k->get()
            - MatrixXd::Ones(k->get().rows(), k->get().rows())*k->get()
            - k->get()*MatrixXd::Ones(k->get().cols(), k->get().cols())
            + MatrixXd::Ones(k->get().rows(), k->get().rows())*k->get()*MatrixXd::Ones(k->get().cols(), k->get().cols());


    //	std::cout << K.row(0) << "\n";
    //	std::cout << eigenvalues << "\n";
    //	std::cout << k->get().row(0) << "\n";

    float result = 0;
    for (int w=0; w<eigenVectors.rows(); w++)
    {
        result += k->get()(0,w) * eigenVectors(w,pi[0].second); // permutation indices
    }
    result = (result * 0.25f - 1)*2;
    //result *= eigenvalues(pi[0].second);
    //std::cout << result << "\n";
    return result;
}

MatrixXd PCA::project(MatrixXd &dataPoints, unsigned int dimSpace)
{
    if(!k) return MatrixXd();

    int m = dataPoints.rows();
    int n = dataPoints.cols();

    switch(kernelType)
    {
    case 0:
        k = new LinearKernel();
        break;
    case 1:
        k = new PolyKernel(degree);
        break;
    case 2:
        k = new RBFKernel(gamma);
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

    MatrixXd sqrtE = MatrixXd::Zero(dimSpace, dimSpace);
    for (unsigned int i = 0; i < dimSpace; i++)
    {
        //		sqrtE(i, i) = 0.9;
    }

    for(int i=0; i < results.rows(); i++)
    {
        for(int j=0; j < results.cols(); j++)
        {
            results(i,j) *= 0.9;
        }
    }

    // get the final data projection
    //results = (sqrtE * results.transpose()).transpose();

    return results;
}


