#pragma once

/* Meanshift non-parametric mode estimator.
 *
 * Code adapted from : Sebastian Nowozin <nowozin@gmail.com>
 */

#define _USE_MATH_DEFINES
#include <math.h>

#include <vector>
#include <map>
#include <iostream>

//#include <gmm/gmm.h>
#include <Eigen/Core>

namespace gmm{
    template<typename V>
    void clear(V & v){
        for(size_t i = 0; i < v.size(); i++)
            v[i] = 0;
    }

    template<typename V, typename W>
    void copy( const V & v, W & w ){
        for(size_t i = 0; i < v.size(); i++)
            w[i] = v[i];
    }

    template<typename V, typename W>
    void add( const V & v, W & w ){
        for(size_t i = 0; i < w.size(); i++)
            w[i] += v[i];
    }

    template<typename V>
    V scaled(const V & v, double s){
        V a = v;
        for(size_t i = 0; i < a.size(); i++)
            a[i] *= s;
        return a;
    }

    template<typename V>
    void scale( V & v, double s ){
        for(size_t i = 0; i < v.size(); i++)
            v[i] *= s;
    }

    template<typename V, typename W>
    double vect_dist2(V & v, W & w){
        Eigen::VectorXd a = Eigen::Map<Eigen::VectorXd>(&v[0], v.size());
        Eigen::VectorXd b = Eigen::Map<Eigen::VectorXd>(&w[0], w.size());
        return (a-b).lpNorm<2>();
    }

    template<typename V>
    double vect_norm2(V & v){
        return Eigen::Map<Eigen::VectorXd>(&v[0], v.size()).norm();
    }
}

namespace clustering {

/* We use the notation of the description of Mean Shift in
 *   [Comaniciu2000], Dorin Comaniciu, Peter Meer,
 *   "Mean Shift: A Robust Approach toward Feature Space Analysis"
 *
 * The implementation is naive and does not exploit fast nearest neighbor
 * lookups or triangle inequalities to speed up the mean shift procedure.
 * Therefore, it is only suitable for low-dimensional input spaces (say, <=
 * 10) with relatively few samples (say, < 1e6).
 */
class Meanshift {
public:
    /* kernel_type selects the kernel profile:
     *   0 for the Epanechnikov kernel profile
     *     k_E(x) = (1 - x) if (0 <= x <= 1), 0 otherwise.
     *   1 for the truncated multivariate normal kernel profile
     *     k_N(x) = exp(-0.5 * x)
     * kernel_bandwidth: The positive bandwidth parameter.
     * mode_tolerance: mode matching tolerance.  Modes which have a L2
     *   distance closer than this value will be treated as being the same.
     */
    Meanshift(int kernel_type, double kernel_bandwidth,
        int dim, double mode_tolerance);

    /* Find modes of an empirical distribution X.
     *
     * X: N vectors of size M, representing N samples from the distribution.
     * modes: Output, will be allocated properly.  Return all modes found.
     * indexmap: N-vector of absolute mode indices.  Each sample point is
     *   assigned to one mode.  The vector must already be properly sized.
     * procedurecount: The mean shift procedure will be run at least this many
     *   times, sampling randomly from X as initialization.  If zero, all
     *   samples from X are used as initializations.
     */
    void FindModes(const std::vector<std::vector<double> >& X,
        std::vector<std::vector<double> >& modes,
        std::vector<int>& indexmap,
        unsigned int procedure_count = 0) const;

    /* Mean Shift Procedure, starting from mode, perform mean shift on the
     * distribution empirically sampled in X.
     *
     * X: N vectors of size M, representing N samples from the distribution.
     * mode: Starting point (for example a point from X).  The result will be
     *    given in mode.
     * visited: N-vector of indicator variables.  If the mean shift procedure
     *    passes through a point in X, the corresponding index in visited will
     *    be set to one.
     *
     * Return the number of iterations used.
     */
    int MeanshiftProcedure(const std::vector<std::vector<double> >& X,
        std::vector<double>& mode, std::vector<unsigned int>& visited) const;

private:
    int dim;	// input space dimension
    int kernel_type;	// 0: Epanechnikov, 1: truncated multivariate normal
    double kernel_c;	// constant normalization factor
    double kernel_bandwidth;
    double mode_tolerance;
};


}
