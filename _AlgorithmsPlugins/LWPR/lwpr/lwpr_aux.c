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
#include <stdio.h>

#if NUM_THREADS > 1
   #ifdef WIN32
      #include <windows.h>
   #else
      #include <pthread.h> 
   #endif
#endif

void lwpr_aux_dist_derivatives(int nIn,int nInS,double *dwdM, double *dJ2dM, double *ddwdMdM, double *ddJ2dMdM,
         double w, double dwdq, double ddwdqdq, 
         const double *RF_D, const double *RF_M, const double *dx,
         int diag_only, double penalty, int meta) {
         
   int m,n;
   /* Fill elements (n,m) */ 
   
   /* penalty only occurs with a factor 2, so we take it out */
   penalty+=penalty;
   
   if (diag_only) {
      if (meta) {
         /* diagonal case WITH meta learning */         
         for (n=0;n<nIn;n++) {
            int n_n = n + n*nInS;
            /* take the derivative of q=dx'*D*dx with respect to nn_th element of M */

            double aux = 2.0 * RF_M[n_n];
            double dqdM_nn = dx[n] * dx[n] * aux;

            dwdM[n_n] = dqdM_nn * dwdq;
            ddwdMdM[n_n] = ddwdqdq*dqdM_nn*dqdM_nn + 2*dwdq*dx[n]*dx[n];

            dJ2dM[n_n] = penalty * RF_D[n_n] * aux;
            ddJ2dMdM[n_n] = penalty*(2*RF_D[n_n] + aux*aux);
         }      
      } else {
         /* diagonal case WITHOUT meta learning */               
         for (n=0;n<nIn;n++) {
            int n_n = n + n*nInS;
            /* take the derivative of q=dx'*D*dx with respect to nn_th element of M */            

            double aux = 2.0 * RF_M[n_n];

            dwdM[n_n] = dx[n] * dx[n] * aux * dwdq;
            dJ2dM[n_n] = penalty * RF_D[n_n] * aux;
         }
      }
      return;
   }
   
   if (meta) {
      /* non-diagonal (= upper-triangular) case WITH meta learning*/   
      for (n=0;n<nIn;n++) {
         for (m=n;m<nIn;m++) {
            double sum_aux = 0.0;
            double sum_aux1 = 0.0;
            double dqdM_nm = 0.0;
            int i;

            /* take the derivative of q = dx'*D*dx with respect to nm_th element of M */            

            for (i=n;i<nIn;i++) {
               /* aux corresponds to the in_th (= ni_th) element of dDdM_nm  
                  this is directly processed for dwdM and dJ2dM   */
               
               double M_ni = RF_M[n+i*nInS];
               dqdM_nm += dx[i] * M_ni;      /* additional factor 2.0*dx[m] comes after the loop */
               sum_aux += RF_D[i + m*nInS] * M_ni;                         
               
               if (i == m) {                                                  
                  sum_aux1 += 2.0*M_ni*M_ni;
               } else {               
                  sum_aux1 += M_ni*M_ni;
               }
            }
            dqdM_nm *= 2.0*dx[m];
            
            dwdM[n+m*nInS] = dqdM_nm * dwdq;
            ddwdMdM[n+m*nInS] = ddwdqdq * dqdM_nm * dqdM_nm + 2*dwdq*dx[m]*dx[m];
                        
            dJ2dM[n+m*nInS] = 2.0*penalty*sum_aux;
            ddJ2dMdM[n+m*nInS] = 2.0*penalty*(RF_D[m+m*nInS] + sum_aux1);
        }
      }
   } else {
      /* non-diagonal (= upper-triangular) case WITHOUT meta learning*/   
      for (n=0;n<nIn;n++) {
         for (m=n;m<nIn;m++) {
            double sum_aux = 0.0;
            double dqdM_nm = 0.0;
            int i;

            /* take the derivative of q = dx'*D*dx with respect to nm_th element of M */
            for (i=n;i<nIn;i++) {
               /* aux corresponds to the i,n_th (= n,i_th) element of dDdm_nm  
                  this is directly processed for dwdM and dJ2dM   */
               double M_ni = RF_M[n+i*nInS];
               dqdM_nm += dx[i] * M_ni;       /* additional factor 2.0*dx[m] comes after the loop */        
               sum_aux += RF_D[i + m*nInS] * M_ni;                         
            }
            dwdM[n+m*nInS] = 2.0 * dx[m] * dqdM_nm * dwdq;
            dJ2dM[n+m*nInS] = 2.0 * penalty * sum_aux;
         }
      }
   }
}


/*
typedef double v2df __attribute__ ((vector_size (16)));


void lwpr_aux_update_b_h_alpha(double *b,double *h, double *alpha, 
            const double *dwdM, const double *dJ2dM, const double *ddwdMdM, const double *ddJ2dMdM,
            const double *wW, const double *dJ1dw, const double *ddJ1dwdw, 
            const double *meta_rate, const double *transMul, int nIn, int nInS) {
   int i,j;
   
   static v2df aux_min = {-0.1, -0.1};
   static v2df aux_max = { 0.1,  0.1};
   static v2df b_min = {-10.0, -10.0};
   static v2df b_max = { 10.0,  10.0};
   static v2df ones  = {1.0, 1.0};
   static v2df zeros = {0.0, 0.0};
   
      
   v2df transMul_2, metaTrans, ddJ1dwdw_2, dJ1dw_2, wW_2;
   
   transMul_2 = __builtin_ia32_loadlpd(transMul_2, transMul);
   transMul_2 = __builtin_ia32_loadhpd(transMul_2, transMul);   
   
   metaTrans = __builtin_ia32_loadlpd(metaTrans, meta_rate);
   metaTrans = __builtin_ia32_loadhpd(metaTrans, meta_rate);      
   metaTrans = __builtin_ia32_mulpd(metaTrans, transMul_2);
   
   dJ1dw_2 = __builtin_ia32_loadlpd(dJ1dw_2, dJ1dw);
   dJ1dw_2 = __builtin_ia32_loadhpd(dJ1dw_2, dJ1dw);   
   
   ddJ1dwdw_2 = __builtin_ia32_loadlpd(ddJ1dwdw_2, ddJ1dwdw);
   ddJ1dwdw_2 = __builtin_ia32_loadhpd(ddJ1dwdw_2, ddJ1dwdw);   
   
   wW_2 = __builtin_ia32_loadlpd(wW_2, wW);
   wW_2 = __builtin_ia32_loadhpd(wW_2, wW);   
      
   for (j=0;j<nIn;j++) {
      for (i=0;i<=j;i+=2) {
         v2df aux_i;
         v2df b_i;
         v2df alpha_i;
         v2df h_i;
         v2df dJ2dM_i;

         v2df dwdM_i;
         v2df ddJdMdM;

         dwdM_i = __builtin_ia32_loadupd(dwdM+i);
         ddJdMdM = __builtin_ia32_loadupd(ddJ2dMdM+i);
         ddJdMdM = __builtin_ia32_mulpd(wW_2, ddJdMdM);

         aux_i = __builtin_ia32_loadupd(ddwdMdM+i);
         aux_i = __builtin_ia32_mulpd(aux_i, dJ1dw_2);
         ddJdMdM = __builtin_ia32_addpd(ddJdMdM, aux_i);

         aux_i = __builtin_ia32_loadupd(dwdM+i);
         aux_i = __builtin_ia32_mulpd(aux_i, aux_i);
         aux_i = __builtin_ia32_mulpd(aux_i, ddJ1dwdw_2);
         ddJdMdM = __builtin_ia32_addpd(ddJdMdM, aux_i);

         h_i = __builtin_ia32_loadupd(h+i);
         dJ2dM_i = __builtin_ia32_loadupd(dJ2dM+i);
         aux_i = __builtin_ia32_mulpd(dJ2dM_i,h_i);
         aux_i = __builtin_ia32_mulpd(aux_i,metaTrans);      

         aux_i = __builtin_ia32_maxpd(aux_i, aux_min);
         aux_i = __builtin_ia32_minpd(aux_i, aux_max);      

         b_i = __builtin_ia32_loadupd(b+i);
         b_i = __builtin_ia32_subpd(b_i, aux_i);

         b_i = __builtin_ia32_maxpd(b_i, b_min);
         b_i = __builtin_ia32_minpd(b_i, b_max);      

         __builtin_ia32_storeupd(b+i, b_i);

         alpha[i] = exp(b[i]);
         alpha[i+1] = exp(b[i+1]);

         alpha_i = __builtin_ia32_loadupd(alpha+i);
         alpha_i = __builtin_ia32_mulpd(alpha_i, transMul_2);      

         aux_i = __builtin_ia32_mulpd(alpha_i, ddJdMdM);
         aux_i = __builtin_ia32_subpd(ones, aux_i);

         aux_i = __builtin_ia32_maxpd(aux_i, zeros);

         h_i = __builtin_ia32_mulpd(h_i, aux_i);

         alpha_i = __builtin_ia32_mulpd(alpha_i, dJ2dM_i);
         h_i = __builtin_ia32_subpd(h_i, alpha_i);

         __builtin_ia32_storeupd(h+i, h_i);
      }
      h += nInS;
      b += nInS;
      alpha += nInS;
      dwdM += nInS;
      ddwdMdM += nInS;
      dJ2dM += nInS;
      ddJ2dMdM += nInS;
   }
}
*/


double lwpr_aux_update_distance_metric(LWPR_ReceptiveField *RF, 
      double w, double dwdq, double ddwdqdq, double e_cv, double e, const double *xn, LWPR_Workspace *WS) {
      
   double transMul;
   double penalty;
   
   int nInS = RF->model->nInStore;
   int nIn = RF->model->nIn;
   int nR = RF->nReg;
   
   int *derivOk = WS->derivOk;

   double *Ps = WS->Ps;
   double *Pse = WS->Pse;
   
   double *dwdM = WS->dwdM;
   double *dJ2dM = WS->dJ2dM;
   double *ddwdMdM = WS->ddwdMdM;
   double *ddJ2dMdM = WS->ddJ2dMdM;
   double *dx = WS->dx;
   
   double h=0.0;
   double e2;
   double e_cv2;
   
   double W,E;
   double dJ1dw;
   double maxM;
   double wW;
   
   int reduced = 0;
      
   int i,j;
   
   penalty = RF->model->penalty / RF->model->nIn;
   
   for (i=0;i<nR;i++) {
      derivOk[i] = (RF->n_data[i]*(1.0 - RF->lambda[0]) > 0.1) ? 1:0;
   }
   
   if (!derivOk[0]) return 0.0;
    
   e2 = e*e;
   e_cv2 = e_cv*e_cv;
   
   for (i=0;i<nR;i++) {
      if (derivOk[i]) h+=RF->s[i]*RF->s[i]/RF->SSs2[i];
   }
   h*=w;
   W = RF->sum_w[0];   /* has already been updated in update_regression */
   if (nR==1) {
      E = RF->sum_e_cv2[0];
   } else {
      if (RF->n_data[nR-1]>2*nIn) {
	      E = RF->sum_e_cv2[nR-1];
      } else {
         E = RF->sum_e_cv2[nR-2];
      }
   }
   transMul = RF->sum_e2/(E+1E-10); /* to the 4th power ... */
   transMul*=transMul;   transMul*=transMul; 
   
   if (transMul>1.0) transMul = 1.0;
   
   dJ1dw = -E/W + e_cv2; /* another division by W comes later */
   for (i=0;i<nR;i++) {
      if (derivOk[i]) {
         Ps[i] = RF->s[i]/RF->SSs2[i];
         Pse[i] = e*Ps[i];
         dJ1dw -= 2.0*Pse[i]*RF->H[i] + 2.0*Ps[i]*Ps[i]*RF->r[i];
      } else {
         Ps[i] = Pse[i] = 0.0;
      }
   }
   dJ1dw/=W;
   
   wW = w/W;
   
   for (i=0;i<nIn;i++) dx[i]=xn[i]-RF->c[i];
     
   lwpr_aux_dist_derivatives(nIn, nInS, dwdM, dJ2dM, ddwdMdM, ddJ2dMdM, w, dwdq, ddwdqdq, RF->D, RF->M, dx, RF->model->diag_only, penalty, RF->model->meta);  
   
   if (RF->model->diag_only) {
   
      maxM = 0.0;
      for (j=0;j<nIn;j++) {   
         double m = fabs(RF->M[j+j*nInS]);
         if (m>maxM) maxM=m;
      }
      
      for (j=0;j<nIn;j++) {
         int off = j + j*nInS;         
         dJ2dM[off] = wW * dJ2dM[off] + dwdM[off]*dJ1dw;
      }
      
      if (RF->model->meta) {
         /* Reuse dJ2dM as dJdM */

         double ddJ1dwdw;
         double sPse = 0.0;
         
         for (i=0;i<nR;i++) sPse+=RF->s[i]*Pse[i];

         ddJ1dwdw = 2.0*(e2*h/w - e_cv2/W) + E/(W*W); /* another division by W comes later */
         for (i=0;i<nR;i++) {
            ddJ1dwdw += 4.0 * (Pse[i]/W + Ps[i]*sPse) * RF->H[i];
         }
         ddJ1dwdw/=W;
         
         for (j=0;j<nIn;j++) {
            int off = j + j*nInS;         
            double ddJdMdM_jj = wW * ddJ2dMdM[off] + ddwdMdM[off]*dJ1dw + dwdM[off]*dwdM[off] * ddJ1dwdw;
            double aux_jj;
            double b_jj;
            double alpha_jj;
            
            /* This implements the incremental Delta-Bar-Delta algorithm (Sutton, 1992)
               with some additional safety heuristics */
            aux_jj = RF->model->meta_rate * transMul * dJ2dM[off] * RF->h[off];

            if (aux_jj > 0.1) {
               aux_jj = 0.1; 
            } else if (aux_jj < -0.1) {
               aux_jj = -0.1;
            }

            b_jj = RF->b[off] - aux_jj;
            if (b_jj > 10) {
               b_jj = 10;
            } else if (b_jj < -10) {
               b_jj = -10;
            }
            RF->b[off] = b_jj;
            alpha_jj = exp(b_jj);
            RF->alpha[off] = alpha_jj;

            aux_jj = 1.0 - alpha_jj*ddJdMdM_jj*transMul;
            if (aux_jj < 0) aux_jj = 0;
            RF->h[off] = RF->h[off] * aux_jj - alpha_jj* transMul * dJ2dM[off];
         }
      }

      for (j=0;j<nIn;j++) {   
         int off = j + j*nInS;                     
         double delta_M_jj = RF->alpha[off] * transMul * dJ2dM[off];
         if (delta_M_jj > 0.1*maxM) {
            RF->alpha[off]*=0.5;
            reduced = 1;
         } else {
            RF->M[off] -= delta_M_jj;
         }
      }

      for (j=0;j<nIn;j++) {   
         RF->D[j+j*nInS] = RF->M[j+j*nInS] * RF->M[j+j*nInS];
      }
   
   } else {
      /* Full distance matrix (non-diagonal) case */
      maxM = 0.0;
      for (j=0;j<nIn;j++) {   
         for (i=0;i<=j;i++) {
            double m = fabs(RF->M[i+j*nInS]);
            if (m>maxM) maxM=m;
         }
      }
   
      /* Reuse dJ2dM as dJdM */
      for (j=0;j<nIn;j++) {   
         /* for (i=0;i<=j;i++) dJ2dM[i+j*nInS] = wW * dJ2dM[i+j*nInS] + dwdM[i+j*nInS]*dJ1dw; */  
         lwpr_math_scale_add_scalar_vector(wW, dJ2dM + j*nInS, dJ1dw, dwdM + j*nInS, j+1);
      }

      if (RF->model->meta) {
         double ddJ1dwdw;
         double sPse = 0.0;
         for (i=0;i<nR;i++) sPse+=RF->s[i]*Pse[i];

         ddJ1dwdw = 2.0*(e2*h/w - e_cv2/W) + E/(W*W); /* another division by W comes later */
         for (i=0;i<nR;i++) {
            ddJ1dwdw += 4.0 * (Pse[i]/W + Ps[i]*sPse) * RF->H[i];
         }
         ddJ1dwdw/=W;

         /* Reuse ddJ2dMdM as ddJdMdM */

         /* SSE2 routine
         lwpr_aux_update_b_h_alpha(RF->b, RF->h, RF->alpha, dwdM, dJ2dM, ddwdMdM, ddJ2dMdM, 
                  &wW, &dJ1dw, &ddJ1dwdw, &(RF->model->meta_rate), &transMul, nIn, nInS);
         */

         for (j=0;j<nIn;j++) {   
            for (i=0;i<=j;i++) {
               int off = i+j*nInS;
               double aux_ij;
               double b_ij;
               double alpha_ij;
               double dwdM_ij = dwdM[off];
               double ddJdMdM_ij = wW * ddJ2dMdM[off] + ddwdMdM[off]*dJ1dw + dwdM_ij*dwdM_ij * ddJ1dwdw;

               /* This implements the incremental Delta-Bar-Delta algorithm (Sutton, 1992),
                  with some additional safety heuristics */
               
               aux_ij = RF->model->meta_rate * transMul * dJ2dM[off] * RF->h[off];
               if (aux_ij > 0.1) {
                  aux_ij = 0.1; 
               } else if (aux_ij < -0.1) {
                  aux_ij = -0.1;
               }

               b_ij = RF->b[off] - aux_ij;
               if (b_ij > 10.0) {
                  b_ij = 10.0;
               } else if (b_ij < -10.0) {
                  b_ij = -10.0;
               }

               RF->b[off] = b_ij;
               alpha_ij = exp(b_ij);
               RF->alpha[off] = alpha_ij;

               aux_ij = 1.0 - alpha_ij*ddJdMdM_ij*transMul;
               if (aux_ij < 0) aux_ij = 0;
               RF->h[off] = RF->h[off] * aux_ij - alpha_ij* transMul * dJ2dM[off];
            }
         }
      }


      for (j=0;j<nIn;j++) {   
         for (i=0;i<=j;i++) {
            double delta_M_ij = RF->alpha[i+j*nInS] * transMul * dJ2dM[i+j*nInS];
            if (delta_M_ij > 0.1*maxM) {
               reduced = 1;
               RF->alpha[i+j*nInS]*=0.5;
            } else {
               RF->M[i+j*nInS] -= delta_M_ij;
            }
         }
      }

      for (j=0;j<nIn;j++) {   
         /* Calculate in lower triangle, fill upper */
         for (i=0;i<j;i++) {   
            RF->D[i+j*nInS] = RF->D[j+i*nInS];
         }
         for (i=j;i<nIn;i++) {
            RF->D[i+j*nInS] = lwpr_math_dot_product(RF->M + i*nInS, RF->M + j*nInS,j+1);
         }
      }
   }
   
   for (i=0;i<nR;i++) {
      if (derivOk[i]) {
         RF->H[i] = RF->lambda[i] * RF->H[i] + (w/(1-h))*RF->s[i]*e_cv*transMul;
         RF->r[i] = RF->lambda[i] * RF->r[i] + (w*w*e_cv2/(1-h))*RF->s[i]*RF->s[i]*transMul;
      }
   }
   #ifdef MATLAB
      if (reduced) printf("Reduced learning rate.\n"); 
   #endif
   return transMul; 
}

LWPR_ReceptiveField *lwpr_aux_add_rf(LWPR_SubModel *sub, int nReg) {
   LWPR_ReceptiveField *RF;
   
   if (sub->numRFS == sub->numPointers) {
      LWPR_ReceptiveField **newStore = (LWPR_ReceptiveField **) LWPR_REALLOC(sub->rf, (sub->numPointers+16)*sizeof(LWPR_ReceptiveField *));
      if (newStore == NULL) return NULL;      

      sub->rf = newStore;
      sub->numPointers+=16;
      
      #ifdef MATLAB
         if (sub->model->isPersistent) mexMakeMemoryPersistent(sub->rf);
      #endif   
   }
   
   RF = (LWPR_ReceptiveField *) LWPR_MALLOC(sizeof(LWPR_ReceptiveField));
   if (RF == NULL) return NULL;
   #ifdef MATLAB
      if (sub->model->isPersistent) mexMakeMemoryPersistent(RF);
   #endif   
   
   if (nReg > 0) {
      int nRegStore = (nReg > LWPR_REGSTORE) ? nReg : LWPR_REGSTORE;
      lwpr_mem_alloc_rf(RF, sub->model, nReg, nRegStore);
   } else {
      memset(RF, 0, sizeof(LWPR_ReceptiveField));
   }
   
   sub->rf[sub->numRFS++]=RF;
   
   return RF;  
}


/* returns ymz. xmz is also output value */
double lwpr_aux_update_means(LWPR_ReceptiveField *RF, const double *x, double y, double w, double *xmz) {
   int i;
   int nIn = RF->model->nIn;
   double swl = RF->sum_w[0] * RF->lambda[0];
   double invFac = 1.0/(swl + w);
   
   for (i=0;i<nIn;i++) {
      double mx = RF->mean_x[i];
      mx = RF->mean_x[i] = (swl * mx + w*x[i])*invFac;
      mx = xmz[i] = x[i] - mx;
      RF->var_x[i] = (swl * RF->var_x[i] + w*mx*mx)*invFac;
   }
   RF->beta0 = (swl * RF->beta0 + w*y)*invFac;
   return (y-RF->beta0);
}


void lwpr_aux_compute_projection_r(int nIn, int nInS, int nReg, 
      double *s, double *xres, const double *x, const double *U, const double *P) {
      
   int i,j;
   double sj;
   
   for (i=0;i<nIn;i++) xres[i] = x[i];
   for (j=0;j<nReg-1;j++) {
      s[j] = sj = lwpr_math_dot_product(U+j*nInS, xres + j*nInS, nIn);
      for (i=0;i<nIn;i++) {
         xres[i + (j+1)*nInS] = xres[i + j*nInS] - P[i+j*nInS]*sj;
      }
   }
   s[nReg-1] = lwpr_math_dot_product(U+(nReg-1)*nInS, xres + (nReg-1)*nInS, nIn);          
}


void lwpr_aux_compute_projection(int nIn, int nInS, int nReg, 
      double *s, const double *x, const double *U, const double *P, LWPR_Workspace *WS) {
      
   int i,j;
   double sj;
   double *xu = WS->xu; 
   
   for (i=0;i<nIn;i++) xu[i] = x[i];
   
   for (j=0;j<nReg-1;j++) {
      s[j] = sj = lwpr_math_dot_product(U+j*nInS, xu, nIn);
      /* for (i=0;i<nIn;i++) xu[i] -= P[i+j*nInS]*sj; */
      lwpr_math_add_scalar_vector(xu,-sj,P+j*nInS, nIn);
   }
   s[nReg-1]=lwpr_math_dot_product(U+(nReg-1)*nInS, xu, nIn);           
}

void lwpr_aux_compute_projection_d(int nIn, int nInS, int nReg, 
      double *s, double *dsdx, const double *x, const double *U, const double *P, LWPR_Workspace *WS) {
      
   int i,j;
   double sj;
   double *xu = WS->xu; 
   double *dxdx = WS->dwdM;  /* it's also an nIns x nIn matrix */
   
   memset(dxdx,0,nInS*nIn*sizeof(double));
   for (i=0;i<nIn;i++) xu[i] = x[i];
   for (i=0;i<nIn;i++) dxdx[i+i*nInS] = 1.0;
   
   for (j=0;j<nReg-1;j++) {
      s[j] = sj = lwpr_math_dot_product(U+j*nInS, xu, nIn);
      
      /* dsdx(:,j) = dxdx * U(:,j); */
      lwpr_math_scalar_vector(dsdx + j*nInS, U[j*nInS], dxdx, nIn);
      for (i=1;i<nIn;i++) {
         lwpr_math_add_scalar_vector(dsdx + j*nInS, U[i+j*nInS], dxdx + i*nInS, nIn);
      }
      /* for (i=0;i<nIn;i++) xu[i] -= P[i+j*nInS]*sj; */
      lwpr_math_add_scalar_vector(xu,-sj,P+j*nInS,nIn);

      /* dxdx = dxdx - dsdx(:,j) * P(:,j)'; */
      for (i=0;i<nIn;i++) {
         lwpr_math_add_scalar_vector(dxdx + i*nInS, -P[i+j*nInS], dsdx + j*nInS, nIn);
      }
   }
   j=nReg-1;
   s[j]=lwpr_math_dot_product(U+j*nInS, xu, nIn);  
   lwpr_math_scalar_vector(dsdx + j*nInS, U[j*nInS], dxdx, nIn);
   for (i=1;i<nIn;i++) {
      lwpr_math_add_scalar_vector(dsdx + j*nInS, U[i+j*nInS], dxdx + i*nInS, nIn);
   }         
}

void lwpr_aux_update_regression(LWPR_ReceptiveField *RF, double *yp, double *e_cv_R, double *e,
   const double *x, double y, double w, LWPR_Workspace *WS) {
   
   int nIn = RF->model->nIn;
   int nInS = RF->model->nInStore;
   int nReg = RF->nReg;
   
   double *yres = WS->yres; 
   double *ytarget = WS->ytarget; 
   double *e_cv = WS->e_cv;
   double *xres = WS->xres; 
   double ypred = 0.0;
   double ws2_SSs2 = 0.0;
   int i,j;
   
   lwpr_aux_compute_projection_r(nIn,nInS,nReg,RF->s,xres,x,RF->U,RF->P);

   yres[0] = RF->beta[0] * RF->s[0];
   for (i=1;i<nReg;i++) {
      yres[i] = RF->beta[i] * RF->s[i] + yres[i-1];
   }

   for (i=0;i<nReg;i++) {
      RF->sum_w[i] = RF->sum_w[i] * RF->lambda[i] + w;   
      e_cv[i] = y - yres[i];
   }

   ytarget[0] = y;
   for (i=0;i<nReg-1;i++) {
      ytarget[i+1] = e_cv[i];
   }
   
   for (j=0;j<nReg;j++) {
      int jN = j*nInS;
      double lambda_slow = 0.9 + 0.1*RF->lambda[j];
      double wytar = w * ytarget[j];
      double Unorm = 0.0;
      double wsj = w*RF->s[j];
      double inv_SSs2j;
     
      for (i=0;i<nIn;i++) {
         double help = RF->SXresYres[i+jN] = RF->SXresYres[i+jN] * lambda_slow
               + wytar * xres[i+jN];
         Unorm += help*help;
      }
      
      /* Numerical safety measure */
      if (Unorm > 1e-24) {
         Unorm = 1.0/sqrt(Unorm);

         /* for (i=0;i<nIn;i++) RF->U[i+j*nInS] = Unorm*RF->SXresYres[i+j*nInS]; */
         lwpr_math_scalar_vector(RF->U + jN, Unorm, RF->SXresYres + jN,nIn); 
      }
      
      RF->SSs2[j] = RF->lambda[j]*RF->SSs2[j] + RF->s[j]*wsj;
      RF->SSYres[j] = RF->lambda[j]*RF->SSYres[j] + ytarget[j]*wsj;
      
      /* for (i=0;i<nIn;i++) RF->SSXres[i+j*nInS] = RF->lambda[j]*RF->SSXres[i+j*nInS] + wsj*xres[i+j*nInS]; */
      lwpr_math_scale_add_scalar_vector(RF->lambda[j], RF->SSXres + jN, wsj, xres + jN, nIn); 

      inv_SSs2j = 1.0/RF->SSs2[j];
      RF->beta[j] = RF->SSYres[j] * inv_SSs2j;
      
      /* for (i=0;i<nIn;i++) RF->P[i+j*nInS] = RF->SSXres[i+j*nInS] * inv_SSs2j; */
      lwpr_math_scalar_vector(RF->P + jN, inv_SSs2j, RF->SSXres + jN,nIn);       
      
      ws2_SSs2 += wsj * wsj * inv_SSs2j;
   }
   
   RF->SSp = RF->lambda[nReg-1]*RF->SSp + ws2_SSs2;
   
   lwpr_aux_compute_projection(nIn,nInS,nReg,RF->s,x,RF->U,RF->P,WS);
   
   /* new addition: do not include last PLS dimension if not trustworthy yet */
   /* TODO: check stuff below, in particular e_cv_R */
   if (RF->n_data[nReg-1] > 2.0*nIn) {
      for (j=0;j<nReg;j++) ypred+=RF->beta[j] * RF->s[j];
      *e_cv_R = e_cv[nReg-1];
   } else {
      for (j=0;j<nReg-1;j++) ypred+=RF->beta[j] * RF->s[j];
      *e_cv_R = e_cv[nReg-2];
   }

   *e = y-ypred;
   
   if (RF->n_data[0]*(1.0-RF->lambda[0]) > 0.1) {
      RF->sum_e2 = RF->sum_e2 * RF->lambda[nReg-1] + w*(*e)*(*e);
      for (i=0;i<nReg;i++) {
         RF->sum_e_cv2[i] = RF->sum_e_cv2[i]*RF->lambda[i] + w*e_cv[i]*e_cv[i];   
      }
   }
   
   *yp = ypred + RF->beta0;
   
   if (RF->n_data[0] > 2.0*nIn) RF->trustworthy = 1;
   RF->slopeReady = 0;
   
}


int lwpr_aux_check_add_projection(LWPR_ReceptiveField *RF) {
   int nReg = RF->nReg;
   int nIn = RF->model->nIn;
   int nInS = RF->model->nInStore;
   double mse_n_reg, mse_n_reg_1;

   if (nReg >= nIn) return 0; /* already at full complexity */
   
   mse_n_reg = RF->sum_e_cv2[nReg-1] / RF->sum_w[nReg-1] + 1e-10;
   mse_n_reg_1 = RF->sum_e_cv2[nReg-2] / RF->sum_w[nReg-2] + 1e-10;   
   
   if ((mse_n_reg < RF->model->add_threshold*mse_n_reg_1)
      && (RF->n_data[nReg-1] > 0.99*RF->n_data[0] ) 
      && (RF->n_data[nReg-1]*(1.0-RF->lambda[nReg-1]) > 0.5)) {
      
        
      if (nReg == RF->nRegStore) {
         /* try to enlarge the RF, if this fails, return */
         if (!lwpr_mem_realloc_rf(RF,nReg + LWPR_REGINCR)) return -1;
      }
      /* lwpr_mem_realloc_rf will have initialised the new elements to 0 
      ** So we just fill in non-zero elements */
      
      RF->SSs2[nReg] = RF->model->init_S2;
      RF->U[nReg + nReg*nInS] = 1.0;
      RF->P[nReg + nReg*nInS] = 1.0;
      RF->sum_w[nReg]=1e-10;
      RF->lambda[nReg]=RF->model->init_lambda;
      RF->nReg = nReg+1;
      
      RF->SSp = 0.0;
      return 1;
   } else {
      return 0;
   }
}

int lwpr_aux_init_rf(LWPR_ReceptiveField *RF, const LWPR_Model *model, const LWPR_ReceptiveField *RFT, const double *xc, double y) {
   int i,j,nReg, nRegStore;
   int nIn = model->nIn;
   int nInS = model->nInStore;
   
   if (RFT==NULL) {
      nReg = (nIn>1)? 2:1;
      nRegStore = (nReg > LWPR_REGSTORE) ? nReg : LWPR_REGSTORE;
      if (!lwpr_mem_alloc_rf(RF, model, nReg, nRegStore)) return 0;
      
      memcpy(RF->D, model->init_D, nInS*nIn*sizeof(double));
      memcpy(RF->M, model->init_M, nInS*nIn*sizeof(double));
      memcpy(RF->alpha, model->init_alpha, nInS*nIn*sizeof(double));      
      RF->beta0 = y;
   } else {
      nReg = RFT->nReg;
      nRegStore = RFT->nRegStore;

      if (!lwpr_mem_alloc_rf(RF, model, nReg, nRegStore)) return 0;
      
      memcpy(RF->D, RFT->D, nInS*nIn*sizeof(double));
      memcpy(RF->M, RFT->M, nInS*nIn*sizeof(double));
      memcpy(RF->alpha, RFT->alpha, nInS*nIn*sizeof(double));      
      RF->beta0 = RFT->beta0;
   }
   /* lwpr_mem_alloc_rf has initialised all elements to zero */
   memcpy(RF->c, xc, nIn*sizeof(double));
   
   RF->trustworthy = 0;
   RF->w = 0.0;
   RF->sum_e2 = 0.0;
   RF->SSp = 0.0;
   RF->model = model;
   
   for (i=0;i<nReg;i++) {
      RF->SSs2[i] = model->init_S2;
      RF->U[i+i*nInS] = 1.0;
      RF->P[i+i*nInS] = 1.0;
      RF->sum_w[i] = 1e-10;
      RF->n_data[i] = 1e-10;
      RF->lambda[i] = model->init_lambda;
   }
   for (j=0;j<nIn;j++) {
      for (i=0;i<=j;i++) {
         RF->b[i+j*nInS] = log(RF->alpha[i+j*nInS] + 1e-10);
      }
   }
   return 1;   
}

void lwpr_aux_update_model_stats(LWPR_Model *model, const double *x) {
   double *mx = model->mean_x;
   double *vx = model->var_x;
   double n = (double) model->n_data;
   double invN1 = 1.0/((double) (++model->n_data));
   int nIn = model->nIn;
   int i;
   
   for (i=0;i<nIn;i++) {
      double aux;
      mx[i] = (mx[i]*n + x[i])*invN1;
      aux = x[i] - mx[i];
      vx[i] = (vx[i]*n + aux*aux)*invN1;
   }
}



void *lwpr_aux_update_one_T(void *ptr) {
   LWPR_ThreadData *TD = (LWPR_ThreadData *) ptr;
   LWPR_SubModel *sub = &(TD->model->sub[TD->dim]);
   LWPR_Workspace *WS = TD->ws;
   const LWPR_Model *model = TD->model;
      
   int i,j,n,nIn,nInS;
   double *xc; 
   double e,e_cv; 
  
   double ymz;
   
   double w, w_sec = 0.0, w_max = 0.0;
   int ind = -1, ind_sec = -1, ind_max = -1;
   double yp = 0.0, yp_n;
   
   double sum_w = 0.0;

   double dwdq,ddwdqdq;
   
   nIn = TD->model->nIn;
   nInS = TD->model->nInStore;   
      
   xc = WS->xc;
      
   for (n=TD->start;n<TD->end;n+=TD->incr) {
   
      double dist = 0.0;
      LWPR_ReceptiveField *RF = sub->rf[n];
      
      
      for (i=0;i<nIn;i++) {
         xc[i] = TD->xn[i] - RF->c[i];
      }
      
      for (j=0;j<nIn;j++) {
         dist += xc[j] * lwpr_math_dot_product(RF->D + j*nInS, xc, nIn);
      }
      switch(TD->model->kernel) {
         case LWPR_GAUSSIAN_KERNEL:
            w = exp(-0.5*dist);
            dwdq = -0.5 * w;
            ddwdqdq = 0.25 * w;
            break;
         case LWPR_BISQUARE_KERNEL:
            dwdq = 1-0.25*dist;
            if (dwdq<0) {
               w = dwdq = ddwdqdq = 0.0;
            } else {
               w = dwdq*dwdq;
               ddwdqdq = 0.125;
               dwdq = -0.5*dwdq;
            }
            break;
         default:
            w = dwdq = ddwdqdq = 0;
      }
      
     
      if (w>w_sec) {
         ind = ind_sec;
         if (w>w_max) {
            ind_sec = ind_max;
            w_sec = w_max;            
            ind_max = n;
            w_max = w;
         } else {
            ind_sec = n;
            w_sec = w;
         }
      } else {
         ind = n;
      }
      
      if (w>0.001) {
         double transmul;
         
         RF->w = w;

         ymz = lwpr_aux_update_means(RF,TD->xn,TD->yn,w,WS->xmz);
         lwpr_aux_update_regression(RF, &yp_n, &e_cv, &e, WS->xmz, ymz,w, WS);
         
         if (RF->trustworthy) {
            yp += w*yp_n;
            sum_w += w;
         }
         
         if (model->update_D) {
            transmul = lwpr_aux_update_distance_metric(RF, w, dwdq, ddwdqdq, e_cv, e, TD->xn, WS);
         }
         
         lwpr_aux_check_add_projection(RF);
         
         for (i=0;i<RF->nReg;i++) {
            RF->n_data[i] = RF->n_data[i] * RF->lambda[i] + 1;
            RF->lambda[i] = model->tau_lambda * RF->lambda[i] + model->final_lambda*(1.0-model->tau_lambda);
         }
      } else {
         RF->w = 0.0;
      }
   }

   TD->w_max = w_max;
   TD->ind_max = ind_max;
   TD->w_sec = w_sec;
   TD->ind_sec = ind_sec;
   TD->yp = yp;
   TD->sum_w = sum_w;
   return NULL;
}

int lwpr_aux_update_one_add_prune(LWPR_Model *model, LWPR_ThreadData *TD, int dim, const double *xn, double yn) {
   LWPR_SubModel *sub = &model->sub[dim];   
   
   if (TD->w_max <= model->w_gen) {
      LWPR_ReceptiveField *RF = lwpr_aux_add_rf(sub,0);

      /* Receptive field could not be allocated. The LWPR model is still
         valid, but return "0" to indicate this */      
      if (RF == NULL) return 0;

      if ((TD->w_max > 0.1*model->w_gen) && (sub->rf[TD->ind_max]->trustworthy)) {
         return lwpr_aux_init_rf(RF,model,sub->rf[TD->ind_max], xn, yn);
      }
      return lwpr_aux_init_rf(RF,model,NULL, xn, yn);
   }
   
   /* Prune ReceptiveFields */
   if (TD->w_sec > model->w_prune) {
      double tr_max = 0.0, tr_sec = 0.0;
      int i,prune;
      for (i=0;i<model->nIn;i++) {
         /*
         tr_max += lwpr_math_norm2(sub->rf[TD->ind_max]->M + i*model->nInStore, model->nIn);
         tr_sec += lwpr_math_norm2(sub->rf[TD->ind_sec]->M + i*model->nInStore, model->nIn);
         */
         /* code for just comparing the traces of D */
         tr_max += sub->rf[TD->ind_max]->D[i+i*model->nInStore];
         tr_sec += sub->rf[TD->ind_sec]->D[i+i*model->nInStore];
      }
      /* TODO: ORIGINAL LOGIC WAS REVERSED -- CHECK */
      prune = (tr_max < tr_sec) ? TD->ind_max : TD->ind_sec;
      
      lwpr_mem_free_rf(sub->rf[prune]);
      LWPR_FREE(sub->rf[prune]);
      
      if (prune < sub->numRFS-1) {
         /* Fill the gap with last RF (we just move around the pointer) */      
         sub->rf[prune] = sub->rf[sub->numRFS-1];
      }
      sub->numRFS--;
      sub->n_pruned++;
      
      /* printf("Output %d, pruned RF %d\n",dim+1,prune+1); */
   }
   
   return 1;   
}

int lwpr_aux_update_one(LWPR_Model *model, int dim, const double *xn, double yn, double *y_pred, double *max_w) {
   LWPR_ThreadData TD[NUM_THREADS];
   int i;

#if NUM_THREADS > 1
   #ifdef WIN32
      HANDLE thread[NUM_THREADS-1];
      DWORD ID[NUM_THREADS-1];
   #else
      pthread_t thread[NUM_THREADS-1];
      int rc[NUM_THREADS-1];      
   #endif
#endif

   for (i=0;i<NUM_THREADS;i++) {
      TD[i].model = model;
      TD[i].dim = dim;
      TD[i].xn = xn;
      TD[i].yn = yn;
      TD[i].incr = NUM_THREADS;
      TD[i].start = i;
      TD[i].end = model->sub[dim].numRFS;
      TD[i].ws = &model->ws[i];
   }

#if NUM_THREADS > 1
   #ifdef WIN32
      for (i=0;i<NUM_THREADS-1;i++) {
         thread[i] = CreateThread(NULL,0,lwpr_aux_update_one_T,&TD[i],0, &ID[i]);
      }
   #else
      for (i=0;i<NUM_THREADS-1;i++) {   
         rc[i] = pthread_create(&thread[i], NULL, lwpr_aux_update_one_T, &TD[i]);
      }      
   #endif
#endif

   (void) lwpr_aux_update_one_T(&TD[NUM_THREADS-1]);      
   
#if NUM_THREADS > 1
   /* Wait for other threads to finish, or do their calculations if they
   ** couldn't be spawned in the first place */
   #ifdef WIN32
      for (i=0;i<NUM_THREADS-1;i++) {
         if (thread[i]!=NULL) {
            WaitForSingleObject(thread[i],INFINITE);
            CloseHandle(thread[i]);
         } else {
            (void) lwpr_aux_update_one_T(&TD[i]);       
         }
      }
   #else
      for (i=0;i<NUM_THREADS-1;i++) {
         if (rc[i]==0) {
            pthread_join(thread[i],NULL);
         } else {
            (void) lwpr_aux_update_one_T(&TD[i]);       
         }
      }
   #endif
   
   /* Accumulate statistics in TD[0] */

   for (i=1;i<NUM_THREADS;i++) {
      TD[0].sum_w += TD[i].sum_w;
      TD[0].yp += TD[i].yp;
      if (TD[i].w_max > TD[0].w_max) {
         if (TD[i].w_sec > TD[0].w_max) {
            /* if TD[i].w_sec > "old" w_max, then we have  
               TD[i].w_max and TD[i].w_sec as new largest two values */         
            TD[0].w_max = TD[i].w_max;
            TD[0].ind_max = TD[i].ind_max;
            TD[0].w_sec = TD[i].w_sec;
            TD[0].ind_sec = TD[i].ind_sec;
         } else {
            /* TD[i].w_max > "old" w_max, but TD[i].w_sec is not, that is,
               TD[i].w_max and "old" w_max are now the largest two */
            TD[0].w_sec = TD[0].w_max;
            TD[0].ind_sec = TD[0].ind_max;
            TD[0].w_max = TD[i].w_max;
            TD[0].ind_max = TD[i].ind_max;
         }
      } else {
         /* "old" w_sec < TD[i].w_max < "old" w_max, that is,
             TD[i].w_max gets "new" w_sec */
         if (TD[i].w_max > TD[0].w_sec) {
            TD[0].w_sec = TD[i].w_max;
            TD[0].ind_sec = TD[i].ind_max;
         }
      }
   }
#endif

   if (TD[0].sum_w > 0.0) {
      *y_pred = TD[0].yp/TD[0].sum_w;
   } else {
      *y_pred = 0.0;
   }
   
   if (max_w != NULL) *max_w = TD[0].w_max;
   
   return lwpr_aux_update_one_add_prune(model, &TD[0], dim, xn, yn);
}



/* A note to developers:  lwpr_aux_predict_one_T and lwpr_aux_predict_conf_one_T
** are very similar, and one might wonder why we need two functions.
** This is done for efficieny. Without confidence bounds, we also might
** be able to use readily calculated slopes instead of doing PLS.
**
** Also, please note the difference between threaded updates and predictions:
** The threaded predict-functions are designed to operate on whole submodels,
** whereas the updates break up submodels into multiple threads. Therefore,
** univariate LWPR models (single SubModel) cannot be predicted faster when
** using threads, but the updates might get faster.
** The design rationale was that predictions are fast, and therefore the
** overhead of creating multiple threads per submodel is significant.
** On the other hand, updates are rather slow, and we wish to speed up
** also models with univariate outputs.
*/
void *lwpr_aux_predict_one_T(void *ptr) {
   LWPR_ThreadData *TD = (LWPR_ThreadData *) ptr;
   LWPR_SubModel *sub = &(TD->model->sub[TD->dim]);
   LWPR_Workspace *WS = TD->ws;
   int i,j,n;
   int nIn=TD->model->nIn;
   int nInS=TD->model->nInStore;
   
   double *xc = WS->xc;
   double *s = WS->s;
   
   double w;
   
   double yp = 0.0;
   
   double sum_w = 0.0;
   
   TD->w_max = 0.0;

   for (n=0;n<sub->numRFS;n++) {
      double dist = 0.0;
      LWPR_ReceptiveField *RF = sub->rf[n];

      for (i=0;i<nIn;i++) {
         xc[i] = TD->xn[i] - RF->c[i];
      }

      for (j=0;j<nIn;j++) {
         dist += xc[j] * lwpr_math_dot_product(RF->D + j*nInS, xc, nIn);
      }

      switch(TD->model->kernel) {
         case LWPR_GAUSSIAN_KERNEL:
            w = exp(-0.5*dist);
            break;
         case LWPR_BISQUARE_KERNEL:
            w = 1-0.25*dist;
            w = (w<0) ? 0 : w*w;
            break;
      }

      if (w > TD->w_max) {
         TD->w_max = w;
      }

      if (w > TD->cutoff && RF->trustworthy) {
         double yp_n = RF->beta0;

         for (i=0;i<nIn;i++) {
            xc[i] = TD->xn[i] - RF->mean_x[i];
         }      
         
         if (RF->slopeReady) {   
            yp_n += lwpr_math_dot_product(xc, RF->slope, nIn);
         } else {
            int nR = RF->nReg;
            
            if (RF->n_data[nR-1] <= 2*nIn) nR--;
                        
            lwpr_aux_compute_projection(nIn, nInS, nR, s, xc, RF->U, RF->P, WS);
            
            for (i=0;i<nR;i++) {
               yp_n+=s[i]*RF->beta[i];
            }
         }

         yp += w*yp_n;
         sum_w += w;
      }
   }
   if (sum_w > 0.0) yp/=sum_w;
   TD->yn = yp;
   
   return NULL;
}


void *lwpr_aux_predict_conf_one_T(void *ptr) {
   LWPR_ThreadData *TD = (LWPR_ThreadData *) ptr;
   LWPR_SubModel *sub = &(TD->model->sub[TD->dim]);
   LWPR_Workspace *WS = TD->ws;
   int i,j,n;
   int nIn=TD->model->nIn;
   int nInS=TD->model->nInStore;
   
   double *xc = WS->xc;
   double *s = WS->s;
   
   double w;
  
   
   double sum_w = 0.0;
   double sum_wyy = 0.0;
   double sum_conf = 0.0;
   
   TD->w_max = 0.0;
   TD->yn = 0.0;

   /* Prediction and confidence bounds in one go */
   for (n=0;n<sub->numRFS;n++) {
      double dist = 0.0;
      LWPR_ReceptiveField *RF = sub->rf[n];

      for (i=0;i<nIn;i++) {
         xc[i] = TD->xn[i] - RF->c[i];
      }

      for (j=0;j<nIn;j++) {
         dist += xc[j] * lwpr_math_dot_product(RF->D + j*nInS, xc, nIn);
      }

      switch(TD->model->kernel) {
         case LWPR_GAUSSIAN_KERNEL:
            w = exp(-0.5*dist);
            break;
         case LWPR_BISQUARE_KERNEL:
            w = 1-0.25*dist;
            w = (w<0) ? 0 : w*w;
            break;
      }

      if (w > TD->w_max) {
         TD->w_max = w;
      }

      if (w > TD->cutoff && RF->trustworthy) {
         double yp_n = RF->beta0;
         double sigma2 = 0.0;
         int nR = RF->nReg;
            
         if (RF->n_data[nR-1] <= 2*nIn) nR--;

         for (i=0;i<nIn;i++) {
            xc[i] = TD->xn[i] - RF->mean_x[i];
         }      
         lwpr_aux_compute_projection(nIn, nInS, nR, s, xc, RF->U, RF->P, WS);
         for (i=0;i<nR;i++) {
            yp_n+=s[i]*RF->beta[i];
            sigma2 +=s[i]*s[i] / RF->SSs2[i];
         }
         sigma2 = RF->sum_e_cv2[nR-1]/(RF->sum_w[nR-1] - RF->SSp)*(1+w*sigma2);

         sum_wyy += w*yp_n*yp_n;
         sum_conf += w*sigma2;

         TD->yn += w*yp_n;
         sum_w += w;
      }
   }
   if (sum_w > 0.0) {
      double sum_wy = TD->yn;
      TD->yn /= sum_w;
      TD->w_sec = sqrt(fabs(sum_conf + sum_wyy - sum_wy*TD->yn))/sum_w; /* This serves as confidence bound */
   } else {
      TD->w_sec = 1e20; /* DBL_INFTY; */
   }
   return NULL;
}

double lwpr_aux_predict_one(const LWPR_Model *model, int dim, 
      const double *xn, double cutoff, double *conf, double *max_w) {
      
   LWPR_ThreadData TD;  
   TD.model = model;
   TD.xn = xn;
   TD.ws = &model->ws[0];
   TD.cutoff = cutoff;   
   TD.dim = dim;
   
   if (conf == NULL) {
      (void) lwpr_aux_predict_one_T(&TD);
   } else {
      (void) lwpr_aux_predict_conf_one_T(&TD);
      *conf = TD.w_sec;
   }   
   if (max_w!=NULL) *max_w = TD.w_max;
   return TD.yn;
}      




void *lwpr_aux_predict_one_J_T(void *ptr) {
   LWPR_ThreadData *TD = (LWPR_ThreadData *) ptr;
   LWPR_SubModel *sub = &(TD->model->sub[TD->dim]);
   LWPR_Workspace *WS = TD->ws;

   int i,j,n;
   int nIn=TD->model->nIn;
   int nInS=TD->model->nInStore;
   
   double *xc = WS->xc;
   double *s = WS->s;
   double *dsdx = WS->dsdx;
   double *Dx = WS->Dx;
   double *sum_dwdx = WS->sum_dwdx;
   double *sum_ydwdx_wdydx = WS->sum_ydwdx_wdydx;
     
   double w, dwdq;
   double yp = 0.0;
   
   double sum_w = 0.0;

   memset(sum_dwdx,0,nIn*sizeof(double));
   memset(sum_ydwdx_wdydx,0,nIn*sizeof(double));
         
   for (n=0;n<sub->numRFS;n++) {
      double dist = 0.0;
      LWPR_ReceptiveField *RF = sub->rf[n];
      
      for (i=0;i<nIn;i++) {
         xc[i] = TD->xn[i] - RF->c[i];
      }
      
      for (j=0;j<nIn;j++) {
         Dx[j] = lwpr_math_dot_product(RF->D + j*nInS, xc, nIn);
         dist += xc[j] * Dx[j];
      }
      switch(TD->model->kernel) {
         case LWPR_GAUSSIAN_KERNEL:
            w = exp(-0.5*dist);
            dwdq = -0.5 * w;
            break;
         case LWPR_BISQUARE_KERNEL:
            dwdq = 1-0.25*dist;
            if (dwdq<0) {
               w = dwdq = 0.0;
            } else {
               w = dwdq*dwdq;
               dwdq = -0.5*dwdq;
            }
            break;
         default:
            w = dwdq = 0;
      }
           
      if (w>TD->cutoff && RF->trustworthy) {
         double yp_n = RF->beta0;
         
         for (i=0;i<nIn;i++) {
            xc[i] = TD->xn[i] - RF->mean_x[i];
         }  
         
         sum_w += w;
         
         if (RF->slopeReady) {
            yp_n += lwpr_math_dot_product(xc, RF->slope, nIn);
            yp += w*yp_n;
         } else {
            int nR = RF->nReg;
            
            if (RF->n_data[nR-1] <= 2*nIn) nR--;


            lwpr_aux_compute_projection_d(nIn, nInS, nR, s, dsdx, xc, RF->U, RF->P,WS);
            for (i=0;i<nR;i++) {
               yp_n+=s[i]*RF->beta[i];
            }
            yp += w*yp_n;
            lwpr_math_scalar_vector(RF->slope, RF->beta[0], dsdx, nIn);
            for (i=1;i<nR;i++) {
               lwpr_math_add_scalar_vector(RF->slope, RF->beta[i], dsdx + i*nInS, nIn);
            }
            /*  part of original code without cached slopes:
            for (i=0;i<RF->nReg;i++) {
               lwpr_math_add_scalar_vector(sum_ydwdx_wdydx, w * RF->beta[i], dsdx + i*nInS, nIn);
            }
            */
            RF->slopeReady=1;
         }
         
         lwpr_math_add_scalar_vector(sum_dwdx, 2.0*dwdq, Dx, nIn);
         lwpr_math_add_scalar_vector(sum_ydwdx_wdydx, yp_n*2.0*dwdq, Dx, nIn);
         lwpr_math_add_scalar_vector(sum_ydwdx_wdydx, w, RF->slope, nIn);            
      }
   }
     
   if (sum_w > 0.0) {
      yp/=sum_w;
      
      /* Put gradient into sum_dwdx */
      /* dydx = -(yp/sum_w) * dw/dx  + (1/sum_w)*sum_ydwdx_wdydx) */
      lwpr_math_scale_add_scalar_vector(-yp/sum_w, sum_dwdx, 1.0/sum_w, sum_ydwdx_wdydx, nIn);
      TD->yn = yp;
   } else {
      /* sum_dwdx = 0 */
      /* memset(sum_dwdx,0,nIn*sizeof(double)); */
      TD->yn = 0.0;
   }
   return NULL;
}


double lwpr_aux_predict_one_J(const LWPR_Model *model, int dim, const double *xn, double cutoff, double *dydx) {
   LWPR_ThreadData TD;
   
   TD.model = model;
   TD.dim = dim;
   TD.xn = xn;
   TD.cutoff = cutoff;
   TD.ws = &model->ws[0];
   
   (void) lwpr_aux_predict_one_J_T(&TD);
   
   memcpy(dydx, TD.ws->sum_dwdx, model->nIn * sizeof(double));
   return TD.yn;
}





void *lwpr_aux_predict_one_JcJ_T(void *ptr) {
   LWPR_ThreadData *TD = (LWPR_ThreadData *) ptr;
   LWPR_SubModel *sub = &(TD->model->sub[TD->dim]);
   LWPR_Workspace *WS = TD->ws;

   int i,j,n;
   int nIn=TD->model->nIn;
   int nInS=TD->model->nInStore;
   
   double *xc = WS->xc;
   double *s = WS->s;
   double *dsdx = WS->dsdx;
   double *Dx = WS->Dx;
   double *sum_dwdx = WS->sum_dwdx;
   double *sum_ydwdx_wdydx = WS->sum_ydwdx_wdydx;
   
   double w, dwdq;
   double yp = 0.0;
   
   double sum_w = 0.0;
   double sum_R = 0.0;
   
   double *sum_dRdx = WS->sum_ddRdxdx;

   memset(sum_dwdx,0,nIn*sizeof(double));
   memset(sum_ydwdx_wdydx,0,nIn*sizeof(double));
   
   memset(sum_dRdx,0,nIn*sizeof(double));
         
   for (n=0;n<sub->numRFS;n++) {
      double dist = 0.0;
      LWPR_ReceptiveField *RF = sub->rf[n];
      
      for (i=0;i<nIn;i++) {
         xc[i] = TD->xn[i] - RF->c[i];
      }
      
      for (j=0;j<nIn;j++) {
         Dx[j] = lwpr_math_dot_product(RF->D + j*nInS, xc, nIn);
         dist += xc[j] * Dx[j];
      }
      switch(TD->model->kernel) {
         case LWPR_GAUSSIAN_KERNEL:
            w = exp(-0.5*dist);
            dwdq = -0.5 * w;
            break;
         case LWPR_BISQUARE_KERNEL:
            dwdq = 1-0.25*dist;
            if (dwdq<0) {
               w = dwdq = 0.0;
            } else {
               w = dwdq*dwdq;
               dwdq = -0.5*dwdq;
            }
            break;
         default:
            w = dwdq = 0;
      }
           
      if (w>TD->cutoff && RF->trustworthy) {
         int nR = RF->nReg;
         double yp_n = RF->beta0;
         double Gamma,sigma2;
         double sum_sS2 = 0.0;
         
         for (i=0;i<nIn;i++) {
            xc[i] = TD->xn[i] - RF->mean_x[i];
         }  
         
         sum_w += w;
         
         /* we can't just use cached slopes here */
         
         if (RF->n_data[nR-1] <= 2*nIn) nR--;

         lwpr_aux_compute_projection_d(nIn, nInS, nR, s, dsdx, xc, RF->U, RF->P,WS);
         for (i=0;i<nR;i++) {
            yp_n+=s[i]*RF->beta[i];
            sum_sS2 +=s[i]*s[i] / RF->SSs2[i];
         }
         yp += w*yp_n;
         
         Gamma = RF->sum_e_cv2[nR-1]/(RF->sum_w[nR-1] - RF->SSp);
         sigma2 = Gamma*(1+w*sum_sS2);

         sum_R += w*(sigma2 + yp_n*yp_n);
         
         lwpr_math_scalar_vector(RF->slope, RF->beta[0], dsdx, nIn);
         for (i=1;i<nR;i++) {
            lwpr_math_add_scalar_vector(RF->slope, RF->beta[i], dsdx + i*nInS, nIn);
         }
         RF->slopeReady=1;
         
         /* dwdx = 2.0*dwdq*Dx */
         
         lwpr_math_add_scalar_vector(sum_dwdx, 2.0*dwdq, Dx, nIn);
         lwpr_math_add_scalar_vector(sum_ydwdx_wdydx, yp_n*2.0*dwdq, Dx, nIn);
         lwpr_math_add_scalar_vector(sum_ydwdx_wdydx, w, RF->slope, nIn);            
         
         /* Three parts depending on dw/dx = 2.0*dwdq*Dx 
          *    dw/dx * sigma2 
          *    dw/dx * yp_n^2
          *    dw/dx * w * Gamma * sum_sS2 (as part of sigma2 derivative) */
         
         lwpr_math_add_scalar_vector(sum_dRdx, (sigma2 + yp_n*yp_n + w*Gamma*sum_sS2)*2.0*dwdq, Dx, nIn);

         /* This part is w * Gamma * w * d(sum_sS2)/dx  (as part of sigma2 derivative) */
         for (i=0;i<nR;i++) {
            lwpr_math_add_scalar_vector(sum_dRdx, w*Gamma*w*2.0*s[i]/RF->SSs2[i], dsdx + i*nInS, nIn);
         }
         
         /* This part is for w*d(yp_n*yp_n)/dx */
         lwpr_math_add_scalar_vector(sum_dRdx, 2.0*w*yp_n, RF->slope, nIn);
      }
   }
     
   if (sum_w > 0.0) {
      yp/=sum_w;
      
      /* Put gradient into sum_ydwdx_wdydx -- this is different from predict_one_T!!! */
      /* dydx = -(yp/sum_w) * dw/dx  + (1/sum_w)*sum_ydwdx_wdydx) */
      
      lwpr_math_scale_add_scalar_vector(1.0/sum_w, sum_ydwdx_wdydx, -yp/sum_w, sum_dwdx, nIn);
      TD->yn = yp;
      
      sum_R -= sum_w*yp*yp;
      
      /*
         R    = sum_R    - sum_w   *yp^2;  
         dRdx = sum_dRdx - sum_dwdx*yp^2 - sum_w*2*yp*dypdx;
      */
      lwpr_math_add_scalar_vector(sum_dRdx, -yp*yp, sum_dwdx, nIn);
      lwpr_math_add_scalar_vector(sum_dRdx, -2.0*sum_w*yp, sum_ydwdx_wdydx, nIn);
      
      /*
         conf = sqrt(R)/sum_w;   
         dcdx = 0.5/(sum_w * sqrt(R))*dRdx - (conf/sum_w)*sum_dwdx;
      */
      TD->w_sec = sqrt(sum_R)/sum_w;
      lwpr_math_scale_add_scalar_vector(0.5/(sum_w * sqrt(sum_R)), sum_dRdx, -TD->w_sec/sum_w, sum_dwdx, nIn);
      
   } else {
      /* sum_dwdx = 0 */
      /* memset(sum_ydwdx_wdydx,0,nIn*sizeof(double)); */
      TD->yn = 0.0;
      TD->w_sec = 1e20;
   }
   return NULL;
}


double lwpr_aux_predict_one_JcJ(const LWPR_Model *model, int dim, const double *xn, double cutoff, double *dydx, double *conf, double *dconfdx) {
   LWPR_ThreadData TD;
   
   TD.model = model;
   TD.dim = dim;
   TD.xn = xn;
   TD.cutoff = cutoff;
   TD.ws = &model->ws[0];
   
   (void) lwpr_aux_predict_one_JcJ_T(&TD);
   
   (void) lwpr_aux_predict_conf_one_T(&TD);
   *conf = TD.w_sec;
   
   memcpy(dydx,    TD.ws->sum_ydwdx_wdydx, model->nIn * sizeof(double));
   memcpy(dconfdx, TD.ws->sum_ddRdxdx,     model->nIn * sizeof(double));
   return TD.yn;
}






void *lwpr_aux_predict_one_gH_T(void *ptr) {
   LWPR_ThreadData *TD = (LWPR_ThreadData *) ptr;
   LWPR_SubModel *sub = &(TD->model->sub[TD->dim]);
   LWPR_Workspace *WS = TD->ws;

   int i,j,n;
   int nIn=TD->model->nIn;
   int nInS=TD->model->nInStore;
   
   double *xc = WS->xc;
   double *s = WS->s;
   double *dsdx = WS->dsdx;
   double *Dx = WS->Dx;
   double *sum_dwdx = WS->sum_dwdx;
   double *sum_ydwdx_wdydx = WS->sum_ydwdx_wdydx;
   double *sum_ddwdxdx = WS->sum_ddwdxdx;
   double *sum_ddRdxdx = WS->sum_ddRdxdx;
     
   double w, dwdq, ddwdqdq;
   double yp = 0.0;
   
   double sum_w = 0.0;

   memset(sum_dwdx,0,nIn*sizeof(double));
   memset(sum_ydwdx_wdydx,0,nIn*sizeof(double));
   
   memset(sum_ddRdxdx,0,nInS*nIn*sizeof(double));
   memset(sum_ddwdxdx,0,nInS*nIn*sizeof(double));
         
   for (n=0;n<sub->numRFS;n++) {
      double dist = 0.0;
      LWPR_ReceptiveField *RF = sub->rf[n];
      
      for (i=0;i<nIn;i++) {
         xc[i] = TD->xn[i] - RF->c[i];
      }
      
      for (j=0;j<nIn;j++) {
         Dx[j] = lwpr_math_dot_product(RF->D + j*nInS, xc, nIn);
         dist += xc[j] * Dx[j];
      }
      switch(TD->model->kernel) {
         case LWPR_GAUSSIAN_KERNEL:
            w = exp(-0.5*dist);
            dwdq = -0.5 * w;
            ddwdqdq = 0.25 * w;
            break;
         case LWPR_BISQUARE_KERNEL:
            dwdq = 1-0.25*dist;
            if (dwdq<0) {
               w = dwdq = ddwdqdq = 0.0;
            } else {
               w = dwdq*dwdq;
               dwdq = -0.5*dwdq;
               ddwdqdq = 0.125;
            }
            break;
         default:
            w = dwdq = ddwdqdq = 0;
      }
           
      if (w>TD->cutoff && RF->trustworthy) {
         double yp_n = RF->beta0;
         
         for (i=0;i<nIn;i++) {
            xc[i] = TD->xn[i] - RF->mean_x[i];
         }  
         
         sum_w += w;
         
         if (RF->slopeReady) {
            yp_n += lwpr_math_dot_product(xc, RF->slope, nIn);
            yp += w*yp_n;
         } else {
            int nR = RF->nReg;
            
            if (RF->n_data[nR-1] <= 2*nIn) nR--;


            lwpr_aux_compute_projection_d(nIn, nInS, nR, s, dsdx, xc, RF->U, RF->P,WS);
            for (i=0;i<nR;i++) {
               yp_n+=s[i]*RF->beta[i];
            }
            yp += w*yp_n;
            lwpr_math_scalar_vector(RF->slope, RF->beta[0], dsdx, nIn);
            for (i=1;i<nR;i++) {
               lwpr_math_add_scalar_vector(RF->slope, RF->beta[i], dsdx + i*nInS, nIn);
            }
            /*  part of original code without cached slopes:
            for (i=0;i<RF->nReg;i++) {
               lwpr_math_add_scalar_vector(sum_ydwdx_wdydx, w * RF->beta[i], dsdx + i*nInS, nIn);
            }
            */
            RF->slopeReady=1;
         }
         
         lwpr_math_add_scalar_vector(sum_dwdx, 2.0*dwdq, Dx, nIn);
         lwpr_math_add_scalar_vector(sum_ydwdx_wdydx, yp_n*2.0*dwdq, Dx, nIn);
         lwpr_math_add_scalar_vector(sum_ydwdx_wdydx, w, RF->slope, nIn);            
         
         for (i=0;i<nIn;i++) {
            /* sum up ddwdxdx */
            lwpr_math_add_scalar_vector(sum_ddwdxdx + i*nInS, 4.0*ddwdqdq*Dx[i], Dx, nIn);
            lwpr_math_add_scalar_vector(sum_ddwdxdx + i*nInS, 2.0*dwdq, RF->D + i*nInS, nIn);

            /* sum up ddRdxdx */
            /* ... the yp_n * ddwdxdx part */
            lwpr_math_add_scalar_vector(sum_ddRdxdx + i*nInS, yp_n*4.0*ddwdqdq*Dx[i], Dx, nIn);
            lwpr_math_add_scalar_vector(sum_ddRdxdx + i*nInS, yp_n*2.0*dwdq, RF->D + i*nInS, nIn);
            /* += dwdx*dydx'  ,that is, 2*dwdq*Dx * RF->slope' */
            lwpr_math_add_scalar_vector(sum_ddRdxdx + i*nInS, 2.0*dwdq*RF->slope[i], Dx, nIn);
            /* += dydx*dwdx'  ,that is, 2*dwdq*Dx' * RF->slope */            
            lwpr_math_add_scalar_vector(sum_ddRdxdx + i*nInS, 2.0*dwdq*Dx[i], RF->slope, nIn);
         }            
      }
   }
     
   if (sum_w > 0.0) {
      yp/=sum_w;
      
      /* Put Hessian into sum_ddwdxdx */     
      lwpr_math_scale_add_scalar_vector(-yp/sum_w, sum_ddwdxdx, 1.0/sum_w, sum_ddRdxdx, nIn*nInS);
      /* put 1/sum_w * sum_dwdx into sum_ddRdxdx, we'll need that later */
      lwpr_math_scalar_vector(sum_ddRdxdx, 1.0/sum_w, sum_dwdx, nIn);

      /* Put gradient into sum_dwdx */
      /* dydx = -(yp/sum_w) * dw/dx  + (1/sum_w)*sum_ydwdx_wdydx) */
      lwpr_math_scale_add_scalar_vector(-yp/sum_w, sum_dwdx, 1.0/sum_w, sum_ydwdx_wdydx, nIn);
      
      /* Add further terms to Hessian */
      for (i=0;i<nIn;i++) {
         lwpr_math_add_scalar_vector(sum_ddwdxdx + i*nInS, -sum_dwdx[i], sum_ddRdxdx, nIn);
         lwpr_math_add_scalar_vector(sum_ddwdxdx + i*nInS, -sum_ddRdxdx[i], sum_dwdx, nIn);
      }         
            
      TD->yn = yp;
   } else {
      /* sum_dwdx = 0 */
      /* memset(sum_dwdx,0,nIn*sizeof(double)); */
      TD->yn = 0.0;
   }
   return NULL;
}


double lwpr_aux_predict_one_gH(const LWPR_Model *model, int dim, const double *xn, double cutoff, double *dydx, double *ddydxdx) {
   LWPR_ThreadData TD;
   
   TD.model = model;
   TD.dim = dim;
   TD.xn = xn;
   TD.cutoff = cutoff;
   TD.ws = &model->ws[0];
   
   (void) lwpr_aux_predict_one_gH_T(&TD);
   
   memcpy(dydx, TD.ws->sum_dwdx, model->nIn * sizeof(double));
   return TD.yn;
}
