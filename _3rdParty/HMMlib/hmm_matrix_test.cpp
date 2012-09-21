#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN hmm_table_test

#include <boost/test/included/unit_test.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/mpl/list.hpp>
using namespace boost::unit_test;

#include "HMMlib/hmm_matrix.hpp"
using namespace hmmlib;

#include <pmmintrin.h>
#include <cmath>

BOOST_AUTO_TEST_SUITE( hmm_matrix_test )

typedef boost::mpl::list<HMMMatrix<double, double>, 
			 HMMMatrix<float, float> > test_types_non_sse;
typedef boost::mpl::list<HMMMatrix<double, double>, 
			 HMMMatrix<float, float>, 
			 HMMMatrix<double>, 
			 HMMMatrix<float> > test_types_all;

BOOST_AUTO_TEST_CASE_TEMPLATE(construction_test_non_sse, Table, test_types_non_sse) {
  Table table(6, 5, 2.0);
  
  BOOST_CHECK_EQUAL(table.get_no_rows(), 6);
  BOOST_CHECK_EQUAL(table.get_no_columns(), 5);
  BOOST_CHECK_EQUAL(table.get_no_allocated_columns(), 5);
  BOOST_CHECK_EQUAL(table.get_no_chunks_per_row(), 5);
  
  for(int r = 0; r < table.get_no_rows(); ++r)
    for(int c = 0; c < table.get_no_columns(); ++c)
      BOOST_CHECK_CLOSE(table(r, c), 2.0, 0.001);
}

BOOST_AUTO_TEST_CASE(construction_test_double_sse) {
	HMMMatrix<double, __m128d> table(6, 5, 2.0);
	
	BOOST_CHECK_EQUAL(table.get_no_rows(), 6);
	BOOST_CHECK_EQUAL(table.get_no_columns(), 5);
	BOOST_CHECK_EQUAL(table.get_no_allocated_columns(), 6);
	BOOST_CHECK_EQUAL(table.get_no_chunks_per_row(), 3);
	
	for(int r = 0; r < table.get_no_rows(); ++r)
		for(int c = 0; c < table.get_no_columns(); ++c)
			BOOST_CHECK_CLOSE(table(r, c), 2.0, 0.001);
	
}

BOOST_AUTO_TEST_CASE(construction_test_float_sse) {
    HMMMatrix<float, __m128> table(6, 5, 2.0f);
	
	BOOST_CHECK_EQUAL(table.get_no_rows(), 6);
	BOOST_CHECK_EQUAL(table.get_no_columns(), 5);
	BOOST_CHECK_EQUAL(table.get_no_allocated_columns(), 8);
	BOOST_CHECK_EQUAL(table.get_no_chunks_per_row(), 2);
	
	for(int r = 0; r < table.get_no_rows(); ++r)
		for(int c = 0; c < table.get_no_columns(); ++c)
			BOOST_CHECK_CLOSE(table(r, c), 2.0f, 0.001);	
}

BOOST_AUTO_TEST_CASE_TEMPLATE(assign_scalar_test, Table, test_types_all) {
	Table table(6, 5, 2.0);
	
	for(int r = 0; r < table.get_no_rows(); ++r)
		for(int c = 0; c < table.get_no_columns(); ++c)
			BOOST_CHECK_CLOSE(table(r, c), 2.0, 0.001);
	
	table = 4.0;
	
	for(int r = 0; r < table.get_no_rows(); ++r)
		for(int c = 0; c < table.get_no_columns(); ++c)
			BOOST_CHECK_CLOSE(table(r, c), 4.0, 0.001);
}

BOOST_AUTO_TEST_CASE(get_chunk_test_float) {
	typedef float float_type;
	typedef __m128 sse_float_type;
	
	HMMMatrix<float_type, sse_float_type> table(6,5);
	for(int r = 0; r < table.get_no_rows(); ++r)
		for(int c = 0; c < table.get_no_columns(); ++c)
			table(r, c) = (float_type) c;
	
	__m128 chunk1 = table.get_chunk(0, 0);
	__m128 chunk2 = table.get_chunk(0, 1);
	__m128 chunk3 = table.get_chunk(1, 0);
	__m128 chunk4 = table.get_chunk(1, 1);
	
	BOOST_CHECK_CLOSE(*((float_type *) &chunk1),  .0f, 0.001);
	BOOST_CHECK_CLOSE(*(((float_type *) &chunk1) + 1), 1.0f, 0.001);
	BOOST_CHECK_CLOSE(*(((float_type *) &chunk1) + 2), 2.0f, 0.001);
	BOOST_CHECK_CLOSE(*(((float_type *) &chunk1) + 3), 3.0f, 0.001);
	
	BOOST_CHECK_CLOSE(*((float_type *) &chunk2),  4.0f, 0.001);
	
	BOOST_CHECK_CLOSE(*((float_type *) &chunk3),  .0f, 0.001);
	BOOST_CHECK_CLOSE(*(((float_type *) &chunk3) + 1), 1.0f, 0.001);
	BOOST_CHECK_CLOSE(*(((float_type *) &chunk3) + 2), 2.0f, 0.001);
	BOOST_CHECK_CLOSE(*(((float_type *) &chunk3) + 3), 3.0f, 0.001);
	
	BOOST_CHECK_CLOSE(*((float_type *) &chunk4),  4.0f, 0.001);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(add_test, Table, test_types_all) {
	Table table(6, 5);
	
	for(int r = 0; r < table.get_no_rows(); ++r)
		for(int c = 0; c < table.get_no_columns(); ++c)
			table(r, c) =  c;
	
	table += 3.0;
	
	for(int r = 0; r < table.get_no_rows(); ++r)
		for(int c = 0; c < table.get_no_columns(); ++c)
			BOOST_CHECK_CLOSE(table(r, c), c + 3.0, 0.001);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(sub_test, Table, test_types_all) {
	Table table(6, 5);
	
	for(int r = 0; r < table.get_no_rows(); ++r)
		for(int c = 0; c < table.get_no_columns(); ++c)
			table(r, c) = c;
	
	table -= 3.0;
	
	for(int r = 0; r < table.get_no_rows(); ++r)
		for(int c = 0; c < table.get_no_columns(); ++c)
			BOOST_CHECK_CLOSE(table(r, c), c - 3.0, 0.001);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(mul_test, Table, test_types_all) {
	Table table(6, 5);
	
	for(int r = 0; r < table.get_no_rows(); ++r)
		for(int c = 0; c < table.get_no_columns(); ++c)
			table(r, c) = c;
	
	table *= 3.0;
	
	for(int r = 0; r < table.get_no_rows(); ++r)
		for(int c = 0; c < table.get_no_columns(); ++c)
			BOOST_CHECK_CLOSE(table(r, c), c * 3.0, 0.001);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(div_test, Table, test_types_all) {
	Table table(6, 5);
	
	for(int r = 0; r < table.get_no_rows(); ++r)
		for(int c = 0; c < table.get_no_columns(); ++c)
			table(r, c) = c;
	
	table /= 3.0;
	
	for(int r = 0; r < table.get_no_rows(); ++r)
		for(int c = 0; c < table.get_no_columns(); ++c)
			BOOST_CHECK_CLOSE(table(r, c), c / 3.0, 0.001);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(row_sum_test, Table, test_types_all) {
	Table table(6, 5);
	
	for(int r = 0; r < table.get_no_rows(); ++r)
		for(int c = 0; c < table.get_no_columns(); ++c)
			table(r, c) =  c;
	
	BOOST_CHECK_CLOSE(table.row_sum(0), 0+1+2+3+4, 0.001);
	BOOST_CHECK_CLOSE(table.row_sum(1), 0+1+2+3+4, 0.001);
	BOOST_CHECK_CLOSE(table.row_sum(2), 0+1+2+3+4, 0.001);
	BOOST_CHECK_CLOSE(table.row_sum(3), 0+1+2+3+4, 0.001);
	BOOST_CHECK_CLOSE(table.row_sum(4), 0+1+2+3+4, 0.001);
	BOOST_CHECK_CLOSE(table.row_sum(5), 0+1+2+3+4, 0.001);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(column_sum_test, Table, test_types_all) {
	Table table(6, 5);
	
	for(int r = 0; r < table.get_no_rows(); ++r)
		for(int c = 0; c < table.get_no_columns(); ++c)
			table(r, c) = r;
	
	BOOST_CHECK_CLOSE(table.column_sum(0), 0+1+2+3+4+5, 0.001);
	BOOST_CHECK_CLOSE(table.column_sum(1), 0+1+2+3+4+5, 0.001);
	BOOST_CHECK_CLOSE(table.column_sum(2), 0+1+2+3+4+5, 0.001);
	BOOST_CHECK_CLOSE(table.column_sum(3), 0+1+2+3+4+5, 0.001);
	BOOST_CHECK_CLOSE(table.column_sum(4), 0+1+2+3+4+5, 0.001);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(transpose_test, Table, test_types_all) {
  Table table(6, 5);
	
  for(int r = 0; r < table.get_no_rows(); ++r)
    for(int c = 0; c < table.get_no_columns(); ++c)
      table(r, c) = c;
	
  Table transpose(5, 6);
  table.transpose(transpose);
	
  for(int r = 0; r < transpose.get_no_rows(); ++r)
    for(int c = 0; c < transpose.get_no_columns(); ++c)
      BOOST_CHECK_CLOSE(transpose(r, c), r, 0.001);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(log_non_sse_test, Table, test_types_all) {
  Table table(6,5);
  for(int r = 0; r < table.get_no_rows(); ++r)
    for(int c = 0; c < table.get_no_columns(); ++c)
      table(r, c) = c+1;

  Table log_table(6,5);
  table.log(log_table);

  for(int r = 0; r < table.get_no_rows(); ++r)
    for(int c = 0; c < table.get_no_columns(); ++c)
      BOOST_CHECK_CLOSE(log_table(r,c), std::log(c+1), 0.001);
}

BOOST_AUTO_TEST_SUITE_END()

