/* ************************************************************************ *
 * ************************************************************************ *

   File: discreteObsProb.C
   The class CDiscreteObsProb defines operations
   for the discrete observation distribution B

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
#include "obs.h"
#include "obsProb.h"
#include "discreteObsProb.h"
using namespace std;

//===============================================================================

CDiscreteObsProb::CDiscreteObsProb(int nbSymbols, int nbStates)
{
	mM = nbSymbols;
	mN = nbStates;
	
	mThisB = SetMatrix(mN, mM);
	mNextB = SetMatrix(mN, mM);
	mLogB = SetMatrix(mN, mM);
	InitGaussianRows(mThisB, mN, mM);// Do not randomize this one
//      LogMat(mThisB, mLogB, mN, mM);
}

//===============================================================================

CDiscreteObsProb::CDiscreteObsProb(ifstream &hmmFile, int nbStates)
// Constructor used to read an hmm from a file
{
        int j, k;
	char magicID[32];

	mN = nbStates;
	
	hmmFile >> magicID;
	assert(strcmp(magicID, "M=")==0);
	hmmFile >> mM;// nb of symbols

	mThisB = SetMatrix(mN, mM);
	mNextB = SetMatrix(mN, mM);
//	mLogB = SetMatrix(mN, mM);

	hmmFile >> magicID;
	assert(strcmp(magicID, "B:")==0);

	for (j = 1; j <= mN; j++) { 
		for (k = 1; k <= mM; k++) {
			hmmFile >> mThisB[j][k]; 
		}
	}
//      LogMat(mThisB, mLogB, mN, mM);
}

//===============================================================================

CDiscreteObsProb::~CDiscreteObsProb(void)
{
  delete [] mThisB[1];
  delete [] mThisB;
  delete [] mNextB[1];
  delete [] mNextB;
//  delete [] mLogB[1];
//  delete [] mLogB;
}

//===============================================================================

void CDiscreteObsProb::Start(void)
{
	mSum = SetMatrix(mN, mM);
}

//===============================================================================

void CDiscreteObsProb::StartIter(void)
{
	SetToZero(mSum, mN, mM);
}

//===============================================================================

void CDiscreteObsProb::BWSum(double *gamma, CObs *obs)
{
  int i, obsVal;
  
  CIntObs *intObs = (CIntObs*)obs;
  
  obsVal = intObs->Get();

  for (i = 1; i <= mN; i++) {
      mSum[i][obsVal] += gamma[i];
  }
}

//===============================================================================

void CDiscreteObsProb::SKMSum(int state, CObs *obs)
{
  int obsVal;
  
  CIntObs *intObs = (CIntObs*)obs;
  obsVal = intObs->Get();

  mSum[state][obsVal] += 1.0;// histogram
}

//===============================================================================

double CDiscreteObsProb::EndIter()
{
      double maxDiff;
		
      CopyArray(mSum, mNextB, mN, mM);
      NonZeroNormalizeAllRows(mNextB, mN, mM);// check if mBWSum is really not needed
      maxDiff = MaxElemDiff(mNextB, mThisB, mN, mM);// compare previous and new B result
      CopyArray(mNextB, mThisB, mN, mM);// copy new result into mThisB
//      LogMat(mThisB, mLogB, mN, mM);
      
      return maxDiff;
}

//===============================================================================

void CDiscreteObsProb::End()
{
  delete [] mSum[1];
  delete [] mSum;
}

//===============================================================================

CObs* CDiscreteObsProb::PickObservation(int state)
// Return a random observation given a state using the observation prob. matrix B
{
	int i;
	double val = MyRand();
	CIntObs* obs = new CIntObs;
	double *vect;
	vect = mThisB[state];
	
	for(i=1;i<= mM; i++){
		val = val - vect[i];
		if(val < 0.0){
		        obs->Set(i);
			return obs;
		}
	}
	obs->Set(mM);
	return obs;
}

//===============================================================================

CObs** CDiscreteObsProb::MapStateToObs(void)
// return array of expected observations for all states
{
        int j, k;
	double expectedVal;
	CObs** expectedObs;
	CIntObs* intObs;
	
	expectedObs = new CObs*[mN+1];
 
	for (j=1; j <= mN; j++){// states
	  expectedVal = 0;
	  for(k=1; k<= mM; k++){// observations
	      expectedVal += k * mThisB[j][k];
	  }
	  expectedObs[j] = new CIntObs;
	  intObs = (CIntObs*)expectedObs[j];
	  intObs->Set(int(expectedVal+0.5));
	}
	return expectedObs;
  }

//===============================================================================

void CDiscreteObsProb::Print(ostream &outFile)
{
        outFile << "M= " << mM << endl;

	outFile << "B: " << endl;

        PrintMatrix(outFile, mN, mM, mThisB);

}

//===============================================================================

CObs* CDiscreteObsProb::ReadObsFrom(ifstream &inFile)
{
	CIntObs *obs = new CIntObs;
	obs->ReadFrom(inFile);

	return obs;
}

//===============================================================================

#if 0
void CDiscreteObsProb::ReadFileHeader(ifstream &inFile)
{
  CIntObs::ReadHeader(inFile);
}

//===============================================================================

void CDiscreteObsProb::PrintFileHeader(ostream &outFile)
{
  CIntObs::PrintHeader(outFile);
}

//===============================================================================

#endif

//===============================================================================
//===============================================================================
