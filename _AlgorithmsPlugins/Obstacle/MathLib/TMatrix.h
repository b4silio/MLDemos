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

#ifndef __TMATRIX_H__
#define __TMATRIX_H__

#include "MathLibCommon.h"

#define  USE_T_EXTENSIONS


#include "TVector.h"
#include "Matrix.h"

#ifdef USE_MATHLIB_NAMESPACE
namespace MathLib {
#endif

/**
 * \class TMatrix
 *
 * \ingroup MathLib
 *
 * \brief The basic template square matrix class. See the Matrix class for the explanation of the functions.
 *
 * This template square matrix class can be used for doing various matrix manipulation.
 * This should be combined with the TVector class for doing almost anything
 * you ever dreamt of.
 */
template<unsigned int ROW> class TMatrix
{
protected:
    static REALTYPE _s[ROW*ROW];
    static float    _sf[ROW*ROW];
    static REALTYPE undef;

public:
    REALTYPE _[ROW*ROW];

public:
    /// Empty constructor
    inline TMatrix(bool clear = true){
        if(clear) Zero();
    }

    /// Copy constructor
    inline TMatrix(const TMatrix<ROW> & matrix){
        Set(matrix);
    }

    /// Constructor with data pointer to be copied in the matrix
    inline TMatrix(const REALTYPE *_){
        Set(_);
    }


    /// copy of the argument
    inline TMatrix<ROW>& Set(const TMatrix<ROW> & matrix){
        memcpy(_,matrix._,ROW*ROW*sizeof(REALTYPE));
        return *this;
    }

    /// Assigment of data pointer
    inline TMatrix<ROW>& Set(const REALTYPE *array){
        if(array) memcpy(_,array,ROW*ROW*sizeof(REALTYPE));
        return *this;
    }

    /// Assigment of data pointer
    inline TMatrix<ROW>& SetForceFloat(const float *array){
        float    *arrayPos = array;
        REALTYPE *dst      = _;
        unsigned int len = ROW*ROW;
        while(len--)
            *(dst++) = REALTYPE(*(arrayPos++));
        return *this;
    }

    /// Assigment of data pointer
    inline TMatrix<ROW>& Set(const Matrix& matrix){
        if((matrix.column == ROW)&&(matrix.row>=ROW)){
            memcpy(_,matrix._,ROW*ROW*sizeof(REALTYPE));
        }else{
            unsigned int len = MIN(ROW*ROW,matrix.row*matrix.column);
            memcpy(_,matrix._,len*sizeof(REALTYPE));
        }
        return *this;
    }

    /// Gets the data array
    inline REALTYPE* Array() const{
        return (REALTYPE*)_;
    }

    /// Sets all values to zero
    inline TMatrix<ROW>& Zero(){
        memset(_,0,ROW*ROW*sizeof(REALTYPE));
        return *this;
    }

    /// Sets the matrix to identity
    inline TMatrix<ROW>& Identity(){
        Zero();
        REALTYPE *dst = _;
        unsigned len = ROW;
        while(len--){
            *dst = R_ONE;
            dst += ROW+1;
        }
        return *this;
    }

    /// Gets a reference to the given element
    inline REALTYPE& operator() (const unsigned int row, const unsigned int col) {
        if((row<ROW)&&(col<ROW))
            return _[row*ROW + col];
        return undef;
    }
    inline REALTYPE& Ref(const unsigned int row, const unsigned int col) {
        if((row<ROW)&&(col<ROW))
            return _[row*ROW + col];
        return undef;
    }
    inline REALTYPE& RefNoCheck(const unsigned int row, const unsigned int col) {
        return _[row*ROW + col];
    }
    /// Gets the value of the given element
    inline REALTYPE At(const unsigned int row, const unsigned int col) const{
        if((row<ROW)&&(col<ROW))
            return _[row*ROW + col];
        return undef;
    }
    inline REALTYPE AtNoCheck(const unsigned int row, const unsigned int col) const{
        return _[row*ROW + col];
    }

    /// Returns the given column
    inline TVector<ROW> GetColumn(unsigned int col) const{
        TVector<ROW> vector(false);
        return GetColumn(col,vector);
    }
    /// Returns the given column in the vector
    inline TVector<ROW>& GetColumn(unsigned int col, TVector<ROW> &result) const {
        if(col<ROW){
            REALTYPE *src = (REALTYPE*)_ + col;
            REALTYPE *dst = result._;
            unsigned int len = ROW;
            while(len--){
                *(dst++) = *(src);
                src+=ROW;
            }
        }else{
            result.Zero();
        }
        return result;
    }

    /// Returns the given row
    inline TVector<ROW> GetRow(unsigned int row) const {
        TVector<ROW> vector(false);
        return GetRow(row,vector);
    }
    /// Returns the given column in the vector
    inline TVector<ROW>& GetRow(unsigned int row, TVector<ROW> &result) const {
        if(row<ROW){
            REALTYPE *src = _ + row*ROW;
            REALTYPE *dst = result._;
            unsigned int len = ROW;
            while(len--)
                *(dst++) = *(src++);
        }else{
            result.Zero();
        }
        return result;
    }


    /// Sets the given column
    inline TMatrix<ROW>& SetColumn(const TVector<ROW> &vector, unsigned int col){
        if(col<ROW){
            REALTYPE *dst = _ + col;
            REALTYPE *src = (REALTYPE*)vector._;
            unsigned int len = ROW;
            while(len--){
                *(dst) = *(src++);
                dst+=ROW;
            }
        }
        return *this;
    }
    /// Sets the given row
    inline TMatrix<ROW>& SetRow(const TVector<ROW> &vector, unsigned int row){
        if(row<ROW){
            REALTYPE *dst = _ + row*ROW;
            REALTYPE *src = (REALTYPE*)vector._;
            unsigned int len = ROW;
            while(len--)
                *(dst++) = *(src++);
        }
        return *this;
    }


    /// Assignment operator
    inline TMatrix<ROW>& operator = (const TMatrix<ROW> &matrix){
        return Set(matrix);
    }

    /// Inverse operator
    inline TMatrix<ROW> operator - () const{
        TMatrix<ROW> result(false);
        return Minus(result);
    }
    /// Inverse operator
    inline TMatrix<ROW>& Minus(TMatrix<ROW>& result) const{
        REALTYPE *dst = result._, *src =(REALTYPE*) _; unsigned len = ROW*ROW;
        while(len--) *(dst++) = -*(src++);
        return result;
    }
    /// Inverse operator
    inline TMatrix<ROW>& SMinus(){
        REALTYPE *src = _; unsigned len = ROW*ROW;
        while(len--){
            *(src) = -*(src);
            src++;
        }
        return *this;
    }


    /// Assignment data-wise operations
    inline TMatrix<ROW>& operator += (const TMatrix<ROW> &matrix) {
        REALTYPE *dst = (REALTYPE*)_, *src = (REALTYPE*)matrix._;
        unsigned int len = ROW*ROW;
        while(len--) *(dst++) += (*(src++));
        return *this;
    }
    inline TMatrix<ROW>& operator -= (const TMatrix<ROW> &matrix) {
        REALTYPE *dst = (REALTYPE*)_, *src = (REALTYPE*)matrix._;
        unsigned int len = ROW*ROW;
        while(len--) *(dst++) -= (*(src++));
        return *this;
    }
    inline TMatrix<ROW>& operator ^= (const TMatrix<ROW> &matrix) {
        REALTYPE *dst = (REALTYPE*)_, *src = (REALTYPE*)matrix._;
        unsigned int len = ROW*ROW;
        while(len--) *(dst++) *= (*(src++));
        return *this;
    }
    inline TMatrix<ROW>& operator /= (const TMatrix<ROW> &matrix) {
        REALTYPE *dst = (REALTYPE*)_, *src = (REALTYPE*)matrix._;
        unsigned int len = ROW*ROW;
        while(len--) *(dst++) /= (*(src++));
        return *this;
    }
    inline TMatrix<ROW>& operator *= (const TMatrix<ROW> &matrix) {
        TMatrix<ROW> copy(*this);
        copy.Mult(matrix,*this);
        return *this;
    }







    inline TMatrix<ROW>& operator += (REALTYPE scalar){
        REALTYPE *src = _; unsigned len = ROW*ROW;
        while(len--) *(src++) += scalar;
        return *this;
    }
    inline TMatrix<ROW>& operator -= (REALTYPE scalar){
        REALTYPE *src = _; unsigned len = ROW*ROW;
        while(len--) *(src++) -= scalar;
        return *this;
    }
    inline TMatrix<ROW>& operator *= (REALTYPE scalar){
        REALTYPE *src = _; unsigned len = ROW*ROW;
        while(len--) *(src++) *= scalar;
        return *this;
    }
    inline TMatrix<ROW>& operator /= (REALTYPE scalar){
        scalar = R_ONE/scalar;
        REALTYPE *src = _; unsigned len = ROW*ROW;
        while(len--) *(src++) *= scalar;
        return *this;
    }


    inline TMatrix<ROW> operator + (const TMatrix<ROW> &matrix) const{
        TMatrix<ROW> result(false);
        return Add(matrix,result);
    }
    inline TMatrix<ROW> operator - (const TMatrix<ROW> &matrix) const{
        TMatrix<ROW> result(false);
        return Sub(matrix,result);
    }
    inline TMatrix<ROW> operator ^ (const TMatrix<ROW> &matrix) const{
        TMatrix<ROW> result(false);
        return PMult(matrix,result);
    }
    inline TMatrix<ROW> operator / (const TMatrix<ROW> &matrix) const{
        TMatrix<ROW> result(false);
        return PDiv(matrix,result);
    }
    inline TMatrix<ROW> operator * (const TMatrix<ROW> &matrix) const{
        TMatrix<ROW> result(false);
        return Mult(matrix,result);
    }


    inline TMatrix<ROW>& Add(const TMatrix<ROW> &matrix, TMatrix<ROW> & result) const{
        REALTYPE *src0 = (REALTYPE*)_, *src1 = (REALTYPE*)matrix._, *dst = (REALTYPE*)result._;
        unsigned int len = ROW*ROW;
        while(len--)
                *(dst++) = (*(src0++)) + (*(src1++));
        return result;
    }
    inline TMatrix<ROW>& Sub(const TMatrix<ROW> &matrix, TMatrix<ROW> & result) const{
        REALTYPE *src0 = (REALTYPE*)_, *src1 = (REALTYPE*)matrix._, *dst = (REALTYPE*)result._;
        unsigned int len = ROW*ROW;
        while(len--)
                *(dst++) = (*(src0++)) - (*(src1++));
        return result;
    }
    inline TMatrix<ROW>& PMult(const TMatrix<ROW> &matrix, TMatrix<ROW> & result) const{
        REALTYPE *src0 = (REALTYPE*)_, *src1 = (REALTYPE*)matrix._, *dst = (REALTYPE*)result._;
        unsigned int len = ROW*ROW;
        while(len--)
                *(dst++) = (*(src0++)) * (*(src1++));
        return result;
    }
    inline TMatrix<ROW>& PDiv(const TMatrix<ROW> &matrix, TMatrix<ROW> & result) const{
        REALTYPE *src0 = (REALTYPE*)_, *src1 = (REALTYPE*)matrix._, *dst = (REALTYPE*)result._;
        unsigned int len = ROW*ROW;
        while(len--)
                *(dst++) = (*(src0++)) / (*(src1++));
        return result;
    }
    inline TMatrix<ROW>& Mult(const TMatrix<ROW> &matrix, TMatrix<ROW> & result) const{
        result.Zero();
        REALTYPE *cP1   = (REALTYPE*)_;
        REALTYPE *eP1   = cP1 + ROW*ROW;
        REALTYPE *cD    = result._;
        while(cP1!=eP1){
            REALTYPE *currP1  = cP1;
            REALTYPE *endP1   = currP1 + ROW;
            REALTYPE *currP2  = (REALTYPE*)matrix._;
            while(currP1!=endP1){
                REALTYPE *currPD  = cD;
                REALTYPE  curr1   = *currP1;
                REALTYPE *endP2   = currP2 + ROW;
                while(currP2!=endP2){
                    (*currPD++) += curr1 * (*(currP2++));
                }
                currP1++;
            }
            cD  += ROW;
            cP1 += ROW;
        }
        return result;
    }
    inline TMatrix<ROW>& TransposeMult(const TMatrix<ROW>& matrix, TMatrix<ROW>& result) const
    {
        result.Zero();
        REALTYPE *cP1   = _;
        REALTYPE *cP2   = matrix._;
        unsigned int kk = ROW;
        while(kk--){
            REALTYPE *currD  = result._;
            REALTYPE *currP1 = cP1;
            unsigned int len1 = ROW;
            while(len1--){
                REALTYPE *currP2 = cP2;
                unsigned int len2 = ROW;
                while(len2--){
                    *(currD++) += *(currP1) *(*(currP2++));
                }
                currP1++;
            }
            cP2 += ROW;
            cP1 += ROW;
        }
        return result;
    }
    inline TMatrix<ROW>& MultTranspose(const TMatrix<ROW>& matrix, TMatrix<ROW>& result) const{
        REALTYPE *cP1   = _;
        REALTYPE *currD = result._;
        unsigned int len1 = ROW;
        while(len1--){
            REALTYPE *currP2 = matrix._;
            unsigned int len2 = ROW;
            while(len2--){
                REALTYPE *currP1 = cP1;
                REALTYPE sum = R_ZERO;
                unsigned int len = ROW;
                while(len--)
                    sum += *(currP1++) * (*(currP2++));
                *(currD++) = sum;
            }
            cP1 += ROW;
        }
        return result;
    }


    inline TMatrix<ROW> operator + (REALTYPE scalar) const {
        TMatrix<ROW> result(false);
        return Add(scalar,result);
    }
    inline TMatrix<ROW> operator - (REALTYPE scalar) const {
        TMatrix<ROW> result(false);
        return Sub(scalar,result);
    }
    inline TMatrix<ROW> operator * (REALTYPE scalar) const {
        TMatrix<ROW> result(false);
        return Mult(scalar,result);
    }
    inline TMatrix<ROW> operator / (REALTYPE scalar) const {
        TMatrix<ROW> result(false);
        return Div(scalar,result);
    }

    inline TMatrix<ROW>& Add(REALTYPE scalar, TMatrix<ROW> & result) const{
        REALTYPE *dst = (REALTYPE*)result._, *src = (REALTYPE*)_; unsigned len = ROW*ROW;
        while(len--) *(dst++) = *(src++) + scalar;
        return result;
    }
    inline TMatrix<ROW>& Sub(REALTYPE scalar, TMatrix<ROW> & result) const{
        REALTYPE *dst = (REALTYPE*)result._, *src = (REALTYPE*)_; unsigned len = ROW*ROW;
        while(len--) *(dst++) = *(src++) - scalar;
        return result;
    }
    inline TMatrix<ROW>& Mult(REALTYPE scalar, TMatrix<ROW> & result) const{
        REALTYPE *dst = (REALTYPE*)result._, *src = (REALTYPE*)_; unsigned len = ROW*ROW;
        while(len--) *(dst++) = *(src++) * scalar;
        return result;
    }
    inline TMatrix<ROW>& Div(REALTYPE scalar, TMatrix<ROW> & result) const{
        scalar = R_ONE / scalar;
        REALTYPE *dst = (REALTYPE*)result._, *src = (REALTYPE*)_; unsigned len = ROW*ROW;
        while(len--) *(dst++) = *(src++) * scalar;
        return result;
    }



    inline TVector<ROW> operator * (const TVector<ROW> &vector) const{
        TVector<ROW> result(false);
        return Mult(vector,result);
    }
    inline TVector<ROW> Mult(const TVector<ROW> &vector) const {
        TVector<ROW> result(false);
        return Mult(vector,result);
    }
    inline TVector<ROW>& Mult(const TVector<ROW> &vector, TVector<ROW> & result) const {
        REALTYPE *src = (REALTYPE*)_;
        REALTYPE *dst = result._;
        unsigned int rowLen = ROW;
        while(rowLen--){
            REALTYPE sum = R_ZERO;
            REALTYPE *vSrc = (REALTYPE*)vector._;
            unsigned int colLen = ROW;
            while(colLen--)
                sum +=*(src++) * (*(vSrc++));
            *(dst++) = sum;
        }
        return result;
    }

    inline TVector<ROW> TransposeMult(const TVector<ROW> &vector) const{
        TVector<ROW> result(false);
        return MultTranspose(vector,result);
    }
    inline TVector<ROW>& TransposeMult(const TVector<ROW> &vector, TVector<ROW> & result) const{
        result.Zero();
        REALTYPE *src = (REALTYPE*)_;
        REALTYPE *vSrc = (REALTYPE*)vector._;
        unsigned int rowLen = ROW;
        while(rowLen--){
            REALTYPE *dst = result._;
            unsigned int colLen = ROW;
            while(colLen--)
                *(dst++) += *(src++) * (*vSrc);
            vSrc++;
        }
        return result;
    }

    /// Tests equality of two matrices
    inline bool operator == (const TMatrix<ROW>& matrix) const {
        REALTYPE *src = (REALTYPE*) _;
        REALTYPE *dst = (REALTYPE*) matrix._;
        unsigned int len = ROW*ROW;
        while(len--)
            if(*(src++) != *(dst++)) return false;
        return true;
    }
    /// tests inequality of two vectors
    inline bool operator != (const TMatrix<ROW>& matrix) const {
        return !(*this ==  matrix);
    }

    inline TMatrix<ROW> Abs(){
        TMatrix<ROW> result(false);
        Abs(result);
        return result;
    }
    inline TMatrix<ROW>& Abs(const TMatrix<ROW>& result){
        REALTYPE *src = _;
        REALTYPE *dst = result._;
        unsigned int len = ROW*ROW;
        while(len--)
            *(dst++) = fabs(*(src++));
        return result;
    }
    inline TMatrix<ROW>& SAbs(){
        REALTYPE *src = _;
        unsigned int len = ROW*ROW;
        while(len--){
            *src = fabs(*src);
            src++;
        }
        return *this;
    }

    inline REALTYPE Sum(){
        REALTYPE sum = R_ZERO;
        REALTYPE *src = _;
        unsigned int len = ROW*ROW;
        while(len--)
            sum += *(src++);
        return sum;
    }


    /// Self transpose
    inline TMatrix<ROW> STranspose(){
        REALTYPE tmp;
        REALTYPE *src = _+1;
        REALTYPE *dst = _+ROW;
        unsigned int rowLen = ROW;
        while(rowLen--){
            REALTYPE *cDst = dst;
            unsigned int colLen = rowLen;
            while(colLen--){
                tmp      = *cDst;
                *cDst    = *src;
                *(src++) = tmp;
                cDst += ROW;
            }
            src+=ROW-rowLen+1;
            dst+=ROW+1;
        }
        return *this;
    }

    /// Returns the transpose
    inline TMatrix<ROW> Transpose() const{
        TMatrix<ROW> result(false);
        return Transpose(result);
    }
    /// Returns the transpose in the result
    inline TMatrix<ROW>& Transpose(TMatrix<ROW>& result) const{
        REALTYPE *src = (REALTYPE*)_;
        REALTYPE *dst = result._;
        unsigned int rowLen = ROW;
        while(rowLen--){
            REALTYPE *cDst = dst;
            unsigned int colLen = ROW;
            while(colLen--){
                *cDst = *(src++);
                cDst += ROW;
            }
            dst++;
        }
        return result;
    }

    /// Set a diagonal matrix
    inline TMatrix<ROW>& Diag(const TVector<ROW>& diag){
        Zero();
        const REALTYPE *src = diag._;
        REALTYPE *dst = _;
        unsigned int len = ROW;
        while(len--){
            *dst = *(src++);
            dst+=ROW+1;
        }
        return *this;
    }

    /**
     * \brief Return a data pointer with data ordered in rows and not in column. Useful for opengl matrix manipulation
     * \param result The data array. If null, this function uses the internal static member of the matrix template.
     * \return The pointer to the data
     */
    REALTYPE* RowOrder(REALTYPE *result=NULL) const{
        REALTYPE *res;
        REALTYPE *src = _;
        REALTYPE *dst = res = (result?result:_s);
        unsigned int rowLen = ROW;
        while(rowLen--){
            REALTYPE *cDst = dst;
            unsigned int colLen = ROW;
            while(colLen--){
                *cDst = *(src++);
                cDst += ROW;
            }
            dst++;
        }
        return res;
    }

    /// Same as previous but force the target array to be of type float (usefl for OpenGL)
    float* RowOrderForceFloat(float result[ROW*ROW]=NULL) const {
        float *res;
        REALTYPE *src = (REALTYPE*)_;
        float *dst = res = (result?result:_sf);
        unsigned int rowLen = ROW;
        while(rowLen--){
            float *cDst = dst;
            unsigned int colLen = ROW;
            while(colLen--){
                *cDst = float(*(src++));
                cDst += ROW;
            }
            dst++;
        }
        return res;
    }

    /// Prints out the vector to stdout
    void Print() const {
        std::cout << "Matrix " <<ROW<<"x"<<ROW<<std::endl;;
        for (unsigned int j = 0; j < ROW; j++){
            for (unsigned int i = 0; i < ROW; i++)
                std::cout << _[j*ROW+i] <<" ";
            std::cout << std::endl;
        }
    }

};


template<unsigned int ROW> REALTYPE TMatrix<ROW>::_s[ROW*ROW];
template<unsigned int ROW> float    TMatrix<ROW>::_sf[ROW*ROW];
template<unsigned int ROW> REALTYPE TMatrix<ROW>::undef=R_ZERO;

#ifdef USE_MATHLIB_NAMESPACE
}
#endif

#endif
