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


/** \file lwpr_mem.h
   \brief Prototypes for auxiliary LWPR routines that handle memory allocation and disposition.

   When compiling the library for usage within MEX-files, it is important
   to use Matlab's memory management, because otherwise memory leaks may
   occur. The macros LWPR_MALLOC(), LWPR_CALLOC(), LWPR_REALLOC(), and LWPR_FREE()
   handle everything automatically.
   \ingroup LWPR_C       
*/

#ifndef __LWPR_MEM_H
#define __LWPR_MEM_H

#ifndef MATLAB
   #define LWPR_MALLOC(s)    malloc(s)       /**< \brief Standard malloc, or mxMalloc if compiling MEX-files */
   #define LWPR_CALLOC(n,s)  calloc(n,s)     /**< \brief Standard calloc, or mxCalloc if compiling MEX-files */
   #define LWPR_REALLOC(p,s) realloc(p,s)    /**< \brief Standard realloc, or mxRealloc if compiling MEX-files */
   #define LWPR_FREE(p)      free(p)         /**< \brief Standard free, or mxFree if compiling MEX-files */
#else
   #include <mex.h>
   #define LWPR_MALLOC(s)    mxMalloc(s)     /**< \brief Standard malloc, or mxMalloc if compiling MEX-files */
   #define LWPR_CALLOC(n,s)  mxCalloc(n,s)   /**< \brief Standard calloc, or mxCalloc if compiling MEX-files */
   #define LWPR_REALLOC(p,s) mxRealloc(p,s)  /**< \brief Standard realloc, or mxRealloc if compiling MEX-files */
   #define LWPR_FREE(p)      mxFree(p)       /**< \brief Standard free, or mxFree if compiling MEX-files */
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** \brief Allocates memory for the internal variables of a receptive field.

   \param[in,out] RF     Pointer to a receptive field structure (must already be allocated).
   \param[in] model      Pointer to a valid LWPR model structure. 
   \param[in] nReg       Initial number of PLS regression axes
   \param[in] nRegStore  Number of PLS axes that can initially be stored (>= <em>nReg</em>)
   \return
      - 1 in case of succes
      - 0 in case of failure (e.g. memory could not be allocated).
*/             
int lwpr_mem_alloc_rf(LWPR_ReceptiveField *RF, const LWPR_Model *model, int nReg, int nRegStore);

/** \brief Re-allocates memory for the PLS-related variables of a receptive field.

   \param[in,out] RF     Pointer to a valid receptive field structure.
   \param[in] nRegStore  Number of PLS axes that can be stored. 
   \return
      - 1 in case of succes
      - 0 in case of failure (e.g. memory could not be allocated).
      
   This function does NOT add a new PLS axis, but only creates the space for it.
   Note that in case of failure to do so, the receptive field is still fully 
   functional.
*/         
int lwpr_mem_realloc_rf(LWPR_ReceptiveField *RF, int nRegStore);

/** \brief Disposes the memory for the internal variables of a receptive field.

   \param[in,out] RF     Pointer to a receptive field structure.
   
   Note that this function does not dispose the LWPR_ReceptiveField structure itself.
   \sa lwpr_free_model       
*/
void lwpr_mem_free_rf(LWPR_ReceptiveField *RF);

/** \brief Allocates memory for internal variables of a LWPR workspace structure.

   \param[in,out] ws     Pointer to a LWPR_Workspace structure (must already be allocated).
   \param[in] nIn        Input dimensionality of the LWPR model
   \return
      - 1 in case of succes
      - 0 in case of failure 
*/             
int lwpr_mem_alloc_ws(LWPR_Workspace *ws, int nIn);

/** \brief Disposes the internal memory for an internally used "workspace"

   \param[in,out] ws     Pointer to a LWPR_Workspace structure.
   
   Note that this function does not dispose the LWPR_Workspace structure itself.
   \sa lwpr_free_model       
*/
void lwpr_mem_free_ws(LWPR_Workspace *ws);

/** \brief Allocates memory for internal variables of a LWPR model structure.

   \param[in,out] model  Pointer to an existing LWPR_Model structure
   \param[in] nIn        Input dimensionality of the LWPR model
   \param[in] nOut       Output dimensionality of the LWPR model   
   \param[in] storeRFS   Expected number of receptive fields per output dimension
   \return
      - 1 in case of succes
      - 0 in case of failure 
      
   This function also allocates space for workspaces (one per thread) and
   submodels (one per output dimension), the latter by calling lwpr_mem_alloc_sub().
      
   Note that this function does not allocate the LWPR_Workspace structure itself.
   \sa lwpr_init_model, lwpr_free_model       
*/             
int lwpr_mem_alloc_model(LWPR_Model *model, int nIn, int nOut, int storeRFS);

/** \brief Allocates memory for internal variables of a LWPR submodel structure.

   \param[in,out] sub  Pointer to an existing LWPR_SubModel structure
   \param[in] storeRFS   Expected number of receptive fields 
   \return
      - 1 in case of succes
      - 0 in case of failure 

   Note that storeRFS determines only the number of <b>pointers</b>, that is,
   this function does not allocate any receptive field by itself.
*/      
int lwpr_mem_alloc_sub(LWPR_SubModel *sub, int storeRFS);

#ifdef __cplusplus
}
#endif

#endif
