// Works well

//////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2010, Lawrence Livermore National Security, LLC.  
// Produced at the Lawrence Livermore National Laboratory  
//////////////////////////////////////////////////////////////////////////////////////////////////
/// @author Todd Gamblin tgamblin@llnl.gov

#include <vector>
#include <sstream>

#include <algorithm>
#include <numeric>
#include <cassert>
#include <cstdlib>
#include <random>
#include <set>

using namespace std;

//#include "random.h"
//#include "bic.h"
//#include "matrix_utils.h"

namespace clustering {
	typedef float Scalar;
	typedef Eigen::Matrix<Scalar,-1,-1> dissimilarity_matrix;

	typedef size_t medoid_id;       ///< More descriptive type for medoid index
	typedef size_t object_id;       ///< More descriptive type for object index

	/// 
	/// Explicit representation of a clustering.  Instead of a vecto of representative
	/// ids, this has <i>k</i> sets of object_ids indicating which objects are in a 
	/// particular cluster.  You can convert a partition to a cluster_list with 
	/// to_cluster_list().
	/// 
	typedef std::vector< std::set<object_id> > cluster_list;

	struct kmedoids{

		/// Gives the index of the object that is the ith medoid.
		/// medoids[i] == index in object array for last call to findClusters()
		std::vector<object_id> medoid_ids;

		/// Gives cluster id (index in medoids) for the ith object.
		/// clusterid[i]          == id of cluster of which object i is a member.
		/// medoids[clusterid[i]] == representative of that cluster.
		std::vector<medoid_id> cluster_ids;

		std::vector<medoid_id> sec_nearest;      /// Index of second closest medoids.  Used by PAM.
		double total_dissimilarity;              /// Total dissimilarity bt/w objects and their medoid
		bool sort_medoids;                       /// Whether medoids should be canonically sorted by object id.
		double epsilon;                          /// Normalized sensitivity for convergence
		size_t init_size;                        /// initial sample size (before 2*k)
		size_t max_reps;                         /// initial sample size (before 2*k)

		typedef std::mt19937 random_type;                /// Type for RNG used in this algorithm

		kmedoids(size_t num_objects) 
			: cluster_ids(num_objects, 0),
			total_dissimilarity(std::numeric_limits<double>::infinity()),
			sort_medoids(false),
			epsilon(1e-15),
			init_size(40),
			max_reps(5)
		{ if (num_objects) medoid_ids.resize(1); }

		double average_dissimilarity() const {
			return total_dissimilarity / cluster_ids.size();
		}

		void set_sort_medoids(bool sort) {
			sort_medoids = sort;
		}

		void set_epsilon(double e) {
			epsilon = e;
		}

		void init_medoids(size_t k, const dissimilarity_matrix& distance) {
			medoid_ids.clear();
			// find first object: object minimum dissimilarity to others
			object_id first_medoid = 0;
			double min_dissim = std::numeric_limits<Scalar>::max();
			for (size_t i=0; i < distance.rows(); i++) {
				double total = 0.0;
				for (size_t j=0; j < distance.cols(); j++) {
					total += distance(i,j);
				}
				if (total < min_dissim) {
					min_dissim   = total;
					first_medoid = i;
				}
			}

			// add first object to medoids and compute medoid ids.
			medoid_ids.push_back(first_medoid);
			assign_objects_to_clusters(distance);

			// now select next k-1 objects according to KR's BUILD algorithm
			for (size_t cur_k = 1; cur_k < k; cur_k++) {
				object_id best_obj = 0;
				double max_gain = 0.0;
				for (size_t i=0; i < distance.rows(); i++) {
					if (is_medoid(i)) continue;

					double gain = 0.0;
					#pragma omp parallel for reduction(+:gain)
					for (int j=0; j < (int)distance.rows(); j++) {
						double Dj = distance(j, medoid_ids[cluster_ids[j]]);  // distance from j to its medoid
						gain += max(Dj - distance(i,j), 0.0);                 // gain from selecting i  
					}

					if (gain >= max_gain) {   // set the next medoid to the object that 
						max_gain = gain;        // maximizes the gain function.
						best_obj = i;
					}
				}

				medoid_ids.push_back(best_obj);
				assign_objects_to_clusters(distance);
			}
		}

		double cost(medoid_id i, object_id h, const dissimilarity_matrix& distance) const {
			double total = 0;
			for (object_id j = 0; j < cluster_ids.size(); j++) {
				object_id mi  = medoid_ids[i];                // object id of medoid i
				double    dhj = distance(h, j);               // distance between object h and object j

				object_id mj1 = medoid_ids[cluster_ids[j]];   // object id of j's nearest medoid
				double    dj1 = distance(mj1, j);             // distance to j's nearest medoid

				// check if distance bt/w medoid i and j is same as j's current nearest medoid.
				if (distance(mi, j) == dj1) {
					double dj2 = std::numeric_limits<Scalar>::max();
					if (medoid_ids.size() > 1) {   // look at 2nd nearest if there's more than one medoid.
						object_id mj2 = medoid_ids[sec_nearest[j]];  // object id of j's 2nd-nearest medoid
						dj2 = distance(mj2, j);                      // distance to j's 2nd-nearest medoid
					}
					total += min(dj2, dhj) - dj1;

				} else if (dhj < dj1) {
					total += dhj - dj1;
				}
			}
			return total;
		}

		/// True if and only if object i is a medoid.
		bool is_medoid(object_id oi) const {
			return medoid_ids[cluster_ids[oi]] == oi;
		}

		void pam(const dissimilarity_matrix& distance, size_t k, const object_id *initial_medoids) {
			if (k > distance.rows()) {
				throw std::logic_error("Attempt to run PAM with more clusters than data.");
			}

			if (distance.rows() != distance.cols()) {
				throw std::logic_error("Error: distance matrix is not square!");
			}

			// first get this the right size.
			cluster_ids.resize(distance.rows());

			// size cluster_ids appropriately and randomly pick initial medoids
			if (initial_medoids) {
				medoid_ids.clear();
				copy(initial_medoids, initial_medoids + k, back_inserter(medoid_ids));
			} else {
				init_medoids(k, distance);
			}

			// set tolerance equal to epsilon times mean magnitude of distances.
			// Note that distances *should* all be non-negative.
			double tolerance = epsilon * distance.sum() / (distance.rows() * distance.cols());

			while (true) {
				// initial cluster setup
				total_dissimilarity = assign_objects_to_clusters(distance);

				//vars to keep track of minimum
				double minTotalCost = std::numeric_limits<Scalar>::max();
				medoid_id minMedoid = 0;
				object_id minObject = 0;

				//iterate over each medoid
				for (medoid_id i=0; i < k; i++) {
					//iterate over all non-medoid objects
					for (object_id h = 0; h < cluster_ids.size(); h++) {
						if (is_medoid(h)) continue;

						//see if the total cost of swapping i & h was less than min
						double curCost = cost(i, h, distance);
						if (curCost < minTotalCost) {
							minTotalCost = curCost;
							minMedoid = i;
							minObject = h;
						}
					}
				}

				// bail if we can't gain anything more (we've converged)
				if (minTotalCost >= -tolerance) break;

				// install the new medoid if we found a beneficial swap
				medoid_ids[minMedoid] = minObject;
				cluster_ids[minObject] = minMedoid;
			}

			//if (sort_medoids) sort();
		}

		/// Assign each object to the cluster with the closest medoid.
		///
		/// @return Total dissimilarity of objects w/their medoids.
		/// 
		/// @param distance a callable object that computes distances between indices, as a distance 
		///                 matrix would.  Algorithms are free to use real distance matrices (as in PAM) 
		///                 or to compute lazily (as in CLARA medoid assignment).
		double assign_objects_to_clusters(const dissimilarity_matrix& distance) {
			if (sec_nearest.size() != cluster_ids.size()) {
				sec_nearest.resize(cluster_ids.size());
			}

			// go through and assign each object to nearest medoid, keeping track of total dissimilarity.
			double total_dissimilarity = 0;

			#pragma omp parallel for reduction(+:total_dissimilarity)
			for (int i=0; i < (int)cluster_ids.size(); i++) {
				double    d1, d2;  // smallest, second smallest distance to medoid, respectively
				medoid_id m1, m2;  // index of medoids with distances d1, d2 from object i, respectively

				d1 = d2 = std::numeric_limits<Scalar>::max();
				m1 = m2 = medoid_ids.size();
				for (medoid_id m=0; m < medoid_ids.size(); m++) {
					double d = distance(i, medoid_ids[m]);
					if (d < d1 || medoid_ids[m] == i) {  // prefer the medoid in case of ties.
						d2 = d1;  m2 = m1;
						d1 = d;   m1 = m;
					} else if (d < d2) {
						d2 = d;   m2 = m;
					}
				}

				cluster_ids[i] = m1;
				sec_nearest[i] = m2;
				total_dissimilarity += d1;
			}

			return total_dissimilarity;
		}
	};

} // namespace cluster  
