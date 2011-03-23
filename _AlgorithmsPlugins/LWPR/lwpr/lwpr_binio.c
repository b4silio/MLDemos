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
#include "lwpr_binio.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define LWPR_BINIO_VERSION    -1


int lwpr_io_write_matrix(FILE *fp,int M, int Ms, int N, const double *data) {
   int n;
   
   for (n=0;n<N;n++) {
      if (M!=(int) fwrite(data + n*Ms, sizeof(double), (size_t) M, fp)) return 0;
   }
   return 1;
}

int lwpr_io_read_matrix(FILE *fp, int M, int Ms, int N, double *data) {
   int n;
   
   for (n=0;n<N;n++) {
      if (M!=(int) fread(data + n*Ms, sizeof(double), (size_t) M, fp)) return 0;
   }
   return 1;
}

int lwpr_io_write_vector(FILE *fp, int N, const double *data) {
   return (N==(int) fwrite(data, sizeof(double), (size_t) N, fp)) ? 1:0;
}

int lwpr_io_read_vector(FILE *fp, int N, double *data) {
   return (N==(int) fread(data, sizeof(double), (size_t) N, fp)) ? 1:0;
}

int lwpr_io_write_scalar(FILE *fp, double data) {
   return (int) fwrite(&data, sizeof(double), 1, fp);
}

int lwpr_io_read_scalar(FILE *fp, double *data) {
   return (int) fread(data, sizeof(double), 1, fp);
}

int lwpr_io_write_int(FILE *fp, int data) {
   return (int) fwrite(&data, sizeof(int), 1, fp);
}

int lwpr_io_read_int(FILE *fp, int *data) {
   return (int) fread(data, sizeof(int), 1, fp);
}

int lwpr_io_write_rf(FILE *fp, const LWPR_ReceptiveField *RF) {
   int ok;
   int nIn = RF->model->nIn;
   int nInS = RF->model->nInStore;
   int nReg = RF->nReg;
   
   ok = (fwrite("[RF]", 1, 4, fp)==4) ? 1:0;
   ok &= lwpr_io_write_int(fp, nReg);
   ok &= lwpr_io_write_matrix(fp,nIn,nInS,nIn,RF->D);
   ok &= lwpr_io_write_matrix(fp,nIn,nInS,nIn,RF->M);   
   ok &= lwpr_io_write_matrix(fp,nIn,nInS,nIn,RF->alpha);   
   ok &= lwpr_io_write_scalar(fp,RF->beta0);
   ok &= lwpr_io_write_vector(fp,nReg,RF->beta);
   ok &= lwpr_io_write_vector(fp,nIn,RF->c);   
   ok &= lwpr_io_write_matrix(fp,nIn,nInS,nReg,RF->SXresYres);
   ok &= lwpr_io_write_vector(fp,nReg,RF->SSs2);   
   ok &= lwpr_io_write_vector(fp,nReg,RF->SSYres);      
   ok &= lwpr_io_write_matrix(fp,nIn,nInS,nReg,RF->SSXres);   
   ok &= lwpr_io_write_matrix(fp,nIn,nInS,nReg,RF->U);      
   ok &= lwpr_io_write_matrix(fp,nIn,nInS,nReg,RF->P);      
   ok &= lwpr_io_write_vector(fp,nReg,RF->H);            
   ok &= lwpr_io_write_vector(fp,nReg,RF->r);  
   ok &= lwpr_io_write_matrix(fp,nIn,nInS,nIn,RF->h);                      
   ok &= lwpr_io_write_matrix(fp,nIn,nInS,nIn,RF->b);                   
   ok &= lwpr_io_write_vector(fp,nReg,RF->sum_w);  
   ok &= lwpr_io_write_vector(fp,nReg,RF->sum_e_cv2);  
   ok &= lwpr_io_write_scalar(fp,RF->sum_e2);
   ok &= lwpr_io_write_scalar(fp,RF->SSp);
   ok &= lwpr_io_write_vector(fp,nReg,RF->n_data);     
   ok &= lwpr_io_write_int(fp,RF->trustworthy);   
   ok &= lwpr_io_write_vector(fp,nReg,RF->lambda);     
   ok &= lwpr_io_write_vector(fp,nIn,RF->mean_x);   
   ok &= lwpr_io_write_vector(fp,nIn,RF->var_x);         
   ok &= lwpr_io_write_scalar(fp,RF->w);   
   ok &= lwpr_io_write_vector(fp,nReg,RF->s);     
   return ok;
}

int lwpr_io_read_rf(FILE *fp, LWPR_SubModel *sub) {
   char str[5];
   int ok;
   int nIn = sub->model->nIn;
   int nInS = sub->model->nInStore;
   int nReg;
   LWPR_ReceptiveField *RF;
   
   ok = (int) fread(str, 1, 4, fp);
   if (ok!=4) return 0;
   str[4]=0;
   if (strcmp(str,"[RF]")!=0) return 0;

   ok = lwpr_io_read_int(fp, &nReg);
   if (ok!=1 || nReg<=0 || nReg>nIn) return 0;
   
   RF = lwpr_aux_add_rf(sub,nReg);
   if (RF==NULL) return 0;
   
   ok &= lwpr_io_read_matrix(fp,nIn,nInS,nIn,RF->D);
   ok &= lwpr_io_read_matrix(fp,nIn,nInS,nIn,RF->M);   
   ok &= lwpr_io_read_matrix(fp,nIn,nInS,nIn,RF->alpha);   
   ok &= lwpr_io_read_scalar(fp,&RF->beta0);
   ok &= lwpr_io_read_vector(fp,nReg,RF->beta);
   ok &= lwpr_io_read_vector(fp,nIn,RF->c);   
   ok &= lwpr_io_read_matrix(fp,nIn,nInS,nReg,RF->SXresYres);
   ok &= lwpr_io_read_vector(fp,nReg,RF->SSs2);   
   ok &= lwpr_io_read_vector(fp,nReg,RF->SSYres);      
   ok &= lwpr_io_read_matrix(fp,nIn,nInS,nReg,RF->SSXres);   
   ok &= lwpr_io_read_matrix(fp,nIn,nInS,nReg,RF->U);      
   ok &= lwpr_io_read_matrix(fp,nIn,nInS,nReg,RF->P);      
   ok &= lwpr_io_read_vector(fp,nReg,RF->H);            
   ok &= lwpr_io_read_vector(fp,nReg,RF->r);  
   ok &= lwpr_io_read_matrix(fp,nIn,nInS,nIn,RF->h);                      
   ok &= lwpr_io_read_matrix(fp,nIn,nInS,nIn,RF->b);                   
   ok &= lwpr_io_read_vector(fp,nReg,RF->sum_w);  
   ok &= lwpr_io_read_vector(fp,nReg,RF->sum_e_cv2);  
   ok &= lwpr_io_read_scalar(fp,&RF->sum_e2);
   ok &= lwpr_io_read_scalar(fp,&RF->SSp);
   ok &= lwpr_io_read_vector(fp,nReg,RF->n_data);     
   ok &= lwpr_io_read_int(fp,&RF->trustworthy);   
   ok &= lwpr_io_read_vector(fp,nReg,RF->lambda);     
   ok &= lwpr_io_read_vector(fp,nIn,RF->mean_x);   
   ok &= lwpr_io_read_vector(fp,nIn,RF->var_x);         
   ok &= lwpr_io_read_scalar(fp,&RF->w);   
   ok &= lwpr_io_read_vector(fp,nReg,RF->s);     
   return ok;
}

int lwpr_write_binary_fp(const LWPR_Model *model, FILE *fp) {
   int ok;
   int nIn = model->nIn;
   int nInS = model->nInStore;
   int nOut = model->nOut;
   int i,dim;
   int version = LWPR_BINIO_VERSION;
   
   ok = (int) fwrite("LWPR", sizeof(char), 4, fp);
   if (ok!=4) return 0;
   
   ok = lwpr_io_write_int(fp,  version);
   ok &= lwpr_io_write_int(fp,  nIn);
   ok &= lwpr_io_write_int(fp, nOut);
   ok &= lwpr_io_write_int(fp, (int) model->kernel);
   
   if (model->name == NULL) {
      ok &= lwpr_io_write_int(fp, 0);
   } else {
      size_t len = strlen(model->name);
      ok &= lwpr_io_write_int(fp, (int ) len);
      ok &= (fwrite(model->name, sizeof(char), len, fp)==len)? 1:0;
   }
   ok &= lwpr_io_write_int(fp,model->n_data);
   ok &= lwpr_io_write_vector(fp, nIn, model->mean_x);
   ok &= lwpr_io_write_vector(fp, nIn, model->var_x);   
   ok &= lwpr_io_write_int(fp, model->diag_only);
   ok &= lwpr_io_write_int(fp, model->update_D);   
   ok &= lwpr_io_write_int(fp, model->meta);
   ok &= lwpr_io_write_scalar(fp, model->meta_rate);
   ok &= lwpr_io_write_scalar(fp, model->penalty);   
   ok &= lwpr_io_write_matrix(fp, nIn, nInS, nIn, model->init_alpha);
   ok &= lwpr_io_write_vector(fp, nIn, model->norm_in);
   ok &= lwpr_io_write_vector(fp, nOut, model->norm_out);   
   ok &= lwpr_io_write_matrix(fp, nIn, nInS, nIn, model->init_D);   
   ok &= lwpr_io_write_matrix(fp, nIn, nInS, nIn, model->init_M); 
   
   ok &= lwpr_io_write_scalar(fp, model->w_gen);  
   ok &= lwpr_io_write_scalar(fp, model->w_prune);   
   ok &= lwpr_io_write_scalar(fp, model->init_lambda);   
   ok &= lwpr_io_write_scalar(fp, model->final_lambda);   
   ok &= lwpr_io_write_scalar(fp, model->tau_lambda);      
   ok &= lwpr_io_write_scalar(fp, model->init_S2);      
   ok &= lwpr_io_write_scalar(fp, model->add_threshold);      

   for (dim=0;dim<model->nOut;dim++) {
      const LWPR_SubModel *sub = &model->sub[dim];   
      ok &= (fwrite("SUBM", sizeof(char), 4, fp)==4)?1:0;
      ok &= lwpr_io_write_int(fp, dim);
      ok &= lwpr_io_write_int(fp, sub->numRFS);      
      ok &= lwpr_io_write_int(fp, sub->n_pruned);            
      for (i=0;i<sub->numRFS;i++) {
         ok &= lwpr_io_write_rf(fp, sub->rf[i]);
      }
   }
   ok &= (fwrite("RPWL", sizeof(char), 4, fp) == 4)?1:0;   
   return ok;
}

int lwpr_read_binary_fp(LWPR_Model *model, FILE *fp) {
   char str[5];
   int ok;
   int nIn,nInS,nOut;
   int i,dim;
   int version;
   
   ok = (int) fread(str, sizeof(char), 4, fp);
   if (ok!=4) return 0;
   
   str[4]=0;
   if (strcmp(str,"LWPR")!=0) return 0;  
   
   if (!lwpr_io_read_int(fp, &version)) return 0;
   
   if (version!=LWPR_BINIO_VERSION) {
      fprintf(stderr,"Sorry, version of binary LWPR file does not match this implementation.\n");
      return 0;
   }
  
   if (!lwpr_io_read_int(fp, &nIn) || !lwpr_io_read_int(fp, &nOut)) return 0;
   if (nIn<=0) return 0;
   if (nOut<=0) return 0;
   if (!lwpr_init_model(model, nIn, nOut, NULL)) return 0;
   
   ok = lwpr_io_read_int(fp, &i);
   model->kernel = (LWPR_Kernel) i;
   
   ok &= lwpr_io_read_int(fp, &i);
   
   if (i>0) {
      size_t len = (size_t) i;
      model->name = (char *) LWPR_MALLOC((len+1)*sizeof(char));
      if (model->name == NULL) return 0;
      ok &= (fread(model->name, sizeof(char), len, fp) == len)?1:0;
      model->name[i] = 0;
   }
   nInS = model->nInStore;
   
   ok &= lwpr_io_read_int(fp, &model->n_data);
   ok &= lwpr_io_read_vector(fp, nIn, model->mean_x);
   ok &= lwpr_io_read_vector(fp, nIn, model->var_x);   
   ok &= lwpr_io_read_int(fp, &model->diag_only);
   ok &= lwpr_io_read_int(fp, &model->update_D);   
   ok &= lwpr_io_read_int(fp, &model->meta);
   ok &= lwpr_io_read_scalar(fp, &model->meta_rate);
   ok &= lwpr_io_read_scalar(fp, &model->penalty);   
   ok &= lwpr_io_read_matrix(fp, nIn, nInS, nIn, model->init_alpha);
   ok &= lwpr_io_read_vector(fp, nIn, model->norm_in);
   ok &= lwpr_io_read_vector(fp, nOut, model->norm_out);   
   ok &= lwpr_io_read_matrix(fp, nIn, nInS, nIn, model->init_D);   
   ok &= lwpr_io_read_matrix(fp, nIn, nInS, nIn, model->init_M); 
   
   ok &= lwpr_io_read_scalar(fp, &model->w_gen);  
   ok &= lwpr_io_read_scalar(fp, &model->w_prune);   
   ok &= lwpr_io_read_scalar(fp, &model->init_lambda);   
   ok &= lwpr_io_read_scalar(fp, &model->final_lambda);   
   ok &= lwpr_io_read_scalar(fp, &model->tau_lambda);      
   ok &= lwpr_io_read_scalar(fp, &model->init_S2);      
   ok &= lwpr_io_read_scalar(fp, &model->add_threshold); 
   
   for (dim=0;dim<model->nOut;dim++) {
      int numRFS;
      LWPR_SubModel *sub = &model->sub[dim];   
      ok &= (fread(str, sizeof(char), 4, fp)==4)?1:0;
      str[4]=0;
      if (!ok || strcmp(str,"SUBM")!=0) {
         lwpr_free_model(model);
         return 0;
      }
      ok &= lwpr_io_read_int(fp, &i);
      ok &= (i==dim);
      ok &= lwpr_io_read_int(fp, &numRFS);      
      ok &= lwpr_io_read_int(fp, &sub->n_pruned);            
      for (i=0;i<numRFS;i++) {
         ok &= lwpr_io_read_rf(fp, sub);
      }
      ok &= (numRFS == sub->numRFS);
   }
   ok &= (fread(str, sizeof(char), 4, fp) == 4)?1:0;   
   str[4] = 0;
   if (!ok || strcmp(str,"RPWL")!=0) {
      lwpr_free_model(model);
      return 0;
   }

   return 1;
}


int lwpr_write_binary(const LWPR_Model *model, const char *filename) {
   int ok;
   FILE *fp;

   fp = fopen(filename, "wb");
   if (fp==NULL) return 0;
   ok = lwpr_write_binary_fp(model,fp);
   fclose(fp);
   return ok;
}

int lwpr_read_binary(LWPR_Model *model, const char *filename) {
   int ok;
   FILE *fp;
   
   fp = fopen(filename, "rb");
   if (fp==NULL) return 0;
   ok = lwpr_read_binary_fp(model,fp);
   fclose(fp);
   return ok;
}
