#ifndef HMM_MATRIX_HPP
#define HMM_MATRIX_HPP

#include "HMMlib/hmm_table.hpp"

namespace hmmlib {


  /**
   * \class HMMMatrix
   *
   * \brief A table of float_type's used for transiton matrices,
   * emission matrices, forward dynamic programming tables, backward
   * dynamic programming tables etc.
   *
   * A table of float_type's used for transiton matrices, emission
   * matrices, forward dynamic programming tables, backward dynamic
   * programming tables etc. The entries are stored row by row in
   * memory. If SSE instructions are enabled (default), each row is
   * 16-byte aligned in memory, such that rows can be read efficiently
   * using SSE instructions. \a get_chunk() may then be used to read
   * row entries in chunks of size 4 (when using floats) or 2 (when
   * using doubles).
   *
   * Possible values of \a float_type are: \a double or \a float.
   *
   * Possible values of sse_float_type are: \a double, \a float, \a
   * __m128d or __m128. If \a float_type is \a double, sse_float_type
   * must be either \a __m128d (default) or \a double. If \a
   * float_type is \a float, sse_float_type must be either \a __m128
   * (default) or \a float.
   *
   */
  template < typename float_type, 
	     typename sse_float_type = typename FloatTraits<float_type>::sse_type>
  class HMMMatrix : public HMMTable<float_type, sse_float_type> {
  public:

    /**
     * \brief Constructs a HMMMatrix with \a no_rows rows and \a no_columns
     * columns, setting all entries to \a val (default 0.0).
     *
     * Constructs a HMMMatrix with \a no_rows rows and \a no_columns
     * columns, setting all entries to \a val (default 0.0).
     *
     * \param no_rows     The number of rows.
     * \param no_columns  The number of columns.
     * \param val         The value of all entries (default 0.0).
     *
     * Possible values of \a float_type are: \a double or \a float.
     *
     * Possible values of sse_float_type are: \a double, \a float, \a
     * __m128d or __m128. If \a float_type is \a double, sse_float_type
     * must be either \a __m128d (default) or \a double. If \a
     * float_type is \a float, sse_float_type must be either \a __m128
     * (default) or \a float.
     */
    HMMMatrix(int no_rows, int no_columns, float_type val = 0.0)
      :HMMTable<float_type,sse_float_type>(no_rows, no_columns, val)
    { }

    ~HMMMatrix();

    // scalar operations
    /**
     * \brief Setting all entries in the matrix to \a val.
     *
     * Setting all entries in the matrix to \a val.
     *
     * \param val    The new value of all entries in the table.
     * \returns      A new table in which all entries are set to \a val.
     */
    HMMMatrix &operator=(float_type val);

    /**
     * \brief Transpose this matrix.
     *
     * Transpose this matrix.
     *
     * \param transpose   After running this function, the \f$i,j\f$th entry of \a transpose will be this(j,i).
     */
    void transpose(HMMMatrix<float_type, sse_float_type> &transpose) const;

  private:
    // don't copy!
    HMMMatrix(const HMMMatrix &);
    typedef OperatorTraits<float_type, sse_float_type> operators;
    typedef AllocatorTraits<float_type, sse_float_type> allocator;
  };

  template <typename float_type, typename sse_float_type>
  HMMMatrix<float_type,sse_float_type>::~HMMMatrix() { 

  }

  template<typename float_type, typename sse_float_type>
  inline HMMMatrix<float_type, sse_float_type> &
  HMMMatrix<float_type, sse_float_type>::operator=(float_type val) {
    reset(val);
    return *this;
  }

  template<typename float_type, typename sse_float_type>
  inline void
  HMMMatrix<float_type, sse_float_type>::transpose(HMMMatrix<float_type, sse_float_type> &transpose) const {
    operators::transpose(transpose, *this);
  }

} // namespace

#endif // HMM_MATRIX_HPP
