#include <stdlib.h>
#include "fgmm.h"
#include "gaussian.h"


void fgmm_update(struct gmm * gmm,const _fgmm_real * data_point)
{
  int state_i=0;
  _fgmm_real * pxi;
  _fgmm_real nf = 0;
  _fgmm_real eta = .05;
  _fgmm_real weighted_lr;
   
  pxi = (_fgmm_real *) malloc( sizeof(_fgmm_real) * gmm->nstates);
  for(;state_i<gmm->nstates;state_i++)
    {
      pxi[state_i] = gaussian_pdf(&gmm->gauss[state_i], data_point);
      nf += pxi[state_i];
    }
 
  for(state_i=0;state_i<gmm->nstates;state_i++)
    {
      weighted_lr = eta * pxi[state_i]/nf;
      gaussian_update(&gmm->gauss[state_i],data_point,weighted_lr);
      invert_covar(&gmm->gauss[state_i]);
    }
	free(pxi);	
}


/* winner take all approach */

void fgmm_update_wta(struct gmm * gmm,const _fgmm_real * data_point)
{
  int state_i=0;
  int the_state = 0;
  _fgmm_real eta = .05;
  _fgmm_real max_p = 0.;
  _fgmm_real w =  0.;
  for(;state_i<gmm->nstates;state_i++)
    {
      w = gaussian_pdf(&gmm->gauss[state_i], data_point);
      if(w>max_p) 
	{
	  max_p = w;
	  the_state = state_i;
	}
    }
  gaussian_update(&gmm->gauss[the_state],data_point,eta);
  invert_covar(&gmm->gauss[the_state]);
}
  
  
