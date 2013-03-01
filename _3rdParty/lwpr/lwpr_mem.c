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
#include <string.h>
#include <stdlib.h>

int lwpr_mem_alloc_rf(LWPR_ReceptiveField *RF, const LWPR_Model *model, int nReg, int nRegStore) {
   double *storage;
   int nIn = model->nIn;
   int nInS = model->nInStore;
   
   if (nRegStore < nReg) nRegStore = nReg;
   
   RF->nReg = nReg;
   RF->nRegStore = nRegStore;   
   
   RF->model = model;
   
   /* First allocate stuff independent of nReg:
   **    D,M,alpha,h,b are nIn x nIn
   **    mean_x, var_x are nIn x 1
   **           slope  is  nIn x 1
   **      ==>  nIn * (5*nIn + 4)
   */
   
   storage = RF->fixStorage = (double *) LWPR_CALLOC((size_t) (1 + nInS*(5*nIn + 4)), sizeof(double));
   if (storage==NULL) return 0;
   
   if (((long int)((void *) storage)) & 8) storage++;
   RF->alpha  = storage; storage+=nInS*nIn;
   RF->D      = storage; storage+=nInS*nIn;
   RF->M      = storage; storage+=nInS*nIn;
   RF->h      = storage; storage+=nInS*nIn;
   RF->b      = storage; storage+=nInS*nIn;
   RF->c      = storage; storage+=nInS;   
   RF->mean_x = storage; storage+=nInS;
   RF->slope  = storage; storage+=nInS;
   RF->var_x  = storage; 
   
   /* Now, allocate stuff dependent of nReg (nRegStore):
   ** Align the matrices SXresYres, SSXres, W, and U on 16-Byte boundaries
   ** Alignment of the rest can be assured if nRegStore is always chosen even (2,4,...)
   */
   
   storage = RF->varStorage = (double *) LWPR_CALLOC((size_t) (1 + nRegStore*(4*nInS + 10)), sizeof(double));
   
   if (storage==NULL) {
      /* free already alloced storage */
      LWPR_FREE(RF->fixStorage);
      RF->fixStorage=NULL;
      return 0;
   }
   
   #ifdef MATLAB
      if (model->isPersistent) {
         mexMakeMemoryPersistent(RF->varStorage);
         mexMakeMemoryPersistent(RF->fixStorage);
      }
   #endif   
     
   if (((long int)((void *) storage)) & 8) storage++;

   RF->SXresYres = storage; storage+=nInS*nRegStore;
   RF->SSXres    = storage; storage+=nInS*nRegStore;
   RF->U         = storage; storage+=nInS*nRegStore;
   RF->P         = storage; storage+=nInS*nRegStore;

   RF->beta      = storage; storage+=nRegStore;
   RF->SSs2      = storage; storage+=nRegStore;
   RF->SSYres    = storage; storage+=nRegStore;
   RF->H         = storage; storage+=nRegStore;
   RF->r         = storage; storage+=nRegStore;
   RF->sum_w     = storage; storage+=nRegStore;
   RF->sum_e_cv2 = storage; storage+=nRegStore;
   RF->n_data    = storage; storage+=nRegStore;
   RF->lambda    = storage; storage+=nRegStore;
   RF->s         = storage; 
   
   RF->w = RF->beta0 = RF->sum_e2 = 0.0;
   RF->trustworthy = 0;
   RF->slopeReady = 0;

   
   return 1;
}

int lwpr_mem_realloc_rf(LWPR_ReceptiveField *RF, int nRegStore) {
   double *newStorage, *storage;
   int nInS,nReg;
   
   nInS = RF->model->nInStore;
   nReg = RF->nReg;
   
   storage = newStorage = (double *) LWPR_CALLOC((size_t)(1 + nRegStore*(4*nInS + 11)), sizeof(double));
   if (newStorage==NULL) return 0;
      
   if (((long int)((void *) storage)) & 8) storage++;
   
   memcpy(storage, RF->SXresYres, nInS*nReg*sizeof(double)); RF->SXresYres = storage; storage+=nInS*nRegStore;
   memcpy(storage, RF->SSXres,    nInS*nReg*sizeof(double)); RF->SSXres    = storage; storage+=nInS*nRegStore;
   memcpy(storage, RF->U,         nInS*nReg*sizeof(double)); RF->U         = storage; storage+=nInS*nRegStore;
   memcpy(storage, RF->P,         nInS*nReg*sizeof(double)); RF->P         = storage; storage+=nInS*nRegStore;

   memcpy(storage, RF->beta,      nReg*sizeof(double)); RF->beta      = storage; storage+=nRegStore;
   memcpy(storage, RF->SSs2,      nReg*sizeof(double)); RF->SSs2      = storage; storage+=nRegStore;
   memcpy(storage, RF->SSYres,    nReg*sizeof(double)); RF->SSYres    = storage; storage+=nRegStore;
   memcpy(storage, RF->H,         nReg*sizeof(double)); RF->H         = storage; storage+=nRegStore;   
   memcpy(storage, RF->r,         nReg*sizeof(double)); RF->r         = storage; storage+=nRegStore;
   memcpy(storage, RF->sum_w,     nReg*sizeof(double)); RF->sum_w     = storage; storage+=nRegStore;
   memcpy(storage, RF->sum_e_cv2, nReg*sizeof(double)); RF->sum_e_cv2 = storage; storage+=nRegStore;
   memcpy(storage, RF->n_data,    nReg*sizeof(double)); RF->n_data    = storage; storage+=nRegStore;
   memcpy(storage, RF->lambda,    nReg*sizeof(double)); RF->lambda    = storage; storage+=nRegStore;
   memcpy(storage, RF->s,         nReg*sizeof(double)); RF->s         = storage;
   
   LWPR_FREE(RF->varStorage);
   RF->varStorage = newStorage;
   RF->nRegStore = nRegStore;   
#ifdef MATLAB
   if (RF->model->isPersistent) mexMakeMemoryPersistent(RF->varStorage);
#endif   
   return 1;
}

void lwpr_mem_free_rf(LWPR_ReceptiveField *RF) {
   RF->nRegStore = 0;
   
   LWPR_FREE(RF->fixStorage);
   LWPR_FREE(RF->varStorage);
}

int lwpr_mem_alloc_model(LWPR_Model *model, int nIn, int nOut, int storeRFS) {
   int i,nInS;
   double *storage;
   
   nInS = (nIn&1)?(nIn+1):nIn;
   
   model->sub = (LWPR_SubModel *) LWPR_CALLOC((size_t)nOut, sizeof(LWPR_SubModel));
   if (model->sub == NULL) return 0;
      
   model->ws = (LWPR_Workspace *) LWPR_CALLOC(NUM_THREADS, sizeof(LWPR_Workspace));
   if (model->ws == NULL) {
      LWPR_FREE(model->sub);
      return 0;
   }
   
   for (i=0;i<NUM_THREADS;i++) {
      if (!lwpr_mem_alloc_ws(&model->ws[i],nIn)) {
         int j;
         for (j=0;j<i;j++) lwpr_mem_free_ws(&model->ws[j]);
         LWPR_FREE(model->ws);
         LWPR_FREE(model->sub);
         return 0;
      }
   }

   
   storage = (double *) LWPR_CALLOC((size_t)(1 + 2*nOut + nInS*(3*nIn + 4)), sizeof(double));
   if (storage==NULL) {
      LWPR_FREE(model->sub);
      for (i=0;i<NUM_THREADS;i++) lwpr_mem_free_ws(&model->ws[i]);      
      LWPR_FREE(model->ws);
      return 0;
   } 
   model->storage = storage;
   if (((long int)((void *) storage)) & 8) storage++;

   model->mean_x = storage;     storage+=nInS;
   model->var_x = storage;      storage+=nInS;
   model->init_D = storage;     storage+=nInS*nIn;
   model->init_M = storage;     storage+=nInS*nIn;
   model->init_alpha = storage; storage+=nInS*nIn;
   model->norm_in = storage;    storage+=nInS;
   model->xn = storage;         storage+=nInS;
   model->norm_out = storage;   storage+=nOut;
   model->yn = storage;
   
   model->name = NULL;
   
   model->nOut = nOut;
   for (i=0;i<nOut;i++) {
      model->sub[i].n_pruned = 0;   
      model->sub[i].numRFS = 0;
      model->sub[i].numPointers = storeRFS;
      model->sub[i].model = model;
      if (storeRFS>0) {
         model->sub[i].rf = (LWPR_ReceptiveField **) LWPR_CALLOC((size_t)storeRFS, sizeof(LWPR_ReceptiveField *));
         if (model->sub[i].rf == NULL) {
            int j;
            model->sub[i].numPointers = 0;
            for (j=0;j<i;j++) {
               LWPR_FREE(model->sub[j].rf);
               model->sub[j].numPointers = 0;
            }
            LWPR_FREE(model->sub);
            for (i=0;i<NUM_THREADS;i++) lwpr_mem_free_ws(&model->ws[i]);                  
            LWPR_FREE(model->ws);
            LWPR_FREE(model->storage);
            return 0;
         }
      }
   }
   model->nIn = nIn;
   model->nInStore = nInS;
   model->nOut = nOut;
   return 1;
}

int lwpr_mem_alloc_sub(LWPR_SubModel *sub, int storeRFS) {
   sub->n_pruned = 0;   
   sub->numRFS = 0;
   sub->numPointers = storeRFS;
   sub->rf = (LWPR_ReceptiveField **) LWPR_CALLOC((size_t)storeRFS, sizeof(LWPR_ReceptiveField *));
      
   if (sub->rf == NULL) {
      sub->numPointers = 0;
      return 0;
   }
   return 1;
}


void lwpr_free_model(LWPR_Model *model) {
   int i;
   if (model->nOut * model->nIn == 0) return;
   for (i=0;i<model->nOut;i++) {
      int j;
      for (j=0; j < model->sub[i].numRFS; j++) {
         lwpr_mem_free_rf(model->sub[i].rf[j]);
         LWPR_FREE(model->sub[i].rf[j]);
      }
      LWPR_FREE(model->sub[i].rf);
   }
   LWPR_FREE(model->sub);

   for (i=0;i<NUM_THREADS;i++) {
      lwpr_mem_free_ws(&model->ws[i]);
   }
   LWPR_FREE(model->ws);
   LWPR_FREE(model->storage);
   if (model->name != NULL) LWPR_FREE(model->name);
}


int lwpr_mem_alloc_ws(LWPR_Workspace *ws, int nIn) {
   int nInS;
   double *storage;
   
   nInS = (nIn&1) ? nIn+1 : nIn;
  
   ws->derivOk = (int *) LWPR_CALLOC((size_t)nIn,sizeof(int));
   
   if (ws->derivOk == NULL) return 0;
   
   ws->storage = storage = (double *) LWPR_CALLOC((size_t)(1 + 8*nInS*nIn + 7*nInS + 6*nIn), sizeof(double));
   
   if (storage == NULL) {
      LWPR_FREE(ws->derivOk);
      return 0;
   }
   
   if (((long int)((void *) storage)) & 8) storage++;
   ws->dwdM     = storage; storage+=nInS*nIn;
   ws->dJ2dM    = storage; storage+=nInS*nIn;
   ws->ddwdMdM  = storage; storage+=nInS*nIn;
   ws->ddJ2dMdM = storage; storage+=nInS*nIn;
   ws->xres     = storage; storage+=nInS*nIn;   
   ws->dx       = storage; storage+=nInS;
   ws->xu       = storage; storage+=nInS;
   ws->xc       = storage; storage+=nInS;   
   ws->xmz      = storage; storage+=nInS;      
   
   ws->dsdx     = storage; storage+=nInS*nIn;      
   ws->Dx       = storage; storage+=nInS;
   /* The following variables are needed for calculating 
   ** gradients and Hessians of the predictions.
   ** In theory they could use the same space as, say, dwdM etc.
   ** However, small savings in memory consumption is probably
   ** not worth the hassle */
   ws->sum_dwdx        = storage; storage+=nInS;
   ws->sum_ydwdx_wdydx = storage; storage+=nInS;   
   ws->sum_ddwdxdx     = storage; storage+=nInS*nIn;      
   ws->sum_ddRdxdx     = storage; storage+=nInS*nIn;            
   
   /* needs only nReg storage (<=nIn), no alignment necessary */
   ws->e_cv     = storage; storage+=nIn;   
   ws->Ps       = storage; storage+=nIn;   
   ws->Pse      = storage; storage+=nIn;      
   ws->ytarget  = storage; storage+=nIn;         
   ws->yres     = storage; storage+=nIn;            
   ws->s        = storage; storage+=nIn;            
   
   return 1;
}

void lwpr_mem_free_ws(LWPR_Workspace *ws) {
   LWPR_FREE(ws->derivOk);
   LWPR_FREE(ws->storage);
}
