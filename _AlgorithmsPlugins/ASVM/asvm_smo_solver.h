/*
 * Copyright (C) 2012 Learning Algorithms and Systems Laboratory, EPFL, Switzerland
 * 
 *  asvm_smo_solver.h
 *
 *  Created on : Aug 14, 2012
 *  Author     : Ashwini Shukla
 *  Email      : ashwini.shukla@epfl.ch
 *  Website    : lasa.epfl.ch
 *
 * Permission is granted to copy, distribute, and/or modify this program
 * under the terms of the GNU General Public License, version 2 or any
 * later version published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
 */


#ifndef ASVM_SMO_SOLVER_H_
#define ASVM_SMO_SOLVER_H_

#include "asvm.h"


class ASVM_SMO_Solver
{

public:
    /* model parameters*/
    double beta_tol;
    double alpha_tol;
    double Cparam;
    double beta_relax;
    double lambda;
    int max_iter;

private:

	/* temporary variables for SMO*/
	bool isStopRequested;
	double *x_smo;
	int *dlabels;
	double* err_cache_alpha;
	double* err_cache_beta;
	double Bparam;
	double** ker_matrix;
	unsigned int M, P, N;
	int maximum_alpha, minimum_alpha;
	double* H_ii;

	bool bVerbose;
    int iter;


public:

	ASVM_SMO_Solver()
	{
		isStopRequested = false;
		bVerbose = false;
		beta_relax = 0.0;
		Cparam = 1e8;
		beta_tol = 0.001;
		alpha_tol = 0.001;
		Bparam = 0.0;
		max_iter = 1e8;
		iter=0;
	}

	void configure(const char* filename);
	int learn(asvmdata& input1, unsigned int tclass, asvm* svmobj);

	void setLyapunovRelaxation(double rel)		{ beta_relax = rel;}
	void setCParam(double C) 					{ Cparam = C;}
	void setLyapunovTol(double tol)				{ beta_tol = tol;}
	void setClassificationTol(double tol)		{ alpha_tol = tol; }
	void force_stop() 							{ isStopRequested = true; }

private:

	void doSMO();
	bool  examineForAlpha(unsigned int index);
	bool  examineForBeta(unsigned int index);
	bool  examineForGamma(unsigned int index);
	void updateB0();

	bool takeStepForAlpha(unsigned int i1, unsigned int i2, double E2);
	bool takeStepForBeta(unsigned int i1, double E1);
	bool takeStepForGamma(unsigned int i1, double E1);

	double forward_alpha(int index);
	double forward_beta(int index);
	double forward_gamma(int index);
	double forward(int index);

	void init_warm_start(asvmdata* copy_data);

};

#endif /* ASVM_SMO_SOLVER_H_ */
