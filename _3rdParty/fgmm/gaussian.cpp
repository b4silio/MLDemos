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

#include "gaussian.h"
//#include "smat.h"
#include <float.h>
#include <stdio.h>
#include <assert.h>


/* check the inverse covariance computation */ 
/* #define CHECK_INVERSE  */ 

//float gaussian_pdf(struct gaussian* g, const float* x)
//{
  
  
  /* dist = -.5 * (x - mu)^T Sigma^-1 (x-mu) */
  /*
  float dist = 0;
  float cdata[g->dim];
  float tmp[g->dim];
  float ivect[g->dim];

  int i=0;
  for(i=0;i<g->dim;i++)
    {
      cdata[i] = x[i] - g->mean[i];
    }

  smat_tforward(g->covar_cholesky,cdata,tmp);
  smat_tbackward(g->covar_cholesky,tmp,ivect);
  
  for(i=0;i<g->dim;i++)
    {
      dist += ivect[i]*cdata[i];
    }
  
  float dist2;
  float dist = smat_sesq(g->icovar_cholesky,g->mean,x);
  dist *= .5;
  
  dist2 =  expf(-dist)/g->nfactor;
  //dist = 0.2;
  // returning zero here would give weird results for EM 
  if( isnan(dist2))
    {
      printf("NaN gaussian pdf .. ");
      printf(" %e %e \n ", dist, g->nfactor);
    }
  if(dist2 == 0)
    dist2 = FLT_MIN;
  return dist2;
}*/


void dump(struct gaussian* g)
{
  int k=0;
  printf("  prior : %f \n",g->prior);
  printf("  mean : ");
  for(k=0;k<g->dim;k++)
    printf("%f  ",g->mean[k]);
  printf("\n");

  printf("  covariance : ");
  /*for(k=0;k<6;k++)
    printf("%f  ",g->covar[k]);*/
  smat_pmatrix(g->covar);
}

void invert_covar(struct gaussian* g)
{
  _fgmm_real det=1.;
  int i=0,j=0;
  _fgmm_real * pichol, * chol;
  if(!smat_cholesky(g->covar,g->covar_cholesky))
    {
      // g->covar is not full ranked .. adding some noise
      smat_add_diagonal(g->covar, 1.);
      // if after that the covariance is still not positive, we are into
      // big big trouble so let's just give up here.. something went horribly 
      // wrong before .. 
	  //assert(smat_cholesky(g->covar,g->covar_cholesky));
	  if(smat_cholesky(g->covar,g->covar_cholesky))
	  {
		  return;
	  }
    }
  pichol = g->icovar_cholesky->_;
  chol = g->covar_cholesky->_;

  for(i=0;i<g->dim;i++)
    {
      det *= *chol;
      *pichol = 1./(*chol);

      chol++;
      pichol++;

      for(j=i+1;j<g->dim;j++)
	{
	  *pichol++ = *chol++;
	}
    }

  det = det*det;
  g->nfactor = sqrtf( pow(M_PI,g->dim) * det);

  if(g->nfactor <= FLT_MIN)
    {
      // almost non invertible gaussian :: lets add some noise
      g->nfactor = FLT_MIN;
      smat_add_diagonal(g->covar, 1.);
      //printf("determinant :: %e\n", det);
      invert_covar(g);
      //exit(0);
    }
}

void gaussian_init(struct gaussian * g,int dim)
{
  int i;
  g->dim = dim;
  g->mean = (_fgmm_real *) malloc(dim * sizeof(_fgmm_real));
  g->covar = NULL;
  g->covar_cholesky = NULL;
  g->icovar_cholesky = NULL;
  smat_zero(&(g->icovar_cholesky),dim);
  for(i=0;i<dim;i++)
    g->mean[i] = 0.;
  smat_zero(&(g->covar),dim);
  smat_identity(g->covar); // just in case :) 
  smat_zero(&(g->covar_cholesky),dim);
  invert_covar(g);
}

void gaussian_free(struct gaussian * g)
{
  free(g->mean);
  smat_free(&g->covar);
  smat_free(&g->covar_cholesky);
  smat_free(&g->icovar_cholesky);
}
/*
void init_random(struct gaussian3d* g)
{
  int k=0;
  for(k=0;k<3;k++)
    {
      g->mean[k] = (float)rand() / RAND_MAX;
      g->covar[k] = 1.;
      g->icovar[k] = 1.;
      g->covar[k+3] = 0.;
      g->icovar[k+3] = 0.;
    }
  invert_covar(g);
  }*/
  

void gaussian_draw(struct gaussian * g, _fgmm_real * out)
{
  int i=0;
  _fgmm_real * tvec;
  tvec = (_fgmm_real *) malloc(g->dim * sizeof(_fgmm_real)); // irk, 
  for(;i<g->dim;i++)
    tvec[i] = randn_boxmuller();
  smat_multv_lt(g->covar_cholesky,tvec,out);
  for(i=0;i<g->dim;i++)
    out[i] += g->mean[i];
  free(tvec);
}

void gaussian_get_subgauss(struct gaussian* g, struct gaussian* result,
			   int n_dim, int * dims)
{
  int i=0;
  if(result->dim != n_dim)
    {
      gaussian_free(result);
      gaussian_init(result,n_dim);
    }
  smat_get_submatrix(g->covar,result->covar,n_dim,dims);  
  for(;i<n_dim;i++)
    result->mean[i] = g->mean[dims[i]];
  invert_covar(result);
}
      
void gaussian_update(struct gaussian * g,
		     const _fgmm_real * data, 
		     _fgmm_real lr)
{
  int i=0;
  int j=0;
  int curs=0;
  for(;i<g->dim;i++)
    {
      g->mean[i] += lr*(data[i] - g->mean[i]);
      for(j=i;j<g->dim;j++)
	{
	  g->covar->_[curs] += lr*( (data[i]-g->mean[i])*(data[j] - g->mean[j]) - g->covar->_[curs]) ;
	  curs++;
	}
    }
}
	    
