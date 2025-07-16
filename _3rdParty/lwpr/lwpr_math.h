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


/** \file lwpr_math.h
   \brief Prototypes for some rather simple vector and matrix operations
   \ingroup LWPR_C       
*/

#ifndef __LWPR_MATH_H
#define __LWPR_MATH_H

#ifdef __cplusplus
extern "C" {
#endif


/** \brief Computes the square norm of a vector of doubles.

   \param[in] x   Input vector, must point to an array of <em>n</em> doubles
   \param[in] n   Length of the vector
   \return  The square norm \f[\|\mathbf{x}\|^2 = \sum_{i=1}^n x_i^2\f]
*/      
double lwpr_math_norm2(const double *x, int n);

/** \brief Computes the dot product between two vectors of doubles.

   \param[in] a   First input vector, must point to an array of <em>n</em> doubles
   \param[in] b   Second input vector, must point to an array of <em>n</em> doubles
   \param[in] n   Length of the vectors
   \return  The dot product \f[\mathbf{a}\cdot\mathbf{b} = \sum_{i=1}^n a_i b_i\f]
*/      
double lwpr_math_dot_product(const double *a,const double *b,int n);


/** \brief Multiplies a vector by a scalar and stores the result in another vector.
  
   \param[out] y  Output vector, must point to an array of <em>n</em> doubles
   \param[in] a   Scalar multiplier
   \param[in] x   Input vector, must point to an array of <em>n</em> doubles
   \param[in] n   Length of the vectors

   Computes \f[\mathbf{y} \leftarrow a\mathbf{x}\quad\Leftrightarrow\quad y_i \leftarrow a x_i \quad i=1\dots n\f]
*/    
void lwpr_math_scalar_vector(double *y, double a,const double *x,int n);

/** \brief Multiplies a vector by a scalar and adds the result to another vector.
  
   \param[in,out] y  Output vector, must point to an array of <em>n</em> doubles
   \param[in] a   Scalar multiplier
   \param[in] x   Input vector, must point to an array of <em>n</em> doubles
   \param[in] n   Length of the vectors

   Computes \f[\mathbf{y} \leftarrow \mathbf{y} + a\mathbf{x}\quad\Leftrightarrow\quad y_i \leftarrow y_i + a x_i \quad i=1\dots n\f]
*/    
void lwpr_math_add_scalar_vector(double *y, double a,const double *x,int n);

/** \brief Multiplies a vector by a scalar and adds the result to another vector, which
      is scaled before the addition.
  
   \param[in] b   Scalar multiplier for <em>y</em> before the addition
   \param[in,out] y  Output vector, must point to an array of <em>n</em> doubles
   \param[in] a   Scalar multiplier for <em>x</em>
   \param[in] x   Input vector, must point to an array of <em>n</em> doubles
   \param[in] n   Length of the vectors

   Computes \f[\mathbf{y} \leftarrow b\mathbf{y} + a\mathbf{x}\quad\Leftrightarrow\quad y_i \leftarrow b y_i + a x_i \quad i=1\dots n\f]
*/    
void lwpr_math_scale_add_scalar_vector(double b, double *y, double a,const double *x,int n);

/** \brief Computes the Cholesky decomposition of a matrix.

   \param[in] N   Number of columns and rows of the matrix
   \param[in] Ns  Stride parameter, i.e. offset between the first element of adjacent columns
   \param[in,out] R   Upper triangular Cholesky factor. Also serves as input matrix if <em>A==NULL</em>
   \param[in] A   Matrix to decompose. May be NULL, in which case the decomposition of R is done in place.
   \return  
      - 1 in case of succes
      - 0 in case of failure (e.g. input matrix is not positive definite)
      
   Given a positive definite matrix <em>A</em>, this function computes an 
   upper triangular matrix <em>R</em> such that
   \f[\mathbf{R}^T\mathbf{R} = \mathbf{A}.\f]
   Since the decomposition is done in place, you can also call
   \code
   lwpr_math_cholesky(n,n,R,NULL);
   \endcode
   if you can afford to overwrite the original contents of the matrix.
*/   
int lwpr_math_cholesky(int N,int Ns,double *R,const double *A);

#ifdef __cplusplus
}
#endif

#endif
