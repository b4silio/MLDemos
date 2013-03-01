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

#ifndef MATRIX4_H
#define MATRIX4_H

#include "MathLibCommon.h"


#include <math.h>
#include "TMatrix.h"
#include "Vector3.h"
#include "Matrix3.h"

#ifdef USE_MATHLIB_NAMESPACE
namespace MathLib {
#endif

/**
 * \class Matrix4
 * 
 * \ingroup MathLib
 * 
 * \brief An implementation of the template TMatrix class
 * 
 * This template square matrix class can be used for doing various matrix manipulation.
 * This should be combined with the TVector class for doing almost anything
 * you ever dreamt of. 
 */

class Matrix4 :public TMatrix<4>
{
public:
    /// A constant zero matrix
    static const Matrix4 ZERO;
    /// A constant identity matrix
    static const Matrix4 IDENTITY;

    /// Empty constructor
    inline Matrix4(bool clear=true):TMatrix<4>(clear){};
    /// Copy constructor
    inline Matrix4(const Matrix4 &matrix):TMatrix<4>(matrix){};
    /// Copy constructor
    inline Matrix4(const TMatrix<4> &matrix):TMatrix<4>(matrix){};
    /*
    /// Copy constructor
    inline Matrix4(const Matrix & matrix):TMatrix<4>(matrix){};  
    */
    /// Constructor with data pointer
    inline Matrix4(const REALTYPE *array):TMatrix<4>(array){};
    /// Constructor with values
    inline Matrix4(REALTYPE _00, REALTYPE _01, REALTYPE _02, REALTYPE _03,
                   REALTYPE _10, REALTYPE _11, REALTYPE _12, REALTYPE _13,
                   REALTYPE _20, REALTYPE _21, REALTYPE _22, REALTYPE _23,
                   REALTYPE _30, REALTYPE _31, REALTYPE _32, REALTYPE _33):TMatrix<4>(false){
        Set(_00,_01,_02,_03,
            _10,_11,_12,_13,
            _20,_21,_22,_23,
            _30,_31,_32,_33);
    }
    /// Copy constructor of Matrix3 type
    inline Matrix4(const Matrix3 &matrix, const Vector3 &vector):TMatrix<4>(false){
        SetTransformation(matrix, vector);
    }

    inline virtual ~Matrix4(){}

    inline Matrix4& Set(const Matrix4& matrix){
        TMatrix<4>::Set(matrix);
        return  *this;
    }
    /*
    inline Matrix4& Set(const Matrix& matrix){
    TMatrix<4>::Set(matrix);
    return *this;
    }*/

    inline Matrix4& Set(REALTYPE _00, REALTYPE _01, REALTYPE _02, REALTYPE _03,
                        REALTYPE _10, REALTYPE _11, REALTYPE _12, REALTYPE _13,
                        REALTYPE _20, REALTYPE _21, REALTYPE _22, REALTYPE _23,
                        REALTYPE _30, REALTYPE _31, REALTYPE _32, REALTYPE _33){
        REALTYPE *dst = _;
        *(dst++) = _00; *(dst++) = _01; *(dst++) = _02; *(dst++) = _03;
        *(dst++) = _10; *(dst++) = _11; *(dst++) = _12; *(dst++) = _13;
        *(dst++) = _20; *(dst++) = _21; *(dst++) = _22; *(dst++) = _23;
        *(dst++) = _30; *(dst++) = _31; *(dst++) = _32; *(dst++) = _33;
        return *this;
    }
    
    /// Return the translation amount of a transformation matrix
    inline Vector3 GetTranslation() const {
        Vector3 result(false);
        return GetTranslation(result);
    }
    /// Return the translation amount of a transformation matrix
    inline Vector3& GetTranslation(Vector3 &result) const {
        result._[0] = _[0*4+3];
        result._[1] = _[1*4+3];
        result._[2] = _[2*4+3];
        return result;
    }
    /// Set the translation amount of a transformation matrix
    inline Matrix4& SetTranslation(const Vector3 &trans) {
        _[0*4+3] = trans._[0];
        _[1*4+3] = trans._[1];
        _[2*4+3] = trans._[2];
        return *this;
    }
  
    /// Return the rotation matrix of a transformation matrix
    inline Matrix3 GetOrientation() const {
        Matrix3 result(false);
        return GetOrientation(result);
    }
    /// Return the rotation matrix of a transformation matrix
    inline Matrix3& GetOrientation(Matrix3 &result) const {
        result._[0*3+0] = _[0*4+0]; result._[0*3+1] = _[0*4+1]; result._[0*3+2] = _[0*4+2];
        result._[1*3+0] = _[1*4+0]; result._[1*3+1] = _[1*4+1]; result._[1*3+2] = _[1*4+2];
        result._[2*3+0] = _[2*4+0]; result._[2*3+1] = _[2*4+1]; result._[2*3+2] = _[2*4+2];
        return result;
    }
    /// Set the rotation matrix of a transformation matrix
    inline Matrix4& SetOrientation(const Matrix3 &orient) {
        _[0*4+0] = orient._[0*3+0]; _[0*4+1] = orient._[0*3+1]; _[0*4+2] = orient._[0*3+2];
        _[1*4+0] = orient._[1*3+0]; _[1*4+1] = orient._[1*3+1]; _[1*4+2] = orient._[1*3+2];
        _[2*4+0] = orient._[2*3+0]; _[2*4+1] = orient._[2*3+1]; _[2*4+2] = orient._[2*3+2];
        return *this;
    }

    /// Set to a transformation according to a rotation matrix and a translation vector
    inline Matrix4& SetTransformation(const Matrix3 &rotation, const Vector3 &translation){
        SetOrientation(rotation);
        SetTranslation(translation);
        _[3*4+0] = _[3*4+1] = _[3*4+2] = R_ZERO;
        _[3*4+3] = R_ONE;
        return *this;
    }
    /// Set to a the inverse transformation according to a rotation matrix and a translation vector
    inline Matrix4& SetInverseTransformation(const Matrix3 &rotation, const Vector3 &translation){
        _[0*4+0] = rotation._[0*3+0]; _[1*4+0] = rotation._[0*3+1]; _[2*4+0] = rotation._[0*3+2];
        _[0*4+1] = rotation._[1*3+0]; _[1*4+1] = rotation._[1*3+1]; _[2*4+1] = rotation._[1*3+2];
        _[0*4+2] = rotation._[2*3+0]; _[1*4+2] = rotation._[2*3+1]; _[2*4+2] = rotation._[2*3+2];
        Vector3 v(false);
        SetTranslation(rotation.TransposeMult(translation,v).SMinus());
        _[3*4+0] = _[3*4+1] = _[3*4+2] = R_ZERO;
        _[3*4+3] = R_ONE;
        return *this;
    }

    /// Return the inverse of the current transformation matrix 
    inline Matrix4& InverseTransformation() const{
        Matrix4 result;
        return InverseTransformation(result);   
    }
    /// Return the inverse of the current transformation matrix 
    inline Matrix4& InverseTransformation(Matrix4 &result) const {
        return result.SetInverseTransformation(GetOrientation(),GetTranslation());
    }
  
  
    /// Transform a vector
    inline Vector3 Transform(const Vector3 &vector) const{
        Vector3 result;
        return Transform(vector,result);
    }
    /// Transform a vector
    inline Vector3& Transform(const Vector3 &vector, Vector3 & result) const{
        result._[0] = _[0*4+0] * vector._[0] + _[0*4+1] * vector._[1] + _[0*4+2] * vector._[2] + _[0*4+3];
        result._[1] = _[1*4+0] * vector._[0] + _[1*4+1] * vector._[1] + _[1*4+2] * vector._[2] + _[1*4+3];
        result._[2] = _[2*4+0] * vector._[0] + _[2*4+1] * vector._[1] + _[2*4+2] * vector._[2] + _[2*4+3];
        return result;
    } 
/*
  /// Transform a matrix
  inline Matrix4& Transform(const Matrix4 &matrix, Matrix4 & result) const
  {
    result.Identity();
            
    for(unsigned int k = 0; k< 3; k++){          
      for (unsigned int j = 0; j < 3; j++){
        result._[j][k] = 0.0f;
        for (unsigned int i = 0; i < 3; i++)
          result._[j][k] += _[j][i] * matrix._[i][k];
      }
    }
    for (unsigned int j = 0; j < 3; j++){
      result._[j][3] = 0.0f;
      for (unsigned int i = 0; i < 3; i++)
        result._[j][3] += _[j][i] * matrix._[i][3];
      result._[j][3] += _[j][3];  
    }
        //
    return result;
  } 
  */   
};

typedef Matrix4 Mat4;
#ifdef USE_MATHLIB_NAMESPACE
}
#endif

#endif
