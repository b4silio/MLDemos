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

#ifndef MATRIX3_H
#define MATRIX3_H

#include "MathLibCommon.h"

#include <math.h>
#include "TMatrix.h"
#include "Vector3.h"

#ifdef USE_MATHLIB_NAMESPACE
namespace MathLib {
#endif

/**
 * \class Matrix3
 * 
 * \ingroup MathLib
 * 
 * \brief An implementation of the template TMatrix class
 * 
 * This template square matrix class can be used for doing various matrix manipulation.
 * This should be combined with the TVector class for doing almost anything
 * you ever dreamt of. 
 */

class Matrix3 : public TMatrix<3>
{
  //friend class Referential;
  //friend class Matrix4;
public:
    /// A constant zero matrix
    static const Matrix3 ZERO;
    /// A constant identity matrix
    static const Matrix3 IDENTITY;

    /// Empty constructor
    inline Matrix3(bool clear=true):TMatrix<3>(clear){};
    /// Copy constructor
    inline Matrix3(const Matrix3 &matrix):TMatrix<3>(matrix){};
    /// Copy constructor
    inline Matrix3(const TMatrix<3> &matrix):TMatrix<3>(matrix){};
    /*
    /// Copy constructor
    //inline Matrix3(const Matrix & matrix):TMatrix<3>(matrix){};
    */
    /// Constructor with data pointer
    inline Matrix3(const REALTYPE *array):TMatrix<3>(array){};
    /// Constructor with values
    inline Matrix3(REALTYPE _00, REALTYPE _01, REALTYPE _02,
                   REALTYPE _10, REALTYPE _11, REALTYPE _12,
                   REALTYPE _20, REALTYPE _21, REALTYPE _22):TMatrix<3>(false){
        Set(_00,_01,_02,
            _10,_11,_12,
            _20,_21,_22);
    }

    inline virtual ~Matrix3(){}

    inline Matrix3& Set(REALTYPE _00, REALTYPE _01, REALTYPE _02,
                        REALTYPE _10, REALTYPE _11, REALTYPE _12,
                        REALTYPE _20, REALTYPE _21, REALTYPE _22){
        REALTYPE *dst = _;
        *(dst++) = _00; *(dst++) = _01; *(dst++) = _02;
        *(dst++) = _10; *(dst++) = _11; *(dst++) = _12;
        *(dst++) = _20; *(dst++) = _21; *(dst++) = _22;
        return *this;
    }
  
    inline Matrix3& Set(const Matrix3& matrix){
        TMatrix<3>::Set(matrix);
        return *this;
    }
    inline Matrix3& Set(const Matrix& matrix){
        TMatrix<3>::Set(matrix);
        return *this;
    }
    inline Matrix3& Set(const REALTYPE *array){
        TMatrix<3>::Set(array);
        return *this;
    }

    /*
    inline Matrix3& Set(const Matrix& matrix){
        TMatrix<3>::Set(matrix);
        return *this;
    }
    */

    /// Assign each column
    inline Matrix3& SetColumns(const Vector3 &vector0, const Vector3 &vector1, const Vector3 &vector2){
        SetColumn(vector0,0);
        SetColumn(vector1,1);
        SetColumn(vector2,2);
        return *this;
    }

    /// Assign each row
    inline Matrix3& SetRows(const Vector3 &vector0, const Vector3 &vector1, const Vector3 &vector2){
        SetRow(vector0,0);
        SetRow(vector1,1);
        SetRow(vector2,2);
        return *this;
    }

    /// Create a skew symmetric matix from a vector 
    inline Matrix3& SkewSymmetric(Vector3 vector){
        REALTYPE *dst=_;
        *(dst++) = R_ZERO;      *(dst++) =-vector._[2]; *(dst++) = vector._[1];
        *(dst++) = vector._[2]; *(dst++) = R_ZERO;      *(dst++) =-vector._[0];
        *(dst++) =-vector._[1]; *(dst++) = vector._[0]; *(dst++) = R_ZERO;
        return *this;
    }

    inline static Matrix3 SRotationX(REALTYPE angleX){
        Matrix3 result; return result.RotationX(angleX);
    }
    inline static Matrix3 SRotationY(REALTYPE angleY){
        Matrix3 result; return result.RotationY(angleY);
    }
    inline static Matrix3 SRotationZ(REALTYPE angleZ){
        Matrix3 result; return result.RotationZ(angleZ);
    }
    inline static Matrix3 SRotationYXZ(REALTYPE angleX, REALTYPE angleY, REALTYPE angleZ){
        Matrix3 result; return result.RotationYXZ(angleX,angleY,angleZ);
    }
    inline static Matrix3 SRotationV(REALTYPE angle, const Vector3 &axis){
        Matrix3 result; return result.RotationV(angle,axis);
    }
    inline static Matrix3 SRotationV(const Vector3 &axis){
        Matrix3 result; return result.RotationV(axis);
    }
  
    /// Create a rotation matrix around axis X with a given angle 
    Matrix3& RotationX(REALTYPE angleX);
    /// Create a rotation matrix around axis Y with a given angle 
    Matrix3& RotationY(REALTYPE angleY);
    /// Create a rotation matrix around axis Z with a given angle 
    Matrix3& RotationZ(REALTYPE angleZ);
    /// Create a rotation matrix around axes X,Y,Z with given angles 
    Matrix3& RotationYXZ(REALTYPE angleX, REALTYPE angleY, REALTYPE angleZ);

    /// Create a rotation matrix around an arbitrary axis with a given angle 
    Matrix3& RotationV(REALTYPE angle, const Vector3 &axis);
  
    /// Create a rotation matrix around an arbitrary axis with a given angle 
    inline Matrix3& RotationV(const Vector3 &axis){
        return RotationV(axis.Norm(),axis);
    }

    /// Get the rotation axis of a rotation matrix (arbitrary norm)
    inline Vector3  GetRotationAxis(){
        Vector3 result;
        return GetRotationAxis(result);
    }
    /// Get the rotation axis of a rotation matrix (arbitrary norm)
    inline Vector3& GetRotationAxis(Vector3 &result){
        result._[0] =  _[2*3+1]-_[1*3+2];
        result._[1] =  _[0*3+2]-_[2*3+0];
        result._[2] =  _[1*3+0]-_[0*3+1];
        return result;
    }
  
    /// Get the rotation axis of a rotation matrix (the norm equals the rotation angle)
    inline Vector3  GetExactRotationAxis() const{
        Vector3 result;
        return GetExactRotationAxis(result);
    }

    /// Get the rotation axis of a rotation matrix (the norm equals the rotation angle)
    inline Vector3& GetExactRotationAxis(Vector3 &result) const{
        GetNormRotationAxis(result);
        result *= GetRotationAngle();
        return result;
    }

    /// Get the rotation axis of a rotation matrix (the norm equals 1)
    inline Vector3& GetNormRotationAxis(Vector3 &result) const{
        result._[0] =  _[2*3+1]-_[1*3+2];
        result._[1] =  _[0*3+2]-_[2*3+0];
        result._[2] =  _[1*3+0]-_[0*3+1];
        REALTYPE norm = result.Norm();
        if(norm>EPSILON)
            result*=(R_ONE/norm);
        else
            result.Zero();
        return result;
    }

    /// Get the rotation angle of a rotation matrix
    inline REALTYPE GetRotationAngle() const{
        REALTYPE res = (_[0*3+0]+_[1*3+1]+_[2*3+2]-1.0f)/2.0;
        if(res>R_ONE)           return R_ZERO;
        else if (res<-R_ONE)    return PI;
        else return acos(res);
    }

    /// Return a matrix where the amount of rotation has been scaled
    inline Matrix3& RotationScale(REALTYPE scale){
        Matrix3 result;
        return RotationScale(scale,result);      
    }
  
    /// Return a matrix where the amount of rotation has been scaled
    inline Matrix3& RotationScale(REALTYPE scale, Matrix3 & result){
        Vector3 v;
        GetNormRotationAxis(v);
        result.RotationV(GetRotationAngle()*scale,v);
        return result;
    }

    /// Return a rotation matrix in between src and dst. Scale is in [0,1]
    inline Matrix3& RotationScale(const Matrix3& src, const Matrix3& dst, REALTYPE scale){
    
        if(scale<R_ZERO) scale = R_ZERO;
        if(scale>R_ONE)  scale = R_ONE;
    
        Matrix3 tmpM;    
        src.Transpose(tmpM);
        tmpM.Mult(dst,*this);
    
        Vector3 tmpV;
        GetExactRotationAxis(tmpV);    
        tmpV *= scale;
    
        tmpM.RotationV(tmpV);
    
        src.Mult(tmpM,*this);
        return *this;
    }
  
  /// Normalize the matrix (Gram-Schmidt) with the given primary axis (x=0,y=1,z=2)
  Matrix3& Normalize(int mainAxe = 2)
  {
    if((mainAxe<0)||(mainAxe>2)) mainAxe=2;
    
    Vector3 v0(_[0*3+0],_[1*3+0],_[2*3+0]);
    Vector3 v1(_[0*3+1],_[1*3+1],_[2*3+1]);
    Vector3 v2(_[0*3+2],_[1*3+2],_[2*3+2]);
    switch(mainAxe){
    case 0:
      
      v0.Normalize();
      v1-=v0 * v0.Dot(v1);
      v1.Normalize();
      v2 = v0.Cross(v1);
  
      break;
    case 1:
      v1.Normalize();
      v2-=v1 * v1.Dot(v2);
      v2.Normalize();
      v0 = v1.Cross(v2);
      break;
    case 2:
      v2.Normalize();
      v1-=v2 * v2.Dot(v1);
      v1.Normalize();
      v0 = v1.Cross(v2);
      break;
    }
    SetColumn(v0,0);  
    SetColumn(v1,1);  
    SetColumn(v2,2);
    return *this;
  }  
  
  /// Normalize the matrix (Gram-Schmidt) according to the given order of axis (x=0,y=1,z=2)
  Matrix3& Normalize(int firAxe, int secAxe, int trdAxe)
  {
    
    Vector3 v0(_[0*3+0],_[1*3+0],_[2*3+0]);
    Vector3 v1(_[0*3+1],_[1*3+1],_[2*3+1]);
    Vector3 v2(_[0*3+2],_[1*3+2],_[2*3+2]);

    Vector3 w0;
    Vector3 w1;
    Vector3 w2;

    switch(firAxe){case 0: w0 = v0; break; case 1: w0 = v1; break; case 2: w0 = v2; break;}
    switch(secAxe){case 0: w1 = v0; break; case 1: w1 = v1; break; case 2: w1 = v2; break;}
    switch(trdAxe){case 0: w2 = v0; break; case 1: w2 = v1; break; case 2: w2 = v2; break;}

    w0.Normalize();
    w1-=w0 * w0.Dot(w1);
    w1.Normalize();
    w2 = w0.Cross(w1);
  
    switch(firAxe){case 0: v0 = w0; break; case 1: v0 = w1; break; case 2: v0 = w2; break;}
    switch(secAxe){case 0: v1 = w0; break; case 1: v1 = w1; break; case 2: v1 = w2; break;}
    switch(trdAxe){case 0: v2 = w0; break; case 1: v2 = w1; break; case 2: v2 = w2; break;}
    
    SetColumn(v0,0);  
    SetColumn(v1,1);  
    SetColumn(v2,2);
    return *this;
  }
  
    inline Matrix3& SetCross(const Vector3& vector){
        return SkewSymmetric(vector);
    }
  
    Matrix3& EulerRotation(int axis0, int axis1, int axis2, const Vector3& angles);  

    Vector3& GivensRotationPlane(REALTYPE a, REALTYPE b, Vector3 & result, int path=0);
    Vector3& GetEulerAnglesGeneric(int i, int neg, int alt, int rev, Vector3& result, int path=0);

    Vector3& GetEulerAnglesXZX(bool rev, Vector3& result, int path=0){return GetEulerAnglesGeneric(0,0,0,(rev?1:0),result,path);} 
    Vector3& GetEulerAnglesYXY(bool rev, Vector3& result, int path=0){return GetEulerAnglesGeneric(1,0,0,(rev?1:0),result,path);}
    Vector3& GetEulerAnglesZYZ(bool rev, Vector3& result, int path=0){return GetEulerAnglesGeneric(2,0,0,(rev?1:0),result,path);} 

    Vector3& GetEulerAnglesXZY(bool rev, Vector3& result, int path=0){
        if(rev){return GetEulerAnglesGeneric(1,1,1,(rev?1:0),result,path);}
        else{   return GetEulerAnglesGeneric(0,0,1,(rev?1:0),result,path);}
    } 
    Vector3& GetEulerAnglesYXZ(bool rev, Vector3& result, int path=0){
        if(rev){return GetEulerAnglesGeneric(2,1,1,(rev?1:0),result,path);}
        else{   return GetEulerAnglesGeneric(1,0,1,(rev?1:0),result,path);}
    } 
    Vector3& GetEulerAnglesZYX(bool rev, Vector3& result, int path=0){ 
        if(rev){return GetEulerAnglesGeneric(0,1,1,(rev?1:0),result,path);}
        else{   return GetEulerAnglesGeneric(2,0,1,(rev?1:0),result,path);}
    }  
    Vector3& GetEulerAnglesXYX(bool rev, Vector3& result, int path=0){return GetEulerAnglesGeneric(0,1,0,(rev?1:0),result,path);} 
    Vector3& GetEulerAnglesYZY(bool rev, Vector3& result, int path=0){return GetEulerAnglesGeneric(1,1,0,(rev?1:0),result,path);} 
    Vector3& GetEulerAnglesZXZ(bool rev, Vector3& result, int path=0){return GetEulerAnglesGeneric(2,1,0,(rev?1:0),result,path);} 
 
    Vector3& GetEulerAnglesXYZ(bool rev, Vector3& result, int path=0){
        if(rev){return GetEulerAnglesGeneric(2,0,1,(rev?1:0),result,path);}
        else{   return GetEulerAnglesGeneric(0,1,1,(rev?1:0),result,path);}
    } 
    Vector3& GetEulerAnglesYZX(bool rev, Vector3& result, int path=0){
        if(rev){return GetEulerAnglesGeneric(0,0,1,(rev?1:0),result,path);}
        else{   return GetEulerAnglesGeneric(1,1,1,(rev?1:0),result,path);}
    } 
    Vector3& GetEulerAnglesZXY(bool rev, Vector3& result, int path=0){
        if(rev){return GetEulerAnglesGeneric(1,0,1,(rev?1:0),result,path);}
        else{   return GetEulerAnglesGeneric(2,1,1,(rev?1:0),result,path);}    
    } 

    Vector3& GetEulerAngles(int r1, int r2, int r3, int rev, Vector3& result, int path=0){
        switch(r1){
        case 0:
            switch(r2){
            case 1:
                switch(r3){
                case 0: return GetEulerAnglesXYX(rev, result,path); break;
                case 2: return GetEulerAnglesXYZ(rev, result,path); break;
                }
                break;
            case 2:
                switch(r3){
                case 0: return GetEulerAnglesXZX(rev, result,path); break;
                case 1: return GetEulerAnglesXZY(rev, result,path); break;
                }
                break;
            }
            break;
        case 1:
            switch(r2){
            case 0:
                switch(r3){
                case 1: return GetEulerAnglesYXY(rev, result,path); break;
                case 2: return GetEulerAnglesYXZ(rev, result,path); break;
                }
                break;
            case 2:
                switch(r3){
                case 0: return GetEulerAnglesYZX(rev, result,path); break;
                case 1: return GetEulerAnglesYZY(rev, result,path); break;
                }
                break;
            }
            break;
        case 2:
            switch(r2){
            case 0:
                switch(r3){
                    case 1: return GetEulerAnglesZXY(rev, result,path); break;
                    case 2: return GetEulerAnglesZXZ(rev, result,path); break;
                    }
                    break;
            case 1:
                switch(r3){
                case 0: return GetEulerAnglesZYX(rev, result,path); break;
                case 2: return GetEulerAnglesZYZ(rev, result,path); break;
                }
                break;
            }
            break;
        }
        cout << "GET EULER ANGLES ERROR (bad indices)"<<endl;
        return result;
    } 

};

typedef Matrix3 Mat3;

#ifdef USE_MATHLIB_NAMESPACE
}
#endif
#endif
