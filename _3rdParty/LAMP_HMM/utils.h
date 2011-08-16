/* ************************************************************************ *
 * ************************************************************************ *

   File: utils.h
   Vector and array operations for HMM

  * ************************************************************************ *

   Author: Daniel DeMenthon, daniel@cfar.umd.edu
   Date:  2-18-99 

 * ************************************************************************ *

   Modification Log:
	

 * ************************************************************************ *
   Log for new ideas:
 * ************************************************************************ *
               Language and Media Processing
               Center for Automation Research
               University of Maryland
               College Park, MD  20742
 * ************************************************************************ *
 * ************************************************************************ */
 
//===============================================================================

//===============================================================================

#ifndef _UTILS

#define SUN
//#define MAC
//#define LOCAL_RAND

//===============================================================================

typedef int boolean;
// const int TRUE = 1;
// const int FALSE = 0;
#define TRUE 1
#define FALSE 0

const double gPi = 3.141592635;
typedef unsigned char uChar;

//===============================================================================

  void MyInitRand(unsigned int seed);
  int* SetIntVector(int maxElem);
  long* SetLongVector(int maxElem);
  double* SetVector(int maxElem);
  double **SetMatrix(int maxRow, int maxCol);// from 1 to maxRow and maxCol
  int **SetIntMatrix(int maxRow, int maxCol);// from 1 to maxRow and maxCol
  int** SetZeroBasedIntMatrix(int maxRow, int maxCol);
  double*** Set3DMatrix(int maxRow, int maxCol, int dim);
  int*** Set3DIntMatrix(int maxRow, int maxCol, int dim);
  void SetToZero(double **m, int maxRow, int maxCol);
  void SetToZero(int **m, int maxRow, int maxCol);
  void SetToZero(double *v, int maxElem);
  void SetToZero(int *v, int maxElem);
  void SetToEqual(double *v, int maxElem);
  void CopyArray(double **m, double **n, int maxRow, int maxCol);
  void CopyArray(double *v, double *w, int maxElem);
  void LogVect(double *v, double *w, int maxElem);
  void LogMat(double **m, double **n, int maxRow, int maxCol);
  void CumulArray(double *v, double *sum, int maxElem);
  void CumulArray(double **m, double **sum, int maxRow, int maxCol);
  double MaxElemDiff(double *v, double *w, int maxElem);
  double MaxElemDiff(double **m, double **n, int maxRow, int maxCol);
  void RescaleArray(double *v, double scale, int maxElem);
  void RescaleArray(double **m, double scale, int maxRow, int maxCol);
  void SetToRandom(double **m, int maxRow, int maxCol);
  void SetToRandom(double *v, int maxElem);
  double Sum(double *vect, int maxElem);
  double Normalize(double *v, int maxCol);
  void Normalize(double **m, int maxRow, int maxCol);
  void NormalizeRow(double *row, int maxCol);
  void NonZeroNormalizeRow(double *row, int maxCol);
  void NormalizeAllRows(double **m, int maxRow, int maxCol);
  void NonZeroNormalizeAllRows(double **m, int maxRow, int maxCol);
  void InitGaussianRows(double **m, int maxRow, int maxCol);
  double GetGauss(double x, double mean, double std);
  double GetRandomGauss(void);
  void PrintMatrix(std::ostream &outFile, int maxRow, int maxCol, double **m);
  void PrintVector(std::ostream &outFile, int maxElem, double *v);
  double MyRand(void);
  double CenteredRand(void);
  double LocalRand(void);  

//===============================================================================

#define _UTILS 
#endif
//===============================================================================
