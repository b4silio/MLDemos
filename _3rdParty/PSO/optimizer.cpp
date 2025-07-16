
/*
//---------------- Author: Francesco Castellini* and Annalisa Riccardi** ---------------//
//------- Affiliation: -----------------------------------------------------------------//
//--------------* Dipartimento di Ingegneria Aerospaziale, Politecnico di Milano -------//
//--------------**Centrum for Industrial Mathematics, University of Bremen -------------//
//--------E-mail: castellfr@gmail.com, nina1983@gmail.com ------------------------------//

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>. 
*/

#include "optimizer.h"

#include <unistd.h>
//#include <Windows.h>
//#include <Winbase.h>

#ifdef _DEBUG
#ifdef WIN32
int omp_get_thread_num() { return 0; };
int omp_get_num_threads() { return 1; };
#endif
#endif

using namespace Eigen;
using namespace std;

Optimizer::Optimizer(int NVAR, int NCONS, Eigen::VectorXd LOWERBOUND, Eigen::VectorXd UPPERBOUND, Eigen::VectorXd GLOWERBOUND, Eigen::VectorXd GUPPERBOUND): m_bestFeasible(NVAR){
	m_name = "O";
	modelEvaluationsCount = 0;
	dim = NVAR;
	constraintCount = NCONS;
	m_LOWERBOUND = LOWERBOUND;
	m_UPPERBOUND = UPPERBOUND;
	m_GLOWERBOUND = GLOWERBOUND;
	m_GUPPERBOUND = GUPPERBOUND;
	opt_print_level = 0;
	bFeasibleOnly = false;
	m_nIterReverseComm = 0;
	bUseMultiThread = false;
	multiThreadType = 0;
	initType = 0;
	m_indexInit = 0;
    m_filenameInit = 0;
	data = 0;
}

Optimizer::~Optimizer() {
	// TODO Auto-generated destructor stub
}

void Optimizer::setModel(Eigen::VectorXd (*model)(Eigen::VectorXd& x)){
	m_model = model;
}

void Optimizer::setProblemName(string name){
	m_name = name;
}

void Optimizer::setPrintLevel(int i){
	opt_print_level = i;
}

void Optimizer::setFeasibleOnly(){
	bFeasibleOnly = true;
}

double Optimizer::constrViolation(double constrValue, int constrIndex){
	double violation = 0;

	if(constrValue>m_GUPPERBOUND(constrIndex))
		violation += constrValue - m_GUPPERBOUND(constrIndex);
	if(constrValue<m_GLOWERBOUND(constrIndex))
		violation += m_GLOWERBOUND(constrIndex) - constrValue;

	return violation;
}

//This function loads from file to matrix the optimization solutions to Eigen matrix of nsol x nvar double variables in [0;1])
//the procedure reads the number of lines in the file nsol (number of solutions loaded)
// but NOTE: no check is performed on the input file except for all variables being in [0;1] --> all lines MUST have 
//				the same number of columns, equal to nvar (one column per each variable)
//INPUTS:
//	nvar: number of variables in each solution (number of optimization variables)
//	name: string with the name of the input file containing the solutions matrix, including extension. 
//			NOTE: The file will be searched in data/optimization folder and all its subfolders
//OUTPUTS:
//	sol: Eigen matrix of nsol x nvar elements containing the requested solutions (nsol rows for the nsol solutions in the file)
//	flagread: true if the file has been found and the loaded solution is correct (i.e. all variables in [0;1])
bool Optimizer::loadFirstGuessSolutionFromFile(int nvar, string name, Eigen::MatrixXd& sol)
{
	//Look for input file containing first guess solution(s) in all optimization subfolders: 
	bool flagread=false; string filename; ifstream filestream;
	if(!flagread) { filename="optimization/"+name; 
		filestream.open(filename.data(), ios::in); if(filestream.good()) flagread=true; else filestream.clear(); }
	if(!flagread) { filename="optimization/DGMOPSO/"+name;
		filestream.open(filename.data(), ios::in); if(filestream.good()) flagread=true; else filestream.clear(); }
	if(!flagread) { filename="optimization/HybridGO/"+name;
		filestream.open(filename.data(), ios::in); if(filestream.good()) flagread=true; else filestream.clear(); }
	if(!flagread) { filename="optimization/MADS/"+name;
		filestream.open(filename.data(), ios::in); if(filestream.good()) flagread=true; else filestream.clear(); }
	if(!flagread) { filename="optimization/MOACOr/"+name;
		filestream.open(filename.data(), ios::in); if(filestream.good()) flagread=true; else filestream.clear(); }
	if(!flagread) { filename="optimization/NSGA2/"+name;
		filestream.open(filename.data(), ios::in); if(filestream.good()) flagread=true; else filestream.clear(); }
	if(!flagread) { filename="optimization/PSO/"+name;
		filestream.open(filename.data(), ios::in); if(filestream.good()) flagread=true; else filestream.clear(); }
	if(!flagread) { filename="optimization/WORHP/"+name;
		filestream.open(filename.data(), ios::in); if(filestream.good()) flagread=true; else filestream.clear(); }
	if(!flagread)
		cout << "Error opening file " << name << ", no such file found in any of the optimization subfolders" << endl;
	else		//if the given file has been found, read first guess solution data
	{
		int nsol = std::count(istreambuf_iterator<char>(filestream), istreambuf_iterator<char>(),'\n');
		filestream.close();
		filestream.open(filename.data(), ios::in);
		sol=Eigen::MatrixXd(nsol,nvar);
		//Read sol-th solution from solutions matrix in the file:
		for(int j=0;j<nsol;j++)
		{
			if(!flagread)
				break;
			for(int k=0;k<nvar;k++)
				if(filestream.eof())
				{
					flagread=false;
					cout << "Error loading from file variable "<<k<<" of solution "<<j<<": end of file reached" << endl;
					break;
				}
				else
				{
					filestream >> sol(j,k);				//load k-th variable of j-th solution
					if(sol(j,k)<0 || sol(j,k)>1)		//check if k-th variable of j-th solution is within [0;1]
					{
						flagread=false;
						cout << "Error loading from file variable "<<k<<" of solution "<<j<<": variable value "<<sol(j,k)<<" is not within [0;1]" << endl;
						break;
					}
				}
		}
	}
	return flagread;
}

void Optimizer::SetData(float *data, int w, int h)
{
	this->data = data;
	dataW = w;
	dataH = h;
    evaluationHistory.clear();
}

Eigen::VectorXd Optimizer::EvaluateModel(Eigen::VectorXd& x)
{
	if(dim == 2 && data)
	{
		Eigen::VectorXd y(1);
		int i = (x[0] - m_LOWERBOUND(0)) / (m_UPPERBOUND(0) - m_LOWERBOUND(0)) * dataW;
		int j = (x[1] - m_LOWERBOUND(1)) / (m_UPPERBOUND(1) - m_LOWERBOUND(1)) * dataH;
		i = max(0, min(dataW-1, i));
		j = max(0, min(dataH-1, j));
        y(0) = 1.f - data[j*dataW + i];
        evaluationHistory.push_back(make_pair(i,j));
		return y;
	}
	else return m_model(x);
}

// Function used by PSO and DGMOPSO to perform the model evaluation for all swarm particles in a given iteration, also 
// implementing the parallel computing specifications and printing outputs to video if requested by the opt_print_level
//INPUTS:
//  * iternumber: number of current iteration, just for screen printing
//  * useOpenMP: boolean for using OpenMP parallel implementation for shared memory machines 
//			--> NOTE: OMP_NUM_THREADS environment variables MUST be set to the number of processors you want to use!
//  * useMPI:    boolean for using MPI parallel implementation for distributed memory machines 
//  * useOpenCL: boolean for using OpenCL GPU computing implementation for graphic cards (NVIDIA and ATI supported)
//  * swarm: matrix of (nparticles x nvars) elements with the current position of the swarm, to be evalauted
//OUTPUTS:
//  * Jswarm: matrix of (nparticles x nobj) elements with the computed objective function values for the current swarm positions
//  * Cswarm: matrix of (nparticles x ncstrs) elements with the computed constraints values for the current swarm positions
//	* quitOptimization: false --> continue optimization with following iterations; true --> quit optimization upon user's request
void Optimizer::evaluateParticles(const int iternumber, const bool useOpenMP, int swarmsize, double **swarm, double **Jswarm, double **Cswarm)
{
	if(!swarm || !Jswarm || !Cswarm) return;

	Eigen::VectorXd var(dim), objconst(objectiveCount+constraintCount);
	int chunk=1; double penalty=0;

//#pragma omp parallel default(shared) private(var,objconst,penalty) if(useOpenMP)
	{
		var.resize(dim); objconst.resize(1+constraintCount);
		int nthreads = 1;
		//int nthreads=omp_get_num_threads();
//#pragma omp for schedule(dynamic,chunk)
		for(int i=0;i<swarmsize;i++)
		{
			for(int j=0;j<dim;j++) var(j)=swarm[i][j];									//copy current swarm to temp. vector
			if(opt_print_level>1)									//print particle output if requested
			{
				if(iternumber==0)
				{
					if(nthreads==1) cout << "Initialization, particle " << i << endl;
//					else cout << "Initialization, particle " << i << ", processor " << omp_get_thread_num()+1 << " of " << nthreads << endl;
				}
				else
				{
					if(nthreads==1) cout << "Iteration " << iternumber << ", particle " << i << endl;
//					else cout << "Iteration " << iternumber << ", particle " << i << ", processor " << omp_get_thread_num()+1 << " of " << nthreads << endl;
				}
			}
			//cout << "Thread " << omp_get_thread_num() << ", PsoBeforeEval, mem:" << (int)(this) << endl;
			objconst = EvaluateModel(var); modelEvaluationsCount++;			//evaluate model
			//cout << "Thread " << omp_get_thread_num() << ", PsoAfterEval, mem:" << (int)(this) << endl;
			penalty=0;
			if(opt_print_level>2)
			{
				for(int j=0;j<objectiveCount;j++)
					cout << " " << objconst[j];
				cout << "  | ";
			}
			for(int j=0;j<constraintCount;j++)								//copy constraints from temp. vector
			{
				Cswarm[i][j]=constrViolation(objconst[j+objectiveCount],j);	//the function returns the constraint violation from the constraint value, according to the defined boundaries
				if(opt_print_level>2) cout << " " << Cswarm[i][j];
				penalty+=Cswarm[i][j];								//adding current constraint violation to penalty
			}
			for(int j=0;j<objectiveCount;j++)
				Jswarm[i][j]=objconst[j]+penalty*hugeCstrVio();		//adding a huge value if the constraint is violated
			if(opt_print_level>2)
			{
				cout << "  |  " << penalty << "  | ";
				for(int j=0;j<objectiveCount;j++)
					cout << " " << Jswarm[i][j];
				cout << endl;
			}
		}
	}	//CLOSE PARALLEL cycle on solutions to be evaluated at the current iteration
}


// Function used by PSO and DGMOPSO to print to file the optimization variables of the current archive of 1 best 
//  solution for single-objective PSO or of N non dominated solutions for multi-objective DGMOPSO
//INPUTS:
//  * fp: ofstream for the file to write on (MUST HAVE BEEN OPENED AND MUST BE CLOSED OUTSIDE THIS FUNCTION)
//  * iternumber: number of current iteration, to be printed in the first column
//  * size: current size of the archive (1 for single-objective, variable or fixed to maxsize for multi-objective)
//  * archive: matrix of (nparticles x nvars) elements with the optimization variables of the current solutions 
//			contained in the archive (DIMENSIONS AND MEMORY ALLOCATION IS NOT CHECKED HERE, MUST BE TAKEN CARE OF OUTSIDE!)
//OUTPUTS: None, the variables are printed on a row of the file fp

void Optimizer::printArchiveVars(ofstream& fp, const int& iternumber, const int& size, double **archive)
{
	for(int i=0;i<size;i++)
	{
		fp << iternumber << " ";
		for(int j=0;j<dim;j++)
			fp << archive[i][j] << " ";
		fp << endl;
	}
}

// Function used by PSO and DGMOPSO to print to file the optimization variables of the current archive of 1 best 
//  solution for single-objective PSO or of N non dominated solutions for multi-objective DGMOPSO
//INPUTS:
//  * fp: ofstream for the file to write on (MUST HAVE BEEN OPENED AND MUST BE CLOSED OUTSIDE THIS FUNCTION)
//  * iternumber: number of current iteration, to be printed in the first column
//  * size: current size of the archive (1 for single-objective, variable or fixed to maxsize for multi-objective)
//  * Jarchive: matrix of (nparticles x nobj) elements with the optimization objectives of the current solutions 
//			contained in the archive (DIMENSIONS AND MEMORY ALLOCATION IS NOT CHECKED HERE, MUST BE TAKEN CARE OF OUTSIDE!)
//  * Carchive: matrix of (nparticles x ncstr) elements with the optimization constraints of the current solutions 
//			contained in the archive (DIMENSIONS AND MEMORY ALLOCATION IS NOT CHECKED HERE, MUST BE TAKEN CARE OF OUTSIDE!)
//OUTPUTS: None, the variables are printed on a row of the file fp

void Optimizer::printArchiveObjCstr(ofstream& fp, const int& iternumber, const int& size, double **Jarchive, double **Carchive)
{
	for(int i=0;i<size;i++)
	{
		fp << iternumber << " ";
		for(int j=0;j<objectiveCount;j++)
			fp << Jarchive[i][j] << " ";
		for(int j=0;j<constraintCount;j++)
			fp << Carchive[i][j] << " ";
		fp << endl;
	}
}

