/* ************************************************************************ *
 * ************************************************************************ *

   File: explicitDurationTrans.C
   The class CExplicitDurationTrans defines operations
   for the state transition matrix A
   when the probability of staying in each state is computed explicitely

  * ************************************************************************ *

   Authors: Daniel DeMenthon & Marc Vuilleumier
   Date:  2-18-99 

 * ************************************************************************ *

   Modification Log:
	5-27-99: Normalize without worrying about zeros and 
	use utils.C log functions that check for zeros 
	and put -1000 in those cases as log(0)

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
#include "stateTrans.h"
#include "gammaProb.h"
#include "explicitDurationTrans.h"
using namespace std;

//===============================================================================

CExplicitDurationTrans::CExplicitDurationTrans(int nbStates, int seed)
{
	int i;
	
	mN = nbStates;
	
	mThisA = SetMatrix(mN, mN);
	mNextA = SetMatrix(mN, mN);
	mLogA = SetMatrix(mN, mN);
	mRowA = SetVector(mN);	

	mViterbiDuration = SetIntVector(mN);
	mGammaProb = new CGammaProb[mN+1];

	MyInitRand(seed);
	SetToRandom(mThisA, mN, mN);// Initialize 
	LogMat(mThisA, mLogA, mN, mN);

	for (i = 1; i <=mN; i++) { 
		mGammaProb[i].IsDefined(false);
		mGammaProb[i].ResetSums();
	}
}

//===============================================================================

CExplicitDurationTrans::CExplicitDurationTrans(ifstream &hmmFile, int nbStates)
// Constructor used to read an hmm from a file
{
        int i, j;
	char magicID[32];

	mN = nbStates;

	mThisA = SetMatrix(mN, mN);
	mNextA = SetMatrix(mN, mN);
	mLogA = SetMatrix(mN, mN);
	mRowA = SetVector(mN);	

	mViterbiDuration = SetIntVector(mN);
	mGammaProb = new CGammaProb[mN+1];

	hmmFile >> magicID;
	assert(strcmp(magicID, "A:")==0);

	for (i = 1; i <=mN; i++) { 
		for (j = 1; j <= mN; j++) {
			hmmFile >>  mThisA[i][j];
		}
	}
#if 0	
	NonZeroNormalizeAllRows(mThisA, mN, mN);
#else
	NormalizeAllRows(mThisA, mN, mN);
#endif
	LogMat(mThisA, mLogA, mN, mN);
	
// Read state duration probability means and stds
	hmmFile >> magicID;
	assert(strcmp(magicID, "Durations:")==0);

	for (i = 1; i <= mN; i++) { 
	  mGammaProb[i].Read(hmmFile);
	}
}

//===============================================================================

CExplicitDurationTrans::~CExplicitDurationTrans(void)
{
  delete [] mViterbiDuration;
  delete [] mGammaProb;
  delete [] mThisA[1];
  delete [] mThisA;
  delete [] mNextA[1];
  delete [] mNextA;
  delete [] mLogA[1];
  delete [] mLogA;
  delete [] mRowA;
}

//===============================================================================


double CExplicitDurationTrans::logAt(int sourceState, int destState)
// Use duration in addition to matrix A
{
	  if(mGammaProb[sourceState].IsDefined() == false){// no gamma distribution available
	  		return mLogA[sourceState][destState];// do not use state durations
	  }
	  
	  double logAij;
	  int duration = mViterbiDuration[sourceState];

	  if(sourceState == destState){
	    logAij = mGammaProb[sourceState].GetStayProb(duration);
	  }// end same state

	  else{// different states i and j
	    logAij = mGammaProb[sourceState].GetLeaveProb(duration) + 
	    			mLogA[sourceState][destState] - 
	    			log(1-mThisA[sourceState][sourceState]);// if Aij is normalized
	  }
	  return logAij;
}     

//===============================================================================

void CExplicitDurationTrans::Start(void)
{
	mSum = SetMatrix(mN, mN);
}

//===============================================================================

void CExplicitDurationTrans::StartIter(void)
{
	int i;

	SetToZero(mSum, mN, mN);

	mDuration = 1;// duration for any one state
	
   for (i = 1; i <= mN; i++){
   	mGammaProb[i].ResetSums();
   }
}

//===============================================================================

void CExplicitDurationTrans::SKMSum(int thisState, int nextState)
{
  mSum[thisState][nextState] += 1;

  if(nextState == thisState){
      mDuration++;
  }
  else{// state just changed
      mGammaProb[thisState].SKMSum(mDuration);
      mDuration = 1;
  }
}

//===============================================================================

void CExplicitDurationTrans::InitViterbiDurations(int thisState)
// call when initializing ViterbiLog
{
      mViterbiDuration[thisState] = 1;
}

//===============================================================================

void CExplicitDurationTrans::UpdateViterbiDurations(int thisState, int nextState)
// call in ViterbiLog after max delta is found
{
  if(nextState == thisState){
      mViterbiDuration[nextState]++;
  }
  else{// state just changed
    mViterbiDuration[nextState] = 1;
  }
}

//===============================================================================

double CExplicitDurationTrans::EndIter(void)
{
		int i;
		double maxDiff;
		
		CopyArray(mSum, mNextA, mN, mN);// do not touch mBWSum
#if 0
		NonZeroNormalizeAllRows(mNextA, mN, mN);
#else
		NormalizeAllRows(mNextA, mN, mN);
#endif
		maxDiff = MaxElemDiff(mNextA, mThisA, mN, mN);// compare A and nextA
		CopyArray(mNextA, mThisA, mN, mN);
		LogMat(mThisA, mLogA, mN, mN);

		for (i = 1; i <= mN; i++){
			mGammaProb[i].ProcessSums();
		}
		return maxDiff;
}

//===============================================================================

void CExplicitDurationTrans::End(void)
{
  delete [] mSum[1];
  delete [] mSum;
}

//===============================================================================

void CExplicitDurationTrans::InitDuration(int thisState)
{
	mDuration = 1;
	if(mGammaProb[thisState].IsDefined()){
		mRandomGammaDuration = int(floor(0.5 + mGammaProb[thisState].FindRandomDuration()));
		assert(mRandomGammaDuration>=1);
	}
}

//===============================================================================

int CExplicitDurationTrans::PickNextState0(int thisState)
// Return a new random state given a state using the transition matrix A and durations
// mDuration have been initialized to 1 by InitDuration() when PickInitState is set
{
	int i;
	int nextState;
	double gammaProb;
	
// Assume that we are staying in present state
// What is the probability for that to happen?
// This is our new A[i][i] for thisState
	mDuration++;
	
	if(!mGammaProb[thisState].IsDefined()){
		nextState = PickRandomState(mThisA[thisState]);	
	}
	else{
		for(i=1;i<= mN; i++){
			mRowA[i] = mThisA[thisState][i];// avoid roundoffs from multiple renormalizations
		}
		gammaProb = exp(mGammaProb[thisState].GetStayProb(mDuration));// new mA[i][i]
		assert(gammaProb < 1.0);
		mRowA[thisState] = gammaProb;
		NormalizeRow(mRowA, mN);// renormalize since we changed mA[i][i]
			
		nextState = PickRandomState(mRowA);
	}// end else
	
	if(nextState != thisState){
		mDuration = 1;
	}
	
	return nextState;
}

//===============================================================================

int CExplicitDurationTrans::PickNextState(int thisState)
// Return a new random state given a state using the transition matrix A and durations
// In this variant we explicitly find a probable duration according to a Gamma distribution
// and use it to stay in the same state
{
	int i;
	int nextState;
	
// Assume that we are staying in present state
// What is the probability for that to happen?
// This is our new A[i][i] for thisState
	mDuration++;
	if(!mGammaProb[thisState].IsDefined()){
		nextState = PickRandomState(mThisA[thisState]);// use regular mA
		if(nextState != thisState){
			mDuration = 1;
		}
	}
	else{// gamma prob defined
		if(mDuration<=mRandomGammaDuration){
			nextState = thisState;// stay in same state if duration is not over
		}
		else{// duration is over
			for(i=1;i<= mN; i++){
				mRowA[i] = mThisA[thisState][i];// avoid roundoffs from multiple renormalizations
			}
			mRowA[thisState] = 0.0;// time is up, we have to get out of thisState
			NormalizeRow(mRowA, mN);// renormalize since we changed mA[i][i]
			nextState = PickRandomState(mRowA);
			assert(nextState != thisState);
			InitDuration(nextState);
		}// end duration over
	}// end gamma defined
		
	return nextState;
}

//===============================================================================

int CExplicitDurationTrans::PickRandomState(double *rowA)
// Return a new random state given a state using a row of matrix A
{
	int i;
	double val = MyRand();
	
	for(i=1;i<= mN; i++){
		val = val - rowA[i];
		if(val < 0.0){
			return i;
		}
	}
	return mN;
}

//===============================================================================


double CExplicitDurationTrans::DurationPenalty(int *q, int T)
// Compute a postprocessing penalty over a sequence of states q[1..T]
// using delta penalties relates to durations for staying in each state
// 
{
	int t;
	int thisState, nextState, duration;
	double deltaPenalty;
	
	double durationPenalty = 0.0;

	thisState = q[1];
	duration = 1;
	
	for(t=2;t<= T; t++){
	  nextState = q[t];
	  if(!mGammaProb[thisState].IsDefined()){
	  		deltaPenalty = 0;
	  }
	  else{
	  	 if(nextState == thisState){
	    	deltaPenalty = mGammaProb[thisState].GetStayProb(duration);
	    }
	    else{// state changed
	    	deltaPenalty = mGammaProb[thisState].GetLeaveProb(duration);
	    }
	  }

	  if(nextState == thisState){
	  		duration++;
	  }
	  else{
	  		duration = 1;
	  }
	  thisState = nextState;
	  durationPenalty += deltaPenalty;
	}
	return durationPenalty;
}

//===============================================================================

void CExplicitDurationTrans::Print(ostream &outFile)
{

        int i;
	outFile << "N= " << mN << endl;
	
	outFile << "A: " << endl;

	PrintMatrix(outFile, mN, mN, mThisA);

	outFile << "Durations: " << endl;

	for (i=1; i <= mN; i++){// states
	  mGammaProb[i].Print(outFile);
	}
}

//===============================================================================
//===============================================================================
