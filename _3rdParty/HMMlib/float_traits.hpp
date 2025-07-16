#ifndef FLOAT_TRAITS_HPP
#define FLOAT_TRAITS_HPP

#include <pmmintrin.h>

namespace hmmlib {

  template <typename float_type>
  class FloatTraits {
    //    typedef float_type simd_type;
  };
  
  template <>
  class FloatTraits<float> {
  public :
    typedef __m128 sse_type;
  };
  
  template <>
  class FloatTraits<double> {
  public :
    typedef __m128d sse_type;
  };

} // end of namespace

#endif
