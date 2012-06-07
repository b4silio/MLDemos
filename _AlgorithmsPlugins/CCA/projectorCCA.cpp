#include "projectorCCA.h"
#include <mymaths.h>
#include <Eigen/Core>
#include <Eigen/Eigen>
#include <Eigen/LU>
#include <QDebug>
#include <math.h>
#include <map>

#include <iostream>

using namespace std;
using namespace Eigen;

ProjectorCCA::ProjectorCCA()
{
    separating_index = 0;
}


void ProjectorCCA::Train(std::vector< fvec > samples, ivec labels)
{


    projected.clear();
    source.clear();
    if(!samples.size()) return;
    source = samples;
    projected = samples;

    int M = samples.size();
    dim = samples[0].size();
    if(separating_index == 0 || separating_index >= dim) separating_index = max(1, (int)dim/2);
    int q = separating_index;
    int p = dim - q;

    X.setZero(q,M);
    Y.setZero(p,M);
    MatrixXd zz(q+p,M);
    MatrixXd z(q+p,M);
    MatrixXd C(q+p,q+p);
    MatrixXd Cxx(q,q);
    MatrixXd Cyy(p,p);
    MatrixXd Cxy(q,p);
    MatrixXd Cyx(p,q);
    MatrixXd I1(q,q);
    MatrixXd I2(p,p);
    I1.setIdentity();
    I2.setIdentity();
    C.setZero();
    z.setZero();
    Cxx.setZero();

    //read in data
    convert3(samples,X,0,q,M);
    convert3(samples,Y,q,q+p,M);

    z.block(0,0,X.rows(),zz.cols()) = X;
    z.block(X.rows(),0,Y.rows(),zz.cols()) = Y;
    z.transposeInPlace();
    cov(z,C);

    Cxx = C.block(0,0,q,q) + (powf(10,-8))*I1;
    Cyy = C.block(q,q,p,p) + (powf(10,-8))*I2;
    Cxy = C.block(0,q,q,p);
    Cyx = Cxy.transpose();

    MatrixXd Rx = (Cxx.llt()).matrixU();
    MatrixXd invRx = Rx.inverse();
    MatrixXd invCyy = Cyy.inverse();
    MatrixXd Z = invRx.transpose() * Cxy * invCyy * Cyx * invRx;
    Z = 0.5*(Z.transpose() + Z);
    EigenSolver<MatrixXd> eig_solver(Z);

    VectorXcd eig_values = eig_solver.eigenvalues();
    MatrixXcd eig_vectors = eig_solver.eigenvectors();


    VectorXd sqrt_eig = (eig_values.real()).cwiseSqrt();
    MatrixXd eig_vec_real = eig_vectors.real();

    MatrixXd wx = invRx * eig_vec_real;

    MatrixXd wy = (invCyy * Cyx * wx);
    MatrixXd R;
    R.setOnes(wy.rows(),wy.cols());
    for(unsigned int i = 0; i < R.cols();i++){
        R.col(i) = R.col(i) * sqrt_eig(i);
    }
    wy = wy.cwiseQuotient(R);


    Wx = wx.block(0,0,wx.rows(),min(q,p));
    Wy = wy.block(0,0,wy.rows(),min(q,p));
    r = sqrt_eig.segment(0,min(q,p));
    sort(r,Wx);
    sort(r,Wy);
    sort(r);

    MatrixXd Sx = Wx.transpose()*X;
    MatrixXd Sy = Wy.transpose()*Y;

    Sx.transposeInPlace();
    Sy.transposeInPlace();

    Sxy.setZero(M,Sx.cols() + Sy.cols());

    int sx_index = 0;
    int sy_index = 0;
    FOR(i,Sxy.cols()){
        if(i%2 == 0){
            Sxy.col(i) = Sx.col(sx_index);
            sx_index++;
        }else{
            Sxy.col(i) = Sy.col(sy_index);
            sy_index++;
        }
    }

    projected.resize(Sxy.rows());
    FOR(i, Sxy.rows())
    {
        projected[i].resize(Sxy.cols());
        FOR(d, Sxy.cols()) projected[i][d] = Sxy(i,d);
    }


    MatrixXd clps(Sx.cols(),Sy.cols());
    clps.setZero();
    corr(Sx,Sy,clps);
    convert(clps,CLPS);

    double wilks = 1;
    double chi = 0;
    double latent = 0;
    int dof;
    probability.clear();
    chi_square.clear();
    wilks_lambda.clear();
    latent_roots.clear();
    canonical_roots.clear();
    FOR(i,r.size())
    {
        wilks = 1;
        chi=0;
        latent = r(i)*r(i);
        FOR(j,r.size() - i)
        {
            if(latent == 1){
                 wilks = wilks*(1 - 0.999); // to avoid -inf results in chi column
            }else{
               wilks = wilks*(1 - latent);
            }
        }
        dof = (p - i)*(q - i);
        chi = -( (M - 1) - 0.5*(p + q + 1))*log(wilks);
        boost::math::chi_squared distribution(dof);
        probability.push_back(1 - boost::math::cdf(distribution,chi));
        chi_square.push_back(chi);
        wilks_lambda.push_back(wilks);
        latent_roots.push_back(latent);
    }


    FOR(i,r.size())
    {
       canonical_roots.push_back(r(i));
    }
}



fvec ProjectorCCA::Project(const fvec &sample)
{


    int q = separating_index;
    int p = sample.size() - q;
    Eigen::MatrixXd x;
    Eigen::MatrixXd y;
    x.setZero(q,1);
    y.setZero(p,1);

    FOR(i, q)
    {
        x(i,0) = sample[i];
    }
    FOR(i, p)
    {
        y(i,0) = sample[q + i];
    }


    MatrixXd newX = Wx.transpose()*x;
    MatrixXd newY = Wy.transpose()*y;

    fvec newSample(min(q,p)*2);
    FOR(i, min(p,q))
    {

        newSample[2*i] = newX(i,0);
        newSample[2*i+1] = newY(i,0);
    }

    return newSample;

}

std::vector< fvec >& ProjectorCCA::getCLPS(){
    return CLPS;
}

fvec &ProjectorCCA::getChiSquare(){
    return chi_square;
}

fvec &ProjectorCCA::getWilksLambda(){
    return wilks_lambda;
}

fvec &ProjectorCCA::getLatentRoots(){
    return latent_roots;
}

fvec &ProjectorCCA::getCanonicalRoots(){
    return canonical_roots;
}

fvec &ProjectorCCA::getProbability(){
    return probability;
}

std::vector<fvec> &ProjectorCCA::getSxy(){
    convert(Sxy,Sxy_fvec);
    return Sxy_fvec;
}

void ProjectorCCA::corr(const Eigen::MatrixXd &X, const Eigen::MatrixXd &Y, Eigen::MatrixXd& correlation){


    int P1 = X.cols();
    int P2 = Y.cols();

    FOR(i,P1)
    {
        FOR(j,P2)
        {
            correlation(i,j) = corrcoef(X.col(i),Y.col(j));

        }
    }


}

float ProjectorCCA::corrcoef(const Eigen::VectorXd &x, const Eigen::VectorXd &y){
    float mean_x = x.mean();
    float mean_y = y.mean();

    float sd_x = 0;
    float sd_y = 0;
    float r = 0;

    FOR(i,x.size())
    {
        r = r + (x(i) - mean_x)*(y(i) - mean_y);
        sd_x = sd_x +  (x(i) - mean_x)*(x(i) - mean_x);
        sd_y = sd_y +  (y(i) - mean_y)*(y(i) - mean_y);
    }

    r = r/(std::sqrt(sd_x)*std::sqrt(sd_y));

    return r;
}

void ProjectorCCA::sort(Eigen::VectorXd &eigenvalues, Eigen::MatrixXd &eigenvectors){

    std::vector<std::pair<float,int> > sort_eig(eigenvalues.size());

    FOR(i,eigenvalues.size())
    {
        sort_eig.push_back(std::pair<float,int>(eigenvalues(i),i));
    }
    std::sort(sort_eig.begin(),sort_eig.end(),std::greater<std::pair<float,int> >());

    MatrixXd eig_vec(eigenvectors.rows(),eigenvectors.cols());
    eig_vec.setZero();

    FOR(i,eig_vec.cols())
    {
        eig_vec.col(i) = eigenvectors.col(sort_eig[i].second);

    }

    eigenvectors = eig_vec;
}

void ProjectorCCA::sort(Eigen::VectorXd &eigenvalues){

    std::vector< float > sort_eig(eigenvalues.size());

    FOR(i,eigenvalues.size())
    {
        sort_eig[i] = eigenvalues(i);
    }
    std::sort(sort_eig.begin(),sort_eig.end(),std::greater<float>());

    FOR(i,eigenvalues.size()){
        eigenvalues(i) = sort_eig[i];
    }
}

void ProjectorCCA::cov(MatrixXd &data,MatrixXd &cov){
    int n = data.rows();
    VectorXd mean(data.cols());
    VectorXd v;
    cmean(mean,1,data);
    for(unsigned int i = 0; i < data.rows();i++){
        v = (data.row(i).transpose() - mean);
        cov = cov +  v * v.transpose();
    }
    cov = cov/(n-1);
}

void ProjectorCCA::cmean(VectorXd &mean,int dim, MatrixXd &data){

    if(dim == 1){ // mean of cols
        assert(mean.size() == data.cols());
        mean = data.colwise().mean();
    }else{ // mean of row
        assert(mean.size() == data.rows());
        mean = data.rowwise().mean();

    }



}

VectorXd ProjectorCCA::std(Eigen::MatrixXd &data){

    VectorXd mean(data.cols());
    VectorXd sd(data.cols());
        VectorXd sd2(data.cols());
        sd.setZero();

    cmean(mean,1,data);
    double N = data.rows();
    for(unsigned int j = 0; j < data.cols();j++){
        for(unsigned int i = 0; i < data.rows();i++){
            sd(j) = sd(j) + std::pow(data(i,j) - mean(j),2);
        }
        sd(j) = std::sqrt((1/(N-1))*sd(j));
    }


    return sd;
}

float ProjectorCCA::std(const Eigen::VectorXd &data){
    float m = data.mean();
    float sd = 0;
    float N = data.size();
    FOR(i,data.size())
    {
        sd = sd + std::pow(data(i) - m,2);

    }

    sd = std::sqrt((1/(N - 1)) * sd);

    return sd;
}

fvec ProjectorCCA::convert(const Eigen::VectorXd &vector){
    fvec vec(vector.size());
    FOR(i,vector.size()){
        vec[i] = vector(i);
    }
    return vec;
}

void ProjectorCCA::convert(const std::vector< fvec> &in, MatrixXd &out){
    for(unsigned int i = 0; i < in.size();i++){
        for(unsigned int j = 0; j < in[0].size();j++){
            out(i,j) = in[i][j];
        }
    }

}

void ProjectorCCA::convert(const Eigen::MatrixXd& in, std::vector< fvec > &out){

    out.clear();
    FOR(i,in.rows())
    {
        out.push_back(convert(in.row(i)));
    }


}

void ProjectorCCA::convert3(const vector< fvec >& in, Eigen::MatrixXd &out, int start_row, int end_row, int col_max){

    int index = 0;
    for(unsigned int i = start_row; i < end_row;i++){
        for(unsigned int j = 0; j < col_max;j++){
            out(index,j) = in[j][i];

        }
        index++;
    }
}

void ProjectorCCA::combine(MatrixXd &Z, const std::vector< fvec > &X, const std::vector< fvec > &Y){

    int p = X[0].size();
    int q = Y[0].size();
    int M = X.size();

    for(unsigned int d = 0; d < p;d++){
        for(unsigned int i = 0; i < M; i++){
            Z(d,i) = X[i][d];
        }
    }

    for(unsigned int d = 0; d < q;d++){
        for(unsigned int i = 0; i < M; i++){
            Z(d,i) = X[i][d];
        }
    }

}
