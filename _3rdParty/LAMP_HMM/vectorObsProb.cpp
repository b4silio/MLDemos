/* ************************************************************************ *
 * ************************************************************************ *

   File: vectorObsProb.C
   The class CVectorObsProb defines operations 
   for the observation distribution B
   when observations are vectors with independent components

  * ************************************************************************ *

   Authors: Daniel DeMenthon & Marc Vuilleumier
   Date:  2-27-99 

 * ************************************************************************ *

   Modification Log:
   January 2003: Added a switch for eaither Gaussian or discrete pdfs of vector components
                 Also, discrete components read their nbSymbols from an array

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
#include "gaussianObsProb.h"
#include "vectorObsProb.h"
using namespace std;

//===============================================================================

CVectorObsProb::CVectorObsProb(int nbSymbols, int nbStates, int nbComponents, int isGaussian)
// Here a single nbSymbols is passed and used for all the components
{
	int i;
	
	mN = nbStates;
	mDimension = nbComponents;
//	mComponentProb = new CDiscreteObsProb*[mDimension+1];
	mComponentProb = new CObsProb*[mDimension+1];
//        CDiscreteObsProb* test;

	for(i=1;i<= mDimension; i++){
                if (isGaussian){
                    mComponentProb[i] = new CGaussianObsProb(nbSymbols, nbStates);
                }
                else{ // discrete (histograms)
                    mComponentProb[i] = new CDiscreteObsProb(nbSymbols, nbStates);
                }
	}
}

//===============================================================================

CVectorObsProb::CVectorObsProb(int *listNbSymbols, int nbStates, int nbComponents, int isGaussian)
// A list of nb of symbols is passed, defining how many are used for each component
{
	int i;
	
	mN = nbStates;
	mDimension = nbComponents;
//	mComponentProb = new CDiscreteObsProb*[mDimension+1];
	mComponentProb = new CObsProb*[mDimension+1];
//        CDiscreteObsProb* test;

	for(i=1;i<= mDimension; i++){
                if (isGaussian){
                    mComponentProb[i] = new CGaussianObsProb(listNbSymbols[i], nbStates);
                }
                else{ // discrete (histograms)
                    mComponentProb[i] = new CDiscreteObsProb(listNbSymbols[i], nbStates);
                }
	}
}

//===============================================================================

CVectorObsProb::CVectorObsProb(int nbSymbols, int nbStates, int nbComponents, 
			CObs **centroids, CObs **meanSquares)
// Constructor used for initializing Gaussian components
// here a single nbSymbols is passed and used for all the components
{
	int i, j;
	
	mN = nbStates;
//	mM = nbSymbols;
	mDimension = nbComponents;
//	mComponentProb = new CGaussianObsProb*[mDimension+1];
	mComponentProb = new CObsProb*[mDimension+1];
	double* means = SetVector(mN);
	double* stds = SetVector(mN);

	for(i=1;i<= mDimension; i++){
		for(j=1; j<= mN; j++){
			means[j] = ((CVectorObs*)centroids[j])->Get(i);
			stds[j] = sqrt(((CVectorObs*)meanSquares[j])->Get(i));
		}
		mComponentProb[i] = new CGaussianObsProb(nbSymbols, nbStates, means, stds);
	}
	delete [] means;
	delete [] stds;
}

//===============================================================================

CVectorObsProb::CVectorObsProb(ifstream &hmmFile, int nbStates, int nbComponents, int isGaussian)
// Constructor used to read an hmm from a file
{
	int i;

	mN = nbStates;
	mDimension = nbComponents;
//	mComponentProb = new CGaussianObsProb*[mDimension+1];
	mComponentProb = new CObsProb*[mDimension+1];

	for(i=1;i<= mDimension; i++){
            if (isGaussian){
		mComponentProb[i] = new CGaussianObsProb(hmmFile, nbStates);
            }
            else{
		mComponentProb[i] = new CDiscreteObsProb(hmmFile, nbStates);
            }
	}
//	mM = mComponentProb[1]->GetM();
}

//===============================================================================

CVectorObsProb::~CVectorObsProb(void)
{
	int i;
	
	for(i=1;i<= mDimension; i++){
		delete mComponentProb[i];
	}
	delete [] mComponentProb;
}

//===============================================================================

double CVectorObsProb::at(int state, CObs *obs)
// return prob of seeing this obs vector in this state
// equal to the product of the probs of seeing the components of the obs vector in state
{
	int i;
	int val;
	CIntObs *intObs = new CIntObs;
	double prob = 1.0;
	double componentProb;
	
	for(i=1;i<= mDimension; i++){
		val = (int)(((CVectorObs*)obs)->Get(i)); // component of obs
		intObs->Set(val);
		componentProb = mComponentProb[i]->at(state, intObs);// prob of component
		prob *= componentProb;// multiply individual probabilities
	}

	delete intObs;
	return prob;
}

//===============================================================================

void CVectorObsProb::Start(void)
{
	int i;
	
	for(i=1;i<= mDimension; i++){
		mComponentProb[i]->Start();
	}
}

//===============================================================================

void CVectorObsProb::StartIter(void)
{
	int i;
	for(i=1;i<= mDimension; i++){
		mComponentProb[i]->StartIter();
	}
}

//===============================================================================

void CVectorObsProb::BWSum(double *gamma, CObs *obs)
{
	int i;
	int val;
	CIntObs *intObs = new CIntObs;
	
	for(i=1; i<=mDimension; i++){
		val = (int)(((CVectorObs*)obs)->Get(i)); // component i of obs vector
		intObs->Set(val);
		mComponentProb[i]->BWSum(gamma, intObs);
	}
	delete intObs;
}

//===============================================================================

void CVectorObsProb::SKMSum(int state, CObs *obs)
{
	int i;
	int val;
	CIntObs *intObs = new CIntObs;
	
	for(i=1;i<= mDimension; i++){
		val = (int)(((CVectorObs*)obs)->Get(i)); // component of obs vector
		intObs->Set(val);
		mComponentProb[i]->SKMSum(state, intObs);
	}
	delete intObs;
}

//===============================================================================

double CVectorObsProb::EndIter()
// Finish Baum-Welch iteration; return magnitude of largest change
{
	int i;
	double maxDiff = 0.0;
	double componentMaxDiff = 0.0;
	
	for(i=1;i<= mDimension; i++){
		componentMaxDiff = mComponentProb[i]->EndIter();
		if(componentMaxDiff > maxDiff) maxDiff = componentMaxDiff;
	}
   return maxDiff;
}

//===============================================================================

void CVectorObsProb::End()
// Finish Baum-Welch session
{
	int i;
	for(i=1;i<= mDimension; i++){
		mComponentProb[i]->End();
	}
}

//===============================================================================

CObs* CVectorObsProb::PickObservation(int state)
// Return a random observation given a state using the observation prob. matrix B
{
	int i;
	CIntObs *intObs;
	CVectorObs *vectObs = new CVectorObs(mDimension);

	for(i=1;i<= mDimension; i++){
		intObs = (CIntObs*)(mComponentProb[i]->PickObservation(state));
		vectObs->Set(intObs->Get(), i);
	}
	return vectObs;
}

//===============================================================================

CObs** CVectorObsProb::MapStateToObs(void)
// return array of expected observation vectors for all states
{
        int i, j;
	CIntObs ***expectedComponentObs;
	CVectorObs **expectedObs;
	CVectorObs *vectorObs;
	int obsVal;

	expectedComponentObs = new CIntObs**[mN+1];// array of maps 
	expectedObs = new CVectorObs*[mN+1];

	for(i=1;i<= mDimension; i++){
	  expectedComponentObs[i] =  (CIntObs**)mComponentProb[i]->MapStateToObs();
	}  

	for (j=1; j <= mN; j++){// states
	  expectedObs[j] = new CVectorObs(mDimension);
	  vectorObs = expectedObs[j];
	  for(i=1;i<= mDimension; i++){
	         obsVal = expectedComponentObs[i][j]->Get();
		 vectorObs->Set(obsVal, i);
	  }
	}
	delete [] expectedComponentObs[1];
	delete [] expectedComponentObs;

	return (CObs**)expectedObs;
}

//===============================================================================

void CVectorObsProb::Print(ostream &outFile)
{
	int i;

	for(i=1;i<= mDimension; i++){
		mComponentProb[i]->Print(outFile);
	}
}

//===============================================================================

CObs* CVectorObsProb::ReadObsFrom(ifstream &inFile)
{
	CVectorObs *obs = new CVectorObs(mDimension);
	obs->ReadFrom(inFile);

	return obs;
}

//===============================================================================

#if 0

void CVectorObsProb::ReadFileHeader(ifstream &inFile)
{
  CVectorObs::ReadHeader(inFile);
}

//===============================================================================

void CVectorObsProb::PrintFileHeader(ostream &outFile)
{
  CVectorObs::PrintHeader(outFile);
}

//===============================================================================
#endif
//===============================================================================
//===============================================================================
