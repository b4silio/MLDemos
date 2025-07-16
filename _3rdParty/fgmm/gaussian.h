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
#ifndef _GAUSSIAN_H_
#define _GAUSSIAN_H_

/* 
   FOR INTERNAL USE ONLY : this file is not part of the public API to 
   libfgmm. If you need access to model parameters, please use the 
   fgmm_get_* functions defined in fgmm.h 
*/ 

#include "smat.h"


/** One gaussian distribution */

struct gaussian{
  _fgmm_real prior; /* prior probability */
  int dim;  /* dimensionality */ 
  _fgmm_real * mean ;
  struct smat * covar; /* covariance matrix */ 
  struct smat * covar_cholesky; /* cache for cholesky decomp of covar */ 
  struct smat * icovar_cholesky; /* cholesky matrix with inverse diagonal */
  _fgmm_real nfactor; /* cache for 1. / determinant of covar */
};   

/** compute the probability density at vector value 
    
    value should be at the same dimension than g->dim */
_minline _fgmm_real gaussian_pdf(struct gaussian* g, const _fgmm_real* x)
{
  _fgmm_real dist2;
  _fgmm_real dist = smat_sesq(g->icovar_cholesky,g->mean,x);
  dist *= .5;
  dist2 =  expf(-dist)*g->nfactor;
  if(dist2 == 0) dist2 = FLT_MIN;
  return dist2;
}

/** alloc memory for the gaussian 
    and init it to zero with identity covariance matrix
*/
void gaussian_init(struct gaussian* g,int dim);
void gaussian_free(struct gaussian* g);

void invert_covar(struct gaussian* g);

void dump(struct gaussian* g);

/* draw one sample from the gaussian */
void gaussian_draw(struct gaussian* g, _fgmm_real * out);

/* get the projection of the gaussian on the given dimensions 
 * if result in NULL or wrong dimension .. is it (re) alloc'd */
void gaussian_get_subgauss(struct gaussian* g, struct gaussian* result,
			   int n_dim, int * dims);

#define ranf() ( (_fgmm_real) rand())/RAND_MAX

/** random sample from normal law ( mu = 0, sigma = 1. ) **/ 
_minline _fgmm_real randn_boxmuller( void )
{
  _fgmm_real x1, x2, w;
  do {
    x1 = 2.0 * ranf() - 1.0;
    x2 = 2.0 * ranf() - 1.0;
    w = x1 * x1 + x2 * x2;
  } while ( w >= 1.0 );

  w = sqrt( (-2.0 * log( w ) ) / w );
  x1 *= w;
  /* x2 *= w */   /* 2nd indpdt gaussian */
  return x1;
};



/** incremental mean/var update */
void gaussian_update(struct gaussian * g, 
		     const _fgmm_real * datapoint, 
		     _fgmm_real learning_rate);

#endif // _GAUSSIAN_H_
