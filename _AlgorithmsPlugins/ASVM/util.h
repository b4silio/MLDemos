#ifndef _util_H_
#define _util_H_

#include <math.h>
#include <string.h>
#include <iostream>

#ifndef KILL
#define KILL(a) {if(a!=0) {delete [] a; a = 0;}}
#endif

double getkernel(double *x1, double *x2, double lambda, const char* type, int n);
bool getfirstkernelderivative(double *x1, double *x2, double lambda, const char* type, int der_wrt, double* der_val, int n);
bool getsecondkernelderivative(double *x1, double *x2, int n, double lambda, const char *type, double **hesval);


void VectorMatrixMultipy(double *VectorA, double **MatrixB, double *Result, int n, int p);
void MatrixVectorMultipy(double **MatrixB, double *VectorA, double *Result, int cols, int rows);
double arraydot(double *x, double *y, int m);
double norm(double *x, int m);
double norm2(double *x, int m);
#endif
