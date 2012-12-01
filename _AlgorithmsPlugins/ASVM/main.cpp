
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <iostream>

#include "ASVMLearning.h"
//#include "sys/time.h"

#include "signal.h"
using namespace std;
ASVM_SMO_Solver smo_solver;
#ifdef USE_NLOPT
ASVM_NLopt_Solver nlopt_solver;
#endif
//ASVM_ALGLIB_Solver solver;

//void exit_cleanly(int sig)
//{
//	cout<<"Force Exiting at current solution..."<<endl;
//	solver.force_stop();
//}

int main(int argc, char **argv)
{
//	double f=0;
//#pragma omp parallel for
//	for(int i=0;i<10;i++)
//	{
//		f+=i;
//		printf("F = %lf\n", f);
//	}
//
//	cout<<f<<endl;
//	return 0;
//	(void) signal(SIGINT, exit_cleanly);

	char datafile[1025]; strcpy(datafile, "data/data.txt");
	char outfile[1025]; strcpy(outfile, "models/learned_svm.txt");
	char paramfile[1025]; strcpy(paramfile, "./optparams.ini");
	char solvername[1025]; strcpy(solvername, "smo");
	double kernel_width = 1.0;
	double initial = 1e-8;
	int target_class = 0;

	if(argc == 1)
	{
		cout<<"==================== Usage ======================"<<endl;
		cout<<"./bin/train [OPTION1 VALUE1] [OPTION2 VALUE2] ..."<<endl<<endl;
		cout<<"OPTIONS  \t\t"<<"VALUES"<<endl;
		cout<<"--solver \t\t"<<"Which solver to use (smo/nlopt) [default: "<<solvername<<"]"<<endl;
		cout<<"--data   \t\t"<<"Data file name with extension [default: "<<datafile<<"]"<<endl;
		cout<<"--output \t\t"<<"Output file name with extension [default: "<<outfile<<"]"<<endl;
		cout<<"--param  \t\t"<<"Optimization parameter file [default: "<<paramfile<<"]"<<endl;
		cout<<"--sigma  \t\t"<<"Kernel Width [default: "<<kernel_width<<"]"<<endl;
		cout<<"--tclass \t\t"<<"Zero based target class [default: "<<target_class<<"]"<<endl;
		cout<<"--xinit \t\t"<<"Initial guess for optimization [default: 0]"<<endl;
		cout<<"        \t\t\to "<<"Positive number (p): All dimenstions are set equal to p"<<endl;
		cout<<"        \t\t\to "<<"-1: SVM Classifier solution"<<endl;
		cout<<"================================================="<<endl;
		return -6;
	}
	for(int i=0;i<argc;i++)
	{
		if(!strcmp(argv[i], "--data"))
			strcpy(datafile, argv[i+1]);
		else if(!strcmp(argv[i], "--output"))
			strcpy(outfile, argv[i+1]);
		else if(!strcmp(argv[i], "--param"))
			strcpy(paramfile, argv[i+1]);
		else if(!strcmp(argv[i], "--sigma"))
			kernel_width = atof(argv[i+1]);
		else if(!strcmp(argv[i], "--xinit"))
			initial = atof(argv[i+1]);
		else if(!strcmp(argv[i], "--tclass"))
			target_class = atoi(argv[i+1]);
		else if(!strcmp(argv[i], "--solver"))
			strcpy(solvername, argv[i+1]);
	}

	if(initial < 1e-8)
	{
		initial = 1e-8;
		cout<<"Overriding the initial value to 1e-8"<<endl;
	}
	asvmdata input;

	if(!input.loadFromFile(datafile))
		return -10;

	input.setParams("rbf", kernel_width, initial);

	int code=0;

	asvm svmobj;
	if(!strcmp(solvername, "nlopt"))
	{
#ifdef USE_NLOPT
		nlopt_solver.configure(paramfile);
		code = nlopt_solver.learn(input, target_class, &svmobj);
#else
		cout<<"NLOPT is not compiled!!"<<endl;
#endif
	}
	else if(!strcmp(solvername, "smo"))
	{
		smo_solver.configure(paramfile);
		code = smo_solver.learn(input, target_class, &svmobj);
	}


//		svmobj.printinfo();

		svmobj.saveToFile(outfile);

	return code;

}



