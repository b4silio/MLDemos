//-----------------------------------------------------------------------------------
// File name: memoryAllocation.h
//
//-Copyright (C) 2010 European Space Agency - Politecnico di Milano - Universitat Bremen
//
// This program is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the Free Software
// Foundation; either version 2 of the License, or (at your option) any later
// version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
// details.
//
// You should have received a copy of the GNU Lesser General Public License along with
// this program; if not, write to the Free Software Foundation, Inc., 59 Temple
// Place - Suite 330, Boston, MA  02111-1307, USA.
//
// Further information about the GNU Lesser General Public License can also be found on
// the world wide web at http://www.gnu.org.
//
//---------------- Authors: Francesco Castellini* and Annalisa Riccardi^ ---------------//
//------- Affiliation: *^ European Space Agency ----------------------------------------//
//---------------------* Politecnico di Milano -----------------------------------------//
//---------------------^ University of Bremen ------------------------------------------//
//--------E-mail: * castellfr@gmail.com, ^ nina1983@gmail.com --------------------------//



#ifndef MEMORYALLOCATION_H
#define MEMORYALLOCATION_H

#include <stdio.h>

double** dmatrix_allocation(int nrow,int ncol);

void dmatrix_free(double **mat, int nrow, int ncol);

void initialize_dmatrix (double **matrix, int nrow, int ncol);

double*** d3darray_allocation(int npage,int nrow,int ncol);

void d3darray_free(double*** arrays,int npage,int nrow,int ncol);

int** imatrix_allocation(int nrow,int ncol);

void imatrix_free(int** mat,int nrow,int ncol);

int*** i3darray_allocation(int npage,int nrow,int ncol);

void i3darray_free(int*** arrays,int npage,int nrow,int ncol);

#endif // MEMORYALLOCATION_H
