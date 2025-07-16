#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN hmm_test
//#define par

#include <boost/test/included/unit_test.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/mpl/list.hpp>
using namespace boost::unit_test;

#include "HMMlib/hmm_vector.hpp"
#include "HMMlib/hmm_table.hpp"
#include "HMMlib/hmm.hpp"
using namespace hmmlib;

#include <pmmintrin.h>


#ifdef WITH_OMP
#include<omp.h>
#endif

BOOST_AUTO_TEST_SUITE( hmm_test )

template <typename float_type, typename sse_float_type>
static void check_1_table(const HMMMatrix<float_type, sse_float_type> &T) {
  for (int r = 0; r < T.get_no_rows(); ++r)
    for (int c = 0; c < T.get_no_columns(); ++c)
      BOOST_CHECK_CLOSE(T(r,c), (float_type)1.0, 0.001);
}

template <typename float_type, typename sse_float_type>
static void check_matrix_column(const HMMMatrix<float_type, sse_float_type> &T, int c, double val) {
  for (int r = 0; r < T.get_no_rows(); ++r)
    BOOST_CHECK_CLOSE(T(r,c), (float_type)val, 0.001);
}

template <typename float_type>
void construct_non_sse() {
  boost::shared_ptr<HMMVector<float_type,float_type> > 
    pi_ptr(new HMMVector<float_type,float_type>(7));
  boost::shared_ptr<HMMMatrix<float_type,float_type> > 
    T_ptr(new HMMMatrix<float_type,float_type>(7,7));
  boost::shared_ptr<HMMMatrix<float_type,float_type> > 
    E_ptr(new HMMMatrix<float_type,float_type>(9,7));

  HMM<float_type,float_type> hmm(pi_ptr,
		      T_ptr,
		      E_ptr);
}

template <typename float_type, typename sse_float_type>
void construct_sse() {
  boost::shared_ptr<HMMVector<float_type, sse_float_type> > 
    pi_ptr(new HMMVector<float_type, sse_float_type>(7));
  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > 
    T_ptr(new HMMMatrix<float_type, sse_float_type>(7,7));
  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > 
    E_ptr(new HMMMatrix<float_type, sse_float_type>(9,7));

  HMM<float_type, sse_float_type> hmm(pi_ptr,
		      T_ptr,
		      E_ptr);
}

// ############ FORWARD TESTS ############### //
template <typename float_type, typename sse_float_type>
void forward_1x1() {	
  int no_states = 1;
  int alphabet_size = 1;
  int seq_length = 3;
  
  boost::shared_ptr<HMMVector<float_type, sse_float_type> > pi_ptr(new HMMVector<float_type, sse_float_type>(no_states));
  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > T_ptr(new HMMMatrix<float_type, sse_float_type>(no_states,no_states));
  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > E_ptr(new HMMMatrix<float_type, sse_float_type>(alphabet_size, no_states));
  
  HMMVector<float_type, sse_float_type> &pi = *pi_ptr;
  HMMMatrix<float_type, sse_float_type> &T = *T_ptr;
  HMMMatrix<float_type, sse_float_type> &E = *E_ptr;
	
  pi(0)  = 1.0;
  T(0,0) = 1.0;
  E(0,0) = 1.0;
	
  HMM<float_type, sse_float_type> hmm(pi_ptr, T_ptr, E_ptr);
	
  unsigned obs_array[] = { 0, 0, 0, 0 };
  sequence obs(obs_array, obs_array+seq_length);
	
  HMMMatrix<float_type, sse_float_type> F(obs.size(), no_states);
  HMMVector<float_type, sse_float_type> scales(obs.size());
  hmm.forward(obs, scales, F);
	
  BOOST_CHECK_CLOSE(hmm.likelihood(scales), (float_type) 0.0, 0.001); // log-likelihood
  check_1_table(F);
}

template <typename float_type, typename sse_float_type>
void forward_1x2_no_transitions() {
  int no_states = 2;
  int alphabet_size = 1;
  int seq_length = 4;
    
  boost::shared_ptr<HMMVector<float_type, sse_float_type> > pi_ptr(new HMMVector<float_type, sse_float_type>(no_states));
  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > T_ptr(new HMMMatrix<float_type, sse_float_type>(no_states,no_states));
  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > E_ptr(new HMMMatrix<float_type, sse_float_type>(alphabet_size, no_states));
	
  HMMVector<float_type, sse_float_type> &pi = *pi_ptr;
  HMMMatrix<float_type, sse_float_type> &T = *T_ptr;
  HMMMatrix<float_type, sse_float_type> &E = *E_ptr;
	
  pi(0)  = 1.0; pi(1) = 0.0;
  T(0,0) = 1.0; T(0,1) = 0.0;
  T(1,0) = 0.0; T(1,1) = 1.0;
  E(0,0) = 1.0; E(0,1) = 0.0;
	
  HMM<float_type, sse_float_type> hmm(pi_ptr, T_ptr, E_ptr);
	
  unsigned obs_array[] = { 0, 0, 0, 0 };
  sequence obs(obs_array, obs_array+seq_length);
	
  HMMMatrix<float_type, sse_float_type> F(seq_length, no_states);
  HMMVector<float_type, sse_float_type> scales(seq_length);
  hmm.forward(obs, scales, F);
	
  BOOST_CHECK_CLOSE(hmm.likelihood(scales), (float_type)0.0, 0.001);
	
  check_matrix_column(F, 0, 1.0);
  check_matrix_column(F, 1, 0.0);
}

template <typename float_type, typename sse_float_type>
void forward_2x2_no_transitions() {
  int no_states = 2;
  int alphabet_size = 2;
  int seq_length = 4;
	
  boost::shared_ptr<HMMVector<float_type, sse_float_type> > pi_ptr(new HMMVector<float_type, sse_float_type>(no_states));
  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > T_ptr(new HMMMatrix<float_type, sse_float_type>(no_states,no_states));
  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > E_ptr(new HMMMatrix<float_type, sse_float_type>(alphabet_size, no_states));
	
  HMMVector<float_type, sse_float_type> &pi = *pi_ptr;
  HMMMatrix<float_type, sse_float_type> &T = *T_ptr;
  HMMMatrix<float_type, sse_float_type> &E = *E_ptr;
	
  pi(0)  = 1.0; pi(1) = 0.0;
  T(0,0) = 1.0; T(0,1) = 0.0;
  T(1,0) = 0.0; T(1,1) = 1.0;
	
  // emissions from state 0
  E(0,0) = 0.25;
  E(1,0) = 0.75;
  // emissions from state 1
  E(0,1) = 0.25;
  E(1,1) = 0.75;
	
  HMM<float_type, sse_float_type> hmm(pi_ptr, T_ptr, E_ptr);
	
  unsigned obs_array[] = { 0, 1, 0, 1 };
  sequence obs(obs_array, obs_array+seq_length);
	
  HMMMatrix<float_type, sse_float_type> F(seq_length, no_states);
  HMMVector<float_type, sse_float_type> scales(seq_length);
	
  hmm.forward(obs, scales, F);
	
  BOOST_CHECK_CLOSE(hmm.likelihood(scales), (float_type)std::log(0.25*0.75*0.25*0.75), 0.01);
	
  BOOST_CHECK_CLOSE(F(0,0), (float_type)1, 0.001);
  BOOST_CHECK_CLOSE(F(1,0), (float_type)1, 0.001);
  BOOST_CHECK_CLOSE(F(2,0), (float_type)1, 0.001);
  BOOST_CHECK_CLOSE(F(3,0), (float_type)1, 0.001);
	
  check_matrix_column(F, 1, 0.0);
}

template <typename float_type, typename sse_float_type>
void forward_2x2_with_transitions() {
  int no_states = 2;
  int alphabet_size = 2;
  int seq_length = 4;
	
  boost::shared_ptr<HMMVector<float_type, sse_float_type> > pi_ptr(new HMMVector<float_type, sse_float_type>(no_states));
  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > T_ptr(new HMMMatrix<float_type, sse_float_type>(no_states,no_states));
  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > E_ptr(new HMMMatrix<float_type, sse_float_type>(alphabet_size, no_states));
	
  HMMVector<float_type, sse_float_type> &pi = *pi_ptr;
  HMMMatrix<float_type, sse_float_type> &T = *T_ptr;
  HMMMatrix<float_type, sse_float_type> &E = *E_ptr;
	
  pi(0)  = 0.2; pi(1) = 0.8;
  T(0,0) = 0.1; T(0,1) = 0.9;
  T(1,0) = 0.9; T(1,1) = 0.1;
	
  // emissions from state 0
  E(0,0) = 0.25;
  E(1,0) = 0.75;
  // emissions from state 1
  E(0,1) = 0.25;
  E(1,1) = 0.75;
	
  HMM<float_type, sse_float_type> hmm(pi_ptr, T_ptr, E_ptr);
	
  unsigned obs_array[] = { 0, 1, 0, 1 };
  sequence obs(obs_array, obs_array+seq_length);
	
  HMMMatrix<float_type, sse_float_type> F(seq_length, no_states);
  HMMVector<float_type, sse_float_type> scales(seq_length);
	
  hmm.forward(obs, scales, F);
	
  BOOST_CHECK_CLOSE(hmm.likelihood(scales), (float_type)std::log(0.25*0.75*0.25*0.75), 0.001);
	
  BOOST_CHECK_CLOSE((float_type)F(0,0), (float_type)(0.2*0.25 * scales(0)), 0.001);
  BOOST_CHECK_CLOSE((float_type)F(0,1), (float_type)(0.8*0.25 * scales(0)), 0.001);
	
  BOOST_CHECK_CLOSE((float_type)F(1,0), (float_type)((F(0,0)*0.1+F(0,1)*0.9)*0.75 * scales(1)), 0.001);
  BOOST_CHECK_CLOSE((float_type)F(1,1), (float_type)((F(0,0)*0.9+F(0,1)*0.1)*0.75 * scales(1)), 0.001);
	
  BOOST_CHECK_CLOSE((float_type)F(2,0), (float_type)((F(1,0)*0.1+F(1,1)*0.9)*0.25 * scales(2)), 0.001);
  BOOST_CHECK_CLOSE((float_type)F(2,1), (float_type)((F(1,0)*0.9+F(1,1)*0.1)*0.25 * scales(2)), 0.001);
	
  BOOST_CHECK_CLOSE((float_type)F(3,0), (float_type)((F(2,0)*0.1+F(2,1)*0.9)*0.75 * scales(3)), 0.001);
  BOOST_CHECK_CLOSE((float_type)F(3,1), (float_type)((F(2,0)*0.9+F(2,1)*0.1)*0.75 * scales(3)), 0.001);
}

#ifdef WITH_OMP
template <typename float_type, typename sse_float_type>
void parallel_forward() {
  int obsseq_length = 100;
  int no_states = 128;
  int alphabet_size = 16;

  vector<unsigned int> obs;
  for(int i = 0; i < obsseq_length; ++i) {
    unsigned r = random() % alphabet_size;
    obs.push_back(r);
  }
	
  boost::shared_ptr<HMMVector<float_type, sse_float_type> > pi_ptr(new HMMVector<float_type, sse_float_type>(no_states));
  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > T_ptr(new HMMMatrix<float_type, sse_float_type>(no_states, no_states));
  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > E_ptr(new HMMMatrix<float_type, sse_float_type>(alphabet_size, no_states));
	
  HMMVector<float_type, sse_float_type> &pi = *pi_ptr;
  HMMMatrix<float_type, sse_float_type> &T = *T_ptr;
  HMMMatrix<float_type, sse_float_type> &E = *E_ptr;
	
  for(int i = 0; i < no_states; ++i)
    pi(i) = random();
  double sum = pi.sum();
  pi /= sum;
	
  for(int r = 0; r < no_states; ++r) {
    for(int c = 0; c < no_states; ++c)
      T(r,c) = random();
    sum = T.row_sum(r);
    for(int c = 0; c < no_states; ++c)
      T(r,c) /= sum;
  }
	
  for(int c = 0; c < no_states; ++c) {
    for(int r = 0; r < alphabet_size; ++r)
      E(r,c) = random();
    sum = T.column_sum(c);
    for(int r = 0; r < alphabet_size; ++r)
      E(r,c) /= sum;
  }
	
  HMM<float_type, sse_float_type> hmm(pi_ptr, T_ptr, E_ptr);

  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > F_ptr(new HMMMatrix<float_type, sse_float_type>(obsseq_length, no_states));
  HMMMatrix<float_type, sse_float_type> &F = *F_ptr;
  boost::shared_ptr<HMMVector<float_type, sse_float_type> > scales_ptr(new HMMVector<float_type, sse_float_type>(obsseq_length));
  HMMVector<float_type, sse_float_type> &scales = *scales_ptr;

  omp_set_num_threads(1);
  hmm.forward(obs, scales, F);
  float_type likelihood = hmm.likelihood(scales);
  //  std::cout << likelihood << std::endl;

  omp_set_num_threads(2);
  hmm.forward(obs, scales, F);
  float_type likelihood2 = hmm.likelihood(scales);
  //  std::cout << likelihood2 << std::endl;

  omp_set_num_threads(4);
  hmm.forward(obs, scales, F);
  float_type likelihood4 = hmm.likelihood(scales);
  //  std::cout << likelihood4 << std::endl;

  omp_set_num_threads(8);
  hmm.forward(obs, scales, F);
  float_type likelihood8 = hmm.likelihood(scales);
  //  std::cout << likelihood8 << std::endl;

  BOOST_CHECK_CLOSE(likelihood, likelihood2, 0.01);
  BOOST_CHECK_CLOSE(likelihood, likelihood4, 0.01);
  BOOST_CHECK_CLOSE(likelihood, likelihood8, 0.01);
}
#endif

template <typename float_type>
void sse_vs_nonSse_forward() {

  typedef typename FloatTraits<float_type>::sse_type sse_float_type;

  int obsseq_length = 133;
  int no_states = 128;
  int alphabet_size = 16;

  vector<unsigned int> obs;
  for(int i = 0; i < obsseq_length; ++i) {
    unsigned r = random() % alphabet_size;
    obs.push_back(r);
  }
	
  boost::shared_ptr<HMMVector<float_type, float_type> > pi_ptr(new HMMVector<float_type, float_type>(no_states));
  boost::shared_ptr<HMMMatrix<float_type, float_type> > T_ptr(new HMMMatrix<float_type, float_type>(no_states, no_states));
  boost::shared_ptr<HMMMatrix<float_type, float_type> > E_ptr(new HMMMatrix<float_type, float_type>(alphabet_size, no_states));

  boost::shared_ptr<HMMVector<float_type, sse_float_type> > pi_ptr_sse(new HMMVector<float_type, sse_float_type>(no_states));
  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > T_ptr_sse(new HMMMatrix<float_type, sse_float_type>(no_states, no_states));
  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > E_ptr_sse(new HMMMatrix<float_type, sse_float_type>(alphabet_size, no_states));

	
  HMMVector<float_type, float_type> &pi = *pi_ptr;
  HMMMatrix<float_type, float_type> &T = *T_ptr;
  HMMMatrix<float_type, float_type> &E = *E_ptr;

  HMMVector<float_type, sse_float_type> &pi_sse = *pi_ptr_sse;
  HMMMatrix<float_type, sse_float_type> &T_sse = *T_ptr_sse;
  HMMMatrix<float_type, sse_float_type> &E_sse = *E_ptr_sse;
	
  for(int i = 0; i < no_states; ++i) {
    pi(i) = random();
    pi_sse(i) = pi(i);
  }
  double sum = pi.sum();
  pi /= sum;
  pi_sse /= sum;
	
  for(int r = 0; r < no_states; ++r) {
    for(int c = 0; c < no_states; ++c) {
      T(r,c) = random();
    }
    sum = T.row_sum(r);
    for(int c = 0; c < no_states; ++c) {
      T(r,c) /= sum;
      T_sse(r,c) = T(r,c);
    }
  }
	
  for(int c = 0; c < no_states; ++c) {
    for(int r = 0; r < alphabet_size; ++r) {
      E(r,c) = random();
    }
    sum = T.column_sum(c);
    for(int r = 0; r < alphabet_size; ++r) {
      E(r,c) /= sum;
      E_sse(r,c) = E(r,c);
    }
  }
	
  HMM<float_type, float_type> hmm(pi_ptr, T_ptr, E_ptr);
  HMM<float_type, sse_float_type> hmm_sse(pi_ptr_sse, T_ptr_sse, E_ptr_sse);

  boost::shared_ptr<HMMMatrix<float_type, float_type> > F_ptr(new HMMMatrix<float_type, float_type>(obsseq_length, no_states));
  HMMMatrix<float_type, float_type> &F = *F_ptr;
  boost::shared_ptr<HMMVector<float_type, float_type> > scales_ptr(new HMMVector<float_type, float_type>(obsseq_length));
  HMMVector<float_type, float_type> &scales = *scales_ptr;

  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > F_ptr_sse(new HMMMatrix<float_type, sse_float_type>(obsseq_length, no_states));
  HMMMatrix<float_type, sse_float_type> &F_sse = *F_ptr_sse;
  boost::shared_ptr<HMMVector<float_type, sse_float_type> > scales_ptr_sse(new HMMVector<float_type, sse_float_type>(obsseq_length));
  HMMVector<float_type, sse_float_type> &scales_sse = *scales_ptr_sse;

  hmm.forward(obs, scales, F);
  hmm_sse.forward(obs, scales_sse, F_sse);

  float_type likelihood = hmm.likelihood(scales);
  float_type likelihood_sse = hmm_sse.likelihood(scales_sse);

  BOOST_CHECK_CLOSE(F(132,127), F_sse(132,127), 0.01);
  BOOST_CHECK_CLOSE(F(100,25), F_sse(100,25), 0.01);
  BOOST_CHECK_CLOSE(F(87,3), F_sse(87,3), 0.01);
  BOOST_CHECK_CLOSE(likelihood, likelihood_sse, 0.01);
}

// ############ BACKWARD TESTS ############### //
template<typename float_type, typename sse_float_type>
void backward_1x1() {
  int no_states = 1;
  int alphabet_size = 1;
  int seq_length = 3;
    
  boost::shared_ptr<HMMVector<float_type, sse_float_type> > pi_ptr(new HMMVector<float_type, sse_float_type>(no_states));
  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > T_ptr(new HMMMatrix<float_type, sse_float_type>(no_states,no_states));
  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > E_ptr(new HMMMatrix<float_type, sse_float_type>(alphabet_size, no_states));
	
  HMMVector<float_type, sse_float_type> &pi = *pi_ptr;
  HMMMatrix<float_type, sse_float_type> &T = *T_ptr;
  HMMMatrix<float_type, sse_float_type> &E = *E_ptr;
	
  pi(0)  = 1.0;
  T(0,0) = 1.0;
  E(0,0) = 1.0;
	
  HMM<float_type, sse_float_type> hmm(pi_ptr, T_ptr, E_ptr);
	
  unsigned obs_array[] = { 0, 0, 0, 0 };
  sequence obs(obs_array, obs_array+seq_length);
	
  HMMMatrix<float_type, sse_float_type> F(seq_length, no_states);
  HMMVector<float_type, sse_float_type> scales(seq_length);
  hmm.forward(obs, scales, F);
  HMMMatrix<float_type, sse_float_type> B(seq_length, no_states);
  hmm.backward(obs, scales, B);
	
  check_1_table(B);
}

template <typename float_type, typename sse_float_type>
void backward_1x2_no_transitions() {
  int no_states = 2;
  int alphabet_size = 1;
  int seq_length = 4;
	
  boost::shared_ptr<HMMVector<float_type, sse_float_type> > pi_ptr(new HMMVector<float_type, sse_float_type>(no_states));
  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > T_ptr(new HMMMatrix<float_type, sse_float_type>(no_states,no_states));
  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > E_ptr(new HMMMatrix<float_type, sse_float_type>(alphabet_size, no_states));
	
  HMMVector<float_type, sse_float_type> &pi = *pi_ptr;
  HMMMatrix<float_type, sse_float_type> &T = *T_ptr;
  HMMMatrix<float_type, sse_float_type> &E = *E_ptr;
	
  pi(0)  = 1.0; pi(1) = 0.0;
  T(0,0) = 1.0; T(0,1) = 0.0;
  T(1,0) = 0.0; T(1,1) = 1.0;
  E(0,0) = 1.0; E(0,1) = 0.0;
	
  HMM<float_type, sse_float_type> hmm(pi_ptr, T_ptr, E_ptr);
	
  unsigned obs_array[] = { 0, 0, 0, 0 };
  sequence obs(obs_array, obs_array+seq_length);
	
  HMMMatrix<float_type, sse_float_type> F(seq_length, no_states);
  HMMMatrix<float_type, sse_float_type> B(seq_length, no_states);
  HMMVector<float_type, sse_float_type> scales(seq_length);
	
  hmm.forward(obs, scales, F);
  hmm.backward(obs, scales, B);
	
  check_matrix_column(B, 0, 1.0);
	
  BOOST_WARN_CLOSE(B(3,1), scales(3), 0.001);
	
  BOOST_WARN_CLOSE(B(2,1), (B(3,0)*T(1,0)*E(1,0) + B(3,1)*T(1,1)*E(1,1))*scales(2), 0.001);
	
  BOOST_WARN_CLOSE(B(1,1), (B(2,0)*T(1,0)*E(0,0) + B(2,1)*T(1,1)*E(0,1))*scales(1), 0.001);
	
  BOOST_WARN_CLOSE(B(0,1), (B(1,0)*T(1,0)*E(1,0) + B(1,1)*T(1,1)*E(1,1))*scales(0), 0.001);
}

template <typename float_type, typename sse_float_type>
void backward_2x2_no_transitions() {
  int no_states = 2;
  int alphabet_size = 2;
  int seq_length = 4;
	
  boost::shared_ptr<HMMVector<float_type, sse_float_type> > pi_ptr(new HMMVector<float_type, sse_float_type>(no_states));
  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > T_ptr(new HMMMatrix<float_type, sse_float_type>(no_states,no_states));
  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > E_ptr(new HMMMatrix<float_type, sse_float_type>(alphabet_size, no_states));
	
  HMMVector<float_type, sse_float_type> &pi = *pi_ptr;
  HMMMatrix<float_type, sse_float_type> &T = *T_ptr;
  HMMMatrix<float_type, sse_float_type> &E = *E_ptr;
	
  pi(0)  = 1.0; pi(1) = 0.0;
  T(0,0) = 1.0; T(0,1) = 0.0;
  T(1,0) = 0.0; T(1,1) = 1.0;
	
  // emissions from state 0
  E(0,0) = 0.25;
  E(1,0) = 0.75;
  // emissions from state 1
  E(0,1) = 0.25;
  E(1,1) = 0.75;
	
  HMM<float_type, sse_float_type> hmm(pi_ptr, T_ptr, E_ptr);
	
  unsigned obs_array[] = { 0, 1, 0, 1 };
  sequence obs(obs_array, obs_array+seq_length);
	
  HMMMatrix<float_type, sse_float_type> F(seq_length, no_states);
  HMMMatrix<float_type, sse_float_type> B(seq_length, no_states);
  HMMVector<float_type, sse_float_type> scales(seq_length);
	
  hmm.forward(obs, scales, F);
  hmm.backward(obs, scales, B);
	
  BOOST_CHECK_CLOSE(B(3,0), scales(3), 0.001);
  BOOST_CHECK_CLOSE(B(3,1), scales(3), 0.001);
	
  BOOST_CHECK_CLOSE(B(2,0), (B(3,0)*T(0,0)*E(1,0) + B(3,1)*T(0,1)*E(1,1))*scales(2), 0.001);
  BOOST_CHECK_CLOSE(B(2,1), (B(3,0)*T(1,0)*E(1,0) + B(3,1)*T(1,1)*E(1,1))*scales(2), 0.001);
	
  BOOST_CHECK_CLOSE(B(1,0), (B(2,0)*T(0,0)*E(0,0) + B(2,1)*T(0,1)*E(0,1))*scales(1), 0.001);
  BOOST_CHECK_CLOSE(B(1,1), (B(2,0)*T(1,0)*E(0,0) + B(2,1)*T(1,1)*E(0,1))*scales(1), 0.001);
	
  BOOST_CHECK_CLOSE(B(0,0), (B(1,0)*T(0,0)*E(1,0) + B(1,1)*T(0,1)*E(1,1))*scales(0), 0.001);
  BOOST_CHECK_CLOSE(B(0,1), (B(1,0)*T(1,0)*E(1,0) + B(1,1)*T(1,1)*E(1,1))*scales(0), 0.001);
}

template <typename float_type, typename sse_float_type>
void backward_2x2_with_transitions() {
  int no_states = 2;
  int alphabet_size = 2;
  int seq_length = 4;
	
  boost::shared_ptr<HMMVector<float_type, sse_float_type> > pi_ptr(new HMMVector<float_type, sse_float_type>(no_states));
  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > T_ptr(new HMMMatrix<float_type, sse_float_type>(no_states,no_states));
  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > E_ptr(new HMMMatrix<float_type, sse_float_type>(alphabet_size, no_states));
	
  HMMVector<float_type, sse_float_type> &pi = *pi_ptr;
  HMMMatrix<float_type, sse_float_type> &T = *T_ptr;
  HMMMatrix<float_type, sse_float_type> &E = *E_ptr;
	
  pi(0)  = 0.2; pi(1) = 0.8;
  T(0,0) = 0.1; T(0,1) = 0.9;
  T(1,0) = 0.9; T(1,1) = 0.1;
	
  // emissions from state 0
  E(0,0) = 0.25;
  E(1,0) = 0.75;
  // emissions from state 1
  E(0,1) = 0.25;
  E(1,1) = 0.75;
	
  HMM<float_type, sse_float_type> hmm(pi_ptr, T_ptr, E_ptr);
	
  unsigned obs_array[] = { 0, 1, 0, 1 };
  sequence obs(obs_array, obs_array+seq_length);
	
  HMMMatrix<float_type, sse_float_type> F(seq_length, no_states);
  HMMMatrix<float_type, sse_float_type> B(seq_length, no_states);
  HMMVector<float_type, sse_float_type> scales(seq_length);
	
  hmm.forward(obs, scales, F);
  hmm.backward(obs, scales, B);
	
  BOOST_CHECK_CLOSE(B(3,0), scales(3), 0.01);
  BOOST_CHECK_CLOSE(B(3,1), scales(3), 0.01);
	
  BOOST_CHECK_CLOSE(B(2,0), (B(3,0)*T(0,0)*E(1,0) + B(3,1)*T(0,1)*E(1,1))*scales(2), 0.001);
  BOOST_CHECK_CLOSE(B(2,1), (B(3,0)*T(1,0)*E(1,0) + B(3,1)*T(1,1)*E(1,1))*scales(2), 0.001);
	
  BOOST_CHECK_CLOSE(B(1,0), (B(2,0)*T(0,0)*E(0,0) + B(2,1)*T(0,1)*E(0,1))*scales(1), 0.001);
  BOOST_CHECK_CLOSE(B(1,1), (B(2,0)*T(1,0)*E(0,0) + B(2,1)*T(1,1)*E(0,1))*scales(1), 0.001);
	
  BOOST_CHECK_CLOSE(B(0,0), (B(1,0)*T(0,0)*E(1,0) + B(1,1)*T(0,1)*E(1,1))*scales(0), 0.001);
  BOOST_CHECK_CLOSE(B(0,1), (B(1,0)*T(1,0)*E(1,0) + B(1,1)*T(1,1)*E(1,1))*scales(0), 0.001);
}

#ifdef WITH_OMP
template <typename float_type, typename sse_float_type>
void parallel_backward() {
  int obsseq_length = 100;
  int no_states = 128;
  int alphabet_size = 16;

  vector<unsigned int> obs;
  for(int i = 0; i < obsseq_length; ++i) {
    unsigned r = random() % alphabet_size;
    obs.push_back(r);
  }
	
  boost::shared_ptr<HMMVector<float_type, sse_float_type> > pi_ptr(new HMMVector<float_type, sse_float_type>(no_states));
  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > T_ptr(new HMMMatrix<float_type, sse_float_type>(no_states, no_states));
  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > E_ptr(new HMMMatrix<float_type, sse_float_type>(alphabet_size, no_states));
	
  HMMVector<float_type, sse_float_type> &pi = *pi_ptr;
  HMMMatrix<float_type, sse_float_type> &T = *T_ptr;
  HMMMatrix<float_type, sse_float_type> &E = *E_ptr;
	
  for(int i = 0; i < no_states; ++i)
    pi(i) = random();
  double sum = pi.sum();
  pi /= sum;
	
  for(int r = 0; r < no_states; ++r) {
    for(int c = 0; c < no_states; ++c)
      T(r,c) = random();
    sum = T.row_sum(r);
    for(int c = 0; c < no_states; ++c)
      T(r,c) /= sum;
  }
	
  for(int c = 0; c < no_states; ++c) {
    for(int r = 0; r < alphabet_size; ++r)
      E(r,c) = random();
    sum = T.column_sum(c);
    for(int r = 0; r < alphabet_size; ++r)
      E(r,c) /= sum;
  }
	
  HMM<float_type, sse_float_type> hmm(pi_ptr, T_ptr, E_ptr);

  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > F_ptr(new HMMMatrix<float_type, sse_float_type>(obsseq_length, no_states));
  HMMMatrix<float_type, sse_float_type> &F = *F_ptr;
  boost::shared_ptr<HMMVector<float_type, sse_float_type> > scales_ptr(new HMMVector<float_type, sse_float_type>(obsseq_length));
  HMMVector<float_type, sse_float_type> &scales = *scales_ptr;

  hmm.forward(obs, scales, F);

  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > B_ptr(new HMMMatrix<float_type, sse_float_type>(obsseq_length, no_states));
  HMMMatrix<float_type, sse_float_type> &B = *B_ptr;

  omp_set_num_threads(1);
  hmm.backward(obs, scales, B);
  float_type v11 = B(obsseq_length-1, 3);
  float_type v12 = B(obsseq_length-1, 64);

  omp_set_num_threads(2);
  hmm.backward(obs, scales, B);
  float_type v21 = B(obsseq_length-1, 3);
  float_type v22 = B(obsseq_length-1, 64);

  omp_set_num_threads(4);
  hmm.backward(obs, scales, B);
  float_type v41 = B(obsseq_length-1, 3);
  float_type v42 = B(obsseq_length-1, 64);

  omp_set_num_threads(8);
  hmm.backward(obs, scales, B);
  float_type v81 = B(obsseq_length-1, 3);
  float_type v82 = B(obsseq_length-1, 64);

  BOOST_CHECK_CLOSE(v11, v21, 0.01);
  BOOST_CHECK_CLOSE(v11, v41, 0.01);
  BOOST_CHECK_CLOSE(v11, v81, 0.01);
  BOOST_CHECK_CLOSE(v12, v22, 0.01);
  BOOST_CHECK_CLOSE(v12, v42, 0.01);
  BOOST_CHECK_CLOSE(v12, v82, 0.01);
}
#endif

template <typename float_type>
void sse_vs_nonSse_backward() {

  typedef typename FloatTraits<float_type>::sse_type sse_float_type;

  int obsseq_length = 133;
  int no_states = 128;
  int alphabet_size = 16;

  vector<unsigned int> obs;
  for(int i = 0; i < obsseq_length; ++i) {
    unsigned r = random() % alphabet_size;
    obs.push_back(r);
  }
	
  boost::shared_ptr<HMMVector<float_type, float_type> > pi_ptr(new HMMVector<float_type, float_type>(no_states));
  boost::shared_ptr<HMMMatrix<float_type, float_type> > T_ptr(new HMMMatrix<float_type, float_type>(no_states, no_states));
  boost::shared_ptr<HMMMatrix<float_type, float_type> > E_ptr(new HMMMatrix<float_type, float_type>(alphabet_size, no_states));

  boost::shared_ptr<HMMVector<float_type, sse_float_type> > pi_ptr_sse(new HMMVector<float_type, sse_float_type>(no_states));
  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > T_ptr_sse(new HMMMatrix<float_type, sse_float_type>(no_states, no_states));
  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > E_ptr_sse(new HMMMatrix<float_type, sse_float_type>(alphabet_size, no_states));

	
  HMMVector<float_type, float_type> &pi = *pi_ptr;
  HMMMatrix<float_type, float_type> &T = *T_ptr;
  HMMMatrix<float_type, float_type> &E = *E_ptr;

  HMMVector<float_type, sse_float_type> &pi_sse = *pi_ptr_sse;
  HMMMatrix<float_type, sse_float_type> &T_sse = *T_ptr_sse;
  HMMMatrix<float_type, sse_float_type> &E_sse = *E_ptr_sse;
	
  for(int i = 0; i < no_states; ++i) {
    pi(i) = random();
    pi_sse(i) = pi(i);
  }
  double sum = pi.sum();
  pi /= sum;
  pi_sse /= sum;
	
  for(int r = 0; r < no_states; ++r) {
    for(int c = 0; c < no_states; ++c) {
      T(r,c) = random();
    }
    sum = T.row_sum(r);
    for(int c = 0; c < no_states; ++c) {
      T(r,c) /= sum;
      T_sse(r,c) = T(r,c);
    }
  }
	
  for(int c = 0; c < no_states; ++c) {
    for(int r = 0; r < alphabet_size; ++r) {
      E(r,c) = random();
    }
    sum = T.column_sum(c);
    for(int r = 0; r < alphabet_size; ++r) {
      E(r,c) /= sum;
      E_sse(r,c) = E(r,c);
    }
  }
	
  HMM<float_type, float_type> hmm(pi_ptr, T_ptr, E_ptr);

  HMM<float_type, sse_float_type> hmm_sse(pi_ptr_sse, T_ptr_sse, E_ptr_sse);

  boost::shared_ptr<HMMMatrix<float_type, float_type> > F_ptr(new HMMMatrix<float_type, float_type>(obsseq_length, no_states));
  HMMMatrix<float_type, float_type> &F = *F_ptr;
  boost::shared_ptr<HMMVector<float_type, float_type> > scales_ptr(new HMMVector<float_type, float_type>(obsseq_length));
  HMMVector<float_type, float_type> &scales = *scales_ptr;

  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > F_ptr_sse(new HMMMatrix<float_type, sse_float_type>(obsseq_length, no_states));
  HMMMatrix<float_type, sse_float_type> &F_sse = *F_ptr_sse;
  boost::shared_ptr<HMMVector<float_type, sse_float_type> > scales_ptr_sse(new HMMVector<float_type, sse_float_type>(obsseq_length));
  HMMVector<float_type, sse_float_type> &scales_sse = *scales_ptr_sse;

  hmm.forward(obs, scales, F);
  hmm_sse.forward(obs, scales_sse, F_sse);

  boost::shared_ptr<HMMMatrix<float_type, float_type> > B_ptr(new HMMMatrix<float_type, float_type>(obsseq_length, no_states));
  HMMMatrix<float_type, float_type> &B = *B_ptr;

  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > B_ptr_sse(new HMMMatrix<float_type, sse_float_type>(obsseq_length, no_states));
  HMMMatrix<float_type, sse_float_type> &B_sse = *B_ptr_sse;

  hmm.backward(obs, scales, B);
  hmm_sse.backward(obs, scales_sse, B_sse);

  float_type v1 = B(obsseq_length-1, 3);
  float_type v2 = B(obsseq_length-1, 64);
  float_type v1_sse = B_sse(obsseq_length-1, 3);
  float_type v2_sse = B_sse(obsseq_length-1, 64);

  BOOST_CHECK_CLOSE(v1, v1_sse, 0.01);
  BOOST_CHECK_CLOSE(v2, v2_sse, 0.01);
}

// ############ BAUM WELCH TESTS ############### //
template <typename float_type, typename sse_float_type>
void baum_welch_2x2_with_transitions() {
  int no_states = 2;
  int alphabet_size = 2;
  int seq_length = 4;
	
  boost::shared_ptr<HMMVector<float_type, sse_float_type> > pi_ptr(new HMMVector<float_type, sse_float_type>(no_states));
  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > T_ptr(new HMMMatrix<float_type, sse_float_type>(no_states,no_states));
  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > E_ptr(new HMMMatrix<float_type, sse_float_type>(alphabet_size, no_states));
	
  HMMVector<float_type, sse_float_type> &pi = *pi_ptr;
  HMMMatrix<float_type, sse_float_type> &T = *T_ptr;
  HMMMatrix<float_type, sse_float_type> &E = *E_ptr;

  // initial state probabilities
  pi(0)  = 0.2; pi(1) = 0.8;

  // transitions from state 0
  T(0,0) = 0.1; T(0,1) = 0.9;
  // transitions from state 1
  T(1,0) = 0.9; T(1,1) = 0.1;
	
  // emissions from state 0
  E(0,0) = 0.25; E(1,0) = 0.75;
  // emissions from state 1
  E(0,1) = 0.25; E(1,1) = 0.75;
	
  HMM<float_type, sse_float_type> hmm(pi_ptr, T_ptr, E_ptr);
	
  unsigned obs_array[] = { 0, 1, 0, 1 };
  sequence obsseq(obs_array, obs_array + seq_length);
	
  HMMMatrix<float_type, sse_float_type> F(seq_length, no_states,seq_length);
  HMMMatrix<float_type, sse_float_type> B(seq_length,no_states);
  HMMVector<float_type, sse_float_type> scales(seq_length);
	
  hmm.forward(obsseq, scales, F);
  hmm.backward(obsseq, scales, B);
	
  HMMVector<float_type, sse_float_type> new_pi(no_states);
  HMMMatrix<float_type, sse_float_type> new_T(no_states,no_states);
  HMMMatrix<float_type, sse_float_type> new_E(alphabet_size,no_states);
	
  hmm.baum_welch(obsseq, F, B, scales, new_pi, new_T, new_E);

  // check that new_pi sums to 1
  double sum = 0;
  for (int s = 0; s < new_pi.get_size(); ++s)
    sum += new_pi(s);
  BOOST_CHECK_CLOSE(sum, 1.0, 0.0001);
	
  // check that all rows of new_T sums to 1
  for (int r = 0; r < new_T.get_no_rows(); ++r) {
    sum = 0.0;
    for (int c = 0; c < new_T.get_no_columns(); ++c) {
      sum += new_T(r,c);
    }
    BOOST_CHECK_CLOSE(sum, 1.0, 0.0001);
  }

  // check that all columns of new_E sums to 1
  for (int c = 0; c < new_E.get_no_columns(); ++c) {
    sum = 0;
    for (int r = 0; r < new_E.get_no_rows(); ++r) {
      sum += new_E(r,c);
    }
    BOOST_CHECK_CLOSE(sum, 1.0, 0.0001);
  }
}

#ifdef WITH_OMP
template <typename float_type, typename sse_float_type>
void parallel_bw() {
  int obsseq_length = 100;
  int no_states = 128;
  int alphabet_size = 16;

  vector<unsigned int> obs;
  for(int i = 0; i < obsseq_length; ++i) {
    unsigned r = random() % alphabet_size;
    obs.push_back(r);
  }
	
  boost::shared_ptr<HMMVector<float_type, sse_float_type> > pi_ptr(new HMMVector<float_type, sse_float_type>(no_states));
  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > T_ptr(new HMMMatrix<float_type, sse_float_type>(no_states, no_states));
  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > E_ptr(new HMMMatrix<float_type, sse_float_type>(alphabet_size, no_states));
	
  HMMVector<float_type, sse_float_type> &pi = *pi_ptr;
  HMMMatrix<float_type, sse_float_type> &T = *T_ptr;
  HMMMatrix<float_type, sse_float_type> &E = *E_ptr;
	
  for(int i = 0; i < no_states; ++i)
    pi(i) = random();
  double sum = pi.sum();
  pi /= sum;
	
  for(int r = 0; r < no_states; ++r) {
    for(int c = 0; c < no_states; ++c) 
      T(r,c) = random();
    sum = T.row_sum(r);
    for(int c = 0; c < no_states; ++c)
      T(r,c) /= sum;
  }
	
  for(int c = 0; c < no_states; ++c) {
    for(int r = 0; r < alphabet_size; ++r)
      E(r,c) = random();
    sum = T.column_sum(c);
    for(int r = 0; r < alphabet_size; ++r)
      E(r,c) /= sum;
  }
	
  HMM<float_type, sse_float_type> hmm(pi_ptr, T_ptr, E_ptr);

  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > F_ptr(new HMMMatrix<float_type, sse_float_type>(obsseq_length, no_states));
  HMMMatrix<float_type, sse_float_type> &F = *F_ptr;
  boost::shared_ptr<HMMVector<float_type, sse_float_type> > scales_ptr(new HMMVector<float_type, sse_float_type>(obsseq_length));
  HMMVector<float_type, sse_float_type> &scales = *scales_ptr;

  hmm.forward(obs, scales, F);

  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > B_ptr(new HMMMatrix<float_type, sse_float_type>(obsseq_length, no_states));
  HMMMatrix<float_type, sse_float_type> &B = *B_ptr;

  hmm.backward(obs, scales, B);

  HMMVector<float_type, sse_float_type> pi_counts(no_states);
  HMMMatrix<float_type, sse_float_type> T_counts(no_states,no_states);
  HMMMatrix<float_type, sse_float_type> E_counts(alphabet_size,no_states);
	
  omp_set_num_threads(1);
  hmm.baum_welch(obs, F, B, scales, pi_counts, T_counts, E_counts);
  float_type pi11 = pi_counts(5);
  float_type pi12 = pi_counts(120);
  float_type T11 = T_counts(64, 45);
  float_type T12 = T_counts(1, 120);
  float_type E11 = E_counts(8, 45);
  float_type E12 = E_counts(1, 120);

  omp_set_num_threads(2);
  hmm.baum_welch(obs, F, B, scales, pi_counts, T_counts, E_counts);
  float_type pi21 = pi_counts(5);
  float_type pi22 = pi_counts(120);
  float_type T21 = T_counts(64, 45);
  float_type T22 = T_counts(1, 120);
  float_type E21 = E_counts(8, 45);
  float_type E22 = E_counts(1, 120);

  omp_set_num_threads(4);
  hmm.baum_welch(obs, F, B, scales, pi_counts, T_counts, E_counts);
  float_type pi41 = pi_counts(5);
  float_type pi42 = pi_counts(120);
  float_type T41 = T_counts(64, 45);
  float_type T42 = T_counts(1, 120);
  float_type E41 = E_counts(8, 45);
  float_type E42 = E_counts(1, 120);

  omp_set_num_threads(8);
  hmm.baum_welch(obs, F, B, scales, pi_counts, T_counts, E_counts);
  float_type pi81 = pi_counts(5);
  float_type pi82 = pi_counts(120);
  float_type T81 = T_counts(64, 45);
  float_type T82 = T_counts(1, 120);
  float_type E81 = E_counts(8, 45);
  float_type E82 = E_counts(1, 120);

  BOOST_CHECK_CLOSE(pi11, pi21, 0.01);
  BOOST_CHECK_CLOSE(pi11, pi41, 0.01);
  BOOST_CHECK_CLOSE(pi11, pi81, 0.01);

  BOOST_CHECK_CLOSE(pi12, pi22, 0.01);
  BOOST_CHECK_CLOSE(pi12, pi42, 0.01);
  BOOST_CHECK_CLOSE(pi12, pi82, 0.01);

  BOOST_CHECK_CLOSE(T11, T21, 0.01);
  BOOST_CHECK_CLOSE(T11, T41, 0.01);
  BOOST_CHECK_CLOSE(T11, T81, 0.01);

  BOOST_CHECK_CLOSE(T12, T22, 0.01);
  BOOST_CHECK_CLOSE(T12, T42, 0.01);
  BOOST_CHECK_CLOSE(T12, T82, 0.01);

  BOOST_CHECK_CLOSE(E11, E21, 0.01);
  BOOST_CHECK_CLOSE(E11, E41, 0.01);
  BOOST_CHECK_CLOSE(E11, E81, 0.01);

  BOOST_CHECK_CLOSE(E12, E22, 0.01);
  BOOST_CHECK_CLOSE(E12, E42, 0.01);
  BOOST_CHECK_CLOSE(E12, E82, 0.01);
}
#endif

template <typename float_type>
void sse_vs_nonSse_bw() {

  typedef typename FloatTraits<float_type>::sse_type sse_float_type;

  int obsseq_length = 133;
  int no_states = 128;
  int alphabet_size = 16;

  vector<unsigned int> obs;
  for(int i = 0; i < obsseq_length; ++i) {
    unsigned r = random() % alphabet_size;
    obs.push_back(r);
  }
	
  boost::shared_ptr<HMMVector<float_type, float_type> > pi_ptr(new HMMVector<float_type, float_type>(no_states));
  boost::shared_ptr<HMMMatrix<float_type, float_type> > T_ptr(new HMMMatrix<float_type, float_type>(no_states, no_states));
  boost::shared_ptr<HMMMatrix<float_type, float_type> > E_ptr(new HMMMatrix<float_type, float_type>(alphabet_size, no_states));

  boost::shared_ptr<HMMVector<float_type, sse_float_type> > pi_ptr_sse(new HMMVector<float_type, sse_float_type>(no_states));
  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > T_ptr_sse(new HMMMatrix<float_type, sse_float_type>(no_states, no_states));
  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > E_ptr_sse(new HMMMatrix<float_type, sse_float_type>(alphabet_size, no_states));

	
  HMMVector<float_type, float_type> &pi = *pi_ptr;
  HMMMatrix<float_type, float_type> &T = *T_ptr;
  HMMMatrix<float_type, float_type> &E = *E_ptr;

  HMMVector<float_type, sse_float_type> &pi_sse = *pi_ptr_sse;
  HMMMatrix<float_type, sse_float_type> &T_sse = *T_ptr_sse;
  HMMMatrix<float_type, sse_float_type> &E_sse = *E_ptr_sse;
	
  for(int i = 0; i < no_states; ++i) {
    pi(i) = random();
    pi_sse(i) = pi(i);
  }
  double sum = pi.sum();
  pi /= sum;
  pi_sse /= sum;
	
  for(int r = 0; r < no_states; ++r) {
    for(int c = 0; c < no_states; ++c) {
      T(r,c) = random();
    }
    sum = T.row_sum(r);
    for(int c = 0; c < no_states; ++c) {
      T(r,c) /= sum;
      T_sse(r,c) = T(r,c);
    }
  }
	
  for(int c = 0; c < no_states; ++c) {
    for(int r = 0; r < alphabet_size; ++r) {
      E(r,c) = random();
    }
    sum = T.column_sum(c);
    for(int r = 0; r < alphabet_size; ++r) {
      E(r,c) /= sum;
      E_sse(r,c) = E(r,c);
    }
  }
	
  HMM<float_type, float_type> hmm(pi_ptr, T_ptr, E_ptr);

  HMM<float_type, sse_float_type> hmm_sse(pi_ptr_sse, T_ptr_sse, E_ptr_sse);

  boost::shared_ptr<HMMMatrix<float_type, float_type> > F_ptr(new HMMMatrix<float_type, float_type>(obsseq_length, no_states));
  HMMMatrix<float_type, float_type> &F = *F_ptr;
  boost::shared_ptr<HMMVector<float_type, float_type> > scales_ptr(new HMMVector<float_type, float_type>(obsseq_length));
  HMMVector<float_type, float_type> &scales = *scales_ptr;

  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > F_ptr_sse(new HMMMatrix<float_type, sse_float_type>(obsseq_length, no_states));
  HMMMatrix<float_type, sse_float_type> &F_sse = *F_ptr_sse;
  boost::shared_ptr<HMMVector<float_type, sse_float_type> > scales_ptr_sse(new HMMVector<float_type, sse_float_type>(obsseq_length));
  HMMVector<float_type, sse_float_type> &scales_sse = *scales_ptr_sse;

  hmm.forward(obs, scales, F);
  hmm_sse.forward(obs, scales_sse, F_sse);

  boost::shared_ptr<HMMMatrix<float_type, float_type> > B_ptr(new HMMMatrix<float_type, float_type>(obsseq_length, no_states));
  HMMMatrix<float_type, float_type> &B = *B_ptr;

  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > B_ptr_sse(new HMMMatrix<float_type, sse_float_type>(obsseq_length, no_states));
  HMMMatrix<float_type, sse_float_type> &B_sse = *B_ptr_sse;

  hmm.backward(obs, scales, B);
  hmm_sse.backward(obs, scales_sse, B_sse);

  HMMVector<float_type, float_type> pi_counts(no_states);
  HMMMatrix<float_type, float_type> T_counts(no_states,no_states);
  HMMMatrix<float_type, float_type> E_counts(alphabet_size,no_states);

  HMMVector<float_type, sse_float_type> pi_counts_sse(no_states);
  HMMMatrix<float_type, sse_float_type> T_counts_sse(no_states,no_states);
  HMMMatrix<float_type, sse_float_type> E_counts_sse(alphabet_size,no_states);

  hmm.baum_welch(obs, F, B, scales, pi_counts, T_counts, E_counts);
  hmm_sse.baum_welch(obs, F_sse, B_sse, scales_sse, pi_counts_sse, T_counts_sse, E_counts_sse);

  float_type pi1 = pi_counts(5);
  float_type pi2 = pi_counts(120);
  float_type T1 = T_counts(64, 45);
  float_type T2 = T_counts(1, 120);
  float_type E1 = E_counts(8, 45);
  float_type E2 = E_counts(1, 120);

  float_type pi1_sse = pi_counts_sse(5);
  float_type pi2_sse = pi_counts_sse(120);
  float_type T1_sse = T_counts_sse(64, 45);
  float_type T2_sse = T_counts_sse(1, 120);
  float_type E1_sse = E_counts_sse(8, 45);
  float_type E2_sse = E_counts_sse(1, 120);

  BOOST_CHECK_CLOSE(pi1, pi1_sse, 0.01);
  BOOST_CHECK_CLOSE(pi2, pi2_sse, 0.01);
  BOOST_CHECK_CLOSE(T1, T1_sse, 0.01);
  BOOST_CHECK_CLOSE(T2, T2_sse, 0.01);
  BOOST_CHECK_CLOSE(E1, E1_sse, 0.01);
  BOOST_CHECK_CLOSE(E2, E2_sse, 0.01);
}

// ############ VITERBI TESTS ############### //
template <typename float_type, typename sse_float_type>
void viterbi_2x2_with_transitions() {
  int no_states = 2;
  int alphabet_size = 2;
  int seq_length = 4;
  
  boost::shared_ptr<HMMVector<float_type, sse_float_type> > pi_ptr(new HMMVector<float_type, sse_float_type>(no_states));
  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > T_ptr(new HMMMatrix<float_type, sse_float_type>(no_states,no_states));
  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > E_ptr(new HMMMatrix<float_type, sse_float_type>(alphabet_size, no_states));
	
  HMMVector<float_type, sse_float_type> &pi = *pi_ptr;
  HMMMatrix<float_type, sse_float_type> &T = *T_ptr;
  HMMMatrix<float_type, sse_float_type> &E = *E_ptr;
	
  pi(0)  = 0.2; pi(1) = 0.8;

  // transitions from state 0
  T(0,0) = 0.1; T(0,1) = 0.9;
  // transitions from state 1
  T(1,0) = 0.9; T(1,1) = 0.1;
	
  // emissions from state 0
  E(0,0) = 0.25; E(1,0) = 0.75;
  // emissions from state 1
  E(0,1) = 0.25; E(1,1) = 0.75;
	
  HMM<float_type, sse_float_type> hmm(pi_ptr, T_ptr, E_ptr);
	
  unsigned obs_array[] = { 0, 1, 0, 1 };
  sequence obsseq(obs_array, obs_array + seq_length);
  sequence hiddenseq(seq_length);

  hmm.viterbi(obsseq, hiddenseq);

  BOOST_CHECK_EQUAL(hiddenseq[0], 1u);
  BOOST_CHECK_EQUAL(hiddenseq[1], 0u);
  BOOST_CHECK_EQUAL(hiddenseq[2], 1u);
  BOOST_CHECK_EQUAL(hiddenseq[3], 0u);
}

#ifdef WITH_OMP
template <typename float_type, typename sse_float_type>
void parallel_viterbi() {
  int obsseq_length = 100;
  int no_states = 128;
  int alphabet_size = 16;

  vector<unsigned int> obs;
  for(int i = 0; i < obsseq_length; ++i) {
    unsigned r = random() % alphabet_size;
    obs.push_back(r);
  }
	
  boost::shared_ptr<HMMVector<float_type, sse_float_type> > pi_ptr(new HMMVector<float_type, sse_float_type>(no_states));
  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > T_ptr(new HMMMatrix<float_type, sse_float_type>(no_states, no_states));
  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > E_ptr(new HMMMatrix<float_type, sse_float_type>(alphabet_size, no_states));
	
  HMMVector<float_type, sse_float_type> &pi = *pi_ptr;
  HMMMatrix<float_type, sse_float_type> &T = *T_ptr;
  HMMMatrix<float_type, sse_float_type> &E = *E_ptr;
	
  for(int i = 0; i < no_states; ++i)
    pi(i) = random();
  double sum = pi.sum();
  pi /= sum;
	
  for(int r = 0; r < no_states; ++r) {
    for(int c = 0; c < no_states; ++c)
      T(r,c) = random();
    sum = T.row_sum(r);
    for(int c = 0; c < no_states; ++c)
      T(r,c) /= sum;
  }
	
  for(int c = 0; c < no_states; ++c) {
    for(int r = 0; r < alphabet_size; ++r)
      E(r,c) = random();
    sum = T.column_sum(c);
    for(int r = 0; r < alphabet_size; ++r)
      E(r,c) /= sum;
  }
	
  HMM<float_type, sse_float_type> hmm(pi_ptr, T_ptr, E_ptr);

  sequence hidden_seq(obsseq_length);
  omp_set_num_threads(1);
  hmm.viterbi(obs, hidden_seq);
  unsigned s11 = hidden_seq[obsseq_length -1];
  unsigned s12 = hidden_seq[64];

  omp_set_num_threads(2);
  hmm.viterbi(obs, hidden_seq);
  unsigned s21 = hidden_seq[obsseq_length -1];
  unsigned s22 = hidden_seq[64];

  omp_set_num_threads(4);
  hmm.viterbi(obs, hidden_seq);
  unsigned s41 = hidden_seq[obsseq_length -1];
  unsigned s42 = hidden_seq[64];

  omp_set_num_threads(8);
  hmm.viterbi(obs, hidden_seq);
  unsigned s81 = hidden_seq[obsseq_length -1];
  unsigned s82 = hidden_seq[64];

  BOOST_CHECK_EQUAL(s11, s21);
  BOOST_CHECK_EQUAL(s11, s41);
  BOOST_CHECK_EQUAL(s11, s81);
  BOOST_CHECK_EQUAL(s12, s22);
  BOOST_CHECK_EQUAL(s12, s42);
  BOOST_CHECK_EQUAL(s12, s82);
}
#endif

template <typename float_type>
void sse_vs_nonSse_viterbi() {

  typedef typename FloatTraits<float_type>::sse_type sse_float_type;

  int obsseq_length = 175;
  int no_states = 128;
  int alphabet_size = 16;

  vector<unsigned int> obs;
  for(int i = 0; i < obsseq_length; ++i) {
    unsigned r = random() % alphabet_size;
    obs.push_back(r);
  }
	
  boost::shared_ptr<HMMVector<float_type, float_type> > pi_ptr(new HMMVector<float_type, float_type>(no_states));
  boost::shared_ptr<HMMMatrix<float_type, float_type> > T_ptr(new HMMMatrix<float_type, float_type>(no_states, no_states));
  boost::shared_ptr<HMMMatrix<float_type, float_type> > E_ptr(new HMMMatrix<float_type, float_type>(alphabet_size, no_states));

  boost::shared_ptr<HMMVector<float_type, sse_float_type> > pi_ptr_sse(new HMMVector<float_type, sse_float_type>(no_states));
  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > T_ptr_sse(new HMMMatrix<float_type, sse_float_type>(no_states, no_states));
  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > E_ptr_sse(new HMMMatrix<float_type, sse_float_type>(alphabet_size, no_states));

	
  HMMVector<float_type, float_type> &pi = *pi_ptr;
  HMMMatrix<float_type, float_type> &T = *T_ptr;
  HMMMatrix<float_type, float_type> &E = *E_ptr;

  HMMVector<float_type, sse_float_type> &pi_sse = *pi_ptr_sse;
  HMMMatrix<float_type, sse_float_type> &T_sse = *T_ptr_sse;
  HMMMatrix<float_type, sse_float_type> &E_sse = *E_ptr_sse;
	
  for(int i = 0; i < no_states; ++i) {
    pi(i) = random();
    pi_sse(i) = pi(i);
  }
  double sum = pi.sum();
  pi /= sum;
  pi_sse /= sum;
	
  for(int r = 0; r < no_states; ++r) {
    for(int c = 0; c < no_states; ++c) {
      T(r,c) = random();
    }
    sum = T.row_sum(r);
    for(int c = 0; c < no_states; ++c) {
      T(r,c) /= sum;
      T_sse(r,c) = T(r,c);
    }
  }
	
  for(int c = 0; c < no_states; ++c) {
    for(int r = 0; r < alphabet_size; ++r) {
      E(r,c) = random();
    }
    sum = T.column_sum(c);
    for(int r = 0; r < alphabet_size; ++r) {
      E(r,c) /= sum;
      E_sse(r,c) = E(r,c);
    }
  }
	
  HMM<float_type, float_type> hmm(pi_ptr, T_ptr, E_ptr);
  HMM<float_type, sse_float_type> hmm_sse(pi_ptr_sse, T_ptr_sse, E_ptr_sse);

  sequence hidden_seq(obsseq_length);
  sequence hidden_seq_sse(obsseq_length);
  
  hmm.viterbi(obs, hidden_seq);
  hmm_sse.viterbi(obs, hidden_seq_sse);

  unsigned s1 = hidden_seq[obsseq_length -1];
  unsigned s2 = hidden_seq[64];
  unsigned s3 = hidden_seq[86];
  unsigned s4 = hidden_seq[24];

  unsigned s1_sse = hidden_seq_sse[obsseq_length -1];
  unsigned s2_sse = hidden_seq_sse[64];
  unsigned s3_sse = hidden_seq_sse[86];
  unsigned s4_sse = hidden_seq_sse[24];

  BOOST_CHECK_EQUAL(s1, s1_sse);
  BOOST_CHECK_EQUAL(s2, s2_sse);
  BOOST_CHECK_EQUAL(s3, s3_sse);
  BOOST_CHECK_EQUAL(s4, s4_sse);
}

// ############ POSTERIOR DECODING TESTS ############### //
#ifdef WITH_OMP
template <typename float_type, typename sse_float_type>
void parallel_pd() {
  int obsseq_length = 100;
  int no_states = 128;
  int alphabet_size = 16;

  vector<unsigned int> obs;
  for(int i = 0; i < obsseq_length; ++i) {
    unsigned r = random() % alphabet_size;
    obs.push_back(r);
  }
	
  boost::shared_ptr<HMMVector<float_type, sse_float_type> > pi_ptr(new HMMVector<float_type, sse_float_type>(no_states));
  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > T_ptr(new HMMMatrix<float_type, sse_float_type>(no_states, no_states));
  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > E_ptr(new HMMMatrix<float_type, sse_float_type>(alphabet_size, no_states));
	
  HMMVector<float_type, sse_float_type> &pi = *pi_ptr;
  HMMMatrix<float_type, sse_float_type> &T = *T_ptr;
  HMMMatrix<float_type, sse_float_type> &E = *E_ptr;
	
  for(int i = 0; i < no_states; ++i)
    pi(i) = random();
  double sum = pi.sum();
  pi /= sum;
	
  for(int r = 0; r < no_states; ++r) {
    for(int c = 0; c < no_states; ++c)
      T(r,c) = random();
    sum = T.row_sum(r);
    for(int c = 0; c < no_states; ++c)
      T(r,c) /= sum;
  }
	
  for(int c = 0; c < no_states; ++c) {
    for(int r = 0; r < alphabet_size; ++r)
      E(r,c) = random();
    sum = T.column_sum(c);
    for(int r = 0; r < alphabet_size; ++r)
      E(r,c) /= sum;
  }
	
  HMM<float_type, sse_float_type> hmm(pi_ptr, T_ptr, E_ptr);

  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > F_ptr(new HMMMatrix<float_type, sse_float_type>(obsseq_length, no_states));
  HMMMatrix<float_type, sse_float_type> &F = *F_ptr;
  boost::shared_ptr<HMMVector<float_type, sse_float_type> > scales_ptr(new HMMVector<float_type, sse_float_type>(obsseq_length));
  HMMVector<float_type, sse_float_type> &scales = *scales_ptr;

  hmm.forward(obs, scales, F);

  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > B_ptr(new HMMMatrix<float_type, sse_float_type>(obsseq_length, no_states));
  HMMMatrix<float_type, sse_float_type> &B = *B_ptr;

  hmm.backward(obs, scales, B);

  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > pd_ptr(new HMMMatrix<float_type, sse_float_type>(obsseq_length, no_states));
  HMMMatrix<float_type, sse_float_type> &pd = *pd_ptr;

  omp_set_num_threads(1);
  hmm.posterior_decoding(obs, F, B, scales, pd);
  float_type pd11 = pd(120, 8);
  float_type pd12 = pd(127, 56);

  omp_set_num_threads(2);
  hmm.posterior_decoding(obs, F, B, scales, pd);
  float_type pd21 = pd(120, 8);
  float_type pd22 = pd(127, 56);

  omp_set_num_threads(4);
  hmm.posterior_decoding(obs, F, B, scales, pd);
  float_type pd41 = pd(120, 8);
  float_type pd42 = pd(127, 56);

  omp_set_num_threads(8);
  hmm.posterior_decoding(obs, F, B, scales, pd);
  float_type pd81 = pd(120, 8);
  float_type pd82 = pd(127, 56);

  BOOST_CHECK_CLOSE(pd11, pd21, 0.01);
  BOOST_CHECK_CLOSE(pd11, pd41, 0.01);
  BOOST_CHECK_CLOSE(pd11, pd81, 0.01);

  BOOST_CHECK_CLOSE(pd12, pd22, 0.01);
  BOOST_CHECK_CLOSE(pd12, pd42, 0.01);
  BOOST_CHECK_CLOSE(pd12, pd82, 0.01);
}
#endif

template <typename float_type>
void sse_vs_nonSse_pd() {

  typedef typename FloatTraits<float_type>::sse_type sse_float_type;

  int obsseq_length = 133;
  int no_states = 128;
  int alphabet_size = 16;

  vector<unsigned int> obs;
  for(int i = 0; i < obsseq_length; ++i) {
    unsigned r = random() % alphabet_size;
    obs.push_back(r);
  }
	
  boost::shared_ptr<HMMVector<float_type, float_type> > pi_ptr(new HMMVector<float_type, float_type>(no_states));
  boost::shared_ptr<HMMMatrix<float_type, float_type> > T_ptr(new HMMMatrix<float_type, float_type>(no_states, no_states));
  boost::shared_ptr<HMMMatrix<float_type, float_type> > E_ptr(new HMMMatrix<float_type, float_type>(alphabet_size, no_states));

  boost::shared_ptr<HMMVector<float_type, sse_float_type> > pi_ptr_sse(new HMMVector<float_type, sse_float_type>(no_states));
  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > T_ptr_sse(new HMMMatrix<float_type, sse_float_type>(no_states, no_states));
  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > E_ptr_sse(new HMMMatrix<float_type, sse_float_type>(alphabet_size, no_states));

	
  HMMVector<float_type, float_type> &pi = *pi_ptr;
  HMMMatrix<float_type, float_type> &T = *T_ptr;
  HMMMatrix<float_type, float_type> &E = *E_ptr;

  HMMVector<float_type, sse_float_type> &pi_sse = *pi_ptr_sse;
  HMMMatrix<float_type, sse_float_type> &T_sse = *T_ptr_sse;
  HMMMatrix<float_type, sse_float_type> &E_sse = *E_ptr_sse;
	
  for(int i = 0; i < no_states; ++i) {
    pi(i) = random();
    pi_sse(i) = pi(i);
  }
  double sum = pi.sum();
  pi /= sum;
  pi_sse /= sum;
	
  for(int r = 0; r < no_states; ++r) {
    for(int c = 0; c < no_states; ++c) {
      T(r,c) = random();
    }
    sum = T.row_sum(r);
    for(int c = 0; c < no_states; ++c) {
      T(r,c) /= sum;
      T_sse(r,c) = T(r,c);
    }
  }
	
  for(int c = 0; c < no_states; ++c) {
    for(int r = 0; r < alphabet_size; ++r) {
      E(r,c) = random();
    }
    sum = T.column_sum(c);
    for(int r = 0; r < alphabet_size; ++r) {
      E(r,c) /= sum;
      E_sse(r,c) = E(r,c);
    }
  }
	
  HMM<float_type, float_type> hmm(pi_ptr, T_ptr, E_ptr);

  HMM<float_type, sse_float_type> hmm_sse(pi_ptr_sse, T_ptr_sse, E_ptr_sse);

  boost::shared_ptr<HMMMatrix<float_type, float_type> > F_ptr(new HMMMatrix<float_type, float_type>(obsseq_length, no_states));
  HMMMatrix<float_type, float_type> &F = *F_ptr;
  boost::shared_ptr<HMMVector<float_type, float_type> > scales_ptr(new HMMVector<float_type, float_type>(obsseq_length));
  HMMVector<float_type, float_type> &scales = *scales_ptr;

  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > F_ptr_sse(new HMMMatrix<float_type, sse_float_type>(obsseq_length, no_states));
  HMMMatrix<float_type, sse_float_type> &F_sse = *F_ptr_sse;
  boost::shared_ptr<HMMVector<float_type, sse_float_type> > scales_ptr_sse(new HMMVector<float_type, sse_float_type>(obsseq_length));
  HMMVector<float_type, sse_float_type> &scales_sse = *scales_ptr_sse;

  hmm.forward(obs, scales, F);
  hmm_sse.forward(obs, scales_sse, F_sse);

  boost::shared_ptr<HMMMatrix<float_type, float_type> > B_ptr(new HMMMatrix<float_type, float_type>(obsseq_length, no_states));
  HMMMatrix<float_type, float_type> &B = *B_ptr;

  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > B_ptr_sse(new HMMMatrix<float_type, sse_float_type>(obsseq_length, no_states));
  HMMMatrix<float_type, sse_float_type> &B_sse = *B_ptr_sse;

  hmm.backward(obs, scales, B);
  hmm_sse.backward(obs, scales_sse, B_sse);

  boost::shared_ptr<HMMMatrix<float_type, float_type> > pd_ptr(new HMMMatrix<float_type, float_type>(obsseq_length, no_states));
  HMMMatrix<float_type, float_type> &pd = *pd_ptr;

  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > pd_ptr_sse(new HMMMatrix<float_type, sse_float_type>(obsseq_length, no_states));
  HMMMatrix<float_type, sse_float_type> &pd_sse = *pd_ptr_sse;

  hmm.posterior_decoding(obs, F, B, scales, pd);
  hmm_sse.posterior_decoding(obs, F_sse, B_sse, scales_sse, pd_sse);

  float_type pd1 = pd(120, 8);
  float_type pd2 = pd(127, 56);
  float_type pd3 = pd(50, 76);
  float_type pd4 = pd(87, 12);

  float_type pd1_sse = pd_sse(120, 8);
  float_type pd2_sse = pd_sse(127, 56);
  float_type pd3_sse = pd_sse(50, 76);
  float_type pd4_sse = pd_sse(87, 12);

  BOOST_CHECK_CLOSE(pd1, pd1_sse, 0.01);
  BOOST_CHECK_CLOSE(pd2, pd2_sse, 0.01);
  BOOST_CHECK_CLOSE(pd3, pd3_sse, 0.01);
  BOOST_CHECK_CLOSE(pd4, pd4_sse, 0.01);
}

// ############ TEST ALL ############### //
template <typename float_type, typename sse_float_type>
void all_sse() {
  int K = 2;
  int M = 2;
  int n = 4;
  
  boost::shared_ptr<HMMVector<float_type, sse_float_type> > 
    pi_ptr(new HMMVector<float_type, sse_float_type>(K));
  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > 
    T_ptr(new HMMMatrix<float_type, sse_float_type>(K,K));
  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > 
    E_ptr(new HMMMatrix<float_type, sse_float_type>(M, K));
	
  HMMVector<float_type, sse_float_type> &pi = *pi_ptr;
  HMMMatrix<float_type, sse_float_type> &T = *T_ptr;
  HMMMatrix<float_type, sse_float_type> &E = *E_ptr;
	
  pi(0)  = 0.2; pi(1) = 0.8;

  // transitions from state 0
  T(0,0) = 0.1; T(0,1) = 0.9;
  // transitions from state 1
  T(1,0) = 0.9; T(1,1) = 0.1;
	
  // emissions from state 0
  E(0,0) = 0.25;
  E(1,0) = 0.75;
  // emissions from state 1
  E(0,1) = 0.75;
  E(1,1) = 0.25;
	
  HMM<float_type, sse_float_type> hmm(pi_ptr, T_ptr, E_ptr);
	
  sequence obsseq(n);
  obsseq[0] = 0;  
  obsseq[1] = 1;
  obsseq[2] = 0;
  obsseq[3] = 1;

  sequence hiddenseq(n);
  hmm.viterbi(obsseq, hiddenseq);

  HMMMatrix<float_type, sse_float_type> 
    F(n, K);
  HMMVector<float_type, sse_float_type> 
    scales(n);
  hmm.forward(obsseq, scales, F);

  float_type log_likelihood = hmm.likelihood(scales);
  log_likelihood += 1; // to avoid warnings about unused variable.

  HMMMatrix<float_type, sse_float_type> 
    B(n, K);
  hmm.backward(obsseq, scales, B);

  HMMMatrix<float_type, sse_float_type> 
    pd(n, K);
  hmm.posterior_decoding(obsseq, F, B, scales, pd);

  boost::shared_ptr<HMMVector<float_type, sse_float_type> > 
    pi_counts_ptr(new HMMVector<float_type, sse_float_type>(K));
  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > 
    T_counts_ptr(new HMMMatrix<float_type, sse_float_type>(K,K));
  boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > 
    E_counts_ptr(new HMMMatrix<float_type, sse_float_type>(M,K));
  hmm.baum_welch(obsseq,
	     F,
	     B, 
	     scales,
	     *pi_counts_ptr,
	     *T_counts_ptr,
	     *E_counts_ptr);

  HMM<float_type, sse_float_type> new_hmm(pi_counts_ptr,
			  T_counts_ptr,
			  E_counts_ptr);
}

template <typename float_type>
void all_non_sse() {
  int no_states = 2;
  int alphabet_size = 2;
  int seq_length = 4;
  
  boost::shared_ptr<HMMVector<float_type,float_type> > 
    pi_ptr(new HMMVector<float_type,float_type>(no_states));
  boost::shared_ptr<HMMMatrix<float_type,float_type> > 
    T_ptr(new HMMMatrix<float_type,float_type>(no_states,no_states));
  boost::shared_ptr<HMMMatrix<float_type,float_type> > 
    E_ptr(new HMMMatrix<float_type,float_type>(alphabet_size, no_states));
	
  HMMVector<float_type,float_type> &pi = *pi_ptr;
  HMMMatrix<float_type,float_type> &T = *T_ptr;
  HMMMatrix<float_type,float_type> &E = *E_ptr;
	
  pi(0)  = 0.2; pi(1) = 0.8;

  // transitions from state 0
  T(0,0) = 0.1; T(0,1) = 0.9;
  // transitions from state 1
  T(1,0) = 0.9; T(1,1) = 0.1;
	
  // emissions from state 0
  E(0,0) = 0.25;
  E(1,0) = 0.75;
  // emissions from state 1
  E(0,1) = 0.25;
  E(1,1) = 0.75;
	
  HMM<float_type,float_type> hmm(pi_ptr, T_ptr, E_ptr);
	
  unsigned obs_array[] = { 0, 1, 0, 1 };
  sequence obsseq(obs_array, obs_array + seq_length);
 
  sequence hiddenseq(obsseq.size());
  hmm.viterbi(obsseq, hiddenseq);

  HMMMatrix<float_type,float_type> F(obsseq.size(), no_states);
  HMMVector<float_type,float_type> scales(obsseq.size());
  hmm.forward(obsseq, scales, F);

  float_type log_likelihood = hmm.likelihood(scales);
  log_likelihood += 1;

  HMMMatrix<float_type,float_type> B(obsseq.size(), no_states);
  hmm.backward(obsseq, scales, B);

  HMMMatrix<float_type,float_type> pd(obsseq.size(), no_states);
  hmm.posterior_decoding(obsseq, F, B, scales, pd);

  boost::shared_ptr<HMMVector<float_type,float_type> > 
    pi_counts_ptr(new HMMVector<float_type,float_type>(no_states));
  boost::shared_ptr<HMMMatrix<float_type,float_type> > 
    T_counts_ptr(new HMMMatrix<float_type,float_type>(no_states,no_states));
  boost::shared_ptr<HMMMatrix<float_type,float_type> > 
    E_counts_ptr(new HMMMatrix<float_type,float_type>(alphabet_size,no_states));
  hmm.baum_welch(obsseq,
	     F,
	     B, 
	     scales,
	     *pi_counts_ptr,
	     *T_counts_ptr,
	     *E_counts_ptr);

  HMM<float_type,float_type> new_hmm(pi_counts_ptr,
				     T_counts_ptr,
				     E_counts_ptr);
}

#define TEST(test)					\
  BOOST_AUTO_TEST_CASE( hmm_double_double_##test ) {	\
    test<double, double>();				\
  }							\
  BOOST_AUTO_TEST_CASE( hmm_float_float_##test ) {	\
    test<float, float>();				\
  }							\
  BOOST_AUTO_TEST_CASE( hmm_double_m128d_##test ) {	\
    test<double, __m128d>();				\
  }							\
  BOOST_AUTO_TEST_CASE( hmm_float_m128_##test ) {	\
    test<float, __m128>();				\
  }							\

#define TEST_NON_SSE(test)				\
  BOOST_AUTO_TEST_CASE( hmm_double_double_##test ) {	\
    test<double>();	        			\
  }							\
  BOOST_AUTO_TEST_CASE( hmm_float_float_##test ) {	\
    test<float>();        				\
  }							\
  
TEST(forward_1x1)
TEST(forward_1x2_no_transitions)
TEST(forward_2x2_no_transitions)
TEST(forward_2x2_with_transitions)

TEST(backward_1x1)
TEST(backward_1x2_no_transitions)
TEST(backward_2x2_no_transitions)
TEST(backward_2x2_with_transitions)

TEST(baum_welch_2x2_with_transitions)

TEST(viterbi_2x2_with_transitions)

TEST(construct_sse)
TEST(all_sse)

TEST_NON_SSE(construct_non_sse)
TEST_NON_SSE(all_non_sse)

#ifdef WITH_OMP
TEST(parallel_forward)
TEST(parallel_backward)
TEST(parallel_viterbi)
TEST(parallel_bw)
TEST(parallel_pd)
#endif

TEST_NON_SSE(sse_vs_nonSse_forward)
TEST_NON_SSE(sse_vs_nonSse_backward)
TEST_NON_SSE(sse_vs_nonSse_bw)
TEST_NON_SSE(sse_vs_nonSse_viterbi)
TEST_NON_SSE(sse_vs_nonSse_pd)

BOOST_AUTO_TEST_SUITE_END()
