#ifndef OPERATOR_TRAITS_HPP
#define OPERATOR_TRAITS_HPP

#include <xmmintrin.h>
#include <emmintrin.h>
#include <pmmintrin.h>
#include <cstdlib>
#include <assert.h>
#include <iostream>
#include <cmath>

namespace hmmlib {

template<typename float_type, typename sse_float_type>
class OperatorTraits {
	
public :
    template<typename Table>
    static inline void reset(float_type val, Table &T) {
		for(int r = 0; r < T.get_no_rows(); ++r)
			for(int c = 0; c < T.get_no_allocated_columns(); ++c)
				T(r, c) = val;
    }

    template<typename Table>
    static inline sse_float_type &
    get_chunk(int row, int chunk, Table &T) {
		return T.table[T.get_no_allocated_columns() * row + chunk];
    }

    template<typename Table>
    static inline Table &
    transpose(Table &transpose, const Table &T) {
		assert(transpose.no_rows == T.no_columns);
		assert(transpose.no_columns == T.no_rows);

		for(int r = 0; r < T.no_rows; ++r)
			for(int c = 0; c < T.no_columns; ++c)
				transpose(c, r) = T(r,c);
		return transpose;
    }

    template<typename Table>
    static inline Table &
    log(Table &log_table, const Table &T) {
		assert(log_table.no_rows == T.no_rows);
		assert(log_table.no_columns == T.no_columns);

		for(int r = 0; r < T.no_rows; ++r)
			for(int c = 0; c < T.no_columns; ++c)
				log_table(r, c) = std::log(T(r,c));
		return log_table;
    }

    template<typename Table>
    static inline Table &
    add(float_type scalar, Table &T) {
		for(int r = 0; r < T.no_rows; ++r)
			for(int c = 0; c < T.no_columns; ++c)
				T(r, c) += scalar;
		return T;
    }

    template<typename Table>
    static inline Table &
    mul(float_type scalar, Table &T) {
		for(int r = 0; r < T.no_rows; ++r)
			for(int c = 0; c < T.no_columns; ++c)
				T(r, c) *= scalar;
		return T;
    }
	
    template<typename Table>
    static inline float_type
    row_sum(int row, Table &T) {
		float_type sum = 0.0;
		for(int c = 0; c < T.no_columns; ++c)
			sum += T(row, c);
		return sum;
    }

    template<typename Table>
    static inline float_type
    column_sum(int column, Table &T) {
		float_type sum = 0.0;
		for(int r = 0; r < T.no_rows; ++r)
			sum += T(r, column);
		return sum;
    }
};

template<>
class OperatorTraits<double, __m128d> {
    typedef double float_type;
    typedef __m128d sse_float_type;

public :
    static const int floats_per_chunk = sizeof(sse_float_type) / sizeof(float_type);

    template<typename Table>
    static inline void 
    reset(float_type val, Table &T) {
		for(int r = 0; r < T.get_no_rows(); ++r) {
			for(int chunk = 0; chunk < T.get_no_chunks_per_row(); ++chunk)
				T.get_chunk(r, chunk) = _mm_set_pd1(val);
		}
    }

    template<typename Table>
    static inline sse_float_type &
    get_chunk(int row, int chunk, Table &T) {
		return *((sse_float_type *) &T(row, chunk * floats_per_chunk));
    }	

    template<typename Table>
    static inline Table &
    transpose(Table &transpose, const Table &T) {
		assert(transpose.no_rows == T.no_columns);
		assert(transpose.no_columns == T.no_rows);

		for(int r = 0; r < T.no_rows; ++r)
			for(int c = 0; c < T.no_columns; ++c)
				transpose(c, r) = T(r,c);
		return transpose;
    }

    template<typename Table>
    static inline Table &
    log(Table &log_table, const Table &T) {
		assert(log_table.no_rows == T.no_rows);
		assert(log_table.no_columns == T.no_columns);

		for(int r = 0; r < T.no_rows; ++r)
			for(int c = 0; c < T.no_columns; ++c)
				log_table(r, c) = std::log(T(r,c));
		return log_table;
    }

    template<typename Table>
    static inline Table &
    add(float_type scalar, Table &T) {
		for(int r = 0; r < T.no_rows; ++r)
			for(int c = 0; c < T. no_chunks_per_row; ++c) {
				sse_float_type scalar_chunk = _mm_set_pd1(scalar);
				T.get_chunk(r,c) += scalar_chunk;
			}
		return T;
    }

    template<typename Table>
    static inline Table &
    mul(float_type scalar, Table &T) {
		for(int r = 0; r < T.no_rows; ++r)
			for(int c = 0; c < T.no_chunks_per_row; ++c) {
				sse_float_type scalar_chunk = _mm_set_pd1(scalar);
				T.get_chunk(r,c) *= scalar_chunk;
			}
		return T;
    }

    template<typename Table>
    static inline float_type
    row_sum(int row, Table &T) {
		sse_float_type sum_chunk = _mm_set_pd1((float_type) 0.0);
		int c;
		for(c = 0; c < T.no_chunks_per_row - 1; ++c)
			sum_chunk += T.get_chunk(row, c);
		sum_chunk = _mm_hadd_pd(sum_chunk, sum_chunk);
		float_type sum = *((float_type *) &sum_chunk);
		for(int i = c*floats_per_chunk; i < T.no_columns; ++i)
			sum += T(row,i);
		return sum;
    }

    template<typename Table>
    static inline float_type
    column_sum(int column, Table &T) {
		float_type sum = 0.0;
		for(int r = 0; r < T.no_rows; ++r)
			sum += T(r, column);
		return sum;
    }
};

template<>
class OperatorTraits<float, __m128> {
    typedef float float_type;
    typedef __m128 sse_float_type;

public :
    static const int floats_per_chunk = sizeof(sse_float_type) / sizeof(float_type);

    template<typename Table>
    static inline void 
    reset(float_type val, Table &T) {
		for(int r = 0; r < T.get_no_rows(); ++r)
			for(int chunk = 0; chunk < T.get_no_chunks_per_row(); ++chunk)
				T.get_chunk(r, chunk) = _mm_set_ps1(val);
    }

    template<typename Table>
    static inline sse_float_type &
    get_chunk(int row, int chunk, Table &T) {
		return *((sse_float_type *) &T(row, chunk * floats_per_chunk));
    }

    template<typename Table>
    static inline Table &
    transpose(Table &transpose, const Table &T) {
		assert(transpose.no_rows == T.no_columns);
		assert(transpose.no_columns == T.no_rows);

		for(int r = 0; r < T.no_rows; ++r)
			for(int c = 0; c < T.no_columns; ++c)
				transpose(c, r) = T(r,c);
		return transpose;
    }

    template<typename Table>
    static inline Table &
    log(Table &log_table, const Table &T) {
		assert(log_table.no_rows == T.no_rows);
		assert(log_table.no_columns == T.no_columns);

		for(int r = 0; r < T.no_rows; ++r)
			for(int c = 0; c < T.no_columns; ++c)
				log_table(r, c) = std::log(T(r,c));
		return log_table;
    }

    template<typename Table>
    static inline Table &
    add(float_type scalar, Table &T) {
		for(int r = 0; r < T.no_rows; ++r)
			for(int c = 0; c < T.no_chunks_per_row; ++c){
				sse_float_type scalar_chunk = _mm_set_ps1(scalar);
				T.get_chunk(r,c) += scalar_chunk;
			}
		return T;
    }

    template<typename Table>
    static inline Table &
    mul(float_type scalar, Table &T) {
		for(int r = 0; r < T.no_rows; ++r)
			for(int c = 0; c < T.no_chunks_per_row; ++c) {
				sse_float_type scalar_chunk = _mm_set_ps1(scalar);
				T.get_chunk(r,c) *= scalar_chunk;
			}
		return T;
    }

    template<typename Table>
    static inline float_type
    row_sum(int row, Table &T) {
		sse_float_type sum_chunk = _mm_set_ps1((float_type) 0.0);
		int c;
		for(c = 0; c < T.no_chunks_per_row - 1; ++c)
			sum_chunk += T.get_chunk(row, c);
		sum_chunk = _mm_hadd_ps(sum_chunk, sum_chunk);
		sum_chunk = _mm_hadd_ps(sum_chunk, sum_chunk);
		float_type sum = *((float_type *) &sum_chunk);
		for(int i = c*floats_per_chunk; i < T.no_columns; ++i)
			sum += T(row,i);
		return sum;
    }

    template<typename Table>
    static inline float_type
    column_sum(int column, Table &T) {
		float_type sum = 0.0;
		for(int r = 0; r < T.no_rows; ++r)
			sum += T(r, column);
		return sum;
    }
};
}

#endif
