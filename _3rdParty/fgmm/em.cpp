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
#include <math.h> // isinf , isnan 
#include <float.h>
#include <stdio.h>
#include <assert.h>

#define max_iter 100

/**
 * for all data compute p(x|i) prob that state i generated data point x
 * correspond to the E step of EM.
 *
 * @param *pix is an alloc'd float table of dimension nstates*data_len
 * returns total log_likelihood
 */
_fgmm_real fgmm_e_step(struct gmm * GMM,
                       const _fgmm_real * data,
                       int data_len,
                       _fgmm_real * pix)
{
    /* E step */
    _fgmm_real log_lik=0;
    _fgmm_real like;
    _fgmm_real * pxi;
    int data_i=0;
    int state_i;

    pxi = (_fgmm_real *) malloc(sizeof(_fgmm_real) * GMM->nstates);
    for(;data_i<data_len;data_i++)
    {
        like=0;
        for(state_i=0;state_i<GMM->nstates;state_i++)
        {
            pxi[state_i] = gaussian_pdf(&GMM->gauss[state_i], data + data_i*GMM->dim) ;

            //printf("state %d -> lik : %f\n",state_i,pxi[state_i]);
            like += pxi[state_i]* GMM->gauss[state_i].prior;
            /*
            pdata++;
            ppxi++;
            */
        }
        if(like<= FLT_MIN)
        {
            //printf("too far from current distrib %d\n",data_i);
            // exit(0);
        }
        else log_lik += log(like);
        /*
        if(isnan(log_lik) || isinf(log_lik))
        exit(0);
        */
        for(state_i=0;state_i<GMM->nstates;state_i++)
        {
            pix[data_i + state_i*data_len] = pxi[state_i] * GMM->gauss[state_i].prior / like;
            if(pix[data_i + state_i*data_len] <= FLT_MIN)
                pix[data_i + state_i*data_len] = FLT_MIN;

        }

    }
    free(pxi);
    return log_lik;
}

/** updates the mean and covariances of the model, 
    given the data and the probabilty of each point to
    be generated from each state (pix)

    reestimate_flag is set to one if we need to do another round
                    (mainly when a cluster was empty)
    covar_t         sets the covariance type (diag, sphere of full )
*/

void fgmm_m_step(struct gmm * GMM,
                 const _fgmm_real * data,
                 int data_len,
                 _fgmm_real * pix,
                 int * reestimate_flag,
                 enum COVARIANCE_TYPE covar_t)
{
    int state_i,k;
    int random_point = 0;
    for(state_i=0;state_i<GMM->nstates;state_i++)
    {
        GMM->gauss[state_i].prior = 0;
        for(k=0;k<GMM->dim;k++)
            GMM->gauss[state_i].mean[k] = 0;

        switch(covar_t)
        {
        case COVARIANCE_DIAG :
            GMM->gauss[state_i].prior = smat_covariance_diag(GMM->gauss[state_i].covar,
                                                             data_len,
                                                             &pix[state_i*data_len],
                                                             data,
                                                             GMM->gauss[state_i].mean);
            break;

        case COVARIANCE_SPHERE :
            GMM->gauss[state_i].prior = smat_covariance_single(GMM->gauss[state_i].covar,
                                                               data_len,
                                                               &pix[state_i*data_len],
                                                               data,
                                                               GMM->gauss[state_i].mean);
            break;

        default :
            GMM->gauss[state_i].prior = smat_covariance(GMM->gauss[state_i].covar,
                                                        data_len,
                                                        &pix[state_i*data_len],
                                                        data,
                                                        GMM->gauss[state_i].mean);
            break;
        }


        // If no point belong to us, reassign to a random one ..
        if(GMM->gauss[state_i].prior == 0)
        {
            random_point = rand()%data_len;
            for(k=0;k<GMM->dim;k++)
                GMM->gauss[state_i].mean[k] = data[random_point*GMM->dim + k];
            *reestimate_flag = 1; // then we shall restimate mean/covar of this cluster
        }
        else
        {
            GMM->gauss[state_i].prior /= data_len;
            invert_covar(&GMM->gauss[state_i]);
        }
    }


}

/** perform em on the giver data
 * @param data : the given dataset (data_length*3 floats)
 *               /!\ aligned malloc'd float *
 * @param num_states : number of states of the GMM
 * @return  # of iterations
 */
int fgmm_em( struct gmm * GMM,
             const _fgmm_real * data,
             int data_length,
             _fgmm_real * end_loglikelihood,
             _fgmm_real likelihood_epsilon,
             enum COVARIANCE_TYPE covar_t,
             const _fgmm_real * weights) // if not NULL, weighted version ..
{
    _fgmm_real * pix;
    _fgmm_real log_lik;
    int niter=0;
    _fgmm_real oldlik=0;
    _fgmm_real deltalik=0;
    int state_i;
    int d=0;
    int reestimate_flag=0; // shall we do one more iteration ??

    pix = (_fgmm_real *) malloc( sizeof(_fgmm_real) * data_length * GMM->nstates);

    for(state_i=0;state_i<GMM->nstates;state_i++)
    {
        invert_covar(&GMM->gauss[state_i]);
    }


    for(niter=0;niter<max_iter;niter++)
    {
        reestimate_flag = 0;
        log_lik = fgmm_e_step(GMM,data,data_length,pix);
        log_lik/=data_length;
#ifndef NDEBUG
        //printf("Log lik :: %f \n",log_lik);
#endif
        // M step
        deltalik = log_lik - oldlik;
        oldlik = log_lik;

        if(fabs(deltalik) < likelihood_epsilon && !reestimate_flag)
            break;

        if(weights != NULL)
        {
            for(d=0;d<data_length;d++)
            {
                for(state_i=0;state_i< GMM->nstates; state_i++)
                    pix[d*GMM->nstates + state_i] *= weights[d];

            }
        }

        fgmm_m_step(GMM,data,data_length,pix,&reestimate_flag,covar_t);
        //      pdata = data;
    }
    if(end_loglikelihood != NULL)
        *end_loglikelihood = log_lik;

    free(pix);
    return niter;
}


/**
 * this is the k-means estimation steps :
 *
 * works the same way than normal e_step, just that
 * pix are computed as 0 and 1, given if the current
 * data points belong or not to the cluster...
 *
 * returns total distance to all clusters.
 */

_fgmm_real fgmm_kmeans_e_step(struct gmm * GMM,
                              const _fgmm_real * data,
                              int data_len,
                              _fgmm_real * pix)
{
    /* E step */
    _fgmm_real total_distance=0;
    _fgmm_real distance;
    _fgmm_real max_distance;
    int cstate = 0;
    int data_i=0;
    int state_i;
    int _;
    const _fgmm_real * pdata = data;

    for(;data_i<data_len;data_i++)
    {
        max_distance = FLT_MAX;
        cstate = -1;
        for(state_i=0;state_i<GMM->nstates;state_i++)
        {
            distance = 0;
            for(_ = 0; _ < GMM->dim ; _++)
            {
                distance += (*pdata - GMM->gauss[state_i].mean[_]) * (*pdata - GMM->gauss[state_i].mean[_]);
                pdata++;
            }
            // printf("%d %f \n", state_i, distance);
            pdata -= GMM->dim;
            if( distance < max_distance)
            {
                cstate = state_i;
                max_distance = distance;
            }
        }
        pdata += GMM->dim;
        if(cstate == -1) cstate = 0;
        assert(cstate != -1);
        // fucking unreadable but this sets pix[.. , state_i] to
        // 0 unless state_i is cstate
        //      printf(" cstate : %d \n",cstate);
        for(state_i=0;state_i<GMM->nstates;state_i++)
        {
            pix[data_i + state_i*data_len] = 0;
            if(state_i == cstate)
                pix[data_i + state_i*data_len] = 1.;
        }


        total_distance += max_distance;
    }
    return total_distance;
}

/* do kmeans , reusing lots of code .. */ 

int fgmm_kmeans( struct gmm * GMM,
                 const _fgmm_real * data,
                 int data_length,
                 _fgmm_real likelihood_epsilon,
                 const _fgmm_real * weights) // if not NULL, weighted version ..
{
    _fgmm_real * pix;
    _fgmm_real total_distance;
    int niter=0;
    _fgmm_real oldlik=0;
    _fgmm_real deltalik=0;
    int state_i;
    int d=0;
    int reestimate_flag = 0;

    pix = (_fgmm_real *) malloc( sizeof(_fgmm_real) * data_length * GMM->nstates);

    for(state_i=0;state_i<GMM->nstates;state_i++)
    {
        invert_covar(&GMM->gauss[state_i]);
    }


    for(niter=0;niter<max_iter;niter++)
    {
        reestimate_flag = 0;
        total_distance = fgmm_kmeans_e_step(GMM,data,data_length,pix);
        total_distance/=data_length;
#ifndef NDEBUG
        //printf("Kmeans distance :: %f \n",total_distance);
#endif
        // M step
        deltalik = total_distance - oldlik;
        oldlik = total_distance;

        if(fabs(deltalik) < likelihood_epsilon && !reestimate_flag)
            break;

        if(weights != NULL)
        {
            for(d=0;d<data_length;d++)
            {
                for(state_i=0;state_i< GMM->nstates; state_i++)
                    pix[d + state_i*data_length] *= weights[d];

            }
        }
        // the song remains the same ..
        fgmm_m_step(GMM,data,data_length,pix,&reestimate_flag,COVARIANCE_FULL);
    }

    free(pix);
    return niter;

}

