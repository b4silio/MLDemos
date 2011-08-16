/*********************************************************************
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
#ifndef _DYNAMICAL_HMM_H_
#define _DYNAMICAL_HMM_H_

// LAMP_HMM code from
// http://www.cfar.umd.edu/~daniel/Site_2/Code.html
#include <vector>
#include "dynamical.h"
#include <LAMP_HMM/utils.h>
#include <LAMP_HMM/obs.h>
#include <LAMP_HMM/obsSeq.h>
#include <LAMP_HMM/obsProb.h>
#include <LAMP_HMM/discreteObsProb.h>
#include <LAMP_HMM/gaussianObsProb.h>
#include <LAMP_HMM/vectorObsProb.h>
#include <LAMP_HMM/stateTrans.h>
#include <LAMP_HMM/plainStateTrans.h>
#include <LAMP_HMM/gammaProb.h>
#include <LAMP_HMM/explicitDurationTrans.h>
#include <LAMP_HMM/initStateProb.h>
#include <LAMP_HMM/hmm.h>

class DynamicalHMM : public Dynamical
{
public:
	int mixtures, states, trainType, obsType;
	CStateTrans *a;
	CObsProb *b;
	CInitStateProb *pi;
	CHMM *learnedHMM;
	std::vector< std::vector<fvec> > trajectories;
public:
	DynamicalHMM() : mixtures(1), states(1), trainType(1), obsType(0), a(0), b(0), pi(0), learnedHMM(0){};
	~DynamicalHMM();
    void Train(std::vector< std::vector<fvec> > trajectories, ivec labels);
    std::vector<fvec> Test( const fvec &sample, const int count);
    fvec Test( const fvec &sample);
    fVec Test( const fVec &sample);
    char *GetInfoString();
	bool IsOrphanedState(int state);

	void SetParams(int mixtures, int states, int trainType, int obsType);
};

#endif // _DYNAMICAL_HMM_H_
