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
	DEL(learnedHMM);
	DEL(pi);
	DEL(b);
	DEL(a);
}

void DynamicalHMM::Train(std::vector< std::vector<fvec> > trajectories, ivec labels)
{
	if(!trajectories.size()) return;
	int count = trajectories[0].size();
	if(!count) return;
	dim = trajectories[0][0].size();

	DEL(learnedHMM);
	DEL(pi);
	DEL(b);
	DEL(a);

	int nbDimensions = dim/2;
	int nbSymbols = 1000;
	int nbStates = states;
	int seed = QTime::currentTime().msec();
	bool isGaussian = obsType == 0;
	int whichEMMethod = trainType;

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

	CObs *obsType;

	a = new CPlainStateTrans(nbStates, seed);
	obsType = new CVectorObs(nbDimensions);
	b = new CVectorObsProb(nbSymbols, nbStates, nbDimensions, isGaussian);

	pi = new CInitStateProb(nbStates);

	learnedHMM = new CHMM(a, b, pi);

	CObsSeq *obsSeq;
	obsSeq = new CObsSeq(obsType, trajectories);

	switch (whichEMMethod){
	case 0:
		learnedHMM->LearnBaumWelch(obsSeq);
		break;
	case 1:
		learnedHMM->LearnSegmentalKMeans(obsSeq);
		break;
	case 2:
		learnedHMM->LearnHybridSKM_BW(obsSeq);
		break;
	}

	cout << "Learned Model\n";
	learnedHMM->Print(cout);

	//cout << "States and Expected Observations\n";
	//learnedHMM->PrintStatesAndExpectedObs(obsSeq, cout, cout);

	// Distances to each sequence in obsSeq and total distance are stored in distanceOutput file
	//cout << "Viterbi Distances to each sequence\n";
	double normalizedLogProb = learnedHMM->FindViterbiDistance(obsSeq, cout);

	//delete obsSeq;// check with purify

}

bool DynamicalHMM::IsOrphanedState(int state)
{
	if(state > states) return true;
	FOR(i, a->GetN())
	{
		if(a->at(state+1, i+1) != 1. / a->GetN()) return false;
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

void DynamicalHMM::SetParams(int mixtures, int states, int trainType, int obsType)
{
	this->mixtures = mixtures;
	this->states = states;
	this->trainType = trainType;
	this->obsType = obsType;
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
