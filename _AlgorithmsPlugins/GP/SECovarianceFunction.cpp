#include "SECovarianceFunction.h"

void SECovarianceFunction::SetParams(int d, SECOVFT l[], SECOVFT sn, SECOVFT sf){
    dim=d;
    sigma_n = sn;
    sigma_f = sf;
    memcpy(lengthscales,l,dim*sizeof(SECOVFT));
}

SECOVFT SECovarianceFunction::ComputeCovariance(const float *a, const float *b) const {
    SECOVFT res = 0.0;
    float diff;
    for(int i=0;i<dim;i++){
        diff = a[i] - b[i];
        res += (diff*lengthscales[i]*diff);
    }
    res *= -0.5;
    res = exp(res);
    res *= sigma_f;
    return res;
}
//
SECOVFT * SECovarianceFunction::ComputeCovarianceVector(const SECOVFT * a, const int N, const SECOVFT * b) const {
   SECOVFT * tmp = new SECOVFT[N];
   for(int i=0;i<N;i++){
       tmp[i] = ComputeCovariance(a + dim*i,b);
   }
   return tmp;
}
//
SECOVFT * SECovarianceFunction::ComputeCovarianceMatrix(const SECOVFT * a, const int N) const {
    SECOVFT * tmp;
    tmp = new SECOVFT[N*N];
    SECOVFT tmp2;
    for(int i=0;i<N;i++){ //row
        for(int j=0;j<i+1;j++){ //column
            tmp2 = ComputeCovariance( a + dim*i , a+dim*j);
            tmp[i*N+j] = tmp2;
            tmp[j*N+i] = tmp2;
        }
    }
    return tmp;
}
//
void SECovarianceFunction::ComputeCovarianceMatrix(const float *a, const int N, SECOVFT * res) const {
    SECOVFT tmp2;
    for(int i=0;i<N;i++){ //row
        for(int j=0;j<i+1;j++){ //column
            tmp2 = ComputeCovariance(a + dim*i,a+dim*j);
            res[i*N+j] = tmp2;
            res[j*N+i] = tmp2;
        }
    }
}



void SECovarianceFunction::ComputeCovarianceVector(const SECOVFT * a, const int N, const SECOVFT * b,SECOVFT * res) const {
    for(int i=0;i<N;i++){
        res[i] = ComputeCovariance(a+dim*i,b);
    }
 }
