
/*
//---------------- Author: Francesco Castellini* and Annalisa Riccardi** ---------------//
//------- Affiliation: -----------------------------------------------------------------//
//--------------* Dipartimento di Ingegneria Aerospaziale, Politecnico di Milano -------//
//--------------**Centrum for Industrial Mathematics, University of Bremen -------------//
//--------E-mail: castellfr@gmail.com, nina1983@gmail.com ------------------------------//

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>. 
*/

#include "memoryAllocation.h"
#include <iostream>
#include <stdlib.h>

using namespace std;

double** dmatrix_allocation(int nrow,int ncol){
/* Function allocating memory for a matrix of (nrow x ncol) double type numbers
   Inputs:
   nrow,ncol = number of rows and columns
   Outputs:
   mat = matrix, defined as an array of pointers to double pointers
*/
	int i;
	double **mat;
	mat = new(nothrow) double*[nrow];
	//mat = (double**) malloc (nrow*sizeof(double*));

	if(mat == NULL)	{
		cerr<<"Error: Not enough memory for matrix allocation"<<endl;
		exit(1);
	}
	for(i=0;i<nrow;i++)	{
//		mat[i] = (double*) malloc (ncol*sizeof(double));
		mat[i] = new(nothrow) double [ncol];

		if(mat[i] == NULL)	{
			cerr<<"Error: Not enough memory for matrix allocation"<<endl;
			exit(1);
		}
	}
	return mat;
}

void dmatrix_free(double **mat, int nrow, int ncol)
/* Function DE-allocating memory for a matrix of (nrow x ncol) double type numbers allocated with dmatrix_allocation
   Inputs:
   nrow,ncol = number of rows and columns
   mat = matrix, defined as an array of pointers to double pointers
*/
{
	for (int i = 0; i < nrow; i++)
	  delete[] mat[i];

	delete[] mat;

}

void initialize_dmatrix (double **matrix, int nrow, int ncol)
{

	int i,j;

	for(i=0;i<nrow;i++)
		for(j=0;j<ncol;j++)
			matrix[i][j]=0.0;

}
double*** d3darray_allocation(int npage,int nrow,int ncol)
/* Function allocating memory for a 3D array of (npage x nrow x ncol) double type numbers
   Inputs:
   npage,nrow,ncol = number of pages, rows and columns
   Outputs:
   array = 3D array, defined as an array of pointers to an array of pointers to double pointers
   pppd: pointer to pointer to pointer to double
*/
{
	int i;
	double ***arrays;

	arrays = new(nothrow) double**[npage];
	//arrays = (double***)malloc(npage*sizeof(double**));
	if(arrays == NULL)
		{
			cerr<<"\nError: Not enough memory"<<endl;
			exit(1);
		}

	for(i=0;i<npage;i++)
		arrays[i]=dmatrix_allocation(nrow,ncol); //allocating memory for a matrix for each page of the 3D array

	return arrays;
}

void d3darray_free(double*** arrays,int npage,int nrow,int ncol){
	for(int i=0;i<npage;i++)
		dmatrix_free(arrays[i],nrow,ncol);
	delete[] arrays;
}


int** imatrix_allocation(int nrow,int ncol)
{
/* Function allocating memory for a matrix of (nrow x ncol) int type numbers
   Inputs:
   nrow,ncol = number of rows and columns
   Outputs:
   mat = matrix, defined as an array of pointers to int pointers */

	int i;
	int **mat;
	mat = new int*[nrow];
	//mat = (int **)malloc(nrow*sizeof(int*));

	if(mat == NULL)	{
		cerr<<"\nError: Not enough memory"<<endl;
		exit(1);
	}
	for(i=0;i<nrow;i++)	{
		mat[i] = new int [ncol];
		//mat[i] = (int*)malloc((size_t)(ncol*sizeof(int)));

		if(mat[i] == NULL)	{
			cerr<<"\nError: Not enough memory"<<endl;
			exit(1);
		}
	}
	return mat;
}

void imatrix_free(int** mat,int nrow,int ncol){
	for (int i = 0; i < nrow; i++)
	  delete[] mat[i];
	delete[] mat;
}

int*** i3darray_allocation(int npage,int nrow,int ncol)
{
/* Function allocating memory for a 3D array of (npage x nrow x ncol) double type numbers
   Inputs:
   npage,nrow,ncol = number of pages, rows and columns
   Outputs:
   array = 3D array, defined as an array of pointers to an array of pointers to double pointers
   pppd: pointer to pointer to pointer to double*/

	int i;
	int ***array;

	array = new int**[npage];
	//array = (int***)malloc(npage*sizeof(int**));
	if(array == NULL){
		cerr<<"\nError: Not enough memory"<<endl;
		exit(1);
	}

	for(i=0;i<npage;i++)
		array[i]=imatrix_allocation(nrow,ncol);	//allocating memory for a matrix for each page of the 3D array

	return array;
}

void i3darray_free(int*** arrays,int npage,int nrow,int ncol){
	for(int i=0;i<npage;i++)
		imatrix_free(arrays[i],nrow,ncol);
	delete[] arrays;

}

