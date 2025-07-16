/*********************************************************************
MLDemos: A User-Friendly visualization toolkit for machine learning
Copyright (C) 2010  Basilio Noris
Contact: mldemos@b4silio.com

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free
Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*********************************************************************/
#include "public.h"
#include "dynamicalHMM.h"
#include <QDebug>
#include <QTime>

using namespace std;

DynamicalHMM::~DynamicalHMM()
{
    int N = learnedHMM.size();
    FOR(i, N)
    {
        DEL(learnedHMM[i]);
        DEL(pi[i]);
        DEL(b[i]);
        DEL(a[i]);
    }
}

void DynamicalHMM::Train(std::vector< std::vector<fvec> > trajectories, ivec trajLabels)
{
	if(!trajectories.size()) return;
	int count = trajectories[0].size();
	if(!count) return;
	dim = trajectories[0][0].size();

    if(!learnedHMM.size())
    {
        int N = learnedHMM.size();
        FOR(i, N)
        {
            DEL(learnedHMM[i]);
            DEL(pi[i]);
            DEL(b[i]);
            DEL(a[i]);
        }
    }

	int nbDimensions = dim/2;
	int nbSymbols = 1000;
	int nbStates = states;
	int seed = QTime::currentTime().msec();
	bool isGaussian = obsType == 0;
	int whichEMMethod = trainType;
    int whichInitialVector = initType;
    int whichTransitionMatrix = transType;

	FOR(i, trajectories.size())
	{
		FOR(j, trajectories[i].size())
		{
			FOR(d, dim)
			{
				float v = trajectories[i][j][d];
				v *= 1000.f;
				trajectories[i][j][d] = v;
			}
		}
	}

	this->trajectories = trajectories;

    // Find number of different trajectory classes
    int NClass = *(std::max_element(trajLabels.begin(),trajLabels.end()));

    // Split overall trajectories into "per class" trajectories
    FOR(i, NClass)
    {
        data.push_back(vector< vector<fvec> >());
    }

    FOR(i, trajectories.size())
    {
        data[(int)(trajLabels[i]-1)].push_back(trajectories[i]);
    }

    FOR(i, NClass)
    {
        CObs *obsType;

        switch(whichTransitionMatrix)
        {
            case 0:
                a.push_back(new CPlainStateTrans(nbStates, seed));
                break;
            case 1:
                a.push_back(new CPlainStateTrans(nbStates, seed, 0));
                break;
            case 2:
                a.push_back(new CPlainStateTrans(nbStates, seed, 1));
                break;
            case 3:
                a.push_back(new CPlainStateTrans(nbStates, seed, 2));
                break;
        }

        obsType = new CVectorObs(nbDimensions);
        b.push_back(new CVectorObsProb(nbSymbols, nbStates, nbDimensions, isGaussian));

        switch(whichInitialVector)
        {
            case 0:
                pi.push_back(new CInitStateProb(nbStates));
                break;
            case 1:
                pi.push_back(new CInitStateProb(nbStates, 0));
                break;
            case 2:
                pi.push_back(new CInitStateProb(nbStates, 1));
                break;
        }

        learnedHMM.push_back(new CHMM(a[i], b[i], pi[i]));
        cout << "Initial Model\n";
        learnedHMM[i]->Print(cout);

        CObsSeq *obsSeq;
        obsSeq = new CObsSeq(obsType, data[i]);

        switch (whichEMMethod){
            case 0:
                learnedHMM[i]->LearnBaumWelch(obsSeq);
                break;
            case 1:
                learnedHMM[i]->LearnSegmentalKMeans(obsSeq);
                break;
            case 2:
                learnedHMM[i]->LearnHybridSKM_BW(obsSeq);
                break;
        }

        cout << "Learned Model\n";
        learnedHMM[i]->Print(cout);

        //cout << "States and Expected Observations\n";
        //learnedHMM->PrintStatesAndExpectedObs(obsSeq, cout, cout);

        // Distances to each sequence in obsSeq and total distance are stored in distanceOutput file
        //cout << "Viterbi Distances to each sequence\n";
        double normalizedLogProb = learnedHMM[i]->FindViterbiDistance(obsSeq, cout);

        delete obsSeq;// check with purify
    }
}

bool DynamicalHMM::IsOrphanedState(int HMMClass, int state)
{
	if(state > states) return true;
    FOR(i, a[HMMClass]->GetN())
	{
        if(a[HMMClass]->at(state+1, i+1) != 1. / a[HMMClass]->GetN()) return false;
	}
	return true;
}


std::vector<fvec> DynamicalHMM::Test( const fvec &sample, int count)
{
	dim = sample.size();
	std::vector<fvec> res(10);
	FOR(i, res.size()) res[i] = fvec(dim,0);
	return res;
}

fvec DynamicalHMM::Test( const fvec &sample)
{
	int dim = sample.size();
	if(!dim) return fvec(2,0);
	fvec res(dim, 0);
	return res;
}

fVec DynamicalHMM::Test( const fVec &sample)
{
	fVec res;
	return res;
}

void DynamicalHMM::SetParams(int mixtures, int states, int trainType, int obsType, int initType, int transType)
{
	this->mixtures = mixtures;
	this->states = states;
	this->trainType = trainType;
	this->obsType = obsType;
    this->initType = initType;
    this->transType = transType;
}

char *DynamicalHMM::GetInfoString()
{
	char *text = new char[2048];
	sprintf(text, "HMM\n");
	sprintf(text, "%sMixture Components: %d\n", text, mixtures);
	sprintf(text, "%sStates: %d\n", text, mixtures);
	sprintf(text, "%sTraining Method\n", text);
	switch(trainType)
	{
	case 0:
		sprintf(text, "%sBaum-Welch\n", text);
		break;
	case 1:
		sprintf(text, "%sSegmental K-Means\n", text);
		break;
	case 2:
		sprintf(text, "%sHybrid\n", text);
		break;
	}
	return text;
}
