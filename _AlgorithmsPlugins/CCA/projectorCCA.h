#ifndef PROJECTORCCA_H
#define PROJECTORCCA_H
#include <public.h>
#include <mymaths.h>
#include <projector.h>
#include <Eigen/Core>
#include <iostream>
#include <QPainter>
#include <QDebug>
#include <boost/math/distributions/chi_squared.hpp>

class ProjectorCCA : public Projector
{
    fvec w, mean;
public:
    ProjectorCCA();

public:

    void Train(std::vector< fvec > samples, ivec labels);

    fvec Project(const fvec &sample);

    const char *GetInfoString(){return "Canonical correlation analysis";}

    void setSeperatingIndex(int seperating_index){this->separating_index = seperating_index;}

    fvec &getCanonicalRoots();

    fvec &getChiSquare();

    fvec &getWilksLambda();

    fvec &getLatentRoots();

    fvec &getProbability();

    std::vector< fvec >& getSxy();

    std::vector< fvec >& getCLPS();

private:


    void cov(Eigen::MatrixXd& data,Eigen::MatrixXd& cov);

    void corr(const Eigen::MatrixXd& X,const Eigen::MatrixXd& Y,Eigen::MatrixXd& correlation);

    float corrcoef(const Eigen::VectorXd& x,const Eigen::VectorXd& y);

    void cmean(Eigen::VectorXd& mean,int dim, Eigen::MatrixXd &data);

    void sort(Eigen::VectorXd& eigenvalues, Eigen::MatrixXd& eigenvectors);

    void sort(Eigen::VectorXd& eigenvalues);

    Eigen::VectorXd std(Eigen::MatrixXd& data);

    float std(const Eigen::VectorXd &data);

    fvec convert(const Eigen::VectorXd &vector);

    void convert(const std::vector< fvec> &in, Eigen::MatrixXd &out);

    void convert(const Eigen::MatrixXd& in, std::vector< fvec > &out);

    void convet(const Eigen::MatrixXd& in,std::vector<fvec> out);

    void convert3(const std::vector< fvec >& in, Eigen::MatrixXd &out, int start_row, int end_row, int col_max);

    void combine(Eigen::MatrixXd& Z,const std::vector< fvec > &X, const std::vector< fvec > &Y);



private:

    int separating_index; ///< index marking the speration between data set X and Y
    Eigen::MatrixXd X;    ///< first data set
    Eigen::MatrixXd Y;    ///< second data set
    Eigen::MatrixXd Sxy;  ///< canonical scores
    Eigen::MatrixXd Wx;   ///< canonical weights for X
    Eigen::MatrixXd Wy;   ///< canonical weights for Y
    Eigen::VectorXd r;    ///< canonical roots
    fvec wilks_lambda;
    fvec chi_square;
    fvec latent_roots;
    fvec canonical_roots;
    fvec probability;
    std::vector< fvec > Sxy_fvec;
    std::vector< fvec > CLPS;   ///< correlation linear projected space
};

#endif // PROJECTORCCA_H
