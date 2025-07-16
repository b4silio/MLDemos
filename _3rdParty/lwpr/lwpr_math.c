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
#include <math.h>
#include <string.h>
#include "lwpr_math.h"

double lwpr_math_norm2(const double *x, int n) {
   double norm = 0.0;

   while (n>=4) {
      norm += x[0] * x[0];
      norm += x[1] * x[1];
      norm += x[2] * x[2];
      norm += x[3] * x[3];
      n-=4;
      x+=4;
   }
   switch(n) {
      case 3: norm += x[2] * x[2];
      case 2: norm += x[1] * x[1];
      case 1: norm += x[0] * x[0];
   }         
   return norm;
   
}

double lwpr_math_dot_product(const double *x,const double *y,int n) {
   double dp=0;
   while (n>=4) {
      dp += y[0] * x[0];
      dp += y[1] * x[1];
      dp += y[2] * x[2];
      dp += y[3] * x[3];
      n-=4;
      y+=4;
      x+=4;
   }
   switch(n) {
      case 3: dp += y[2] * x[2];
      case 2: dp += y[1] * x[1];
      case 1: dp += y[0] * x[0];
   }         
   return dp;
}

void lwpr_math_scalar_vector(double *y, double a,const double *x,int n) {
   /* for (i=0;i<n;i++) y[i] = a*x[i]; */
   while (n>=8) {
      y[0] = a*x[0];
      y[1] = a*x[1];
      y[2] = a*x[2];
      y[3] = a*x[3];
      y[4] = a*x[4];
      y[5] = a*x[5];
      y[6] = a*x[6];
      y[7] = a*x[7];
      n-=8;
      y+=8;
      x+=8;
   }
   switch(n) {
      case 7: y[6] = a*x[6];
      case 6: y[5] = a*x[5];
      case 5: y[4] = a*x[4];
      case 4: y[3] = a*x[3];
      case 3: y[2] = a*x[2];
      case 2: y[1] = a*x[1];
      case 1: y[0] = a*x[0];
   }         
}

void lwpr_math_add_scalar_vector(double *y, double a,const double *x,int n) {
   /*
   DAXPY_SSE2(X,n,a,x,y);
   */
   /* for (i=0;i<n;i++) y[i] += a*x[i]; */
   while (n>=8) {
      y[0] += a*x[0];
      y[1] += a*x[1];
      y[2] += a*x[2];
      y[3] += a*x[3];
      y[4] += a*x[4];
      y[5] += a*x[5];
      y[6] += a*x[6];
      y[7] += a*x[7];
      n-=8;
      y+=8;
      x+=8;
   }
   switch(n) {
      case 7: y[6] += a*x[6];
      case 6: y[5] += a*x[5];
      case 5: y[4] += a*x[4];
      case 4: y[3] += a*x[3];
      case 3: y[2] += a*x[2];
      case 2: y[1] += a*x[1];
      case 1: y[0] += a*x[0];
   }      
}

void lwpr_math_scale_add_scalar_vector(double b, double *y, double a,const double *x,int n) {
   /* for (i=0;i<n;i++) y[i] = b*y[i] + a*x[i]; */
   while (n>=8) {
      y[0] = b*y[0] + a*x[0];
      y[1] = b*y[1] + a*x[1];
      y[2] = b*y[2] + a*x[2];
      y[3] = b*y[3] + a*x[3];
      y[4] = b*y[4] + a*x[4];
      y[5] = b*y[5] + a*x[5];
      y[6] = b*y[6] + a*x[6];
      y[7] = b*y[7] + a*x[7];
      n-=8;
      y+=8;
      x+=8;
   }
   switch(n) {
      case 7: y[6] = b*y[6] + a*x[6];
      case 6: y[5] = b*y[5] + a*x[5];
      case 5: y[4] = b*y[4] + a*x[4];
      case 4: y[3] = b*y[3] + a*x[3];
      case 3: y[2] = b*y[2] + a*x[2];
      case 2: y[1] = b*y[1] + a*x[1];
      case 1: y[0] = b*y[0] + a*x[0];
   }      
}

int lwpr_math_cholesky(int N,int Ns,double *R,const double *A) {
   int i,j,k;
   double A_00, R_00;
   
   if (A!=NULL) {
      memcpy(R,A,N*Ns*sizeof(double));
   }

   A_00 = R[0];
   if (A_00 <= 0) return 0;
      
   R[0] = R_00 = sqrt(A_00);

   if (N > 1) {
      double A_01 = R[Ns];
      double A_11 = R[1+Ns];
      double R_01,diag;
      
      R_01 = A_01 / R_00;
      diag = A_11 - R_01 * R_01;

      if (diag<=0) return 0;

      R[0+Ns]=R_01;
      R[1+Ns]=sqrt(diag); 

      for (k = 2; k < N; k++) {
         double A_kk = R[k+k*Ns];
         double diag;
         
         for (i = 0; i < k; i++) {
            double sum;
            double A_ik = R[i+k*Ns];
            double A_ii = R[i+i*Ns];
  
            sum = lwpr_math_dot_product(R+i*Ns,R+k*Ns,i);
  
            A_ik = (A_ik-sum)/A_ii;
            R[i+k*Ns]=A_ik;
         }

         diag = A_kk - lwpr_math_dot_product(R+k*Ns,R+k*Ns,k);
         if (diag <= 0) return 0; 
         R[k+k*Ns]=sqrt(diag);
      }
   }

   for (j=0;j<N-1;j++) {
      for (i=j+1;i<N;i++) {
         R[i+j*Ns]=0;
      }
   }
   return 1;
}

