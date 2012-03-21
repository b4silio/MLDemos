/*********************************************************************
MLDemos: A User-Friendly visualization toolkit for machine learning
Copyright (C) 2010  Basilio Noris
Contact: mldemos@b4silio.com

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public License,
version 3 as published by the Free Software Foundation.

This library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free
Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*********************************************************************/
#ifndef _DLIB_TYPES_H_
#define _DLIB_TYPES_H_

// dim=2
typedef dlib::matrix<double, 2, 1> sample_type;
typedef dlib::radial_basis_kernel<sample_type> rbf_kernel;
typedef dlib::polynomial_kernel<sample_type> pol_kernel;
typedef dlib::linear_kernel<sample_type> lin_kernel;
typedef dlib::decision_function<lin_kernel> lin_func;
typedef dlib::decision_function<pol_kernel> pol_func;
typedef dlib::decision_function<rbf_kernel> rbf_func;

// dim=2
typedef dlib::matrix<double, 2, 1> sample_type2;
typedef dlib::radial_basis_kernel<sample_type2> rbf_kernel2;
typedef dlib::polynomial_kernel<sample_type2> pol_kernel2;
typedef dlib::linear_kernel<sample_type2> lin_kernel2;
typedef dlib::decision_function<lin_kernel2> lin_func2;
typedef dlib::decision_function<pol_kernel2> pol_func2;
typedef dlib::decision_function<rbf_kernel2> rbf_func2;

// dim=3
typedef dlib::matrix<double, 3, 1> sample_type3;
typedef dlib::radial_basis_kernel<sample_type3> rbf_kernel3;
typedef dlib::polynomial_kernel<sample_type3> pol_kernel3;
typedef dlib::linear_kernel<sample_type3> lin_kernel3;
typedef dlib::decision_function<lin_kernel3> lin_func3;
typedef dlib::decision_function<pol_kernel3> pol_func3;
typedef dlib::decision_function<rbf_kernel3> rbf_func3;

// dim=4
typedef dlib::matrix<double, 4, 1> sample_type4;
typedef dlib::radial_basis_kernel<sample_type4> rbf_kernel4;
typedef dlib::polynomial_kernel<sample_type4> pol_kernel4;
typedef dlib::linear_kernel<sample_type4> lin_kernel4;
typedef dlib::decision_function<lin_kernel4> lin_func4;
typedef dlib::decision_function<pol_kernel4> pol_func4;
typedef dlib::decision_function<rbf_kernel4> rbf_func4;

// dim=5
typedef dlib::matrix<double, 5, 1> sample_type5;
typedef dlib::radial_basis_kernel<sample_type5> rbf_kernel5;
typedef dlib::polynomial_kernel<sample_type5> pol_kernel5;
typedef dlib::linear_kernel<sample_type5> lin_kernel5;
typedef dlib::decision_function<lin_kernel5> lin_func5;
typedef dlib::decision_function<pol_kernel5> pol_func5;
typedef dlib::decision_function<rbf_kernel5> rbf_func5;

// dim=6
typedef dlib::matrix<double, 6, 1> sample_type6;
typedef dlib::radial_basis_kernel<sample_type6> rbf_kernel6;
typedef dlib::polynomial_kernel<sample_type6> pol_kernel6;
typedef dlib::linear_kernel<sample_type6> lin_kernel6;
typedef dlib::decision_function<lin_kernel6> lin_func6;
typedef dlib::decision_function<pol_kernel6> pol_func6;
typedef dlib::decision_function<rbf_kernel6> rbf_func6;

// dim=7
typedef dlib::matrix<double, 7, 1> sample_type7;
typedef dlib::radial_basis_kernel<sample_type7> rbf_kernel7;
typedef dlib::polynomial_kernel<sample_type7> pol_kernel7;
typedef dlib::linear_kernel<sample_type7> lin_kernel7;
typedef dlib::decision_function<lin_kernel7> lin_func7;
typedef dlib::decision_function<pol_kernel7> pol_func7;
typedef dlib::decision_function<rbf_kernel7> rbf_func7;

// dim=8
typedef dlib::matrix<double, 8, 1> sample_type8;
typedef dlib::radial_basis_kernel<sample_type8> rbf_kernel8;
typedef dlib::polynomial_kernel<sample_type8> pol_kernel8;
typedef dlib::linear_kernel<sample_type8> lin_kernel8;
typedef dlib::decision_function<lin_kernel8> lin_func8;
typedef dlib::decision_function<pol_kernel8> pol_func8;
typedef dlib::decision_function<rbf_kernel8> rbf_func8;

// dim=9
typedef dlib::matrix<double, 9, 1> sample_type9;
typedef dlib::radial_basis_kernel<sample_type9> rbf_kernel9;
typedef dlib::polynomial_kernel<sample_type9> pol_kernel9;
typedef dlib::linear_kernel<sample_type9> lin_kernel9;
typedef dlib::decision_function<lin_kernel9> lin_func9;
typedef dlib::decision_function<pol_kernel9> pol_func9;
typedef dlib::decision_function<rbf_kernel9> rbf_func9;

// dim=10
typedef dlib::matrix<double, 10, 1> sample_type10;
typedef dlib::radial_basis_kernel<sample_type10> rbf_kernel10;
typedef dlib::polynomial_kernel<sample_type10> pol_kernel10;
typedef dlib::linear_kernel<sample_type10> lin_kernel10;
typedef dlib::decision_function<lin_kernel10> lin_func10;
typedef dlib::decision_function<pol_kernel10> pol_func10;
typedef dlib::decision_function<rbf_kernel10> rbf_func10;


typedef dlib::matrix<double, 1, 1> reg_sample_type;
typedef dlib::radial_basis_kernel<reg_sample_type> reg_rbf_kernel;
typedef dlib::polynomial_kernel<reg_sample_type> reg_pol_kernel;
typedef dlib::linear_kernel<reg_sample_type> reg_lin_kernel;
typedef dlib::decision_function<reg_lin_kernel> reg_lin_func;
typedef dlib::decision_function<reg_pol_kernel> reg_pol_func;
typedef dlib::decision_function<reg_rbf_kernel> reg_rbf_func;

#endif // _DLIB_TYPES_H_
