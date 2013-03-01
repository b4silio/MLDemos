/* ************************************************************************ *
 * ************************************************************************ *

   File: plainStateTrans.C
   The class CPlainStateTrans defines operations
   for the state transition matrix A

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
#include "stateTrans.h"
#include "plainStateTrans.h"
using namespace std;

//===============================================================================

CPlainStateTrans::CPlainStateTrans(int nbStates, int seed)
{
	mN = nbStates;
	
	mThisA = SetMatrix(mN, mN);
	mNextA = SetMatrix(mN, mN);
	mLogA = SetMatrix(mN, mN);

	MyInitRand(seed);
	SetToRandom(mThisA, mN, mN);// Initialize 
	LogMat(mThisA, mLogA, mN, mN);
}

//===============================================================================

CPlainStateTrans::CPlainStateTrans(int nbStates, int seed, int mode)
{
    mN = nbStates;

    mThisA = SetMatrix(mN, mN);
    mNextA = SetMatrix(mN, mN);
    mLogA = SetMatrix(mN, mN);

    //Set everything to 0 at first
    for(int i=1;i<=mN;i++){
        for(int j=1;j<=mN;j++){
            mThisA[i][j]=0;
        }
    }

    MyInitRand(seed);
    switch(mode)
    {
        case 0:
            for(int i=1;i<=mN;i++)
            {
                for(int j=1;j<=mN;j++)
                {
                    mThisA[i][j] = 1;
                }
            }
            break;
        case 1:
        for(int i=1;i<=mN;i++)
        {
            for(int j=i;j<=mN;j++)
            {
                mThisA[i][j] = CenteredRand();
            }
        }
            break;
        case 2:
            for(int i=1;i<mN;i++)
            {
                double c = 1.0/(double)(mN-i+1);
                for(int j=i;j<=mN;j++)
                {
                    mThisA[i][j] = c;
                }
            }
            mThisA[mN][mN] = 1.0;
            break;
    }
    NormalizeAllRows(mThisA,mN,mN);
    LogMat(mThisA, mLogA, mN, mN);
}

//===============================================================================

CPlainStateTrans::CPlainStateTrans(ifstream &hmmFile, int nbStates)
// Constructor used to read an hmm from a file
{
        int i, j;
    char magicID[32];

    mN = nbStates;
	mThisA = SetMatrix(mN, mN);
	mNextA = SetMatrix(mN, mN);
	mLogA = SetMatrix(mN, mN);

	hmmFile >> magicID;
    assert(strcmp(magicID, "A:")==0);

	for (i = 1; i <=mN; i++) { 
		for (j = 1; j <= mN; j++) {
			hmmFile >>  mThisA[i][j];
		}
	}
#if 0
	NonZeroNormalizeAllRows(mThisA, mN, mN);// cancel zero terms
#else
	NormalizeAllRows(mThisA, mN, mN);
#endif
	LogMat(mThisA, mLogA, mN, mN);
}

//===============================================================================

CPlainStateTrans::~CPlainStateTrans(void)
{
  delete [] mThisA[1];
  delete [] mThisA;
  delete [] mNextA[1];
  delete [] mNextA;
  delete [] mLogA[1];
  delete [] mLogA;
}


//===============================================================================

void CPlainStateTrans::Start(void)
{
	mSum = SetMatrix(mN, mN);
}

//===============================================================================

void CPlainStateTrans::StartIter(void)
{
	SetToZero(mSum, mN, mN);
}

//===============================================================================

void CPlainStateTrans::BWSum(double **xi)
// Sum xi
{
	int i, j;
	
   for (i = 1; i <= mN; i++) { 
		for (j = 1; j <= mN; j++) {
			mSum[i][j] += xi[i][j];
		}
   }
}

//===============================================================================

void CPlainStateTrans::SKMSum(int thisState, int nextState)
{

  mSum[thisState][nextState] += 1;

}

//===============================================================================

double CPlainStateTrans::EndIter(void)
{
      double maxDiff;
		
      CopyArray(mSum, mNextA, mN, mN);// do not touch mBWSum
#if 0
      NonZeroNormalizeAllRows(mNextA, mN, mN);
#else
      NormalizeAllRows(mNextA, mN, mN);
#endif
      maxDiff = MaxElemDiff(mNextA, mThisA, mN, mN);// compare A and nextA
      CopyArray(mNextA, mThisA, mN, mN);// copy mNextA into mThisA
      LogMat(mThisA, mLogA, mN, mN);
      
      return maxDiff;
}

//===============================================================================

void CPlainStateTrans::End(void)
{
  delete [] mSum[1];
  delete [] mSum;
}

//===============================================================================

int CPlainStateTrans::PickNextState(int presentState)
// Return a new random state given a state using the transition matrix A
{
	int i;
	double val = MyRand();
	
	for(i=1;i<= mN; i++){
		val = val - mThisA[presentState][i];
		if(val < 0.0){
			return i;
		}
	}
	return mN;
}

//===============================================================================

void CPlainStateTrans::Print(ostream &outFile)
{

	outFile << "N= " << mN << endl;
	
	outFile << "A: " << endl;

        PrintMatrix(outFile, mN, mN, mThisA);

}

//===============================================================================
//===============================================================================
