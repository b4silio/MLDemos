
#include "asvm_nlopt_solver.h"


double * x_curr;

double objective(unsigned n, const double *x, double *grad, void *my_func_data)
{

	unsigned int nalpha =  ((obj_func_data*)my_func_data)->num_alpha;
	double** matkgh = ((obj_func_data*)my_func_data)->NLModulationKernel;
	bool b = ((obj_func_data*)my_func_data)->bVerbose;

	x_curr = (double*)x;

	unsigned int i,j;
	double val =0.0;
	double alphasum = 0.0;					//sum of alpha values
	for(i=0; i<nalpha; i++)
		alphasum += x[i];


	for(i=0;i<n;i++)
		for( j=0;j<n;j++)
			val +=x[i]*x[j]*matkgh[i][j];

	val = 0.5*val - alphasum;

	if(b)
		    printf("Objective Value:    %10.5lf\n", val);

	if(grad)
	{
		for( i=0;i<n;i++)
		{
			grad[i] = 0.0;

			for(j=0;j<n;j++)
				grad[i] += matkgh[i][j]*x[j];

		}

		for(i=0;i<nalpha;i++)
			grad[i] -= 1.0;

	}


	return val;
}


double constraint1(unsigned n, const double *x, double *grad, void *data)
{

	unsigned int nalpha = ((constr_func_data *)data)->num_alpha;
	int *labels = ((constr_func_data *)data)->data_labels;
//	bool b = ((constr_func_data *)data)->bVerbose;

	double ret_val = 0.0;

	unsigned int i;
	for(i=0;i<nalpha;i++)
		ret_val += labels[i]*x[i];

	if(grad)
	{

		for(i=0;i<nalpha;i++)
			grad[i] = labels[i];

		for(i=nalpha; i<n; i++)
			grad[i] = 0;

	}




	return ret_val;
}

double constraint2(unsigned n, const double *x, double *grad, void *data)
{

	unsigned int nalpha = ((constr_func_data *)data)->num_alpha;
	int *labels = ((constr_func_data *)data)->data_labels;

	double ret_val = 0.0;

	unsigned int i;
	for(i=0;i<nalpha;i++)
		ret_val -= labels[i]*x[i];

	if(grad)
	{
		for(i=0;i<nalpha;i++)
			grad[i] = -labels[i];

		for(i=nalpha; i<n; i++)
			grad[i] = 0;
	}

	return ret_val;
}

void hessian(unsigned n, const double *x, const double *v, double *vpre, void *f_data)
{
	double** matkgh = ((obj_func_data*)f_data)->NLModulationKernel;

	for(unsigned int i=0;i<n;i++)
	{
		vpre[i] = 0.0;
		for(unsigned int j=0;j<n;j++)
			vpre[i] += matkgh[i][j]*v[j];
	}
	cout<<"pre: "<<endl;
}

void ASVM_NLopt_Solver::configure(const char* filename)
{



	int dum;
	FILE* file = NULL;
	file = fopen(filename,"r");
	char tmpstr[1025];
	double tmpdbl;
	int tmpint;
	if(file != NULL)
	{
		cout<<"Reading NLOPT configuration from file "<<filename<<"..."<<endl;
		while(!feof(file))
		{
			if(fscanf(file, "%s", tmpstr) >= 0)
			{
				if(!strcmp(tmpstr, "#"))
				{
					break;
				}
				else if(!strcmp(tmpstr, "algo"))
				{

					if((dum=fscanf(file, "%s", tmpstr)) > 0)
					{
						if(!strcmp(tmpstr, "LD_AUGLAG"))
							main_algo = NLOPT_LD_AUGLAG;
						else if(!strcmp(tmpstr, "AUGLAG"))
							main_algo = NLOPT_AUGLAG;
						else if(!strcmp(tmpstr, "LD_AUGLAG_EQ"))
							main_algo = NLOPT_LD_AUGLAG_EQ;
						else if(!strcmp(tmpstr, "AUGLAG_EQ"))
							main_algo = NLOPT_AUGLAG_EQ;

						else if(!strcmp(tmpstr, "LD_SLSQP"))
							main_algo = NLOPT_LD_SLSQP;
						else if(!strcmp(tmpstr, "LD_MMA"))
							main_algo = NLOPT_LD_MMA;

						else if(!strcmp(tmpstr, "LD_LBFGS"))
							main_algo = NLOPT_LD_LBFGS;
						else if(!strcmp(tmpstr, "LD_LBFGS_NOCEDAL"))
							main_algo = NLOPT_LD_LBFGS_NOCEDAL;

						else if(!strcmp(tmpstr, "LD_TNEWTON"))
							main_algo = NLOPT_LD_TNEWTON;
						else if(!strcmp(tmpstr, "LD_TNEWTON_PRECOND"))
							main_algo = NLOPT_LD_TNEWTON_PRECOND;
						else if(!strcmp(tmpstr, "LD_TNEWTON_PRECOND_RESTART"))
							main_algo = NLOPT_LD_TNEWTON_PRECOND_RESTART;
						else if(!strcmp(tmpstr, "LD_TNEWTON_RESTART"))
							main_algo = NLOPT_LD_TNEWTON_RESTART;

						else if(!strcmp(tmpstr, "LD_VAR1"))
							main_algo = NLOPT_LD_VAR1;
						else if(!strcmp(tmpstr, "LD_VAR2"))
							main_algo = NLOPT_LD_VAR1;

						//						else if(!strcmp(tmpstr, "LD_CCSAQ"))
						//							main_algo = NLOPT_LD_CCSAQ;

					}

					fseek(file, SEEK_CUR, -dum);

				}
				else if(!strcmp(tmpstr, "sub_algo"))
				{
					if((dum=fscanf(file, "%s", tmpstr)) > 0)
					{

						if(!strcmp(tmpstr, "LD_SLSQP"))
							sub_algo = NLOPT_LD_SLSQP;
						else if(!strcmp(tmpstr, "LD_MMA"))
							sub_algo = NLOPT_LD_MMA;

						else if(!strcmp(tmpstr, "LD_LBFGS"))
							sub_algo = NLOPT_LD_LBFGS;
						else if(!strcmp(tmpstr, "LD_LBFGS_NOCEDAL"))
							sub_algo = NLOPT_LD_LBFGS_NOCEDAL;

						else if(!strcmp(tmpstr, "LD_TNEWTON"))
							sub_algo = NLOPT_LD_TNEWTON;
						else if(!strcmp(tmpstr, "LD_TNEWTON_PRECOND"))
							sub_algo = NLOPT_LD_TNEWTON_PRECOND;
						else if(!strcmp(tmpstr, "LD_TNEWTON_PRECOND_RESTART"))
							sub_algo = NLOPT_LD_TNEWTON_PRECOND_RESTART;
						else if(!strcmp(tmpstr, "LD_TNEWTON_RESTART"))
							sub_algo = NLOPT_LD_TNEWTON_RESTART;

						else if(!strcmp(tmpstr, "LD_VAR1"))
							sub_algo = NLOPT_LD_VAR1;
						else if(!strcmp(tmpstr, "LD_VAR2"))
							sub_algo = NLOPT_LD_VAR1;

						//						else if(!strcmp(tmpstr, "LD_CCSAQ"))
						//								sub_algo = NLOPT_LD_CCSAQ;

					}

					fseek(file, SEEK_CUR, -dum);
				}
				else if(!strcmp(tmpstr, "C"))
				{
					if((dum=fscanf(file, "%lf", &tmpdbl)) > 0)
					{

						paramC = tmpdbl;

					}

					fseek(file, SEEK_CUR, -dum);
				}
				else if(!strcmp(tmpstr, "ftol_rel"))
				{
					if((dum=fscanf(file, "%lf", &tmpdbl)) > 0)
					{

						fun_rel_tol = tmpdbl;

					}

					fseek(file, SEEK_CUR, -dum);
				}
				else if(!strcmp(tmpstr, "xtol_rel"))
				{
					if((dum=fscanf(file, "%lf", &tmpdbl)) > 0)
					{

						x_rel_tol = tmpdbl;

					}

					fseek(file, SEEK_CUR, -dum);
				}
				else if(!strcmp(tmpstr, "constr_tol"))
				{
					if((dum=fscanf(file, "%lf", &tmpdbl)) > 0)
					{

						constr_tol = tmpdbl;

					}

					fseek(file, SEEK_CUR, -dum);
				}
				else if(!strcmp(tmpstr, "max_eval"))
				{
					if((dum=fscanf(file, "%d", &tmpint)) > 0)
					{

						max_iter = tmpint;

					}

					fseek(file, SEEK_CUR, -dum);
				}
				else if(!strcmp(tmpstr, "max_time_sec"))
				{
					if((dum=fscanf(file, "%d", &tmpint)) > 0)
					{

						max_time_sec = tmpint;

					}

					fseek(file, SEEK_CUR, -dum);
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

					fseek(file, SEEK_CUR, -dum);
				}
			}
		}
	}
	else
		cout<<"Configuration file not found. Using default configuration..."<<endl;

	cout<<"algo\t\t\t"<<main_algo<<endl;
	cout<<"sub_algo\t\t"<<sub_algo<<endl;
	cout<<"Verbose\t\t"<<bVerbose<<endl;
	cout<<"max_time_sec\t\t"<<max_time_sec<<endl;
	cout<<"max_eval\t\t"<<max_iter<<endl;
	cout<<"constr_tol\t\t"<<constr_tol<<endl;
	cout<<"xtol_rel\t\t"<<x_rel_tol<<endl;
	cout<<"C\t\t\t"<<paramC<<endl;
	cout<<"ftol_rel\t\t"<<fun_rel_tol<<endl;

	cout<<"Done."<<endl;

}


int ASVM_NLopt_Solver::learn(asvmdata& input1, unsigned int target_class, asvm* svmobj)
{
	copy_data = new asvmdata(input1);

	copy_data->preprocess(target_class);

	cout<<"Dimension: "<<copy_data->dim<<endl;
	cout<<"Num Alpha: "<<copy_data->num_alpha<<endl;
	cout<<"Num Beta: "<<copy_data->num_beta <<endl;
	cout<<"Target Class: "<<target_class<<endl;

	unsigned int nVariables = copy_data->num_alpha + copy_data->num_beta + copy_data->dim;

	obj_func_data funcdata;
	funcdata.num_alpha = copy_data->num_alpha;
	funcdata.bVerbose = bVerbose;
	funcdata.NLModulationKernel = new double*[nVariables];
	for(unsigned int i=0;i<nVariables;i++)
	{
		funcdata.NLModulationKernel[i] = new double[nVariables];
		for(unsigned int j=0;j<nVariables;j++)
			funcdata.NLModulationKernel[i][j] = copy_data->matkgh[i][j];
	}

	FILE* file = fopen("Matrixauglag.txt", "w");
	for(unsigned int i=0;i<nVariables;i++)
	{
		for(unsigned int j=0;j<nVariables;j++)
			fprintf(file, "%lf  ", funcdata.NLModulationKernel[i][j]);
		fprintf(file, "\n");
	}
	fclose(file);
	constr_func_data  constrdata;
	constrdata.bVerbose = bVerbose;
	constrdata.num_alpha = copy_data->num_alpha;
	constrdata.data_labels = new int[constrdata.num_alpha];
	for(unsigned int i=0;i<constrdata.num_alpha;i++)
		constrdata.data_labels[i] = copy_data->labels[i];

	vector<double> lb, ub;
	lb.resize(nVariables);
	ub.resize(nVariables);

	unsigned int index = 0;
	for(unsigned  int i=0; i<copy_data->num_alpha; i++)	//bounds for alpha varialbes
	{
		lb[index] = 0.0f;
		ub[index++] = 1e10;
	}

	for(unsigned int i=0; i<copy_data->num_beta ; i++)	//bounds for beta varialbes
	{
		lb[index] = 0;
		ub[index++] = paramC;
	}

	for(unsigned int i=0; i<copy_data->dim; i++)	//bounds for gamma varialbes
	{
		lb[index] = -1e10;
		ub[index++] = 1e10;
	}

	nlopt_opt 			sub_opt;
	sub_opt = nlopt_create(sub_algo, nVariables);
	nlopt_set_vector_storage(sub_opt, 100);
	nlopt_set_ftol_rel(sub_opt, fun_rel_tol);
	nlopt_set_xtol_rel(sub_opt, x_rel_tol);

	main_opt = nlopt_create(main_algo, nVariables);

	nlopt_set_lower_bounds(main_opt, lb.data() );
	nlopt_set_upper_bounds(main_opt, ub.data() );
	nlopt_set_ftol_rel(main_opt, fun_rel_tol);
	nlopt_set_xtol_rel(main_opt, x_rel_tol);
	nlopt_set_maxeval(main_opt, max_iter);
	nlopt_set_maxtime(main_opt, max_time_sec);
	nlopt_set_local_optimizer(main_opt, sub_opt);
	//	nlopt_set_precond_min_objective(main_opt2, objective, hessian, &funcdata);
	nlopt_set_min_objective(main_opt, objective, &funcdata);

	nlopt_add_inequality_constraint(main_opt, constraint1, &constrdata, constr_tol);
	nlopt_add_inequality_constraint(main_opt, constraint2, &constrdata, constr_tol);



	vector<double> opt_sol;
	opt_sol.resize(nVariables);

	for(unsigned int i=0; i<nVariables; i++)
		opt_sol[i] = copy_data->initial;


	double minf; /* the minimum objective value, upon return */

	cout<<endl<<"Solving..."<<endl;
#ifdef LINUX
	clock_t begin = clock();
#endif
	int return_code = nlopt_optimize(main_opt, opt_sol.data(), &minf);
#ifdef LINUX
	clock_t ending = clock();
#endif
	cout<<"Done"<<endl;
	cout<<"NLOPT return code = "<<return_code<<endl;
	for(unsigned int i=0;i<opt_sol.size();i++)
	{
		opt_sol[i] = x_curr[i];
		//					cout<<x_curr[i]<<"  ";
	}
	//		cout<<endl;

	svmobj->setSVMFromOptimizedSol(opt_sol.data(), copy_data);

	svmobj->printinfo();
	cout<<endl<<"*******************************************"<<endl;
	cout<<"Alpha error   : "<<svmobj->getclassifiervalue(svmobj->svalpha[0])-svmobj->y[0]<<endl;
	double err=0;
	unsigned int M = copy_data->num_alpha;
	unsigned int P = copy_data->num_beta;
	unsigned int N = copy_data->dim;
	for(unsigned int i=0;i<P;i++)
		err += MYMIN(forward(i+M, opt_sol.data(), copy_data), 0);
	cout<<"Beta error    : "<<err<<endl;
	cout<<"Gamma error   : ";
	for(unsigned int i=0;i<N;i++)
		cout<<-forward(i+M+P, opt_sol.data(), copy_data)<<"  ";
	cout<<endl;
	cout<<"Sum(alpha.*y) : "<<constraint1(nVariables, opt_sol.data(), NULL, &constrdata)<<endl;
#ifdef LINUX
	cout<<"Time elapsed  : "<<((double)(ending - begin))/CLOCKS_PER_SEC<<endl;
#endif
	cout<<"*******************************************"<<endl<<endl;



	if(return_code < 0)
		cout<<"WARNING: Optimization terminated with error!!"<<endl;
	else if(return_code == 1)
		cout<<"Optimization succeeded."<<endl;
	else if(return_code == 3)
		cout<<"Optimization terminated due to ftol_rel OR ftol_abs reached."<<endl;
	else if(return_code == 4)
		cout<<"Optimization terminated due to xtol_rel OR xtol_abs reached."<<endl;
	else if(return_code == 5)
		cout<<"WARNING: Optimization terminated due to max_evals!!"<<endl;
	else if(return_code == 6)
		cout<<"WARNING: Optimization terminated due to max_time!!"<<endl;

	return return_code;

}

double ASVM_NLopt_Solver::forward(int index, double* x_smo, asvmdata* dat)
{
	int M = dat->num_alpha;
	int P = dat->num_beta;
	int N = dat->dim;

	double fval = 0;
	int i;
	for(i=0;i<M;i++)
		if ( x_smo[i] > 0 )
			fval += dat->matkgh[index][i]*x_smo[i];


	for(i=M;i<M+P;i++)
		if ( x_smo[i] > 0 )
			fval += dat->matkgh[index][i]*x_smo[i];


	for(i=M+P;i<M+P+N;i++)
		fval += dat->matkgh[index][i]*x_smo[i];

	return fval;
}

/*
svm ASVM_NLopt_Solver::learn(svmdata& input1, unsigned int target_class, int &return_code)
{
	copy_data = new svmdata(input1);

	copy_data->preprocess(target_class);


	cout<<"\nDimension: "<<copy_data->dim<<endl;
	cout<<"Total points:"<<copy_data->totalpoints()<<endl;
	cout<<"Positive class points: "<<copy_data->tar[copy_data->target_class].classPoints() <<endl;
	cout<<"Target Class: "<<copy_data->target_class<<endl;

	int nVariables = copy_data->totalpoints() + copy_data->tar[copy_data->target_class].classPoints() + copy_data->dim;

	obj_func_data funcdata;
	funcdata.num_var = copy_data->totalpoints()+copy_data->tar[copy_data->target_class].classPoints()+copy_data->dim;
	funcdata.num_points = copy_data->totalpoints();
	funcdata.NLModulationKernel = new double*[nVariables];
	for(int i=0;i<nVariables;i++)
		funcdata.NLModulationKernel[i] = new double[nVariables];

	copy_data->getModulationKernel(funcdata.NLModulationKernel);



	vector<double> lb, ub;
	lb.resize(nVariables);
	ub.resize(nVariables);

	for(int i=0; i<copy_data->totalpoints(); i++)	//bounds for alpha varialbes
	{
		lb[i] = 0.0f;
		ub[i] = 1e10;
	}

	for(int i=copy_data->totalpoints(); i<copy_data->totalpoints()+copy_data->tar[copy_data->target_class].classPoints() ; i++)	//bounds for beta varialbes
	{
		lb[i] = 0;
		ub[i] = paramC;
	}

	for(int i=copy_data->totalpoints()+copy_data->tar[copy_data->target_class].classPoints() ; i<nVariables; i++)	//bounds for gamma varialbes
	{
		lb[i] = -1e10;
		ub[i] = 1e10;
	}


	sub_opt = new nlopt::opt(sub_algo, nVariables);
//	sub_opt->set_vector_storage(10);
	sub_opt->set_ftol_rel(fun_rel_tol);
	sub_opt->set_xtol_rel(x_rel_tol);

	main_opt = new nlopt::opt(main_algo, nVariables);
	main_opt->set_lower_bounds(lb);
	main_opt->set_upper_bounds(ub);
	main_opt->set_ftol_rel(fun_rel_tol);
	main_opt->set_xtol_rel(x_rel_tol);
	main_opt->set_maxeval(max_iter);
	main_opt->set_maxtime(max_time_sec);
	main_opt->set_local_optimizer(*sub_opt);

	main_opt->set_min_objective(objective, &funcdata);

		main_opt->add_inequality_constraint(constraint1, global_data, constr_tol);
	main_opt->add_inequality_constraint(constraint2, global_data, constr_tol);
//	main_opt->add_equality_constraint(constraint1, copy_data, constr_tol);

	vector<double> opt_sol;
	opt_sol.resize(nVariables);


	for(int i=0; i<nVariables; i++)
		opt_sol[i] = copy_data->initial;

	double minf;

	cout<<endl<<"Solving..."<<endl;
	clock_t begin = clock();
	return_code = main_opt->optimize(opt_sol, minf);
	clock_t ending = clock();
	cout<<"Done"<<endl;

	for(unsigned int i=0;i<opt_sol.size();i++)
		opt_sol[i] = x_curr[i];

	double elapsed = ((double)(ending - begin))/CLOCKS_PER_SEC;
	if (return_code < 0) {
		cout<<"nlopt failed with error code: "<<return_code<<" !!"<<endl;
	}

	cout<<endl<<"*******************************************"<<endl;
	cout<<"NLOPT Return Code:\t\t"<<return_code<<endl;
	cout<<"Mininum function value:\t\t"<<minf<<endl;
	cout<<"Time elapsed:\t\t\t"<<elapsed<<endl;
	cout<<"*******************************************"<<endl<<endl;

	svm svmobj;
	svmobj.setSVMFromOptimizedSol(opt_sol.data(), copy_data);

	return svmobj;

}*/
