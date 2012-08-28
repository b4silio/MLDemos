#ifndef SSE_OPERATOR_TRAITS_HPP
#define SSE_OPERATOR_TRAITS_HPP

#include <xmmintrin.h>
#include <emmintrin.h>
#include <pmmintrin.h>
#include <algorithm>

namespace hmmlib {
	
  template <typename float_type,typename sse_float_type>
  struct SSEOperatorTraits { };
	
  template <typename float_type>
  struct SSEOperatorTraits<float_type, float_type> {
    typedef float_type sse_float_type;
		
    static inline void set_all(float_type &chunk, const sse_float_type &val){
      chunk = val;
    }
		
    static inline void sum(sse_float_type &chunk) { 
      // do nothing; single value already summed 
    }

    static inline sse_float_type mul(sse_float_type &chunk1, sse_float_type &chunk2) { 
      return chunk1 * chunk2;
    }
		
    static inline void store(float_type &dest, sse_float_type src) {
      dest = src;
    }
		
    static inline void max(sse_float_type &dest, sse_float_type &src) {
      dest = std::max(dest, src);
    }

    static inline float_type hmax(sse_float_type &src) {
      return src;
    }
  };
	
  template<>
  struct SSEOperatorTraits<float, __m128> {
    typedef float float_type;
    typedef __m128 sse_float_type;
		
    static inline void set_all(sse_float_type &chunk, const float_type &val) {
      chunk = _mm_set1_ps(val);
    }
		
    static inline void sum(sse_float_type &chunk) {
      chunk = _mm_hadd_ps(chunk, chunk);
      chunk = _mm_hadd_ps(chunk, chunk);
    }

    static inline sse_float_type mul(sse_float_type &chunk1, sse_float_type &chunk2) { 
      return _mm_mul_ps(chunk1, chunk2);
    }
		
    static inline void store(float_type &dest, sse_float_type src) {
      _mm_store_ss(&dest, src);
    }
		
    static inline void max(sse_float_type &dest, sse_float_type &src) {
      dest = _mm_max_ps(dest, src);
    }		

    static inline float_type hmax(sse_float_type &src) {
      float_type *temp = (float_type *)&src;
      return std::max(temp[0], std::max(temp[1], std::max(temp[2], temp[3])));
    }
  };
	
  template<>
  struct SSEOperatorTraits<double, __m128d> {
    typedef double float_type;
    typedef __m128d sse_float_type;
		
    static inline void set_all(sse_float_type &chunk, const float_type &val) {
      chunk = _mm_set1_pd(val);
    }
		
    static inline void sum(sse_float_type &chunk) {
      chunk = _mm_hadd_pd(chunk, chunk);
    }

    static inline sse_float_type mul(sse_float_type &chunk1, sse_float_type &chunk2) { 
      return _mm_mul_pd(chunk1, chunk2);
    }
		
    static inline void store(float_type &dest, const sse_float_type src) {
      _mm_store_sd(&dest, src);
    }
		
    static inline void max(sse_float_type &dest, const sse_float_type &src) {
      dest = _mm_max_pd(dest, src);
    }		

    static inline float_type hmax(const sse_float_type &src) {
      float_type *temp = (float_type *)&src;
      return std::max(temp[0], temp[1]);
    }

  };
	
}

#endif
