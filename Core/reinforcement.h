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
#include <QDebug>
#include <QPainter>

class Reinforcement
{
protected:
	u32 dim;
	int w, h;
	bool bIterative;
	bool bConverged;
	fvec maximum;
	std::vector< fvec > visited;
	std::vector< fvec > history;
	std::vector<double> historyValue;
	double maximumValue;
	float *data;
	int evaluations;

public:
    int age, maxAge, simulationSteps, problemType, displayIterationsCount;
    double stopValue;

    Reinforcement() : evaluations(0), stopValue(.99), maxAge(200), age(0), dim(2),
        bIterative(false) , bConverged(true), data(NULL), w(1), h(1), maximumValue(-FLT_MAX),
        problemType(0), simulationSteps(100), displayIterationsCount(10)
    { maximum.resize(2);}
    virtual ~Reinforcement(){if(data) delete [] data;}
    void Maximize(float *dataMap, int w, int h) {Train(dataMap,fVec(w,h));}
    bool hasConverged(){return bConverged;}
    void SetConverged(bool converged){bConverged = converged;}
    std::vector<fvec> &History(){return history;}
    std::vector<double> &HistoryValue(){return historyValue;}
    fvec &Maximum(){return maximum;}
    double MaximumValue(){return GetValue(maximum);}
    std::vector<fvec> &Visited(){return visited;}
    int &Evaluations(){return evaluations;}
    static float GetValue(fvec sample, float *data, int w, int h)
    {
        int xIndex = max(0, min(w-1, (int)(sample[0]*w)));
        int yIndex = max(0, min(h-1, (int)(sample[1]*h)));
        int index = yIndex*w + xIndex;
        return data[index];
    }
    float GetValue(fvec sample)
	{
		int xIndex = max(0, min(w-1, (int)(sample[0]*w)));
		int yIndex = max(0, min(h-1, (int)(sample[1]*h)));
		int index = yIndex*w + xIndex;
		return data[index];
	}
    float GetSimulationValue(fvec sample)
    {
        float reward = 0;
        fvec direction(dim);
        switch(problemType)
        {
        case 0:
        {
            reward += GetValue(sample);
            FOR(i, simulationSteps)
            {
                fvec newSample = PerformAction(sample);
                float currentReward = GetValue(newSample);
                fvec newDirection = newSample - sample;
                if(i && direction*newDirection < 0.f) currentReward = 0;
                reward += currentReward;
                sample = newSample;
                direction = newDirection;
            }
        }
            break;
        }
        return reward;
    }
    // use the policy to decide which action to take, and perform the action
    virtual fvec PerformAction(fvec sample){return sample;}
    virtual float GetReward(){fvec sample(dim); FOR(d, dim) sample[d]=drand48(); return GetSimulationValue(sample);}

    virtual void Draw(QPainter &painter){}
    virtual void Train(float *dataMap, fVec size, fvec startingPoint=fvec()){}
    virtual fvec Test( const fvec &sample){ return fvec(); }
    virtual fvec Test(const fVec &sample){ return Test((fvec)sample); }
    virtual const char *GetInfoString(){return NULL;}
};

#endif // _REINFORCEMENT_H_
