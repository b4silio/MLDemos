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
#include "lwpr.h"
#include "lwpr_aux.h"
#include "lwpr_mem.h"
#include "lwpr_math.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>

#if NUM_THREADS > 1
   #ifdef WIN32
      #include <windows.h>
   #else
      #include <pthread.h> 
   #endif
#endif


int lwpr_init_model(LWPR_Model *model, int nIn, int nOut, const char *name) {

   int i,j,nInS;
   
   if (0==lwpr_mem_alloc_model(model,nIn,nOut,0)) {
      model->nIn = model->nOut = 0;
      return 0;
   }
   
   nInS = model->nInStore;
   
   if (name==NULL) {
      model->name = NULL;
   } else {
      /* Better not use strdup here, since we use LWPR_FREE later */
      model->name = (char *) LWPR_MALLOC(strlen(name)+1);
      if (model->name != NULL) {
         strcpy(model->name,name);
      }
   }
   
   model->n_data = 0;
   model->diag_only = 1;
   model->meta = 0;
   model->meta_rate = 250;
   model->penalty = 1e-6;
   for (j=0;j<nIn;j++) {
      for (i=0;i<nIn;i++) {
         model->init_alpha[i+j*nInS]=50;
      }
   }
   for (i=0;i<nOut;i++) model->norm_out[i] = 1.0;
   for (i=0;i<nIn;i++) model->norm_in[i] = 1.0;
   for (i=0;i<nIn;i++) {
      model->init_D[i+i*nInS] = 25.0;
      model->init_M[i+i*nInS] = 5.0;
   }
   model->w_gen = 0.1;
   model->w_prune = 1.0;
   model->init_lambda = 0.999;
   model->final_lambda = 0.99999;
   model->tau_lambda = 0.9999;
   model->init_S2 = 1e-10;
   model->add_threshold = 0.5;
   model->kernel = LWPR_GAUSSIAN_KERNEL;
   model->update_D = 1;
   return 1;
}

int lwpr_set_init_alpha(LWPR_Model *model, double alpha) {
   int i,j;
   int nIn = model->nIn;
   int nInS = model->nInStore;
   
   if (alpha<=0) return 0;
   
   for (j=0;j<nIn;j++) {
      for (i=0;i<nIn;i++) {
         model->init_alpha[i+j*nInS] = alpha;
      }
   }
   return 1;
}

int lwpr_set_init_D_spherical(LWPR_Model *model, double alpha) {
   int i;
   int nIn = model->nIn;
   int nInS = model->nInStore;
   double sqr;
   
   if (alpha<=0) return 0;
   
   sqr = sqrt(alpha);
   memset(model->init_D,0,nIn*nInS*sizeof(double));
   memset(model->init_M,0,nIn*nInS*sizeof(double));   
   for (i=0;i<nIn;i++) {
      model->init_D[i+i*nInS] = alpha;
      model->init_M[i+i*nInS] = sqr;
   }
   return 1;
}

int lwpr_set_init_D_diagonal(LWPR_Model *model, const double *d) {
   int i;
   int nIn = model->nIn;
   int nInS = model->nInStore;
   
   memset(model->init_D,0,nIn*nInS*sizeof(double));
   memset(model->init_M,0,nIn*nInS*sizeof(double));   
   for (i=0;i<nIn;i++) {
      double sqr,di = d[i];
      if (di<=0.0) return 0;
      sqr = sqrt(di);
      model->init_D[i+i*nInS] = di;
      model->init_M[i+i*nInS] = sqr;
   }
   return 1;
}

int lwpr_set_init_D(LWPR_Model *model, const double *D, int stride) {
   int i;
   int nIn = model->nIn;
   int nInS = model->nInStore;
   
   for (i=0;i<nIn;i++) {
      memcpy(model->init_D + i*nInS, D + i*stride, nIn*sizeof(double));
   }
   return lwpr_math_cholesky(nIn,nInS,model->init_M,model->init_D);
}


int lwpr_duplicate_model(LWPR_Model *dest, const LWPR_Model *src) {
   int dim, n;
   int nIn = src->nIn;
   int nInS = src->nInStore;
   
   if (!lwpr_init_model(dest, nIn, src->nOut, src->name)) return 0;
   
   dest->diag_only     = src->diag_only;
   dest->meta          = src->meta;
   dest->meta_rate     = src->meta_rate;
   dest->penalty       = src->penalty;
   dest->w_gen         = src->w_gen;
   dest->w_prune       = src->w_prune;      
   dest->init_lambda   = src->init_lambda;
   dest->final_lambda  = src->final_lambda;
   dest->tau_lambda    = src->tau_lambda;      
   dest->init_S2       = src->init_S2;
   dest->add_threshold = src->add_threshold;
   dest->kernel        = src->kernel;
   dest->update_D      = src->update_D;
   dest->n_data        = src->n_data;
   
   memcpy(dest->mean_x,     src->mean_x,     nIn * sizeof(double));
   memcpy(dest->var_x,      src->var_x,      nIn * sizeof(double));
   memcpy(dest->norm_in,    src->norm_in,    nIn * sizeof(double));
   memcpy(dest->norm_out,   src->norm_out,   src->nOut * sizeof(double));   
   memcpy(dest->init_D,     src->init_D,     nIn * nInS * sizeof(double));
   memcpy(dest->init_M,     src->init_M,     nIn * nInS * sizeof(double));
   memcpy(dest->init_alpha, src->init_alpha, nIn * nInS * sizeof(double));               
   
   for (dim=0;dim<src->nOut;dim++) {
      for (n=0;n<src->sub[dim].numRFS;n++) {
         LWPR_ReceptiveField *RFd;      
         const LWPR_ReceptiveField *RFs = src->sub[dim].rf[n];
         int nReg = RFs->nReg;
         
         RFd = lwpr_aux_add_rf(&(dest->sub[dim]), RFs->nReg);
         if (RFd==NULL) {
            lwpr_free_model(dest);
            return 0;
         }
         
         RFd->trustworthy = RFs->trustworthy;
         RFd->w           = RFs->w;
         RFd->sum_e2      = RFs->sum_e2;
         RFd->beta0       = RFs->beta0;
         RFd->SSp         = RFs->SSp;
         
         memcpy(RFd->D,      RFs->D,      nInS * nIn * sizeof(double));
         memcpy(RFd->M,      RFs->M,      nInS * nIn * sizeof(double));
         memcpy(RFd->alpha,  RFs->alpha,  nInS * nIn * sizeof(double));
         memcpy(RFd->beta,   RFs->beta,   nReg * sizeof(double));
         memcpy(RFd->c,      RFs->c,      nIn * sizeof(double));
         memcpy(RFd->SXresYres, RFs->SXresYres, nInS * nReg * sizeof(double));
         memcpy(RFd->SSs2,   RFs->SSs2,   nReg * sizeof(double));
         memcpy(RFd->SSYres, RFs->SSYres, nReg * sizeof(double));
         memcpy(RFd->SSXres, RFs->SSXres, nInS * nReg * sizeof(double));
         memcpy(RFd->U,      RFs->U,      nInS * nReg * sizeof(double));
         memcpy(RFd->P,      RFs->P,      nInS * nReg * sizeof(double));
         memcpy(RFd->H,      RFs->H,      nReg * sizeof(double));
         memcpy(RFd->r,      RFs->r,      nReg * sizeof(double));
         memcpy(RFd->h,      RFs->h,      nInS * nIn * sizeof(double));
         memcpy(RFd->b,      RFs->b,      nInS * nIn * sizeof(double));
         memcpy(RFd->sum_w,  RFs->sum_w,  nReg * sizeof(double));
         memcpy(RFd->sum_e_cv2, RFs->sum_e_cv2, nReg * sizeof(double));
         memcpy(RFd->n_data, RFs->n_data, nReg * sizeof(double));
         memcpy(RFd->lambda, RFs->lambda, nReg * sizeof(double));         
         memcpy(RFd->s,      RFs->s,      nReg * sizeof(double));    
         memcpy(RFd->mean_x, RFs->mean_x, nIn * sizeof(double));                       
         memcpy(RFd->var_x,  RFs->var_x,  nIn * sizeof(double));                                
      }
      dest->sub[dim].n_pruned = src->sub[dim].n_pruned;
   }
   return 1;
}


int lwpr_update(LWPR_Model *model, const double *x, const double *y, double *yp, double *max_w) {
   double maxw;
   double ypi;
   
   int i,code=0;
   
   lwpr_aux_update_model_stats(model,x);
   
   for (i=0;i<model->nIn;i++) model->xn[i]=x[i]/model->norm_in[i];
   for (i=0;i<model->nOut;i++) model->yn[i]=y[i]/model->norm_out[i];   
   
   for (i=0;i<model->nOut;i++) {
      code |= lwpr_aux_update_one(model, i, model->xn, model->yn[i], &ypi, &maxw);   
      if (max_w!=NULL) max_w[i]=maxw;
      if (yp!=NULL) yp[i]=ypi * model->norm_out[i];
   }
   return code;
}



#if NUM_THREADS == 1
/* Predictions (and Jacobians) without multi-threading
** We directly use the thread-based functions anyway */

void lwpr_predict(const LWPR_Model *model, const double *x, double cutoff, double *y, double *conf, double *max_w) {
   int i;
   LWPR_ThreadData TD; 
   
   for (i=0;i<model->nIn;i++) model->xn[i]=x[i]/model->norm_in[i];
   
   TD.model = model;
   TD.xn = model->xn;
   TD.ws = &model->ws[0];
   TD.cutoff = cutoff;   
   
   if (conf == NULL) {
      for (i=0;i<model->nOut;i++) {
         TD.dim = i;
         (void) lwpr_aux_predict_one_T(&TD);
         if (max_w!=NULL) max_w[i]=TD.w_max;
         y[i] = TD.yn;
      }
   } else {
      for (i=0;i<model->nOut;i++) {
         TD.dim = i;
         (void) lwpr_aux_predict_conf_one_T(&TD);
         if (max_w!=NULL) max_w[i]=TD.w_max;
         conf[i] = model->norm_out[i]*TD.w_sec; /* this holds the confidence bounds */
         y[i] = TD.yn;
      }
   }
   for (i=0;i<model->nOut;i++) y[i]*=model->norm_out[i];
}

void lwpr_predict_J(const LWPR_Model *model, const double *x, double cutoff, double *y, double *J) {
   int nIn = model->nIn;
   LWPR_ThreadData TD; 
   const double *dydx;
   int i,j;
   
   for (i=0;i<nIn;i++) model->xn[i]=x[i]/model->norm_in[i];
   TD.model = model;
   TD.xn = model->xn;
   TD.ws = &model->ws[0];
   TD.cutoff = cutoff;   
   
   dydx = TD.ws->sum_dwdx;
      
   for (i=0;i<model->nOut;i++) {
      TD.dim = i;
      (void) lwpr_aux_predict_one_J_T(&TD);
      y[i] = model->norm_out[i] * TD.yn;
      for (j=0;j<nIn;j++) {
         J[i+j*model->nOut] = dydx[j]*model->norm_out[i]/model->norm_in[j];
      }
   }
}

void lwpr_predict_JcJ(const LWPR_Model *model, const double *x, double cutoff, double *y, double *J, double *conf, double *Jconf) {
   int nIn = model->nIn;
   LWPR_ThreadData TD; 
   const double *dydx;
   const double *dcdx;
   int i,j;
   
   for (i=0;i<nIn;i++) model->xn[i]=x[i]/model->norm_in[i];
   TD.model = model;
   TD.xn = model->xn;
   TD.ws = &model->ws[0];
   TD.cutoff = cutoff;   
   
   dydx = TD.ws->sum_ydwdx_wdydx;
   dcdx = TD.ws->sum_ddRdxdx;
      
   for (i=0;i<model->nOut;i++) {
      TD.dim = i;
      (void) lwpr_aux_predict_one_JcJ_T(&TD);
      y[i]    = model->norm_out[i] * TD.yn;
      conf[i] = model->norm_out[i] * TD.w_sec;
      
      for (j=0;j<nIn;j++) {
         double factor = model->norm_out[i]/model->norm_in[j];
         J[i+j*model->nOut]     = dydx[j]*factor;
         Jconf[i+j*model->nOut] = dcdx[j]*factor;
      }
   }
}

void lwpr_predict_JH(const LWPR_Model *model, const double *x, double cutoff, double *y, double *J, double *H) {
   int nIn = model->nIn;
   int nInS = model->nInStore;
   LWPR_ThreadData TD; 
   const double *dydx;
   const double *Hi;
   int i,j,k;
   
   for (i=0;i<nIn;i++) model->xn[i]=x[i]/model->norm_in[i];
   TD.model = model;
   TD.xn = model->xn;
   TD.ws = &model->ws[0];
   TD.cutoff = cutoff;   
   
   dydx = TD.ws->sum_dwdx;
   Hi = TD.ws->sum_ddwdxdx;
      
   for (i=0;i<model->nOut;i++) {
      TD.dim = i;
      (void) lwpr_aux_predict_one_gH_T(&TD);
      y[i] = model->norm_out[i] * TD.yn;
      for (j=0;j<nIn;j++) {
         double factor = model->norm_out[i]/model->norm_in[j];
         
         J[i+j*model->nOut] = dydx[j]*factor;
         for (k=0;k<nIn;k++) {
            H[k + j*nIn + i*nIn*nIn] = Hi[k+j*nInS]*factor/model->norm_in[k];
         }
      }
   }
}


#else

/* Multi-threaded predictions (and Jacobians)
** Each thread is responsible for a complete submodel (output dimension)
*/
void lwpr_predict(const LWPR_Model *model, const double *x, double cutoff, double *y, double *conf, double *max_w) {
   int i,dim;
   LWPR_ThreadData TD[NUM_THREADS];
   
   void *(*predict_func)(void *);
   
#ifdef WIN32
   HANDLE thread[NUM_THREADS-1];
   DWORD ID[NUM_THREADS-1];
#else
   pthread_t thread[NUM_THREADS-1];
   int rc[NUM_THREADS-1];      
#endif

   predict_func = (conf==NULL) ? lwpr_aux_predict_one_T : lwpr_aux_predict_conf_one_T;
 
   for (i=0;i<model->nIn;i++) model->xn[i]=x[i]/model->norm_in[i];

   for (i=0;i<NUM_THREADS;i++) {
      TD[i].model = model;
      TD[i].xn = model->xn;
      TD[i].ws = &model->ws[i];
      TD[i].cutoff = cutoff;
   }
   
   dim = 0;
   while (dim < model->nOut) {
      int todo = model->nOut - dim;
      if (todo > NUM_THREADS) todo=NUM_THREADS;
      
      for (i=0;i<todo-1;i++) {
         TD[i].dim = dim+i;
#ifdef WIN32
         thread[i] = CreateThread(NULL,0, predict_func ,&TD[i],0, &ID[i]);
#else
         rc[i] = pthread_create(&thread[i], NULL, predict_func , &TD[i]);
#endif         
      }      
      i = todo-1;
      TD[i].dim = dim + i;
      (void) predict_func(&TD[i]);

      y[dim+i] = TD[i].yn;
      if (conf!=NULL) conf[dim+i] = model->norm_out[dim+i] * TD[i].w_sec; 
      if (max_w!=NULL) max_w[dim+i] = TD[i].w_max;

      for (i=0;i<todo-1;i++) {      
#ifdef WIN32
         if (thread[i]!=NULL) {
            WaitForSingleObject(thread[i],INFINITE);
            CloseHandle(thread[i]);            
#else
         if (rc[i]==0) {
            pthread_join(thread[i],NULL);
#endif            
         } else { 
            /* Thread could not be started, do its calculations now */         
            (void) predict_func(&TD[i]);       
         }

         y[dim+i] = TD[i].yn;
         if (conf!=NULL) conf[dim+i] = model->norm_out[dim+i] * TD[i].w_sec;
         if (max_w!=NULL) max_w[dim+i] = TD[i].w_max;
      }
      dim+=todo;
   }
   for (i=0;i<model->nOut;i++) y[i]*=model->norm_out[i];
}


void lwpr_predict_J(const LWPR_Model *model, const double *x, double cutoff, double *y, double *J) {
   int i,j,dim;
   LWPR_ThreadData TD[NUM_THREADS];
   
#ifdef WIN32
   HANDLE thread[NUM_THREADS-1];
   DWORD ID[NUM_THREADS-1];
#else
   pthread_t thread[NUM_THREADS-1];
   int rc[NUM_THREADS-1];      
#endif

   for (i=0;i<model->nIn;i++) model->xn[i]=x[i]/model->norm_in[i];

   for (i=0;i<NUM_THREADS;i++) {
      TD[i].model = model;
      TD[i].xn = model->xn;
      TD[i].ws = &model->ws[i];
      TD[i].cutoff = cutoff;
   }
   
   dim = 0;
   while (dim < model->nOut) {
      int todo = model->nOut - dim;
      if (todo > NUM_THREADS) todo=NUM_THREADS;
      
      for (i=0;i<todo-1;i++) {
         TD[i].dim = dim+i;
#ifdef WIN32
         thread[i] = CreateThread(NULL,0, lwpr_aux_predict_one_J_T ,&TD[i],0, &ID[i]);
#else
         rc[i] = pthread_create(&thread[i], NULL, lwpr_aux_predict_one_J_T , &TD[i]);
#endif         
      }     
       
      i = todo-1;
      TD[i].dim = dim + i;
      lwpr_aux_predict_one_J_T(&TD[i]);

      for (i=0;i<todo-1;i++) {      
#ifdef WIN32
         if (thread[i]!=NULL) {
            WaitForSingleObject(thread[i],INFINITE);
            CloseHandle(thread[i]);
#else
         if (rc[i]==0) {
            pthread_join(thread[i],NULL);
#endif            
         } else { 
            /* Thread could not be started, do its calculations now */
            lwpr_aux_predict_one_J_T(&TD[i]);       
         }
      }
      
      for (i=0;i<todo;i++) {
         const double *dydx = TD[i].ws->sum_dwdx;
         double no = model->norm_out[dim+i];
         
         y[dim+i] = no * TD[i].yn ;
         for (j=0;j<model->nIn;j++) {
            J[dim+i+j*model->nOut] = dydx[j]*no/model->norm_in[j];
         }
      }
      dim+=todo;
   }
}



void lwpr_predict_JcJ(const LWPR_Model *model, const double *x, double cutoff, double *y, double *J, double *conf, double *Jconf) {
   int i,j,dim;
   LWPR_ThreadData TD[NUM_THREADS];
   
#ifdef WIN32
   HANDLE thread[NUM_THREADS-1];
   DWORD ID[NUM_THREADS-1];
#else
   pthread_t thread[NUM_THREADS-1];
   int rc[NUM_THREADS-1];      
#endif

   for (i=0;i<model->nIn;i++) model->xn[i]=x[i]/model->norm_in[i];

   for (i=0;i<NUM_THREADS;i++) {
      TD[i].model = model;
      TD[i].xn = model->xn;
      TD[i].ws = &model->ws[i];
      TD[i].cutoff = cutoff;
   }
   
   dim = 0;
   while (dim < model->nOut) {
      int todo = model->nOut - dim;
      if (todo > NUM_THREADS) todo=NUM_THREADS;
      
      for (i=0;i<todo-1;i++) {
         TD[i].dim = dim+i;
#ifdef WIN32
         thread[i] = CreateThread(NULL,0, lwpr_aux_predict_one_JcJ_T ,&TD[i],0, &ID[i]);
#else
         rc[i] = pthread_create(&thread[i], NULL, lwpr_aux_predict_one_JcJ_T_T , &TD[i]);
#endif         
      }     
       
      i = todo-1;
      TD[i].dim = dim + i;
      lwpr_aux_predict_one_JcJ_T(&TD[i]);

      for (i=0;i<todo-1;i++) {      
#ifdef WIN32
         if (thread[i]!=NULL) {
            WaitForSingleObject(thread[i],INFINITE);
            CloseHandle(thread[i]);
#else
         if (rc[i]==0) {
            pthread_join(thread[i],NULL);
#endif            
         } else { 
            /* Thread could not be started, do its calculations now */
            lwpr_aux_predict_one_JcJ_T(&TD[i]);       
         }
      }
      
      for (i=0;i<todo;i++) {
         const double *dydx = TD[i].ws->sum_ydwdx_wdydx;
         const double *dcdx = TD[i].ws->sum_ddRdxdx;         
         double no = model->norm_out[dim+i];
         
         y[dim+i] = no * TD[i].yn;
         conf[dim+i] = no * TD[i].w_sec;
         
         for (j=0;j<model->nIn;j++) {
            double noni = no/model->norm_in[j];
            J[dim+i+j*model->nOut]     = dydx[j]*noni;
            Jconf[dim+i+j*model->nOut] = dcdx[j]*noni;
         }
      }
      dim+=todo;
   }
}



void lwpr_predict_JH(const LWPR_Model *model, const double *x, double cutoff, double *y, double *J, double *H) {
   int i,j,dim;
   LWPR_ThreadData TD[NUM_THREADS];
   
#ifdef WIN32
   HANDLE thread[NUM_THREADS-1];
   DWORD ID[NUM_THREADS-1];
#else
   pthread_t thread[NUM_THREADS-1];
   int rc[NUM_THREADS-1];      
#endif

   for (i=0;i<model->nIn;i++) model->xn[i]=x[i]/model->norm_in[i];

   for (i=0;i<NUM_THREADS;i++) {
      TD[i].model = model;
      TD[i].xn = model->xn;
      TD[i].ws = &model->ws[i];
      TD[i].cutoff = cutoff;
   }
   
   dim = 0;
   while (dim < model->nOut) {
      int todo = model->nOut - dim;
      if (todo > NUM_THREADS) todo=NUM_THREADS;
      
      for (i=0;i<todo-1;i++) {
         TD[i].dim = dim+i;
#ifdef WIN32
         thread[i] = CreateThread(NULL,0, lwpr_aux_predict_one_gH_T ,&TD[i],0, &ID[i]);
#else
         rc[i] = pthread_create(&thread[i], NULL, lwpr_aux_predict_one_gH_T , &TD[i]);
#endif         
      }     
       
      i = todo-1;
      TD[i].dim = dim + i;
      lwpr_aux_predict_one_gH_T(&TD[i]);

      for (i=0;i<todo-1;i++) {      
#ifdef WIN32
         if (thread[i]!=NULL) {
            WaitForSingleObject(thread[i],INFINITE);
            CloseHandle(thread[i]);
#else
         if (rc[i]==0) {
            pthread_join(thread[i],NULL);
#endif            
         } else { 
            /* Thread could not be started, do its calculations now */
            lwpr_aux_predict_one_gH_T(&TD[i]);       
         }
      }
      
      for (i=0;i<todo;i++) {
         const double *dydx = TD[i].ws->sum_dwdx;
         const double *Hi = TD[i].ws->sum_ddwdxdx;
         double no = model->norm_out[dim+i];
         
         y[dim+i] = no * TD[i].yn ;
         for (j=0;j<model->nIn;j++) {
            double fac = no/model->norm_in[j];
            int k;
            
            J[dim+i+j*model->nOut] = dydx[j]*fac;
            for (k=0;k<model->nIn;k++) {
               H[k+j*model->nIn+(dim+i)*model->nIn*model->nIn] = Hi[k+j*model->nInStore]*fac/model->norm_in[k];
            }
         }
      }
      dim+=todo;
   }
}


#endif
