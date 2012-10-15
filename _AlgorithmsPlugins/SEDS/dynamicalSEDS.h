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
#ifndef _DYNAMICAL_SEDS_H_
#define _DYNAMICAL_SEDS_H_

#include <vector>
#include "dynamical.h"
#include "SEDS.h"
#include "fgmm/fgmm++.hpp"

class DynamicalSEDS : public Dynamical
{
public:
    static Gmm *globalGMM;
    Gmm *gmm;
    SEDS *seds;
	float resizeFactor;
private:
    u32 nbClusters;
    float penalty;
    bool bPrior;
    bool bMu;
    bool bSigma;
    int objectiveType;
    int maxIteration;
    int maxMinorIteration;
    int constraintCriterion;
    int optimizationType;
    float *data;
public:
#ifdef USEQT
    QLabel *displayLabel;
#endif
    fvec endpoint;
    fVec endpointFast;
    DynamicalSEDS();
    ~DynamicalSEDS();
    void Train(std::vector< std::vector<fvec> > trajectories, ivec labels);
    std::vector<fvec> Test( const fvec &sample, const int count);
    fvec Test( const fvec &sample);
    fVec Test( const fVec &sample);
    const char *GetInfoString();
    void SaveModel(string filename);
    bool LoadModel(string filename);

    void SetParams(int clusters, bool bPrior, bool bMu, bool bSigma, int objectiveType,
                   int maxIteration, int constraintCriterion, int optimizationType=0);
};

#endif // _DYNAMICAL_SEDS_H_
