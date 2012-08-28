#include "vectorSpace.hpp"
#include <boost/foreach.hpp>

namespace AG
{
namespace Data{

void random_init_vector_space(VectorSpace & vs, unsigned int num_vectors, unsigned int dimensions, double max_value)
{
	for (unsigned int vid = 0 ; vid < num_vectors; vid++)
	{
		Vect v(dimensions);
		for (unsigned int i=0; i < dimensions; i++)
			v(i) = rand() / (double)RAND_MAX * max_value;
		vs.push_back(v);
	}
}

std::ostream& operator << (std::ostream& os,  VectorSpace & vs)
{
	unsigned int i = 0;
	BOOST_FOREACH(Vect v, vs)
	{
		os << "[" << i << "]: ";
		BOOST_FOREACH(Vect::value_type  value, v)
				os << value << " ";
		os << std::endl;
		i++;
	}

	return os;
}

std::ostream& operator << (std::ostream& os,  Vect & v)
{
	BOOST_FOREACH(Vect::value_type  value, v)
			os << value << " ";
	os << std::endl;

	return os;
}
};
};
