/* ************************************************************************ *
 * ************************************************************************ *

   File: obsSeq.C
   The class CObsSeq defines operations on observation sequences for HMM

  * ************************************************************************ *

   Authors: Daniel DeMenthon & Marc Vuilleumier
   Date:  2-18-99 

 * ************************************************************************ *

   Modification Log:
	5-11-99: Initialized m1DArray in second constructor and called Make1DArray
	Was bug since m1DArray was destroyed without being called

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
#include "obsSeq.h"
using namespace std;

//===============================================================================
#ifndef DEBUG
#define DEBUG
#endif
//===============================================================================

//===============================================================================

#if 0
CObsSeq::CObsSeq(long nbSequences, long nbObs)
// Constructor used with random generation of observations
// Each sequence length is set equal to nbObs
{
	long T;
	int i;

	mNbSequences = nbSequences;
	mNbObs = new long[mNbSequences+1];
	mObs = new CObs**[mNbSequences+1];

	T = nbObs;

	for(i=1;i<=mNbSequences;i++){
		mNbObs[i] = T;// all sequences have same length
	}
}
#endif

//===============================================================================

CObsSeq::CObsSeq(CObs *obsType, long nbSequences, long nbObs)
// Constructor used with random generation of observations
// Each sequence length is set equal to nbObs
{
	long T;
	int i;

	mObsType = obsType;
	mObsCount = nbSequences * nbObs;
	mNbSequences = nbSequences;
	mNbObs = new long[mNbSequences+1];
	mObs = new CObs**[mNbSequences+1];

	T = nbObs;

	for(i=1;i<=mNbSequences;i++){
		mNbObs[i] = T;// all sequences have same length
	}
	m1DArray = new CObs*[mObsCount+1];
}

//===============================================================================

CObsSeq::CObsSeq(CObs *obsType, std::vector< std::vector< std::vector<float> > > sequences)
// Constructor used with random generation of observations
// Each sequence length is set equal to nbObs
{
	long T;
	int i,j,d;
	int nbSequences = sequences.size();
	int nbObs = sequences[0].size();
	mObsType = obsType;
	mObsCount = nbSequences * nbObs;
	mNbSequences = nbSequences;
	mNbObs = new long[mNbSequences+1];
	mObs = new CObs**[mNbSequences+1];
    int dim = sequences[0][0].size();


	T = nbObs;

	for(i=1;i<=mNbSequences;i++){
		mNbObs[i] = T;// all sequences have same length
		mObsCount += T;
		mObs[i] = mObsType->AllocateVector(T+1);

		for(j=1; j <= T; j++){
			for(d=1; d <= dim; d++)
			{
				mObs[i][j]->SetDouble(sequences[i-1][j-1][d-1],d);
			}
		}
	}
	m1DArray = new CObs*[mObsCount+1];
	Make1DArray();
}

//===============================================================================


CObsSeq::CObsSeq(CObs *obsType, ifstream &inFile)
// Constructor reading observation sequences from file
{
	long i, j, T, nbSequences;
	char magicID[32];

	mObsType = obsType;
	mObsCount = 0;

	mObsType->ReadHeader(inFile);
//        inFile >> magicID; 
//        assert(strcmp(magicID, "P6")==0);
	inFile >> magicID;
	assert(strcmp(magicID, "nbSequences=")==0);
	inFile >> nbSequences;
	mNbSequences = nbSequences;
	mObs = new CObs**[mNbSequences+1];
	mNbObs = new long[mNbSequences+1];

	for(i=1;i<=mNbSequences;i++){
		cout <<"Sequence "<< i <<endl;
		inFile >> magicID;
		assert(strcmp(magicID, "T=")==0);

		inFile >> T;// nb of observations for each sequence
		mNbObs[i] =  T;
		mObsCount += T;
		mObs[i] = mObsType->AllocateVector(T+1);// This array is from 1 to T

		for(j=1; j <=T; j++){
			mObs[i][j]->ReadFrom(inFile);// check that it's using the right subclass
		}
	}
#if 1
	double averageT = mObsCount/nbSequences;
	cout << "Average T = " << averageT << endl << endl;
#endif	
	m1DArray = new CObs*[mObsCount+1];
	Make1DArray();
}

//===============================================================================

CObsSeq::~CObsSeq(void)
{
  long i, j;
  long T;

  delete [] m1DArray;

  for(i=1;i<=mNbSequences;i++){
    T = mNbObs[i];
    for(j=1; j <=T; j++){
      delete mObs[i][j];// individual CObs classes
    }
    delete [] mObs[i];
  }
  delete [] mObs;
  delete [] mNbObs;
#if 0 // we should not delete mObsType, the pointer comes from outside
  if(mObsType != NULL){
    delete mObsType;
    mObsType = NULL;
  }
#endif
}

//===============================================================================

void CObsSeq::Make1DArray(void)
{
	long i, j, T;
	long iCount = 1;
	
	for(i=1;i<=mNbSequences;i++){
		T = mNbObs[i];
		for(j=1;j<=T; j++){
			m1DArray[iCount++] = mObs[i][j];// array of pointers to data in mObs
		}
	}
}

//===============================================================================

void CObsSeq::Print(ostream &outFile)
{
	long i, j;
	long T;
	
	PrintHeader(outFile);

	for(i=1;i<=mNbSequences;i++){
		T = mNbObs[i];
		outFile <<"T= "<< T << endl;
		for(j=1;j<=T;j++){
			mObs[i][j]->Print(outFile);
		}
		outFile << endl;
	}
}

//===============================================================================

void CObsSeq::PrintHeader(ostream &outFile)
{
	mObsType->PrintHeader(outFile);
	outFile << "nbSequences= " << mNbSequences << endl;
}

//===============================================================================
//===============================================================================


