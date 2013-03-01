/************************************************************************/
/* This file is part of libfgmm.				        */
/* 								        */
/* libfgmm is free software: you can redistribute it and/or modify      */
/* it under the terms of the GNU Lesser General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or    */
/* (at your option) any later version.				        */
/* 								        */
/* libfgmm is distributed in the hope that it will be useful,	        */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/* GNU Lesser General Public License for more details.			        */
/* 								        */
/* You should have received a copy of the GNU Lesser General Public License    */
/* along with libfgmm.  If not, see <http://www.gnu.org/licenses/>.     */
/* 								        */
/* Copyright 2010        LASA  - EPFL   http://lasa.epfl.ch             */
/*                                                                      */
/*       Florent D'halluin   <florent.dhalluin@epfl.ch>		        */
/************************************************************************/

/* FOR INTERNAL USE ONLY, not part of public libfgmm API */ 

#ifndef _SMAT_H_
#define _SMAT_H_

/** 
    Awesome fast positive definite matrix computations .. 
 */

#define _USE_MATH_DEFINES  // force visual studio to define M_PI ... sigh .. 
#include <math.h>
#include <stdlib.h>
#include <float.h>


#define _fgmm_real float
    
/**
 * stores Symetric matrices in a efficient way : 
 *   data is stored row first order such as for a 3x3 symetrical matrix 
 *     1 2 3
 * m = 2 4 5    ->  m->_ = [ 1 2 3 4 5 6] 
 *     3 5 6
 */

#ifdef _MSC_VER  // change inline keyword into __inline if compiling with visual studio
	#define _minline __inline
	#define isnan(x) _isnan(x)
#else 
        #define _minline static inline // and gcc prefers this one .
#endif		

struct smat {
  _fgmm_real * _; /* data is actually stored here */
  int dim;   /* dimensionnality of the matrix */
  int _size; /* actual size of the data pointer dim * (dim+1) /2 */
};

/**
 * allocate memory for smat if *mat == NULL 
 * and zero the matrix in all cases 
 *
 * Use this function to allocate memory for smat structures
 */
void smat_zero(struct smat ** mat,int dim);


/**
 *  free the memory used by the matrix 
 *  (allocated by smat_zero) 
 *  *mat is set to NULL after that
 */
void smat_free(struct smat ** mat);

/**
 *  Matrix x Vector multiplication 
 *  When m is symetrical 
 *  out = m * v 
 */

_minline void smat_multv(const struct smat* m, const _fgmm_real * v,_fgmm_real * out)
{
  _fgmm_real * pcoef = m->_;
  int i,j;
  for(i=0;i<m->dim;i++)
    out[i] = 0;
  for(i=0;i<m->dim;i++)
    {
      for(j=i;j<m->dim;j++)
	{
	  out[i] += *pcoef * v[j];
	  if(j>i)
	    out[j] += *pcoef * v[i];
	  pcoef++;
	}
    }
};

/**
 *  Matrix x Vector multiplication 
 *  When m is LOWER TRIANGULAR ( eg cholesky decomp of a covariance .. ) 
 *  out = m * v 
 */

_minline void smat_multv_lt(const struct smat* m, const _fgmm_real * v,_fgmm_real * out)
{
  _fgmm_real * pcoef = m->_;
  int i,j;
  for(i=0;i<m->dim;i++)
    out[i] = 0;
  for(i=0;i<m->dim;i++)
    {
      for(j=i;j<m->dim;j++)
	{
	  out[j] += *pcoef * v[i];
	  pcoef++;
	}
    }
};

/**
 *  _in place _ Matrix x float  multiplication 
 */
_minline void smat_multf(struct smat* m,const _fgmm_real *f)
{
  int i=0;
  for(i=0;i<m->_size;i++)
    m->_[i] *= *f;
};

/**
 * get the value at row col if the matrix were stored as 
 * square matrix 
 */
_fgmm_real smat_get_value(struct smat * mat,int row,int col);

/**
 * return the symetrical matrix considering only 
 * a given subset of dimensions 
 */
void smat_get_submatrix(struct smat * mat , struct smat * res,
			int n_dims, int * dims);

/* fill the mat with identity  /!\ you must first 
 allocate memory (wigh smat_zero for ex .. ) */
void smat_identity(struct smat * mat);

/** 
 * add value on the diagonal .. (e.g. add diag noise on 
 * a gaussian .. ) 
 */
void smat_add_diagonal(struct smat * mat , _fgmm_real value );


/* print matrix to screen (for debug purposes ) */ 
void smat_pmatrix(const struct smat* mat);

/* transform a symetric ordered matrix to a square one .. */ 
void smat_as_square(const struct smat* mat, _fgmm_real *  square);
void smat_from_square(struct smat * mat, const _fgmm_real * square);

/* Cholesky decomposition 
  
   out is a UPPER triang matrix such as  out^T * out = in 
  
   returns 1 on success, 0 on failure (if the matrix is not 
   strictly positive or full ranked */

int smat_cholesky(const struct smat* in,struct smat* out);


/* L^T * L  for a triang SUP matrix, such as cholesky results .. */
void smat_ttmult(const struct smat* tri, struct smat* out);

/* 
   Using cholesky to invert systems : 

   foward resolve L*y = b  where L is * LOWER *  triangular 

   backward resolve U*y = b where U is upper triangular (cholesky results ) 


   to solve A*y = b  :

   smat_cholesky(A,U);
   smat_tforward(U,b,tmp);
   smat_tbackward(U,tmp,y);
*/ 

void smat_tforward(struct smat * lower, _fgmm_real * b, _fgmm_real * y) ;
void smat_tbackward(const struct smat * upper, _fgmm_real * b, _fgmm_real * y);

/**
 * computes sesquilinear form :
 *   (x - bias)^T Sigma^-1 (x-bias) 
 * given two vectors x and bias and the cholesky decomposition of Sigma 
 * ichol is actualy the cholesky decomposition of Sigma, where its value 
 * on the diagonal are inverted ... 
 */
_minline _fgmm_real smat_sesq(struct smat * ichol,const _fgmm_real * bias,const _fgmm_real * x)
{
  _fgmm_real out = 0.;
  int i,j;
  _fgmm_real * cdata; //[ichol->dim];
  _fgmm_real * pichol = ichol->_;
  
  cdata = (_fgmm_real *) malloc(sizeof(_fgmm_real) * ichol->dim);
  for(i=0;i<ichol->dim;i++)
    cdata[i] = 0.;      
  for(i=0;i<ichol->dim;i++)
    {
      cdata[i] += x[i] - bias[i];
      cdata[i] *= *pichol++;
      for(j=i+1;j<ichol->dim;j++)
	{
	  cdata[j] -= (*pichol++)*cdata[i];
	}
      out += cdata[i]*cdata[i];
    }
  free(cdata);
  return out;
};

/**
 * compute the weighted covariance matrix of a given dataset
 * 
 * mainly used in the maximisation step of EM 
 * weight are the weights of each element of data
 * ndata : number of elements in data (data's size is ndata*cov->dim ) 
 * mean : the computed weighted mean (must be alloc'd before , its size is cov->dim ) 
 */

_fgmm_real smat_covariance(struct smat * cov, 
		      int ndata, 
		      const _fgmm_real * weight,
		      const _fgmm_real * data,
		      _fgmm_real * mean);



_fgmm_real smat_covariance_diag(struct smat * cov, 
			   int ndata, 
			   const _fgmm_real * weight,
			   const _fgmm_real * data,
			   _fgmm_real * mean);


_fgmm_real smat_covariance_single(struct smat * cov, 
			     int ndata, 
			     const _fgmm_real * weight,
			     const _fgmm_real * data,
			     _fgmm_real * mean);

#endif /* _SMAT_H_ */
