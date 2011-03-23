/*
 * Copyright (C) 2010 Learning Algorithms and Systems Laboratory, EPFL, Switzerland
 * Author: Eric Sauser
 * email:   eric.sauser@a3.epf.ch
 * website: lasa.epfl.ch
 *
 * Permission is granted to copy, distribute, and/or modify this program
 * under the terms of the GNU General Public License, version 2 or any
 * later version published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
 */

#ifndef __TVECTOR_H__
#define __TVECTOR_H__

#include "MathLibCommon.h"
#define  USE_T_EXTENSIONS
#include <math.h>
#include <iostream>

#include "Vector.h"

#ifdef USE_MATHLIB_NAMESPACE
namespace MathLib {
#endif

template<unsigned int ROW> class TMatrix;

/**
 * \class TVector
 * 
 * \ingroup MathLib
 * 
 * \brief The basic template vector class. See class Vector for function definition
 * 
 * This template vector class can be used for doing various vector manipulation.
 * This should be combined with the Matrix class for doing almost anything
 * you ever dreamt of.
 */
template<unsigned int ROW> class TVector
{
public:
  /*
  friend class Vector;
  friend class Vector3;
  friend class TMatrix<ROW>;
  friend class Matrix3;
  friend class Matrix4;
  */

public:
  static const TVector<ROW>   ZERO;
  static const unsigned int   row   = ROW;
  static       REALTYPE          undef;
   

public:
  REALTYPE _[ROW];

    /// The empty constructor
    inline TVector(bool clear = true){
        if(clear) Zero();
    }
    /// Copy constructor
    inline TVector(const TVector<ROW> &vector){
        Set(vector);
    }
    /// Constructor with data pointer to be copied
    inline TVector(const REALTYPE *array){
        Set(array);
    }

    /// Sets data using data pointer
    inline TVector<ROW>& Set(const REALTYPE *array){
        if(array)
            memcpy(_,array,ROW*sizeof(REALTYPE));
        return *this;
    }
    /// Makes a copy of the TVector argument
    inline TVector<ROW>& Set(const TVector<ROW> &vector){
        memcpy(_,vector._,ROW*sizeof(REALTYPE));
        return *this;    
    }   
    /// Makes a copy of the Vector argument
    inline TVector<ROW>& Set(const Vector &vector){
        const unsigned int k = (ROW<=vector.row?ROW:vector.row);
        memcpy(_,vector._,k*sizeof(REALTYPE));
        if(k<ROW){
            memset(_+k,0,(ROW-k)*sizeof(REALTYPE));
        }
        return *this;
    }   
    /// Set all elements to a given value
    inline TVector<ROW>& Set(const REALTYPE value){
        REALTYPE *dst = (REALTYPE*) _; unsigned int len = ROW;
        while(len--) *(dst++) = value;
        return *this;
    }   

    /// Gets the data array
    inline REALTYPE *Array() const{
        return (REALTYPE*)_;
    }  

    /// Sets all values to zero
    inline void Zero(){
        memset(_,0,ROW*sizeof(REALTYPE));
    }
  
    /// Gets a reference to the row element
    inline REALTYPE& operator[] (const unsigned int row){
        if(row<ROW)
            return _[row];
        return undef; 
    }
    /// Gets a reference to the row element
    inline REALTYPE& operator() (const unsigned int row){
        if(row<ROW)
            return _[row];
        return undef; 
    }
    inline REALTYPE& Ref(const unsigned int row){
        if(row<ROW)
            return _[row];
        return undef; 
    }
    inline REALTYPE& RefNoCheck(const unsigned int row){
        return _[row];
    }
    /// Gets the value of the row element
    inline REALTYPE At(const unsigned int row) const {
        if(row<ROW)
            return _[row];
        return undef; 
    }
    inline REALTYPE AtNoCheck(const unsigned int row) const {
        return _[row];
    }
  
    /// Assigment operator
    inline TVector<ROW>& operator = (const TVector<ROW> &vector) {        
        return Set(vector);    
    }
     
    inline TVector<ROW>& operator = (const REALTYPE &value) {        
        return Set(value);    
    }

    /// Inverse operator
    inline TVector<ROW> operator - () const {
        TVector<ROW> result(false);
        return Minus(result);
    }
    /// Inverse operator
    inline TVector<ROW>& Minus(TVector<ROW>& result) const {
        REALTYPE *src = (REALTYPE*) _;
        REALTYPE *dst = (REALTYPE*) result._;
        unsigned int len = ROW;
        while(len--)
            *(dst++) = -*(src++);
        return result;
    }
    /// Self Inversion
    inline TVector<ROW>& SMinus() {
        REALTYPE *dst = (REALTYPE*) _;
        unsigned int len = ROW;
        while(len--){
            *dst = -*dst;
            dst++;
        }
        return *this;
    }


    /// Assigment data-wise operations 
    inline TVector<ROW>& operator += (const TVector<ROW> &vector) {
        REALTYPE *src = (REALTYPE*) vector._, *dst = (REALTYPE*) _; unsigned int len = ROW;
        while(len--) *(dst++) += *(src++);
        return *this;
    }
    inline TVector<ROW>& operator -= (const TVector<ROW> &vector) {
        REALTYPE *src = (REALTYPE*) vector._, *dst = (REALTYPE*) _; unsigned int len = ROW;
        while(len--) *(dst++) -= *(src++);
        return *this;
    }
    inline TVector<ROW>& operator ^= (const TVector<ROW> &vector){
        REALTYPE *src = (REALTYPE*) vector._, *dst = (REALTYPE*) _; unsigned int len = ROW;
        while(len--) *(dst++) *= *(src++);
        return *this;
    }
    inline TVector<ROW>& operator /= (const TVector<ROW> &vector){
        REALTYPE *src = (REALTYPE*) vector._, *dst = (REALTYPE*) _; unsigned int len = ROW;
        while(len--) *(dst++) /= *(src++);
        return *this;
    }
 

    /// Assigment operations
    inline TVector<ROW>& operator += (REALTYPE scalar){
        REALTYPE *dst = (REALTYPE*) _; unsigned int len = ROW;
        while(len--) *(dst++) += scalar;
        return *this;
    }
    inline TVector<ROW>& operator -= (REALTYPE scalar){
        REALTYPE *dst = (REALTYPE*) _; unsigned int len = ROW;
        while(len--) *(dst++) -= scalar;
        return *this;
    }
    inline TVector<ROW>& operator *= (REALTYPE scalar){
        REALTYPE *dst = (REALTYPE*) _; unsigned int len = ROW;
        while(len--) *(dst++) *= scalar;
        return *this;
    }
    inline TVector<ROW>& operator /= (REALTYPE scalar){
        scalar = R_ONE/scalar;
        REALTYPE *dst = (REALTYPE*) _; unsigned int len = ROW;
        while(len--) *(dst++) *= scalar;
        return *this;
    }



    /// Vector data-wise operators
    inline TVector<ROW> operator + (const TVector<ROW> &vector) const {
        TVector<ROW> result(false);
        return Add(vector,result);
    }
    inline TVector<ROW> operator - (const TVector<ROW> &vector) const {
        TVector<ROW> result(false);
        return Sub(vector,result); 
    }
    inline TVector<ROW> operator ^ (const TVector<ROW> &vector) const {
        TVector<ROW> result(false);
        return Mult(vector,result);
    }
    inline TVector<ROW> operator / (const TVector<ROW> &vector) const {
        TVector<ROW> result(false);
        return Div(vector,result); 
    }

    /// Vector data-wise operations (faster than using operators)
    inline TVector<ROW>& Add(const TVector<ROW> &vector, TVector<ROW> &result) const {
        REALTYPE *src0 = (REALTYPE*) _, *src1 = (REALTYPE*) vector._, *dst = (REALTYPE*) result._; unsigned int len = ROW;
        while(len--) *(dst++) = *(src0++) + (*(src1++));
        return result;
    }
    inline TVector<ROW>& Sub(const TVector<ROW> &vector, TVector<ROW> &result) const{
        REALTYPE *src0 = (REALTYPE*) _, *src1 = (REALTYPE*) vector._, *dst = (REALTYPE*) result._; unsigned int len = ROW;
        while(len--) *(dst++) = *(src0++) - (*(src1++));
        return result;
    }
    inline TVector<ROW>& Mult(const TVector<ROW> &vector, TVector<ROW> &result) const{
        REALTYPE *src0 = (REALTYPE*) _, *src1 = (REALTYPE*) vector._, *dst = (REALTYPE*) result._; unsigned int len = ROW;
        while(len--) *(dst++) = *(src0++) * (*(src1++));
        return result;
    }
    inline TVector<ROW>& Div(const TVector<ROW> &vector, TVector<ROW> &result) const{
        REALTYPE *src0 = (REALTYPE*) _, *src1 = (REALTYPE*) vector._, *dst = (REALTYPE*) result._; unsigned int len = ROW;
        while(len--) *(dst++) = *(src0++) / (*(src1++));
        return result;
    }


    /// Scalar operations using operators
    inline TVector<ROW> operator + (REALTYPE scalar) const {
        TVector<ROW> result(false);
        return Add(scalar,result);
    }
    inline TVector<ROW> operator - (REALTYPE scalar) const {
        TVector<ROW> result(false);
        return Sub(scalar,result);
    }
    inline TVector<ROW> operator * (REALTYPE scalar) const {
        TVector<ROW> result(false);
        return Mult(scalar,result);
    }
    inline TVector<ROW> operator / (REALTYPE scalar) const {
        TVector<ROW> result(false);
        return Div(scalar,result);
    }

    /// Scalar operations with result as a parameter (faster than pure operators)
    inline TVector<ROW>& Add(REALTYPE scalar, TVector<ROW>& result) const {
        REALTYPE *src = (REALTYPE*) _, *dst = (REALTYPE*) result._; unsigned int len = ROW;
        while(len--) *(dst++) = *(src++)  + scalar;
        return result;
    }
    inline TVector<ROW>& Sub(REALTYPE scalar, TVector<ROW>& result) const{
        REALTYPE *src = (REALTYPE*) _, *dst = (REALTYPE*) result._; unsigned int len = ROW;
        while(len--) *(dst++) = *(src++)  - scalar;
        return result;
    }
    inline TVector<ROW>& Mult(REALTYPE scalar, TVector<ROW>& result) const{
        REALTYPE *src = (REALTYPE*) _, *dst = (REALTYPE*) result._; unsigned int len = ROW;
        while(len--) *(dst++) = *(src++)  * scalar;
        return result;
    }
    inline TVector<ROW>& Div(REALTYPE scalar, TVector<ROW>& result) const{
        scalar = R_ONE/scalar;
        REALTYPE *src = (REALTYPE*) _, *dst = (REALTYPE*) result._; unsigned int len = ROW;
        while(len--) *(dst++) = *(src++)  * scalar;
        return result;
    }

    inline TVector<ROW>& ScaleAddTo(REALTYPE scale, TVector<ROW> &result) const{
        REALTYPE *src = (REALTYPE*) _, *dst = (REALTYPE*) result._; unsigned int len = ROW;
        while(len--) *(dst++) += *(src++)  * scale;
        return result;
    }


    /// Tests equality of two vectors
    inline bool operator == (const TVector<ROW>& vector) const {
        REALTYPE *src = (REALTYPE*) _, *dst = (REALTYPE*) vector._; unsigned int len = ROW;
        while(len--) 
            if(*(dst++) != *(src++))
                return false;
        return true;
    }
    /// tests inequality of two vectors
    inline bool operator != (const TVector<ROW>& vector) const {
        return !(*this ==  vector);
    }

    /// Returns the norm
    inline REALTYPE Norm() const {
        return sqrt(Norm2());
    }
    /// Returns the squared norm
    inline REALTYPE Norm2() const {
        REALTYPE result = R_ZERO;    
        REALTYPE *src = (REALTYPE*) _; unsigned int len = ROW;
        while(len--){
            result += (*src) * (*(src));
            src++;
        } 
        return result;
    }
    /// Normalize the data to 1
    inline void Normalize() {
        REALTYPE scalar = R_ONE / Norm();
        (*this)*=scalar;
    }
  
    /// Performs the dot product
    inline REALTYPE Dot(const TVector<ROW> &vector) const {
        REALTYPE result = R_ZERO;
        REALTYPE *src = (REALTYPE*) _, *dst = (REALTYPE*) vector._; unsigned int len = ROW;
        while(len--) 
            result += *(dst++) * (*(src++));
        return result;     
    }

    inline TMatrix<ROW> & MultTranspose(TVector<ROW> & vector, TMatrix<ROW> & result) const{
        REALTYPE *src0 = (REALTYPE*) _;
        REALTYPE *dst  = (REALTYPE*) result._;
        unsigned int colLen = ROW;
        while(colLen--) {
            REALTYPE *src1 = (REALTYPE*) vector._;
            unsigned int rowLen = ROW;
            while(rowLen--)
                *(dst++) = *(src0) *(*(src1++));
            src0++;
        }      
        return result;
    }

    /// Truncs the data between minVal and maxVal
    inline TVector<ROW> Trunc(const REALTYPE minVal, const REALTYPE maxVal) {
        REALTYPE *src = (REALTYPE*) _; unsigned int len = ROW;
        while(len--){
            *src = TRUNC(*src,minVal,maxVal);
            src++;
        } 
        return *this;     
    }

  /// Truncs each data between each minVal and maxVal vectors
  inline TVector<ROW> Trunc(const TVector<ROW> &minVal, const TVector<ROW> &maxVal) 
  {
    for (unsigned int i = 0; i < ROW; i++)
      _[i] = TRUNC(_[i],minVal._[i],maxVal._[i]);
    return *this;     
  }
  
  /// Prints out the vector to stdout 
  void Print() const
  {
    std::cout << "Vector" <<ROW<<std::endl;;
    for (unsigned int i = 0; i < ROW; i++)
      std::cout << _[i] <<" ";
    std::cout << std::endl;    
  }
};

template<unsigned int ROW> const TVector<ROW> TVector<ROW>::ZERO;

template<unsigned int ROW> REALTYPE TVector<ROW>::undef = R_ZERO;


#ifdef USE_MATHLIB_NAMESPACE
}
#endif
#endif
