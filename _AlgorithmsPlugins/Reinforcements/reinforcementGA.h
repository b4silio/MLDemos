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
#ifndef _REINFORCEMENT_GA_H_
#define _REINFORCEMENT_GA_H_

#include <vector>
#include "reinforcement.h"
#include "gaTrainer.h"

class ReinforcementGA : public Reinforcement
{
private:
	double mutation, cross, survival;
	int population;
	GATrain *trainer;
public:
    ReinforcementGA();
    ~ReinforcementGA();

	void SetParams(double mutation, double cross, double survival, int population);

    void Draw(QPainter &painter);
    void Initialize(ReinforcementProblem *problem);
    fvec Update();
    const char *GetInfoString();
};

#endif // _REINFORCEMENT_GA_H_
