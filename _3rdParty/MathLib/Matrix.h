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

#ifndef MATRIX_H
#define MATRIX_H
#include <assert.h>
#include <vector>
using namespace std;

#include "MathLibCommon.h"


#include "Macros.h"
#include "Vector.h"

#ifdef USE_MATHLIB_NAMESPACE
namespace MathLib {
#endif

#ifdef  USE_T_EXTENSIONS
template<unsigned int ROW> class TMatrix;
#endif


/**
 * \class Matrix
 *
 * \ingroup MathLib
 *
 * \brief The basic matrix class
 *
 * This matrix class can be used for doing various matrix manipulation.
 * This shFould be combined with the Vector class for doing almost anything
 * you ever dreamt of. Please understand almost as almost...
 */

class Matrix
{
    friend class Vector;
#ifdef  USE_T_EXTENSIONS
    template<unsigned int ROW> friend class TMatrix;
#endif

protected:
    static int bInverseOk;    ///< Tell if last inverse operation was sucessfull

    unsigned int  row;        ///< Number of rows
    unsigned int  column;     ///< Number of columns
    REALTYPE        *_;          ///< The data array

public:

    /// Empty constructor
    inline Matrix() {
        row    = 0;
        column = 0;
        _      = NULL;
    }

    /// Destructor
    inline virtual ~Matrix(){
        Release();
    }

    /// Copy constructor
    inline Matrix(const Matrix &matrix)
    {
        row    = 0;
        column = 0;
        _      = NULL;
        Set(matrix);
    }

    /**
    * \brief Create a sized matrix
    * \param rowSize  The row size
    * \param colSize  The column size
    * \param clear    Tell if the matrix sould be set to 0
    */
    inline Matrix(unsigned int rowSize, unsigned int colSize, bool clear = true)
    {
        row    = 0;
        column = 0;
        _      = NULL;
        Resize(rowSize,colSize,false);
        if(clear) Zero();
    }

    /**
     * \brief Create a matrix by copying a memory area
     * \param _     The array of REALTYPE to be copied
     * \param rowSize  The row size
     * \param colSize  The column size
     */
    inline Matrix(const REALTYPE *array, unsigned int rowSize, unsigned int colSize)
    {
        row     = 0;
        column  = 0;
        _       = NULL;
        Set(array,rowSize,colSize);
    }

#ifdef  USE_T_EXTENSIONS
    /// Copy Contructor of a template Matrix (see TMatrix)
    template<unsigned int ROW> inline Matrix(const TMatrix<ROW> &matrix)
    {
        row    = 0;
        column = 0;
        _      = NULL;
        Set(matrix._,ROW,ROW);
    }
#endif

    /**
    * \brief Set a matrix by copying a memory area
    * \param _     The array of REALTYPE to be copied
    * \param rowSize  The row size
    * \param colSize  The column size
    */
    inline Matrix& Set(const REALTYPE *array, unsigned int rowSize, unsigned int colSize){
        Resize(rowSize,colSize,false);
        if((row)&&(column)&&(array))
            memcpy(_,array,rowSize*colSize*sizeof(REALTYPE));
        return *this;
    }

    /**
    * \brief Set a matrix by copying a memory area
    * \param _     The array of REALTYPE to be copied
    * \param rowSize  The row size
    * \param colSize  The column size
    */
    inline Matrix& Set(const Matrix & matrix){
        Resize(matrix.row,matrix.column,false);
        if((row)&&(column))
            memcpy(_,matrix._,row*column*sizeof(REALTYPE));
        return *this;
    }


    /// Get the number of rows
    inline unsigned int RowSize() const{
        return row;
    }
    /// Get the number of columns
    inline unsigned int ColumnSize() const{
        return column;
    }
    /// Get the data array
    inline REALTYPE *Array() const{
        return _;
    }

    /// Clear the matrix
    inline Matrix& Zero()
    {
        if((row)&&(column))
            memset(_,0,row*column*sizeof(REALTYPE));
        return *this;
    }
    /// Identity matrix
    inline Matrix& Identity()
    {
        unsigned int k = (row>column?column:row);
        Zero();
        REALTYPE *ptr = _;
        while(k--){
            *ptr = R_ONE;
            ptr += column+1;
        }
        return *this;
    }
    /// Set all elements to 1
    inline Matrix& One()
    {
        REALTYPE *src = _;
        unsigned int len = row*column;
        while(len--)
            *(src++) = R_ONE;
        return *this;
    }

    /// Access to a reference of any element of the matrix
    inline REALTYPE& operator() (const unsigned int row, const unsigned int col){
        if((row<this->row)&&(col<this->column))
            return _[row*column+col];
        return Vector::undef;
    }
    /// Access to a reference of any element of the matrix
    inline REALTYPE& Ref(const unsigned int row, const unsigned int col){
        if((row<this->row)&&(col<this->column))
            return _[row*column+col];
        return Vector::undef;
    }
    /// Access to an element of the matrix
    inline REALTYPE At(const unsigned int row, const unsigned int col) const{
        if((row<this->row)&&(col<this->column))
            return _[row*column+col];
        return Vector::undef;
    }
    /// Access to a reference of any element of the matrix without checks
    inline REALTYPE& RefNoCheck(const unsigned int row, const unsigned int col){
        return _[row*column+col];
    }
    /// Access to an element of the matrix without checks
    inline REALTYPE AtNoCheck(const unsigned int row, const unsigned int col) const{
        return _[row*column+col];
    }

    /// Get a copy of a row into a vector
    inline Vector GetRow(const unsigned int r) const
    {
        Vector result;
        return GetRow(r,result);
    }
    /// Get a copy of a row into a vector
    inline Vector& GetRow(const unsigned int r, Vector& result) const
    {
        result.Resize(column,false);
        if(r<this->row){
            REALTYPE *src = _ + r*column;
            REALTYPE *dst = result._;
            unsigned int len = column;
            while(len--)
                *(dst++) = *(src++);
        }else{
            result.Zero();
        }
        return result;
    }

    /// Get a copy of a column into a vector
    inline Vector GetColumn(const unsigned int col) const
    {
        Vector result;
        return GetColumn(col,result);
    }
    /// Get a copy of a column into a vector
    inline Vector& GetColumn(const unsigned int col, Vector& result) const
    {
        result.Resize(row,false);
        if(col<column){
            REALTYPE *src = _ + col;
            REALTYPE *dst = result._;
            unsigned int len = row;
            while(len--){
                *(dst++) = *(src);
                src += column;
            }
        }else{
            result.Zero();
        }
        return result;
    }

    inline Matrix GetRows(unsigned int rowStart, unsigned int rowEnd) const{
        Matrix result;
        return GetRows(rowStart,rowEnd,result);
    }
    inline Matrix& GetRows(unsigned int rowStart, unsigned int rowEnd, Matrix& result) const{
        if(rowStart>rowEnd){
            return result.Resize(0,0,false);
        }
        unsigned int rowLength = rowEnd-rowStart+1;
        result.Resize(rowLength,column,false);
        if(rowStart>=row){
            return result.Zero();
        }
        REALTYPE *src = _ + rowStart*column;
        REALTYPE *dst = result._;
        unsigned int len;
        if(rowEnd<row) len = rowLength*column;
        else           len = (row-rowStart)*column;
        while(len--)
            *(dst++) = *(src++);
        if(rowEnd>=row){
            len = (rowEnd-row+1)*column;
            while(len--)
                *(dst++) = R_ZERO;
        }
        return result;
    }

    inline Matrix GetColumns(unsigned int colStart, unsigned int colEnd) const{
        Matrix result;
        return GetColumns(colStart,colEnd,result);
    }
    inline Matrix& GetColumns(unsigned int colStart, unsigned int colEnd, Matrix& result) const{
        if(colStart>colEnd){
            return result.Resize(0,0,false);
        }
        unsigned int colLength = colEnd-colStart+1;
        result.Resize(row,colLength,false);
        if(colStart>=column){
            return result.Zero();
        }
        REALTYPE *src = _ + colStart;
        REALTYPE *dst = result._;
        unsigned int dstOffset = 0;
        if(colEnd>=column){
            result.Zero();
            dstOffset = (colEnd-column+1);
            colEnd    = column-1;
            colLength = colEnd-colStart+1;
        }
        unsigned int rowLen = row;
        while(rowLen--){
            unsigned int colLen = colLength;
            REALTYPE *cSrc = src;
            while(colLen--)
                *(dst++) = *(cSrc++);
            dst += dstOffset;
            src += column;
        }
        return result;
    }

    inline Matrix GetMatrix(unsigned int rowStart, unsigned int rowEnd, unsigned int colStart, unsigned int colEnd) const{
        Matrix result;
        return GetMatrix(rowStart,rowEnd,colStart,colEnd,result);
    }
    inline Matrix& GetMatrix(unsigned int rowStart, unsigned int rowEnd, unsigned int colStart, unsigned int colEnd, Matrix &result) const{
        if((rowStart>rowEnd)||(colStart>colEnd)){
            return result.Resize(0,0,false);
        }
        unsigned int rowLength = rowEnd-rowStart+1;
        unsigned int colLength = colEnd-colStart+1;
        result.Resize(rowLength,colLength,false);
        if((rowStart>=row)||(colStart>=column)){
            return result.Zero();
        }
        unsigned int dstOffset = 0;
        if((rowEnd>=row)||(colEnd>=column)){
            result.Zero();
            if(rowEnd>=row){
                rowEnd = row-1;
                rowLength = rowEnd-rowStart+1;
            }
            if(colEnd>=column){
                dstOffset = (colEnd-column+1);
                colEnd    = column-1;
                colLength = colEnd-colStart+1;
            }
        }
        REALTYPE *src = _ + colStart + rowStart*column;
        REALTYPE *dst = result._;
        unsigned int rowLen = rowLength;
        while(rowLen--){
            REALTYPE *cSrc = src;
            unsigned int colLen = colLength;
            while(colLen--){
                *(dst++) = *(cSrc++);
            }
            src += column;
            dst += dstOffset;
        }
        return result;
    }

    /**
     * \brief Get a matrix spanning several rows of the matrix
     * \param row      The starting row
     * \param rowSize  The number of rows
     * \return         The resulting matrix
     */
    inline Matrix GetRowSpace(const unsigned int row, const unsigned int len) const
    {
        Matrix result;
        return GetRowSpace(row,len,result);
    }
    /**
    * \brief Get a matrix spanning several rows of the matrix
    * \param row      The starting row
    * \param len      The number of rows
    * \param result   The target matrix
    * \return         The resulting matrix
    */
    inline Matrix& GetRowSpace(const unsigned int row, const unsigned int len, Matrix &result) const
    {
        if(len>0){
            return GetRows(row,row+len-1,result);
        }
        return result.Resize(0,0,false);
    }

    /**
     * \brief Get a matrix spanning several columns of the matrix
     * \param row      The starting column
     * \param len      The number of columns
     * \return         The resulting matrix
     */
    inline Matrix GetColumnSpace(const unsigned int col, const unsigned int len) const
    {
        Matrix result;
        return GetColumnSpace(col,len,result);
    }
    /**
     * \brief Get a matrix spanning several columns of the matrix
     * \param row      The starting column
     * \param len      The number of columns
     * \param result   The target matrix
     * \return         The resulting matrix
     */
    inline Matrix& GetColumnSpace(const unsigned int col, const unsigned int len, Matrix &result) const
    {
        if(len>0){
            return GetColumns(col,col+len-1,result);
        }
        return result.Resize(0,0,false);
    }

    inline Matrix GetMatrixSpace(const unsigned int row, const unsigned int rowLen,
                                 const unsigned int col, const unsigned int colLen) const
    {
        Matrix result;
        return GetMatrixSpace(row,rowLen,col,colLen,result);
    }
    inline Matrix GetMatrixSpace(const unsigned int row, const unsigned int rowLen,
                                 const unsigned int col, const unsigned int colLen,
                                 Matrix& result) const
    {
        if((rowLen>0)&&(colLen>0)){
            return GetMatrix(row,row+rowLen-1,col,col+colLen-1,result);
        }
        return result.Resize(0,0,false);
    }


    /**
     * \brief Assign a value to a matrix row
     * \param value    The value
     * \param row      The row
     */
    inline Matrix& SetRow(const REALTYPE value, const unsigned int row)
    {
        if(row<this->row){
            REALTYPE *src = _+ row*column;
            unsigned int len = column;
            while(len--){
                *(src++) = value;
            }
        }
        return *this;
    }

    /**
     * \brief Assign a vector to a matrix row
     * \param vector   The input vector
     * \param row      The row
     */
    inline Matrix& SetRow(const Vector &vector, const unsigned int row)
    {
        if(row<this->row){
            REALTYPE *src = vector._;
            REALTYPE *dst = _+ row*column;
            unsigned int len = (column<=vector.row?column:vector.row);
            while(len--){
                *(dst++) = *(src++);
            }
        }
        return *this;
    }

    /**
     * \brief Assign a value to a matrix column
     * \param value    The value
     * \param row      The row
     */
    inline Matrix& SetColumn(const REALTYPE value, const unsigned int col)
    {
        if(col<this->column){
            REALTYPE *src = _ + col;
            unsigned int len = row;
            while(len--){
                *src = value;
                src += column;
            }
        }
        return *this;
    }
    /**
     * \brief Assign a vector to a matrix column
     * \param vector   The input vector
     * \param col      The column
     */
    inline Matrix& SetColumn(const Vector &vector, const unsigned int col)
    {
        if(col<this->column){
            REALTYPE *src = vector._;
            REALTYPE *dst = _+ col;
            unsigned int len = (row<=vector.row?row:vector.row);
            while(len--){
                *dst = *(src++);
                dst += column;
            }
        }
        return *this;
    }

    /**
   * \brief Assign a matrix to the current matrix rows
   * \param vector   The input matrix
   * \param row      The starting row
   */
    inline Matrix& SetRowSpace(const Matrix &matrix, const unsigned int row)
    {
        if(row<this->row){
            const unsigned int ki = (column<=matrix.column?column:matrix.column);
            const unsigned int kj = (row+matrix.row<=this->row?row+matrix.row:this->row);
            for (unsigned int j = row; j < kj; j++)
                for (unsigned int i = 0; i < ki; i++)
                    _[j*column+i] = matrix._[(j-row)*matrix.column+i];
        }
        return *this;
    }

    /**
   * \brief Assign a matrix to the current matrix columns
   * \param vector   The input matrix
   * \param col      The starting column
   */
    inline Matrix& SetColumnSpace(const Matrix &matrix, const unsigned int col)
    {
        if(col<this->column){
            const unsigned int kj = (row<=matrix.row?row:matrix.row);
            const unsigned int ki = (col+matrix.column<=this->column?col+matrix.column:this->column);
            for (unsigned int j = 0; j < kj; j++)
                for (unsigned int i = col; i < ki; i++)
                    _[j*column+i] = matrix._[j*matrix.column+(i-col)];
        }
        return *this;
    }


    inline Matrix GetRowSpace(const Vector& ids, Matrix &result) const
    {
        int s = ids.Size();
        IndicesVector id;
        for(int i=0;i<s;i++) id.push_back(int(ROUND(ids.At(i))));
        return GetRowSpace(id,result);
    }

    /**
   * \brief Get a matrix spanning several rows of the matrix
   * \param ids      The indices of the desired rows
   * \return         The resulting matrix
   */
    inline Matrix GetRowSpace(const IndicesVector& ids) const
    {
        Matrix result(ids.size(),column);
        return GetRowSpace(ids,result);
    }

    inline Matrix& GetRowSpace(const IndicesVector& ids, Matrix &result) const
    {
        const unsigned int k=ids.size();
        result.Resize(k,column,false);
        for(unsigned int i=0;i<k;i++){
            const unsigned int g      = ids[i];
            const unsigned int offset = i*column;
            if(g<row){
                for(unsigned int j=0;j<column;j++)
                    result._[offset+j] = _[g*column+j];
            }else{
                for(unsigned int j=0;j<column;j++)
                    result._[offset+j] = R_ZERO;
            }
        }
        return result;
    }

    inline Matrix GetColumnSpace(const Vector& ids, Matrix &result) const
    {
        int s = ids.Size();
        IndicesVector id;
        for(int i=0;i<s;i++) id.push_back(int(ROUND(ids.At(i))));
        return GetColumnSpace(id,result);
    }
    /**
   * \brief Get a matrix spanning several columns of the matrix
   * \param ids      The indices of the desired columns
   * \return         The resulting matrix
   */
    inline Matrix GetColumnSpace(const IndicesVector& ids) const
    {
        Matrix result(row,ids.size());
        return GetColumnSpace(ids,result);
    }

    inline Matrix& GetColumnSpace(const IndicesVector& ids, Matrix &result) const
    {
        const unsigned int k=ids.size();
        result.Resize(row,k);
        for(unsigned int i=0;i<k;i++){
            const unsigned int g = ids[i];
            if(g<column){
                for(unsigned int j=0;j<row;j++)
                    result._[j*k+i] = _[j*column+g];
            }else{
                for(unsigned int j=0;j<row;j++)
                    result._[j*k+i] = R_ZERO;
            }
        }
        return result;
    }

    inline Matrix GetMatrixSpace(const Vector& rowIds, const Vector& colIds, Matrix &result) const
    {
        int r = rowIds.Size();
        int c = colIds.Size();
        IndicesVector idr,idc;
        for(int i=0;i<r;i++) idr.push_back(int(ROUND(rowIds.At(i))));
        for(int i=0;i<c;i++) idc.push_back(int(ROUND(colIds.At(i))));
        return GetMatrixSpace(idr,idc,result);
    }
    /**
   * \brief Get a matrix spanning several rows and columns of the matrix
   * \param rowIds      The indices of the desired rows
   * \param colIds      The indices of the desired columns
   * \return            The resulting matrix
   */
    inline Matrix GetMatrixSpace(const IndicesVector& rowIds,const IndicesVector& colIds) const
    {
        Matrix result(rowIds.size(),colIds.size());
        return GetMatrixSpace(rowIds,colIds,result);
    }

    inline Matrix& GetMatrixSpace(const IndicesVector& rowIds,const IndicesVector& colIds, Matrix &result) const
    {
        const unsigned int k1=rowIds.size();
        const unsigned int k2=colIds.size();
        result.Resize(k1,k2);
        for(unsigned int i=0;i<k1;i++){
            const unsigned int g1 = rowIds[i];
            if(g1<row){
                for(unsigned int j=0;j<k2;j++){
                    const unsigned int g2 = colIds[j];
                    if(g2<column){
                        result._[i*k2+j] = _[g1*column+g2];
                    }else{
                        result._[i*k2+j] = R_ZERO;
                    }
                }
            }else{
                for(unsigned int j=0;j<k2;j++)
                    result._[i*k2+j] = R_ZERO;
            }
        }
        return result;
    }

    /**
   * \brief Set a specified column set of the matrix with columns of the source matrix
   * \param ids      The indices of the desired columns
   * \param source   The source matrix
   * \return         The resulting matrix
   */
    inline Matrix& SetColumnSpace(const IndicesVector& ids, const Matrix &source)
    {
        const unsigned int k      = MIN(ids.size(),source.column);
        const unsigned int r      = MIN(row,source.row);
        for(unsigned int i=0;i<k;i++){
            const unsigned int g = ids[i];
            if(g<column){
                for(unsigned int j=0;j<r;j++)
                    _[j*column+g] = source._[j*source.column+i];
            }
        }
        return *this;
    }


    inline Matrix& InsertSubRow(unsigned int startRow, unsigned int startColumn,
                                const Matrix& matrix,
                                unsigned int matrixRow,
                                unsigned int matrixStartColumn, unsigned int matrixColumnLength){

        // Check submatrix boundaries
        if((matrixRow         >= matrix.row)||
                (matrixStartColumn >= matrix.column)) return *this;

        // Check matrix boundaries
        if((startRow    >= row)||
                (startColumn >= column)) return *this;

        if(matrixStartColumn+matrixColumnLength > matrix.column) matrixColumnLength = matrix.column-matrixStartColumn;

        if(startColumn+matrixColumnLength > column) matrixColumnLength = column-startColumn;

        unsigned int rowOffset       = startRow*column;
        unsigned int matrixRowOffset = matrixRow*matrix.column;
        unsigned int colOffset       = startColumn;
        unsigned int matrixColOffset = matrixStartColumn;
        for(unsigned int j=0;j<matrixColumnLength;j++){
            _[rowOffset+colOffset] = matrix._[matrixRowOffset+matrixColOffset];
            colOffset++;
            matrixColOffset++;
        }
        return *this;
    }

    inline Matrix& InsertSubColumn(unsigned int startRow, unsigned int startColumn,
                                   const Matrix& matrix,
                                   unsigned int matrixStartRow, unsigned int matrixRowLength,
                                   unsigned int matrixColumn){

        if((matrixStartRow >= matrix.row)||
                (matrixColumn   >= matrix.column)) return *this;
        if((startRow    >= row)||
                (startColumn >= column)) return *this;

        if(matrixStartRow+   matrixRowLength    > matrix.row)    matrixRowLength    = matrix.row   -matrixStartRow;

        if(startRow+   matrixRowLength    > row)    matrixRowLength    = row   -startRow;

        unsigned int rowOffset       = startRow*column;
        unsigned int matrixRowOffset = matrixStartRow*matrix.column;
        for(unsigned int i=0;i<matrixRowLength;i++){
            _[rowOffset+startColumn] = matrix._[matrixRowOffset+matrixColumn];
            rowOffset       +=column;
            matrixRowOffset +=matrix.column;
        }
        return *this;
    }

    inline Matrix& InsertSubMatrix(unsigned int startRow, unsigned int startColumn,
                                   const Matrix& matrix,
                                   unsigned int matrixStartRow, unsigned int matrixRowLength,
                                   unsigned int matrixStartColumn, unsigned int matrixColumnLength){

        // Check submatrix boundaries
        if((matrixStartRow    >= matrix.row)||
                (matrixStartColumn >= matrix.column)) return *this;

        // Check matrix boundaries
        if((startRow    >= row)||
                (startColumn >= column)) return *this;

        if(matrixStartRow+   matrixRowLength    > matrix.row)    matrixRowLength    = matrix.row   -matrixStartRow;
        if(matrixStartColumn+matrixColumnLength > matrix.column) matrixColumnLength = matrix.column-matrixStartColumn;

        if(startRow+   matrixRowLength    > row)    matrixRowLength    = row   -startRow;
        if(startColumn+matrixColumnLength > column) matrixColumnLength = column-startColumn;

        unsigned int rowOffset       = startRow*column;
        unsigned int matrixRowOffset = matrixStartRow*matrix.column;
        for(unsigned int i=0;i<matrixRowLength;i++){
            unsigned int colOffset       = startColumn;
            unsigned int matrixColOffset = matrixStartColumn;
            for(unsigned int j=0;j<matrixColumnLength;j++){
                _[rowOffset+colOffset] = matrix._[matrixRowOffset+matrixColOffset];
                colOffset++;
                matrixColOffset++;
            }
            rowOffset       +=column;
            matrixRowOffset +=matrix.column;
        }
        return *this;
    }

    inline Matrix operator - () const
    {
        Matrix result(row,column,false);
        REALTYPE *src = _;
        REALTYPE *dst = result._;
        unsigned int len = row*column;
        while(len--){
            *(dst++) = -(*(src++));
        }
        return result;
    }
    inline Matrix& SMinus()
    {
        REALTYPE *src = _;
        unsigned int len = row*column;
        while(len--){
            *(src) = -(*(src));
            src++;
        }
        return *this;
    }

    inline virtual Matrix& operator = (const Matrix &matrix)
    {
        return Set(matrix);
    }

    inline Matrix& operator += (const Matrix &matrix)
    {
        const unsigned int kj = (row<=matrix.row?row:matrix.row);
        REALTYPE *dst = _;
        REALTYPE *src = matrix._;
        if(column==matrix.column){
            unsigned int len = column*kj;
            while(len--){
                *(dst++) += *(src++);
            }
        }else if(column < matrix.column){
            unsigned int colLen = column;
            unsigned int colDif = matrix.column-column;
            while(colLen--){
                unsigned int rowLen = kj;
                while(rowLen--){
                    *(dst++) += *(src++);
                }
                src += colDif;
            }
        }else{
            unsigned int colLen = matrix.column;
            unsigned int colDif = column - matrix.column;
            while(colLen--){
                unsigned int rowLen = kj;
                while(rowLen--){
                    *(dst++) += *(src++);
                }
                dst += colDif;
            }
        }
        return *this;
    }

    inline Matrix& operator -= (const Matrix &matrix)
    {
        const unsigned int kj = (row<=matrix.row?row:matrix.row);
        REALTYPE *dst = _;
        REALTYPE *src = matrix._;
        if(column==matrix.column){
            unsigned int len = column*kj;
            while(len--){
                *(dst++) -= *(src++);
            }
        }else if(column < matrix.column){
            unsigned int colLen = column;
            unsigned int colDif = matrix.column-column;
            while(colLen--){
                unsigned int rowLen = kj;
                while(rowLen--){
                    *(dst++) -= *(src++);
                }
                src += colDif;
            }
        }else{
            unsigned int colLen = matrix.column;
            unsigned int colDif = column - matrix.column;
            while(colLen--){
                unsigned int rowLen = kj;
                while(rowLen--){
                    *(dst++) -= *(src++);
                }
                dst += colDif;
            }
        }
        return *this;
    }

    /// matrix multiplication
    inline Matrix& operator *= (const Matrix &matrix){
        Matrix tmp;
        Mult(matrix,tmp);
        return Swap(tmp);
    }

    /// Element-wise multiplication
    inline Matrix& operator ^= (const Matrix &matrix)
    {
        const unsigned int kj = (row<=matrix.row?row:matrix.row);
        REALTYPE *dst = _;
        REALTYPE *src = matrix._;
        if(column==matrix.column){
            unsigned int len = column*kj;
            while(len--){
                *(dst++) *= *(src++);
            }
        }else if(column < matrix.column){
            unsigned int colLen = column;
            unsigned int colDif = matrix.column-column;
            while(colLen--){
                unsigned int rowLen = kj;
                while(rowLen--){
                    *(dst++) *= *(src++);
                }
                src += colDif;
            }
        }else{
            unsigned int colLen = matrix.column;
            unsigned int colDif = column - matrix.column;
            while(colLen--){
                unsigned int rowLen = kj;
                while(rowLen--){
                    *(dst++) *= *(src++);
                }
                dst += colDif;
            }
        }
        return *this;
    }

    
    /* Row wise multiplication by a vector */

    inline Matrix& operator ^= (Vector vec)
    {

        for (unsigned int j = 0; j < row; j++)
        {
            REALTYPE scalar = vec(j);
            for (unsigned int i = 0; i < column; i++)
                _[j*column+i] *= scalar;
        }
        return *this;
    }



    /* Row wise division by a vector */

    inline Matrix& operator /= (Vector vec)
    {

        for (unsigned int j = 0; j < row; j++)
        {
            REALTYPE scalar = R_ONE/vec(j);
            for (unsigned int i = 0; i < column; i++)
                _[j*column+i] *= scalar;
        }
        return *this;
    }



    /// Element-wise division
    inline Matrix& operator /= (const Matrix &matrix)
    {
        const unsigned int kj = (row<=matrix.row?row:matrix.row);
        REALTYPE *dst = _;
        REALTYPE *src = matrix._;
        if(column==matrix.column){
            unsigned int len = column*kj;
            while(len--){
                *(dst++) /= *(src++);
            }
        }else if(column < matrix.column){
            unsigned int colLen = column;
            unsigned int colDif = matrix.column-column;
            while(colLen--){
                unsigned int rowLen = kj;
                while(rowLen--){
                    *(dst++) /= *(src++);
                }
                src += colDif;
            }
        }else{
            unsigned int colLen = matrix.column;
            unsigned int colDif = column - matrix.column;
            while(colLen--){
                unsigned int rowLen = kj;
                while(rowLen--){
                    *(dst++) /= *(src++);
                }
                dst += colDif;
            }
        }
        return *this;
    }

    inline Matrix& operator += (REALTYPE scalar)
    {
        REALTYPE *src = _;
        unsigned int len = row*column;
        while(len--){
            *(src++) += scalar;
        }
        return *this;
    }

    inline Matrix& operator -= (REALTYPE scalar)
    {
        REALTYPE *src = _;
        unsigned int len = row*column;
        while(len--){
            *(src++) -= scalar;
        }
        return *this;
    }

    inline Matrix& operator *= (REALTYPE scalar)
    {
        REALTYPE *src = _;
        unsigned int len = row*column;
        while(len--){
            *(src++) *= scalar;
        }
        return *this;
    }

    inline Matrix& operator /= (REALTYPE scalar)
    {
        scalar = R_ONE/scalar;
        return *(this) *= scalar;
    }

    inline Matrix operator + (const Matrix &matrix) const
    {
        Matrix result(row,column,false);
        return Add(matrix,result);
    }
    inline Matrix operator - (const Matrix &matrix) const
    {
        Matrix result(row,column,false);
        return Sub(matrix,result);
    }
    /// Element-wise multiplication
    inline Matrix operator ^ (const Matrix &matrix) const
    {
        Matrix result(row,column,false);
        return PMult(matrix,result);
    }
    inline Matrix operator / (const Matrix &matrix) const
    {
        Matrix result(row,column,false);
        return PDiv(matrix,result);
    }

    inline Matrix& ScaleAddTo(REALTYPE scale, Matrix &result) const
    {
        const unsigned int kj = (row<=result.row?row:result.row);
        REALTYPE *dst = result._;
        REALTYPE *src = _;
        if(column==result.column){
            unsigned int len = column*kj;
            while(len--){
                *(dst++) += *(src++) * scale;
            }
        }else if(result.column < column){
            unsigned int colLen = result.column;
            unsigned int colDif = column-result.column;
            while(colLen--){
                unsigned int rowLen = kj;
                while(rowLen--){
                    *(dst++) += *(src++) * scale;
                }
                src += colDif;
            }
        }else{
            unsigned int colLen = column;
            unsigned int colDif = result.column - column;
            while(colLen--){
                unsigned int rowLen = kj;
                while(rowLen--){
                    *(dst++) += *(src++) * scale;
                }
                dst += colDif;
            }
        }
        return result;
    }

    inline Matrix& Add(const Matrix &matrix, Matrix &result) const
    {
        result.Resize(row,column,false);
        const unsigned int kj = (row<=matrix.row?row:matrix.row);
        REALTYPE *dst   = result._;
        REALTYPE *src0  = _;
        REALTYPE *src   = matrix._;
        if(column==matrix.column){
            unsigned int len = column*kj;
            while(len--){
                *(dst++) = *(src0++) + *(src++);
            }
        }else if(column < matrix.column){
            unsigned int colLen = column;
            unsigned int colDif = matrix.column-column;
            while(colLen--){
                unsigned int rowLen = kj;
                while(rowLen--){
                    *(dst++) = *(src0++) + *(src++);
                }
                src += colDif;
            }
        }else{
            unsigned int colLen = matrix.column;
            unsigned int colDif = column - matrix.column;
            while(colLen--){
                unsigned int rowLen = kj;
                while(rowLen--){
                    *(dst++) = *(src0++) + *(src++);
                }
                rowLen = colDif;
                while(rowLen--){
                    *(dst++) = *(src0++);
                }
            }
        }
        if(kj!=row){
            unsigned int len = column*(kj-matrix.row);
            while(len--){
                *(dst++) = *(src0++);
            }
        }
        return result;
    }
    inline Matrix& Sub(const Matrix &matrix, Matrix &result) const
    {
        result.Resize(row,column,false);
        const unsigned int kj = (row<=matrix.row?row:matrix.row);
        REALTYPE *dst   = result._;
        REALTYPE *src0  = _;
        REALTYPE *src   = matrix._;
        if(column==matrix.column){
            unsigned int len = column*kj;
            while(len--){
                *(dst++) = *(src0++) - *(src++);
            }
        }else if(column < matrix.column){
            unsigned int colLen = column;
            unsigned int colDif = matrix.column-column;
            while(colLen--){
                unsigned int rowLen = kj;
                while(rowLen--){
                    *(dst++) = *(src0++) - *(src++);
                }
                src += colDif;
            }
        }else{
            unsigned int colLen = matrix.column;
            unsigned int colDif = column - matrix.column;
            while(colLen--){
                unsigned int rowLen = kj;
                while(rowLen--){
                    *(dst++) = *(src0++) - *(src++);
                }
                rowLen = colDif;
                while(rowLen--){
                    *(dst++) = *(src0++);
                }
            }
        }
        if(kj!=row){
            unsigned int len = column*(kj-matrix.row);
            while(len--){
                *(dst++) = *(src0++);
            }
        }
        return result;
    }
    /// Element-wise multiplication
    inline Matrix& PMult(const Matrix &matrix, Matrix &result) const
    {
        result.Resize(row,column,false);
        const unsigned int kj = (row<=matrix.row?row:matrix.row);
        REALTYPE *dst   = result._;
        REALTYPE *src0  = _;
        REALTYPE *src   = matrix._;
        if(column==matrix.column){
            unsigned int len = column*kj;
            while(len--){
                *(dst++) = *(src0++) * *(src++);
            }
        }else if(column < matrix.column){
            unsigned int colLen = column;
            unsigned int colDif = matrix.column-column;
            while(colLen--){
                unsigned int rowLen = kj;
                while(rowLen--){
                    *(dst++) = *(src0++) * *(src++);
                }
                src += colDif;
            }
        }else{
            unsigned int colLen = matrix.column;
            unsigned int colDif = column - matrix.column;
            while(colLen--){
                unsigned int rowLen = kj;
                while(rowLen--){
                    *(dst++) = *(src0++) * *(src++);
                }
                rowLen = colDif;
                while(rowLen--){
                    *(dst++) = *(src0++);
                }
            }
        }
        if(kj!=row){
            unsigned int len = column*(kj-matrix.row);
            while(len--){
                *(dst++) = *(src0++);
            }
        }
        return result;
    }
    /// Element-wise division
    inline Matrix& PDiv(const Matrix &matrix, Matrix &result) const
    {
        result.Resize(row,column,false);
        const unsigned int kj = (row<=matrix.row?row:matrix.row);
        REALTYPE *dst   = result._;
        REALTYPE *src0  = _;
        REALTYPE *src   = matrix._;
        if(column==matrix.column){
            unsigned int len = column*kj;
            while(len--){
                *(dst++) = *(src0++) / *(src++);
            }
        }else if(column < matrix.column){
            unsigned int colLen = column;
            unsigned int colDif = matrix.column-column;
            while(colLen--){
                unsigned int rowLen = kj;
                while(rowLen--){
                    *(dst++) = *(src0++) / *(src++);
                }
                src += colDif;
            }
        }else{
            unsigned int colLen = matrix.column;
            unsigned int colDif = column - matrix.column;
            while(colLen--){
                unsigned int rowLen = kj;
                while(rowLen--){
                    *(dst++) = *(src0++) / *(src++);
                }
                rowLen = colDif;
                while(rowLen--){
                    *(dst++) = *(src0++);
                }
            }
        }
        if(kj!=row){
            unsigned int len = column*(kj-matrix.row);
            while(len--){
                *(dst++) = *(src0++);
            }
        }
        return result;
    }


    inline Matrix operator + (REALTYPE scalar) const
    {
        Matrix result(row,column,false);
        return Add(scalar,result);
    }
    inline Matrix operator - (REALTYPE scalar) const
    {
        Matrix result(row,column,false);
        return Sub(scalar,result);
    }
    inline Matrix operator * (REALTYPE scalar) const
    {
        Matrix result(row,column,false);
        return Mult(scalar,result);
    }
    inline Matrix operator / (REALTYPE scalar) const
    {
        Matrix result(row,column,false);
        return Div(scalar,result);
    }

    inline Matrix& Add(REALTYPE scalar, Matrix& result) const
    {
        result.Resize(row,column,false);
        REALTYPE *src = _;
        REALTYPE *dst = result._;
        unsigned int len = row*column;
        while(len--){
            *(dst++) = *(src++) + scalar;
        }
        return result;
    }
    inline Matrix& Sub(REALTYPE scalar, Matrix& result) const
    {
        return Add(-scalar,result);
    }
    inline Matrix& Mult(REALTYPE scalar, Matrix& result) const
    {
        result.Resize(row,column,false);
        REALTYPE *src = _;
        REALTYPE *dst = result._;
        unsigned int len = row*column;
        while(len--){
            *(dst++) = *(src++) * scalar;
        }
        return result;
    }
    inline Matrix& Div(REALTYPE scalar, Matrix& result) const
    {
        return Mult(R_ONE/scalar,result);
    }

    inline bool operator == (const Matrix& matrix) const
    {
        if((row!=matrix.row)||(column!=matrix.column)) return false;
        REALTYPE *src = _;
        REALTYPE *dst = matrix._;
        unsigned int len = row*column;
        while(len--){
            if( *(dst++) != *(src++) ) return false;
        }
        return true;
    }

    inline bool operator != (const Matrix& matrix) const
    {
        return !(*this ==  matrix);
    }

    /// Matrix multiplication against a vector
    inline Vector operator * (const Vector &vector) const
    {
        Vector result(row,false);
        return Mult(vector,result);
    }
    /// Matrix multiplication against a vector
    inline Vector Mult(const Vector &vector) const
    {
        Vector result(row,false);
        return Mult(vector,result);
    }
    /// Matrix multiplication against a vector
    inline Vector& Mult(const Vector &vector, Vector &result) const
    {
        result.Resize(row,false);
        const unsigned int ki = (column<=vector.row?column:vector.row);
        unsigned int rowLen = row;
        REALTYPE *dst = result._;
        REALTYPE *srcM = _;
        while(rowLen--){
            REALTYPE *srcV = vector._;
            REALTYPE sum = R_ZERO;
            unsigned int colLen = ki;
            while(colLen--){
                sum += (*(srcM++)) * (*(srcV++));
            }
            srcM += column-ki;
            *(dst++) = sum;
        }
        return result;
    }

    /// Transpose Matrix multiplication against a vector
    inline Vector TransposeMult(const Vector &vector) const
    {
        Vector result(column,false);
        return TransposeMult(vector,result);
    }

    /// Transpose Matrix multiplication against a vector
    inline Vector& TransposeMult(const Vector &vector, Vector &result) const
    {
        result.Resize(column,false);
        result.Zero();
        const unsigned int ki = (row<vector.row?row:vector.row);

        REALTYPE *srcV = vector._;
        REALTYPE *srcM = _;
        unsigned int rowLen = ki;
        while(rowLen--){
            REALTYPE *dst = result._;
            unsigned int colLen = column;
            while(colLen--){
                *(dst++) += (*(srcM++)) * (*(srcV));
            }
            srcV++;
        }
        return result;
    }


    inline Matrix& SAddToRow(const Vector &vector){
        const unsigned int k = (column<=vector.row?column:vector.row);
        REALTYPE *ptr = _;
        unsigned int rowLen = row;
        while(rowLen--){
            REALTYPE *ptr2 = ptr;
            REALTYPE *ptrv = vector._;
            unsigned int colLen = k;
            while(colLen--){
                *(ptr2++) += *(ptrv++);
            }
            ptr += column;
        }
        return *this;
    }

    inline Matrix& SAddToColumn(const Vector &vector){
        REALTYPE *ptr = _;
        REALTYPE *ptrv = vector._;
        unsigned int rowLen = (row<=vector.row?row:vector.row);
        while(rowLen--){
            unsigned int colLen = column;
            while(colLen--)
                *(ptr++) += *(ptrv);
            ptrv++;
        }
        return *this;
    }


    /// Multiplication of each ith row by the ith element of a vector
    inline Matrix MultRow(const Vector &vector) const {
        Matrix result;
        return MultRow(vector,result);
    }
    inline Matrix& MultRow(const Vector &vector, Matrix& result) const {
        result.Set(*this);
        return result.SMultRow(vector);
    }
    /// Self multiplication of each ith row by the ith element of a vector
    inline Matrix& SMultRow(const Vector &vector)
    {
        REALTYPE *cDst = _;
        REALTYPE *cSrc = vector._;
        unsigned int ki = (row<=vector.row?row:vector.row);
        while(ki--){
            REALTYPE v = *(cSrc++);
            unsigned int len = column;
            while(len--) (*cDst++) *= v;
        }
        if(row>vector.row){
            unsigned int len = column * (row-vector.row);
            while(len--) *(cDst++) = R_ZERO;
        }
        return *this;
    }

    /// Multiplication of each ith column by the ith element of a vector
    inline Matrix MultColumn(const Vector &vector) const {
        Matrix result;
        return MultColumn(vector,result);
    }
    inline Matrix& MultColumn(const Vector &vector, Matrix& result) const {
        result.Set(*this);
        return result.SMultColumn(vector);
    }
    /// Self multiplication of each ith row by the ith element of a vector
    inline Matrix& SMultColumn(const Vector &vector)
    {
        REALTYPE *cDst = _;
        if(column<=vector.row){
            unsigned int len = row;
            while(len--){
                REALTYPE *cSrc = vector._;
                unsigned int colLen = column;
                while(colLen--) *(cDst++) *= *(cSrc++);
            }
        }else{
            unsigned int len = row;
            while(len--){
                REALTYPE *cSrc = vector._;
                unsigned int colLen = vector.row;
                while(colLen--) *(cDst++) *= *(cSrc++);
                colLen = row-vector.row;
                while(colLen--) *(cDst++) = R_ZERO;
            }
        }
        return *this;
    }


    /// Matrix multiplication
    inline Matrix operator * (const Matrix &matrix) const {
        Matrix result;
        return Mult(matrix,result);
    }
    /// Matrix multiplication
    inline Matrix Mult(const Matrix &matrix) const {
        Matrix result;
        return Mult(matrix,result);
    }
    /// Matrix multiplication
    inline Matrix& Mult(const Matrix &matrix, Matrix &result) const
    {
        result.Resize(row,matrix.column,false);
        result.Zero();
        const unsigned int rcol = result.column;
        const unsigned int kk = (column<=matrix.row?column:matrix.row);

        REALTYPE *cP1   = _;
        REALTYPE *eP1   = cP1 + row*column;
        REALTYPE *cD    = result._;

        while(cP1!=eP1){
            REALTYPE *currP1  = cP1;
            REALTYPE *endP1   = currP1 + kk;
            REALTYPE *currP2  = matrix._;
            while(currP1!=endP1){
                REALTYPE *currPD  = cD;
                REALTYPE  curr1   = *currP1;
                REALTYPE *endP2   = currP2 + rcol;
                while(currP2!=endP2){
                    (*currPD++) += curr1 * (*(currP2++));
                }
                currP1++;
            }
            cD  += rcol;
            cP1 += column;
        }
        return result;
    }


    inline Matrix& TransposeMult(const Matrix &matrix) const{
        Matrix result;
        return TransposeMult(matrix,result);
    }
    /// Transpose then Multiply by the matrix :
    // result = this^T * matrix (avoiding a useless call to .Transpose() )
    inline Matrix& TransposeMult(const Matrix &matrix, Matrix &result) const
    {
        //[a0 a1 .. an] [b0 b1 .. bn]
        result.Resize(column,matrix.column,false);
        result.Zero();
        unsigned int kk = (row<=matrix.row?row:matrix.row);
        REALTYPE *cP1   = _;
        REALTYPE *cP2   = matrix._;
        while(kk--){
            REALTYPE *currD  = result._;
            REALTYPE *currP1 = cP1;
            unsigned int len1 = column;
            while(len1--){
                REALTYPE *currP2 = cP2;
                unsigned int len2 = matrix.column;
                while(len2--){
                    *(currD++) += *(currP1) *(*(currP2++));
                }
                currP1++;
            }
            cP2 += matrix.column;
            cP1 += column;
        }
        return result;
    }

    inline Matrix& MultTranspose2(const Matrix &matrix) const{
        Matrix result;
        return MultTranspose2(matrix,result);
    }
    /// Transpose then Multiply by the matrix :
    // result = this^T * matrix (avoiding a useless call to .Transpose() )
    inline Matrix& MultTranspose2(const Matrix &matrix, Matrix &result) const
    {
        result.Resize(row,matrix.row,false);
        //[a0 a1 .. an] [b0 b1 .. bn]
        if(column==matrix.column){
            REALTYPE *cP1   = _;
            REALTYPE *currD = result._;
            unsigned int len1 = row;
            while(len1--){
                REALTYPE *currP2 = matrix._;
                unsigned int len2 = matrix.row;
                while(len2--){
                    REALTYPE *currP1 = cP1;
                    REALTYPE sum=0.0;
                    unsigned int len = column;
                    while(len--)
                        sum += *(currP1++) * (*(currP2++));
                    *(currD++) = sum;
                }
                cP1 += column;
            }
        }else{
            REALTYPE *cP1   = _;
            REALTYPE *currD = result._;
            unsigned int len1 = row;
            const unsigned int kk = (column<=matrix.column?column:matrix.column);
            unsigned int off2 = matrix.column - kk;
            while(len1--){
                REALTYPE *currP2 = matrix._;
                unsigned int len2 = matrix.row;
                while(len2--){
                    REALTYPE *currP1 = cP1;
                    REALTYPE sum=0.0;
                    unsigned int len = kk;
                    while(len--)
                        sum += *(currP1++) * (*(currP2++));
                    *(currD++) = sum;
                    currP2 += off2;
                }
                cP1 += column;
            }
        }
        return result;
    }

    /// Set a diagonal matrix given a vector of diagonal elements
    inline Matrix& Diag(const Vector &vector)
    {
        Resize(vector.row,vector.row,false);
        Zero();
        REALTYPE *src = vector._;
        REALTYPE *dst = _;
        unsigned int len = row;
        while(len--){
            *dst = *(src++);
            dst += column+1;
        }
        return *this;
    }

    /// Set a random matrix with value uniformly distributed between 0 and 1
    inline Matrix& Random(){
        REALTYPE *src = _;
        unsigned int len = row*column;
        while(len--){
            *(src++) = RND(R_ONE);
        }
        return *this;
    }

    /// Return the transpose of a matrix
    inline Matrix Transpose() const
    {
        Matrix result;
        return Transpose(result);
    }
    /// Compute the transpose of a matrix
    inline Matrix& Transpose(Matrix &result) const
    {
        result.Resize(column,row,false);
        REALTYPE *src = _;
        REALTYPE *dst = result._;
        unsigned int rowLen = row;
        while(rowLen--){
            REALTYPE *cDst = dst;
            unsigned int colLen = column;
            while(colLen--){
                *cDst = *(src++);
                cDst += row;
            }
            dst++;
        }
        return result;
    }

    /// Compute the self transpose
    inline Matrix& STranspose()
    {
        if(column!=row){
            Matrix tmp;
            Transpose(tmp);
            return Swap(tmp);
        }
        REALTYPE tmp;
        REALTYPE *src = _+1;
        REALTYPE *dst = _+column;
        unsigned int rowLen = row;
        while(rowLen--){
            REALTYPE *cDst = dst;
            unsigned int colLen = rowLen;
            while(colLen--){
                tmp      = *cDst;
                *cDst    = *src;
                *(src++) = tmp;
                cDst += column;
            }
            src+=row-rowLen+1;
            dst+=column+1;
        }
        return *this;
    }

    /// Return the vertical concatenation with another matrix
    inline Matrix VCat(const Matrix& matrix) const
    {
        Matrix result;
        return VCat(matrix,result);
    }
    /// Return the vertical concatenation with another matrix in result
    inline Matrix& VCat(const Matrix& matrix, Matrix & result) const
    {
        if(column == matrix.column){
            result.Resize(row+matrix.row,column,false);
            REALTYPE *src = _;
            REALTYPE *dst = result._;
            unsigned int len = row*column;
            while(len--)
                *(dst++) = *(src++);
            src = matrix._;
            len = matrix.row*column;
            while(len--)
                *(dst++) = *(src++);
        }else if(column < matrix.column){
            result.Resize(row+matrix.row,matrix.column,false);
            REALTYPE *src = _;
            REALTYPE *dst = result._;
            unsigned int off = matrix.column - column;
            unsigned int rowLen = row;
            while(rowLen--){
                unsigned int colLen = column;
                while(colLen--) *(dst++) = *(src++);
                colLen = off;
                while(colLen--) *(dst++) = R_ZERO;
            }
            src = matrix._;
            unsigned int len = matrix.row*matrix.column;
            while(len--)
                *(dst++) = *(src++);
        }else{
            result.Resize(row+matrix.row,column,false);
            REALTYPE *src = _;
            REALTYPE *dst = result._;
            unsigned int len = row*column;
            while(len--)
                *(dst++) = *(src++);
            src = matrix._;
            unsigned int off = column - matrix.column;
            unsigned int rowLen = matrix.row;
            while(rowLen--){
                unsigned int colLen = matrix.column;
                while(colLen--) *(dst++) = *(src++);
                colLen = off;
                while(colLen--) *(dst++) = R_ZERO;
            }
        }
        return result;
    }
    /// Return the vertical concatenation with another matrix
    inline Matrix& SVCat(const Matrix& matrix)
    {
        Matrix result;
        VCat(matrix,result);
        return Swap(result);
    }

    /// Return the horizontal concatenation with another matrix
    inline Matrix HCat(const Matrix& matrix) const
    {
        Matrix result;
        return HCat(matrix,result);
    }
    /// Return the horizontal concatenation with another matrix in result
    inline Matrix& HCat(const Matrix& matrix, Matrix & result) const
    {
        unsigned int kk = MIN(row,matrix.row);
        result.Resize(MAX(row,matrix.row),column+matrix.column,false);
        REALTYPE *src0 = _;
        REALTYPE *src1 = matrix._;
        REALTYPE *dst = result._;
        while(kk--){
            unsigned int len = column;
            while(len--) *(dst++) = *(src0++);
            len = matrix.column;
            while(len--) *(dst++) = *(src1++);
        }
        if(row>matrix.row){
            kk = row - matrix.row;
            while(kk--){
                unsigned int len = column;
                while(len--) *(dst++) = *(src0++);
                len = matrix.column;
                while(len--) *(dst++) = R_ZERO;
            }
        }else if(row<matrix.row){
            kk = matrix.row - row;
            while(kk--){
                unsigned int len = column;
                while(len--) *(dst++) = R_ZERO;
                len = matrix.column;
                while(len--) *(dst++) = *(src1++);
            }
        }
        return result;
    }
    /// Concatenate with another matrix
    inline Matrix& SHCat(const Matrix& matrix)
    {
        Matrix result;
        HCat(matrix,result);
        return Swap(result);
    }

    /**
    * \brief Inverse the matrix (One should check, using IsInverseOk() if the inversion succeeded)
    * \param determinant If not NULL, allows to get the determinant of the matrix
    * \return         The resulting inverse matrix
    */
    inline Matrix Inverse(REALTYPE *determinant=NULL) const
    {
        Matrix result;
        return Inverse(result,determinant);
    }

    /**
     * \brief Inverse the matrix (One should check, using IsInverseOk() if the inversion succeeded)
     * \param result        A reference to the resulting inverse matrix
     * \param determinant   If not NULL, allows to get the determinant of the matrix
     * \param baseDetFactor Don't touch this param, please...
     * \return         The resulting inverse matrix
     */
    inline Matrix& Inverse(Matrix &result, REALTYPE *determinant=NULL, REALTYPE baseDetFactor = R_ONE, Matrix *work=NULL) const
    {
        bInverseOk = TRUE;
        if(row==column){ // Square matrix

            Matrix *MM = work;
            if(work==NULL)  MM = new Matrix(*this);
            else            MM->Set(*this);

            if(determinant!=NULL) *determinant = baseDetFactor;
            result.Resize(row,column,false);
            const unsigned int n = row;
            result.Identity();

            REALTYPE *pivotPtr = MM->_;
            unsigned int ioffset = 0;
            for(unsigned int i=0;i<n;i++){
                REALTYPE pivot = *pivotPtr; // MM->_[i*column+i];
                if(fabs(pivot)<=EPSILON){
                    REALTYPE *pivotPtrTmp = pivotPtr + column;
                    for(unsigned int j=i+1;j<n;j++){
                        if(fabs(pivot = *pivotPtrTmp)<=EPSILON){
                            MM->SwapRow(i,j);
                            result.SwapRow(i,j);
                            break;
                        }
                        pivotPtrTmp  += column;
                    }
                    if(fabs(pivot)<=EPSILON){
                        bInverseOk = FALSE;
                        if(determinant!=NULL) *determinant = R_ZERO;
                        return result;
                    }
                }
                if(determinant!=NULL) *determinant *= pivot;
                pivot = R_ONE/pivot;
                REALTYPE *mmPtr  = MM->_    + ioffset;
                REALTYPE *resPtr = result._ + ioffset;
                unsigned int len = n;
                while(len--){
                    *(mmPtr++)  *= pivot;
                    *(resPtr++) *= pivot;
                }
                unsigned int koffset = 0;
                for(unsigned int k=0;k<n;k++){
                    if(k!=i){

                        const REALTYPE mki = *(MM->_ + koffset+i);

                        REALTYPE *mmPtr1  = MM->_    + koffset;
                        REALTYPE *resPtr1 = result._ + koffset;
                        REALTYPE *mmPtr2  = MM->_    + ioffset;
                        REALTYPE *resPtr2 = result._ + ioffset;
                        unsigned int len = n;
                        while(len--){
                            *(mmPtr1++)  -= *(mmPtr2++)  *mki;
                            *(resPtr1++) -= *(resPtr2++) *mki;
                        }
                    }
                    koffset += column;
                }
                pivotPtr += column+1;
                ioffset  += column;
            }
            if(work==NULL) delete MM;
        }else{ // Moore-Penrose pseudo inverse
            if(determinant!=NULL) *determinant = R_ZERO;
            if(row>column){ // (JtJ)^(-1)Jt
                Matrix SQ;
                TransposeMult(*this,SQ);
                SQ.SInverseSymmetric();
                SQ.MultTranspose2(*this,result);
            }else{ // Jt(JJt)^(-1)
                Matrix SQ;
                MultTranspose2(*this,SQ);
                SQ.SInverseSymmetric();
                TransposeMult(SQ,result);
            }
        }
        return result;
    }

    /// After an inverse operation, tell if it was a success
    inline static int IsInverseOk(){
        return bInverseOk;
    }

    /// Echange two row of the matrix
    inline Matrix& SwapRow(unsigned int j1, unsigned int j2){
        if((j1<row)&&(j2<row)){
            REALTYPE tmp;
            REALTYPE *src = _ + j1*column;
            REALTYPE *dst = _ + j2*column;
            unsigned int len = column;
            while(len--){
                tmp      = *src;
                *(src++) = *dst;
                *(dst++) = tmp;
            }
        }
        return *this;
    }

    /// Echange two columns of the matrix
    inline Matrix& SwapColumn(unsigned int i1, unsigned int i2){
        if((i1<column)&&(i2<column)){
            REALTYPE tmp;
            REALTYPE *src = _ + i1;
            REALTYPE *dst = _ + i2;
            unsigned int len = row;
            while(len--){
                tmp     = *src;
                *src    = *dst;
                *dst    = tmp;
                src += column;
                dst += column;
            }
        }
        return *this;
    }

    /// Do the square root on each matrix element
    inline Matrix& Sqrt(){
        REALTYPE *src = _;
        unsigned int len = row*column;
        while(len--){
            *(src) = sqrt(fabs(*src));
            src++;
        }
        return *this;
    }
    /*
    /// Print the matrix of stdout
    void Print() const
    {
        std::cout << "Matrix " <<row<<"x"<<column<<std::endl;;
        REALTYPE *ptr = _;
        unsigned int rowLen = row;
        while(rowLen--){
            unsigned int colLen = column;
            while(colLen--){
                REALTYPE val = *(ptr++);
                if(fabs(val)<=EPSILON) val = R_ZERO;
                std::cout << val <<" ";
            }
            std::cout << "\n";
        }
    }
*/
    /// Print the matrix of stdout
    void Print() const;
    void Print(string name) const;

    friend std::ostream& operator<<(std::ostream& out, const Matrix & a){
        PRINT_BEGIN(out);

        for (unsigned int j = 0; j < a.RowSize(); j++){
            for (unsigned int i = 0; i < a.ColumnSize(); i++){
                out.width(PRINT_WIDTH);
                out << a.AtNoCheck(j,i)<<" ";
            }
            //out <<"; ";
        }

        PRINT_END(out);

        return out;
    }

    /*{
    std::cout<<*this;
  }*/

    /**
   * \brief Do a QR Decomposition of the matrix, where A=QR, and Q is a base and ortonornal matrix and R a triangular matrix
   * \param Q The resulting Q
   * \param R The resulting R
   */
    void QRDecomposition(Matrix & Q, Matrix & R){
        Matrix QR;
        QRDecomposition(Q,R,QR);
    }

    /**
   * \brief Do a QR Decomposition of the matrix, where A=QR, and Q is a base and ortonornal matrix and R a triangular matrix
   * \param Q The resulting Q
   * \param R The resulting R
   * \param QR A temporary processing matrix
   */
    void QRDecomposition(Matrix & Q, Matrix & R, Matrix & QR){
        if(row>=column){
            QR = *this;
        }else{
            Transpose(QR);
        }
        unsigned int m = QR.row;
        unsigned int n = QR.column;
        Vector RDiag(n);

        for(unsigned int k=0;k<n;k++){
            REALTYPE nrm = R_ZERO;
            for (unsigned int i = k; i < m; i++) {
                nrm = hypot_s(nrm, QR(i,k));
            }
            if (nrm != R_ZERO) {
                if(QR(k,k)<R_ZERO){
                    nrm = -nrm;
                }
                for (unsigned int i = k; i < m; i++) {
                    QR(i,k) /= nrm;
                }
                QR(k,k)+=R_ONE;

                for (unsigned int j = k + 1; j < n; j++) {
                    REALTYPE s = R_ZERO;
                    for (unsigned int i = k; i < m; i++) {
                        s += QR(i,k) * QR(i,j);
                    }
                    s = -s / QR(k,k);
                    for (unsigned int i = k; i < m; i++) {
                        QR(i,j) += s * QR(i,k);
                    }
                }
            }
            RDiag(k) = -nrm;
        }

        R.Resize(n,n);
        for(unsigned int i = 0; i < n; i++) {
            for(unsigned int j = 0; j < n; j++) {
                if(i<j){
                    R(i,j) = QR(i,j);
                }else if(i==j){
                    R(i,j) = RDiag(i);
                }else{
                    R(i,j) = R_ZERO;
                }
            }
        }

        Q.Resize(m,n);
        for(int k= n-1;k>=0;k--){
            for(unsigned int i = 0; i < m; i++) {
                Q(i,k) = R_ZERO;
            }
            Q(k,k)=R_ONE;
            for(unsigned int j = k; j < n; j++) {
                if(QR(k,k)!=R_ZERO){
                    REALTYPE s = R_ZERO;
                    for(unsigned int i = k; i < m; i++) {
                        s += QR(i,k) * Q(i,j);
                    }
                    s = -s / QR(k,k);
                    for(unsigned int i = k; i < m; i++) {
                        Q(i,j) = Q(i,j) + s*QR(i,k);
                    }
                }
            }
        }
    }


    /**
     * \brief Perform a one-shot eigen values and vectors decomposition
     * \param eigenvalues  the resulting eigen values
     * \param eigenVectors the resulting eigen vectors
     * \param maxIter      the maximum number of iteration
     */
    void EigenValuesDecomposition(Vector &eigenValues, Matrix& eigenVectors,int maxIter = 30) const{
        Matrix tri;
        Tridiagonalize(tri,eigenVectors);
        tri.TriEigen(eigenValues, eigenVectors, maxIter);
    }
    void EigenValuesDecomposition2(Vector &eigenValues, Matrix& eigenVectors,int maxIter = 30) const{
        Matrix tri;
        Tridiagonalize2(tri,eigenVectors);
        tri.TriEigen2(eigenValues, eigenVectors, maxIter);
    }



    /**
     * \brief Do a compressed tridiagonalization of the matrix (Use TriDiag() to recover a normal form of the matrix)
     * \param result The resulting matrix. It has three rows, one for the diagonal element, and the others of the up and down diagonal elements
     * \param trans  The resulting transformation matrix
     */
    Matrix& Tridiagonalize(Matrix &result, Matrix &trans) const{
        result.Resize(2,row,false);
        if(row==0) return result;

        trans.Set(*this);

        int n = row;
        int l,k,j,i;
        REALTYPE scale,hh,h,g,f;
        for(i=n-1;i>=1;i--){
            l = i-1;
            h = scale = R_ZERO;
            if(l>0){
                for(k=0;k<=l;k++)
                    scale += fabs(trans._[i*column+k]);
                if(scale == R_ZERO){
                    result._[column+i] = trans._[i*column+l];
                }else{
                    for(k=0;k<=l;k++){
                        trans._[i*column+k] /= scale;
                        h += trans._[i*column+k]*trans._[i*column+k];
                    }
                    f= trans._[i*column+l];
                    g=(f>=R_ZERO?-sqrt(h):sqrt(h));
                    result._[column+i] = scale*g;
                    h-=f*g;
                    trans._[i*column+l] = f-g;
                    f=R_ZERO;
                    for(j=0;j<=l;j++){
                        trans._[j*column+i] = trans._[i*column+j] /h;
                        g=R_ZERO;
                        for(k=0;k<=j;k++)
                            g+=  trans._[j*column+k]*trans._[i*column+k];
                        for(k=j+1;k<=l;k++)
                            g+=  trans._[k*column+j]*trans._[i*column+k];
                        result._[column+j] = g/h;
                        f+= result._[column+j]*trans._[i*column+j];
                    }
                    hh = f/(h+h);
                    for(j=0;j<=l;j++){
                        f = trans._[i*column+j];
                        result._[column+j]=g=result._[column+j]-hh*f;
                        for(k=0;k<=j;k++)
                            trans._[j*column+k] -= (f*result._[column+k]+g*trans._[i*column+k]);
                    }
                }
            }else{
                result._[column+i] = trans._[i*column+l];
            }
            result._[i]=h;
        }
        result._[0]=R_ZERO;
        result._[column+0]=R_ZERO;
        for(i=0;i<n;i++){
            l=i-1;
            if(result._[i]){
                for(j=0;j<=l;j++){
                    g=R_ZERO;
                    for(k=0;k<=l;k++)
                        g+= trans._[i*column+k]*trans._[k*column+j];
                    for(k=0;k<=l;k++)
                        trans._[k*column+j] -= g*trans._[k*column+i];
                }
            }
            result._[i] = trans._[i*column+i];
            trans._[i*column+i] = R_ONE;
            for(j=0;j<=l;j++) trans._[j*column+i]=trans._[i*column+j]=R_ZERO;
        }
        return result;
    }

    /**
     * \brief Do a compressed tridiagonalization of the matrix (Use TriDiag() to recover a normal form of the matrix)
     * \param result The resulting matrix. It has three rows, one for the diagonal element, and the others of the up and down diagonal elements
     * \param trans  The resulting transformation matrix
     */
    Matrix& Tridiagonalize2(Matrix &result, Matrix &trans) const{
        result.Resize(2,row,false);
        if(row==0) return result;

        trans.Set(*this);

        int n = row;
        int l,j,i;
        REALTYPE scale,hh,h,g,f;
        REALTYPE *cTrans_i = trans._ + (row-1)*column;
        i = n;
        while(--i){
            l = i-1;
            h = scale = R_ZERO;
            if(l>0){
                REALTYPE *cSrc = cTrans_i;
                unsigned int len = i;
                while(len--){
                    scale += fabs(*(cSrc++));
                }
                if(scale == R_ZERO){
                    result._[column+i] = *(cTrans_i+l);
                }else{
                    REALTYPE oneOnScale = R_ONE/scale;
                    cSrc = cTrans_i;
                    len = i;
                    while(len--){
                        *cSrc *= oneOnScale;
                        h += (*cSrc) * (*cSrc);
                        cSrc++;
                    }
                    f= *(cTrans_i+l);
                    g=(f>=R_ZERO?-sqrt(h):sqrt(h));
                    result._[column+i] = scale*g;
                    h-=f*g;
                    *(cTrans_i+l) = f-g;
                    f=R_ZERO;
                    REALTYPE oneOnH = R_ONE / h;
                    REALTYPE *cDst = trans._ + i;
                    REALTYPE *cSrc = cTrans_i;
                    REALTYPE *cTrans_j = trans._;
                    for(j=0;j<i;j++){
                        *cDst = *cSrc * oneOnH;
                        g=R_ZERO;
                        REALTYPE *cSrc1 = cTrans_i;
                        REALTYPE *cSrc2 = cTrans_j;
                        len = j+1;
                        while(len--)
                            g += *(cSrc1++) * (*(cSrc2++));

                        if(j+1<=l){
                            cSrc2 = cTrans_j + column + j;
                            len =  l-j;
                            while(len--){
                                g+= *cSrc2 * (*(cSrc1++));
                                cSrc2 += column;
                            }
                        }
                        result._[column+j] = g * oneOnH;
                        f += result._[column+j] * (*cSrc);
                        cSrc++;
                        cDst += column;
                        cTrans_j += column;
                    }

                    hh = f/(h+h);
                    cSrc = cTrans_i;
                    cTrans_j = trans._;
                    for(j=0;j<i;j++){
                        f = *(cSrc++);
                        result._[column+j] = g = result._[column+j] - hh * f;
                        cDst = cTrans_j;
                        REALTYPE *cSrc2 = cTrans_i;
                        REALTYPE *cSrc1 = result._+column;
                        len = j+1;
                        while(len--)
                            *(cDst++) -= (f* (*(cSrc1++))+g* (*cSrc2++));
                        cTrans_j += column;
                    }
                }
            }else{
                result._[column+i] = *(cTrans_i+l);
            }
            result._[i] = h;
            cTrans_i -= column;
        }

        result._[0]=R_ZERO;
        result._[column+0]=R_ZERO;
        cTrans_i = trans._;
        for(i=0;i<n;i++){
            if(result._[i]){
                for(j=0;j<i;j++){
                    g=R_ZERO;
                    REALTYPE *currSrc1 = cTrans_i;
                    REALTYPE *currSrc2 = trans._ +j;
                    unsigned int len = i;
                    while(len--){
                        g += *(currSrc1++) * (*currSrc2);
                        currSrc2 += column;
                    }
                    len = i;
                    REALTYPE *currDst = trans._ +j;
                    REALTYPE *currSrc = trans._ +i;
                    while(len--){
                        *currDst -= g * (*currSrc);
                        currSrc += column;
                        currDst += column;
                    }
                }
            }

            REALTYPE *cDst1 = trans._ + i;
            REALTYPE *cDst2 = cTrans_i;
            unsigned int len = i;
            while(len--){
                *cDst1 = *(cDst2++) = R_ZERO;
                cDst1 += column;
            }
            result._[i] = *cDst2;
            *cDst2 = R_ONE;

            cTrans_i += column;
        }
        return result;
    }

    /**
     * \brief Produce a tridiagnal matrix from the compressed tridiagonal matrix resulting from Tridiaglonalize()
     * \param tri The compressed tridiagonal matrix
     */
    Matrix& TriDiag(const Matrix &tri){
        Resize(tri.column,tri.column,false);
        /*
        if(column>0){
            Zero();
            REALTYPE *cDst = _;
            REALTYPE *cDst = _;
            *cDst = tri._[i];
            cDst += column+1;
            unsigned int len = column-2;
            while(len--){
                *cDst
                cDst += column+1;
            }
        */
        for(unsigned int i=0;i<column;i++){
            _[i*(column+1)] = tri._[i];
            if(i<column-1)
                _[i*(column+1)+1] = tri._[column+i+1];
            if(i>0)
                _[i*(column+1)-1] = tri._[column+i];
        }
        return *this;
    }

    /**
    * \brief Compute the eigen values and eigen vectors from a compressed tridiagonal matrix resulting from Tridiaglonalize()
    * \param eigenvalues  the resulting eigern values
    * \param eigenVectors the resulting eigen vectors
    */
    int TriEigen(Vector &eigenValues, Matrix& eigenVectors,int maxIter = 120) const{
        bInverseOk = true;

        if(row!=2) return -1;
        if(column==0) return -1;

        GetRow(0,eigenValues);
        Vector e;
        GetRow(1,e);

        const int n = column;
        int m,l,iter,i,k;
        REALTYPE s,r,p,g,f,dd,c,b;
        int cumIter = 0;
        for(i=1;i<n;i++)
            e._[i-1] = e._[i];
        e._[n-1] = R_ZERO;

        for(l=0;l<n;l++){
            iter=0;
            do{
                for(m=l;m<=n-2;m++){
                    dd = fabs(eigenValues._[m])+fabs(eigenValues._[m+1]);
                    if((REALTYPE)(fabs(e._[m])+dd) == dd) break;
                }
                if(m!=l){
                    if(iter++==maxIter) {
                        bInverseOk = false;
                        break;
                    }
                    g = (eigenValues._[l+1]-eigenValues._[l])/(2.0f*e[l]);
                    r = hypot_s(g,R_ONE);
                    g = eigenValues._[m]-eigenValues._[l]+e._[l]/(g+SIGN2(r,g));
                    s = c = R_ONE;
                    p = R_ZERO;
                    for(i=m-1;i>=l;i--){
                        f        = s*e._[i];
                        b        = c*e._[i];
                        e._[i+1] = (r=hypot_s(f,g));
                        if(r==R_ZERO){
                            eigenValues._[i+1] -= p;
                            e._[m]              = R_ZERO;
                            break;
                        }
                        s = f/r;
                        c = g/r;
                        g = eigenValues._[i+1]-p;
                        r = (eigenValues._[i]-g)*s+2.0f*c*b;
                        eigenValues._[i+1] = g+(p=s*r);
                        g = c*r-b;
                        for(k=0;k<n;k++){
                            f                       = eigenVectors._[k*n+i+1];
                            eigenVectors._[k*n+i+1] = s * eigenVectors._[k*n+i] + c*f;
                            eigenVectors._[k*n+i]   = c * eigenVectors._[k*n+i] - s*f;
                        }
                    }
                    if((r==R_ZERO)&&(i>=0))
                        continue;
                    eigenValues._[l] -= p;
                    e._[l] = g;
                    e._[m] = R_ZERO;
                }
            }while(m!=l);
            cumIter+=iter;
        }
        if(!bInverseOk){
            fprintf(stderr,"Error: too many ierations...%f/%d\n",REALTYPE(cumIter)/REALTYPE(n),maxIter);
        }
        return cumIter;
    }

    int TriEigen2(Vector &eigenValues, Matrix& eigenVectors,int maxIter = 120) const{
        bInverseOk = true;

        if(row!=2) return -1;
        if(column==0) return -1;

        GetRow(0,eigenValues);
        Vector e;
        GetRow(1,e);

        const int n = column;
        int m,l,iter,i,k;
        REALTYPE s,r,p,g,f,dd,c,b;
        int cumIter = 0;

        REALTYPE *cDst = e._;
        unsigned int len=column-1;
        while(len--){
            *(cDst) = *(cDst+1);
            cDst++;
        }*cDst = R_ZERO;
        //for(i=1;i<n;i++)
        //    e._[i-1] = e._[i];
        //e._[n-1] = R_ZERO;

        REALTYPE *cEigVal_l = eigenValues._;
        for(l=0;l<n;l++){
            iter=0;
            do{
                REALTYPE *currEigVal = cEigVal_l;
                REALTYPE currFabs1 = fabs(*currEigVal);
                for(m=l;m<=n-2;m++){
                    REALTYPE currFabs2 = fabs(*(++currEigVal));
                    dd = currFabs1 + currFabs2;
                    currFabs1 = currFabs2;
                    if((fabs(e._[m]) + dd) == dd) break;
                }
                /*for(m=l;m<=n-2;m++){
                    dd = fabs(eigenValues._[m])+fabs(eigenValues._[m+1]);
                    if((REALTYPE)(fabs(e._[m])+dd) == dd) break;
                }*/
                if(m!=l){
                    if(iter++==maxIter) {
                        bInverseOk = false;
                        break;
                    }
                    g = (eigenValues._[l+1] - eigenValues._[l])/(2.0 * e[l]);
                    r = hypot_s(g,R_ONE);
                    g = eigenValues._[m] - eigenValues._[l]+e._[l]/(g+SIGN2(r,g));
                    s = c = R_ONE;
                    p = R_ZERO;
                    for(i=m-1;i>=l;i--){
                        REALTYPE ei = e._[i];
                        f        = s * ei;
                        b        = c * ei;
                        e._[i+1] = (r=hypot_s(f,g));
                        if(r==R_ZERO){
                            eigenValues._[i+1] -= p;
                            e._[m]              = R_ZERO;
                            break;
                        }
                        REALTYPE oneOnR = R_ONE / r;
                        s = f * oneOnR;
                        c = g * oneOnR;
                        g = eigenValues._[i+1]-p;
                        r = (eigenValues._[i]-g) * s+ 2.0 * c * b;
                        eigenValues._[i+1] = g+(p=s*r);
                        g = c*r-b;

                        REALTYPE *cPtr = eigenVectors._ + i;
                        k = n;
                        while(k--){
                            f                       =     (*(cPtr+1));
                            *(cPtr+1)               = s * (*(cPtr  )) + c*f;
                            *(cPtr  )               = c * (*(cPtr  )) - s*f;
                            cPtr += n;
                        }
                    }
                    if((r==R_ZERO)&&(i>=0))
                        continue;
                    eigenValues._[l] -= p;
                    e._[l]            = g;
                    e._[m]            = R_ZERO;
                }
            }while(m!=l);
            cumIter+=iter;

            cEigVal_l++;
        }
        if(!bInverseOk){
            fprintf(stderr,"Error: too many ierations...%f/%d\n",REALTYPE(cumIter)/REALTYPE(n),maxIter);
        }
        return cumIter;
    }





    Matrix& Cholesky(Matrix &result) const {
        result.Set(*this);
        return result.SCholesky();
    }

    Matrix& SCholesky() {
        if(row==column){ // Square matrix
            bInverseOk = TRUE;

            REALTYPE A_00, R_00;

            A_00 = _[0];
            if(A_00 <= R_ZERO){
                bInverseOk = FALSE;
                return *this;
            }
            _[0] = R_00 = sqrt(A_00);

            if(row > 1) {
                REALTYPE A_01 = _[column+0];
                REALTYPE A_11 = _[column+1];
                REALTYPE R_01;
                REALTYPE diag;
                REALTYPE sum;

                R_01 = A_01 / R_00;
                diag = A_11 - R_01 * R_01;

                if(diag<=R_ZERO){
                    bInverseOk = FALSE;
                    return *this;
                }

                _[column+0] = R_01;
                _[column+1] = sqrt(diag);

                REALTYPE *kPtr = _ + (column<<1);
                for(unsigned int k=2; k<row; k++) {
                    REALTYPE *iPtr = _;
                    for(unsigned int i=0; i<k; i++) {
                        sum = R_ZERO;
                        REALTYPE *ckPtr = kPtr;
                        REALTYPE *ciPtr = iPtr;
                        unsigned int len = i;
                        while(len--){
                            sum += *(ciPtr++) * (*(ckPtr++));
                        }
                        *ckPtr = (*ckPtr - sum) / (*ciPtr);
                        iPtr += column;
                    }

                    sum = R_ZERO;
                    REALTYPE *ckPtr = kPtr;
                    unsigned int len = k;
                    while(len--){
                        sum += *ckPtr * (*ckPtr);
                        ckPtr++;
                    }
                    diag = *ckPtr - sum;
                    if(diag <= R_ZERO) {
                        bInverseOk = FALSE;
                        return *this;
                    }
                    *ckPtr = sqrt(diag);

                    kPtr += column;
                }
            }
            REALTYPE *ptr = _ +1;
            unsigned int rowLen = row-1;
            while(rowLen--){
                unsigned int colLen = rowLen+1;
                while(colLen--)
                    *(ptr++) = R_ZERO;
                ptr += row-rowLen;
            }
        }else{
            bInverseOk = FALSE;
        }
        return *this;
    }

    Matrix& InverseLowerTriangular(Matrix& result) const{
        result.Set(*this);
        return result.SInverseLowerTriangular();
    }
    Matrix& SInverseLowerTriangular(){
        if(row==column){ // Square matrix
            bInverseOk = TRUE;

            REALTYPE *cDst = _;
            for(unsigned int i=0;i<row;i++){
                REALTYPE *currDst = cDst;
                REALTYPE *currSrc = _;
                for(unsigned int j=0;j<i;j++){
                    *currDst = -(*currDst) *(*currSrc);
                    REALTYPE *currSrc1 = cDst + j + 1;
                    REALTYPE *currSrc2 = _ + (j+1)*column + j;
                    unsigned int len = i-j-1;
                    while(len--){
                        *currDst -= (*currSrc1++) * (*currSrc2);
                        currSrc2 += column;
                    }
                    currDst++;
                    currSrc += column+1;
                }
                REALTYPE pivot = R_ONE / (*(cDst+i));
                if(fabs(pivot)<=EPSILON) {bInverseOk = FALSE; return *this;}

                currDst = cDst;
                unsigned int len = i;
                while(len--) *(currDst++) *= pivot;
                *currDst = pivot;
                cDst += column;
            }
        }else{
            bInverseOk = FALSE;
        }
        return *this;
    }

    Matrix& InverseSymmetric(Matrix & result,
                             REALTYPE * determinant=NULL) const{
        result.Set(*this);
        return result.SInverseSymmetric(determinant);
    }
    Matrix& SInverseSymmetric(REALTYPE * determinant=NULL){
        if(row==column){ // Square matrix
            bInverseOk = TRUE;

            SCholesky();
            if(determinant){
                REALTYPE det = R_ONE;
                REALTYPE *ptr = _;
                unsigned int len = row;
                while(len--){
                    det *= *(ptr++);
                    ptr += column;
                }
                *determinant = det * det;
            }
            SInverseLowerTriangular();

            // Performs (L^T)^(-1) * (L)^(-1)
            REALTYPE *cDst  = _;

            REALTYPE *cSrc1  = _;
            REALTYPE *cSrc1o = cSrc1;
            REALTYPE *cSrc2  = cSrc1;
            REALTYPE *cSrc2o = cSrc1;
            unsigned int colLen = column;
            while(colLen){
                cSrc1 = cSrc1o;
                cSrc2 = cSrc2o;
                unsigned int rowLen = colLen;
                while(rowLen){
                    REALTYPE *currSrc1 = cSrc1;
                    REALTYPE *currSrc2 = cSrc2;
                    unsigned int kk = rowLen;
                    REALTYPE sum = R_ZERO;
                    while(kk--){
                        sum += *(currSrc1) * (*currSrc2);
                        currSrc1 += column;
                        currSrc2 += column;
                    }
                    *cDst++  = sum;
                    cSrc2 += column + 1;
                    cSrc1 += column;
                    rowLen--;
                }
                cSrc2o += column+1;
                cSrc1o += column+1;
                colLen--;
                cDst  += column-colLen;
            }

            REALTYPE *src = _+1;
            REALTYPE *dst = _+column;
            unsigned int rowLen = row;
            while(rowLen--){
                REALTYPE *cDst = dst;
                unsigned int colLen = rowLen;
                while(colLen--){
                    *cDst = *(src++);
                    cDst += column;
                }
                src+=row-rowLen+1;
                dst+=column+1;
            }
        }else{
            bInverseOk = FALSE;
        }
        return *this;
    }


    /// Sort the column of the matrix according to the indices of the input vector elements sorted according to thir abolut value
    Matrix& SortColumnAbs(Vector & values){
        const int k = (values.Size()<column?values.Size():column);
        REALTYPE cmax;
        int maxId;
        for(int i=0;i<k-1;i++){
            cmax  = fabs(values._[i]);
            maxId = i;
            for(int j=i+1;j<k;j++){
                if(cmax<fabs(values._[j])){
                    cmax = fabs(values._[j]);
                    maxId = j;
                }
            }
            if(maxId!=i){
                REALTYPE tmp       = values._[i];
                values._[i]     = values._[maxId];
                values._[maxId] = tmp;
                SwapColumn(i,maxId);
            }
        }
        return *this;
    }

    Matrix& SquareRoot(Matrix& result){
        Vector eigValD;
        Matrix eigVal;
        Matrix eigVec;
        EigenValuesDecomposition(eigValD, eigVec);
        for(unsigned int i=0;i<eigValD.Size();i++) eigValD(i) = sqrt(eigValD(i));
        eigVal.Diag(eigValD);
        eigVec.Mult(eigVal,result);
        eigVal = result;
        eigVec.STranspose();
        eigVal.Mult(eigVec,result);
        return result;
    }

    Matrix& InverseSquareRoot(Matrix& result){
        Vector eigValD;
        Matrix eigVal;
        Matrix eigVec;
        EigenValuesDecomposition(eigValD, eigVec);
        for(unsigned int i=0;i<eigValD.Size();i++) eigValD(i) = sqrt(1.0/eigValD(i));
        eigVal.Diag(eigValD);
        eigVec.Mult(eigVal,result);
        eigVal = result;
        eigVec.STranspose();
        eigVal.Mult(eigVec,result);
        return result;
    }

    /// Do a Gram-Schmidt ortonormalization of the matrix  but an extra column (the base) is added. Using RemoveZeroColumn may then help clean the matrix.
    Matrix& GramSchmidt(Vector &base){
        Matrix unit(row,1);
        unit.SetColumn(base,0);
        Matrix ext;
        unit.HCat(*this,ext);
        ext.GramSchmidt();
        (*this) = ext;
        return *this;
    }

    /// Do a Gram-Schmidt ortonormalization of the matrix
    Matrix& GramSchmidt(){
        Vector res(row),tmp(row),tmp2(row),tmp3(row);
        for(unsigned int i=0;i<column;i++){
            GetColumn(i,tmp);
            res = tmp;
            for(unsigned int j=0;j<i;j++){
                GetColumn(j,tmp2);
                res-=tmp2.Mult((tmp2.Dot(tmp)),tmp3);
            }
            REALTYPE norm = res.Norm();
            if(norm>EPSILON){
                res /= norm;
            }else{
                res.Zero();
            }
            SetColumn(res,i);
        }
        return *this;
    }

    /// Remove the columns of the matrix being zero
    Matrix& RemoveZeroColumns(){
        int zeroCnt = 0;
        int colCnt  = 0;
        while(colCnt < int(column)-zeroCnt){

            bool bIsZero = true;
            for(unsigned int j=0;j<row;j++){
                if(fabs(_[j*column+colCnt])>EPSILON){
                    bIsZero = false;
                    break;
                }
            }
            if(bIsZero){
                if(colCnt<int(column)-1-zeroCnt){
                    SwapColumn(colCnt,int(column)-1-zeroCnt);
                }
                zeroCnt++;
            }else{
                colCnt++;
            }
        }
        Resize(row,column-zeroCnt,true);
        return *this;
    }


    REALTYPE Trace(){
        REALTYPE sum = R_ZERO;
        REALTYPE *src = _;
        unsigned int len = MIN(row,column);
        while(len--){
            sum += *src;
            src += column+1;
        }
        return sum;
    }

    /// Max matrix value
    REALTYPE Max(){
        REALTYPE *src = _;
        REALTYPE mmax = *(src++);
        unsigned int len = row*column-1;
        while(len--){
            REALTYPE val = *(src++);
            if(mmax< val)
                mmax = val;
        }
        return mmax;
    }

    /// Min matrix value
    REALTYPE Min(){
        REALTYPE *src = _;
        REALTYPE mmin = *(src++);
        unsigned int len = row*column-1;
        while(len--){
            REALTYPE val = *(src++);
            if(mmin > val)
                mmin = val;
        }
        return mmin;
    }

    /// Sum each element
    REALTYPE Sum(){
        REALTYPE sum = R_ZERO;
        REALTYPE *src = _;
        unsigned int len = row*column;
        while(len--){
            sum += *(src++);
        }
        return sum;
    }

    /// Sum each column and give the result in a vector
    Vector SumColumn(){
        Vector result(row);
        return SumColumn(result);
    }

    /// Sum each column and give the result in a vector
    Vector & SumColumn(Vector & result){
        result.Resize(row,false);
        REALTYPE *src = _;
        REALTYPE *dst = result._;
        unsigned int rowLen = row;
        while(rowLen--){
            REALTYPE sum = R_ZERO;
            unsigned int colLen = column;
            while(colLen--){
                sum += *(src++);
            }
            *(dst++) = sum;
        }
        return result;
    }

    /// Sum each row and give the result in a vector
    Vector SumRow(){
        Vector result(column);
        return SumRow(result);
    }

    /// Sum each row and give the result in a vector
    Vector & SumRow(Vector & result){
        result.Resize(column,false);
        result.Zero();
        REALTYPE *src = _;
        unsigned int rowLen = row;
        while(rowLen--){
            REALTYPE *dst = result._;
            unsigned int colLen = column;
            while(colLen--){
                *(dst++) += *(src++);
            }
        }
        return result;
    }

    /// Swap the content of two matrices
    Matrix& Swap(Matrix& matrix){
        unsigned int  rowTmp    = row;
        unsigned int  columnTmp = column;
        REALTYPE     *arrayTmp  = _;
        row                     = matrix.row;
        column                  = matrix.column;
        _                       = matrix._;
        matrix.row              = rowTmp;
        matrix.column           = columnTmp;
        matrix._                = arrayTmp;
        return *this;
    }

    /// Load a matrix from filename
    bool Load(const char* filename);
    /// Save a matrix to filename , if max_row is set, save only the first max_row to file.
    bool Save(const char* filename, unsigned int precision=6,int max_row = -1);
    /// Load a matrix from filename
    bool LoadBinary(const char* filename);
    /// Save a matrix to filename
    bool SaveBinary(const char* filename);

protected:
    string RemoveSpaces(string s);

    inline virtual void Release(){
        if(_!=NULL) delete [] _;
        row    = 0;
        column = 0;
        _      = NULL;
    }
public:
    /// Resize the matrix
    inline virtual Matrix& Resize(unsigned int rowSize, unsigned int colSize, bool copy = true){
        if((row!=rowSize)||(column!=colSize)){
            if((rowSize)&&(colSize)){
                if((!copy)&&(row == colSize)&&(column==rowSize)){
                    row    = rowSize;
                    column = colSize;
                    return *this;
                }
                REALTYPE *arr = new REALTYPE[rowSize*colSize];
                if(copy){
                    unsigned int rowLen = (row   <rowSize?row:   rowSize);
                    if(column==colSize){
                        memcpy(arr,_,rowLen*column*sizeof(REALTYPE));
                    }else if(column<colSize){
                        REALTYPE *src = _;
                        REALTYPE *dst = arr;
                        while(rowLen--){
                            unsigned int colLen = column;
                            while(colLen--) *(dst++) = *(src++);
                            colLen = colSize-column;
                            while(colLen--) *(dst++) = R_ZERO;
                        }
                    }else{
                        REALTYPE *src = _;
                        REALTYPE *dst = arr;
                        while(rowLen--){
                            unsigned int colLen = colSize;
                            while(colLen--) *(dst++) = *(src++);
                            src += column-colSize;
                        }
                    }
                    if(rowSize>row){
                        memset(arr+row*colSize,0,(rowSize-row)*colSize*sizeof(REALTYPE));
                    }
                }
                if(_!=NULL) delete [] _;
                _      = arr;
                row    = rowSize;
                column = colSize;
            }else{
                Release();
            }
        }
        return *this;
    }
};




class SharedMatrix : public Matrix
{
protected:
    unsigned int maxMemorySize;

public:
    inline SharedMatrix():Matrix(){}
    inline SharedMatrix(REALTYPE *array, unsigned int rowSize, unsigned int colSize):Matrix(){
        SetSharedPtr(array,rowSize,colSize);
    }
    inline SharedMatrix(const Matrix & matrix):Matrix(){
        SetSharedPtr(matrix.Array(),matrix.RowSize(),matrix.ColumnSize());
    }
    inline SharedMatrix(const Vector & vector):Matrix(){
        SetSharedPtr(vector.Array(),vector.Size(),1);
    }
#ifdef  USE_T_EXTENSIONS
    /// Copy Contructor of a template Matrix (see TMatrix)
    template<unsigned int ROW> inline SharedMatrix(const TMatrix<ROW> &matrix):Matrix(){
        SetSharedPtr(matrix.Array(),ROW,ROW);
    }
    template<unsigned int ROW> inline SharedMatrix(const TVector<ROW> &vector):Matrix(){
        SetSharedPtr(vector.Array(),ROW,1);
    }
#endif
    inline virtual ~SharedMatrix(){
        Release();
    }

    inline SharedMatrix& SetSharedPtr(REALTYPE *array, unsigned int rowSize, unsigned int colSize){
        row     = rowSize;
        column  = colSize;
        maxMemorySize = rowSize*colSize;
        _       = array;
        return *this;
    }
protected:
    inline virtual void Release(){
        _ = NULL;
    }
    inline virtual SharedMatrix& Resize(unsigned int rowSize, unsigned int colSize, bool copy = true){
        assert(rowSize*colSize<=maxMemorySize);
        row     = rowSize;
        column  = colSize;
        return *this;
    }
};


bool TestClassMatrix();

#ifdef USE_MATHLIB_NAMESPACE
}
#endif
#endif
