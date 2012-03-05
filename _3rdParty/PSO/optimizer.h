
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

#ifndef OPTIMIZER_H_
#define OPTIMIZER_H_

#include <time.h>
#include <cmath>
#include <iostream>
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>

#include <Eigen/Core>

#include "memoryAllocation.h"

#ifdef __linux__
#include <omp.h>							//omp.h is supported by LINUX (GNU gcc 4.3.2 and later versions)
#endif
#ifdef WIN32
#ifndef _DEBUG
#include <omp.h>							//omp.h is supported in Windows by MSVC only in debug mode
#endif
#endif

#define ROUND(dbl) dbl >= 0.0 ? (int)(dbl + 0.5) : ((dbl - (double)(int)dbl) == -0.5 ? (int)dbl : (int)(dbl - 0.5))

using namespace Eigen;
using namespace std;

static const double hugeCstrVio() {return 1e10; }	//Constraints violation penalty

class Optimizer {
public:
	Optimizer(int NVAR, int NCONS, Eigen::VectorXd LOWERBOUND, Eigen::VectorXd UPPERBOUND, Eigen::VectorXd GLOWERBOUND = Eigen::VectorXd::Zero(1), Eigen::VectorXd GUPPERBOUND = Eigen::VectorXd::Zero(1));
	virtual ~Optimizer();

	friend void init();
	friend void optimizeOnce();
	friend void collect();

	double constrViolation(double constrValue, int constraIndex);
	void evaluateParticles(const int iternumber, const bool useOpenMP, int swarmsize, double **swarm, double **Jswarm, double **Cswarm);
	void printArchiveVars(ofstream& fp, const int& iternumber, const int& size, double **archive);
	void printArchiveObjCstr(ofstream& fp, const int& iternumber, const int& size, double **Jarchive, double **Carchive);

	// model: function that returns the objective values and the constraints values (or violation depends on the algorithm used)
	void setModel(Eigen::VectorXd (*model)(Eigen::VectorXd& x));
	void setProblemName( string name);
	void setPrintLevel(int i);
	void setInitialization(int init, char* file_name = 0, int index = 0){initType = init; m_filenameInit = file_name; m_indexInit = index; }
	void setFeasibleOnly();	
	void setBestFeasibleSolution(Eigen::VectorXd sol){m_bestFeasible = sol;}
	void setNIterReverseComm(int nIterReverseComm){ m_nIterReverseComm=nIterReverseComm; }
	void setMultiThread(bool multiThread){ bUseMultiThread=multiThread; }
	void setMultiThreadType(int type){ multiThreadType=type; }

	Eigen::VectorXd getGLBound(){return m_GLOWERBOUND;}
	Eigen::VectorXd getGUBound(){return m_GUPPERBOUND;}
	Eigen::VectorXd getLBound(){return m_LOWERBOUND;}
	Eigen::VectorXd getUBound(){return m_UPPERBOUND;}
	int getInitialization(){return initType;}
	char* getInitializationFile(){return m_filenameInit;}
	int getInitializationIndexInFile(){return m_indexInit;}
	Eigen::VectorXd getBestFeasible(){return m_bestFeasible;}
	bool isFeasibleOnly(){return bFeasibleOnly;}
	int getPrintLevel(){return opt_print_level;}

	static bool loadFirstGuessSolutionFromFile(int,string,Eigen::MatrixXd&);

    std::vector<std::pair<int,int> > evaluationHistory;
    int modelEvaluationsCount;
    Eigen::VectorXd (*m_model)(Eigen::VectorXd& x);
	Eigen::VectorXd EvaluateModel(Eigen::VectorXd& x);
	void SetData(float *data, int w, int h);

protected:
	string m_name;
	int dim, constraintCount, objectiveCount;	//m_NOBJ is used only for multi-objective algorithms
    Eigen::VectorXd m_LOWERBOUND, m_UPPERBOUND;
	Eigen::VectorXd m_GLOWERBOUND, m_GUPPERBOUND;
	Eigen::VectorXd m_bestFeasible; // best feasible solution if m_feasibleOnly option is on
	float *data;
	int dataW, dataH;

	int opt_print_level;		//0: no info messages printed on console, the final results are saved in the files; 1: optimization results printed on console and saved to files; 2: iteration results printed on console and to files; 3: iteration and initialization results print
	int initType;		// = 0: random initialization, = 1: initialization from a given solution, = 2: initialization from a given file
	char* m_filenameInit;
	int m_indexInit;
	int m_nIterReverseComm;
	bool bFeasibleOnly;
	bool bUseMultiThread;			//no: serial execution, yes: parallel execution
	int multiThreadType;		//only for multiThread=yes --> 1: OpenMP, 2: MPI, 3: OpenCL
};

#endif /* OPTIMIZER_H_ */
