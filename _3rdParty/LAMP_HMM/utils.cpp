/* ************************************************************************ *
 * ************************************************************************ *

   File: utils.C
   Vector and array operations for HMM

  * ************************************************************************ *

   Author: Daniel DeMenthon, daniel@cfar.umd.edu
   Date:  2-18-99 

 * ************************************************************************ *

   Modification Log:
	5-27-99: Set log(0) to -1000, and don't use NonZeroNormalize
	Also Reset to equal probabilities if sum in a row is zero
	

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

#include <stdio.h>
#include <stdlib.h>
#include <iostream.h>
#include <iomanip.h>
#include <fstream.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "utils.h"

//===============================================================================

long idum;// seed

//===============================================================================

int* SetIntVector(int maxElem)
// vectors starting at index 1
{
  int *v;
  v = new int[maxElem+1];
  assert(v != NULL);

  return v;
}

//===============================================================================

double* SetVector(int maxElem)
// vectors starting at index 1
{
  double *v;
  v = new double[maxElem+1];
  assert(v != NULL);

  return v;
}

//===============================================================================

double** SetMatrix(int maxRow, int maxCol)
// Sets memory for matrix from 1 to maxRow and from 1 to maxCol
{
  int i;
  double **m;

  m = new double*[maxRow+1];// array of pointers
  assert(m != NULL);
  m[1] = new double[maxRow*maxCol+1];// assign space for elements
  assert(m[1] != NULL);

  for(i=2;i<=maxRow;i++){
    m[i] = m[i-1] + maxCol;// gets pointers to point to each line
  }

 return m;
 
}

//===============================================================================

int** SetIntMatrix(int maxRow, int maxCol)
// Sets memory for matrix from 1 to maxRow and from 1 to maxCol
{
  int i;
  int **m;

  m = new int*[maxRow+1];// array of pointers
  assert(m != NULL);
  m[1] = new int[maxRow*maxCol+1];// assign space for elements
  assert(m[1] != NULL);

  for(i=2;i<=maxRow;i++){
    m[i] = m[i-1] + maxCol;// gets pointers to point to each line
  }


 return m;
 
}

//===============================================================================


int** SetZeroBasedIntMatrix(int maxRow, int maxCol)
// Sets memory for matrix from 1 to maxRow and from 1 to maxCol
{
  int i;
  int **m;

  m = new int*[maxRow];// array of pointers
  assert(m != NULL);
  m[0] = new int[maxRow*maxCol];// assign space for elements
  assert(m[0] != NULL);

  for(i=1;i<maxRow;i++){
    m[i] = m[i-1] + maxCol;// gets pointers to point to each line
  }
  return m;
 }

//===============================================================================

double*** Set3DMatrix(int maxRow, int maxCol, int dim)
// Make 3D array, with third dimension used from 1 to dim
{
	int i, j, k;
	double ***m;
	double **p;
	double *pp;
	double val;

	m = new double**[maxRow];// array of pointers to rows
	
#if 1
	m[0] = new double*[maxRow*maxCol];// array of pointers to each 1D vector

	m[0][0] = new double[maxRow*maxCol*(dim+1)];// assign space for elements

	p = m[0];
	for(i=0;i<maxRow;i++){
	   m[i] = p;
	   p += maxCol;
	}

	pp = m[0][0];
	for(i=0;i<maxRow;i++){
		for(j=0;j<maxCol;j++){
			m[i][j] = pp;
			pp += dim+1;
		}
	}

//	val = 0.0;// Testing
//	for(i=0;i<maxRow;i++){
//	   for(j=0;j<maxCol;j++){
//	     for(k=1;k<=dim;k++){
//	       m[i][j][k] = val;
//	       val++;
//	     }
//	   }
//	}
#else
	//	for(i=1;i<maxRow;i++){
	//		m[i] = &m[i][0];// gets pointers to point to each line
	//	}

	for(i=0;i<maxRow;i++){
		m[i] = new double*[maxCol];
		for(j=0;j<maxCol;j++){
				m[i][j] = new double[dim+1];
		}
	}
#endif
	return m;
} 


//===============================================================================

int*** Set3DIntMatrix(int maxRow, int maxCol, int dim)
// Make 3D array, with third dimension used from 1 to dim
{
	int i, j, k;
	int ***m;
	int **p;
	int *pp;
	int val;

	m = new int**[maxRow];// array of pointers to rows

#if 1
	m[0] = new int*[maxRow*maxCol];// array of pointers to each 1D vector

	m[0][0] = new int[maxRow*maxCol*(dim+1)];// assign space for elements

	p = m[0];
	for(i=0;i<maxRow;i++){
	   m[i] = p;
	   p += maxCol;
	}

	
	pp = m[0][0];
	for(i=0;i<maxRow;i++){
	   for(j=0;j<maxCol;j++){
	      m[i][j] = pp;
	      pp += dim+1;
	   }
	}

	//	val = 0;// Testing
	//	for(i=0;i<maxRow;i++){
	//	   for(j=0;j<maxCol;j++){
	//	     for(k=1;k<=dim;k++){
	//	       m[i][j][k] = val;
	//	       val++;
	//	     }
	//	   }
	//	}
#else
	//	for(i=1;i<maxRow;i++){
	//		m[i] = &m[i][0];// gets pointers to point to each line
	//	}

	for(i=0;i<maxRow;i++){
		m[i] = new int*[maxCol];
		for(j=0;j<maxCol;j++){
				m[i][j] = new int[dim+1];
		}
	}
#endif
	return m;
} 


//===============================================================================

void SetToZero(int **m, int maxRow, int maxCol)
{
  int i, j;
  for(i=1;i<=maxRow;i++){
    for( j=1;j<=maxCol;j++){
      m[i][j] = 0;
    }
  }
}

//===============================================================================

void SetToZero(double **m, int maxRow, int maxCol)
{
  int i, j;
  for(i=1;i<=maxRow;i++){
    for( j=1;j<=maxCol;j++){
      m[i][j] = 0.0;
    }
  }
}

//===============================================================================

void SetToZero(int *v, int maxElem)

{
  int i;
  for(i=1;i<=maxElem;i++){
      v[i] = 0;
  }
}

//===============================================================================

void SetToZero(double *v, int maxElem)

{
  int i;
  for(i=1;i<=maxElem;i++){
      v[i] = 0.0;
  }
}

//===============================================================================

void SetToEqual(double *v, int maxElem)
// Set vector v to unit vector with equal elements
{
  int i;
  for(i=1;i<=maxElem;i++){
      v[i] = 1.0/maxElem;
  }
}

//===============================================================================

double MaxElemDiff(double *v, double *w, int maxElem)
// Find maximum of absolute values of differences between elements of 2 matrices
{
  int i;

  double elemDiff;
  double maxDiff = 0.0;

  for(i=1;i<=maxElem;i++){
      elemDiff = fabs(w[i] - v[i]);
      if(elemDiff>maxDiff) maxDiff = elemDiff;
  }
  return maxDiff;
}

//===============================================================================

double MaxElemDiff(double **m, double **n, int maxRow, int maxCol)
// Find maximum of absolute values of differences between elements of 2 matrices
{
  int i, j;

  double elemDiff;
  double maxDiff = 0.0;

  for(i=1;i<=maxRow;i++){
    for( j=1;j<=maxCol;j++){
      elemDiff = fabs(m[i][j] - n[i][j]);

      if(elemDiff>maxDiff) maxDiff = elemDiff;
    }
  }
  return maxDiff;
}

//===============================================================================

void CopyArray(double *v, double *w, int maxElem)
// Copy v to w
{
  int i;
  for(i=1;i<=maxElem;i++){
      w[i] = v[i];
  }
}

//===============================================================================

void CopyArray(double **m, double **n, int maxRow, int maxCol)
// Copy m to n
{
  int i, j;
  for(i=1;i<=maxRow;i++){
    for( j=1;j<=maxCol;j++){
      n[i][j] = m[i][j];
    }
  }
}

//===============================================================================

void LogVect(double *v, double *w, int maxElem)
// Copy v to w
{
  int i;
  for(i=1;i<=maxElem;i++){
    if(v[i]==0.0){
      w[i] = -1000.0;
    }
    else{
      w[i] = log(v[i]);
    }
  }
}

//===============================================================================

void LogMat(double **m, double **n, int maxRow, int maxCol)
// Copy m to n
{
  int i, j;
  for(i=1;i<=maxRow;i++){
    for( j=1;j<=maxCol;j++){
      if(m[i][j]==0.0){
	n[i][j] = -1000.0;
      }
      else{
	n[i][j] = log(m[i][j]);
      }
    }
  }
}

//===============================================================================

void CumulArray(double *v, double *sum, int maxElem)
// Copy v to w
{
  int i;
  for(i=1;i<=maxElem;i++){
      sum[i] += v[i];
  }
}

//===============================================================================

void CumulArray(double **m, double **sum, int maxRow, int maxCol)
// Copy m to n
{
  int i, j;
  for(i=1;i<=maxRow;i++){
    for( j=1;j<=maxCol;j++){
      sum[i][j] += m[i][j];
    }
  }
}

//===============================================================================

void RescaleArray(double *v, double scale, int maxElem)
// Copy v to w
{
  int i;
  for(i=1;i<=maxElem;i++){
      v[i] *= scale;
  }
}

//===============================================================================

void RescaleArray(double **m, double scale, int maxRow, int maxCol)
// Copy m to n
{
  int i, j;
  for(i=1;i<=maxRow;i++){
    for( j=1;j<=maxCol;j++){
      m[i][j] *= scale;
    }
  }
}

//===============================================================================

void SetToRandom(double **m, int maxRow, int maxCol)
{
  int i, j;
  double sum;
  
  for(i=1;i<=maxRow;i++){
    sum = 0.0;

    for( j=1;j<=maxCol;j++){
      m[i][j] = CenteredRand();
      sum += m[i][j];
    }
    for( j=1;j<=maxCol;j++){
      m[i][j] /= sum;
    }
  }
}

//===============================================================================

void SetToRandom(double *v, int maxElem)
{
  int i;
  double sum = 0.0;
  for(i=1;i<=maxElem;i++){
      v[i] = CenteredRand();
      sum += v[i];
  }
  for(i=1;i<=maxElem;i++){
      v[i] /= sum;
  }
}

//===============================================================================

void MyInitRand(unsigned int seed)
{

#ifdef SUN
  cout << "--- Using srand48 ---" << endl;
  srand48(seed);
#endif
#ifdef MAC
  cout << "--- MAC ---" << endl;
  srand(seed);
#endif
#ifdef LOCAL_RAND
  cout << "--- LOCAL ---" << endl;
  idum = long(seed);// name for Numerical Recipes' rand0
#endif
}

//===============================================================================

double CenteredRand(void)
// used for initial probabilities
{
  double randNb;

  do{
      randNb = MyRand();
  }while(randNb <0.2 || randNb>0.8);
  return randNb;
}

//===============================================================================

double MyRand(void)
{
  double randNb;

#ifdef SUN
  //     cout << "*** SUN ***" << endl;
      randNb = drand48();
#endif
#ifdef MAC
//     cout << "*** MAC ***" << endl;
     randNb = double(rand() % 100) / 100.0;// 0 to 1, 1 excluded
#endif
#ifdef LOCAL_RAND
//     cout << "*** LOCAL ***" << endl;
     randNb = LocalRand();
#endif
  return randNb;
}

//===============================================================================

double LocalRand(void)
     // From ran0(), Numerical Recipes, Second Edition p. 279
{
#define IA 16807
#define IM 2147483647
#define AM (1.0/IM)
#define IQ 127773
#define IR 2836
#define MASK 123459876

  long k;
  double ans;

  idum ^= MASK;
  k = idum/IQ;
  idum = IA*(idum-k*IQ)-IR*k;
  if(idum<0) idum += IM;
  ans = AM*idum;
  idum ^= MASK;
  return ans;
}

//===============================================================================

void InitGaussianRows(double **m, int maxRow, int maxCol)

  // Force initial B assignment from observation to states to be
  //  a diagonal assignment matrix.
{
  int i, k, diag;
  double mean;

  double std = maxCol/10.0;

  for(i=1;i<=maxRow;i++){
    for(k=1;k<=maxCol;k++){
      m[i][k] = 0.1;
    }
  }
  for(i=1;i<=maxRow;i++){
    diag = 1 + ((i-1)*(maxCol-1))/(maxRow-1);// index of most diagonal term; int division
    mean = double(diag);// get a gaussian centered on diagonal
    for(k=1;k<=maxCol;k++){
      m[i][k] = 0.001 + 0.999 * GetGauss(k, mean, std);
    }    
  }

  // Renormalize row by row:

  NormalizeAllRows(m, maxRow, maxCol);

}

//===============================================================================

void NormalizeRow(double *row, int maxCol)
{
  int k;
  double sum, resetValue;

    sum = 0.0;
    for(k=1;k<=maxCol;k++){
      sum += row[k];
    }
    if(sum==0){
      resetValue = 1.0/maxCol;
      for(k=1;k<=maxCol;k++){
	 		row[k] = resetValue;
      }
      return;
    }

    for(k=1;k<=maxCol;k++){
      row[k] /= sum;
    }
}

//===============================================================================

double Normalize(double *row, int maxCol)
{
  int k;
  double sum;

    sum = 0.0;
    for(k=1;k<=maxCol;k++){
      sum += row[k];
    }
    assert(sum != 0.0);

    for(k=1;k<=maxCol;k++){
      row[k] /= sum;
    }
    return sum;
}

//===============================================================================

double Sum(double *vect, int maxElem)
{
  int k;
  double sum;

    sum = 0.0;
    for(k=1;k<=maxElem;k++){
      sum += vect[k];
    }
    return sum;
}

//===============================================================================

void Normalize(double **m, int maxRow, int maxCol)
{
  int i, k;
  double sum;

    sum = 0.0;
    for(i=1;i<=maxRow;i++){
      for(k=1;k<=maxCol;k++){
	sum += m[i][k];
      }
    }
    assert(sum != 0.0);
    for(i=1;i<=maxRow;i++){
      for(k=1;k<=maxCol;k++){
	m[i][k] /= sum;
      }
    }
}

//===============================================================================

void NormalizeAllRows(double **m, int maxRow, int maxCol)
{
  int i;

  for(i=1;i<=maxRow;i++){
    NormalizeRow(m[i], maxCol);
  }
}

//===============================================================================

void NonZeroNormalizeRow(double *rowVect, int maxCol)
// Sum of actual values is used as resolution to which each frequency is estimated
     // Use only when vector contains number of observations
{
    int k;
    double sum, newSum, epsilon, resetValue;
    boolean isSmallTerms = FALSE;

    sum = 0.0;

    for(k=1;k<=maxCol;k++){
      sum += rowVect[k];
    }
    
    if(sum==0){
      resetValue = 1.0/maxCol;
      for(k=1;k<=maxCol;k++){
			rowVect[k] = resetValue;
      }
      return;
    }

    epsilon = 1.0/sum;

    for(k=1;k<=maxCol;k++){
      if(rowVect[k]<epsilon){
			isSmallTerms = TRUE;
			break;
      }
    }

    if(isSmallTerms){
      newSum = sum + maxCol * epsilon;
      for(k=1;k<=maxCol;k++){
			rowVect[k] = (rowVect[k]+epsilon)/newSum;
      }
    }
    else{
      for(k=1;k<=maxCol;k++){
			rowVect[k] /= sum;// Plain normalization
      }
    }
}

//===============================================================================

void NonZeroNormalizeAllRows(double **m, int maxRow, int maxCol)
     // Use only when matrix contains number of observations
{
  int i;

  for(i=1;i<=maxRow;i++){
    NonZeroNormalizeRow(m[i], maxCol);
  }
}

//===============================================================================

double GetGauss(double x, double mean, double stdev)
{
  assert(stdev != 0.0);
  double offsetVar = (x - mean)/stdev;
  if(offsetVar > 30.0 || offsetVar < -30.0) return 0.0;

  double exponent = -(offsetVar * offsetVar) / 2.0;
  double sqrt2Pi = 2.506628274631;
  double denom = sqrt2Pi * stdev;

  double gauss;

  gauss = exp(exponent)/denom;

  return gauss;
}

//===============================================================================

double GetRandomGauss(void)
// Get random deviate with normal distribution
// Zero mean, unit variance
// Numerical Recipes, Second Edition p. 299, from Knuth
{
  static int iset = 0; // flag
  static double gset;
  double v1,v2, sq, fac;

  if(iset==0){
    do{
	v1 = 2.0 * MyRand() - 1.0;
	v2 = 2.0 * MyRand() - 1.0;
	sq = v1*v1 + v2*v2;
    }while(sq>=1.0 || sq==0.0);

    fac = sqrt(-2.0*log(sq)/sq);
    gset=v1*fac;
    iset = 1;
    return v2*fac;
  }
  else{
    iset = 0;
    return gset;
  }
}

//===============================================================================

void PrintMatrix(ostream &outFile, int maxRow, int maxCol, double **m)
{
        int i, j;

	outFile.setf(ios::showpoint);
	outFile.setf(ios::fixed, ios::floatfield);
	outFile.precision(10);
        for (i = 1; i <= maxRow; i++) {
                for (j = 1; j <= maxCol; j++) {
                       outFile << m[i][j] << " ";
		}
		outFile << endl;
	}
}

//===============================================================================

void PrintVector(ostream &outFile, int maxElem, double *v)
{
        int i;
	outFile.setf(ios::showpoint);
	outFile.precision(10);

        for (i = 1; i <= maxElem; i++) {
               outFile << v[i] << " ";
	}
	outFile << endl;
}

//===============================================================================
//===============================================================================
//===============================================================================

