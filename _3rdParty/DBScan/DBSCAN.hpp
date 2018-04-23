#pragma once

// Code adapted from https://github.com/propanoid/DBSCAN

#include <vector>
#include <algorithm>
//#include <omp.h>

// Any basic vector/matrix library should also work
#include <Eigen/Core>

namespace clustering
{
	template<typename Vector, typename Matrix>
	class DBSCAN
	{
	public:
		typedef Vector FeaturesWeights;
		typedef Matrix ClusterData;
		typedef Matrix DistanceMatrix;
		typedef std::vector<unsigned int> Neighbors;
		typedef std::vector<int> Labels;

	private:
		double m_eps;
		size_t m_min_elems;
		double m_dmin;
		double m_dmax;

		Labels m_labels;

	public:

		// 'eps' is the search space for neighbors in the range [0,1], where 0.0 is exactly self and 1.0 is entire dataset
		DBSCAN(double eps, size_t min_elems)
			: m_eps( eps )
			, m_min_elems( min_elems )
			, m_dmin(0.0)
			, m_dmax(0.0)
		{
			reset();
		}

		// Call this to perform clustering, get results by calling 'get_labels()'
		void fit( const ClusterData & C )
		{
			const FeaturesWeights W = std_weights( C.cols() );
			wfit( C, W );
		}

		const Labels & get_labels() const
		{
			return m_labels;
		}

		void reset()
		{
			m_labels.clear();
		}

		void init(double eps, size_t min_elems)
		{
			m_eps = eps;
			m_min_elems = min_elems;
		}

		// Useful for testing
		static ClusterData gen_cluster_data( size_t features_num, size_t elements_num )
		{
			ClusterData cl_d( elements_num, features_num );
			for (size_t i = 0; i < elements_num; ++i)
				for (size_t j = 0; j < features_num; ++j)
					cl_d(i, j) = (-1.0 + rand() * (2.0) / RAND_MAX);
			return cl_d;
		}

		FeaturesWeights std_weights( size_t s )
		{
			// num cols
			FeaturesWeights ws( s );

			for (size_t i = 0; i < s; ++i)
				ws(i) = 1.0;

			return ws;
		}

		void fit_precomputed( const DistanceMatrix & D )
		{
			prepare_labels( D.rows() );
			dbscan( D );
		}

		void wfit( const ClusterData & C, const FeaturesWeights & W )
		{
			prepare_labels( C.rows() );
			const DistanceMatrix D = calc_dist_matrix( C, W );
			dbscan( D );
		}

	private:
		void prepare_labels( size_t s )
		{
			m_labels.resize(s, -1);
		}

		Neighbors find_neighbors(const DistanceMatrix & D, unsigned int pid)
		{
			Neighbors ne;

			for (unsigned int j = 0; j < D.rows(); ++j)
			{
				if 	( D(pid, j) <= m_eps )
				{
					ne.push_back(j);
				}
			}
			return ne;
		}

		const DistanceMatrix calc_dist_matrix( const ClusterData & C, const FeaturesWeights & W )
		{
			ClusterData cl_d = C;

//#pragma omp parallel for
			for (int i = 0; i < (int)cl_d.cols(); ++i)
			{
				auto col = cl_d.col(i);

				const auto r = std::minmax_element( col.data(), col.data() + col.size() );

				double data_min = *r.first;
				double data_range = *r.second - *r.first;

				if (data_range == 0.0) { data_range = 1.0; }

				const double scale = 1/data_range;
				const double min = -1.0*data_min*scale;

				col *= scale;
				col += Vector::Constant(col.size(), min);

				cl_d.col(i) = col;
			}

			// rows x rows
			DistanceMatrix d_m( cl_d.rows(), cl_d.rows() );
			Vector d_max( cl_d.rows() );
			Vector d_min( cl_d.rows() );

			for (int i = 0; i < (int)cl_d.rows(); ++i)
			{
//#pragma omp parallel for
				for (int j = i; j < (int)cl_d.rows(); ++j)
				{
					d_m(i, j) = 0.0;

					if (i != j)
					{
						Vector U = cl_d.row(i);
						Vector V = cl_d.row(j);

						Vector diff = ( U-V );

						for(int k = 0; k < (int)diff.size(); k++)
						{
							auto e = diff[k];
							d_m(i, j) += fabs(e)*W[k];
						}

						d_m(j, i) = d_m(i, j);
					}
				}

				const auto cur_row = d_m.row(i);
				const auto mm = std::minmax_element( cur_row.data(), cur_row.data() + cur_row.size() );

				d_max(i) = *mm.second;
				d_min(i) = *mm.first;
			}

			m_dmin = *(std::min_element( d_min.data(), d_min.data() + d_min.size() ));
			m_dmax = *(std::max_element( d_max.data(), d_max.data() + d_max.size() ));

			m_eps = (m_dmax - m_dmin) * m_eps + m_dmin;

			return d_m;
		}

		void dbscan( const DistanceMatrix & dm )
		{
			std::vector<unsigned int> visited( dm.rows() );

			unsigned int cluster_id = 0;

			for (unsigned int pid = 0; pid < dm.rows(); ++pid)
			{
				if ( !visited[pid] )
				{
					visited[pid] = 1;

					Neighbors ne = find_neighbors(dm, pid );

					if (ne.size() >= m_min_elems)
					{
						m_labels[pid] = cluster_id;

						for (unsigned int i = 0; i < ne.size(); ++i)
						{
							unsigned int nPid = ne[i];

							if ( !visited[nPid] )
							{
								visited[nPid] = 1;

								Neighbors ne1 = find_neighbors(dm, nPid);

								if ( ne1.size() >= m_min_elems )
								{
									for (const auto & n1 : ne1)
									{
										ne.push_back(n1);
									}
								}
							}

							if ( m_labels[nPid] == -1 )
							{
								m_labels[nPid] = cluster_id;
							}
						}

						++cluster_id;
					}
				}
			}
		}
	};
}
