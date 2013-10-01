/*
 * Genezik Framework
 * Copyright (C) 2013 Genezik, Kirell Benzi <kirell@genezik.com>
 * All rights reserved
*/

#ifndef CVOLEARNER_H
#define CVOLEARNER_H

#include <Eigen/Dense>
#include <vector>

class CVOLearner
{
public:
    typedef Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> MatrixXXf;
    typedef Eigen::Matrix<float, 1, Eigen::Dynamic, Eigen::RowMajor> RVecXf;
    typedef Eigen::Matrix<float, Eigen::Dynamic, 1> CVecXf;
    typedef std::vector<float> fvec;
    typedef std::vector<fvec> fvecVec;

    CVOLearner();
    ~CVOLearner();

    void setAlpha( float alpha ) { m_alpha = alpha; }
    void setSteps( int steps ) { m_steps = steps; }
    void setMethod( int method ) { m_method = method; }
    bool isValid() const { return m_valid; }
    void setIsValid( bool v ) { m_valid = v; }

    void reset();
    void train( const fvecVec& similar, const fvecVec& dissimilar );
    fvec project( const fvec& sample );

    /**
     * @brief Return learned coefficient for matrix A
     * @return
     */
    MatrixXXf coeff() const { return m_A; }

private:
    void trainDiagonalA( const fvecVec& similar, const fvecVec& dissimilar );
    void initA( int size );

private:
    int m_method; // Diagonal or Full A
    float m_alpha;
    int m_steps;
    bool m_valid;
    MatrixXXf m_A; // Matrix of coefficients
};

#endif // CVOLEARNER_H
