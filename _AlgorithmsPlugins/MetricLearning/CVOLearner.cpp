/*
 * Genezik Framework
 * Copyright (C) 2013 Genezik, Kirell Benzi <kirell@genezik.com>
 * All rights reserved
*/

#include <iostream>
#include <cmath>
#include <limits.h>
#include "CVOLearner.h"

static const float kINITA = 10000;

namespace {

CVOLearner::CVecXf toColumnVec( const CVOLearner::fvec& in )
{
    CVOLearner::CVecXf res(in.size());
    for( size_t i = 0; i < in.size(); ++i )
        res(i) = in.at(i);
    return res;
}

CVOLearner::RVecXf toRowVec( const CVOLearner::fvec& in )
{
    CVOLearner::RVecXf res(in.size());
    for( size_t i = 0; i < in.size(); ++i )
        res(i) = in.at(i);
    return res;
}

CVOLearner::fvec fromRowVec( const CVOLearner::RVecXf& in )
{
    CVOLearner::fvec res;
    res.reserve(in.size());
    for( size_t i = 0; i < in.size(); ++i )
        res.push_back(in(i));
    return res;
}

CVOLearner::fvec fromColumnVec( const CVOLearner::CVecXf& in )
{
    CVOLearner::fvec res;
    res.reserve(in.size());
    for( size_t i = 0; i < in.size(); ++i )
        res.push_back(in(i));
    return res;
}


float normA2( const CVOLearner::fvec& xi, const CVOLearner::fvec& xj, const CVOLearner::MatrixXXf& A )
{
    // For each dimension
    float res = 0.0;
    // Sum(over N) (xn-xn)^2 * A(n,n)
    for( size_t i = 0; i < xi.size(); ++i ) {
        float t = xi.at(i) - xj.at(i);
        res += (t * t * A(i,i));
    }
    return res;
}

float normA( const CVOLearner::fvec& xi, const CVOLearner::fvec& xj, const CVOLearner::MatrixXXf& A )
{
    return float(sqrt(normA2(xi, xj, A)));
}

float normA3( const CVOLearner::fvec& xi, const CVOLearner::fvec& xj, const CVOLearner::MatrixXXf& A )
{
    float tmp = normA(xi, xj, A);
    return tmp * tmp * tmp;
}

void printMatrix( const CVOLearner::MatrixXXf& A )
{
    std::cout << A << "\n" << std::endl;
}

// Sum over S for the gradient: Sum( xi -xj )^2
CVOLearner::CVecXf sumOverS( const CVOLearner::fvecVec& S )
{
    CVOLearner::CVecXf res(S.at(0).size());
    res.setZero();

    // All pairs from input are calculated skip (0,0) and (n,n)
    size_t size = S.size();
    for( size_t i = 0; i < size - 1; ++i ) {
        for( size_t j = i+1; j < size; ++j ) {
            CVOLearner::CVecXf sub = toColumnVec(S.at(i)) - toColumnVec(S.at(j));
            res += sub.cwiseProduct(sub);
        }
    }
    return res;
}

// Denominator second part gradient
float sumOverDnormA( const CVOLearner::fvecVec& D, const CVOLearner::MatrixXXf& A )
{
    float res = 0.0;
    // All pairs from input are calculated skip (0,0) and (n,n)
    size_t size = D.size();
    for( size_t i = 0; i < size - 1; ++i ) {
        for( size_t j = i+1; j < size; ++j ) {
            res += normA(D.at(i), D.at(j), A);
        }
    }
    return res;
}

// Numerator second part gradient
CVOLearner::CVecXf sumOverDUpperExpr( const CVOLearner::fvecVec& D, const CVOLearner::MatrixXXf& A )
{
    CVOLearner::CVecXf res(D.at(0).size());
    res.setZero();

    // All pairs from input are calculated skip (0,0) and (n,n)
    size_t size = D.size();
    for( size_t i = 0; i < size - 1; ++i ) {
        for( size_t j = i+1; j < size; ++j ) {
            CVOLearner::CVecXf sub = toColumnVec(D.at(i)) - toColumnVec(D.at(j));
            sub = sub.cwiseProduct(sub); // square coeff wise
            sub /= 2 * normA(D.at(i), D.at(j), A);
            res += sub;
        }
    }
    return res;
}

CVOLearner::CVecXf gradientG( const CVOLearner::fvecVec& S, const CVOLearner::fvecVec& D, const CVOLearner::MatrixXXf& A )
{
    float vsumOverDnormA = sumOverDnormA(D, A);
    if( vsumOverDnormA == 0 ) {
        std::cerr << "gradientG: normA if 0" << std::endl;
    }
    CVOLearner::CVecXf res =  sumOverS(S) - (sumOverDUpperExpr(D,A) / vsumOverDnormA);
    return res;
}


// Hessian part
float hessianFirstPart( const CVOLearner::fvecVec& D, const CVOLearner::MatrixXXf& A, int dim )
{
    float res = 0.0;
    // All pairs from input are calculated skip (0,0) and (n,n)
    size_t size = D.size();
    for( size_t i = 0; i < size - 1; ++i ) {
        for( size_t j = i+1; j < size; ++j ) {
            float delta = D.at(i).at(dim) - D.at(j).at(dim);
            delta = delta * delta;
            res += delta / normA(D.at(i), D.at(j), A);
        }
    }
    return res;
}

float hessian2ndPart( const CVOLearner::fvecVec& D, const CVOLearner::MatrixXXf& A, int dim )
{
    float tmp = sumOverDnormA(D, A);
    return hessianFirstPart(D, A, dim) / (tmp * tmp);
}

float hessian3rdPart( const CVOLearner::fvecVec& D, const CVOLearner::MatrixXXf& A )
{
    return 1.0 / sumOverDnormA(D, A);
}

float hessian4thPart( const CVOLearner::fvecVec& D, const CVOLearner::MatrixXXf& A, int k, int l )
{
    float res = 0.0;
    // All pairs from input are calculated skip (0,0) and (n,n)
    size_t size = D.size();
    for( size_t i = 0; i < size - 1; ++i ) {
        for( size_t j = i+1; j < size; ++j ) {
            float delta = D.at(i).at(k) - D.at(j).at(k);
            delta = delta * delta;

            float delta2 = D.at(i).at(l) - D.at(j).at(l);
            delta2 = delta2 * delta2;

            res += (delta * delta2) / normA3(D.at(i), D.at(j), A);
        }
    }
    return res;
}

CVOLearner::MatrixXXf constructHessian( const CVOLearner::fvecVec& D, const CVOLearner::MatrixXXf& A )
{
    CVOLearner::MatrixXXf hessian(A.rows(), A.cols());
    hessian.setZero();
    float part3 = hessian3rdPart(D, A);
    for( int k = 0; k < A.rows(); ++k ) {
        float first = hessianFirstPart(D, A, k);
        for( int l = 0; l < A.cols(); ++l ) {
            hessian(k, l) = (0.25 * first * hessian2ndPart(D, A, l)) + (0.25 * part3 * hessian4thPart(D, A, k, l));
        }
    }
    return hessian;
}


// Add values on diagonal, force A to be > 0
void addOnDiagonal( CVOLearner::MatrixXXf& A, const CVOLearner::CVecXf& b )
{
    for( int i = 0; i < b.size(); ++i ) {
        A(i,i) += b(i);
        if( A(i,i) < 0.0 )
            A(i,i) = 0.0;
    }
}

} // end anonymous namespace

CVOLearner::CVOLearner()
    : m_method(0)
    , m_alpha(1)
    , m_steps(0)
    , m_valid(false)
{
}

CVOLearner::~CVOLearner()
{
}

void CVOLearner::reset()
{
    m_method = 0;
    m_alpha = 1.0;
    m_steps = 0;
    m_valid = false;
}

void CVOLearner::initA( int size )
{
    m_A = MatrixXXf();
    m_A.resize(size, size);
    m_A.setZero();
    // Init diagonal to 1
    for( int i = 0; i < size; ++i )
        m_A(i,i) = kINITA;
}

void CVOLearner::train( const fvecVec& similar, const fvecVec& dissimilar )
{
    if( m_method == 0 )
        trainDiagonalA(similar, dissimilar);
    else {
        std::cerr << "CVOLearner::train method not supported" << std::endl;
    }
}

void CVOLearner::trainDiagonalA( const fvecVec& S, const fvecVec& D )
{
    initA(S.at(0).size()); // Dimension of input data
    m_valid = true;
    // Perform newton method
    for( int i = 0; i < m_steps; ++i ) {
//        CVOLearner::MatrixXXf hessian = constructHessian(D, m_A);
//        if( hessian.norm() == 0 ) {
//            std::cerr << "CVOLearner::trainDiagonalA A < 0 stopping" << std::endl;
//            return;
//        }
        CVOLearner::CVecXf grad = gradientG(S, D, m_A);
        MatrixXXf tmpA = m_A;
        MatrixXXf oldA = m_A;

//        CVOLearner::MatrixXXf hessian2 = hessian.inverse();
//        addOnDiagonal(tmpA, -m_alpha * (hessian2 * grad) );
        addOnDiagonal(tmpA, -m_alpha * grad ); // gradient descent only

        if( tmpA.trace() == 0 ) {
            std::cerr << "CVOLearner::trainDiagonalA A < 0 stopping" << std::endl;
            m_A.setZero();
            return;
        }

        // Test for nan
        for( int i = 0; i < m_A.rows(); ++i ) {
            if( m_A(i,i) != m_A(i,i) ) {
                return;
            }
        }
        // Matrix is not 0
        m_A = tmpA;
        // check if modification is no longer significative
        if( (oldA - m_A).norm() < 0.00001 ) {
            std::cout << "Changes no longer matters stopping !" << std::endl;
            break;
        }
    }
}

CVOLearner::fvec CVOLearner::project( const fvec& sample )
{
    if( m_A.trace() == 0 )
        return sample;

    // Test for nan
    for( int i = 0; i < m_A.rows(); ++i ) {
        if( m_A(i,i) != m_A(i,i) ) {
            return sample;
        }
    }
    // A^1/2 * x
    return fromColumnVec( m_A.cwiseSqrt() * toColumnVec(sample) );
}
