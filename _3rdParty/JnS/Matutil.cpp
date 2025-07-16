#include <stdio.h>
#include <stdlib.h>
// #include <math.h>


void PrintMat (double *mat, int idim, int jdim ) 
{
  int i,j ;
  for (i=0; i<idim; i++)
    { 
      for (j=0; j<jdim; j++)
	printf("%8.5f ", mat[i+j*idim] );
      printf("\n") ;
    }
  return ;
}

// A corriger pour colwise
/* Prints a stack of K matrices of size NxN */
void PrintStack (double *S, int N, int K) 
{
  int i,j,k ;
  for (k=0; k<N; k++)
    {
      printf("%3i ----------------------------------\n",k) ;
      for (i=0; i<N; i++)
	{
	  for (j=0; j<N; j++)
	    printf("%18.12f ", S[i*N+j+k*N*N] );
	  printf("\n") ;
	}
    }
  printf("-------------------------------------\n") ;
}


void Uniform (double *vec, int T)
{
  int i ;
  for (i=0;i<T;i++)
    vec[i] =  2.0 * ( (double) rand() / (double) RAND_MAX ) - 1.0  ; 
}


/*  More or less normal-randomize a vector */
void Gaussian (double *Vec, int p)
{
  int i,cpt ;
  double data ;
  for (i=0;i<p;i++)
	{
	  data = 0.0 ;
	  for (cpt=0; cpt<12; cpt++) /* you know.... */
	    data +=  (double) rand() / (double)RAND_MAX ;
	  Vec[i] = data - 6.0 ;
	}
}


/* A = B', A(nxm), B(mxn)*/
void TransposeSimple(double *A, double *B, int m, int n)
{
  int rowA ;
  int colA ;
  
  for (rowA=0; rowA<n; rowA++)
    for (colA=0; colA<m; colA++)
      A[rowA+m*colA] = 
	B[colA+n*rowA] ;
}



/* A = A' Transpose in place an m*m SQUARE matrix */
void TransposeInPlace(double *A, int m)
{
  int i,j ;
  double temp ;
  
  for (i=0; i<m; i++)
    for (j=i+1; j<m; j++)
      {
	temp = A[i*m+j];
	A[i*m+j] = A[j*m+i];
	A[j*m+i] = temp ;
      }
}



/* Squared Euclidean norm */
double Norme2(double *v, int n)
{
  double sum = 0.0 ;
  double entry  ;
  int   p ;

  for (p=0; p<n; p++)
    {
      entry = v[p] ;
      sum += entry*entry ;
    }
  return sum ;
}


/* Trace */
double Trace(double *A, int n)
{
  double sum = 0.0 ;
  int maxind = n*n ;   
  int   p ;

  for (p=0; p<maxind; p++)
    {
      sum += A[p] ;
      p += n ; /* Ah, Ah, Ah */
    }
  return sum ;
}



/* A(mxp) = B(mxn) x  C(nxp)   */
void MatMultSimple(double *A, double *B, double *C, int m, int n, int p)
{
  int rowA ;
  int colA ;
  int current ;
  double sum ;
  
  for (rowA=0; rowA<m; rowA++)
    for (colA=0; colA<p; colA++)
      {
	sum = 0.0 ;
	for (current=0; current<n; current++ )
	  sum += B[rowA+m*current]* C[current+n*colA];
	A[rowA+m*colA] = sum ;
      }
}




// A corriger pour colwise
/* R = A*A'  R: mxm  A:mxT   */
void Gramm(double *R, double *A, int m, int T)
{
  int i,j,run ;
  int istart = 0 ;
  int jstart = 0 ;
  double sum ;
  
  for (i=0; i<m; i++, istart += T)
    for (j=i, jstart=i*T; j<m; j++, jstart += T)
      {
	sum = 0.0 ;
	for (run=0; run<T; run++ )
	  sum += A[istart+run]* A[jstart+run];
	R[i*m+j] = sum ;
	R[j*m+i] = sum ;
      }
}


// A corriger pour colwise
void ComputeAutoCum(double *Acums, double *X, int n, int T) 
{
  int k,t ;
  double point, sum2, sum4  ;
  for (k=0; k<n; k++)
    {
      sum2 = 0.0 ;
      sum4 = 0.0 ;
      for (t=0; t<T; t++)
	{
	  point = X[k*T+t] ;
	  point = point*point;
	  sum2 += point ;
	  sum4 += point*point ;
	}
      sum2 = sum2 / (double) T ;
      sum4 = sum4 / (double) T ;
      Acums[k] = sum4 - 3.0*sum2*sum2 ;
    }
}

