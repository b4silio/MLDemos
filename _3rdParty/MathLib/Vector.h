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

#ifndef MATHLIB_VECTOR_H
#define MATHLIB_VECTOR_H

#include "MathLibCommon.h"


#include "Macros.h"

#include <assert.h>
#include <math.h>
#include <iostream>
#include <vector>
using namespace std;

#ifndef NULL
#define NULL 0
#endif

#ifdef USE_MATHLIB_NAMESPACE
namespace MathLib {
#endif

#ifdef  USE_T_EXTENSIONS
template<unsigned int ROW> class TVector;
#endif

typedef vector<unsigned int> IndicesVector;

/**
 * \defgroup MathLib MathLib
 *
 * \brief A mathematical library for doing various things such as
 * matrix manipulation
 */


/**
 * \class Vector
 *
 * \ingroup MathLib
 *
 * \brief The basic vector class
 *
 * This vector class can be used for doing various vector manipulation.
 * This should be combined with the Matrix class for doing almost anything
 * you ever dreamt of.
 */

class Matrix;

class Vector
{
    friend class Matrix;
#ifdef  USE_T_EXTENSIONS
    template<unsigned int ROW> friend class TVector;
#endif

protected:
    static  REALTYPE        undef;      ///< The default value returned when a vector out-of-bound is reached

            unsigned int    row;        ///< The size of the vector
            REALTYPE        *_;         ///< The data array.

public:

    /// Create an empty vector
    inline Vector() {
        row = 0;
        _   = NULL;
    }

    /// The destructor
    inline virtual ~Vector(){
        Release();
    }

    /// The copy constructor
    inline Vector(const Vector &vector){
        row = 0;
        _   = NULL;
        Set(vector);
    }

    /**
     * \brief Create a sized vector
     * \param size  The size
     * \param clear Tell if the vector sould be set to 0
     */
    inline Vector(unsigned int size, bool clear = true){
        row = 0;
        _   = NULL;
        Resize(size,false);
        if(clear) Zero();
    }

    /**
     * \brief Create a vector by copying a memory area
     * \param array The array of REALTYPE to be copied
     * \param size  The size
     */
    inline Vector(const REALTYPE *array, unsigned int size){
        row = 0;
        _   = NULL;
        Set(array,size);
    }

#ifdef  USE_T_EXTENSIONS
    /// Copy Contructor of a template Vector (see TVector)
    template<unsigned int ROW> inline Vector(const TVector<ROW> &vector){
        row = 0;
        _   = NULL;
        Set(vector._,ROW);
    }
#endif

    /// An assignment function
    inline Vector& Set(const Vector &vector){
        Resize(vector.row,false);
        memcpy(_,vector._,row*sizeof(REALTYPE));
        return *this;
    }
    /// An assignment function
    inline Vector& Set(const REALTYPE * array, unsigned int size){
        Resize(size,false);
        if((row)&&(array))
            memcpy(_,array,size*sizeof(REALTYPE));
        return *this;
    }

    /// Set all values to 0
    inline Vector& Zero(){
        if(row)
            memset(_,0,row*sizeof(REALTYPE));
        return *this;
    }

    /// Set all values to 1
    inline Vector& One(){
        REALTYPE *src = _;
        unsigned int len = row;
        while(len--)
            *(src++) = R_ONE;
        return *this;
    }

    /// Set random values uniformly distributed between 0 and 1
    inline Vector& Random(){
        REALTYPE *src = _;
        unsigned int len = row;
        while(len--){
            *(src++) = RND(R_ONE);
        }
        return *this;
    }

    /// Get the size of the vector
    inline unsigned int Size() const{
        return row;
    }

    /// Get the data pointer
    inline REALTYPE *Array() const{
        return _;
    }

    /**
    * \brief Get the reference to a vector value
    * \param row   The row number (starting from 0)
    * \return The corresponding value
    */
    inline REALTYPE& operator[] (const unsigned int row){
        if(row<this->row)
            return _[row];
        return undef;
    }
    inline REALTYPE& operator() (const unsigned int row){
        if(row<this->row)
            return _[row];
        return undef;
    }
    inline REALTYPE& Ref(const unsigned int row){
        if(row<this->row)
            return _[row];
        return undef;
    }
    inline REALTYPE& RefNoCheck(const unsigned int row){
        return _[row];
    }
    /**
     * \brief Get a vector value
     * \param row   The row number (starting from 0)
     * \return The corresponding value
     */
    inline REALTYPE At(const unsigned int row) const {
        if(row<this->row)
            return _[row];
        return undef;
    }
    inline REALTYPE AtNoCheck(const unsigned int row) const {
        return _[row];
    }

    inline Vector& operator = (const Vector &vector){
        return Set(vector);
    }
    inline Vector& operator = (const REALTYPE val){
        REALTYPE *src = _;
        unsigned int len = row;
        while(len--)
            *(src++) = val;
        return *this;
    }

    /**
     * \brief The - operator
     * \return A new vector
     */
    inline Vector operator - () const{
        Vector result;
        REALTYPE *src = _;
        REALTYPE *dst = result._;
        unsigned int len = row;
        while(len--)
            *(dst++) = -(*(src++));
        return result;
    }
    inline Vector& SMinus()
    {
        REALTYPE *src = _;
        unsigned int len = row;
        while(len--){
            *(src) = -(*(src));
            src++;
        }
        return *this;
    }


    inline Vector& operator += (const Vector &vector){
        REALTYPE *src = _;
        REALTYPE *dst = vector._;
        unsigned int len = (row<=vector.row?row:vector.row);
        while(len--)
            *(src++) += *(dst++);
        return *this;
    }
    inline Vector& operator -= (const Vector &vector){
        REALTYPE *src = _;
        REALTYPE *dst = vector._;
        unsigned int len = (row<=vector.row?row:vector.row);
        while(len--)
            *(src++) -= *(dst++);
        return *this;
    }
    /// Element-wise multiplication
    inline Vector& operator ^= (const Vector &vector){
        REALTYPE *src = _;
        REALTYPE *dst = vector._;
        unsigned int len = (row<=vector.row?row:vector.row);
        while(len--)
            *(src++) *= *(dst++);
        return *this;
    }
    /// Element-wise division
    inline Vector& operator /= (const Vector &vector){
        REALTYPE *src = _;
        REALTYPE *dst = vector._;
        unsigned int len = (row<=vector.row?row:vector.row);
        while(len--)
            *(src++) /= *(dst++);
        return *this;
    }


    inline Vector& operator += (REALTYPE scalar){
        REALTYPE *src = _;
        unsigned int len = row;
        while(len--) *(src++) += scalar;
        return *this;
    }

    inline Vector& operator -= (REALTYPE scalar){
        REALTYPE *src = _;
        unsigned int len = row;
        while(len--) *(src++) -= scalar;
        return *this;
    }

    inline Vector& operator *= (REALTYPE scalar){
        REALTYPE *src = _;
        unsigned int len = row;
        while(len--) *(src++) *= scalar;
        return *this;
    }

    inline Vector& operator /= (REALTYPE scalar){
        scalar = R_ONE / scalar;
        REALTYPE *src = _;
        unsigned int len = row;
        while(len--) *(src++) *= scalar;
        return *this;
    }

    inline Vector operator + (const Vector &vector) const{
        Vector result;
        return Add(vector,result);
    }
    inline Vector operator - (const Vector &vector) const{
        Vector result;
        return Sub(vector,result);
    }
    /// Element-wise multiplication
    inline Vector operator ^ (const Vector &vector) const{
        Vector result;
        return PMult(vector,result);
    }
    /// Element-wise division
    inline Vector operator / (const Vector &vector) const{
        Vector result;
        return PDiv(vector,result);
    }

    inline REALTYPE operator * (const Vector &vector) const{
        return Dot(vector);
    }

    inline Vector operator + (REALTYPE scalar) const{
        Vector result;
        return Add(scalar,result);
    }
    inline Vector operator - (REALTYPE scalar) const{
        Vector result;
        return Sub(scalar,result);
    }
    inline Vector operator * (REALTYPE scalar) const{
        Vector result;
        return Mult(scalar,result);
    }
    inline Vector operator / (REALTYPE scalar) const{
        Vector result;
        return Div(scalar,result);
    }

    inline bool operator == (const Vector& vector) const{
        if(row!=vector.row) return false;
        REALTYPE *src = _;
        REALTYPE *dst = vector._;
        unsigned int len = row;
        while(len--)
            if(*(src++) != *(dst++)) return false;
        return true;
    }
    inline bool operator != (const Vector& vector) const{
        return !(*this ==  vector);
    }

    inline Vector& ScaleAddTo(REALTYPE scale, Vector &result) const
    {
        REALTYPE *src = _;
        REALTYPE *dst = result._;
        unsigned int len = (row<=result.row?row:result.row);
        while(len--)
            *(dst++) += *(src++) * scale;
        return result;
    }

    /**
     * \brief Sum two vector in a faster way than using the + operator
     * \param vector The second vector to be summed up
     * \param result The result
     * \return The result vector
     */
    inline Vector& Add(const Vector &vector, Vector& result) const{
        result.Resize(row,false);
        REALTYPE *src0 = _;
        REALTYPE *src  = vector._;
        REALTYPE *dst  = result._;
        if(row<=vector.row){
            unsigned int len = row;
            while(len--)
                *(dst++) = *(src0++) + *(src++);
        }else{
            unsigned int len = vector.row;
            while(len--)
                *(dst++) = *(src0++) + *(src++);
            len = row - vector.row;
            while(len--)
                *(dst++) = *(src0++);
        }
        return result;
    }
    /**
     * \brief Substract two vector in a faster way than using the + operator
     * \param vector The substracting vector
     * \param result The result
     * \return The result vector
     */
    inline Vector& Sub(const Vector &vector, Vector& result) const{
        result.Resize(row,false);
        REALTYPE *src0 = _;
        REALTYPE *src  = vector._;
        REALTYPE *dst  = result._;
        if(row<=vector.row){
            unsigned int len = row;
            while(len--)
                *(dst++) = *(src0++) - *(src++);
        }else{
            unsigned int len = vector.row;
            while(len--)
                *(dst++) = *(src0++) - *(src++);
            len = row - vector.row;
            while(len--)
                *(dst++) = *(src0++);
        }
        return result;
    }
    /// Element-wise multiplication
    inline Vector& PMult(const Vector &vector, Vector& result) const{
        result.Resize(row,false);
        REALTYPE *src0 = _;
        REALTYPE *src  = vector._;
        REALTYPE *dst  = result._;
        if(row<=vector.row){
            unsigned int len = row;
            while(len--)
                *(dst++) = *(src0++) * *(src++);
        }else{
            unsigned int len = vector.row;
            while(len--)
                *(dst++) = *(src0++) * *(src++);
            len = row - vector.row;
            while(len--)
                *(dst++) = *(src0++);
        }
        return result;

    }
    /// Element-wise division
    inline Vector& PDiv(const Vector &vector, Vector& result) const{
        result.Resize(row,false);
        REALTYPE *src0 = _;
        REALTYPE *src  = vector._;
        REALTYPE *dst  = result._;
        if(row<=vector.row){
            unsigned int len = row;
            while(len--)
                *(dst++) = *(src0++) / *(src++);
        }else{
            unsigned int len = vector.row;
            while(len--)
                *(dst++) = *(src0++) / *(src++);
            len = row - vector.row;
            while(len--)
                *(dst++) = *(src0++);
        }
        return result;

    }

    /// Scalar summation
    inline Vector& Add(REALTYPE scalar, Vector& result) const{
        result.Resize(row,false);
        REALTYPE *src = _;
        REALTYPE *dst = result._;
        unsigned int len = row;
        while(len--)
            *(dst++) = *(src++) + scalar;
        return result;
    }
    /// Scalar substraction
    inline Vector& Sub(REALTYPE scalar, Vector& result) const{
        result.Resize(row,false);
        REALTYPE *src = _;
        REALTYPE *dst = result._;
        unsigned int len = row;
        while(len--)
            *(dst++) = *(src++) - scalar;
        return result;
    }
    /// Scalar multiplication
    inline Vector& Mult(REALTYPE scalar, Vector& result) const{
        result.Resize(row,false);
        REALTYPE *src = _;
        REALTYPE *dst = result._;
        unsigned int len = row;
        while(len--)
            *(dst++) = *(src++) * scalar;
        return result;
    }
    /// Scalar division
    inline Vector& Div(REALTYPE scalar, Vector& result) const{
        scalar = R_ONE/scalar;
        result.Resize(row,false);
        REALTYPE *src = _;
        REALTYPE *dst = result._;
        unsigned int len = row;
        while(len--)
            *(dst++) = *(src++) * scalar;
        return result;
    }


  Matrix& MultTranspose(const Vector & vec, Matrix& result);
  Matrix MultTranspose(const Vector & vec);

    /// Sum up all elements of the vector
    inline REALTYPE Sum() const {
        REALTYPE result = R_ZERO;
        REALTYPE *src = _;
        unsigned int len = row;
        while(len--)
            result += *(src++);
        return result;
    }

    /// The norm of the vector
    inline REALTYPE Norm() const {
        #ifdef MATHLIB_USE_DOUBLE_AS_REAL
            return sqrt(Norm2());
        #else
            return sqrtf(Norm2());
        #endif
    }

    /// The squared norm of the vector
    inline REALTYPE Norm2() const {
        REALTYPE result = R_ZERO;
        REALTYPE *src = _;
        unsigned int len = row;
        while(len--){
            result += *(src) * (*(src));
            src++;
        }
        return result;
    }

    /// Normalize the vector to 1
    inline void Normalize(){
        REALTYPE norm = Norm();
        if(norm>EPSILON){
            REALTYPE scalar = R_ONE / Norm();
            (*this) *= scalar;
        }else{
            Zero();
        }
    }

    /// The distance between two vectors
    inline REALTYPE Distance(const Vector &vector) const{
        return (*this-vector).Norm();
    }

    /// The squared distance between two vectors
    inline REALTYPE Distance2(const Vector &vector) const{
        return (*this-vector).Norm2();
    }

    /// The dot product with another vector
    inline REALTYPE Dot(const Vector &vector) const{
        unsigned int k = (row<=vector.row?row:vector.row);
        REALTYPE result = R_ZERO;
        REALTYPE *src = _;
        REALTYPE *dst = vector._;
        while(k--)
            result += (*(src++)) * (*(dst++));
        return result;
    }

    /// The maximum value of the vector
    inline REALTYPE Max(unsigned int *index=NULL){
        if(row==0){
            if(index) *index = 0;
            return R_ZERO;
        }
        REALTYPE *src =_;
        REALTYPE res  =*(src++);
        if(index){
            *index = 0;
            for(unsigned int i=1;i<row;i++){
                if(*src>res){
                    res     = *src;
                    *index  = i;
                }
                src++;
            }
        }else{
            for(unsigned int i=1;i<row;i++){
                if(*src>res)
                    res = *src;
                src++;
            }
        }
        return res;
    }

    /// The minimum value of the vector
    inline REALTYPE Min(unsigned int *index=NULL){
        if(row==0){
            if(index) *index = 0;
            return R_ZERO;
        }
        REALTYPE *src =_;
        REALTYPE res  =*(src++);
        if(index){
            *index = 0;
            for(unsigned int i=1;i<row;i++){
                if(*src<res){
                    res     = *src;
                    *index  = i;
                }
                src++;
            }
        }else{
            for(unsigned int i=1;i<row;i++){
                if(*src<res)
                    res = *src;
                src++;
            }
        }
        return res;
    }

    /// The index of the maximum value of the vector
    inline int MaxId(){
        unsigned int res;
        Max(&res);
        return res;
    }

    /// The index of the minimum value of the vector
    inline int MinId(){
        unsigned int res;
        Min(&res);
        return res;
    }

  /// The maximum value of two vector
  inline Vector& Max(const Vector& v0,const Vector& v1){
    const unsigned int kmin = (v0.row<v1.row?v0.row:v1.row);
    const unsigned int kmax = (v0.row<v1.row?v1.row:v0.row);
    Resize(kmax,false);
    for(unsigned int i=0;i<kmin;i++){
      _[i] = (v0._[i]>v1._[i]?v0._[i]:v1._[i]);
    }
    if(kmin<kmax){
      if(v0.row<v1.row){
        for(unsigned int i=kmin;i<kmax;i++){
          _[i] = v1._[i];
        }
      }else{
        for(unsigned int i=kmin;i<kmax;i++){
          _[i] = v0._[i];
        }
      }
    }
    return *this;
  }

  /// The minimum value of two vector
  inline Vector& Min(const Vector& v0,const Vector& v1){
    const unsigned int kmin = (v0.row<v1.row?v0.row:v1.row);
    const unsigned int kmax = (v0.row<v1.row?v1.row:v0.row);
    Resize(kmax,false);
    for(unsigned int i=0;i<kmin;i++){
      _[i] = (v0._[i]<v1._[i]?v0._[i]:v1._[i]);
    }
    if(kmin<kmax){
      if(v0.row<v1.row){
        for(unsigned int i=kmin;i<kmax;i++){
          _[i] = v1._[i];
        }
      }else{
        for(unsigned int i=kmin;i<kmax;i++){
          _[i] = v0._[i];
        }
      }
    }
    return *this;
  }

    /// Return the absolute value of the vector
    inline Vector Abs(){
        Vector result;
        return Abs(result);
    }
    /// Set the result to the absolute value of the vector
    inline Vector& Abs(Vector &result) const{
        result.Resize(row,false);
        REALTYPE *src = _;
        REALTYPE *dst = result._;
        unsigned int len = row;
        while(len--)
            *(dst++) = fabs(*(src++));
        return result;
    }
    /// Absolute value of the vector
    inline Vector& SAbs() {
        REALTYPE *src = _;
        unsigned int len = row;
        while(len--){
            *src = fabs(*src);
            src++;
        }
        return *this;
    }

  inline Vector& Sort(IndicesVector * indices=NULL){
    if(indices){
        indices->resize(row);
        for(unsigned int i=0;i<row;i++) indices->at(i)=i;
    }
    REALTYPE cmax;
    unsigned int maxId;
    for(unsigned int i=0;i<row-1;i++){
      cmax  = _[i];
      maxId = i;
      for(unsigned int j=i+1;j<row;j++){
        if(cmax<_[j]){
          cmax = _[j];
          maxId = j;
        }
      }
      if(maxId!=i){
        REALTYPE tmp    = _[i];
        _[i]            = _[maxId];
        _[maxId]        = tmp;
        if(indices){
            unsigned int idx   = indices->at(i);
            indices->at(i)     = indices->at(maxId);
            indices->at(maxId) = idx;
        }
      }
    }
    return *this;
  }

  inline Vector& AbsSort(IndicesVector * indices=NULL){
    if(indices){
        indices->resize(row);
        for(unsigned int i=0;i<row;i++) indices->at(i)=i;
    }
    REALTYPE cmax;
    unsigned int maxId;
    for(unsigned int i=0;i<row-1;i++){
      cmax  = fabs(_[i]);
      maxId = i;
      for(unsigned int j=i+1;j<row;j++){
        if(cmax<fabs(_[j])){
          cmax = fabs(_[j]);
          maxId = j;
        }
      }
      if(maxId!=i){
        REALTYPE tmp    = _[i];
        _[i]            = _[maxId];
        _[maxId]        = tmp;
        if(indices){
            unsigned int idx   = indices->at(i);
            indices->at(i)     = indices->at(maxId);
            indices->at(maxId) = idx;
        }
      }
    }
    return *this;
  }

  /**
   * \brief Set the value of another vector into the current vector
   * \param startPos The index starting from which the data of the passed vector will be copied
   * \param vector   The input vector
   */
  inline Vector& SetSubVector(unsigned int startPos, const Vector &vector)
  {
    if(startPos<row){
      const unsigned int k = (row-startPos<=vector.row?row-startPos:vector.row);
      for (unsigned int i = 0; i < k; i++){
        _[startPos+i] = vector._[i];
      }
    }
    return *this;
  }

  /**
   * \brief Get a vector containing some of the vector values
   * \param startPos The starting index
   * \param len      The length of data to be copied
   */
  inline Vector GetSubVector(unsigned int startPos, unsigned int len) const
  {
    Vector result(len,false);
    return GetSubVector(startPos,len,result);
  }


  /**
   * \brief Get a vector containing some of the vector values
   * \param startPos The starting index
   * \param len      The length of data to be copied
   * \param result   The output vector
   */
  inline Vector& GetSubVector(unsigned int startPos, unsigned int len, Vector &result) const
  {
    result.Resize(len,false);
    if(startPos<row){
      const unsigned int k = (row-startPos<=len?row-startPos:len);
      for (unsigned int i = 0; i < k; i++){
        result[i] = _[startPos+i];
      }
      for (unsigned int i = k; i < len; i++){
        result[i] = R_ZERO;
      }

    }else{
      result.Zero();
    }
    return result;
  }

  inline Vector& GetSubVector(const Vector& ids, Vector &result) const
  {
    int s = ids.Size();
    IndicesVector id;
    for(int i=0;i<s;i++) id.push_back(int(ROUND(ids.At(i))));
    return GetSubVector(id,result);
  }

  /**
   * \brief Get a vector containing the vector values a given indices
   * \param ids      A vecotr containing the indices
   * \param result   The output vector
   */
  inline Vector& GetSubVector(const IndicesVector& ids, Vector &result) const
  {
    const unsigned int k=ids.size();
    result.Resize(k);
    for(unsigned int i=0;i<k;i++){
      const unsigned int g = ids[i];
      if(g<row){
        result._[i] = _[g];
      }else{
        result._[i] = R_ZERO;
      }
    }
    return result;
  }

  /**
   * \brief Set at given indices a given vector values
   * \param ids      The indices
   * \param result   The vector
   */
  inline Vector& SetSubVector(const IndicesVector& ids, const Vector &source)
  {
    const unsigned int j=ids.size();
    const unsigned int k= (source.row<j?source.row:j);
    for(unsigned int i=0;i<k;i++){
      const unsigned int g = ids[i];
      if(g<row){
        _[g] = source._[i];
      }
    }
    return *this;
  }

  inline Vector& InsertSubVector(unsigned int start,
                                 const Vector& vector,
                                 unsigned int vectorStart, unsigned int vectorLength){

    if(vectorStart >= vector.row) return *this;
    if(start       >= row)        return *this;

    if(vectorStart+vectorLength > vector.row) vectorLength = vector.row-vectorStart;

    if(start+vectorLength > row) vectorLength = row-start;

    unsigned int rowOffset       = start;
    unsigned int vectorRowOffset = vectorStart;
    for(unsigned int j=0;j<vectorLength;j++){
      _[rowOffset] = vector._[vectorRowOffset];
      rowOffset++;
      vectorRowOffset++;
    }
    return *this;
  }

    /// Shift the value to the right (rightmost value is set to the left)
    inline Vector& ShiftRight(){
        if(row>1){
            REALTYPE *src = _ + row-1;
            REALTYPE zero = *src;
            unsigned int len = row-1;
            while(len--){
                *src = *(src-1);
                src--;
            }
            *src = zero;
        }
        return *this;
    }

    /// Shift the value toi the left (leftmost value is set to the right)
    inline Vector& ShiftLeft(){
        if(row>1){
            REALTYPE *src = _;
            REALTYPE zero = *src;
            unsigned int len = row-1;
            while(len--){
                *src = *(src+1);
                src++;
            }
            *src = zero;
        }
        return *this;
    }

    inline Vector& Trunc(const Vector& min, const Vector& max){
        unsigned int k = MIN(min.row,max.row);
        unsigned int len = MIN(row,k);
        REALTYPE *dst = _;
        REALTYPE *v1  = min._;
        REALTYPE *v2  = max._;
        while(len--){
            *dst = TRUNC((*dst),(*(v1)),(*(v2)));
            v1++;
            v2++;
            dst++;
        }
        if(row>k){
            unsigned int len = row-k;
            while(len--)
                *(dst++) = R_ZERO;
        }
        return *this;
    }

    inline Vector& Trunc(const REALTYPE min, const REALTYPE max){
        REALTYPE *dst = _;
        unsigned int len = row;
        while(len--){
            *dst = TRUNC((*dst),min,max);
            dst++;
        }
        return *this;
    }

    /// Print the vector to stdout
    void Print() const;
    void Print(string name) const;
    /*{
        std::cout << "Vector " <<row<<std::endl;;
        for (unsigned int i = 0; i < row; i++)
            std::cout << _[i] <<" ";
        std::cout << "\n";
    }*/

    /// Print the vector to stdout
    friend std::ostream & operator<<(std::ostream& out, const Vector & a){
      PRINT_BEGIN(out);

      for (unsigned int i = 0; i < a.Size(); i++){
        out.width(PRINT_WIDTH);
        out<< a.AtNoCheck(i)<<" " ;

      }

      PRINT_END(out);
      return out;
    }


  int PrintToString(char * str, int maxsize=0){
    int cIndex = 0;
    str[0] = 0;
    if(maxsize<=0){
      for (unsigned int i = 0; i < row; i++){
        cIndex += sprintf(str+cIndex,"%1.12f ",_[i]);
      }
    }else{
      for (unsigned int i = 0; i < row; i++){
        int nb = snprintf(str+cIndex,maxsize-cIndex,"%1.12f ",_[i]);
        if(nb>=maxsize-cIndex) break;
        cIndex += nb;
      }
    }
    return cIndex;
  }


protected:
    inline void Release(){
        if(_!=NULL) delete [] _;
        row = 0;
        _   = NULL;
    }
public:
    /**
     * \brief Resize the vector
     * \param size     The new size
     * \param copy     Should keep the original data or just resize.
     */
    inline virtual Vector& Resize(unsigned int size, bool copy = true){
        if(row!=size){
            if(size){
                REALTYPE *arr = new REALTYPE[size];
                if(copy){
                    unsigned int len = (row<size?row:size);
                    memcpy(arr,_,len*sizeof(REALTYPE));
                    if(len<size){
                        memset(arr+len,0,(size-len)*sizeof(REALTYPE));
                    }
                }
                if(_!=NULL) delete [] _;
                _   = arr;
                row = size;
            }else{
                Release();
            }
        }
        return *this;
    }
};




class SharedVector : public Vector
{
protected:
    unsigned int maxMemorySize;

public:
    inline SharedVector():Vector(){}
    inline SharedVector(REALTYPE *array, unsigned int rowSize):Vector(){
        SetSharedPtr(array,rowSize);
    }
    inline SharedVector(const Vector & vector):Vector(){
        SetSharedPtr(vector.Array(),vector.Size());
    }
#ifdef  USE_T_EXTENSIONS
    /// Copy Contructor of a template Vector (see TVector)
    template<unsigned int ROW> inline SharedVector(const TVector<ROW> &vector):Vector(){
        SetSharedPtr(vector.Array(),ROW);
    }
#endif
    inline virtual ~SharedVector(){
        Release();
    }

    inline SharedVector& SetSharedPtr(REALTYPE *array, unsigned int size){
        row     = size;
        maxMemorySize = size;
        _       = array;
        return *this;
    }
protected:
    inline virtual void Release(){
        _ = NULL;
    }
public:
    inline virtual SharedVector& Resize(unsigned int rowSize, bool copy = true){
        assert(rowSize<=maxMemorySize);
        row     = rowSize;
        return *this;
    }
};


#ifdef USE_MATHLIB_NAMESPACE
}
#endif
#endif
