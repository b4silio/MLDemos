#ifndef _SVMDATA_H_
#define _SVMDATA_H_

#include <stdio.h>
#include <deque>
#include <iostream>
#include "util.h"
using namespace std;

#define VEL_NORM_TOL 1e-4

struct trajectory{
    unsigned int dim;
	unsigned int nPoints;
	double **coords;
	double **vel;
	int *y;
    trajectory(): dim(0), coords(0), vel(0), y(0), nPoints(0){}
    trajectory(int dim, int nPoints) : dim(dim), nPoints(nPoints)
    {
        coords = new double*[nPoints];
        vel = new double*[nPoints];
        for(int i=0; i<nPoints; i++)
        {
            coords[i] = new double[dim];
            vel[i] = new double[dim];
        }
        y = new int[nPoints];
    }
    ~trajectory()
    {
        if(coords)
        {
            for(int i=0; i<nPoints; i++)
                delete [] coords[i];
            delete [] coords;
            coords = 0;
        }
        if(vel)
        {
            for(int i=0; i<nPoints; i++)
                delete [] vel[i];
            delete [] vel;
            vel = 0;
        }
        if(y)
        {
            delete [] y;
            y=0;
        }
    }
    trajectory(const trajectory& other)
    {
        nPoints = other.nPoints;
        dim = other.dim;
        if(other.coords)
        {
            coords = new double*[nPoints];
            for(int i=0; i<nPoints; i++)
            {
                coords[i] = new double[dim];
                memcpy(coords[i], other.coords[i], dim*sizeof(double));
            }
        }
        if(other.vel)
        {
            vel = new double*[nPoints];
            for(int i=0; i<nPoints; i++)
            {
                vel[i] = new double[dim];
                memcpy(vel[i], other.vel[i], dim*sizeof(double));
            }
        }
        if(other.y)
        {
            y = new int[nPoints];
            memcpy(y, other.y, nPoints*sizeof(int));
        }
    }
    trajectory& operator=(const trajectory& other)
    {
        if(&other == this) return *this;
        if(coords)
        {
            for(int i=0; i<nPoints; i++)
                delete [] coords[i];
            delete [] coords;
            coords = 0;
        }
        if(vel)
        {
            for(int i=0; i<nPoints; i++)
                delete [] vel[i];
            delete [] vel;
            vel = 0;
        }
        if(y)
        {
            delete [] y;
            y=0;
        }
        nPoints = other.nPoints;
        dim = other.dim;
        if(other.coords)
        {
            coords = new double*[nPoints];
            for(int i=0; i<nPoints; i++)
            {
                coords[i] = new double[dim];
                memcpy(coords[i], other.coords[i], dim*sizeof(double));
            }
        }
        if(other.vel)
        {
            vel = new double*[nPoints];
            for(int i=0; i<nPoints; i++)
            {
                vel[i] = new double[dim];
                memcpy(vel[i], other.vel[i], dim*sizeof(double));
            }
        }
        if(other.y)
        {
            y = new int[nPoints];
            memcpy(y, other.y, nPoints*sizeof(int));
        }
        return *this;
    }
};

struct target{
    unsigned int dim;
	deque<trajectory> traj;
	double *targ;
    target() : targ(0), dim(0){}
    target(const target& other)
    {
        dim = other.dim;
        traj = other.traj;
        if(other.targ)
        {
            targ = new double[dim];
            memcpy(targ, other.targ, dim*sizeof(double));
        }
        else targ = 0;
    }
    ~target()
    {
        if(targ)
        {
            delete [] targ;
            targ = 0;
        }
    }
    target& operator=(const target& other)
    {
        if(&other == this) return *this;
        if(targ)
        {
            delete [] targ;
            targ = 0;
        }
        dim = other.dim;
        if(other.targ)
        {
            targ = new double[dim];
            memcpy(targ, other.targ, dim*sizeof(double));
        }
        traj = other.traj;
        return *this;
    }

    int classPoints()
	{
		int sum =0;
		for(unsigned int i=0; i<traj.size(); i++)
			sum += traj[i].nPoints;
		return sum;
	}
};

class asvmdata
{
	friend class asvm;
	friend class ASVM_NLopt_Solver;
	friend class ASVM_SMO_Solver;
public:

	bool isOkay;
	unsigned int dim;
	unsigned int num_alpha;
	unsigned int num_beta;
	double lambda;
	int *labels;

	int totalpoints()
	{
		int tp=0;
		for(unsigned int i=0;i<tar.size();i++)
			tp += tar[i].classPoints();

		return tp;
	}

    asvmdata(): isOkay(false), dim(0), num_alpha(0), num_beta(0), lambda(0), labels(0), matkgh(0), target_class(0), initial(0){}
	asvmdata& operator=(const asvmdata& other);
	asvmdata(const asvmdata& other);
    ~asvmdata();
    void printToFile(const char* filename);
	bool loadFromFile(const char* file);
	void setParams(const char *kernel_type="rbf", double kernel_width=0.1, double initial_guess=0);
	void preprocess(unsigned int tclass);
	void addTarget(target &t) { tar.push_back(t);}

public:
	double** matkgh;
	deque<target> tar;
	double initial;
    char type[1024];
	unsigned int target_class;

//	unsigned int* alpha_indices;
//	unsigned int* beta_indices;

private:

	void updateModulationKernel();



};


#endif
