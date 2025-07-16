#ifndef _MATUTIL_H_
#define _MATUTIL_H_

void PrintMat (double *mat, int idim, int jdim )  ;
void PrintStack (double *S, int N, int K) ;
void Uniform (double *vec, int T) ;
void Gaussian (double *Vec, int p) ;
void TransposeSimple(double *A, double *B, int m, int n) ;
void TransposeInPlace(double *A, int m) ;
double Norme2(double *v, int n) ;
double Trace(double *A, int n) ;
void MatMultSimple(double *A, double *B, double *C, int m, int n, int p);
void Gramm(double *R, double *A, int m, int T) ;
void ComputeAutoCum(double *Acums, double *X, int n, int T) ;

#endif // _MATUTIL_H_
