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

/** \defgroup LWPR_CPP  C++ wrapper around the LWPR library */

/** \file lwpr.hh
   \brief Header file for simple C++ wrapper around the LWPR C-library.
   \ingroup LWPR_CPP      
*/

#ifndef __LWPR_HH
#define __LWPR_HH

#include "lwpr.h"
#include "lwpr_math.h"
#include "lwpr_binio.h"
#include "lwpr_xml.h"
#include <string.h>
#include <vector>

/** \brief doubleVec Shortcut typedef for the vector object utilised in the C++
   implementation of LWPR 
   \ingroup LWPR_CPP   
*/
typedef std::vector<double> doubleVec;

/** \brief Simple class for describing exceptions that may be
   thrown during calls to LWPR methods 
   \ingroup LWPR_CPP
*/
class LWPR_Exception {
   public:
   
   /** \brief This enumeration lists the possible error codes for exceptions thrown by LWPR_Object methods
   */
   typedef enum { 
      OUT_OF_MEMORY,    /**< \brief Thrown when a method required allocating new memory, but failed */
      BAD_INPUT_DIM,    /**< \brief Thrown when an argument should have matched the input dimension of the LWPR model, but did not */
      BAD_OUTPUT_DIM,   /**< \brief Thrown when an argument should have matched the output dimension of the LWPR model, but did not */
      BAD_INIT_D,       /**< \brief Thrown when the desired initial distance metric is not positive definite */
      UNKNOWN_KERNEL,   /**< \brief Thrown when the name of an unknown kernel function has been passed */
      IO_ERROR,         /**< \brief Thrown when errors occured during reading from or writing to files */
      OUT_OF_RANGE,     /**< \brief Thrown when an out-of-range index was passed */
      UNSPECIFIED_ERROR /**< \brief Thrown in any other error case (should not happen) */
   } Code;
   
   /** \brief Creates an LWPR exception with error code "code" 
      \param code  One of the values in LWPR_Exception::Code
      \return      A new exception object
   */
   LWPR_Exception(Code code) {
      this->code = code;
   }
   
   /** \brief Returns one of the values in LWPR_Exception::Code */
   Code getCode() const { 
      return code;
   }

   /** \brief Returns a short string describing this exception */
   const char *getString() const {
      switch(code) {
         case OUT_OF_MEMORY: 
            return "Insufficient memory to allocate storage.";
         case BAD_INPUT_DIM: 
            return "Input dimensionality does not match.";
         case BAD_OUTPUT_DIM: 
            return "Output dimensionality does not match.";
         case BAD_INIT_D: 
            return "Invalid initial distance metric (not positive definite).";
         case UNKNOWN_KERNEL:
            return "Passed kernel name was not recognised.";
         case IO_ERROR:
            return "An error occurred during I/O operations.";
         case OUT_OF_RANGE:
            return "Index parameter out of range.";
         default:
            return "Oops: Unspecified error.";
      }
   }
   
   private:
   
   /** \brief The exception's error code */
   Code code;
};

class LWPR_Object;


/** \brief Thin wrapper class for inspecting a receptive field.
   You can only create an object of this class by a call to LWPR_Object::getRF()
   All methods of this class leave the underlying receptive field unchanged.
   \ingroup LWPR_CPP
*/
class LWPR_ReceptiveFieldObject {
   friend class LWPR_Object;
   
   public:
   
   /** \brief Returns the number of PLS regression directions */
   int nReg() const { 
      return RF->nReg; 
   }
   
   /** \brief Returns the weighted mean of the input data, as seen by the receptive field (nIn)*/
   doubleVec meanX() const {
      doubleVec mx(nIn);   
      memcpy(&mx[0], RF->mean_x, sizeof(double)*nIn);
      return mx;
   }   
   
   /** \brief Returns the weighted variance of the input data, as seen by the receptive field (nIn)*/   
   doubleVec varX() const {
      doubleVec vx(nIn);   
      memcpy(&vx[0], RF->var_x, sizeof(double)*nIn);
      return vx;
   } 
   
   /** \brief Returns the center vector of the receptive field (nIn) */
   doubleVec center() const {
      doubleVec c(nIn);
      memcpy(&c[0], RF->c, sizeof(double)*nIn);    
      return c;  
   }
   
   /** \brief Returns whether this receptive field is trustworthy (has seen sufficient data) */
   bool trustworthy() const {
      return (bool) RF->trustworthy;
   }

   /** \brief Returns the distance metric of the receptive field, as a vector of vectors (nIn x nIn) */
   std::vector<doubleVec> D() const {
      std::vector<doubleVec> ds(nIn);
      for (int i=0;i<nIn;i++) {
         ds[i].resize(nIn);
         memcpy(&ds[i][0], RF->D + i*nInS, sizeof(double)*nIn);    
      }
      return ds;
   }

   /** \brief Returns the Cholesky decomposition of the RF's distance metric. The result is a
       vector of vectors with varying length (simulating a triagonal matrix) */   
   std::vector<doubleVec> M() const {
      std::vector<doubleVec> ms(nIn);
      for (int i=0;i<nIn;i++) {
         ms[i].resize(i+1);
         memcpy(&ms[i][0], RF->M + i*nInS, sizeof(double)*(i+1));    
      }
      return ms;
   }
   
   /** \brief Returns the PLS regression directions as a vector of vectors (nReg x nIn) */
   std::vector<doubleVec> U() const {
      std::vector<doubleVec> us(RF->nReg);
      for (int i=0;i<RF->nReg;i++) {
         us[i].resize(nIn);
         memcpy(&us[i][0], RF->U + i*nInS, sizeof(double)*nIn);
      }
      return us;
   }   
   
   /** \brief Returns the PLS projections as a vector of vectors (nReg x nIn) */   
   std::vector<doubleVec> P() const {
      std::vector<doubleVec> ps(RF->nReg);
      for (int i=0;i<RF->nReg;i++) {
         ps[i].resize(nIn);
         memcpy(&ps[i][0], RF->P + i*nInS, sizeof(double)*nIn);
      }
      return ps;
   }   
   
   /** \brief Returns the offset (intercept) of the local model */
   double beta0() const {
      return RF->beta0;
   }
   
   /** \brief Returns the PLS regression coefficients of the local model (nReg) */   
   doubleVec beta() const {
      doubleVec be(RF->nReg);
      memcpy(&be[0], RF->beta, sizeof(double)*RF->nReg);      
      return be;
   }
   
   /** \brief Returns the weighted number of training data the RF has seen (nReg) */
   doubleVec numData() const {
      doubleVec nd(RF->nReg);
      memcpy(&nd[0], RF->n_data, sizeof(double)*RF->nReg);      
      return nd;
   }

   
   /** \brief Returns the slope of the local model (simulating ordinary linear regression) (nIn) */
   doubleVec slope() const {
      doubleVec s(nIn);
      doubleVec t(nIn);
      
      if (RF->slopeReady) {
         memcpy(&s[0], RF->slope, sizeof(double)*RF->nReg);
      } else {
         // calculate the slope by hand, without using any model-internal storage
         // we do this because we do not want this code to interfere with the "real"
         // LWPR_Model
         lwpr_math_scalar_vector(&s[0], RF->beta[0], RF->U, nIn);
         for (int i=1;i<RF->nReg;i++) {
            lwpr_math_scalar_vector(&t[0], RF->beta[i], RF->U + i*nInS, nIn);
            for (int j=i-1;j>=0;j--) {
               // left-multiply  (I - u_j * p_j^T)
               double dp = lwpr_math_dot_product(&t[0],RF->P + j*nInS, nIn);
               lwpr_math_add_scalar_vector(&t[0], dp, RF->U + j*nInS, nIn);
            }
            for (int m=0;m<nIn;m++) s[m]+=t[m];
         }
      }
      return s;
   }
   
   private:

   /** \brief Private constructor, gets called by LWPR_Object::getRF() */
   LWPR_ReceptiveFieldObject(LWPR_ReceptiveField *rf) {
      this->RF = rf;
      nIn = rf->model->nIn;
      nInS = rf->model->nInStore;
   }
   
   /** \brief Pointer to the underlying C structure */
   const LWPR_ReceptiveField *RF;
   int nIn;   /**< \brief Number of input dimensions */
   int nInS;  /**< \brief Stride parameter (LWPR_Model::nInStore) */
};


/** \brief Thin C++ wrapper class around C implementation of LWPR 
    \ingroup LWPR_CPP
*/
class LWPR_Object {
   public:
   
   /** \brief Creates an LWPR_Object for the given input and output dimensionalities
      \param nIn  Desired input dimensionality
      \param nOut Desired output dimensionality
      \return     A new object
      
      In case there is insufficient memory for allocating the underlying
      LWPR_Model (C library), an OUT_OF_MEMORY exception is thrown.
   */
   LWPR_Object(int nIn, int nOut) {
      if (!lwpr_init_model(&model, nIn, nOut, NULL)) {
         throw LWPR_Exception(LWPR_Exception::OUT_OF_MEMORY);
      }
   }
   
   /** \brief Creates a copy of an LWPR_Object.
      \param otherObj   LWPR_Object to be duplicated.
      \return     A new object (copy of otherObj).
      
      In case there is insufficient memory for allocating the underlying
      LWPR_Model (C library), an OUT_OF_MEMORY exception is thrown.
   */
   LWPR_Object(const LWPR_Object& otherObj) {
      if (!lwpr_duplicate_model(&(this->model), &(otherObj.model))) {
         throw LWPR_Exception(LWPR_Exception::OUT_OF_MEMORY);
      }
   }
   
   /** \brief Creates an LWPR_Object from a binary file, or if compiled
              with support for EXPAT, an XML file.
      \param filename  Name of file to read the model from
      \return     A new object
      
      In case there are problems with reading the file, an IO_ERROR 
      exception is thrown.
   */
   LWPR_Object(const char *filename) {
      int ok;
      // First try treating the file as binary
      ok = lwpr_read_binary(&model, filename);
      #if HAVE_LIBEXPAT
      if (!ok) {
         int numErr, numWar;
         numErr = lwpr_read_xml(&model, filename, &numWar);
         ok = (numErr == 0);
      }
      #endif
      if (!ok) throw LWPR_Exception(LWPR_Exception::IO_ERROR);
   }

   /** \brief Destroys an LWPR_Object and disposes allocated memory */
   ~LWPR_Object() {
      lwpr_free_model(&model);
   }
   
   /** \brief Write the model to an XML file
      \param filename   Name of the file, which will we overwritten if it already exists
      \return
         - 1 in case of success
         - 0 if the file could not be written to
   */
   int writeXML(const char *filename) {
      return lwpr_write_xml(&model, filename);
   }
   
   /** \brief Write the model to a binary file
      \param filename   Name of the file, which will we overwritten if it already exists
      \return
         - 1 in case of success
         - 0 if the file could not be written to
   */
   int writeBinary(const char *filename) {
      return lwpr_write_binary(&model, filename);
   }
      
   /** \brief Updates an LWPR model with a given input/output pair (x,y). 
  
      \param x   Input vector
      \param y   Output vector
      \return        Current prediction of y given x, useful for tracking
                     the training error.
      
      \exception LWPR_Exception::OUT_OF_MEMORY  
         if a receptive field would have to be added, but memory could not be allocated
      \exception LWPR_Exception::BAD_INPUT_DIM  
         if the parameter x does not match the model dimensions
      \exception LWPR_Exception::BAD_OUTPUT_DIM
         if the parameter y does not match the model dimensions
   */  
   doubleVec update(const doubleVec& x, const doubleVec& y) {
      doubleVec yp(model.nOut);
      
      if (x.size()!=(unsigned) model.nIn) {
         throw LWPR_Exception(LWPR_Exception::BAD_INPUT_DIM);
      }
      
      if (y.size()!=(unsigned) model.nOut) {
         throw LWPR_Exception(LWPR_Exception::BAD_OUTPUT_DIM);
      }

      if (!lwpr_update(&model, &x[0], &y[0], &yp[0], NULL)) {
         throw LWPR_Exception(LWPR_Exception::OUT_OF_MEMORY);
      }
      return yp;
   }
   
   /** \brief Computes the prediction of an LWPR model given an 
      input vector x.
  
      \param x      Input vector.
      \param cutoff A threshold parameter (default = 0.001). 
         Receptive fields with activation below the cutoff are ignored
      \return    Predicted output vector
      \exception LWPR_Exception::BAD_INPUT_DIM  
         if the parameter x does not match the model dimensions
   */      
   doubleVec predict(const doubleVec& x, double cutoff = 0.001) {
      doubleVec yp(model.nOut);   

      if (x.size()!=(unsigned) model.nIn) {
         throw LWPR_Exception(LWPR_Exception::BAD_INPUT_DIM);
      }      

      lwpr_predict(&model, &x[0], cutoff, &yp[0], NULL, NULL);
      return yp;
   }
   
   /** \brief Computes the prediction of an LWPR model given an 
      input vector x. Also computes confidence bounds per output
      dimension.
  
      \param[in] x      Input vector
      \param[out] confidence   Vector to store the confidence bounds, will
         be resized if necessary
      \param[in] cutoff A threshold parameter (default = 0.001). 
         Receptive fields with activation below the cutoff are ignored
      \return    Predicted output vector
      \exception LWPR_Exception::BAD_INPUT_DIM  
         if the parameter x does not match the model dimensions
   */      
   doubleVec predict(const doubleVec& x, doubleVec& confidence, double cutoff = 0.001) {
      doubleVec yp(model.nOut);   
      
      if (x.size()!=(unsigned) model.nIn) {
         throw LWPR_Exception(LWPR_Exception::BAD_INPUT_DIM);
      }      
      if (confidence.size()!=(unsigned) model.nOut) confidence.resize(model.nOut);

      lwpr_predict(&model, &x[0], cutoff, &yp[0], &confidence[0], NULL);
      return yp;
   }  
   
   /** \brief Computes the prediction of an LWPR model given an 
      input vector x. Also computes confidence bounds and maximal 
      activation per output dimension.

      \param[in] x      Input vector
      \param[out] confidence   Vector to store the confidence bounds, will
         be resized if necessary
      \param[out] maxW  Vector to store maximum activations, will be resized
         if necessary
      \param[in] cutoff A threshold parameter (default = 0.001). 
         Receptive fields with activation below the cutoff are ignored
      \return    Predicted output vector
      \exception LWPR_Exception::BAD_INPUT_DIM  
         if the parameter x does not match the model dimensions
   */
   doubleVec predict(const doubleVec& x, doubleVec& confidence, doubleVec& maxW, double cutoff = 0.001) {
      doubleVec yp(model.nOut);   
      
      if (x.size()!=(unsigned) model.nIn) {
         throw LWPR_Exception(LWPR_Exception::BAD_INPUT_DIM);
      }      
      if (confidence.size()!=(unsigned) model.nOut) confidence.resize(model.nOut);
      if (maxW.size()!=(unsigned) model.nOut) maxW.resize(model.nOut);

      lwpr_predict(&model, &x[0], cutoff, &yp[0], &confidence[0], &maxW[0]);
      return yp;
   } 
   
   /** \brief Sets a spherical initial distance metric
      \param delta   Width parameter, distance matrix will be delta * eye(nIn)
      \exception LWPR_Exception::BAD_INIT_D
         if the parameter delta is <= 0, giving rise to a non-positive matrix
   */
   void setInitD(double delta) {
      if (!lwpr_set_init_D_spherical(&model,delta)) {
         throw LWPR_Exception(LWPR_Exception::BAD_INIT_D);
      }
   }

   /** \brief Sets a diagonal or full initial distance metric
      \param initD  Either a vector with nIn elements, containing the diagonal
         of the desired distance metric, or a vector with nIn*nIn elements containing
         the complete distance matrix. 
      \exception LWPR_Exception::BAD_INIT_D
         if the parameter initD gives rise to a non-positive matrix
   */
   void setInitD(const doubleVec& initD) {
      if (initD.size()==(unsigned) model.nIn) {
         if (!lwpr_set_init_D_diagonal(&model,&initD[0])) {
            throw LWPR_Exception(LWPR_Exception::BAD_INIT_D);
         }
      } else if (initD.size()==(unsigned) (model.nIn*model.nIn)) {
         if (!lwpr_set_init_D(&model,&initD[0],model.nIn)) {
            throw LWPR_Exception(LWPR_Exception::BAD_INIT_D);
         }
      } else {
         throw LWPR_Exception(LWPR_Exception::BAD_INPUT_DIM);
      }
   }
   
   /** \brief Sets init_alpha (learning rate for 2nd order distance metric updates) */
   void setInitAlpha(double alpha) {
      lwpr_set_init_alpha(&model,alpha);
   }
   
   /** \brief Sets w_gen (threshold for adding new receptive fields) */
   void wGen(double w_gen) { model.w_gen = w_gen; }
   
   /** \brief Sets w_prune (threshold for removing a receptive field) */   
   void wPrune(double w_prune) { model.w_prune = w_prune; }

   /** \brief Sets penalty (pre-factor for smoothing term in distance metric updates) */
   void penalty(double pen) { model.penalty = pen; }
   
   /** \brief Sets initial forgetting factor */
   void initLambda(double iLam) { model.init_lambda = iLam; }
   
   /** \brief Sets annealing rate for forgetting factor */
   void tauLambda(double tLam) { model.tau_lambda = tLam; }
      
   /** \brief Sets final forgetting factor */   
   void finalLambda(double fLam) { model.final_lambda = fLam; }
   
   /** \brief Sets initial value for covariance computation SSs2 */
   void initS2(double init_s2) { model.init_S2 = init_s2; }
   
   /** \brief Determines whether distance matrix updates are to be performed */
   void updateD(bool update) { model.update_D = update ? 1:0; }
   
   /** \brief Determines whether distance matrices should be treaded as diagonal-only */
   void diagOnly(bool dOnly) { model.diag_only = dOnly ? 1:0; }
   
   /** \brief Determines whether 2nd order distance matrix updates are to be performed */   
   void useMeta(bool meta) { model.meta = meta ? 1:0; }
   
   /** \brief Sets the learning rate for 2nd order distance matrix updates */
   void metaRate(double rate) { model.meta_rate = rate; }
   
   /** \brief Sets the kernel to be used in the LWPR model */
   void kernel(LWPR_Kernel kern) { model.kernel = kern; }
   
   /** \brief Sets the kernel (either "Gaussian" or "BiSquare") to be used in the LWPR model */
   void kernel(const char *str) {
      if (!strcmp(str,"Gaussian")) {
         model.kernel = LWPR_GAUSSIAN_KERNEL;
         return;
      }
      if (!strcmp(str,"BiSquare")) {
         model.kernel = LWPR_BISQUARE_KERNEL;
         return;
      }
      throw LWPR_Exception(LWPR_Exception::UNKNOWN_KERNEL);
   }
   
   /** \brief Returns the number of training data the model has seen */
   int nData() const { return model.n_data; }
   
   /** \brief Returns the input dimensionality */
   int nIn() const { return model.nIn; }
   
   /** \brief Returns the output dimensionality */   
   int nOut() const { return model.nOut; }
   
   /** \brief Returns w_gen (threshold for adding new receptive fields) */
   double wGen() const { return model.w_gen; }
   
   /** \brief Returns w_prune (threshold for removing a receptive field) */ 
   double wPrune() const { return model.w_prune; }   
   
   /** \brief Returns penalty (pre-factor for smoothing term in distance metric updates) */
   double penalty() const { return model.penalty; }
   
   /** \brief Returns initial forgetting factor */
   double initLambda() const { return model.init_lambda; }

   /** \brief Returns annealing rate for forgetting factor */
   double tauLambda() const { return model.tau_lambda; }

   /** \brief Returns final forgetting factor */   
   double finalLambda() const { return model.final_lambda; }
   
   /** \brief Returns initial value for the covariance computation SSs2 */
   double initS2() const { return model.init_S2; }
   
   /** \brief Returns whether distance matrix updates are performed */
   bool updateD() { return (bool) model.update_D; }

   /** \brief Returns whether distance matrices are treaded as diagonal-only */
   bool diagOnly() { return (bool) model.diag_only; }
   
   /** \brief Returns whether 2nd order distance matrix updates are performed */   
   bool useMeta() { return (bool) model.meta; }
   
   /** \brief Returns learning rate for 2nd order distance matrix updates */      
   double metaRate() { return model.meta_rate; }

   /** \brief Returns the kernel */   
   LWPR_Kernel kernel() { return model.kernel; }
   
   /** \brief Returns the mean of all input samples the model has seen */
   doubleVec meanX() {
      doubleVec mx(model.nIn);
      memcpy(model.mean_x,&mx[0],sizeof(double)*model.nIn);
      return mx;
   }

   /** \brief Returns the variance of all input samples the model has seen */   
   doubleVec varX() {
      doubleVec vx(model.nIn);
      memcpy(model.var_x, &vx[0],sizeof(double)*model.nIn);
      return vx;
   }
   
   /** \brief Sets the input normalisation (expected scale or standard deviation
      of input data */
   void normIn(const doubleVec& norm) {
      if (norm.size()!=(unsigned) model.nIn) {
         throw LWPR_Exception(LWPR_Exception::BAD_INPUT_DIM);      
      }
      memcpy(model.norm_in,&norm[0],sizeof(double)*model.nIn);
   }

   /** \brief Returns the input normalisation factors */
   doubleVec normIn() const {
      doubleVec norm(model.nIn);
      memcpy(&norm[0],model.norm_in,sizeof(double)*model.nIn);
      return norm;
   }
   
   /** \brief Sets the output normalisation (expected scale or standard deviation
      of output data */
   void normOut(const doubleVec& norm) {
      if (norm.size()!=(unsigned) model.nOut) {
         throw LWPR_Exception(LWPR_Exception::BAD_OUTPUT_DIM);
      }
      memcpy(model.norm_out,&norm[0],sizeof(double)*model.nOut);
   }

   /** \brief Returns the output normalisation factors */
   doubleVec normOut() const {
      doubleVec norm(model.nOut);
      memcpy(&norm[0],model.norm_out,sizeof(double)*model.nOut);
      return norm;
   }
   
   /** \brief Returns the number of receptive fields for output dimension "outDim" */
   int numRFS(int outDim) {
      if (outDim < 0 || outDim >= model.nOut) return 0;
      return model.sub[outDim].numRFS;
   }
   
   /** \brief Returns the number of receptive fields for all output dimensions */
   std::vector<int> numRFS() {
      std::vector<int> num(model.nOut);
      for (int i=0;i<model.nOut;i++) num[i] = model.sub[i].numRFS;
      return num;
   }
   
   /** \brief Returns a wrapper object for inspecting a receptive field. 
      \param outDim   Desired output dimension
      \param index    Index of the receptive field within that output dimension
      \return   The wrapper object (keeps the receptive field itself constant)
      \exception LWPR_Exception::OUT_OF_RANGE
         if the parameters outDim and index are out of range
         
      Please note that the retrieved object is only guaranteed to be valid
      as long as the underlying LWPR model is not changed, i.e. predictions
      are fine, but updates may result in pruning just the receptive field
      this wrapper points to!!!   
   */
   LWPR_ReceptiveFieldObject getRF(int outDim, int index) const {
      if (outDim < 0 || outDim >= model.nOut) {
         throw LWPR_Exception(LWPR_Exception::OUT_OF_RANGE);
      }
      if (index < 0 || index >= model.sub[outDim].numRFS) { 
         throw LWPR_Exception(LWPR_Exception::OUT_OF_RANGE);
      }
      return LWPR_ReceptiveFieldObject(model.sub[outDim].rf[index]);
   }
   
   /** \brief Underlying C structure */
   LWPR_Model model;
};

#endif
