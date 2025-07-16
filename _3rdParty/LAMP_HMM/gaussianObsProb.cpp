/* ************************************************************************ *
 * ************************************************************************ *

   File: gaussianObsProb.C
   The class CGaussianObsProb defines operations 
   for the gaussian observation distribution B

  * ************************************************************************ *

   Authors: Daniel DeMenthon & Marc Vuilleumier
   Date:  2-25-99 

 * ************************************************************************ *

   Modification Log:


 * ************************************************************************ *
   Log for new ideas:
 * ************************************************************************ *
               Language and Media Processing
               Center for Automation Research
               University of Maryland
               College Park, MD  20742
 * ************************************************************************ *
 * ************************************************************************ */

//===============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "utils.h"
#include "obs.h"
#include "obsProb.h"
#include "gaussianObsProb.h"
using namespace std;

//===============================================================================

CGaussianObsProb::CGaussianObsProb(int nbSymbols, int nbStates)
{
	mM = nbSymbols;
	mN = nbStates;
	
	mMean = SetVector(mN);
	mStd = SetVector(mN);

	InitParameters();
}

//===============================================================================

CGaussianObsProb::CGaussianObsProb(int nbSymbols, int nbStates, double *means, double* stds)
{
	CGaussianObsProb(nbSymbols, nbStates);
	InitParameters(means, stds);
}

//===============================================================================

CGaussianObsProb::CGaussianObsProb(ifstream &hmmFile, int nbStates)
// Constructor used to read an hmm from a file
{
	int j;
	char magicID[32];

	mN = nbStates;
	
	mMean = SetVector(mN);
	mStd = SetVector(mN);

	hmmFile >> magicID;
	assert(strcmp(magicID, "M=")==0);
	hmmFile >> mM;// nb of symbols

	hmmFile >> magicID;
	assert(strcmp(magicID, "B:")==0);

	for (j = 1; j <= mN; j++) { 
		hmmFile >> mMean[j];
		hmmFile >> mStd[j];
	}
}

//===============================================================================

CGaussianObsProb::~CGaussianObsProb(void)
{
	delete [] mMean;
	delete [] mStd;
}

//===============================================================================

void CGaussianObsProb::InitParameters(void)
// Initialize means and stds with nice values
{
	int j;
	double slope = (mM-1)/(double)(mN-1);

	for (j = 1; j <= mN; j++) { 
		mMean[j] = 1 + slope * (j-1);
		mStd[j] = double(mM)/mN;
	}
}

//===============================================================================

void CGaussianObsProb::InitParameters(double *means, double* stds)
// Initialize means and stds with provided values
{
	int j;

	for (j = 1; j <= mN; j++) { 
		mMean[j] = means[j];
		mStd[j] = stds[j];
	}
}

//===============================================================================

double CGaussianObsProb::at(int state, CObs *obs)
{
	CIntObs *intObs = (CIntObs*)obs;

	int x = intObs->Get();
	double mean = mMean[state];
	double std = mStd[state];
	double prob = GetGauss(x, mean, std);

	return prob;
}

//===============================================================================

void CGaussianObsProb::Start(void)
{
	mSumMean = SetVector(mN);
	mSumVar = SetVector(mN);
	mDiv = SetVector(mN);
}

//===============================================================================

void CGaussianObsProb::StartIter(void)
{
	SetToZero(mSumMean, mN);
	SetToZero(mSumVar, mN);
	SetToZero(mDiv, mN);
}

//===============================================================================

void CGaussianObsProb::BWSum(double *gamma, CObs *obs)
{
	int i, obsVal, obsValSquared;

	CIntObs *intObs = (CIntObs*)obs;

	obsVal = intObs->Get();
	obsValSquared = obsVal * obsVal;
	for (i = 1; i <= mN; i++) {
		mSumMean[i] += obsVal * gamma[i];
		mSumVar[i] += obsValSquared * gamma[i];
		mDiv[i] += gamma[i];
	}
}

//===============================================================================

void CGaussianObsProb::SKMSum(int state, CObs *obs)
{
	int obsVal, obsValSquared;

	CIntObs *intObs = (CIntObs*)(obs);

	obsVal = intObs->Get();
	obsValSquared = obsVal * obsVal;
	mSumMean[state] += obsVal;
	mSumVar[state] += obsValSquared;
	mDiv[state] += 1;
}

//===============================================================================

double CGaussianObsProb::EndIter()
// Finish Baum-Welch iteration; return magnitude of largest change
{
	int i;
	double div, sumDiv, mean, stdev, elemDiff;
	double maxDiff = 0;
	const double limitStd = 0.2;

	for(i = 1; i <= mN; i++){
		div = mDiv[i];
		if(div>0){
			mean = mSumMean[i]/div;
			stdev = sqrt(mSumVar[i]/div - (mean * mean));
		}
		else{
			sumDiv = Sum(mDiv, mN);
			mean = Sum(mSumMean, mN)/sumDiv;
			stdev = sqrt(Sum(mSumVar, mN)/sumDiv - (mean * mean));;
		}
		if(stdev <limitStd) stdev =limitStd;

		elemDiff = fabs(mMean[i] - mean)/stdev;
		if(elemDiff>maxDiff) maxDiff = elemDiff;
		elemDiff = fabs(mStd[i] - stdev)/stdev;
		if(elemDiff>maxDiff) maxDiff = elemDiff;
		mMean[i] = mean;
		mStd[i] = stdev;
	}
	return maxDiff;
}

//===============================================================================

void CGaussianObsProb::End()
// Finish Baum-Welch session
{
	delete [] mSumMean;
	delete [] mSumVar;
	delete [] mDiv;
}

//===============================================================================

CObs* CGaussianObsProb::PickObservation(int state)
// Return a random observation given a state using the observation prob. matrix B
{
	double val;
	int intVal;
	CIntObs *obs = new CIntObs;
	
	val = mStd[state] * GetRandomGauss() + mMean[state];
	intVal = int(val+0.5);
	obs->Set(intVal);
	return obs;
}

//===============================================================================


CObs** CGaussianObsProb::MapStateToObs(void)
// return array of expected observations for all states
{
	int j;
	CIntObs** expectedObs;
	
	expectedObs = new CIntObs*[mN+1];

	for (j=1; j <= mN; j++){// states
		expectedObs[j] = new CIntObs;
		expectedObs[j]->Set(int(mMean[j] + 0.5));
	}
	return (CObs**)expectedObs;
}

//===============================================================================

void CGaussianObsProb::Print(ostream &outFile)
{
	int j;

	outFile << "M= " << mM << endl;

	outFile << "B: " << endl;
	for (j=1; j <= mN; j++){// states
		outFile << mMean[j] << " " << mStd[j] << endl;
	}
}

//===============================================================================

CObs* CGaussianObsProb::ReadObsFrom(ifstream &inFile)
{
	CIntObs *obs = new CIntObs;
	obs->ReadFrom(inFile);

	return obs;
}

//===============================================================================

#if 0

void CGaussianObsProb::ReadFileHeader(ifstream &inFile)
{
	CIntObs::ReadHeader(inFile);
}

//===============================================================================

void CGaussianObsProb::PrintFileHeader(ostream &outFile)
{
	CIntObs::PrintHeader(outFile);
}

#endif

//===============================================================================
//===============================================================================
//===============================================================================
