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
#ifndef _DYNAMICAL_H_
#define _DYNAMICAL_H_

#include "mymaths.h"
#include "obstacles.h"
#include <vector>

extern "C" enum {DYN_SVR, DYN_RVM, DYN_GMR, DYN_GPR, DYN_KNN, DYN_MLP, DYN_LINEAR, DYN_LWPR, DYN_KRLS, DYN_SEDS, DYN_NONE} dynamicalType;

class Dynamical
{
protected:
	std::vector< std::vector<fvec> > trajectories;
	ivec classes;
	ivec labels;
	u32 dim;

public:
	std::vector<fvec> crossval;
	fvec fmeasures;
	fvec trainErrors, testErrors;
	int type;
	float dT;
	u32 count;
	ObstacleAvoidance *avoid;

	Dynamical(): type(DYN_NONE), count(100), dT(0.02f), avoid(0){}
    virtual ~Dynamical(){if(avoid) delete avoid;}
    std::vector< std::vector<fvec> > GetTrajectories(){return trajectories;}
    int Dim(){return dim;}

    virtual void Train(std::vector< std::vector<fvec> > trajectories, ivec labels){}
    virtual std::vector<fvec> Test( const fvec &sample, const int count){ return std::vector<fvec>(); }
    virtual fvec Test( const fvec &sample){ return fvec(); }
    virtual fVec Test(const fVec &sample){ return fVec(Test((fvec)sample)); }
    virtual const char *GetInfoString(){return NULL;}
    virtual void SaveModel(std::string filename){}
    virtual bool LoadModel(std::string filename){return true;}
};

#endif // _DYNAMICAL_H_
