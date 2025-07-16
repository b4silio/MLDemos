/*********************************************************************
LWPR: A library for incremental online learning
Copyright (C) 2007  Stefan Klanke, Sethu Vijayakumar
Contact: sethu.vijayakumar@ed.ac.uk

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either 
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free
Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*********************************************************************/


/** \file lwpr_aux.h
   \brief LWPR auxiliary functions header file
   \ingroup LWPR_C       
*/

#ifndef __LWPR_AUX_H
#define __LWPR_AUX_H

#ifdef __cplusplus
extern "C" {
#endif

/** \brief LWPR_Workspace is a structure that contains enough "working memory" for
    the LWPR computational routines. 
    
    It is automatically allocated within 
    lwpr_init_model, with one LWPR_Workspace structure per thread. You should
    not have to handle any of its elements yourself. */
typedef struct LWPR_Workspace {
   int *derivOk;           /**< \brief Used within lwpr_aux_update_distance_metric for storing which PLS directions can be trusted */
   double *storage;        /**< \brief Pointer to the allocated memory */
   double *dx;             /**< \brief Used to hold the difference between a normalised input vector and a RF's centre */
   double *dwdM;           /**< \brief Derivatives of the weight w with respect to LWPR_ReceptiveField.M */
   double *dJ2dM;          /**< \brief Derivatives of the cost J2 with respect to M */
   double *ddwdMdM;        /**< \brief 2nd derivatives of w wrt. M */
   double *ddJ2dMdM;       /**< \brief 2nd derivatives of J2 wrt. M */
   double *Ps;             /**< \brief Intermediate results used within lwpr_aux_update_distance_metric */
   double *Pse;            /**< \brief Intermediate results used within lwpr_aux_update_distance_metric */
   double *xu;             /**< \brief Used within PLS calculations (updated x) */
   double *yres;           /**< \brief Intermediate results used within lwpr_aux_update_regression */
   double *ytarget;        /**< \brief Intermediate results used within lwpr_aux_update_regression */
   double *xres;           /**< \brief Intermediate results used within lwpr_aux_update_regression */
   double *xc;             /**< \brief Used to hold the difference between a normalised input vector and a RF's centre */
   double *xmz;            /**< \brief Updated mean of a RF */
   double *e_cv;           /**< \brief Intermediate results used within lwpr_aux_update_regression */
   double *s;              /**< \brief Intermediate results used within lwpr_aux_update_regression */
   double *dsdx;           /**< \brief Intermediate results used within lwpr_aux_predict_one_J */
   double *Dx;             /**< \brief Used to store RF.D * (x-RF.c) */
   double *sum_dwdx;       /**< \brief Intermediate results used within lwpr_aux_predict_one_J */
   double *sum_ydwdx_wdydx;/**< \brief Intermediate results used within lwpr_aux_predict_one_J */
   double *sum_ddwdxdx;    /**< \brief Intermediate results used within lwpr_aux_predict_one_gH */
   double *sum_ddRdxdx;    /**< \brief Intermediate results used within lwpr_aux_predict_one_gH */   
} LWPR_Workspace;


/** \brief Data structure that is passed to each thread for updates or predictions. */
typedef struct {
   const LWPR_Model *model;/**< \brief Pointer to the LWPR_Model */
   LWPR_Workspace *ws;     /**< \brief Pointer to the thread's LWPR_Workspace (working memory) */
   const double *xn;       /**< \brief Normalised input vector (Nx1) */
   int dim;                /**< \brief Currently handled output dimension */
   double yn;              /**< \brief Normalised output, dim-th element of normalised output vector */
   double cutoff;          /**< \brief Threshold determining the minimal activation for updating a RF */
   double w_max;           /**< \brief Largest activation encountered in this thread */
   double w_sec;           /**< \brief Second largest activation encountered in this thread */
   double sum_w;           /**< \brief Sum of activations */
   double yp;              /**< \brief Sum of un-normalised predictions of the RFs handled by this thread */
   int start;              /**< \brief Index of first LWPR_ReceptiveField this thread should handle */
   int incr;               /**< \brief Increment for RF index, for splitting up a series of RFs among threads */
   int end;                /**< \brief Upper bound for RF index this thread should handle */
   int ind_max;            /**< \brief Index of RF with largest activation */
   int ind_sec;            /**< \brief Index of RF with second largest activation */
} LWPR_ThreadData;  

/** \brief Computes the derivates of the activation w and a penalty term with
            respect to M, Cholesky factors of the distance metric 
   \param[in] nIn       Number of input dimensions
   \param[in] nInS      Offset between columns in matrices (stride)
   \param[out] dwdM     Derivative of w with respect to M (nIn x nIn)
   \param[out] dJ2dM    Derivative of penalty term J2 to M (nIn x nIn)
   \param[out] ddwdMdM  2nd derivative of w with respect to M (nIn x nIn)
   \param[out] ddJ2dMdM 2nd derivative of J2 with respect to M (nIn x nIn)
   \param[in] w         Activation of receptive field
   \param[in] dwdq      Derivative of w with respect to squared distance (~ outer derivate of the kernel)
   \param[in] ddwdqdq   2nd derivative of w with respect to squared distance 
   \param[in] RF_D      The receptive field's distance metric (nIn x nIn)
   \param[in] RF_M      The Cholesky factorisation of RF_M (nIn x nIn)
   \param[in] dx        The difference between the normalised input x and the receptive fields centre c (nIn x 1)
   \param[in] diag_only Flag that determines whether the distance metric is to be treated as diagonal
   \param[in] penalty   Pre-factor involved in computation of J2
   \param[in] meta      Flag that determines whether 2nd derivatives should be computed
*/              
void lwpr_aux_dist_derivatives(int nIn,int nInS, 
         double *dwdM, double *dJ2dM, double *ddwdMdM, double *ddJ2dMdM, 
         double w, double dwdq, double ddwdqdq, 
         const double *RF_D, const double *RF_M, const double *dx,
         int diag_only, double penalty, int meta);

/** \brief Performs an update of a receptive field's distance metric.
   \param[in,out] RF    Pointer to the receptive field
   \param[in] w         Activation of receptive field
   \param[in] dwdq      Derivative of w with respect to squared distance (~ outer derivate of the kernel)
   \param[in] ddwdqdq   2nd derivative of w with respect to squared distance 
   \param[in] e_cv      Current cross-validation error of the RF
   \param[in] e         Current (non-CV) error 
   \param[in] xn        Normalised input vector (nIn x 1)
   \param[in] ws        Pointer to working memory that may be used
   \return              The "transient multiplier" used to dampen the distance metric updates
*/            
double lwpr_aux_update_distance_metric(LWPR_ReceptiveField *RF, 
      double w, double dwdq, double ddwdqdq, 
      double e_cv, double e, const double *xn, LWPR_Workspace *ws);

/** \brief Performs an update of the receptive field's statistics (weighted mean input and output)
   \param[in,out] RF    Pointer to the receptive field
   \param[in] x         Normalised input vector (nIn)
   \param[in] y         Normalised output (specific to current output dimension) 
   \param[in] w         Activation of receptive field   
   \param[out] xmz      Distance between x and updated weighted mean (nIn)
   \returns Difference between y and updated weighted mean output
*/   
double lwpr_aux_update_means(LWPR_ReceptiveField *RF, 
      const double *x, double y, double w, double *xmz);      
      
/** \brief Computes the PLS projections and its residuals given regression axes
   U, projection axes P, and an input vector x.
   \param[in] nIn    Number of input dimensions
   \param[in] nInS   Storage length (stride) of matrices U, P and xres
   \param[in] nReg   Number of PLS regression directions
   \param[out] s     PLS projections (nReg)
   \param[out] xres  Residuals of input vector after each projection step (nIn x nReg)
   \param[in] x      Input vector (nIn)
   \param[in] U      PLS regression axes (nIn x nReg)
   \param[in] P      PLS projection axes (nIn x nReg)
*/
void lwpr_aux_compute_projection_r(int nIn, int nInS, int nReg, 
      double *s, double *xres, const double *x, const double *U, const double *P);         

/** \brief Computes the PLS projections given regression axes U, projection axes P, 
   and an input vector x.
   \param[in] nIn    Number of input dimensions
   \param[in] nInS   Storage length (stride) of matrices U, P and xres
   \param[in] nReg   Number of PLS regression directions
   \param[out] s     PLS projections (nReg)
   \param[in] x      Input vector (nIn)
   \param[in] U      PLS regression axes (nIn x nReg)
   \param[in] P      PLS projection axes (nIn x nReg)
   \param[in,out] ws Pointer to workspace for intermediate results
*/
void lwpr_aux_compute_projection(int nIn, int nInS, int nReg, 
      double *s, const double *x, const double *U, const double *P, LWPR_Workspace *ws);
      
/** \brief Computes the PLS projections and their derivatives given 
   regression axes U, projection axes P, and an input vector x.
   \param[in] nIn    Number of input dimensions
   \param[in] nInS   Storage length (stride) of matrices U, P and xres
   \param[in] nReg   Number of PLS regression directions
   \param[out] s     PLS projections (nReg)
   \param[out] dsdx  Derivatives of s with respect to x in transposed form (nIn x nReg)
   \param[in] x      Input vector (nIn)
   \param[in] U      PLS regression axes (nIn x nReg)
   \param[in] P      PLS projection axes (nIn x nReg)
   \param[in,out] ws Pointer to workspace for intermediate results
*/      
void lwpr_aux_compute_projection_d(int nIn, int nInS, int nReg, 
      double *s, double *dsdx, const double *x, 
      const double *U, const double *P, LWPR_Workspace *ws);      

/** \brief Performs an update on the regression parameters of one receptive field
   \param[in,out] RF    Pointer to the receptive field
   \param[out] yp       Predicted output of the receptive field AFTER the update
   \param[out] e_cv     cross-validation error (prediction before update) (nReg)
   \param[out] e        Prediction error (after the update)
   \param[in]  x        Input vector sample (nIn)
   \param[in]  y        Output/target sample (specific for one output dimension)
   \param[in]  w        Activation of the receptive field given x
   \param[in,out] ws Pointer to workspace for intermediate results   
*/
void lwpr_aux_update_regression(LWPR_ReceptiveField *RF, double *yp, 
      double *e_cv, double *e, const double *x, double y, double w, LWPR_Workspace *ws);

/** \brief Adds a new receptive field to the specified LWPR_SubModel. Regression
   parameters etc. are not initialised.
   \param[in,out] sub   LWPR_SubModel specific to one output dimension
   \param[in]     nReg  Number of PLS regression directions
   \return     Pointer to the newly created receptive field (is also linked in sub->rf[...])
*/
LWPR_ReceptiveField *lwpr_aux_add_rf(LWPR_SubModel *sub, int nReg);

/** \brief Check if a receptive field needs another PLS regression axis,
   and modify the relevant variables.
   \param[in,out] RF    Pointer to the receptive field
   \return  
      - 0 if a new axis was not needed 
      - 1 if a new axis was added
      - -1 if a new axis was needed, but the necessary memory reallocation could not be done. 
*/
int lwpr_aux_check_add_projection(LWPR_ReceptiveField *RF);

/** \brief Allocates and initialises the variables of a receptive field.
   \param[in,out] RF Pointer to the receptive field to be initialised
   \param[in] model  Pointer to the LWPR model the RF belongs to
   \param[in] RFT    Pointer to a template receptive field (distance metric will be copied), may also be NULL
   \param[in] xc     Input vector (nIn), will be the receptive field's center
   \param[in] y      Output sample (submodel) specific), will be the receptive fields initial bias beta0
   \return 
      - 0 if the necessary memory could not be allocated
      - 1 in case of success
*/      
int lwpr_aux_init_rf(LWPR_ReceptiveField *RF, const LWPR_Model *model, 
      const LWPR_ReceptiveField *RFT, const double *xc, double y);

/** \brief Update the receptive fields specific to one output dimension
   of the LWPR model
   \param[in,out] model Pointer to the LWPR model
   \param[in]  dim      Output dimension to handle [0 ; nOut-1]
   \param[in]  xn       Normalised input vector (nIn)
   \param[in]  yn       Normalised input sample (specific to output dimension "dim")
   \param[out] y_pred   Prediction for yn after update
   \param[out] max_w    Maximum activation over all receptive fields
   \return
      - 1 in case of success
      - 0 if a receptive field would have to be added, but memory allocation failed
*/      
int lwpr_aux_update_one(LWPR_Model *model, int dim, const double *xn, 
      double yn, double *y_pred, double *max_w);

/** \brief Thread function for updating a subset of receptive fields 
   \param[in] ptr    Pointer to an LWPR_ThreadData structure
   \return NULL
*/
void *lwpr_aux_update_one_T(void *ptr);      

/** \brief Update the receptive fields specific to one output dimension
   of the LWPR model
   \param[in,out] model Pointer to the LWPR model
   \param[in]  TD       Pointer to an LWPR_ThreadData structure
   \param[in]  dim      Specific output dimension to handle
   \param[in]  xn       Normalised input vector (nIn)   
   \param[in]  yn       Normalised input sample (specific to output dimension "dim")
   \return
      - 1 in case of success
      - 0 if a receptive field would have to be added, but memory allocation failed
*/      
int lwpr_aux_update_one_add_prune(LWPR_Model *model, LWPR_ThreadData *TD, 
      int dim, const double *xn, double yn);   
      
/** \brief Computes the prediction of an LWPR model for a specific output dimension.
      Can also return confidence bounds and the maximal activation of all receptive fields.
   \param[in] model  Must point to a valid LWPR_Model structure
   \param[in] dim    Specific output dimension to handle   
   \param[in] xn     Input vector, must point to an array of model->nIn doubles
   \param[in] cutoff A threshold parameter. Receptive fields with activation below the cutoff are ignored
   \param[out] conf  Confidence bounds per output dimension. May be NULL.
   \param[out] max_w Maximum activation per output dimension. May be NULL.
   \return  The predicted output value
   
   This function does not do any calculation itself. It just calls 
   either lwpr_aux_predict_one_T or lwpr_aux_predict_conf_T, which are
   written with a less user-friendly, but thread-friendly interface.
*/      
double lwpr_aux_predict_one(const LWPR_Model *model, int dim, 
      const double *xn, double cutoff, double *conf, double *max_w);         
      
      
/** \brief Thread function for predicting the output for one SubModel 
   \param[in,out] ptr    Pointer to an LWPR_ThreadData structure
   \return NULL
      
   You must set the following fields of the LWPR_ThreadData structure that \e ptr points to:
   - \e model  Must point to a valid LWPR_Model structure
   - \e dim    Specific output dimension to handle   
   - \e xn     Input vector, must point to an array of model->nIn doubles
   - \e cutoff A threshold parameter. Receptive fields with activation below the cutoff are ignored
   
   On return, you may read the following fields:
   - \e yn     Prediction of the LWPR model along dimension dim (not yet normalised)
   - \e w_max  Maximum activation per output dimension. May be NULL.
*/
void *lwpr_aux_predict_one_T(void *ptr);

/** \brief Computes the prediction of an LWPR model for a specific output dimension,
            together with its confidence bounds.
   \param[in,out] ptr    Pointer to an LWPR_ThreadData structure      
   \return NULL   
   
   You must set the following fields of the LWPR_ThreadData structure that \e ptr points to:
   - \e model  Must point to a valid LWPR_Model structure
   - \e dim    Specific output dimension to handle   
   - \e xn     Input vector, must point to an array of model->nIn doubles
   - \e cutoff A threshold parameter. Receptive fields with activation below the cutoff are ignored
   
   On return, you may read the following fields:
   - \e yn     Prediction of the LWPR model along dimension dim (not yet normalised)
   - \e w_max  Maximum activation per output dimension. May be NULL.
   - \e w_sec  Confidence bounds for predicted output
*/      
void *lwpr_aux_predict_conf_one_T(void *ptr);


/** \brief Computes the prediction of an LWPR model for a specific output dimension,
      and also the gradient of that prediction with respect to the input vector.
   \param[in] model  Must point to a valid LWPR_Model structure
   \param[in] dim    Specific output dimension to handle   
   \param[in] xn     Input vector, must point to an array of model->nIn doubles
   \param[in] cutoff A threshold parameter. Receptive fields with activation below the cutoff are ignored
   \param[out] dydx  Gradient. Must point to an array of nIn doubles
   \return  The predicted output value
   
   This function does not perform any calculations itself, but just calls
   lwpr_aux_predict_conf_one_J_T()  
   \sa lwpr_aux_predict_one
*/      
double lwpr_aux_predict_one_J(const LWPR_Model *model, int dim, 
      const double *xn, double cutoff, double *dydx);
      
      
/** \brief Computes the prediction of an LWPR model for a specific output dimension,
      the confidence interval for the predictions, and also the gradients of both
      quantities with respect to the input vector.
   \param[in] model  Must point to a valid LWPR_Model structure
   \param[in] dim    Specific output dimension to handle   
   \param[in] xn     Input vector, must point to an array of model->nIn doubles
   \param[in] cutoff A threshold parameter. Receptive fields with activation below the cutoff are ignored
   \param[out] dydx  Gradient of the prediction. Must point to an array of nIn doubles
   \param[out] conf  Confidence interval. Must point to a double
   \param[out] dcdx  Gradient of the confidence interval. Must point to an array of nIn doubles.
   \return  The predicted output value y
   
   This function does not perform any calculations itself, but just calls
   lwpr_aux_predict_conf_one_J_T()  
   \sa lwpr_aux_predict_one
*/      
double lwpr_aux_predict_one_JcJ(const LWPR_Model *model, int dim, 
      const double *xn, double cutoff, double *dydx, double *conf, double *dcdx);      
      
/** \brief Computes the prediction of an LWPR model for a specific output dimension,
      and also the gradient of that prediction with respect to the input vector.
   \param[in] model  Must point to a valid LWPR_Model structure
   \param[in] dim    Specific output dimension to handle   
   \param[in] xn     Input vector, must point to an array of model->nIn doubles
   \param[in] cutoff A threshold parameter. Receptive fields with activation below the cutoff are ignored
   \param[out] dydx  Gradient. Must point to an array of nIn doubles
   \param[out] ddydxdx  Hessian. Must point to an array of nIn x nIn doubles   
   \return  The predicted output value
   
   This function does not perform any calculations itself, but just calls
   lwpr_aux_predict_one_gH_T()  
   \sa lwpr_aux_predict_one
*/      
double lwpr_aux_predict_one_gH(const LWPR_Model *model, int dim, 
      const double *xn, double cutoff, double *dydx, double *ddydxdx);      
      
      
/** \brief Thread function for predicting output and gradient for one SubModel
   \param[in,out] ptr    Pointer to an LWPR_ThreadData structure
   \return NULL
      
   You must set the following fields of the LWPR_ThreadData structure that \e ptr points to:
   - \e model  Must point to a valid LWPR_Model structure
   - \e dim    Specific output dimension to handle   
   - \e xn     Input vector, must point to an array of model->nIn doubles
   - \e cutoff A threshold parameter. Receptive fields with activation below the cutoff are ignored
   
   On return, you may read the following fields:
   - \e yn       Prediction of the LWPR model along dimension dim (not yet normalised)
   - \e sum_dwdx (within LWPR_Workspace pointed to by LWPR_ThreadData) Gradient of \e yn w.r.t. \e xn
   
*/      
void *lwpr_aux_predict_one_J_T(void *ptr);


/** \brief Thread function for predicting output and gradient for one SubModel
   \param[in,out] ptr    Pointer to an LWPR_ThreadData structure
   \return NULL
      
   You must set the following fields of the LWPR_ThreadData structure that \e ptr points to:
   - \e model  Must point to a valid LWPR_Model structure
   - \e dim    Specific output dimension to handle   
   - \e xn     Input vector, must point to an array of model->nIn doubles
   - \e cutoff A threshold parameter. Receptive fields with activation below the cutoff are ignored
   
   On return, you may read the following fields (within LWPR_Workspace pointed to by LWPR_ThreadData)
   - \e yn               Prediction of the LWPR model along dimension dim (not yet normalised)
   - \e sum_ydwdx_wdydx  Gradient of \e yn w.r.t. \e xn
   - \e w_sec            Confidence interval
   - \e sum_ddRdxdx      Gradient of \e w_sec w.r.t. \e xn
   
*/      
void *lwpr_aux_predict_one_JcJ_T(void *ptr);

      
/** \brief Thread function for predicting output, gradient and Hessian for one SubModel
   \param[in,out] ptr    Pointer to an LWPR_ThreadData structure
   \return NULL
      
   You must set the following fields of the LWPR_ThreadData structure that \e ptr points to:
   - \e model  Must point to a valid LWPR_Model structure
   - \e dim    Specific output dimension to handle   
   - \e xn     Input vector, must point to an array of model->nIn doubles
   - \e cutoff A threshold parameter. Receptive fields with activation below the cutoff are ignored
   
   On return, you may read the following fields:
   - \e yn       Prediction of the LWPR model along dimension dim (not yet normalised)
   - \e sum_dwdx (within LWPR_Workspace pointed to by LWPR_ThreadData) Gradient of \e yn w.r.t. \e xn
   - \e sum_ddwdxdx (within LWPR_Workspace pointed to by LWPR_ThreadData) Hessian of \e yn w.r.t. \e xn   
*/      
void *lwpr_aux_predict_one_gH_T(void *ptr);


/** \brief Updates the global model statistics, i.e. the mean and variance of the
      input training data, and also the number of data points.
   \param[in,out] model Pointer to an LWPR model structure
   \param[in]  x        Input vector x (nIn, not normalised)
*/
void lwpr_aux_update_model_stats(LWPR_Model *model, const double *x);

#ifdef __cplusplus
}
#endif

#endif

