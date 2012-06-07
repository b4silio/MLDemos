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
#ifndef _REINFORCEMENT_POWER_H_
#define _REINFORCEMENT_POWER_H_

#include <vector>
#include "reinforcement.h"

class ReinforcementPower: public Reinforcement
{
private:
	int age;
	int maxAge;
	float variance;
	int k;
	std::vector< std::pair<double, std::pair<fvec, fvec> > > best;
	fvec lastSigma;
	bool bAdaptive;
    bool bSingleDim;
public:
    ReinforcementPower();
    ~ReinforcementPower();

	void SetParams(int k, float variance=0.2, bool bAdaptive=false);

	void Draw(QPainter &painter);
    void Initialize(ReinforcementProblem *problem);
    fvec Update();
    const char *GetInfoString();
};

#endif // _REINFORCEMENT_POWER_H_
