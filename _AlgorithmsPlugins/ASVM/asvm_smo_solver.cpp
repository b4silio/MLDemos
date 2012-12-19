/*
 * Copyright (C) 2012 Learning Algorithms and Systems Laboratory, EPFL, Switzerland
 * 
 *  asvm_smo_solver.cpp
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


#include "asvm_smo_solver.h"
#include "svm.h"
#include <iomanip>

void ASVM_SMO_Solver::configure(const char* filename)
{
	int dum;
	FILE* file = NULL;
	file = fopen(filename,"r");
	char tmpstr[1025];
	double tmpdbl;
	int tmpint;
	if(file != NULL)
	{
		cout<<"Reading Configuration from file "<<filename<<"..."<<endl;
		while(!feof(file))
		{
			if(fscanf(file, "%s", tmpstr) >= 0)
			{
				if(!strcmp(tmpstr, "#"))
				{
					break;
				}
				else if(!strcmp(tmpstr, "C"))
				{
					if((dum=fscanf(file, "%lf", &tmpdbl)) > 0)
					{

						Cparam = tmpdbl;

					}

					fseek(file, -dum,SEEK_CUR);
				}
				else if(!strcmp(tmpstr, "classification_tol"))
				{
					if((dum=fscanf(file, "%lf", &tmpdbl)) > 0)
					{
						alpha_tol = tmpdbl;
					}

					fseek(file, -dum,SEEK_CUR);
				}
				else if(!strcmp(tmpstr, "lyapunov_tol"))
				{
					if((dum=fscanf(file, "%lf", &tmpdbl)) > 0)
					{
						beta_tol = tmpdbl;
					}

					fseek(file, -dum,SEEK_CUR);
				}
				else if(!strcmp(tmpstr, "lyapunov_relaxation"))
				{
					if((dum=fscanf(file, "%lf", &tmpdbl)) > 0)
					{
						beta_relax = tmpdbl;
					}

					fseek(file, -dum,SEEK_CUR);
				}
				else if(!strcmp(tmpstr, "max_eval"))
				{
					if((dum=fscanf(file, "%d", &tmpint)) > 0)
					{
						max_iter = tmpint;
					}

					fseek(file, -dum,SEEK_CUR);
				}
				else if(!strcmp(tmpstr, "verbose"))
				{
					if((dum=fscanf(file, "%s", tmpstr)) > 0)
					{
						if(!strcmp("on", tmpstr))
							bVerbose = true;
						else
							bVerbose = false;
					}

					fseek(file, -dum,SEEK_CUR);
				}
			}
		}
	}
	else
		cout<<"WARNING: Configuration file not found!! Using default configuration..."<<endl;

	cout<<"C                   : "<<Cparam<<endl;
	cout<<"Verbose             : "<<bVerbose<<endl;
	cout<<"max_eval            : "<<max_iter<<endl;
	cout<<"classification_tol  : "<<alpha_tol<<endl;
	cout<<"lyapunov_tol        : "<<beta_tol<<endl;
	cout<<"lyapunov_relaxation : "<<beta_relax<<endl;

	cout<<"Done."<<endl;

}


int ASVM_SMO_Solver::learn(asvmdata& input1, unsigned int tclass, asvm* svmobj)
{

	unsigned int i,j;

	asvmdata *copy_data = new asvmdata(input1);
	copy_data->preprocess(tclass);

	cout<<"Reading problem..."<<endl;
	cout<<"Dimension    : "<<copy_data->dim<<endl;
	cout<<"Num Alpha    : "<<copy_data->num_alpha<<endl;
	cout<<"Num Beta     : "<<copy_data->num_beta <<endl;
	cout<<"Target Class : "<<tclass<<endl;

	M = copy_data->num_alpha;
	P = copy_data->num_beta;
	N = copy_data->dim;
	lambda = copy_data->lambda;

	x_smo = new double[M+P+N];
	init_warm_start(copy_data);


	dlabels = new int[M];
	for(i=0;i<M;i++)

		dlabels[i] = copy_data->labels[i];

	ker_matrix = new double*[M+P+N];
	for(i=0;i<M+P+N;i++)
	{
		ker_matrix[i] = new double[M+P+N];
		for(j=0;j<M+P+N;j++)
			ker_matrix[i][j] = copy_data->matkgh[i][j];

	}

	for(i=0;i<M;i++)
	{
		for (j = 0;j<M;j++)
			ker_matrix[i][j] *= dlabels[i]*dlabels[j];

		for(j = M; j<M+P+N; j++)
		{
			ker_matrix[i][j] *= dlabels[i];
			ker_matrix[j][i] = ker_matrix[i][j];
		}
	}


//	FILE* file = fopen("Q.txt", "w");
//	for(unsigned int i=0;i<M+P+N;i++)
//	{
//		for(unsigned int j=0;j<M+P+N;j++)
//			fprintf(file, "%lf  ", copy_data->matkgh[i][j]);
//		fprintf(file, "\n");
//	}
//	fclose(file);

	err_cache_alpha = new double[M];
	for(i=0;i<M;i++)
		err_cache_alpha[i] = forward_alpha(i) - dlabels[i];

	err_cache_beta = new double[P];
	for(i=0;i<P;i++)
	{
		err_cache_beta[i] = forward_beta(i+M);
	}



	H_ii = new double[P];
	for(i = 0;i<P;i++)
		H_ii[i] = ker_matrix[M+i][M+i];

	minimum_alpha = -1;
	maximum_alpha = -1;
	double tmp_mn = 1e40;
	double tmp_mx = -1e40;
	for(i=0;i<M;i++)
	{
		if(tmp_mn > err_cache_alpha[i])
		{
			tmp_mn = err_cache_alpha[i];
			minimum_alpha = i;
		}
		if(tmp_mx < err_cache_alpha[i])
		{
			tmp_mx = err_cache_alpha[i];
			maximum_alpha = i;
		}
	}


	cout<<"Starting SMO..."<<endl;
	fflush(stdout);

//	clock_t begin = clock();
	doSMO();
//	clock_t ending = clock();
	cout<<"Done"<<endl;
    double elapsed = 0;
    //((double)(ending - begin))/(double)CLOCKS_PER_SEC;

	svmobj->setSVMFromOptimizedSol(x_smo, copy_data);
	svmobj->printinfo();

	cout<<endl<<"*****************************************************************"<<endl;
    cout<<"Alpha error     : "<<(svmobj->alpha ? fabs(svmobj->getclassifiervalue(svmobj->svalpha[0])-svmobj->y[0]) : 0 )<<endl;

	int err=0;
	for(i=M;i<M+P;i++)
		err += MYMIN((forward_beta(i)), 0);
	cout<<"Beta error      : "<<err<<endl;

	cout<<"Gamma error     : ";
	for(i=0;i<N;i++)
		cout<<forward_gamma(M+P+i)<<"  ";
	cout<<endl;
	double tmp=0;
	for(i=0;i<M;i++)
		tmp += x_smo[i]*dlabels[i];
	cout<<"Sum(alpha.*y)   : "<<tmp<<endl;
//	printf("Time elapsed    : %.10f sec\n", elapsed);
	cout<<"Time elapsed    : "<<elapsed<<" sec."<<endl;
	cout<<"*****************************************************************"<<endl<<endl;

	if(iter >= max_iter)
	{
		cout<<"WARNING: Max iterations exceeded!!"<<endl;
		return 1;
	}
	else
	{
		cout<<"SMO finished successfully."<<endl;
		return 0;
	}
}


void ASVM_SMO_Solver::doSMO()
{

	unsigned int i, numChanged = 0;
	bool examineAll = true;
	iter=0;

	while(iter++ < max_iter && (numChanged > 0 || examineAll))
	{
		if(bVerbose)
			cout<<"Pass "<<iter<<endl;

		numChanged = 0;

		if(examineAll)
		{
			if(bVerbose)
				cout<<"Examine all..."<<endl;
			for(i = 0;i<M;i++)
				numChanged += examineForAlpha(i);

			if(bVerbose)
				cout<<"NumChanged after Alpha = "<<numChanged<<endl;

			for (i = M;i<M+P;i++)
				numChanged += examineForBeta(i);

			if(bVerbose)
				cout<<"NumChanged after Beta = "<<numChanged<<endl;

			for (i = M+P;i<M+P+N;i++)
				numChanged += examineForGamma(i);

			if(bVerbose)
				cout<<"NumChanged after Gamma = "<<numChanged<<endl;

			examineAll = false;
		}
		else
		{
			if(bVerbose)
				cout<<"Examine active..."<<endl;
			for(i = 0;i<M;i++)
				if (x_smo[i] > 0 && x_smo[i] < Cparam)
					numChanged += examineForAlpha(i);

			if(bVerbose)
				cout<<"NumChanged after Alpha = "<<numChanged<<endl;

			for (i = M;i<M+P;i++)
				if (x_smo[i] > 0 && x_smo[i] < Cparam)
					numChanged += examineForBeta(i);

			if(bVerbose)
				cout<<"NumChanged after Beta = "<<numChanged<<endl;

			for (i = M+P;i<M+P+N;i++)
				numChanged += examineForGamma(i);

			if(bVerbose)
				cout<<"NumChanged after Gamma = "<<numChanged<<endl;

			if ( numChanged == 0 )
				examineAll = true;
		}


		updateB0();		//TODO: different from matlab. check this

	}

}

bool ASVM_SMO_Solver::examineForAlpha(unsigned int i2)
{
	int y2 = dlabels[i2];
	double alph2 = x_smo[i2];
	double E2;

	if(alph2 > 0 && alph2 < Cparam)
		E2 = err_cache_alpha[i2];
	else
		E2 = forward_alpha(i2) - y2;
	double r2 = E2*y2;
	int i1;
	if ((r2 < -alpha_tol && alph2 < Cparam) || (r2 > alpha_tol && alph2 > 0))
	{
		if(fabs(E2 - err_cache_alpha[minimum_alpha]) > fabs(E2 - err_cache_alpha[maximum_alpha]))
			i1 = minimum_alpha;
		else
			i1 = maximum_alpha;

		if ( takeStepForAlpha(i1, i2, E2) )
			return true;


		for(unsigned int i =0;i<M;i++)
			if(x_smo[i] > 0 && x_smo[i] < Cparam)
			{
				if( takeStepForAlpha(i, i2, E2))
					return true;
			}

		for(unsigned int i=0;i<M;i++)
			if ( x_smo[i] == 0 || x_smo[i] == Cparam)
			{
				if( takeStepForAlpha(i, i2, E2))
					return true;
			}
	}
	return false;
}

bool ASVM_SMO_Solver::takeStepForAlpha(unsigned int i1, unsigned int i2, double E2)
{
	if(i1==i2)
		return false;

	double alph1 = x_smo[i1];
	double alph2 = x_smo[i2];
	int y1 = dlabels[i1];
	int y2 = dlabels[i2];


	double E1;
	if(alph1 > 0 && alph1 < Cparam)
		E1 = err_cache_alpha[i1];
	else
		E1 = forward_alpha(i1) - y1;

	double L,H;
	if(y1 != y2)
	{
		L = MYMAX(0, (alph2-alph1));
		H = MYMIN(Cparam, (Cparam+alph2-alph1));
	}
	else
	{

		L = MYMAX(0, (alph2+alph1-Cparam));
		H = MYMIN(Cparam, (alph2+alph1));
	}
	if(fabs(L-H) < alpha_tol)
		return false;

	double eta = ker_matrix[i1][i1] + ker_matrix[i2][i2] - 2*ker_matrix[i1][i2];
	double a2;
	if(eta > 0)
	{
		a2 = alph2 + y2*(E1-E2)/eta;
		if(a2 < L) a2 = L;
		else if(a2 > H) a2 = H;
	}
	else
	{
		return false;
	}

	if(fabs(a2-alph2) < (a2+alph2+alpha_tol)*alpha_tol)
		return false;

	double a1 = alph1 + y1*y2*(alph2-a2);
	if(a1 < alpha_tol)
		a1=0;

	x_smo[i1] = a1; x_smo[i2] = a2;

	double w1 = y1*(a1-alph1);
	double w2 = y2*(a2-alph2);

	if(a1 > 0 && a1 < Cparam)
		err_cache_alpha[i1] = forward_alpha(i1) - y1;

	if(a2 > 0 && a2 < Cparam)
		err_cache_alpha[i2] = forward_alpha(i2) - y2;

	if(err_cache_alpha[i1] > err_cache_alpha[i2])
	{
		minimum_alpha = i2;
		maximum_alpha = i1;
	}

	if(err_cache_alpha[i1] < err_cache_alpha[i2])
	{
		minimum_alpha = i1;
		maximum_alpha = i2;
	}


	register double *xptr = x_smo;
	register double *kerptr1 = ker_matrix[i1];
	register double *kerptr2 = ker_matrix[i2];
	register double *errptr = err_cache_alpha;
	register double *errptr2 = err_cache_beta;
	double theMax = err_cache_alpha[maximum_alpha];
	double theMin = err_cache_alpha[minimum_alpha];
	unsigned int i=0;
	for(;i<M;++i)
	{
		if (i!=i1 && i!=i2 && *xptr > 0 && *xptr<Cparam)
		{
			*errptr += w1*(*kerptr1) + w2*(*kerptr2);
			if(*errptr > theMax)
				maximum_alpha = i;
			if(*errptr < theMin)
				minimum_alpha = i;
		}
		++xptr;
		++errptr;
		++kerptr1;
		++kerptr2;
	}
	for(;i<M+P;++i)
	{
		if(*xptr > 0 && *xptr < Cparam)
			*errptr2 += w1*(*kerptr1) + w2*(*kerptr2);
		++xptr;
		++errptr2;
		++kerptr1;
		++kerptr2;
	}

	/*
	unsigned int i;
	for(;i<M;i++)
	{
		if (  i!=i1 && i!=i2 && x_smo[i] > 0 && x_smo[i]<Cparam)
		{
			err_cache_alpha[i] += w1*ker_matrix[i1][i] + w2*ker_matrix[i2][i];

			//			cout<<err_cache_alpha[i]<<endl;

			if(err_cache_alpha[i] > err_cache_alpha[maximum_alpha])
				maximum_alpha = i;
			if(err_cache_alpha[i] < err_cache_alpha[minimum_alpha])
				minimum_alpha = i;
		}
	}

	//	cout<<err_cache_alpha[minimum_alpha]<<" "<<err_cache_alpha[maximum_alpha]<<endl;
	//	cout<<minimum_alpha<<" "<<maximum_alpha<<endl;
	//getchar();
	for(;i<M+P;i++)
		if(x_smo[i] > 0 && x_smo[i] < Cparam)
			err_cache_beta[i-M] += w1*ker_matrix[i][i1] + w2*ker_matrix[i][i2];
*/

	return true;
}

bool ASVM_SMO_Solver::examineForBeta(unsigned int i1)
{
	double beta1 = x_smo[i1];
	double E1;

	if(beta1 > 0 && beta1 < Cparam)
		E1 = err_cache_beta[i1-M];
	else
		E1 = forward_beta(i1);

	if ((E1 < -beta_tol && x_smo[i1] < Cparam) || (E1 > beta_tol && x_smo[i1] > 0))
		if (takeStepForBeta(i1, E1))
			return true;

	return false;
}

bool ASVM_SMO_Solver::takeStepForBeta(unsigned int i1, double E1)
{
	double beta1 = x_smo[i1];
	double quad_term = H_ii[i1-M];
	double beta_new, bdiff;

	if(quad_term > 0)
		beta_new = beta1 - E1/quad_term;
	else
	{
		cout<<"H_ii("<<i1-M<<") = "<<quad_term<<" !! Expected positive"<<endl;
		return false;
	}

	if (beta_new < 0) beta_new = 0;
	else if (beta_new > Cparam) beta_new = Cparam;

	bdiff = beta_new - beta1;

	if(fabs(bdiff) < (beta_new + beta1 + beta_tol)*beta_tol)
		return false;

	x_smo[i1] = beta_new;


	if(beta_new > 0 && beta_new < Cparam)
		err_cache_beta[i1-M] = forward_beta(i1);

	register double *xptr = x_smo;
	register double *kerptr1 = ker_matrix[i1];
	register double *errptr = err_cache_alpha;
	register double *errptr2 = err_cache_beta;
	double theMax = err_cache_alpha[maximum_alpha];
	double theMin = err_cache_alpha[minimum_alpha];
	unsigned int i=0;
	for(;i<M;++i)
	{
		if (*xptr > 0 && *xptr<Cparam)
		{
			*errptr += bdiff*(*kerptr1);
			if(*errptr > theMax)
				maximum_alpha = i;
			if(*errptr < theMin)
				minimum_alpha = i;
		}
		++xptr;
		++errptr;
		++kerptr1;
	}
	for(;i<M+P;++i)
	{
		if ( i!=i1 && *xptr > 0 && *xptr < Cparam )
			*errptr2 += bdiff*(*kerptr1);
		++xptr;
		++errptr2;
		++kerptr1;
	}
	/*
	unsigned int i;

	for(i=M;i<M+P;i++)
		if ( i!=i1 && x_smo[i] > 0 && x_smo[i] < Cparam )
			err_cache_beta[i-M] += ker_matrix[i1][i]*bdiff;

	for(i=0;i<M;i++)
		if(x_smo[i] > 0 && x_smo[i] < Cparam)
		{
			err_cache_alpha[i] += ker_matrix[i][i1]*bdiff;

			if(err_cache_alpha[i] > err_cache_alpha[maximum_alpha])
				maximum_alpha = i;
			if(err_cache_alpha[i] < err_cache_alpha[minimum_alpha])
				minimum_alpha = i;
		}
*/


	return true;
}

bool  ASVM_SMO_Solver::examineForGamma(unsigned int i1)
{
	double E1 = forward_gamma(i1);

	if(fabs(E1) > alpha_tol)
		if(takeStepForGamma(i1, E1))
			return true;

	return false;
}

bool ASVM_SMO_Solver::takeStepForGamma(unsigned int i1, double E1)
{
	double g1 = x_smo[i1];
	double gamma_new = g1 - E1/(2*lambda);
	double gdiff = gamma_new - g1;

	if(fabs(gdiff) < alpha_tol || fabs(gamma_new) < alpha_tol)
		return false;

	x_smo[i1] = gamma_new;

	unsigned int i;

	for(i=0;i<M;i++)
		if (x_smo[i] > 0 && x_smo[i] < Cparam)
		{
			err_cache_alpha[i] += ker_matrix[i][i1]*gdiff;

			if(err_cache_alpha[i] > err_cache_alpha[maximum_alpha])
			maximum_alpha = i;
			if(err_cache_alpha[i] < err_cache_alpha[minimum_alpha])
			minimum_alpha = i;
		}

	for(i=M;i<M+P;i++)
		if(x_smo[i] > 0 && x_smo[i] < Cparam)
			err_cache_beta[i-M] += ker_matrix[i][i1]*gdiff;

	return true;
}

void ASVM_SMO_Solver::updateB0()
{
	double b_old = Bparam;
	double fnc = 0;
	int cnt=0;
	unsigned int i;
	register double *xptr = x_smo;
	register int *dlabptr = dlabels;
	for(i=0;i<M;i++)
	{
		if(*xptr > 0 && *xptr < Cparam)
		{
			fnc += forward_alpha(i) + Bparam - *dlabptr;
			cnt++;
		}
		++xptr;
		++dlabptr;
	}

	Bparam = fnc/cnt;

	register double *errptr = err_cache_alpha;
	xptr = x_smo;
	double theMax = err_cache_alpha[maximum_alpha];
	double theMin = err_cache_alpha[minimum_alpha];

	for(i=0;i<M;i++)
	{
		if(*xptr > 0 && *xptr < Cparam)
		{
			*errptr += b_old - Bparam;

			if(*errptr > theMax)
				maximum_alpha = i;
			if(*errptr < theMin)
				minimum_alpha = i;
		}

		++xptr;
		++errptr;
	}
}

double ASVM_SMO_Solver::forward_alpha(int index)
{
	return forward(index) -Bparam;
}

double ASVM_SMO_Solver::forward_beta(int index)
{
	return forward(index);
}

double ASVM_SMO_Solver::forward_gamma(int index)
{
	return forward(index);
}

double ASVM_SMO_Solver::forward(int index)
{
	double fval = 0.0;

	register double *kerptr = ker_matrix[index];
	register int *dlabptr = dlabels;
	register double *xptr = x_smo;
	unsigned int i=0;
	double tmp;
	while(i<M)
	{
		tmp = (*kerptr)*(*xptr);
		if(*dlabptr==1)
			fval += tmp;
		else
			fval -= tmp;
		++kerptr;
		++dlabptr;
		++xptr;
		++i;
	}

	while(i<M+P+N)
	{
		fval += (*kerptr)*(*xptr);
		++kerptr;
		++xptr;
		++i;
	}
	/*
	for(unsigned int i=0;i<M+P+N;i++)
	{
			if(i < M)
			{
				if ( x_smo[i] > 0 )
					fval += ker_matrix[index][i]*dlabels[i]*x_smo[i];
			}
			else if( i < M+P)
			{
				if ( x_smo[i] > 0 )
					fval += ker_matrix[index][i]*x_smo[i];
			}
			else
			{
				fval += ker_matrix[index][i]*x_smo[i];
			}
	}
	/**/

//	cout<<fval<<endl;

	return fval;
}

void ASVM_SMO_Solver::init_warm_start(asvmdata* copy_data)
{
	unsigned int i,j,k,l;
	cout<<"Warm starting with libsvm classifier-only solution..."<<endl;
	svm_problem prob;
	prob.l = M;
	prob.y = new double[M];
	prob.x = new svm_node*[M];
	svm_node* x_space = new svm_node[(N+1)*M];

	unsigned int ind=0;
	for( i=0; i<copy_data->tar.size(); i++)
			for( j=0; j<copy_data->tar[i].traj.size(); j++)
				for( k=0; k<copy_data->tar[i].traj[j].nPoints-1 ; k++, ind++)
				{
					for(l=0;l<N;l++)
					{
						x_space[(N+1)*ind + l].index = l+1;
						x_space[(N+1)*ind + l].value = copy_data->tar[i].traj[j].coords[k][l];
					}
					x_space[(N+1)*ind + l].index = -1;

					prob.x[ind] = &x_space[(N+1)*ind];
					prob.y[ind] = (double)copy_data->labels[ind];
				}

	svm_parameter param;
	param.C = Cparam;
	param.svm_type = C_SVC;
	param.kernel_type = RBF;
	param.gamma = lambda;
	param.eps = alpha_tol;
	param.nr_weight = 0;

	svm_model *model = svm_train(&prob, &param);

	for(i=0;i<M;i++)
		x_smo[i] = 0.0;
	for(i=M;i<M+P;i++)
		x_smo[i] = 0.0;
	for(i=M+P;i<M+P+N;i++)
		x_smo[i] = 0.0;

	for(int a = 0; a< model->l ;a++)
		x_smo[model->SV_indices[a]] = fabs(model->sv_coef[0][a]);

	Bparam = model->rho[0];

	cout<<"Done."<<endl;

}

