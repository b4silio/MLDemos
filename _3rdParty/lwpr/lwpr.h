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


/** \defgroup LWPR_C  LWPR library (the low level C implementation) */

/** \file lwpr.h
   \brief Main header file of the LWPR library
   \ingroup LWPR_C
*/

/** \mainpage

   Locally Weighted Projection Regression (LWPR) is an incremental algorithm
   for learning possibly high-dimensional input-output relations online. It
   is composed of linear sub-models, the output/responsibility of which is
   weighted by a locality kernel. The regression parameters of the sub-models
   are updated using an online formulation of partial least squares (PLS).
   Each submodel is endowed with a distance metric, which governs its region
   of validity, and which also gets updated during training. For an in-depth
   description of the LWPR algorithm, please see [1].
   
   This library started life as a Matlab-only implementation, whose subroutines
   were then succesively transformed into C. Therefore, the C library is very 
   closely modelled after the LWPR implementation in Matlab. Indeed, most 
   Matlab functions (lwpr_*.m) have a C/MEX-equivalent.
   
   For easy inter-operation, the data format of vectors and matrices used 
   in this library is that of Matlab (or Fortran). That is, vectors are just 
   arrays of doubles. Matrices are also 1-D arrays of doubles, with the elements 
   stored in column-major order. 
   A 2x2 matrix 
   \f[M = \left(\begin{array}{cc}
   m_{11} & m_{12} \\
   m_{21} & m_{22}
   \end{array}\right)\f]
   is thus stored as\code
         double M[4] = {m11,m21,m12,m22};
   \endcode

   Apart from the using the C-library stand-alone or from Matlab, there is
   also a slim C++ wrapper, and a Python module. The C++ wrapper uses STL 
   vectors, and the Python module uses NumPy arrays.
   LWPR models can be interchanged between all implementations via binary 
   files (platform-dependent, see lwpr_binio.h) or XML files (see lwpr_xml.h).
   
   In order to possibly exploit speedups using SSE2 on newer x86-machines,
   all internally stored vectors whose length is equal to the input dimensionality
   are aligned on 16 byte boundaries. This is also the case for the columns of
   matrices. 
   
   [1] Sethu Vijayakumar, Aaron D'Souza and Stefan Schaal,
   <em>Incremental Online Learning in High Dimensions</em>,
   Neural Computation, vol. 17, no. 12, pp. 2602-2634 (2005).
*/

#ifndef __LWPR_H
#define __LWPR_H

#include "lwpr_config.h"

#ifndef NUM_THREADS 
#define NUM_THREADS   1
#endif

#if NUM_THREADS < 1 || NUM_THREADS > 32
#error "NUM_THREADS must be a number between 1 and 32."
#endif

#ifndef LWPR_REGSTORE
/** LWPR_REGSTORE is the default storage size for PLS regression directions */
#define LWPR_REGSTORE   2
/** When adding a PLS regression direction requires a re-allocation, this
    constant determines how much storage is added */
#define LWPR_REGINCR    2
#endif


#ifdef __cplusplus
extern "C" {
#endif

/** Enumeration of locality kernels that the LWPR library toolbox provides. This determines
    how an activation (or weight) <b>w</b> is computed from a given input vector <b>x</b> and 
    the centre <b>c</b> and distance metric <b>D</b> of a receptive field. 
   \ingroup LWPR_C    
*/
typedef enum {
   LWPR_GAUSSIAN_KERNEL, LWPR_BISQUARE_KERNEL
} LWPR_Kernel;

/** \var LWPR_GAUSSIAN_KERNEL 
      Receptive field activations will be computed using the Gaussian kernel 
      \f[ w = \exp\left(-\frac{1}{2} (\mathbf{x-c})^T \mathbf{D(x-c)}\right)\f] 
      This is the default setting after a call to lwpr_init_model().
   \ingroup LWPR_C      
*/

/** \var LWPR_BISQUARE_KERNEL
      Receptive field activations will be computed using the Bisquare kernel 
      \f[ w = \left\{\begin{array}{ll}
         \left(1 -\frac{1}{2} q\right)^2 & q < 2 \\
         0 & q\geq 2
      \end{array}\right.,\quad\textnormal{where~}
      q = (\mathbf{x-c})^T \mathbf{D(x-c)}\f] 
   \ingroup LWPR_C      
*/

/** \brief This structure completely describes a "receptive field" (a local linear model). 

   In the descriptions of matrix- and vector-valued members of this structure,
   <em>N</em> denotes the input dimensionality, and <em>R</em> denotes the number
   of partial least squares (PLS) dimensions.
   \ingroup LWPR_C
*/
typedef struct {
   int nReg;           /**< \brief The number of PLS regression directions */
   int nRegStore;      /**< \brief The number of PLS directions that can be stored before a re-allocation is necessary */
   
   double *fixStorage; /**< \brief A pointer to memory that is independent of nReg */
   double *varStorage; /**< \brief A pointer to memory that might have to be re-allocated (nReg) */
   
   int trustworthy;    /**< \brief This flag indicates whether a receptive field has "seen" enough data so that its predictions can be trusted */ 
   int slopeReady;     /**< \brief Indicates whether the vector "slope" can be used instead of doing PLS calculatations */    
   double w;           /**< \brief The current activation (weight) */
   double sum_e2;      /**< \brief The accumulated prediction error on the training data */
   double beta0;       /**< \brief Constant part of the PLS output */
   double SSp;         /**< \brief Sufficient statistics used for the confidence bounds */
           
   double *D;          /**< \brief Distance metric (NxN) */
   double *M;          /**< \brief Cholesky factorization of the distance metric (NxN) */
   double *alpha;      /**< \brief Learning rates for updates to M (NxN) */
   double *beta;       /**< \brief PLS regression coefficients (Rx1) */
   double *c;          /**< \brief The centre of the receptive field (Nx1) */
   double *SXresYres;  /**< \brief Sufficient statistics for the PLS regression axes LWPR_ReceptiveField.U (NxR) */
   double *SSs2;       /**< \brief Sufficient statistics for PLS loadings s (Rx1) */
   double *SSYres;     /**< \brief Sufficient statistics for PLS coefficients beta (Rx1) */
   double *SSXres;     /**< \brief Sufficient statistics for PLS input reductions P (NxR) */
   double *U;          /**< \brief PLS regression axes (NxR) */
   double *P;          /**< \brief PLS input reduction parameters (NxR) */
   double *H;          /**< \brief Sufficient statistics for distance metric updates (Rx1) */
   double *r;          /**< \brief Sufficient statistics for distance metric updates (Rx1) */
   double *h;          /**< \brief Sufficient statistics for 2nd order distance metric updates (NxN) */
   double *b;          /**< \brief Memory terms for 2nd order updates to M (NxN) */
   double *sum_w;      /**< \brief Accumulated activation w per PLS direction (Rx1) */
   double *sum_e_cv2;  /**< \brief Accumulated CV-error on training data (Rx1) */
   double *n_data;     /**< \brief Number of training data each PLS direction has seen (Rx1) */
   double *lambda;     /**< \brief Forgetting factor per PLS direction (Rx1) */
   double *mean_x;     /**< \brief Mean of the training data this RF has seen (Nx1) */
   double *var_x;      /**< \brief Variance of the training data this RF has seen (Nx1) */
   double *s;          /**< \brief Current PLS loadings (Rx1) */
   double *slope;      /**< \brief Slope of the local model (Nx1). This avoids PLS calculations when no updates are performed anymore. */
   const struct LWPR_Model *model; /**< \brief Pointer to the LWPR_Model this RF belongs to */
} LWPR_ReceptiveField;

/** \brief The structure LWPR_SubModel holds all the receptive fields (LWPR_ReceptiveField) that
    contribute to a particular output dimension of the complete LWPR_Model. 
   \ingroup LWPR_C    
*/
typedef struct {
   int numRFS;                /**< \brief The number of receptive fields (see LWPR_ReceptiveField) */
   int numPointers;           /**< \brief The number of RFs that can be stored before a re-allocation is necessary */
   int n_pruned;              /**< \brief Number of RFs that were pruned during training */
   LWPR_ReceptiveField **rf;  /**< \brief Array of pointers to LWPR_ReceptiveField */
   const struct LWPR_Model *model;/**< \brief Pointer to the "mother" LWPR_Model. */
} LWPR_SubModel;

/** \brief Main data structure for describing an LWPR model.

   This structure contains flags and initial values that determine the behaviour of
   the LWPR algorithm, and also provides some statistics about the model.
   
   It should always be initialised with lwpr_init_model, and destroyed with lwpr_free_model.
   Note that both functions do not allocate/free the space for the LWPR_Model itself.
   \ingroup LWPR_C   
*/
typedef struct LWPR_Model {
   int nIn;             /**< \brief Number N of input dimensions */
   int nInStore;        /**< \brief Storage-size of any N-vector, for aligment purposes */
   int nOut;            /**< \brief Number M of output dimensions */
   int n_data;          /**< \brief Number of training data the model has seen */

   double *mean_x;      /**< \brief Mean of all training data the model has seen (Nx1) */
   double *var_x;       /**< \brief Mean of all training data the model has seen (Nx1) */
   char *name;          /**< \brief An optional description of the model (Mx1) */
   int diag_only;       /**< \brief Flag that determines whether distance matrices are handled as diagonal-only */
   int meta;            /**< \brief Flag that determines wheter 2nd order updates to LWPR_ReceptiveField.M are computed */
   double meta_rate;    /**< \brief Learning rate for 2nd order updates */
   double penalty;      /**< \brief Penalty factor used within distance metric updates */
   double *init_alpha;  /**< \brief Initial learning rate for 2nd order distance metric updates (NxN) */
   double *norm_in;     /**< \brief Input normalisation (Nx1). Adjust this to the expected variation of your data. */
   double *norm_out;    /**< \brief Output normalisation. Adjust this to the expected variation of your output data. */
   double *init_D;      /**< \brief Initial distance metric (NxN). This often requires some tuning (NxN) */
   double *init_M;      /**< \brief Cholesky factorisation of LWPR_Model.init_D (NxN) */
   double w_gen;        /**< \brief Threshold that determines the minimum activation before a new RF is created. */
   double w_prune;      /**< \brief Threshold that determines above which (second highest) activation a RF is pruned. */
   double init_lambda;  /**< \brief Initial forgetting factor */
   double final_lambda; /**< \brief Final forgetting factor */
   double tau_lambda;   /**< \brief This parameter describes the annealing schedule of the forgetting factor */
   double init_S2;      /**< \brief Initial value for sufficient statistics LWPR_ReceptiveField.SSs2 */
   double add_threshold;/**< \brief Threshold that determines when a new PLS regression axis is added */
   LWPR_Kernel kernel;  /**< \brief Describes which kernel function is used (Gaussian or BiSquare) */
   int update_D;        /**< \brief Flag that determines whether distance metric updates are performed (default: 1) */
   LWPR_SubModel *sub;  /**< \brief Array of SubModels, one for each output dimension. */
   struct LWPR_Workspace *ws;  /**< \brief Array of Workspaces, one for each thread (cf. LWPR_NUM_THREADS) */
   
   double *storage;     /**< \brief Pointer to allocated memory. Do not touch. */
   
   double *xn;          /**< \brief Used to hold a normalised input vector (Nx1) */
   double *yn;          /**< \brief Used to hold a normalised output vector (Nx1) */
   
#ifdef MATLAB
   int isPersistent;    /**< \brief MEX-specific flag which determines whether this LWPR_Model is persistent
                          
      This variable is only included in the LWPR_Model structure if the library is
      compiled with the directive MATLAB (i.e. for MEX-file usage). In that case,
      <em>isPersistent=1</em> indicates that the LWPR model should be protected
      from automatic memory cleanups as performed by MATLAB. */
#endif
} LWPR_Model;

/** \brief Computes the prediction of an LWPR model given an input vector x. Can also
      return confidence bounds and the maximal activation of all receptive fields.
  
   \param[in] model  Must point to a valid LWPR_Model structure
   \param[in] x      Input vector, must point to an array of <em>nIn</em> doubles
   \param[in] cutoff A threshold parameter. Receptive fields with activation below the cutoff are ignored
   \param[out] y     Output vector, must point to an array of <em>nOut</em> doubles
   \param[out] conf  Confidence bounds per output dimension. Must be NULL or point to an array of <em>nOut</em> doubles
   \param[out] max_w Maximum activation per output dimension. Must be NULL or point to an array of <em>nOut</em> doubles
   \ingroup LWPR_C   
*/      
void lwpr_predict(const LWPR_Model *model, const double *x, 
      double cutoff, double *y, double *conf, double *max_w);

/** \brief Computes the prediction and its derivatives (Jacobian) of an LWPR model
      given an input vector x.
  
   \param[in] model  Must point to a valid LWPR_Model structure
   \param[in] x      Input vector, must point to an array of <em>nIn</em> doubles
   \param[in] cutoff A threshold parameter. Receptive fields with activation below the cutoff are ignored
   \param[out] y     Output vector, must point to an array of <em>nOut</em> doubles
   \param[out] J     Jacobian matrix, i.e. derivatives of output vector with respect to input vector. 
                     Must point to an array of <em>nOut*nIn</em> doubles. The matrix is stored in column-major
                     order, that is, for a 3-D input x and 2-D output y the Jacobian
\f[\mathbf{J} = \frac{\partial\mathbf{y}}{\partial\mathbf{x}} 
= \left(\begin{array}{ccc}
\frac{\partial y_1}{\partial x_1} & \frac{\partial y_1}{\partial x_2} & \frac{\partial y_1}{\partial x_3}\medskip\\
\frac{\partial y_2}{\partial x_1} & \frac{\partial y_2}{\partial x_2} & \frac{\partial y_2}{\partial x_3} \\
\end{array}\right)
= \left(\begin{array}{ccc}
j_{11} & j_{12} & j_{13}\\
j_{21} & j_{22} & j_{23}\\
\end{array}\right)
\f]
is thus stored as
\code
   double J[6] = {j11,j21,j12,j22,j13,j23};
\endcode

   \ingroup LWPR_C
*/      
void lwpr_predict_J(const LWPR_Model *model, const double *x, 
      double cutoff, double *y, double *J);


/** \brief Computes the predictions, the confidence intervals and 
           the first derivatives of all the quantities.
  
   \param[in] model  Must point to a valid LWPR_Model structure
   \param[in] x      Input vector, must point to an array of <em>nIn</em> doubles
   \param[in] cutoff A threshold parameter. Receptive fields with activation below the cutoff are ignored
   \param[out] y     Output vector, must point to an array of <em>nOut</em> doubles
   \param[out] J     Jacobian matrix, i.e. derivatives of output vector with respect to input vector. 
                     Must point to an array of <em>nOut*nIn</em> doubles. 
   \param[out] conf  Confidence intervals, must point to an array of <em>nOut</em> doubles
   \param[out] Jconf Jacobian of the confidences, must point to an array of <em>nOut*nIn</em> doubles. 

   \ingroup LWPR_C
*/      
void lwpr_predict_JcJ(const LWPR_Model *model, const double *x, 
      double cutoff, double *y, double *J, double *conf, double *Jconf);         
      
/** \brief Computes the prediction and its first and second derivatives 
           of an LWPR model given an input vector x.
  
   \param[in] model  Must point to a valid LWPR_Model structure
   \param[in] x      Input vector, must point to an array of <em>nIn</em> doubles
   \param[in] cutoff A threshold parameter. Receptive fields with activation below the cutoff are ignored
   \param[out] y     Output vector, must point to an array of <em>nOut</em> doubles
   \param[out] J     Jacobian matrix, i.e. derivatives of output vector with respect to input vector. 
                     Must point to an array of <em>nOut*nIn</em> doubles. 
   \param[out] H     Hessian matrices, i.e. 2nd derivatives of output vector with respect to input vector. 
                     Must point to an array of <em>nIn*nIn*nOut</em> doubles. 
                     The Hessians for each output dimension are stored one after another.

   \ingroup LWPR_C
*/      
void lwpr_predict_JH(const LWPR_Model *model, const double *x, 
      double cutoff, double *y, double *J, double *H);      

/** \brief Updates an LWPR model with a given input/output pair (x,y). Optionally
      returns the model's prediction for y and the maximal activation of all receptive fields.
  
   \param[in,out] model  Must point to a valid LWPR_Model structure
   \param[in] x          Input vector, must point to an array of <em>nIn</em> doubles
   \param[in] y          Output vector, must point to an array of <em>nOut</em> doubles
   \param[out] yp        Current prediction given x. Must be NULL or point to an array of <em>nOut</em> doubles
   \param[out] max_w     Maximum activation per output dimension. Must be NULL or point to an array of <em>nOut</em> doubles
   \return               
      - 1 if the update was succesful
      - 0 if a receptive field would have to be added, but the necessary memory could not be allocated.
   \ingroup LWPR_C                           
*/  
int lwpr_update(LWPR_Model *model, const double *x, const double *y, 
      double *yp, double *max_w);

/** \brief Initialises an LWPR model and allocates internally used storage for submodels etc.
  
   \param[in,out] model  Must point to an LWPR_Model structure 
   \param[in] nIn        Number of input dimensions 
   \param[in] nOut       Number of output dimensions
   \param[in] name       Optional descriptive title. This string (if not NULL) is copied to internally managed memory.
   \return               
      - 1 in case of succes
      - 0 in case of failure (e.g. memory could not be allocated).
   
   Note that "model" must already point to an LWPR_Model structure (e.g. a local variable),
   \sa lwpr_free_model    
   \ingroup LWPR_C   
*/  
int lwpr_init_model(LWPR_Model *model, int nIn, int nOut, const char *name);

/** \brief Cleans up the LWPR_Model structure by disposing all internally allocated memory.
      Make sure you always call this function if you do not need the model anymore.
      
   \param[in] model  The LWPR model structure whose contents should be disposed.
   
   Note that this function does NOT free the memory of the LWPR_Model structure itself.
   Therefore, you should either use static or local variables like
   \code
   LWPR_Model model;
   if (!lwpr_init_model(&model,10,2,"Just a demo")) { 
      error(...);
   }
   ...
   lwpr_free_model(&model);
   \endcode
   or you have to do the malloc() and free() calls yourself, e.g.
   \code
   LWPR_Model *pModel;
   pModel = malloc(sizeof(LWPR_Model));
   if (pModel == NULL || !lwpr_init_model(pModel,10,2,"2nd Demo")) { 
      error(...);
   }
   ... 
   lwpr_free_model(pModel);
   free(pModel);
   \endcode
   \sa lwpr_init_model       
   \ingroup LWPR_C   
*/
void lwpr_free_model(LWPR_Model *model);

/** \brief Set the initial learning rate for 2nd order distance metric updates 
   \param[in,out] model  Pointer to a valid LWPR_Model
   \param[in] alpha      Scalar learning rate (the same for all elements of the distance metric)
   \return  
      - 0 in case of failure (<em>alpha <= 0</em>)
      - 1 in case of success
   \ingroup LWPR_C   
*/
int lwpr_set_init_alpha(LWPR_Model *model, double alpha);

/** \brief Set a spherical initial distance metric for creating new receptive fields
   \param[in,out] model  Pointer to a valid LWPR_Model
   \param[in] sigma      Scale parameter of the distance metric, larger values imply narrower kernels
   \return  
      - 0 in case of failure (<em>sigma <= 0</em>)
      - 1 in case of success
   \ingroup LWPR_C   
*/
int lwpr_set_init_D_spherical(LWPR_Model *model, double sigma);

/** \brief Set a diagonal initial distance metric for creating new receptive fields
   \param[in,out] model  Pointer to a valid LWPR_Model
   \param[in] d   Diagonal elements of the distance metric (must be >= 0). <em>d</em> must point
                  to an array of <em>nIn</em> doubles
   \return 
      - 0 in case of failure (any <em>d[i] <= 0</em>)
      - 1 in case of success
   \ingroup LWPR_C   
*/
int lwpr_set_init_D_diagonal(LWPR_Model *model, const double *d);

/** \brief Set initial distance metric for creating new receptive fields
   \param[in,out] model  Pointer to a valid LWPR_Model
   \param[in] D          Symmetric, positive definite distance metric. Must point
                         to an array of at least nIn*stride doubles. 
   \param[in] stride     Offset between the first element of different columns of D. 
                         Pass <em>nIn</em> if the matrix is stored densely, that is, without
                         any space between adjacent columns. 
   \return  
      - 0 in case of failure (D not positive definite)
      - 1 in case of success
   \ingroup LWPR_C   
*/
int lwpr_set_init_D(LWPR_Model *model, const double *D, int stride);

/** \brief Creates a duplicate (deep copy) of an LWPR model structure
   \param[out] dest  Pointer to an (uninitialised) LWPR_Model
   \param[in] src    Pointer to the LWPR_Model that should be duplicated
   \return 
      - 0 in case of failure (insufficient memory)
      - 1 in case of success
      
      
   \ingroup LWPR_C   
*/   
int lwpr_duplicate_model(LWPR_Model *dest, const LWPR_Model *src);

#ifdef __cplusplus
}
#endif

#endif

