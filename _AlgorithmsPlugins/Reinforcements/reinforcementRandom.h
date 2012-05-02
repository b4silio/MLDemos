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
#ifndef _REINFORCEMENT_RANDOM_H_
#define _REINFORCEMENT_RANDOM_H_

#include <vector>
#include "reinforcement.h"

class ReinforcementRandom : public Reinforcement
{
private:
public:
    float variance;
    int policyType;
    fvec directions;
    int gridSize;
    bool bSingleDim;
    int quantizeType;
public:
    ReinforcementRandom();
    ~ReinforcementRandom();

    void SetParams(float variance=0.1f, int policyType=0, int gridSize=4, bool bSingleDim=false, int quantizeType=2);

    fvec NextStep(fvec sample, fvec directions);
    fvec PerformAction(fvec sample);
    float GetReward(fvec directions);
    float GetReward();

    void Draw(QPainter &painter);
	void Train(float *dataMap, fVec size, fvec startingPoint=fvec());
	fvec Test( const fvec &sample);
	fvec Test(const fVec &sample);
    const char *GetInfoString();
};

#endif // _REINFORCEMENT_RANDOM_H_
