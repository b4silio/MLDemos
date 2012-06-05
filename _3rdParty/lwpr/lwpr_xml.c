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
#include "lwpr_xml.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#if HAVE_LIBEXPAT
#include <expat.h>
#endif

void lwpr_xml_write_matrix(FILE *fp, int level, const char *name, int M, int Ms, int N, const double *val) {
   int m,n,l;
   double abs0 = fabs(val[0]);

   for (l=0;l<level;l++) fprintf(fp,"\t");
   fprintf(fp,"<matrix name='%s' rows='%d' columns='%d'>\n",name,M,N);

   if (abs0 != 0.0 && (abs0 >= 1000 || abs0 < 0.01)) {
      for (m=0;m<M;m++) {
         for (l=0;l<level;l++) fprintf(fp,"\t");
         for (n=0;n<N;n++) {
            fprintf(fp," %12.6e",val[m+n*Ms]);
         }
         fprintf(fp,"\n");
      }
   } else {
      for (m=0;m<M;m++) {
         for (l=0;l<level;l++) fprintf(fp,"\t");
         for (n=0;n<N;n++) {
            fprintf(fp," %12.6f",val[m+n*Ms]);
         }
         fprintf(fp,"\n");
      }
   }
   for (l=0;l<level;l++) fprintf(fp,"\t");
   fprintf(fp,"</matrix>\n");
}

void lwpr_xml_write_vector(FILE *fp, int level, const char *name, int N, const double *val) {
   int n,l;
   double abs0 = fabs(val[0]);

   for (l=0;l<level;l++) fprintf(fp,"\t");
   fprintf(fp,"<vector name='%s' length='%d'>\n",name,N);
   if (abs0 != 0.0 && (abs0 >= 1000 || abs0 < 0.01)) {
      for (n=0;n<N;n++) {
         for (l=0;l<level;l++) fprintf(fp,"\t");
         fprintf(fp," %12.6e\n",val[n]);
      }
   } else {
      for (n=0;n<N;n++) {
         for (l=0;l<level;l++) fprintf(fp,"\t");
         fprintf(fp," %12.6f\n",val[n]);
      }
   }
   for (l=0;l<level;l++) fprintf(fp,"\t");
   fprintf(fp,"</vector>\n");
}

void lwpr_xml_write_int(FILE *fp, int level, const char *name, int val) {
   int l;
   for (l=0;l<level;l++) fprintf(fp,"\t");
   fprintf(fp,"<integer name='%s'> %d </integer>\n",name,val);
}

void lwpr_xml_write_scalar(FILE *fp, int level, const char *name, double val) {
   int l;
   double abs0 = fabs(val);

   for (l=0;l<level;l++) fprintf(fp,"\t");
   if (abs0 == 0.0) {
      fprintf(fp,"<scalar name='%s'> 0.0 </scalar>\n",name);
      return;
   }

   if (abs0 >= 1000 || abs0 < 0.01) {
      fprintf(fp,"<scalar name='%s'> %12.6e </scalar>\n",name,val);
   } else {
      fprintf(fp,"<scalar name='%s'> %12.6f </scalar>\n",name,val);
   }
}

void lwpr_xml_write_rf(FILE *fp, const LWPR_ReceptiveField *RF) {
   int nIn = RF->model->nIn;
   int nInS = RF->model->nInStore;
   int nReg = RF->nReg;

   fprintf(fp,"\t\t<ReceptiveField nReg='%d'>\n",RF->nReg);
   lwpr_xml_write_matrix(fp,3,"D",nIn,nInS,nIn,RF->D);
   lwpr_xml_write_matrix(fp,3,"M",nIn,nInS,nIn,RF->M);
   lwpr_xml_write_matrix(fp,3,"alpha",nIn,nInS,nIn,RF->alpha);
   lwpr_xml_write_scalar(fp,3,"beta0",RF->beta0);
   lwpr_xml_write_vector(fp,3,"beta",nReg,RF->beta);
   lwpr_xml_write_vector(fp,3,"c",nIn,RF->c);
   lwpr_xml_write_matrix(fp,3,"SXresYres",nIn,nInS,nReg,RF->SXresYres);
   lwpr_xml_write_vector(fp,3,"SSs2",nReg,RF->SSs2);
   lwpr_xml_write_vector(fp,3,"SSYres",nReg,RF->SSYres);
   lwpr_xml_write_matrix(fp,3,"SSXres",nIn,nInS,nReg,RF->SSXres);
   lwpr_xml_write_matrix(fp,3,"U",nIn,nInS,nReg,RF->U);
   lwpr_xml_write_matrix(fp,3,"P",nIn,nInS,nReg,RF->P);
   lwpr_xml_write_vector(fp,3,"H",nReg,RF->H);
   lwpr_xml_write_vector(fp,3,"r",nReg,RF->r);
   lwpr_xml_write_matrix(fp,3,"h",nIn,nInS,nIn,RF->h);
   lwpr_xml_write_matrix(fp,3,"b",nIn,nInS,nIn,RF->b);
   lwpr_xml_write_vector(fp,3,"sum_w",nReg,RF->sum_w);
   lwpr_xml_write_vector(fp,3,"sum_e_cv2",nReg,RF->sum_e_cv2);
   lwpr_xml_write_scalar(fp,3,"sum_e2",RF->sum_e2);
   lwpr_xml_write_scalar(fp,3,"SSp",RF->SSp);
   lwpr_xml_write_vector(fp,3,"n_data",nReg,RF->n_data);
   lwpr_xml_write_int(fp,3,"trustworthy",RF->trustworthy);
   lwpr_xml_write_vector(fp,3,"lambda",nReg,RF->lambda);
   lwpr_xml_write_vector(fp,3,"mean_x",nIn,RF->mean_x);
   lwpr_xml_write_vector(fp,3,"var_x",nIn,RF->var_x);
   lwpr_xml_write_scalar(fp,3,"w",RF->w);
   lwpr_xml_write_vector(fp,3,"s",nReg,RF->s);
   fprintf(fp,"\t\t</ReceptiveField>\n");
}


void lwpr_write_xml_fp(const LWPR_Model *model, FILE *fp) {
   int dim;
   const char *kern_name;

   switch(model->kernel) {
      case LWPR_GAUSSIAN_KERNEL:
         kern_name = "Gaussian";
         break;
      case LWPR_BISQUARE_KERNEL:
         kern_name = "BiSquare";
         break;
      default:
         kern_name = "Unknown";
   }

   fprintf(fp,"<?xml version='1.0' encoding='US-ASCII' ?>\n");

   if (model->name != NULL) {
      fprintf(fp,"<LWPR name='%s' nIn='%d' nOut='%d' kernel='%s'>\n",
         model->name,model->nIn,model->nOut,kern_name);
   } else {
      fprintf(fp,"<LWPR nIn='%d' nOut='%d' kernel='%s'>\n",
         model->nIn,model->nOut,kern_name);
   }
   lwpr_xml_write_int(fp,1,"n_data",model->n_data);
   lwpr_xml_write_vector(fp,1,"mean_x",model->nIn,model->mean_x);
   lwpr_xml_write_vector(fp,1,"var_x",model->nIn,model->var_x);
   lwpr_xml_write_int(fp,1,"diag_only",model->diag_only);
   lwpr_xml_write_int(fp,1,"update_D",model->update_D);
   lwpr_xml_write_int(fp,1,"meta",model->meta);
   lwpr_xml_write_scalar(fp,1,"meta_rate",model->meta_rate);
   lwpr_xml_write_scalar(fp,1,"penalty",model->penalty);
   lwpr_xml_write_matrix(fp,1,"init_alpha",model->nIn,model->nInStore,model->nIn,model->init_alpha);
   lwpr_xml_write_vector(fp,1,"norm_in",model->nIn,model->norm_in);
   lwpr_xml_write_vector(fp,1,"norm_out",model->nOut,model->norm_out);
   lwpr_xml_write_matrix(fp,1,"init_D",model->nIn,model->nInStore,model->nIn,model->init_D);
   lwpr_xml_write_matrix(fp,1,"init_M",model->nIn,model->nInStore,model->nIn,model->init_M);
   lwpr_xml_write_scalar(fp,1,"w_gen",model->w_gen);
   lwpr_xml_write_scalar(fp,1,"w_prune",model->w_prune);
   lwpr_xml_write_scalar(fp,1,"init_lambda",model->init_lambda);
   lwpr_xml_write_scalar(fp,1,"final_lambda",model->final_lambda);
   lwpr_xml_write_scalar(fp,1,"tau_lambda",model->tau_lambda);
   lwpr_xml_write_scalar(fp,1,"init_S2",model->init_S2);
   lwpr_xml_write_scalar(fp,1,"add_threshold",model->add_threshold);
   for (dim=0;dim<model->nOut;dim++) {
      int num;
      const LWPR_SubModel *sub = &model->sub[dim];
      fprintf(fp,"\t<SubModel out_dim='%d' numRFS='%d'>\n",dim,sub->numRFS);
      lwpr_xml_write_int(fp,2,"n_pruned",sub->n_pruned);
      for (num=0;num<sub->numRFS;num++) {
         lwpr_xml_write_rf(fp,sub->rf[num]);
      }
      fprintf(fp,"\t</SubModel>\n");
   }
   fprintf(fp,"</LWPR>\n");
}

int lwpr_write_xml(const LWPR_Model *model, const char *filename) {
   FILE *fp;

   fp = fopen(filename,"w");
   if (fp==NULL) return 0;

   lwpr_write_xml_fp(model,fp);
   fclose(fp);
   return 1;
}


int lwpr_xml_parse_scalar(const char **atts, const char **fieldName) {
   int i=0;

   *fieldName = NULL;
   while (atts[i]!=NULL && atts[i+1]!=NULL) {
      if (!strcmp(atts[i],"name")) {
         *fieldName = atts[i+1];
         return 1;
      }
      i+=2;
   }
   return 0;
}

int lwpr_xml_parse_vector(const char **atts, const char **fieldName, int *N) {
   int i=0;

   *fieldName = NULL;
   *N = 0;

   while (atts[i]!=NULL && atts[i+1]!=NULL) {
      if (!strcmp(atts[i],"name")) {
         *fieldName = atts[i+1];
      } else if (!strcmp(atts[i],"length")) {
         *N = atoi(atts[i+1]);
      }
      i+=2;
   }
   return (*N!=0) && (*fieldName!=NULL);
}

int lwpr_xml_parse_matrix(const char **atts, const char **fieldName, int *M, int *N) {
   int i=0;

   *fieldName = NULL;
   *N = 0;
   *M = 0;

   while (atts[i]!=NULL && atts[i+1]!=NULL) {
      if (!strcmp(atts[i],"name")) {
         *fieldName = atts[i+1];
      } else if (!strcmp(atts[i],"rows")) {
         *M = atoi(atts[i+1]);
      } else if (!strcmp(atts[i],"columns")) {
         *N = atoi(atts[i+1]);
      }
      i+=2;
   }
   return (*N!=0) && (*M!=0) && (*fieldName!=NULL);
}

void lwpr_xml_error(LWPR_ParserData *ud, const char *msg) {
   ud->numErrors++;
   if (ud->errFile) {
      switch(ud->level) {
         case 0:
            fprintf(ud->errFile, "Error at top level: ");
            break;
         case 1:
            fprintf(ud->errFile, "Error at LWPR level: ");
            break;
         case 2:
            fprintf(ud->errFile, "Error at SubModel level (%d): ",ud->curSub);
            break;
         case 3:
            fprintf(ud->errFile, "Error at ReceptiveField level (%d/%d): ",ud->curSub,ud->curRF);
            break;
      }
      if (msg!=NULL) {
         fprintf(ud->errFile,"%s",msg);
      }
   }
}

void lwpr_xml_dim_error(LWPR_ParserData *ud, const char *fieldname,int wishM,int wishN) {
   lwpr_xml_error(ud,NULL);

   ud->N = ud->M = 0;
   if (ud->errFile) {
      switch(ud->curType) {
         case 3:
            fprintf(ud->errFile,"Vector '%s' is declared with %d elements, but should have %d.\n",fieldname,ud->N,wishN);
            break;
         case 4:
            fprintf(ud->errFile,"Matrix '%s' is declared with %dx%d elements, but should have %dx%d.\n",fieldname,ud->M,ud->N,wishM,wishN);
            break;
      }
   }
   ud->curType = 0;
}


void lwpr_xml_report_unknown(LWPR_ParserData *ud, const char *fieldname) {
   ud->numWarnings++;
   ud->N = ud->M = 0;
   if (ud->errFile) {
      switch(ud->level) {
         case 0:
            fprintf(ud->errFile, "Warning at top level: ");
            break;
         case 1:
            fprintf(ud->errFile, "Warning at LWPR level: ");
            break;
         case 2:
            fprintf(ud->errFile, "Warning at SubModel level (%d): ",ud->curSub);
            break;
         case 3:
            fprintf(ud->errFile, "Warning at ReceptiveField level (%d/%d): ",ud->curSub,ud->curRF);
            break;
      }
      switch(ud->curType) {
         case 0:
            fprintf(ud->errFile, "Ignoring unknown element '%s'.\n",fieldname);
            break;
         case 1:
            fprintf(ud->errFile,"Ignoring unknown integer '%s'.\n",fieldname);
            break;
         case 2:
            fprintf(ud->errFile,"Ignoring unknown scalar '%s'.\n",fieldname);
            break;
         case 3:
            fprintf(ud->errFile,"Ignoring unknown vector '%s'.\n",fieldname);
            break;
         case 4:
            fprintf(ud->errFile,"Ignoring unknown matrix '%s'.\n",fieldname);
            break;
      }
   }
}


void lwpr_xml_start_element(void *userData, const char *name, const char **atts) {
   int M=0, N=0;
   const char **at;
   const char *fieldName;
   int wishM,wishN;

   LWPR_ParserData *ud = (LWPR_ParserData *) userData;
   LWPR_Model *model = ud->model;
   LWPR_SubModel *sub=NULL;
   LWPR_ReceptiveField *RF=NULL;

   ud->readN = ud->readM = ud->N = ud->M = 0;

   if (model->sub!=NULL) {
      sub = &(model->sub[ud->curSub]);
      if (sub->rf != NULL) RF = sub->rf[ud->curRF];
   }

   if (!strcmp(name,"integer")) {
      ud->curType = 1;
      if (!lwpr_xml_parse_scalar(atts,&fieldName)) {
         lwpr_xml_error(ud,"<integer> element without name.\n");
         return;
      }
   } else if (!strcmp(name,"scalar")) {
      ud->curType = 2;
      if (!lwpr_xml_parse_scalar(atts,&fieldName)) {
         lwpr_xml_error(ud,"<scalar> element without name.");
         return;
      }
   } else if (!strcmp(name,"vector")) {
      ud->curType = 3;
      if (!lwpr_xml_parse_vector(atts,&fieldName,&N)) {
         lwpr_xml_error(ud,"Parse error: <vector> element without name or length.\n");
         return;
      }
   } else if (!strcmp(name,"matrix")) {
      ud->curType = 4;
      if (!lwpr_xml_parse_matrix(atts,&fieldName,&M,&N)) {
         lwpr_xml_error(ud,"Parse error: <matrix> element without name, rows or columns.\n");
         return;
      }
   }

   if (ud->level == 0) {
      if (!strcmp(name,"LWPR")) {
         int nIn = 0,nOut = 0;
         const char *model_name = NULL;
         LWPR_Kernel kern = LWPR_GAUSSIAN_KERNEL;
         at = atts;

         ud->curType = 0;
         while (at[0]!=NULL && at[1]!=NULL) {
            if (!strcmp(at[0],"name")) {
               model_name = at[1];
            } else if (!strcmp(at[0],"nIn")) {
               nIn = atoi(at[1]);
            } else if (!strcmp(at[0],"nOut")) {
               nOut = atoi(at[1]);
            } else if (!strcmp(at[0],"kernel")) {
               kern = LWPR_GAUSSIAN_KERNEL;
               if (!strcmp(at[1],"BiSquare")) {
                  kern = LWPR_BISQUARE_KERNEL;
               } else if (!strcmp(at[1],"Bisquare")) {
                  kern = LWPR_BISQUARE_KERNEL;
               } else {
                  if (strcmp(at[1],"Gaussian")) {
                     ud->numWarnings++;
                     if (ud->errFile) fprintf(ud->errFile,"Unknown kernel, using Gaussian.\n");
                  }
               }
            }
            at+=2;
         }
         if (nIn>0 && nOut > 0) {
            lwpr_init_model(model,nIn,nOut,model_name);
            model->kernel = kern;
         } else {
            ud->numErrors++;
            if (ud->errFile) fprintf(ud->errFile,"Error parsing LWPR element.\n");
         }
         ud->level = 1;
      } else {
         lwpr_xml_report_unknown(ud,name);
      }
      return;
   }

   if (ud->level == 1) {
      if (!strcmp(name,"SubModel")) {
         int out_dim=-1;
         int numRFS=0;
         ud->curType = 0;
         at = atts;
         while (at[0]!=NULL && at[1]!=NULL) {
            if (!strcmp(at[0],"out_dim")) {
               out_dim = atoi(at[1]);
            } else if (!strcmp(at[0],"numRFS")) {
               numRFS = atoi(at[1]);
            }
            at+=2;
         }
         if (out_dim >= 0 && out_dim < model->nOut) {
            lwpr_mem_alloc_sub(&(model->sub[out_dim]), numRFS + 16);
            ud->level = 2;
            ud->curSub = out_dim;
            ud->curRF = 0;
         } else {
            ud->numErrors++;
            if (ud->errFile) fprintf(ud->errFile,"Error parsing SubModel element.\n");
         }
         return;
      }
      switch(ud->curType) {
         case 0:
            lwpr_xml_report_unknown(ud,name);
            break;
         case 1:
            ud->N = 1;
            if (!strcmp(fieldName,"n_data")) {
               ud->curPtr = (void *) &(model->n_data);
            } else if (!strcmp(fieldName,"diag_only")) {
               ud->curPtr = (void *) &(model->diag_only);
            } else if (!strcmp(fieldName,"update_D")) {
               ud->curPtr = (void *) &(model->update_D);
            } else if (!strcmp(fieldName,"meta")) {
               ud->curPtr = (void *) &(model->meta);
            } else {
               lwpr_xml_report_unknown(ud,fieldName);
            }
            break;
         case 2:
            ud->N = 1;
            if (!strcmp(fieldName,"meta_rate")) {
               ud->curPtr = (void *) &(model->meta_rate);
            } else if (!strcmp(fieldName,"penalty")) {
               ud->curPtr = (void *) &(model->penalty);
            } else if (!strcmp(fieldName,"w_gen")) {
               ud->curPtr = (void *) &(model->w_gen);
            } else if (!strcmp(fieldName,"w_prune")) {
               ud->curPtr = (void *) &(model->w_prune);
            } else if (!strcmp(fieldName,"init_lambda")) {
               ud->curPtr = (void *) &(model->init_lambda);
            } else if (!strcmp(fieldName,"final_lambda")) {
               ud->curPtr = (void *) &(model->final_lambda);
            } else if (!strcmp(fieldName,"tau_lambda")) {
               ud->curPtr = (void *) &(model->tau_lambda);
            } else if (!strcmp(fieldName,"init_S2")) {
               ud->curPtr = (void *) &(model->init_S2);
            } else if (!strcmp(fieldName,"add_threshold")) {
               ud->curPtr = (void *) &(model->add_threshold);
            } else {
               lwpr_xml_report_unknown(ud,fieldName);
            }
            break;
         case 3:
            if (!strcmp(fieldName,"mean_x")) {
               ud->curPtr = (void *) model->mean_x;
               wishN = model->nIn;
            } else if (!strcmp(fieldName,"var_x")) {
               ud->curPtr = (void *) model->var_x;
               wishN = model->nIn;
            } else if (!strcmp(fieldName,"norm_in")) {
               ud->curPtr = (void *) model->norm_in;
               wishN = model->nIn;
            } else if (!strcmp(fieldName,"norm_out")) {
               ud->curPtr = (void *) model->norm_out;
               wishN = model->nOut;
            } else {
               lwpr_xml_report_unknown(ud,fieldName);
               break;
            }
            if (wishN != N) {
               lwpr_xml_dim_error(ud,fieldName,0,wishN);
            } else {
               ud->N = N;
            }
            break;
         case 4:
            wishN = wishM = model->nIn;
            if (!strcmp(fieldName,"init_alpha")) {
               ud->curPtr = (void *) model->init_alpha;
            } else if (!strcmp(fieldName,"init_D")) {
               ud->curPtr = (void *) model->init_D;
            } else if (!strcmp(fieldName,"init_M")) {
               ud->curPtr = (void *) model->init_M;
            } else {
               lwpr_xml_report_unknown(ud,fieldName);
               break;
            }
            if (wishN != N || wishM != M) {
               lwpr_xml_dim_error(ud,fieldName,wishM,wishN);
            } else {
               ud->M = ud->N = model->nIn;
               ud->MS = model->nInStore;
            }
            break;

      }

      return;
   }
   if (ud->level == 2) {
      if (!strcmp(name,"ReceptiveField")) {
         int nReg = 0;

         at = atts;
         while (at[0]!=NULL && at[1]!=NULL) {
            if (!strcmp(at[0],"nReg")) {
               nReg = atoi(at[1]);
            }
            at+=2;
         }
         if (nReg > 0) {
            RF = lwpr_aux_add_rf(sub,nReg);
            ud->level = 3;
         } else {
            ud->numErrors++;
            if (ud->errFile) fprintf(ud->errFile,"Error parsing ReceptiveField element %d/%d.\n",ud->curSub,ud->curRF);
         }

         ud->level = 3;

         return;
      }

      ud->curPtr = NULL;
      if (ud->curType == 1 && !strcmp(fieldName,"n_pruned")) {
         ud->curPtr = (void *) &sub->n_pruned;
         ud->N = 1;
         return;
      }
      if (ud->curType == 0) {
         lwpr_xml_report_unknown(ud,name);
      } else {
         lwpr_xml_report_unknown(ud,fieldName);
      }
      return;
   }
   if (ud->level == 3) {
      ud->curPtr = NULL;
      switch(ud->curType) {
         case 0:
            lwpr_xml_report_unknown(ud,name);
            break;
         case 1:
            if (!strcmp(fieldName,"trustworthy")) {
               ud->curPtr = (void *) &RF->trustworthy;
               ud->N = 1;
            } else {
               lwpr_xml_report_unknown(ud,fieldName);
            }
            break;
         case 2:
            ud->N = 1;
            if (!strcmp(fieldName,"beta0")) {
               ud->curPtr = (void *) &RF->beta0;
            } else if (!strcmp(fieldName,"sum_e2")) {
               ud->curPtr = (void *) &RF->sum_e2;
            } else if (!strcmp(fieldName,"SSp")) {
               ud->curPtr = (void *) &RF->SSp;
            } else if (!strcmp(fieldName,"w")) {
               ud->curPtr = (void *) &RF->w;
            } else {
               lwpr_xml_report_unknown(ud,fieldName);
            }
            break;
         case 3:
            ud->N = N;
            if (!strcmp(fieldName,"beta")) {
               ud->curPtr = (void *) RF->beta;
               wishN = RF->nReg;
            } else if (!strcmp(fieldName,"c")) {
               ud->curPtr = (void *) RF->c;
               wishN = model->nIn;
            } else if (!strcmp(fieldName,"SSs2")) {
               ud->curPtr = (void *) RF->SSs2;
               wishN = RF->nReg;
            } else if (!strcmp(fieldName,"SSYres")) {
               ud->curPtr = (void *) RF->SSYres;
               wishN = RF->nReg;
            } else if (!strcmp(fieldName,"H")) {
               ud->curPtr = (void *) RF->H;
               wishN = RF->nReg;
            } else if (!strcmp(fieldName,"r")) {
               ud->curPtr = (void *) RF->r;
               wishN = RF->nReg;
            } else if (!strcmp(fieldName,"sum_w")) {
               ud->curPtr = (void *) RF->sum_w;
               wishN = RF->nReg;
            } else if (!strcmp(fieldName,"sum_e_cv2")) {
               ud->curPtr = (void *) RF->sum_e_cv2;
               wishN = RF->nReg;
            } else if (!strcmp(fieldName,"n_data")) {
               ud->curPtr = (void *) RF->n_data;
               wishN = RF->nReg;
            } else if (!strcmp(fieldName,"lambda")) {
               ud->curPtr = (void *) RF->lambda;
               wishN = RF->nReg;
            } else if (!strcmp(fieldName,"mean_x")) {
               ud->curPtr = (void *) RF->mean_x;
               wishN = model->nIn;
            } else if (!strcmp(fieldName,"var_x")) {
               ud->curPtr = (void *) RF->var_x;
               wishN = model->nIn;
            } else if (!strcmp(fieldName,"s")) {
               ud->curPtr = (void *) RF->s;
               wishN = RF->nReg;
            } else {
               lwpr_xml_report_unknown(ud,fieldName);
               return;
            }
            if (wishN != N) {
               lwpr_xml_dim_error(ud,fieldName,0,wishN);
            }
            break;
         case 4:
            ud->M = M;
            ud->N = N;
            if (!strcmp(fieldName,"D")) {
               ud->curPtr = (void *) RF->D;
               wishM = wishN = model->nIn;
            } else if (!strcmp(fieldName,"M")) {
               ud->curPtr = (void *) RF->M;
               wishM = wishN = model->nIn;
            } else if (!strcmp(fieldName,"alpha")) {
               ud->curPtr = (void *) RF->alpha;
               wishM = wishN = model->nIn;
            } else if (!strcmp(fieldName,"b")) {
               ud->curPtr = (void *) RF->b;
               wishM = wishN = model->nIn;
            } else if (!strcmp(fieldName,"h")) {
               ud->curPtr = (void *) RF->h;
               wishM = wishN = model->nIn;
            } else if (!strcmp(fieldName,"SXresYres")) {
               ud->curPtr = (void *) RF->SXresYres;
               wishM = model->nIn;
               wishN = RF->nReg;
            } else if (!strcmp(fieldName,"SSXres")) {
               ud->curPtr = (void *) RF->SSXres;
               wishM = model->nIn;
               wishN = RF->nReg;
            } else if (!strcmp(fieldName,"U")) {
               ud->curPtr = (void *) RF->U;
               wishM = model->nIn;
               wishN = RF->nReg;
            } else if (!strcmp(fieldName,"P")) {
               ud->curPtr = (void *) RF->P;
               wishM = model->nIn;
               wishN = RF->nReg;
            } else {
               lwpr_xml_report_unknown(ud,fieldName);
               return;
            }
            if (wishN != N || wishM != M) {
               lwpr_xml_dim_error(ud,fieldName,wishM,wishN);
            }
            break;
      }
   }
}


void lwpr_xml_end_element(void *userData, const char *name) {
   LWPR_ParserData *ud = (LWPR_ParserData *) userData;
   int curType = ud->curType;
   ud->curType = 0;


   if (curType==0) {
      if (ud->level == 1 && !strcmp(name,"LWPR")) {
         ud->level = 0;
         return;
      }
      if (ud->level == 2 && !strcmp(name,"SubModel")) {
         ud->level = 1;
         return;
      }
      if (ud->level == 3 && !strcmp(name,"ReceptiveField")) {
         ud->level = 2;
         ud->curRF++;
         return;
      }
      return;
   }

   if (ud->readN==ud->N && ud->readM==ud->M) return;
   lwpr_xml_error(ud,NULL);
   if (ud->errFile == NULL) return;
   switch(curType) {
      case 1:
         fprintf(ud->errFile,"Integer value could not be read.\n");
         break;
      case 2:
         fprintf(ud->errFile,"Scalar value could not be read.\n");
         break;
      case 3:
         fprintf(ud->errFile,"Vector was declared with %d elements, but only %d were read.\n",ud->N,ud->readN);
         break;
      case 4:
         fprintf(ud->errFile,"Matrix was declared with %dx%d elements, but only %dx%d were read.\n",ud->M,ud->N,ud->readM,ud->readN);
         break;
   }
}

void lwpr_xml_handle_data(void *userData, const char *s, int len) {
   LWPR_ParserData *ud = (LWPR_ParserData *) userData;
   char *end;
   int iVal;
   double dVal;
   double *dest;
   int read=0;

   if (ud->curPtr == NULL) return;

   switch(ud->curType) {
      case 1:
         while (read<len) {
            iVal = strtol(s,&end,10);
            if (s==end) break;

            read += (end-s);
            if (ud->readN==0) {
               *((int *) ud->curPtr) = iVal;
               ud->readN = 1;
            } else {
               lwpr_xml_error(ud,"Too many elements in integer field.\n");
            }
            s=end;
         }
         break;
      case 2:
         while (read<len) {
            dVal = strtod(s,&end);
            if (s==end) break;
            read += (end-s);

            if (ud->readN==0) {
               *((double *) ud->curPtr) = dVal;
               ud->readN = 1;
            } else {
               lwpr_xml_error(ud,"Too many elements in scalar field.\n");
            }
            s=end;
         }
         break;
      case 3:
         dest = (double *) ud->curPtr;
         while (1) {
            dVal = strtod(s,&end);
            if (s==end) break;

            read += (end-s);
            if (read>len) break;

            if (ud->readN == ud->N) {
               lwpr_xml_error(ud,"Too many elemtents in vector field.\n");
               break;
            }
            dest[ud->readN++] = dVal;
            s=end;
         }
         break;
      case 4:
         dest = (double *) ud->curPtr;
         while (1) {
            dVal = strtod(s,&end);

            if (s==end) break;
            read += (end-s);

            if (read>len) break;

            if (ud->readN == ud->N && ud->readM == ud->M) {
               lwpr_xml_error(ud,"Too many elemtents in matrix field.\n");
               break;
            }
            dest[ud->readM + ud->readN*ud->MS] = dVal;
            if (++ud->readN == ud->N) {
               if (++ud->readM < ud->M) ud->readN=0;
            }
            s=end;
         }
         break;
   }
}

int lwpr_xml_read_file_into_buffer(const char *filename, char **buffer) {
   int length,numRead;
   char *buf;
   FILE *fp;

   fp = fopen(filename,"r");
   if (fp==NULL) return -1;

   fseek(fp,0,SEEK_END);
   length=ftell(fp);
   buf = (char *) LWPR_MALLOC((size_t)length);
   if (buf==NULL) {
      fclose(fp);
      return -1;
   }

   *buffer = buf;

   rewind(fp);
   numRead = 0;
   while (numRead<length) {
      size_t n = fread(buf,1,(size_t)(length-numRead),fp);
      if (n==0) {
         LWPR_FREE(*buffer);
         fclose(fp);
         return -1;
      }
      buf +=n;
      numRead+=n;
   }
   fclose(fp);
   return length;
}

#if HAVE_LIBEXPAT

int lwpr_read_xml(LWPR_Model *model, const char *filename, int *numWarnings) {
   char *buffer;
   int length;
   XML_Parser parser;
   LWPR_ParserData ud;
   int status;

   model->nOut = 0;
   model->sub = NULL;

   length=lwpr_xml_read_file_into_buffer(filename,&buffer);

   if (length<=0) return -1;

   ud.level = 0;
   ud.numSub = 0;
   ud.curSubNumRF = 0;
   ud.curPtr = NULL;
   ud.curType = 0;
   ud.curRF = 0;
   ud.curSub = 0;
   ud.model = model;
   ud.numErrors = ud.numWarnings = 0;
   ud.errFile = stderr;

   parser = XML_ParserCreate("US-ASCII");
   XML_SetUserData(parser,&ud);
   XML_SetElementHandler(parser,lwpr_xml_start_element,lwpr_xml_end_element);
   XML_SetCharacterDataHandler(parser, lwpr_xml_handle_data);
   status = XML_Parse(parser,buffer,length,1);
   XML_ParserFree(parser);

   if (status == XML_STATUS_ERROR) ud.numErrors+=10000;

   LWPR_FREE(buffer);
   if (numWarnings!=NULL) *numWarnings = ud.numWarnings;

   return ud.numErrors;
}

#else

int lwpr_read_xml(LWPR_Model *model, const char *filename, int *numWarnings) {
   fprintf(stderr,"LWPR library has been compiled without XML-reading support (depends on EXPAT)\n");
   numWarnings = 0;
   return -2;
}

#endif
