/*
 * Represents a vector space as a sparse matrix
 *
 */
#ifndef VECTOR_SPACE_H
#define VECTOR_SPACE_H
#include <boost/numeric/ublas/vector_sparse.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <vector>

namespace AG
{
	namespace Data{

		using namespace boost::numeric::ublas;

		typedef coordinate_vector<double> Vect;
		typedef std::vector< Vect > ArrayOfVectors;
		typedef ArrayOfVectors VectorSpace;

		void random_init_vector_space(VectorSpace & vs, unsigned int num_vectors, unsigned int dimensions, double max_value);

		std::ostream& operator << (std::ostream& os,  Vect & vs);
		std::ostream& operator << (std::ostream& os,  VectorSpace & vs);

	}; // namespace Data

};

#endif
