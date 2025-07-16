/* ================================================================== */
/* 

   Implements the Jade and the Shibbs algorithms

   Copyright: JF Cardoso.  cardoso@tsi.enst.fr

   This is essentially my first C program.  Educated comments are more
   than welcome. 

   version 1.2   Jun. 05, 2002.
   Version 1.1   Jan. 20, 1999.

   Changes wrt 1.1
     o Minor fix for new versions of mex (see History)

   Changes wrt 1.0
     o Switched to Matlab-wise vectorization of matrices 
     o Merged a few subroutines into their callers
     o Implemented more C tricks to make the code more unscrutable
     o Changed the moment estimating routines to prepare for a
       read-from-file operation (the sensor loops are nested inside
       the sample loops)
     o Limited facility to control verbosity levels.  Messages directed
       to sterr.


 To do: 
   o Address the convergence problem of Shibbs on (e.g.) Gaussian data.
   o Control of convergence may/should be based on the variation of the objective
     rather than one the size of the rotations (see above item).   
   o Smarter use of floating types: short for the data, long when 
     during moment estimation (issue of error accumulation).
   o An `out of memory' should return an error code rather than exiting.

*/
/* ================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "Matutil.h"


#define VERBOSITY 0

#define RELATIVE_JD_THRESHOLD  1.0e-4
/* A `null' Jacobi rotation for joint diagonalization is smaller than
   RELATIVE_JD_THRESHOLD/sqrt(T) where T is the number of samples */

#define RELATIVE_W_THRESHOLD  1.0e-12
/* A null Jacobi rotation for the whitening is smaller than
   RELATIVE_W_THRESHOLD/sqrt(T) where T is the number of samples */


void OutOfMemory() 
{
  printf("Out of memory, sorry...\n");
  exit(EXIT_FAILURE) ;
}


#define SPACE_PER_LEVEL 3

void Message0(int level, const char *mess) {
  int count ;
  if (level < VERBOSITY) {
    for (count=0; count<level*SPACE_PER_LEVEL; count++) fprintf(stderr," ");
    fprintf(stderr, "%s", mess);
  }
}
void MessageF(int level, const char *mess, double value) {
  int count ;
  if (level < VERBOSITY) {
    for (count=0; count<level*SPACE_PER_LEVEL; count++) fprintf(stderr," ");
    fprintf(stderr, mess, value);
  }
}
void MessageI(int level, const char *mess, int value) {
  int count ;
  if (level < VERBOSITY) {
    for (count=0; count<level*SPACE_PER_LEVEL; count++) fprintf(stderr," ");
    fprintf(stderr, mess, value);
  }
}

void Identity (double *Mat, int p)
{
  int i;
  int p2 = p*p ;
  for (i=0;i<p2;i++) Mat[i]     = 0.0 ;
  for (i=0;i<p ;i++) Mat[i+i*p] = 1.0 ;
}


/* How far from identity ? Ad hoc answer */
double NonIdentity (double *Mat, int p)
{
  int i,j;
  double point ;
  double sum  = 0.0 ;
  for (i=0;i<p;i++)
    for (j=0;j<p;j++) {
      point = Mat[i*p+j] ;
      if (i!=j) sum += point*point ; 
      else      sum += (point-1.0)*(point-1.0) ; 
    }
  return sum ;
}


/* X=Trans*X : computes IN PLACE the transformation X=Trans*X.  X: nxT, Trans: nxn */
void Transform (double *X, double *Trans, int n, int T)  
{
  double *Tx ; /* buffer for a column vector */
  int i,s,t ;
  int Xind, Xstart, Xstop ;
  double sum ;

  Tx = (double *) calloc(n, sizeof(double)) ; 
  if (Tx == NULL) OutOfMemory() ;

  for (t=0; t<T; t++)
    {
      Xstart = t * n ;
      Xstop  = Xstart + n ;

      /* stores in Tx the t-th colum of X transformed by Trans */
      for (i=0; i<n ; i++) {
	sum = 0.0 ;
	for (s=i, Xind=Xstart; Xind<Xstop; s+=n, Xind++)
	  sum += Trans[s]*X[Xind] ;
	Tx[i]=sum ;
	}

      /* plugs the transformed vector back in the orignal matrix */
      for (i=0, Xind=Xstart; i< n; i++, Xind++) 
	X[Xind]=Tx[i] ;
    }
  free(Tx) ;
}


void EstCovMat(double *R, double *A, int m, int T)
{
  int i, j, t ;
  double *x ;
  double ust = 1.0 / (double) T ;

  for (i=0; i<m; i++)
    for (j=i; j<m; j++)
      R[i+j*m] = 0.0 ;
  
  for (t=0, x=A; t<T; t++, x+=m)
    for (i=0; i<m; i++)
      for (j=i; j<m; j++)
	R[i+j*m] += x[i]* x[j]; 
  
  for (i=0; i<m; i++)
    for (j=i; j<m; j++) {
      R[i+j*m] = ust * R[i+j*m] ;
      R[j+i*m] = R[i+j*m] ;
    }
}  


/* rem: does not depend on n,of course: remove the argument */
/* A(mxn) --> A(mxn) x R where R=[ c s ; -s c ]  rotates the (p,q) columns of R */
void RightRotSimple(double *A, int m, int n, int p, int q, double c, double s )
{
  double nx, ny ;
  int ix = p*m ;
  int iy = q*m ;
  int i ;
  
  for (i=0; i<m; i++) {
    nx = A[ix] ;
    ny = A[iy] ;
    A[ix++] = c*nx - s*ny ;
    A[iy++] = s*nx + c*ny ;
  }
}

/* Ak(mxn) --> Ak(mxn) x R where R rotates the (p,q) columns R =[ c s ; -s c ] 
   and Ak is the k-th M*N matrix in the stack */
void RightRotStack(double *A, int M, int N, int K, int p, int q, double c, double s ) 
{ 
  int k, ix, iy, cpt, kMN ; 
  int pM = p*M ;
  int qM = q*M ;
  double nx, ny ; 

  for (k=0, kMN=0; k<K; k++, kMN+=M*N)
    for ( cpt=0, ix=pM+kMN, iy=qM+kMN; cpt<M; cpt++) { 
      nx = A[ix] ; 
      ny = A[iy] ; 
      A[ix++] = c*nx - s*ny ; 
      A[iy++] = s*nx + c*ny ; 
    } 
}


/* 
   A(mxn) --> R * A(mxn) where R=[ c -s ; s c ]   rotates the (p,q) rows of R 
*/
void LeftRotSimple(double *A, int m, int n, int p, int q, double c, double s )
{
  int ix = p ;
  int iy = q ;
  double nx, ny ;
  int j ;
  
  for (j=0; j<n; j++, ix+=m, iy+=m) {
    nx = A[ix] ;
    ny = A[iy] ;
    A[ix] = c*nx - s*ny ;
    A[iy] = s*nx + c*ny ;
  }
}


/* 
   Ak(mxn) --> R * Ak(mxn) where R rotates the (p,q) rows R =[ c -s ; s c ]  
   and Ak is the k-th matrix in the stack
*/
void LeftRotStack(double *A, int M, int N, int K, int p, int q, double c, double s )
{
  int k, ix, iy, cpt ;
  int MN = M*N ;
  int kMN ;
  double nx, ny ;

  for (k=0, kMN=0; k<K; k++, kMN+=MN)
    for (cpt=0, ix=p+kMN, iy=q+kMN; cpt<N; cpt++, ix+=M, iy+=M) {
      nx = A[ix] ;
      ny = A[iy] ;
      A[ix] = c*nx - s*ny ;
      A[iy] = s*nx + c*ny ;
    }
}


/* Givens angle for the pair (p,q) of an mxm matrix A   */
double Givens(double *A, int m, int p, int q)
{
  double pp = A[p+m*p] ;
  double qq = A[q+m*q] ;
  double pq = A[p+m*q] ;
  double qp = A[q+m*p] ;

  if (pp>qq)
    return 0.5 * atan2(-pq-qp, pp-qq) ;
  else
    return 0.5 * atan2(pq+qp, qq-pp) ;

}

/* Givens angle for the pair (p,q) of a stack of K M*M matrices */
double GivensStack(double *A, int M, int K, int p, int q)
{
  int k ;
  double diff_on, sum_off, ton, toff ;
  double *cm ; /* A cumulant matrix  */
  double G11 = 0.0 ;
  double G12 = 0.0 ;
  double G22 = 0.0 ;
  
  int M2 = M*M ;
  int pp = p+p*M ;
  int pq = p+q*M ;
  int qp = q+p*M ;
  int qq = q+q*M ;

  for (k=0, cm=A; k<K; k++, cm+=M2) {
    diff_on = cm[pp] - cm[qq] ;
    sum_off = cm[pq] + cm[qp] ;
    
    G11 += diff_on * diff_on ;
    G22 += sum_off * sum_off ;
    G12 += diff_on * sum_off ;
  }
  ton  = G11 - G22 ;
  toff = 2.0 * G12  ;
  
  return -0.5 * atan2 ( toff , ton+sqrt(ton*ton+toff*toff) ); 

  /* there is no final minus sign in the matlab code because the
     convention for c/s in the Givens rotations is the opposite ??? */
}


/* 
   Diagonalization of an mxm matrix A by a rotation R.
*/
int Diago (double *A, double *R, int m, double threshold) 
{
  int encore = 1 ;
  int rots = 0 ;
  int p, q ;
  double theta,c,s ;

  Identity(R, m) ;

  /* Sweeps until no pair gets updated  */
  while (encore>0)  { 
    encore = 0 ;
    for (p=0; p<m; p++)
      for (q=p+1; q<m; q++) {
	theta = Givens(A,m,p,q)  ;
	if ( fabs(theta) > threshold ) {
	  c = cos(theta);
	  s = sin(theta);
	  LeftRotSimple (A, m, m, p, q, c, s) ;  
	  RightRotSimple(A, m, m, p, q, c, s) ;  
	  LeftRotSimple (R, m, m, p, q, c, s) ;  
	  encore = 1 ;
	  rots++ ;
	}
      }
  }
  return rots ;
}


/* Joint diagonalization of a stack K of symmetric M*M matrices by Jacobi */
/* returns the number of plane rotations executed */
int JointDiago (double *A, double *R, int M, int K, double threshold) 
{
  int rots = 0 ;
  int more = 1 ;
  int p, q ;
  double theta, c, s ;
  
  Identity(R, M) ;

  while ( more > 0 )   /* One sweep through a stack of K symmetric M*M matrices.  */
    {
      more = 0 ;
      for (p=0; p<M; p++)
	for (q=p+1; q<M; q++) {
	  theta = GivensStack(A,M,K,p,q)  ; 
	  if (fabs(theta)>threshold) {
	    c = cos(theta);
	    s = sin(theta);
	    LeftRotStack (A, M, M, K, p, q, c, s) ;  
	    RightRotStack(A, M, M, K, p, q, c, s) ;  
	    LeftRotSimple(R, M, M,    p, q, c, s) ;  
	    rots++ ; 
	    more = 1  ; /* any pair rotation triggers a new sweep */
	  }
	}
    }
  return rots ;
}


/* W = sqrt(inv(cov(X)))  */
void ComputeWhitener (double *W, double *X, int n, int T)  
{
  double threshold_W  = RELATIVE_W_THRESHOLD /  sqrt((double) T) ;
  double *Cov  = (double *) calloc(n*n,     sizeof(double)) ; 
  double rescale ;
  int i,j ;

  if (Cov == NULL) OutOfMemory() ;

  EstCovMat (Cov, X, n, T) ; 

  Diago (Cov, W, n, threshold_W)  ;

  for (i=0; i<n; i++) {
    rescale= 1.0 / sqrt (Cov[i+i*n]) ;
    for (j=0; j< n ; j++) 
      W[i+j*n] = rescale * W[i+j*n] ;
  }
  free(Cov);
}


/* X: nxT, C: nxnxn.  Computes a stack of n cumulant matrices.  */
void EstCumMats ( double *C, double *X, int n, int T) 
{
  double *x  ; /* pointer to a data vector in the data matrix  */
  double *tm ; /* temp matrix */
  double *R  ; /* EXX' : WE DO NOT ASSUME WHITE DATA */

  double xk2, xijkk, xij ;
  double ust = 1.0 / (float) T ; 

  int n2 = n*n ;
  int n3 = n*n*n ;
  int i,j,k,t, kdec, index ;

  Message0(3, "Memory allocation and reset...\n");
  tm = (double *) calloc(n*n, sizeof(double)) ; 
  R  = (double *) calloc(n*n, sizeof(double)) ; 

  if (tm == NULL || R == NULL) OutOfMemory() ;

  for (i=0; i<n3; i++) C[i] = 0.0 ;
  for (i=0; i<n2; i++) R[i] = 0.0 ;

  Message0(3, "Computing some moments...\n");
  for (t=0, x=X; t<T; t++, x+=n)
    {
      for (i=0; i<n; i++)   /* External product (and accumulate for the covariance)  */
	for (j=i; j<n; j++) /* We do not set the symmetric parts yet */
	  {
	    xij        = x[i]*x[j] ;
	    tm[i+j*n]  = xij  ;
	    R[i+j*n]  += xij  ;
	  }

      /* Accumulate */
      for (k=0; k<n; k++)
	{
	  xk2  = tm[k+k*n] ;       /* x_k^2 */
	  kdec = k*n2 ;            /* pre_computed shift to address the k-th matrx */
	  for (i=0; i<n; i++)
	    for (j=i, index=i+i*n; j<n; j++, index+=n)
	      C[index+kdec] += xk2 * tm[index] ; /* filling the lower part is postponed  */
	}
    }
  
  
  Message0(3, "From moments to cumulants...\n");  
  /* Normalize and symmetrize the 2th-order moments*/
  for (i=0; i<n; i++)
    for (j=i; j<n; j++)
      {
	xij      = ust * R[i+j*n] ;
	R[i+j*n] = xij  ;
	R[j+i*n] = xij  ;
      }
  
  /* from moments to cumulants and symmetrization */
  for (k=0, kdec=0; k<n; k++, kdec+=n2)
    for (i=0; i<n; i++)
      for (j=i; j<n; j++) {
	xijkk
	  = ust * C[i+j*n+kdec]       /* normalization */
	  -       R[i+j*n]*R[k+k*n]   /* cumulant correction */
	  - 2.0 * R[i+k*n]*R[j+k*n] ;
	C[i+j*n+kdec] = xijkk ;
	C[j+i*n+kdec] = xijkk ;
      }
  free(tm) ;
  free(R) ;
}

#define MC(ii,jj,kk,ll)   C[ ii + jj*n + kk*n2 + ll*n3 ]

/* X: nxT, C: nxnxnxn.  Computes the cumulant tensor.  */
void EstCumTens ( double *C, double *X, int n, int T) 
{
  int n2 = n*n ;
  int n3 = n*n*n ;
  int n4 = n*n*n*n ;
  int i,j,k,l,t ;
  double Cijkl, xi, xij, xijk, *x ;
  double ust = 1.0 / (float) T ; 

  double *R = (double *) calloc(n*n, sizeof(double)) ; 
  /* To store Cov(x).  Recomputed: no whiteness assumption here*/
  if (R == NULL) OutOfMemory() ;

  for (i=0; i<n4; i++) C[i] = 0.0 ;
  for (i=0; i<n2; i++) R[i] = 0.0 ;

  Message0(3, "Computing 2nd order cumulants...\n"); 
  /* accumulation */
  for(t=0, x=X; t<T; t++, x+=n)
    for (i=0; i<n; i++)
      for (j=i; j<n; j++)
	  R[i+j*n] += x[i] * x[j] ;
  /* normalization and symmetrization */
  for (i=0; i<n; i++)
    for (j=i; j<n; j++) {
      R[i+j*n] = ust * R[i+j*n] ;
      R[j+i*n] = R[i+j*n] ;
    }
  
  Message0(3, "Computing 4th order cumulants...\n");
  /* accumulation */
  for(t=0, x=X; t<T; t++, x+=n)
    for (i=0; i<n; i++) {
      xi = x[i] ;
      for (j=i; j<n; j++) {
	xij = xi *x[j] ;
	for (k=j; k<n; k++) {
	  xijk = xij*x[k] ;
	  for (l=k; l<n; l++) 
	    MC(i,j,k,l) += xijk*x[l];
	}
      }
    }
  /* normalization, mom2cum, and symmetrization */
  for (i=0; i<n; i++) 
    for (j=i; j<n; j++) 
      for (k=j; k<n; k++) 
	for (l=k; l<n; l++) {
	  Cijkl = ust *  MC(i,j,k,l) 
	    - R[i+j*n] * R[k+l*n]
	    - R[i+k*n] * R[j+l*n]
	    - R[i+l*n] * R[j+k*n] ;
	
	  MC(i,j,k,l)=Cijkl;MC(i,j,l,k)=Cijkl;MC(j,i,k,l)=Cijkl;MC(j,i,l,k)=Cijkl; /* ijxx  */
	  MC(i,k,j,l)=Cijkl;MC(i,k,l,j)=Cijkl;MC(k,i,j,l)=Cijkl;MC(k,i,l,j)=Cijkl; /* ikxx  */
	  MC(i,l,j,k)=Cijkl;MC(i,l,k,j)=Cijkl;MC(l,i,j,k)=Cijkl;MC(l,i,k,j)=Cijkl; /* ilxx  */
	  MC(j,k,i,l)=Cijkl;MC(j,k,l,i)=Cijkl;MC(k,j,i,l)=Cijkl;MC(k,j,l,i)=Cijkl; /* jkxx  */
	  MC(j,l,i,k)=Cijkl;MC(j,l,k,i)=Cijkl;MC(l,j,i,k)=Cijkl;MC(l,j,k,i)=Cijkl; /* jlxx  */
	  MC(k,l,i,j)=Cijkl;MC(k,l,j,i)=Cijkl;MC(l,k,i,j)=Cijkl;MC(l,k,j,i)=Cijkl; /* klxx  */
	}
  free(R) ;
}


void MeanRemoval(double *X, int n, int T)  
{
  double sum ;
  double ust = 1.0 / (double)T ;
  int i, t, tstart, tstop ;
  
  for (i=0; i<n; i++) {
      tstart = i ;
      tstop  = i + n*T ;
      sum = 0.0 ;       for (t=tstart; t<tstop ; t+=n) sum  += X[t] ;
      sum = ust * sum ; for (t=tstart; t<tstop ; t+=n) X[t] -= sum ;
  }
}

/* _________________________________________________________________  */

void Shibbs ( double *B, /* Output. Separating matrix. nbc*nbc */
	      double *X, /* Input.  Data set nbc x nbs */
	      int nbc,   /* Input.  Number of sensors  */
	      int nbs    /* Input.  Number of samples  */
	      )
{
  double threshold_JD = RELATIVE_JD_THRESHOLD / sqrt((double)nbs) ;
  
  int rots = 1 ;

  double *Transf  = (double *) calloc(nbc*nbc,         sizeof(double)) ; 
  double *CumMats = (double *) calloc(nbc*nbc*nbc,     sizeof(double)) ; 
  if ( Transf == NULL || CumMats == NULL ) OutOfMemory() ;

  /* Init */
  Message0(2, "Init...\n") ;
  Identity(B, nbc); 
  MeanRemoval(X, nbc, nbs)  ; 

  Message0(2, "Whitening...\n") ;
  ComputeWhitener(Transf, X, nbc, nbs)  ; 
  Transform (X, Transf,          nbc, nbs) ;
  Transform (B, Transf,          nbc, nbc) ;

  while (rots>0)
    {
      Message0(2, "Computing cumulant matrices...\n") ;
      EstCumMats (CumMats, X,      nbc, nbs) ;

      Message0(2, "Joint diagonalization...\n") ;
      rots = JointDiago (CumMats, Transf,  nbc, nbc, threshold_JD) ;
      MessageI(3, "Total number of plane rotations: %6i.\n", rots) ;
      MessageF(3, "Size of the total rotation: %10.7e\n", NonIdentity(Transf,nbc) );

      Message0(2, "Updating...\n") ;
      Transform  (X, Transf,        nbc, nbs ) ;
      Transform  (B, Transf,        nbc, nbc ) ;
    }
  free(Transf) ; 
  free(CumMats) ;
}

/* _________________________________________________________________  */

void Jade (
	     double *B, /* Output. Separating matrix. nbc*nbc */
	     double *X, /* Input.  Data set nbc x nbs */
	     int nbc,   /* Input.  Number of sensors  */
	     int nbs    /* Input.  Number of samples  */
	     )
{
  double threshold_JD = RELATIVE_JD_THRESHOLD / sqrt((double)nbs) ;
  int rots = 1 ;
  double *Transf  = (double *) calloc(nbc*nbc,         sizeof(double)) ; 
  double *CumTens = (double *) calloc(nbc*nbc*nbc*nbc, sizeof(double)) ; 
  if ( Transf == NULL || CumTens == NULL ) OutOfMemory() ;

  /* Init */
  Message0(2, "Init...\n") ;
  Identity(B, nbc); 

  MeanRemoval(X, nbc, nbs)  ; 

  Message0(2, "Whitening...\n") ;
  ComputeWhitener(Transf, X, nbc, nbs)  ; 
  Transform (X, Transf,          nbc, nbs) ;
  Transform (B, Transf,          nbc, nbc) ;

  Message0(2, "Estimating the cumulant tensor...\n") ;
  EstCumTens (CumTens, X,      nbc, nbs) ;

  Message0(2, "Joint diagonalization...\n") ;
  rots = JointDiago (CumTens, Transf,  nbc, nbc*nbc, threshold_JD) ;
  MessageI(3, "Total number of plane rotations: %6i.\n", rots) ;
  MessageF(3, "Size of the total rotation: %10.7e\n", NonIdentity(Transf,nbc) )  ;

  Message0(2, "Updating...\n") ;
  Transform  (X, Transf,        nbc, nbs ) ;
  Transform  (B, Transf,        nbc, nbc ) ;

  free(Transf) ; 
  free(CumTens) ;
}
