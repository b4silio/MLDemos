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
#ifndef _FGMMPP_H_
#define _FGMMPP_H_

/**
 * ------------
 * FGMM library 
 * ------------
 *
 * a fast(er) and light Gaussian mixture model implementation. 
 *        C++ bindings 
 */


#include <cstdlib>

//extern "C" {

#include "fgmm.h"
#include "gaussian.h"

//}

/**
 * Gaussian Mixture Model class 
 */ 
class Gmm
{
public :

	/**
   * @param states : fixed number of states in the model 
   * @param dim    : dimensionnality of the space we are working in
   */

	int dim;
	int ninput;
	int nstates;
    struct gmm *c_gmm;

    Gmm() : c_gmm(0), c_reg(0), dim(0), ninput(0), nstates(0), likelihood(0){}

    Gmm(const Gmm& o)
    {
        dim = o.dim;
        ninput = o.ninput;
        nstates = o.nstates;
        likelihood = o.likelihood;
        c_reg = NULL;
        fgmm_alloc(&c_gmm, nstates, dim);
        fgmm_copy(&c_gmm, o.c_gmm);
        if(o.c_reg)
        {
            initRegression(ninput); // we don't really copy the data but redo the thing ourselves
        }
    }

    Gmm& operator=(const Gmm& o)
    {
        if(&o == this) return *this;
        if(c_gmm) fgmm_free(&c_gmm);
        dim = o.dim;
        ninput = o.ninput;
        nstates = o.nstates;
        likelihood = o.likelihood;
        c_reg = NULL;
        fgmm_alloc(&c_gmm, nstates, dim);
        fgmm_copy(&c_gmm, o.c_gmm);
        if(o.c_reg)
        {
            initRegression(ninput); // we don't really copy the data but redo the thing ourselves
        }
        return *this;
    }

	Gmm(int states, int dim)
	{
		//c_gmm = (struct gmm *) malloc(sizeof(struct gmm ));
		fgmm_alloc(&c_gmm,states,dim);
		c_reg = NULL;
		this->dim = dim;
		this->ninput = 0;
		this->nstates = states;
    }

	~Gmm()
	{
        if(c_reg != NULL) fgmm_regression_free(&c_reg);
        if(c_gmm != NULL )fgmm_free(&c_gmm);
    }

	/**
   * call this before any kind of learning .. 
   * set means of gaussians by picking random points in 
   * the dataset, and set the variance using the variance 
   * of the dataset
   *
   * @param len : # of points in the dataset
   * @param data : dim*len array, datapoints. (row order) 
   */
	void init(_fgmm_real * data,int len)
	{
		fgmm_init_random(c_gmm,data,len);
	};

	void initKmeans(_fgmm_real * data,int len)
	{
		fgmm_init_kmeans(c_gmm,data,len);
	};

	/**
  * call this before any kind of learning .. 
  * set means of gaussians by picking random points in 
  * the dataset, and set the variance using the variance 
  * of the dataset
  *
  * @param len : # of points in the dataset
  * @param data : dim*len array, datapoints. (row order) 
  */
	void init(_fgmm_real * data,int len, int initType)
	{
		switch(initType)
		{
		case 0: // random
			fgmm_init_random(c_gmm,data,len);
			break;
		case 1: // uniform
			fgmm_init_uniform(c_gmm,data,len);
			break;
		case 2: // kmeans
			fgmm_init_kmeans(c_gmm,data,len);
			break;
		}
	};


	/**
   * Just print the model's parameter on stdout
   */
	void dump()
	{
		fgmm_dump(this->c_gmm);
	};

	/**
   * Expectation Maximization Algorithm. 
   */
	int em(_fgmm_real * data,int len,
		   _fgmm_real epsilon=1e-4, enum COVARIANCE_TYPE covar_t = COVARIANCE_FULL)
	{
		return fgmm_em(c_gmm,data,len,&likelihood,epsilon,covar_t,NULL);
	};


	_fgmm_real pdf(_fgmm_real * obs, _fgmm_real * weights=NULL)
	{
		return fgmm_get_pdf(c_gmm,obs,weights);
	};

	_fgmm_real pdf(const _fgmm_real * obs, int state)
	{
		if(state >= c_gmm->nstates) return 0;
		return gaussian_pdf(&c_gmm->gauss[state], obs);
	};


	/**
   * set Prior probability for the desired state
   */
	void setPrior(int state, _fgmm_real val)
	{
		fgmm_set_prior(this->c_gmm,state,val);
	};

	/**
   * set the mean of the specified state, 
   *
   * @param state : the state index 
   * @param mean : an array of size dim, specify the mean
   */
	void setMean(int state, _fgmm_real * mean)
	{
		fgmm_set_mean(this->c_gmm,state,mean);
	};

	/**
   * set the covariance of the specified state 
   *
   * @param  state : the state index
   * @param  covar : covariance matrix 
   * @param  AsSymetric : Using symetric matrix 
   *                 order .. dim*(dim+1)/2 
   * 
   *      Symetric matrix form : 
   *         [[ 1 2 3 4 ] 
   *          [ 2 5 6 7 ]
   *          [ 3 6 8 9 ]
   *          [ 4 7 9 10]]  
   *  
   * if not we are using a standart row order . 
   */

	void setCovariance(int state, _fgmm_real * covar, bool AsSymetric=true)
	{
		if(AsSymetric)
			fgmm_set_covar_smat(this->c_gmm,state,covar);
		else
			fgmm_set_covar(this->c_gmm,state,covar);
	};


	_fgmm_real getPrior(int state)
	{
		return fgmm_get_prior(this->c_gmm,state);
	};

	void getMean(int state, _fgmm_real * output)
	{
		_fgmm_real * pMean = fgmm_get_mean(this->c_gmm,state);
		for(int i=0;i<this->c_gmm->dim;i++)
			output[i] = pMean[i];
	}

	void getCovariance(int state, _fgmm_real * out,bool AsSymetric=false)
	{
		if(!this->c_gmm) return;
		if(!AsSymetric)
		{
			fgmm_get_covar(this->c_gmm,state,out);
		}
		else
		{
			_fgmm_real * pC = fgmm_get_covar_smat(this->c_gmm,state);
			for(int i=0;i<this->c_gmm->dim*(this->c_gmm->dim+1)/2;
				i++)
				out[i] = pC[i];
		}
	}
	/**
   * draw a random sample from the model
   *
   * @param sample : the output sample, must be alloc'd of 
   *                 size dim. 
   */
	void draw(_fgmm_real * sample)
	{
		fgmm_draw_sample(this->c_gmm,sample);
	};


	/**
   * Initilization for Gaussian Mixture Regression 
   *
   * @param ninput : the first ninput dimension are the inputs, 
   *                 remaining dimensions are outputs. 
   */
	void initRegression(int ninput){
		if( c_reg != NULL)
			fgmm_regression_free(&c_reg);
		this->ninput = ninput;
		fgmm_regression_alloc_simple(&c_reg,c_gmm,ninput);
		fgmm_regression_init(c_reg);
	};

	/**
   * Perform the regression on one input point : 
   *
   * @param input : the input point (array of ninput) 
   * @param output : alloc'd array to store result. 
   * @param covar : eventually store resulting covariance is symetric matrix
   *                order (set SetCovariance ) 
   */
	void doRegression(const _fgmm_real * input, _fgmm_real * output, _fgmm_real * covar=NULL)
	{
        fgmm_regression(c_reg,input,output,covar);
	};


	/**
   * Conditional sampling from the model : 
   * draw a sample in output subspace given the input point in 
   * input subspace. 
   * you must call InitRegression before. 
   */
	void doSamplingRegression(const _fgmm_real * input, _fgmm_real * output)
	{
		fgmm_regression_sampling(c_reg,input,output);
	};


	/**
   * Online learning HIGHLY EXPERIMENTAL :: 
   * 
   * @param point : input point 
   * @param wta   : use winner take all update ( only update 
   *                 Most likely gaussian) 
   */
	void update(const _fgmm_real * point,bool wta=false)
	{
		if(wta)
			fgmm_update_wta(c_gmm,point);
		else
			fgmm_update(c_gmm,point);
	};

	/** returns state index with the highest likelihood
   */
	int getLikelyState(const _fgmm_real * point)
	{
		return fgmm_most_likely_state(this->c_gmm,point);
	};

private :
	struct fgmm_reg * c_reg;
	_fgmm_real likelihood;
};

#endif // _FGMMPP_H_
