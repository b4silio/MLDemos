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
#ifndef _MAXIMIZE_SWARMS_H_
#define _MAXIMIZE_SWARMS_H_

#include <vector>
#include "maximize.h"
#include "PSO/optimizer.h"
#include "PSO/pso.h"

class MaximizeSwarm : public Maximizer
{
private:
	PSO *pso;
	int particleCount;
	float mutation;
	bool inertia;
	float inertiaInit;
	float inertiaFinal;
	float particleConfidence;
	float swarmConfidence;
public:
	MaximizeSwarm();
	~MaximizeSwarm();

	void SetParams(int particleCount, float mutation, bool inertia, float inertiaInit, float inertiaFinal, float particleConfidence, float swarmConfidence);
	void Draw(QPainter &painter);
	void Train(float *dataMap, fVec size, fvec startingPoint=fvec());
	fvec Test( const fvec &sample);
	fvec Test(const fVec &sample);
    const char *GetInfoString();
};

#endif // _MAXIMIZE_SWARMS_H_
