/* ************************************************************************ *
 * ************************************************************************ *

   File: initStateProb.C
   The class CInitStateProb defines operations
   for the initial state probabilities Pi

  * ************************************************************************ *

   Authors: Daniel DeMenthon & Marc Vuilleumier
   Date:  2-18-99 

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
#include "initStateProb.h"
using namespace std;

//===============================================================================

CInitStateProb::CInitStateProb(int nbStates)
{
	mN = nbStates;
	
	mThisPi = SetVector(mN);
	mNextPi = SetVector(mN);
	mLogPi = SetVector(mN);

	SetToEqual(mThisPi, mN);// Normalized vector with equal terms
	LogVect(mThisPi, mLogPi, mN);
}

//===============================================================================

CInitStateProb::CInitStateProb(int nbStates, int mode)
{
    mN = nbStates;

    mThisPi = SetVector(mN);
    mNextPi = SetVector(mN);
    mLogPi = SetVector(mN);

    switch(mode)
    {
        case 0:
            SetToRandom(mThisPi, mN);// Normalized vector with random terms
            break;
        case 1:
            SetToZero(mThisPi, mN);// Normalized vector where the first term-state is 1 and all the rest are 0
            mThisPi[1] = 1.0;
            break;
    }
    LogVect(mThisPi, mLogPi, mN);
}

//===============================================================================
CInitStateProb::CInitStateProb(ifstream &hmmFile, int nbStates)
// Constructor using data from file; A, B and Pi can be read from same file
{
        int i;
	char magicID[32];
	mN = nbStates;
	
	mThisPi = SetVector(mN);
	mNextPi = SetVector(mN);
	mLogPi = SetVector(mN);

	hmmFile >> magicID;
	assert(strcmp(magicID, "pi:")==0);

	for (i = 1; i <= mN; i++) {
		hmmFile >> mThisPi[i];
	}
#if 0
	NonZeroNormalizeRow(mThisPi, mN);
#else
	NormalizeRow(mThisPi, mN);
#endif
	LogVect(mThisPi, mLogPi, mN);
}

//===============================================================================

CInitStateProb::~CInitStateProb(void)
{
  delete [] mThisPi;
  delete [] mNextPi;
  delete [] mLogPi;
}

//===============================================================================

void CInitStateProb::Start(void)
{
	mSum = SetVector(mN);
}

//===============================================================================

void CInitStateProb::StartIter(void)
{
	SetToZero(mSum, mN);
}

//===============================================================================

void CInitStateProb::BWSum(double *gamma)
{
  int i;
	
  for (i = 1; i <= mN; i++) { 
    mSum[i] += gamma[i];
  }
}


//===============================================================================

void CInitStateProb::SKMSum(int state)
{	
  mSum[state] += 1;
}


//===============================================================================

double CInitStateProb::EndIter(void)
{
      double maxDiff;
		
      CopyArray(mSum, mNextPi, mN);
#if 0
      NonZeroNormalizeRow(mNextPi, mN);
#else
      NormalizeRow(mNextPi, mN);
#endif
      maxDiff = MaxElemDiff(mNextPi, mThisPi, mN);// compare previous and new Pi result
      CopyArray(mNextPi, mThisPi, mN);// copy new result into Pi
      LogVect(mThisPi, mLogPi, mN);
      
      return maxDiff;
}

//===============================================================================

void CInitStateProb::End(void)
{
  delete [] mSum;
}

//===============================================================================

int CInitStateProb::PickInitialState(void)
// Return a random initial state using the discrete distribution Pi
{
	int i;
	double val = MyRand();
	
	for(i=1;i<= mN; i++){
		val = val - mThisPi[i];
		if(val < 0.0){
			return i;
		}
	}
	return mN;
}

//===============================================================================

void CInitStateProb::Print(ostream &outFile)
{
	outFile << "pi: " << endl;
	PrintVector(outFile, mN, mThisPi);
}

//===============================================================================
//===============================================================================
