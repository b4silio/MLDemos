#ifndef HMM_VECTOR_HPP
#define HMM_VECTOR_HPP

#include <HMMlib/hmm_table.hpp>

#include <pmmintrin.h>

namespace hmmlib {

  /**
   * \class HMMVector
   *
   * \brief A vector of float_type values used for initial state
   * probability vectors, scaling factor vectors etc.
   *
   * A vector of float_type values used for initial state probability
   * vectors, scaling factor vectors etc. If SSE instructions are
   * enabled (default), the vector is 16-byte aligned in memory, such
   * that it can be read efficiently using SSE instructions. \a
   * get_chunk() may then be used to read entries in chunks of size 4
   * (when using floats) or 2 (when using doubles).
   *
   * Possible values of \a float_type are: \a double or \a float.
   *
   * Possible values of sse_float_type are: \a double, \a float, \a
   * __m128d or __m128. If \a float_type is \a double, sse_float_type
   * must be either \a __m128d (default) or \a double. If \a
   * float_type is \a float, sse_float_type must be either \a __m128
   * (default) or \a float.
   */
  template < typename float_type, typename sse_float_type = typename FloatTraits<float_type>::sse_type>
  class HMMVector : public HMMTable<float_type, sse_float_type> {
  public:
    /**
     * \brief Constructs an HMMVector of size \a size.
     *
     * Constructs an HMMVector of size \a size in which all elements
     * are set to \a val (default 0.0).
     *
     * \param size    The size.
     * \param val     The value of all entries (default 0.0).
     *
     * Possible values of \a float_type are: \a double or \a float.
     *
     * Possible values of sse_float_type are: \a double, \a float, \a
     * __m128d or __m128. If \a float_type is \a double, sse_float_type
     * must be either \a __m128d (default) or \a double. If \a
     * float_type is \a float, sse_float_type must be either \a __m128
     * (default) or \a float.
     */
    HMMVector(int size, float_type val = 0.0);

    ~HMMVector();

    // scalar operations
    /**
     * \brief Setting all entries in the vector to \a val.
     *
     * Setting all entries in the vector to \a val.
     *
     * \param val    The new value of all entries in the table.
     * \returns      A vector in which all entries are set to \a val.
     */
    HMMVector &operator=(float_type val);
	
    // data access
    /**
     * \brief Access the i'th element in the vector.
     *
     * Access the i'th element in the vector.
     *
     * \param i    The index.
     * \returns    The i'th element in the vector.
     */
    float_type &operator()(int i);

    /**
     * \brief get the i'th element in the vector.
     *
     * Get the i'th element in the vector.
     *
     * \param i    The index.
     * \returns    The i'th element in the vector.
     */
    const float_type &operator()(int i) const;

    /**
     * \brief Access the \f$c\f$th chunk in the vector.
     *
     * Access the c'th chunk in the vector. If \a sse_float_type is
     * either \a double or \a float, this is the same as the c'th
     * entry in the vector. Otherwise if the value of \a float_type is
     * \a float and the value of \a sse_float_type is \a __m128, \a
     * get_chunk(c) will return a chunk of type \a __m128 containing
     * the entries 4c, 4c+1, 4c+2 and 4c+3 of the vector. If \a
     * float_type is \a double and \a sse_float_type is \a __m128d,
     * \a get_chunk(c) will return a chunk of type \a __m128d
     * containing the entries 2c and 2c+1 of this HMMTable.
     *
     * \param c   The chunk number.
     * \returns   The c'th chunk in the vector.
     *
     */
    sse_float_type &get_chunk(int c) const;

    /**
     * \brief Get the sum of all elements in the vector.
     *
     * Get the sum of all elements in the vector.
     *
     * \returns   The sum of all elements in the vector.
     */
    float_type sum() const;

    /**
     * \brief Get the size of the vector.
     *
     * Get the size of the vector.
     *
     * \returns The size of the vector.
     */
    const int get_size() const;
  private:
    HMMVector(const HMMVector &);
    int size;
  };
	
  template <typename float_type, typename sse_float_type>
  HMMVector<float_type, sse_float_type>::HMMVector(int size, float_type val)
    : HMMTable<float_type, sse_float_type>(1, size, val), size(size)
  { }

  template <typename float_type, typename sse_float_type>
  HMMVector<float_type,sse_float_type>::~HMMVector() { 

  }

  template<typename float_type, typename sse_float_type>
  inline HMMVector<float_type, sse_float_type> &
  HMMVector<float_type, sse_float_type>::operator=(float_type val) {
    reset(val);
    return *this;
  }
	
  // accessors
  template <typename float_type, typename sse_float_type>
  inline float_type &
  HMMVector<float_type, sse_float_type>::operator()(int i) {
    return HMMTable<float_type, sse_float_type>::operator()(0, i);
  }
	
  template <typename float_type, typename sse_float_type>
  inline const float_type &
  HMMVector<float_type, sse_float_type>::operator()(int i) const {
    return HMMTable<float_type, sse_float_type>::operator()(0, i);
  }

  template <typename float_type, typename sse_float_type>
  inline sse_float_type &
  HMMVector<float_type, sse_float_type>::get_chunk(int chunk) const {
    return HMMTable<float_type, sse_float_type>::get_chunk(0, chunk);
  }
	
  template <typename float_type, typename sse_float_type>
  inline float_type
  HMMVector<float_type, sse_float_type>::sum() const {
    return HMMTable<float_type, sse_float_type>::row_sum(0);
  }

  template <typename float_type, typename sse_float_type>
  inline const int
  HMMVector<float_type, sse_float_type>::get_size() const {
    return size;
  }	
} // end of namespace 

#endif // HMM_VECTOR_CPP
