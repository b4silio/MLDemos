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

#define sampletype dlib::matrix<double,N,1>
#define rbfkernel dlib::radial_basis_kernel< sampletype >
#define linkernel dlib::linear_kernel< sampletype >
#define polkernel dlib::polynomial_kernel< sampletype >
#define rbffunc dlib::decision_function< rbfkernel >
#define linfunc dlib::decision_function< linkernel >
#define polfunc dlib::decision_function< polkernel >

typedef dlib::matrix<double, 0, 1> sample_type;
typedef dlib::radial_basis_kernel<sample_type> rbf_kernel;
typedef dlib::polynomial_kernel<sample_type> pol_kernel;
typedef dlib::linear_kernel<sample_type> lin_kernel;
typedef dlib::decision_function<lin_kernel> lin_func;
typedef dlib::decision_function<pol_kernel> pol_func;
typedef dlib::decision_function<rbf_kernel> rbf_func;

typedef dlib::matrix<double, 0, 1> reg_sample_type;
typedef dlib::radial_basis_kernel<reg_sample_type> reg_rbf_kernel;
typedef dlib::polynomial_kernel<reg_sample_type> reg_pol_kernel;
typedef dlib::linear_kernel<reg_sample_type> reg_lin_kernel;
typedef dlib::decision_function<reg_lin_kernel> reg_lin_func;
typedef dlib::decision_function<reg_pol_kernel> reg_pol_func;
typedef dlib::decision_function<reg_rbf_kernel> reg_rbf_func;

#endif // _DLIB_TYPES_H_
