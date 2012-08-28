
#include "SECovarianceFunction.h"
#include <QDebug>


void SECovarianceFunction::SetParams(int d, SECOVFT l[], SECOVFT sn, SECOVFT sf){
    dim=d;
    sigma_n = sn;
    sigma_f = sf;
    memcpy(lengthscales,l,dim*sizeof(SECOVFT));
//    qDebug()<<"the lengthscales:";
//    for(int i=0;i<dim;i++){
//        qDebug()<<lengthscales[i];
//    }
//    qDebug()<<"the other params:";
//
//    qDebug()<<d<<sn<<sf;

}

SECOVFT SECovarianceFunction::ComputeCovariance(SECOVFT * a, SECOVFT *b){
//    qDebug()<<a[0]<<a[1];
//    qDebug()<<b[0]<<b[1];

    SECOVFT res = 0.0;
    float diff;
    for(int i=0;i<dim;i++){
        diff = a[i] - b[i];
        res += (diff*lengthscales[i]*diff);
    }
    res *= -0.5;
    res = exp(res);
    res *= sigma_f;
//    qDebug()<<"this is the cov:";
//    qDebug()<<res;
    return res;

}
//
SECOVFT * SECovarianceFunction::ComputeCovarianceVector(SECOVFT * a,int N,SECOVFT * b){
   SECOVFT * tmp;
   SECOVFT * tmp2;
   tmp  = new SECOVFT[N];
   for(int i=0;i<N;i++){
       tmp2 = a+dim*i;
       tmp[i] = ComputeCovariance(tmp2,b);
   }
//   qDebug()<<"computing cov vector:";
   return tmp;

}
//
SECOVFT * SECovarianceFunction::ComputeCovarianceMatrix(SECOVFT * a,int N){
    SECOVFT * tmp;
    tmp = new SECOVFT[N*N];
    SECOVFT * p1;
    SECOVFT * p2;
    SECOVFT tmp2;
    for(int i=0;i<N;i++){ //row
        p1 = a + dim*i;
        for(int j=0;j<i+1;j++){ //column
            p2 = a+dim*j;
            tmp2 = ComputeCovariance(p1,p2);
            tmp[i*N+j] = tmp2;
            tmp[j*N+i] = tmp2;
        }
    }
    return tmp;
}
//
void SECovarianceFunction::ComputeCovarianceMatrix(SECOVFT * a,int N,SECOVFT * res){
    //SECOVFT * tmp;
    //tmp = new SECOVFT[N*N];
    SECOVFT * p1;
    SECOVFT * p2;
    SECOVFT tmp2;
    for(int i=0;i<N;i++){ //row
        p1 = a + dim*i;
        for(int j=0;j<i+1;j++){ //column
            p2 = a+dim*j;
            tmp2 = ComputeCovariance(p1,p2);
            res[i*N+j] = tmp2;
            res[j*N+i] = tmp2;
        }
    }
}



void SECovarianceFunction::ComputeCovarianceVector(SECOVFT * a,int N,SECOVFT * b,SECOVFT * res){
    //SECOVFT * tmp;
    SECOVFT * tmp2;
    //tmp  = new SECOVFT[N];
    for(int i=0;i<N;i++){
        tmp2 = a+dim*i;
        res[i] = ComputeCovariance(tmp2,b);
    }
 //   qDebug()<<"computing cov vector:";

 }
