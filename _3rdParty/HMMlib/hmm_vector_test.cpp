#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN hmm_vector_test


#include <boost/test/included/unit_test.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/mpl/list.hpp>
using namespace boost::unit_test;

#include "HMMlib/hmm_vector.hpp"
using namespace hmmlib;

#include <pmmintrin.h>
#include <cmath>

BOOST_AUTO_TEST_SUITE( hmm_vector_test )

typedef boost::mpl::list<HMMVector<double, double>, 
			 HMMVector<double, __m128d>, 
			 HMMVector<float, float>, 
			 HMMVector<float, __m128> > test_types;

BOOST_AUTO_TEST_CASE_TEMPLATE(construction_test, Vector, test_types) {
  Vector vector(5);
  
  BOOST_CHECK_EQUAL(vector.get_no_rows(), 1);
  BOOST_CHECK_EQUAL(vector.get_no_columns(), 5);
  
  for(int i = 0; i < 5; ++i) {
    BOOST_CHECK_CLOSE(vector(i), 0.0, 0.001);
  }
  
  Vector vector2(5, 3.0);
  
  for(int i = 0; i < 5; ++i) {
    BOOST_CHECK_CLOSE(vector2(i), 3.0, 0.001);
  }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(size_test, Vector, test_types) {
  Vector vector1(5);

  BOOST_CHECK_EQUAL(vector1.get_size(), 5);

  Vector vector2(127);

  BOOST_CHECK_EQUAL(vector2.get_size(), 127);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(log_non_sse_test, Vector, test_types) {
  Vector vector(6);
  for(int i = 0; i < vector.get_size(); ++i)
      vector(i) = i+1;

  Vector log_vector(6);
  vector.log(log_vector);

  for(int i = 0; i < vector.get_size(); ++i)
    BOOST_CHECK_CLOSE(log_vector(i), std::log(i+1), 0.001);
}

BOOST_AUTO_TEST_SUITE_END()
