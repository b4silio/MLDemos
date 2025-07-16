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
#ifndef _DYNAMICAL_ASVM_H_
#define _DYNAMICAL_ASVM_H_

#include <vector>
#include "dynamical.h"
#include "ASVMLearning.h"
#include "fgmm/fgmm++.hpp"

class DynamicalASVM : public Dynamical
{
public:
    std::vector<asvm> asvms;
    std::vector<Gmm*> gmms;
    float resizeFactor;
    std::map<int,int> classMap, inverseMap;
    std::map<int,int> classes;
    u32 nbClusters;
    u32 classCount;
private:
    double kernelWidth;
    double Cparam;
    double alphaTol;
    double betaTol;
    double betaRelax;
    int maxIter;
    float epsilon;
public:
    fvec endpoint;
    fVec endpointFast;
    DynamicalASVM();
    ~DynamicalASVM();
    void Train(std::vector< std::vector<fvec> > trajectories, ivec labels);
    std::vector<fvec> Test( const fvec &sample, const int count);
    fvec Classify( const fvec &sample);
    fvec Test( const fvec &sample);
    fVec Test( const fVec &sample);
    const char *GetInfoString();
    void SaveModel(string filename);
    bool LoadModel(string filename);

    void SetParams(int clusters, double kernelWidth, double Cparam, double alphaTol, double betaTol,
                   double betaRelax, double epsilon, int maxIter);
};

#endif // _DYNAMICAL_ASVM_H_
