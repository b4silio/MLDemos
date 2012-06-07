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
#ifndef _REINFORCEMENT_H_
#define _REINFORCEMENT_H_

#include <vector>
#include "public.h"
#include "mymaths.h"
#include "reinforcementProblem.h"
#include <QDebug>
#include <QPainter>

class Reinforcement
{
protected:
	u32 dim;
	bool bIterative;
	bool bConverged;
	fvec maximum;
	std::vector< fvec > visited;
	std::vector< fvec > history;
	std::vector<double> historyValue;
	double maximumValue;
	int evaluations;
    fvec directions;

public:
    ReinforcementProblem *problem;
    int age, maxAge;
    double stopValue;

    Reinforcement() : evaluations(0), stopValue(.99), maxAge(200), age(0), dim(2),
        bIterative(false) , bConverged(true), maximumValue(-FLT_MAX)
    { maximum.resize(2);}
    virtual ~Reinforcement(){}
    bool hasConverged(){return bConverged;}
    void SetConverged(bool converged){bConverged = converged;}
    std::vector<fvec> &History(){return history;}
    std::vector<double> &HistoryValue(){return historyValue;}
    fvec &Maximum(){return maximum;}
    double MaximumValue(){return problem?problem->GetValue(maximum):0;}
    std::vector<fvec> &Visited(){return visited;}
    int &Evaluations(){return evaluations;}

    // use the policy to decide which action to take, and perform the action
    virtual void Draw(QPainter &painter){}
    virtual void Initialize(ReinforcementProblem *problem){}
    virtual fvec Update(){ return fvec(); }
    virtual const char *GetInfoString(){return NULL;}
};

#endif // _REINFORCEMENT_H_
