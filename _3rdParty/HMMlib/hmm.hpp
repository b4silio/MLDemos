#ifndef HMM_HPP
#define HMM_HPP

#include <boost/shared_ptr.hpp>

#include <vector>
#include <cmath>
#include <assert.h>
using namespace std;

#include "HMMlib/hmm_matrix.hpp"
#include "HMMlib/hmm_vector.hpp"
#include "HMMlib/sse_operator_traits.hpp"

#include <pmmintrin.h>

#ifdef WITH_OMP
#include <omp.h>
#endif

typedef std::vector<unsigned int> sequence;

namespace hmmlib {

  /**
   * \class HMM
   *
   * \brief Encapsulates the representation of a Hidden Markov
   * Model.
   *
   * This class encapsulates the representation of a Hidden Markov
   * Model. A hidden Markov model is composed by a transition matrix,
   * an emission matrix and a vector of initial state probabilities.
   * Five classical HMM algorithms are supported: Viterbi, forward,
   * backward, Baum-Welch and posterior decoding.  The algorithms are
   * optimized using the SSE instruction set and parallelized using
   * OpenMP as described in <a
   * href="http://birc.au.dk/~asand/papers/HiBi10_hmmlib.pdf">HMMlib:
   * A C++ Library for General Hidden Markov Models Exploiting Modern
   * CPUs</a>.
   *
   * Possible values of \a float_type are: \a double or \a float.
   *
   * Possible values of sse_float_type are: \a double, \a float, \a
   * __m128d or __m128. If \a float_type is \a double, sse_float_type
   * must be either \a __m128d (default) or \a double. If \a
   * float_type is \a float, sse_float_type must be either \a __m128
   * (default) or \a float.
   *
   *
   * \author Andreas Sand (asand@birc.au.dk)
   * \see http://birc.au.dk/~asand/papers/HiBi10_hmmlib.pdf
   */
  template < typename float_type, 
	     typename sse_float_type = typename FloatTraits<float_type>::sse_type>
  class HMM {
		
    typedef SSEOperatorTraits<float_type, sse_float_type> sse_operations_traits;
		
    boost::shared_ptr< HMMVector<float_type, sse_float_type> > initial_prob;
    boost::shared_ptr< HMMMatrix<float_type, sse_float_type> > trans_prob;
    boost::shared_ptr< HMMMatrix<float_type, sse_float_type> > emission_prob;
		
    const int no_states;
    const int alphabet_size;
    
  public :
    /**
     * \brief Constructs an HMM with the indicated initial state
     * probabilities, transition probabilities and emission
     * probabilities.
     *
     * Constructs an HMM with the indicated initial state
     * probabilities, transition probabilities and emission
     * probabilities.
     *
     * \param initial_prob   The vector of initial state probabilities, in which the \f$i\f$th entry must be the probability of the model initially being in state i. The entries in \a initial_prob must sum to 1.
     * \param trans_prob     The matrix of transition probabilities, in which the \f$i,j\f$th entry must be the probability of the transition from state i to state j. Each row in T must sum to 1.
     * \param emission_prob  The matrix of emission probabilities, in which the \f$i,j\f$th entry must be the probabilities of state \f$j\f$ emitting the \f$i\f$th alphabet symbol. Each column must sum to 1.
     *
     * Possible values of \a float_type are: \a double or \a float.
     *
     * Possible values of sse_float_type are: \a double, \a float, \a
     * __m128d or __m128. If \a float_type is \a double, sse_float_type
     * must be either \a __m128d (default) or \a double. If \a
     * float_type is \a float, sse_float_type must be either \a __m128
     * (default) or \a float.
     *
     * \pre The size of \a initial_prob must match the number of rows in \a trans_prob, the number of columns in trans_prob and the number columns in emission_prob.
     */
    HMM(boost::shared_ptr< HMMVector<float_type, sse_float_type> > initial_prob,
	boost::shared_ptr< HMMMatrix<float_type, sse_float_type> > trans_prob,
	boost::shared_ptr< HMMMatrix<float_type, sse_float_type> > emission_prob);

    // field accessors and mutators
    const HMMVector<float_type,sse_float_type> &get_initial_probs() { return *initial_prob; }
    const HMMMatrix<float_type,sse_float_type> &get_trans_probs() { return *trans_prob; }
    const HMMMatrix<float_type,sse_float_type> &get_emission_probs() { return *emission_prob; }
    const int get_no_states() const { return no_states; }
    const int get_alphabet_size() const { return alphabet_size; }
    

    // algorithms
    /**
     * \brief Runs the forward dynamic programming algorithm.
     *
     * Fills in the forward dynamic programming table, \a F.
     *
     * \param obsseq    The observed sequence of emissions.
     * \param F         The forward dynamic programming table (to be filled in).
     * \param scales    The scaling factors (to be filled in).
     *
     * After running this function, the \f$i,j\f$th entry in \a F will be:
     * \f[f_{ij} = c_i e_{s_i,j} \sum_l f_{i-1,l}t_{l, j},\f]
     * and the \f$i\f$th entry in \a scales, \f$c_i\f$, will be the factor normalizing
     * \f$ \sum_j e_{s_i,j}  \sum_l f_{i-1,l}t_{l, j}\f$.
     *
     * \pre The number of states in the model must match the number of columns in \a F.
     * \pre The number of observations in \a obsseq must match the number of rows in \a F and the size of \a scales.
     */
    void forward(const sequence &obsseq,
		 HMMVector<float_type, sse_float_type> &scales,
		 HMMMatrix<float_type, sse_float_type> &F);
    
    /**
     * \brief Runs the backward dynamic programming algorithm.
     *
     * Fills in the backward dynamic programming table, \a B.
     *
     * \param obsseq    The observed sequence of emissions.
     * \param B         The backward dynamic programming table (to be filled in).
     * \param scales    The scaling factors from the forward algorithm.
     *
     * After running this function, the \f$i,j\f$th entry in \a B will be:
     * \f[b_{ij} = c_{i+1}\sum_l e_{s_{i+1},l} t_{jl} b_{i+1,l}.\f]
     *
     * \pre The number of states in the model must match the number of columns in \a B.  
     * \pre The number of observations in \a obsseq must match the number of rows in \a B and the size of \a scales.
     * \pre \a scales must have been filled out using \a forward().
     */
    void backward(const sequence &obsseq,
		  const HMMVector<float_type, sse_float_type> &scales,
		  HMMMatrix<float_type, sse_float_type> &B);

    /**
     * \brief Computes the loglikelihood of the model generating an observed sequence.
     *
     * Computes the loglikelihood of the model generating an observed sequence, based on the vector of scaling factors computed by the \a forward() function.
     *
     * \param scales    The vector of scaling factors computed by the \a forward() function.
     * \returns         The loglikelihood.
     *
     * \pre \a scales must have been filled out using \a forward().
     */
    float_type likelihood(const HMMVector<float_type, sse_float_type> &scales); 

    /**
     * \brief Computes \f$\Theta=(pi_{counts}, T_{counts}, E_{counts})\f$ such that
     * \f$P(obsseq \vert \Theta)\f$ is maximized.
     *
     * Computes \f$\Theta=(pi_{counts}, T_{counts}, E_{counts})\f$ such that
     * \f$P(obsseq \vert \Theta)\f$ is maximized.
     *
     * \param obsseq    The observed sequence of emissions.
     * \param F         The forward dynamic programming table computed by \a forward().
     * \param B         The backward dynamic programming table table computed by \a backward().
     * \param scales    The scalling factors from the forward algorithms computed by \a forward().
     * \param pi_counts The estimated initial state probability vector (to be filled in).
     * \param T_counts  The estimated transition matrix (to be filled in).
     * \param E_counts  The estimated emission matrix (to be filled in).
     *
     * \pre \a F, \a scales and \a B must have been filled in using the \a forward() and \a backward() functions.
     * \pre The number of states in the model must match the number of columns in \a F and \a B.
     * \pre The number of observations in \a obsseq must match the number of rows in \a F and \a B and the size of \a scales.
     * \pre The number of states in the model must match the size of \a pi_counts, the number of rows in \a T_counts, the number of columns in \a T_counts and the number of columns in \a E_counts.
     * \pre The alphabet size of the model must match the number of rows in \a E_counts.
     */
    void baum_welch(const sequence &obsseq,
		    const HMMMatrix<float_type, sse_float_type> &F,
		    const HMMMatrix<float_type, sse_float_type> &B, 
		    const HMMVector<float_type, sse_float_type> &scales,
		    HMMVector<float_type, sse_float_type> &pi_counts,
		    HMMMatrix<float_type, sse_float_type> &T_counts,
		    HMMMatrix<float_type, sse_float_type> &E_counts);
    
    /**
     * \brief Computes the maximum likelihood sequence of hidden states.
     *
     * Computes the maximum likelihood sequence of hidden states.
     *
     * \param obsseq    The observed sequence of emissions.
     * \param hiddenseq The sequence of hidden states (to be filled in).
     *
     * \returns         The loglikelihood of the probability of \a hiddenseq, given that \a obsseq is emitted.
     *
     * \a hiddenseq is filled in using the Viterbi algorithm (computing in log space).
     *
     * \pre The size of hiddenseq must match the size of obsseq.
     */
    float_type viterbi(const sequence &obsseq,
		       sequence &hiddenseq);

    /**
     * \brief Computes the posterior decoding of \a obsseq.
     *
     * Computes the posterior decoding of \a obsseq.
     *
     * \param obsseq    The observed sequence of emissions.
     * \param F         The forward dynamic programming table computed by \a forward().
     * \param B         The backward dynamic programming table computed by \a backward().
     * \param scales    The vector of scaling factors computed by \a forward().
     * \param post      The matrix of posterior decoding probabilities (to be filled in).
     *
     * After running this function the \f$i,j\f$th entry in \a post
     * will be the probability with which the ith observed symbol in
     * obsseq was emitted by the the jth state:
     * \f[P(z_i = j \vert s) = \frac{f_{ij}b_{ij}}{c_i}.\f]
     *
     * \pre \a F, \a B and \a scales must have been fill in using the \a forward() and \a backward() functions.
     * \pre the number of rows in \a post must match the size of obsseq, and the number of columns in \a post must match the number of states in the model.
     */
    void posterior_decoding(const sequence &obsseq,
			    const HMMMatrix<float_type, sse_float_type> &F,
			    const HMMMatrix<float_type, sse_float_type> &B, 
			    const HMMVector<float_type, sse_float_type> &scales, 
			    HMMMatrix<float_type, sse_float_type> &post);
    
    friend class AllocatorTraits<float_type, sse_float_type>;
    friend class OperatorTraits<float_type, sse_float_type>;
  };

  // ###############################################
  // ########## Implementation comes here ##########
  // ###############################################
  template <typename float_type, typename sse_float_type>
  HMM<float_type,sse_float_type>::HMM(boost::shared_ptr< HMMVector<float_type, sse_float_type> > initial_prob,
				       boost::shared_ptr< HMMMatrix<float_type, sse_float_type> > trans_prob,
				       boost::shared_ptr< HMMMatrix<float_type, sse_float_type> > emission_prob)
    : 
    initial_prob(initial_prob), 
    trans_prob(trans_prob), 
    emission_prob(emission_prob),
    no_states(initial_prob->get_size()),
    alphabet_size(emission_prob->get_no_rows()) {
    assert(trans_prob->get_no_columns() == no_states);
    assert(trans_prob->get_no_rows() == no_states);
    assert(emission_prob->get_no_columns() == no_states);
    assert(emission_prob->get_no_rows() == alphabet_size);
  }


  template <typename float_type, typename sse_float_type>
  void
  HMM<float_type, sse_float_type>::forward(const sequence &obsseq,
					    HMMVector<float_type, sse_float_type> &scales,
					    HMMMatrix<float_type, sse_float_type> &F) {		
    // just making it a bit easier on ourselves...
    const HMMVector<float_type, sse_float_type> &pi = *initial_prob;
    const HMMMatrix<float_type, sse_float_type> &T = *trans_prob;
    const HMMMatrix<float_type, sse_float_type> &E = *emission_prob;
		
    HMMMatrix<float_type, sse_float_type> &T_t = *(new HMMMatrix<float_type, sse_float_type>(T.get_no_columns(), T.get_no_rows()));
    T.transpose(T_t);
		
    const int length = obsseq.size();
    const int no_chunks = F.get_no_chunks_per_row();
		
    // sanity check
    assert(F.get_no_columns() == no_states);
    assert(F.get_no_rows() == length);
    assert(scales.get_size() == length);
		
    // nice to have
    sse_float_type ones;
    sse_operations_traits::set_all(ones, (float_type) 1.0);
		
    // initialise
    unsigned int x = obsseq[0];
    sse_float_type scale;
    sse_operations_traits::set_all(scale, (float_type) 0.0);
    int sc;
    for(sc = 0; sc < no_chunks; ++sc) {
      sse_float_type temp = pi.get_chunk(sc) * E.get_chunk(x,sc);
      F.get_chunk(0, sc) = temp;
      scale += temp;
    }

    float_type scale_float = 0;
    sse_operations_traits::sum(scale);
    scale_float = 1 / *((float_type *) &scale); 
    scales(0) = scale_float;
		
    // scale first column of F
    sse_operations_traits::set_all(scale, scale_float);
    int chunk;
    for(chunk = 0; chunk < no_chunks; ++chunk)
      F.get_chunk(0,chunk) *= scale;

    for(int i = 1; i < length; ++i) { // filling in i'th row of F: F(i, -)
      x = obsseq[i];
      #ifdef WITH_OMP
      #pragma omp parallel for
      #endif
      for(int j = 0; j < no_states; ++j) { // filling in F(i,j)
	sse_float_type prob_sum;
	sse_operations_traits::set_all(prob_sum, (float_type) 0.0);
	for(int c = 0; c < no_chunks; ++c)
	  prob_sum += F.get_chunk(i-1, c) * T_t.get_chunk(j, c);
	sse_operations_traits::sum(prob_sum);
	sse_operations_traits::store(F(i,j), prob_sum);
      }

      sse_operations_traits::set_all(scale, (float_type) 0.0);
      for(int c = 0; c < no_chunks; ++c) {
	F.get_chunk(i, c) *= E.get_chunk(x,c);
	scale += F.get_chunk(i, c);
      }
      sse_operations_traits::sum(scale);
      sse_operations_traits::store(scales(i), scale);

			
      // normalize i'th column
      scales(i) = 1.0 / scales(i);
      sse_operations_traits::set_all(scale, scales(i));
      for(int chunk = 0; chunk < no_chunks; ++chunk)
	F.get_chunk(i,chunk) *= scale;
    }
  }
	
  template <typename float_type, typename sse_float_type>
  void
  HMM<float_type, sse_float_type>::backward(const sequence &obsseq,
					     const HMMVector<float_type, sse_float_type> &scales,
					     HMMMatrix<float_type, sse_float_type> &B) {
    // just making it a little easier on ourselves...
    const HMMMatrix<float_type, sse_float_type> &T = *trans_prob;
    const HMMMatrix<float_type, sse_float_type> &E = *emission_prob;

    // boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > T_t_ptr(new HMMMatrix<float_type, sse_float_type>(T.get_no_columns(), T.get_no_rows()));
    // HMMMatrix<float_type, sse_float_type> &T_t = *T_t_ptr;
    // T.transpose(T_t);

    const int length = obsseq.size();
    const int no_chunks = B.get_no_chunks_per_row();
		
    // sanity check
    assert(B.get_no_columns() == no_states);
    assert(B.get_no_rows() == length);
    assert(scales.get_size() == length);
		
    // Fill in the last column of B
    sse_float_type scale;
    sse_operations_traits::set_all(scale, scales(length - 1));
    int sc;
    for(sc = 0; sc < no_chunks; ++sc)
      B.get_chunk(length-1,sc) = scale;
		
    // Recursion
    for(int i = length - 1; i > 0; --i) { // fill in the (i-1)'th row of B
      int x = obsseq[i];
      #ifdef WITH_OMP
      #pragma omp parallel for
      #endif
      for(int s = 0; s < no_states; ++s) { // fill in B(i-1,s)
	sse_float_type prob_sum;
	sse_operations_traits::set_all(prob_sum, 0.0);

	for(int chunk = 0; chunk < no_chunks; ++chunk)
	  prob_sum += T.get_chunk(s, chunk) * E.get_chunk(x, chunk) * B.get_chunk(i, chunk);

	sse_operations_traits::sum(prob_sum);
	float_type sum;
	sse_operations_traits::store(sum, prob_sum);
	B(i-1,s) = sum * scales(i-1);
      }
    }
  }
	
  template <typename float_type, typename sse_float_type>
  float_type
  HMM<float_type, sse_float_type>::likelihood(const HMMVector<float_type, sse_float_type> &scales) {
    float_type likelihood = 0.0;
    for (int s = 0; s < scales.get_size(); ++s) {
      likelihood -= std::log(scales(s));
    }
    return likelihood;
  }
	
  template <typename float_type, typename sse_float_type>
  void
  HMM<float_type, sse_float_type>::baum_welch(const sequence &obsseq,
					       const HMMMatrix<float_type, sse_float_type> &F,
					       const HMMMatrix<float_type, sse_float_type> &B, 
					       const HMMVector<float_type, sse_float_type> &scales,
					       HMMVector<float_type, sse_float_type> &new_pi,
					       HMMMatrix<float_type, sse_float_type> &new_T,
					       HMMMatrix<float_type, sse_float_type> &new_E) {
		
    // easier reference
    HMMMatrix<float_type, sse_float_type> &T = *trans_prob;
    HMMMatrix<float_type, sse_float_type> &E = *emission_prob;
		
    const int length = obsseq.size();
    const int no_chunks = B.get_no_chunks_per_row();
		
    // sanity check
    assert(new_T.get_no_columns() == no_states);
    assert(new_T.get_no_rows() ==  no_states);
    assert(new_E.get_no_columns() == no_states);
    assert(new_E.get_no_rows() == alphabet_size);
    assert(new_pi.get_size() == no_states);
    assert(scales.get_size() == length);
    assert(F.get_no_rows() == length);
    assert(F.get_no_columns() == no_states);
    assert(B.get_no_rows() == length);
    assert(B.get_no_columns() == no_states);
		
    // initialise
    new_pi.reset();
    new_T.reset();
    new_E.reset();

    // compute counts
    HMMVector<float_type, sse_float_type> pi_counts(no_states);
    HMMMatrix<float_type, sse_float_type> T_counts(no_states, no_states);
    HMMMatrix<float_type, sse_float_type> E_counts(alphabet_size, no_states);
		
    // compute new_pi and initialize E_counts
    unsigned x = obsseq[0];
    sse_float_type scale;
    sse_operations_traits::set_all(scale, (float_type) (1.0/scales(0)));
    for (int sc = 0; sc < no_chunks; ++sc) {
      sse_float_type temp = (F.get_chunk(0,sc) * B.get_chunk(0,sc)) * scale;
      new_pi.get_chunk(sc) = temp;
      E_counts.get_chunk(x, sc) += temp;
    }
		
    // compute transition and emission counts
    for (int i = 1; i < length; ++i) {
      x = obsseq[i];
      #ifdef WITH_OMP
      #pragma omp parallel for
      #endif
      for(int j = 0; j < no_states; ++j) {
	// transition counts
	sse_float_type prev_forward;
	sse_operations_traits::set_all(prev_forward, F(i-1, j));
	for(int chunk = 0; chunk < no_chunks; ++chunk) {
	  T_counts.get_chunk(j, chunk) += prev_forward * 
	    T.get_chunk(j,chunk) * 
	    E.get_chunk(x,chunk) * 
	    B.get_chunk(i,chunk);
	}
      }
      // emission counts
      sse_operations_traits::set_all(scale, (float_type) (1.0/scales(i)));
      for(int chunk = 0; chunk < no_chunks; ++chunk)
	E_counts.get_chunk(x,chunk) += F.get_chunk(i, chunk) * 
	  B.get_chunk(i, chunk) * scale;
    }

    // compute new_T and new_E by normalizing counts
    for (int s = 0; s < no_states; ++s) {
      // transition probabilities
      float_type sum = 0.0;
      for (int dst_state = 0; dst_state < no_states; ++dst_state)
        sum += T_counts(s, dst_state);
      for (int dst_state = 0; dst_state < no_states; ++dst_state)
        new_T(s, dst_state) = T_counts(s, dst_state) / sum;

      // emission probabilities
      sum = 0.0;
      for (int sym = 0; sym < alphabet_size; ++sym)
        sum += E_counts(sym, s);
      for (int sym = 0; sym < alphabet_size; ++sym)
        new_E(sym, s) = E_counts(sym, s) / sum;
    }
  }

  template <typename float_type, typename sse_float_type>
  float_type
  HMM<float_type, sse_float_type>::viterbi(const sequence &obsseq, sequence &hiddenseq) {
    // just making it a bit easier on ourselves...
    const HMMVector<float_type, sse_float_type> &pi = *initial_prob;
    const HMMMatrix<float_type, sse_float_type> &T = *trans_prob;
    const HMMMatrix<float_type, sse_float_type> &E = *emission_prob;
    
    // sanity check
    assert(obsseq.size() == hiddenseq.size());

    // Set up internal data structures
    const int length = obsseq.size();
    
    HMMMatrix<float_type, sse_float_type> path_probs(length, no_states);
    
    const int no_chunks = path_probs.get_no_chunks_per_row();
    
    boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > T_t_ptr(new HMMMatrix<float_type, sse_float_type>(T.get_no_columns(), T.get_no_rows()));
    boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > T_t_log_ptr(new HMMMatrix<float_type, sse_float_type>(T.get_no_rows(), T.get_no_columns()));
    HMMMatrix<float_type, sse_float_type> &T_t_log = *T_t_log_ptr;
    
    boost::shared_ptr<HMMMatrix<float_type, sse_float_type> > E_log_ptr(new HMMMatrix<float_type, sse_float_type>(E.get_no_rows(), E.get_no_columns()));
    HMMMatrix<float_type, sse_float_type> &E_log = *E_log_ptr;

    // construct T_t_log and E_log
    T.transpose(*T_t_ptr);
    (*T_t_ptr).log(T_t_log);
    E.log(E_log);

    // Fill first column of path_probs
    int x = obsseq[0];
    for (int i = 0; i < no_states; ++i) {
      path_probs(0, i) = std::log(pi(i)) + E_log(x, i);
    }

    // Recursion
    for(int i = 1; i < length; ++i) { // fill ith row of path_probs
      x = obsseq[i];
      #ifdef WITH_OMP
      #pragma omp parallel for
      #endif
      for(int s = 0; s < no_states; ++s) { // fill path_probs(i,s)
	sse_float_type max, tmp;
	sse_operations_traits::set_all(max, -INFINITY);
	for(int c = 0; c < no_chunks - 1; ++c) { // no_chunks-1 since we don't want to take the max of some garbage.
	  tmp = path_probs.get_chunk(i-1, c) + T_t_log.get_chunk(s, c);
	  sse_operations_traits::max(max, tmp);
	}
	float_type float_max = sse_operations_traits::hmax(max);
	int floats_per_chunk = sizeof(sse_float_type) / sizeof(float_type);
	for(int c = (no_chunks - 1) * floats_per_chunk; c < path_probs.get_no_columns(); ++c)
	  float_max = std::max(float_max, path_probs(i-1,c) + T_t_log(s,c));
	path_probs(i, s) = float_max;
      }

      for(int c = 0; c < no_chunks; ++c) { 
	path_probs.get_chunk(i,c) += E_log.get_chunk(x,c);
      }
    }

    // Backtracking - final row
    float_type loglikelihood = -INFINITY;
    int hidden_state = 0;
    for(int i = 0; i < no_states; ++i) {
      if (path_probs(length - 1, i) > loglikelihood) {
	loglikelihood = path_probs(length - 1, i);
	hidden_state = i;
      }
    }
    hiddenseq[length - 1] = hidden_state;
    
    // Backtracking - recursion
    for (unsigned i = length - 1; i > 0; --i) {
      float_type max = -INFINITY;
      float_type tmp;
      int maxidx = 0;
      for (int s = 0; s < no_states; ++s) {
	tmp = path_probs(i-1, s) + T_t_log(hidden_state, s);
	if(tmp > max) {
	  max = tmp;
	  maxidx = s;
	}
      }
      hidden_state = maxidx;
      hiddenseq[i-1] = hidden_state;
    }

    return loglikelihood;
  }
  
  template <typename float_type, typename sse_float_type>
  void
  HMM<float_type, sse_float_type>::posterior_decoding(const sequence &obsseq,
						       const HMMMatrix<float_type, sse_float_type> &F,
						       const HMMMatrix<float_type, sse_float_type> &B, 
						       const HMMVector<float_type, sse_float_type> &scales, 
						       HMMMatrix<float_type, sse_float_type> &post) {
    const int length = obsseq.size();
    const int no_chunks = F.get_no_chunks_per_row();
    #ifdef WITH_OMP
    #pragma omp parallel for
    #endif
    for (int i = 0; i < length; ++i) {
      sse_float_type scale;
      sse_operations_traits::set_all(scale, (float_type) (1.0/scales(i)));
      for (int chunk = 0; chunk < no_chunks; ++chunk) {
	post.get_chunk(i, chunk) = F.get_chunk(i, chunk) * B.get_chunk(i, chunk) * scale;
      }
    }
  }
  
} // end of namespace

#endif
