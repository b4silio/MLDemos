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

/* Gaussian Mixture Regression Code */

#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <assert.h>
#include "regression.h"

void fgmm_regression_init_g(struct gaussian_reg * gr)
{
	int i,j;
	struct smat * fcov = gr->gauss->covar;
	gr->subgauss = (struct gaussian *) malloc(sizeof(struct gaussian));
	gaussian_init(gr->subgauss,gr->reg->input_len);
	gaussian_get_subgauss(gr->gauss,gr->subgauss, gr->reg->input_len, gr->reg->input_dim);
	// reg_matrix = (Sigma^00)-1 * (Sigma^0i)
	if(gr->reg_matrix != NULL)
		free(gr->reg_matrix);
	gr->reg_matrix =(_fgmm_real*)  malloc(sizeof(_fgmm_real) * gr->reg->input_len * gr->reg->output_len);

	for(j=0;j<gr->reg->output_len;j++)
    {
		for(i=0;i<gr->reg->input_len;i++)
		{
			gr->reg_matrix[j * gr->reg->input_len + i] = smat_get_value(fcov,
																		gr->reg->output_dim[j],
																		gr->reg->input_dim[i]);
		}
    }
	//dump(gr->subgauss);
}


void fgmm_regression_init(struct fgmm_reg * reg)
{
	int state=0;
	for(;state < reg->model->nstates ; state++)
    {
		fgmm_regression_init_g(&reg->subgauss[state]);
    }
}



void fgmm_regression_gaussian(struct gaussian_reg* gr, 
							  const _fgmm_real * inputs,
							  struct gaussian * result)
{
	/*_fgmm_real result[gr->output_len];*/
	int j=0,i=0;
	_fgmm_real  * tmp, * tmp2;
	_fgmm_real element;
	int off,k;

	tmp = gr->reg->vec1;
	tmp2 = gr->reg->vec2;

	/* OPT : this computation is also done for the
     subgauss pdf (ie weight of the gaussian in the regression .. */

	for(;i<gr->reg->input_len;i++)
		tmp[i] = inputs[i] - gr->subgauss->mean[i];

	smat_tforward(gr->subgauss->covar_cholesky,tmp,tmp2);
	smat_tbackward(gr->subgauss->covar_cholesky,tmp2,tmp);

	for(i=0;i<gr->reg->output_len;i++)
    {
		result->mean[i] = gr->gauss->mean[ gr->reg->output_dim[i]];
		for(j=0;j<gr->reg->input_len;j++)
		{
			result->mean[i] += gr->reg_matrix[i * gr->reg->input_len + j]*tmp[j];
		}
    }

	k=0;
	for(i=0;i<result->covar->dim;i++)
    {
		for(j=i;j<result->covar->dim;j++)
		{
			result->covar->_[k] = smat_get_value(gr->gauss->covar,
												 gr->reg->output_dim[i] ,
												 gr->reg->output_dim[j]);
			k++;
		}
    }

	for(i=0 ; i<gr->reg->output_len ; i++)
	{

		for(j=0;j<gr->reg->input_len;j++)
			tmp[j] = gr->reg_matrix[i*gr->reg->input_len+j];

		smat_tforward(gr->subgauss->covar_cholesky,tmp,tmp2);
		smat_tbackward(gr->subgauss->covar_cholesky,tmp2,tmp);

		element = 0.;
		off = 0;

		for(j=0;j<(i+1);j++)
		{
			for(k=0;k<gr->reg->input_len;k++) // scalar product here
				element += gr->reg_matrix[i*gr->reg->input_len + k]*tmp[k];
			// column wise filling ..
			result->covar->_[i+off] -= element;
			off += (gr->reg->output_len - j - 1);
		}
	}
}

/** use a fgmm_ref struct to perform regression 
 * result and covare stores resulting mean and covariance (covariance 
 * is in magic smat order .. 
 * They must be alloc'd before calling this . 
 *
 * if covar == NULL , don't compute covariance 
 */
void fgmm_regression(struct fgmm_reg * reg, 
					 const _fgmm_real * inputs, // inputs dim (reg->input_len
					 _fgmm_real * result, // outputs    (reg->output_len) /!\ alloc'd by user
					 _fgmm_real * covar)  // out covar  (reg->output_len ** 2/2)  /!\ alloc'd
{
	_fgmm_real weight2 = 0;
	_fgmm_real likelihood = 0;
	int state = 0;
	int i=0;
	if(!reg || !inputs ) return;
	for(i=0;i<reg->output_len;i++)
		result[i] = 0;

	if(covar != NULL)
    {
		for(i=0;i<reg->loc_model->covar->_size;i++)
			covar[i] = 0.;
    }

	for(;state<reg->model->nstates;state++)
    {
		reg->weights[state] = gaussian_pdf(reg->subgauss[state].subgauss,inputs);
		fgmm_regression_gaussian(&reg->subgauss[state],inputs,reg->loc_model);

		for(i=0;i<reg->output_len;i++)
			result[i] += reg->weights[state] * reg->loc_model->mean[i];
		if(covar != NULL)
		{
			for(i=0;i<reg->loc_model->covar->_size;i++)
				reg->covs[state][i] = reg->loc_model->covar->_[i];
		}
		likelihood += reg->weights[state];
    }
	if(likelihood > FLT_MIN)
	{
		if(covar != NULL)
		{
			for(state=0;state<reg->model->nstates;state++)
			{
				weight2 = reg->weights[state] / likelihood;
                weight2 *= weight2;
                for(i=0;i<reg->loc_model->covar->_size;i++)
                {
                    covar[i] += weight2 * reg->covs[state][i];
                }
			}
		}

		for(i=0;i<reg->output_len;i++)
			result[i] /= likelihood;
	}
	else
	{
		for(i=0;i<reg->output_len;i++) result[i] = 0;
	}
}


/**
 * should be like fgmm_regression_alloc() 
 *                fgmm_regression_alloc_simple()
 *
 * then fgmm_regression_init() 
 *                         _g() for single gaussian 
 */
void fgmm_regression_alloc(struct fgmm_reg ** regression,
						   struct gmm * gmm,
						   int input_len, int * input_dim,
						   int output_len, int * output_dim)
{

	struct fgmm_reg * reg;
	int i = 0;
	int state=0;

	reg = (struct fgmm_reg*) malloc(sizeof(struct fgmm_reg));
	reg->model = gmm;
	reg->input_len = input_len;
	reg->input_dim = (int*) malloc(sizeof(int)*input_len);
	for(;i<input_len;i++)
		reg->input_dim[i] = input_dim[i];
	reg->output_len = output_len;
	reg->output_dim = (int*) malloc(sizeof(int)*output_len);
	for(i=0;i<output_len;i++)
		reg->output_dim[i] = output_dim[i];

	// for holding temp results in computations ..
	reg->vec1 = (_fgmm_real *) malloc(sizeof(_fgmm_real) * input_len);
	reg->vec2 = (_fgmm_real *) malloc(sizeof(_fgmm_real) * input_len);
	reg->weights = (_fgmm_real *) malloc(sizeof(_fgmm_real) * gmm->nstates);
	reg->loc_model = (struct gaussian *) malloc(sizeof(struct gaussian));
	gaussian_init(reg->loc_model, output_len);
	reg->covs = (_fgmm_real **) malloc(sizeof(_fgmm_real*) * gmm->nstates);

	reg->subgauss = (struct gaussian_reg*) malloc(sizeof(struct gaussian_reg) * reg->model->nstates);
	for(;state < reg->model->nstates ; state++)
    {
		reg->subgauss[state].gauss = &gmm->gauss[state];
		reg->subgauss[state].reg = reg;
		reg->subgauss[state].reg_matrix = NULL;
		reg->subgauss[state].subgauss = NULL;
		reg->covs[state] = (_fgmm_real *) malloc(sizeof(_fgmm_real) * reg->loc_model->covar->_size);
    }
	*regression = reg;
}

/**
 * initialise a regression structure , considering that 
 * first input_len dimensions are input and the rest the output
 */
void fgmm_regression_alloc_simple(struct fgmm_reg ** regression,
								  struct gmm * gmm,
								  int input_len)
{
	int output_len = gmm->dim - input_len;
	int *inputs, *outs;
	int i;

	inputs = (int*) malloc(sizeof(int) * input_len);
	outs = (int*) malloc(sizeof(int) * output_len);

	for(i=0;i<input_len;i++)
    {
		inputs[i] = i;
    }
	for(i=0;i<output_len;i++)
    {
		outs[i] = input_len + i;
    }
	fgmm_regression_alloc(regression,gmm,input_len,inputs,output_len,outs);
	free(inputs);
	free(outs);
}

/*
void fgmm_regression(struct gmm * gmm, 
		    float * input,
		    float * output)
*/

void fgmm_regression_free(struct fgmm_reg ** regression)
{
	struct fgmm_reg * reg = *regression;
	int g=0;

	free(reg->input_dim);
	free(reg->output_dim);
	free(reg->vec1);
	free(reg->vec2);
	for(;g<reg->model->nstates;g++)
    {
		free(reg->covs[g]);
		if(reg->subgauss[g].reg_matrix != NULL)
			free( reg->subgauss[g].reg_matrix );
		if(reg->subgauss[g].subgauss != NULL)
		{
			gaussian_free(reg->subgauss[g].subgauss);
			free(reg->subgauss[g].subgauss);
		}
    }
	free(reg->covs);
	free(reg->weights);
	gaussian_free(reg->loc_model);
	free(reg->loc_model);
	free( reg->subgauss );
	free( reg );
	*regression = NULL;
}


/* conditionnal sampling */

void fgmm_regression_sampling(struct fgmm_reg * regression, 
							  const _fgmm_real * inputs,
							  _fgmm_real * output)
{
	_fgmm_real * weights;
	_fgmm_real nf=0;
	//float tmp[regression->output_len];
	//float likelihood = 0;
	_fgmm_real acc=0;
	int state = 0;
	struct gaussian * loc_model;
	//  int i=0;

	_fgmm_real picker = ((_fgmm_real) rand())/RAND_MAX;

	weights = (_fgmm_real *) malloc(sizeof(_fgmm_real) * regression->model->nstates);

	for(;state<regression->model->nstates;state++)
    {
		weights[state] = gaussian_pdf(regression->subgauss[state].subgauss,inputs);
		nf += weights[state];
    }

	state = 0;
	printf("%f %f \n",picker,acc);
	while(picker > acc)
    {
		acc += weights[state]/nf;
		state++;
    }
	state--;
	printf("rand state %d\n",state);
	loc_model = (struct gaussian *) malloc(sizeof(struct gaussian));
	gaussian_init(loc_model,regression->output_len);

	fgmm_regression_gaussian(&regression->subgauss[state],inputs,loc_model);

	invert_covar(loc_model);
	gaussian_draw(loc_model,output);

	gaussian_free(loc_model);
	free(loc_model);
	free(weights);
}

