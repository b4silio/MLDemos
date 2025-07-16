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

#include <stdlib.h>
#include "fgmm.h"
#include "gaussian.h"
#include <stdio.h>


void fgmm_alloc(struct gmm ** gmm,int nstates,int dim)
{
  int i=0;
  struct gmm * gm;
  gm = (struct gmm *) malloc(sizeof(struct gmm));
  gm->nstates = nstates;
  gm->dim = dim;
  gm->gauss = (struct gaussian *) malloc(sizeof(struct gaussian) * nstates );
  
  for(i=0;i<nstates;i++)
    gaussian_init(&gm->gauss[i],dim); // this alloc memory for each gaussian 
  *gmm = gm; 
  /*
      GMM[state_i].prior = 1./N_STATES;
      for(j=0;j<DIM;j++)
	GMM[state_i].mean[j] = ((float)rand()/RAND_MAX)*2. - 1.;
	//dump(&GMM[state_i]); */
}

void fgmm_free(struct gmm ** gmm)
{
  struct gmm * gm = *gmm;
  int i=0;
  for(i=0;i<gm->nstates;i++)
    gaussian_free(&gm->gauss[i]);
  free(gm->gauss);
  free(gm);
}

void fgmm_copy(struct gmm ** gmm, struct gmm *src)
{
    struct gmm *gm = *gmm;
    int i=0;
    for(i=0;i<src->nstates;i++)
    {
        fgmm_set_covar_smat(gm, i, fgmm_get_covar_smat(src,i));
        fgmm_set_mean(gm, i, fgmm_get_mean(src, i));
        fgmm_set_prior(gm, i, fgmm_get_prior(src,i));
    }
}

  
/* associate one random data point to 
   a gaussian */
void fgmm_init_random(struct gmm * gmm,
		     const _fgmm_real * data,
		     int data_len)
{
  int state_i =0;
  int i=0;
  int point_idx=0;
  _fgmm_real * weights;
  weights = (_fgmm_real *) malloc(data_len * sizeof(_fgmm_real));
  for(i=0;i<data_len;i++)
    {
      weights[i] = 1.;
    }
 
  smat_covariance(gmm->gauss[0].covar,
		  data_len,
		  weights,
		  data,
		  gmm->gauss[0].mean);

  /*  float xx = 1./gmm->nstates;
      smat_multf(gmm->gauss[0].covar,&xx);*/

  for(;state_i < gmm->nstates;state_i++)
    {
      point_idx = rand()%data_len;
      fgmm_set_mean(gmm,state_i,&data[point_idx*gmm->dim]);
      if(state_i>0) 
	{
	  fgmm_set_covar_smat(gmm,state_i,gmm->gauss[0].covar->_);
	}
      fgmm_set_prior(gmm,state_i,1./gmm->nstates);
    }
  free(weights);
}


void fgmm_init_kmeans(struct gmm * gmm,
		     const _fgmm_real * data,
		     int data_len)
{
  int state_i =0;
  int i=0;
  int point_idx=0;

  for(;state_i < gmm->nstates;state_i++)
    {
      point_idx = rand()%data_len;
      fgmm_set_mean(gmm,state_i,&data[point_idx*gmm->dim]);
      fgmm_set_prior(gmm,state_i,1./gmm->nstates);
    }

  fgmm_kmeans(gmm, data, data_len, 1e-3, NULL);
}


/* associate data points to 
gaussians uniformly along the first dimension*/
void fgmm_init_uniform(struct gmm * gmm,
					   const _fgmm_real * data,
					   int data_len)
{
	int state_i =0;
	int i=0;
	int point_idx=0;
	_fgmm_real *weights = (_fgmm_real *) malloc(sizeof(_fgmm_real) * data_len);
	for(i=0;i<data_len;i++)
	{
		weights[i] = 1.;
	}

	smat_covariance(gmm->gauss[0].covar,
		data_len,
		weights,
		data,
		gmm->gauss[0].mean);

	/*  float xx = 1./gmm->nstates;
	smat_multf(gmm->gauss[0].covar,&xx);*/
	_fgmm_real dataMin = FLT_MAX, dataMax = -FLT_MAX;
	for(i=0; i<data_len; i++)
	{
		_fgmm_real d = data[i*gmm->dim];
		if(d>dataMax) dataMax = d;
		if(d<dataMin) dataMin = d;
	}

	for(;state_i < gmm->nstates;state_i++)
	{
		_fgmm_real dataPos = (dataMax-dataMin)*state_i/gmm->nstates + dataMin;
		int closest = 0;
		_fgmm_real closestDist = FLT_MAX;
		for(i=0; i<data_len; i++)
		{
			if(fabs(dataPos - data[i*gmm->dim]) < closestDist)
			{
				closestDist = fabs(dataPos - data[i*gmm->dim]);
				closest = i;
			}
		}

		point_idx = closest;
		fgmm_set_mean(gmm,state_i,&data[point_idx*gmm->dim]);
		if(state_i>0) 
		{
			fgmm_set_covar_smat(gmm,state_i,gmm->gauss[0].covar->_);
		}
		fgmm_set_prior(gmm,state_i,1./gmm->nstates);
	}
	free(weights);
}

int *perm(int length)
{
	if(length < 0) return NULL;
	int i=0;
	int *perm = (int *) malloc(sizeof(int) * length);
	int *usable = (int *) malloc(sizeof(int) * length);
	int uLength = length;

	for(i=0; i<length; i++)
	{
		perm[i] = 0;
		usable[i] = i;
	}

	for (int i=0; i<length; i++)
	{
		int r = 0;
		if(RAND_MAX <= 0x7fff)
			r = ((rand()<< 7) + rand()) % uLength;
		else
			r = rand() % uLength;

		perm[i] = usable[r];
		uLength--;
		usable[r] = usable[uLength];
		usable[uLength] = 0;
		if(!uLength) break;
	}
	free(usable);

	return perm;
}



void fgmm_draw_sample(struct gmm * gmm, _fgmm_real * out)
{
  int st=-1;
  _fgmm_real cumprod=0.;
  _fgmm_real v = ((_fgmm_real)rand())/RAND_MAX;
  while((cumprod < v) && ( st<(gmm->nstates-1)))
    {
      st++;
      cumprod += gmm->gauss[st].prior;
    }
  //printf("%d %f %f\n",st,v,cumprod);
  gaussian_draw(&(gmm->gauss[st]),out);
}


void fgmm_set_prior(struct gmm * gmm,int state, _fgmm_real prior)
{
  gmm->gauss[state].prior = prior;
}

_fgmm_real fgmm_get_prior(struct gmm * gmm, int state)
{
  return gmm->gauss[state].prior;
}

void fgmm_set_mean(struct gmm * gmm,int state, const _fgmm_real * mean)
{
  int i=0;
  for(;i<gmm->dim;i++)
    gmm->gauss[state].mean[i] = mean[i];
}

_fgmm_real * fgmm_get_mean(struct gmm * gmm,int state)
{
  return gmm->gauss[state].mean;
}

void fgmm_set_covar_smat(struct gmm * gmm,int state, 
			 const _fgmm_real * covar)
{
  int i=0;
  for(;i<gmm->gauss[state].covar->_size;i++)
    gmm->gauss[state].covar->_[i] = covar[i];
  invert_covar(&gmm->gauss[state]);
}


void fgmm_set_covar(struct gmm * gmm,int state, 
		    const _fgmm_real * square_covar)
{
  smat_from_square( gmm->gauss[state].covar,square_covar);
  invert_covar(&gmm->gauss[state]);
}

/* returns pointer to actual address of the covar matrix, 
   that SHALL NOT be altered ... */
_fgmm_real * fgmm_get_covar_smat(struct gmm * gmm, int state) 
{
  return gmm->gauss[state].covar->_; // arghhh 
}

/* safer here .. copy values of the covariance matrix */
void fgmm_get_covar(struct gmm * gmm, 
		    int state,
		    _fgmm_real * square_covar) /* -> must be alloc'd */ 
{
  smat_as_square(gmm->gauss[state].covar, square_covar); 
}


void fgmm_dump(struct gmm * gmm)
{
  int state_i=0;
  for(;state_i<gmm->nstates;state_i++)
    {
      printf("Gaussian %d ::\n",state_i);
      dump(&(gmm->gauss[state_i]));
    }
}
  
_fgmm_real fgmm_get_pdf( struct gmm * gmm,
		    _fgmm_real * point,
		    _fgmm_real * weights)
{
  int state_i = 0;
  _fgmm_real like=0;
  _fgmm_real p=0;
  for(;state_i<gmm->nstates;state_i++)
    {
      p = gmm->gauss[state_i].prior * gaussian_pdf(&(gmm->gauss[state_i]),point);
      if(weights != NULL)
	weights[state_i] = p;
      like += p;
    }
  return like;
}

int fgmm_most_likely_state(struct gmm * gmm,
			   const _fgmm_real * obs)
{
  int state_i = 0;
  int r=0;
  _fgmm_real max_like=0;
  _fgmm_real like;
  
  for(;state_i<gmm->nstates;state_i++)
    {
      like = gmm->gauss[state_i].prior * gaussian_pdf(&(gmm->gauss[state_i]),obs);
      if(like > max_like)
	{
	  max_like = like;
	  r = state_i;
	}
    }
  return r;
}
