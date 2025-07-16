#ifndef HMM_TABLE_HPP
#define HMM_TABLE_HPP

#include "HMMlib/allocator_traits.hpp"
#include "HMMlib/operator_traits.hpp"
#include "HMMlib/float_traits.hpp"

#include <pmmintrin.h>

namespace hmmlib {

  /**
   * \class HMMTable
   *
   * Common interface for HMMMatrix and HMMVector.
   *
   */
  template < typename float_type, 
	     typename sse_float_type = typename FloatTraits<float_type>::sse_type>
  class HMMTable {
    typedef AllocatorTraits<float_type, sse_float_type> allocator;
    typedef OperatorTraits<float_type, sse_float_type> operators;
		
    // member fields
    float_type *table;
    int no_rows;
    int no_columns;
    int no_allocated_columns;
    int no_chunks_per_row;
		
  public:
    /**
     * \brief Constructs a HMMTable with \a no_rows rows and \a no_columns
     * columns, setting all entries to \a val (default 0.0).
     *
     * Constructs a HMMTable with \a no_rows rows and \a no_columns
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
    HMMTable(int no_rows, int no_columns, float_type val = 0.0);

    /**
     * \brief Destruct the HMMtable.
     *
     * Destruct the HMMtable. Deallocating the table.
     */
    virtual ~HMMTable() = 0;
		
    // table access
    /**
     * \brief Access the \f$(row,col)\f$th entry in the table.
     *
     * \param row     The row.
     * \param col     The column.
     *
     * Access the \f$(row,col)\f$th entry in the table.
     */
    float_type &operator()(int row, int col);

    /**
     * \brief Get the \f$(row,col)\f$th entry in the table.
     *
     * \param row     The row.
     * \param col     The column.
     *
     * Get the \f$(row,col)\f$th entry in the table.
     */
    const float_type &operator()(int row, int col) const;

    /**
     * \brief Access the \f$chunk\f$th chunk in the indicated row.
     *
     * Access the \f$chunk\f$th chunk in the indicated row. If \a
     * sse_float_type is either \a double or \a float, this is the
     * same as entry \f$(row, chunk)\f$ in the table.  Otherwise if
     * the value of \a float_type is \a float and the value of \a
     * sse_float_type is \a __m128, \a get_chunk(i,c) will return a
     * chunk of type \a __m128 containing the entries (i, 4c), (i,
     * 4c+1), (i, 4c+2) and (i, 4c+3) of this HMMTable. If \a
     * float_type is \a double and \a sse_float_type is \a __m128d,
     * \a get_chunk(i,c) will return a chunk of type \a __m128d
     * containing the entries (i, 2c) and (i, 2c+1) of this HMMTable.
     *
     * \param row     The row.
     * \param chunk   The chunk number.
     *
     */
    sse_float_type &get_chunk(int row, int chunk) const;
		
    // scalar operations
    /**
     * \brief Setting all entries in the table to \a val.
     *
     * Setting all entries in the table to \a val.
     *
     * \param val    The new value of all entries in the table.
     * \returns      A new table in which all entries are set to \a val.
     */
    virtual HMMTable &operator=(float_type val) = 0;

    /**
     * \brief Adding \a scalar to all entries in the table.
     *
     * Adding \a scalar to all entries in the table.
     *
     * \param scalar The value to be added to all entries of the table.
     * \returns      A new table in which the value of each entry is the sum of \a scalar and the corresponding entry in this table.
     */
    HMMTable &operator+=(float_type scalar);

    /**
     * \brief Substracting \a scalar from all entries in the table.
     *
     * Substracting \a scalar from all entries in the table.
     *
     * \param scalar The value to be substracted from all entries of the table.
     * \returns      A new table in which the value of each entry is the sum of \a -scalar and the corresponding entry in this table.
     */
    HMMTable &operator-=(float_type scalar);

    /**
     * \brief Multiplying \a scalar to all entries in the table.
     *
     * Multiplying \a scalar to all entries in the table. 
     *
     * \param scalar The value to be multiplied on all entries of the table.
     * \returns      A new table in which the value of each entry is the product of \a scalar and the corresponding entry in this table.
     */
    HMMTable &operator*=(float_type scalar);

    /**
     * \brief Dividing all entries in the table with \a scalar.
     *
     * Dividing all entries in the table with \a scalar.
     *
     * \param scalar The value all entries will be divided with.
     * \returns      A new table in which the value of each entry is the product of \f$\frac{1}{scalar}\f$ and the corresponding entry in this table.
     */    
    HMMTable &operator/=(float_type scalar);

    /**
     * \brief Aplies the natural logarithm function of all entries in the table.
     *
     * Applies the natural logarithm of all entries in the table.
     *
     * \returns     A new table in which each entry is log base \a e of the correponding entry in this table.
     *
     */
    HMMTable &log(HMMTable<float_type, sse_float_type> &log_table) const;


    /**
     * \brief Setting all entries of this table to 0.0.
     *
     * Setting all entries of this table to 0.0.
     */
    void reset(float_type val = 0.0);
		
    // matrix operations
    /**
     * \brief Computes the sum of the \a column'th column.
     *
     * Computes the sum of all entries in the \a column'th column in this table.
     *
     * \param column The column.
     */
    float_type column_sum(int column) const;

    /**
     * \brief Computes the sum of the \a row'th row.
     *
     * Computes the sum of all entries in the \a row'th row of this table.
     *
     * \param row    The row.
     */
    float_type row_sum(int row) const;
		
    //field access
    /**
     * \brief Get the number of rows in this table.
     *
     * Get the number of rows in this table.
     */
    int get_no_rows() const { return no_rows; }

    /**
     * \brief Get the number of columns in this table.
     *
     * Get the number of columns in this table.
     */
    int get_no_columns() const {return no_columns; }

    /**
     * \brief Get the number of columns allocated for each row in this table.
     *
     * Get the number of columns allocated for each row in this
     * table. If the value of \a sse_float_type is either \a double
     * of \a float, this is the same as you get from \a
     * get_no_columns(). However to ensure that all rows are 16-byte
     * aligned when SSE instructions are enabled, the number of
     * allocated columns must be divisible by 4 (when the value of \a
     * float_type is \a float) or 2 (when the value of \a float_type
     * is \a double).
     */
    int get_no_allocated_columns() const { return no_allocated_columns; }

    /**
     * \brief Get the number of allocated chunks in each row of this table.
     *
     * Get the number of allocated chunks in each row of this
     * table. If the value of \a sse_float_type is either \a double
     * of \a float, this is the same as you get from \a
     * get_no_columns(), since in these cases a chunk consist of a
     * single value. However if SSE instructions are enabled, the
     * number of chunks per row is either \f$get\_no\_columns()\ 
     * div\ 4\f$ (when \a float_type is \a float) or
     * \f$get\_no\_columns()\ div\ 2\f$ (when \a float_type is \a
     * double), where \f$div\f$ is integer division, rounding upwards
     * if the remainder is not 0.
     */
    int get_no_chunks_per_row() const { return no_chunks_per_row; }
		
  private:
    // copy constructor -- private; don't copy!
    HMMTable(const HMMTable&);
    HMMTable &operator=(const HMMTable&);
		
    friend class AllocatorTraits<float_type, sse_float_type>;
    friend class OperatorTraits<float_type, sse_float_type>;
  };

  template<typename float_type, typename sse_float_type>
  HMMTable<float_type, sse_float_type>::HMMTable(int no_rows, int no_columns, float_type val) {
    allocator::allocate(no_rows, no_columns, *this);
    reset(val);
  }

  template <typename float_type, typename sse_float_type>
  inline HMMTable<float_type,sse_float_type>::~HMMTable() { 
    allocator::free(*this); 
  }
		
  template<typename float_type, typename sse_float_type>
  inline float_type &
  HMMTable<float_type, sse_float_type>::operator()(int row, int col) {
    return table[no_allocated_columns * row + col];
  }
	
  template<typename float_type, typename sse_float_type>
  inline const float_type &
  HMMTable<float_type, sse_float_type>::operator()(int row, int col) const {
    return table[no_allocated_columns * row + col];
  }

  template<typename float_type, typename sse_float_type>
  inline sse_float_type &
  HMMTable<float_type, sse_float_type>::get_chunk(int row, int chunk) const {
    return operators::get_chunk(row, chunk, *this);
  }
	
  template<typename float_type, typename sse_float_type>
  inline void HMMTable<float_type, sse_float_type>::reset(float_type val) {
    operators::reset(val, *this);
  }
	
  template<typename float_type, typename sse_float_type>
  inline HMMTable<float_type, sse_float_type> &
  HMMTable<float_type, sse_float_type>::operator+=(float_type val) {
    return operators::add(val, *this);
  }
	
  template<typename float_type, typename sse_float_type>
  inline HMMTable<float_type, sse_float_type> &
  HMMTable<float_type, sse_float_type>::operator-=(float_type val) {
    return *this += -1 * val;
  }

  template<typename float_type, typename sse_float_type>
  inline HMMTable<float_type, sse_float_type> &
  HMMTable<float_type, sse_float_type>::operator*=(float_type val) {
    return operators::mul(val, *this);
  }
	
  template<typename float_type, typename sse_float_type>
  inline HMMTable<float_type, sse_float_type> &
  HMMTable<float_type, sse_float_type>::operator/=(float_type val) {
    return *this *= 1 / val;
  }

  template<typename float_type, typename sse_float_type>
  inline HMMTable<float_type, sse_float_type> &
  HMMTable<float_type, sse_float_type>::log(HMMTable<float_type, sse_float_type> &log_table) const {
    return operators::log(log_table, *this);
  }
	
  template<typename float_type, typename sse_float_type>
  inline float_type
  HMMTable<float_type, sse_float_type>::row_sum(int row) const {
    return operators::row_sum(row, *this);
  }
	
  template<typename float_type, typename sse_float_type>
  inline float_type
  HMMTable<float_type, sse_float_type>::column_sum(int column) const {
    return operators::column_sum(column, *this);
  }
	
} // end of namespace

#endif // HMM_TABLE_CPP
