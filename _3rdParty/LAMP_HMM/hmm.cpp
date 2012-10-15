
/* ************************************************************************ *
 * ************************************************************************ *

   File: hmm.C
   The class CHMM defines operations for HMM

  * ************************************************************************ *

   Authors: Daniel DeMenthon & Marc Vuilleumier
   building up on original C code by Tapas Kanungo
   Date:  2-18-99 

 * ************************************************************************ *

   Modification Log:
 4-14-99: Compute log(A) and log(pi) in respective classes
 4-16-99: Compute ViterbiLog with state duration probabilities

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
#include "obsProb.h"
#include "discreteObsProb.h"
#include "gaussianObsProb.h"
#include "vectorObsProb.h"
#include "stateTrans.h"
#include "plainStateTrans.h"
#include "gammaProb.h"
#include "explicitDurationTrans.h"
#include "initStateProb.h"
#include "hmm.h"
using namespace std;

//===============================================================================
#ifndef DEBUG
#define DEBUG
#endif
//===============================================================================

//===============================================================================

CHMM::CHMM(CStateTrans *a, CObsProb *b, CInitStateProb *pi)
{
	mA = a;
	mB = b;
	mPi = pi;

	mN = mA->GetN();
}

//===============================================================================

CHMM::~CHMM(void)
{
	// Nothing for now
}

//===============================================================================

double CHMM::Forward(double **alpha, double *scale, CObs **obs, long T, boolean doLog)
// Scaling is used to prevent roundoff errors
// Same scaling is used for backward and forward procedures
// so that the scales cancel out in the Baum Welch formula
// Quantity returned is actually - log(P(O | model),
// i.e. exponential of this quantity is 1 / P(O | model)
{
	int	i, j; 	/* state indices */
	int	t;	/* time index */

	double sum;	/* partial sum */
	double logProb;
	double bi1, aij, bjt1;

	// 1. Initialization
	for (i = 1; i <= mN; i++) {
		bi1 = mB->at(i, obs[1]);
		alpha[1][i] = mPi->at(i) *  bi1;
	}
	scale[1] = Normalize(alpha[1], mN);

	// 2. Induction
	for (t = 1; t <= T - 1; t++) {
		for (j = 1; j <= mN; j++) {
			sum = 0.0;
			for (i = 1; i <= mN; i++){
				aij = mA->at(i, j);
				sum += alpha[t][i] * aij;
			}
			bjt1 =  mB->at(j, obs[t+1]);
			alpha[t+1][j] = sum * bjt1;
		}
		scale[t+1] = Normalize(alpha[t+1], mN);
	}
	logProb = 0.0;

	// 3. Termination
	if(doLog){
		for (t = 1; t <= T; t++){
			logProb += log(scale[t]);
		}
	}// endif

	return logProb;// zero returned if doLog is false
}

//===============================================================================

void CHMM::Backward(double **beta, double *scale, CObs **obs, long T)
{
	int     i, j;   /* state indices */
	int     t;      /* time index */
	double sum;


	// 1. Initialization
	for (i = 1; i <= mN; i++){
		beta[T][i] = 1.0/scale[T];
	}

	// 2. Induction
	for (t = T - 1; t >= 1; t--){
		for (i = 1; i <= mN; i++){
			sum = 0.0;
			for (j = 1; j <= mN; j++){
				sum += mA->at(i,j) * mB->at(j, obs[t+1]) * beta[t+1][j];
			}
			beta[t][i] = sum/scale[t];
	    }
	}
}

//===============================================================================

double CHMM::Viterbi(CObs **obs, long T, int *q)
// returns sequence q of most probable states
// and probability of seeing that sequence
// if A, B, pi are already given
{
	int 	i, j;	// state indices
	int  	t;	// time index

	int	argmaxval;
	double	maxval, val;
	double pprob;
	double *prevDelta, *delta, *tmp;
	int **psi;
	
	delta = SetVector(mN);
	prevDelta = SetVector(mN);
	psi = SetIntMatrix(T, mN);

	//1. Initialization
	for (i = 1; i <= mN; i++) {
		prevDelta[i] = mPi->at(i) * mB->at(i, obs[1]);
		psi[1][i] = 0;
	}	

	// 2. Recursion
	for (t = 2; t <= T; t++) {
		for (j = 1; j <= mN; j++) {
			maxval = 0.0;
			argmaxval = 1;	
			for (i = 1; i <= mN; i++) {
				val = prevDelta[i] * mA->at(i, j);
				if (val > maxval) {
					maxval = val;	
					argmaxval = i;	
				}
			}
			delta[j] = maxval * mB->at(j, obs[t]);
			psi[t][j] = argmaxval; // reverse order of indices?
		}
		tmp = delta; delta = prevDelta; prevDelta = tmp;
	}

	// 3. Termination
	pprob = 0.0;
	q[T] = 1;
	for (i = 1; i <= mN; i++) {
		if (prevDelta[i] > pprob) {
			pprob = prevDelta[i];	
			q[T] = i;
		}
	}

	// 4. Path (state sequence) backtracking

	for (t = T - 1; t >= 1; t--){
		q[t] = psi[t+1][q[t+1]];
	}
	delete [] psi[1];
	delete [] psi;
	delete [] prevDelta;
	delete delta;
	
	return pprob;
}

//===============================================================================

double CHMM::ViterbiLog(CObs **obs, long T, int *q)
// returns sequence q of most probable states
// and probability of seeing that sequence
// if A, B, pi are already given
// This implementation uses logarithms to avoid underflows.
{

	int 	i, j;	// state indices
	int  	t;	// time index

	int	argmaxval;
	double	maxval, val, bVal, logVal;
	double logProb;
	double *prevDelta, *delta, *tmp;
	int **psi;
	double **logBiOt;
	int zeroProbCount;

	delta = SetVector(mN);
	prevDelta = SetVector(mN);
	psi = SetIntMatrix(T, mN);

	// We do not preprocess the logs for B
	// because the data have variable length T
	logBiOt =  SetMatrix(mN, T);
	for (t = 1; t <= T; t++){
		zeroProbCount = 0;
		for (i = 1; i <= mN; i++){
			bVal = mB->at(i, obs[t]);
			if(bVal<=0.0){
				logVal = -1000.0;
				zeroProbCount++;
			}
			else{
				logVal = log(bVal);
			}
			logBiOt[i][t] = logVal;
		}// for i
		if(zeroProbCount == mN){// unseen obs, Viterbi decides which seen obs to use
			cerr << "*** Unseen data, renormalizing logBiOt to equiprobs ***"<<endl;
			for (i = 1; i <= mN; i++){
				logBiOt[i][t] = log(1.0/mN);
			}
		}
 	}// for t

	// 1. Initialization
	for (i = 1; i <= mN; i++){
		//		prevDelta[i] = logPi[i] + logBiOt[i][1];
		prevDelta[i] = mPi->logAt(i) + logBiOt[i][1];
		psi[1][i] = 0;
		mA->InitViterbiDurations(i);
	}

	// 2. Recursion
	for (t = 2; t <= T; t++) {
		for (j = 1; j <= mN; j++) {
			//		        maxval = prevDelta[1] + logA[1][j];
			maxval = prevDelta[1] + mA->logAt(1, j);
			argmaxval = 1;
			for (i = 1; i <= mN; i++) {// previous state
				//			  val = prevDelta[i] + logA[i][j];
				val = prevDelta[i] + mA->logAt(i, j);
				if (val > maxval) {
					maxval = val;
					argmaxval = i;
				}
			}
			//			delta[j] = maxval + logBiOt[j][t];
			delta[j] = maxval + logBiOt[j][t]; 
			psi[t][j] = argmaxval;
			mA->UpdateViterbiDurations(argmaxval, j);// ***dfd 4-16-99
		}
		tmp = delta; delta = prevDelta; prevDelta = tmp;
	}

	// 3. Termination
	logProb = prevDelta[1];
	q[T] = 1;
	for (i = 1; i <= mN; i++) {
		if (prevDelta[i] > logProb) {
			logProb = prevDelta[i];
			q[T] = i;
		}
	} 

	// 4. Path (state sequence) backtracking
	for (t = T - 1; t >= 1; t--){
		q[t] = psi[t+1][q[t+1]];
	}
	
	delete [] psi[1];
	delete [] psi;
	delete [] prevDelta;
	delete [] delta;
	delete [] logBiOt[1];
	delete [] logBiOt;
#if 0
	delete [] logA[1];
	delete [] logA;
	delete [] logPi;
#endif	
	return logProb;
}

//===============================================================================

double CHMM::BaumWelchCore(CObs **obs, long T, double *gamma, double **xi,
						   boolean doLog)
// Operations on a single observation sequence in the Baum-Welch loop are grouped here
{
	int	i, j, t;
	double logProb;
	double bjBeta;
	double *scale;
	double **alpha, **beta;

	alpha = SetMatrix(T, mN);// different size every time
	beta = SetMatrix(T, mN);

	scale = SetVector(T);

	logProb = Forward(alpha, scale, obs, T, doLog);
	Backward(beta, scale, obs, T);

	for (t = 1; t <= T - 1; t++){
		for (j = 1; j <= mN; j++) {
			gamma[j] = alpha[t][j] * beta[t][j];
			bjBeta =  beta[t+1][j] * mB->at(j, obs[t+1]);
			for (i = 1; i <= mN; i++){
				xi[i][j] = alpha[t][i] *  mA->at(i, j) * bjBeta;
			}
		}//end for j
		Normalize(xi, mN, mN);
		Normalize(gamma, mN);

		mA->BWSum(xi);
		mB->BWSum(gamma, obs[t]);

		if(t==1){
			mPi->BWSum(gamma);
		}
	}// end for t

	// Step for t = T
	for (j = 1; j <= mN; j++) {
		gamma[j] = alpha[T][j] * beta[T][j];
	}
	Normalize(gamma, mN);
	mB->BWSum(gamma, obs[T]);

	delete [] alpha[1];
	delete [] alpha;
	delete [] beta[1];
	delete [] beta;
	delete [] scale;

	return logProb;
}

//===============================================================================

double CHMM::IterBaumWelch(CObsSeq *obsSeq, double *gamma, double **xi)
{
	double deltaAB, deltaA, deltaB, deltaPi, delta;
	double logProb;
	int i;
	const boolean NOLOG = FALSE;

	mA->StartIter();// Zero sums used to cumulate results from each sequence
	mB->StartIter();
	mPi->StartIter();

	for(i=1;i<=obsSeq->mNbSequences;i++){ // Loop over observation files:

	    logProb = BaumWelchCore(obsSeq->mObs[i], obsSeq->mNbObs[i], gamma, xi, NOLOG);

	}// end loop over observation files

	deltaA = mA->EndIter();
	deltaB = mB->EndIter();
	deltaPi = mPi->EndIter();

	deltaAB = deltaA > deltaB ? deltaA : deltaB;
	delta = deltaAB > deltaPi ? deltaAB : deltaPi;

	return delta;
}

//===============================================================================

void CHMM::LearnBaumWelch(CObsSeq *obsSeq)
// Follows (6.110) p. 369 of Rabiner-Huang
// The Baum-Welch loop is done over all the sequences
{
	int  iCount = 0;
	double delta;

	double *gamma = SetVector(mN);
	double **xi = SetMatrix(mN, mN);
	
	mA->Start();// Allocate sums used to cumulate results from each sequence
	mB->Start();
	mPi->Start();

	// Baum-Welch loop starts here
	do  {	
		delta = IterBaumWelch(obsSeq, gamma, xi);

		iCount++;
		cout<< endl << "BW iteration no. "<< iCount << endl;
		cout << "delta = " << delta <<endl<<endl;

	}
	while(delta > DELTA);

	mPi->End();
	mB->End();
	mA->End();

	cout << endl << "num iterations " << iCount << endl;
	//	cout << "logTotalProb: " << sumProbf << endl;
	
	delete [] xi[1];
	delete [] xi;
	delete [] gamma;
}

//===============================================================================

double CHMM::SegmentalKMeansCore(CObs **obs, long T)
// Operations on a single observation sequence in the segmental K-means loop
{
	int	t;
	int thisQ, nextQ;
	double logProb;
	int *q;

	q = SetIntVector(T);// best state sequence

	logProb = ViterbiLog(obs, T, q);

	mPi->SKMSum(q[1]);

	nextQ = q[1];
	for (t = 1; t <= T - 1; t++){
		thisQ = nextQ;
		nextQ = q[t+1];
		mA->SKMSum(thisQ, nextQ);
		mB->SKMSum(thisQ, obs[t]);
	}// end for t

	// Step for t = T
	mB->SKMSum(nextQ, obs[T]);

	delete [] q;
	return logProb;
}

//===============================================================================

double CHMM::IterSegmentalKMeans(CObsSeq *obsSeq)
{
	double deltaAB, deltaA, deltaB, deltaPi, delta;
	double logProb;
	int i;

	mA->StartIter();// Zero sums used to cumulate results from each sequence
	mB->StartIter();
	mPi->StartIter();

	for(i=1;i<=obsSeq->mNbSequences;i++){ // Loop over observation files:

	    logProb = SegmentalKMeansCore(obsSeq->mObs[i], obsSeq->mNbObs[i]);

	}// end loop over observation files

	deltaA = mA->EndIter();
	deltaB = mB->EndIter();
	deltaPi = mPi->EndIter();

	deltaAB = deltaA > deltaB ? deltaA : deltaB;
	delta = deltaAB > deltaPi ? deltaAB : deltaPi;

	return delta;
}

//===============================================================================

void CHMM::LearnSegmentalKMeans(CObsSeq *obsSeq)
// Follows (6.15.2) p. 383 of Rabiner-Huang
// The Segmental K-Means loop is done over all the sequences
{
	int  iCount = 0;
	double delta;

	mA->Start();// Allocate sums used to cumulate results from each sequence
	mB->Start();
	mPi->Start();

	// Baum-Welch loop starts here
	do  {	
		delta = IterSegmentalKMeans(obsSeq);

		iCount++;

		cout<< endl << "SKM iteration no. "<< iCount << endl;
		cout << "delta = " << delta <<endl<<endl;

	}
	while(delta > DELTA);

	mPi->End();
	mB->End();
	mA->End();

	cout << endl << "num iterations " << iCount << endl;
	//	cout << "logTotalProb: " << sumProbf << endl;
}

//===============================================================================

void CHMM::LearnHybridSKM_BW(CObsSeq *obsSeq)
// Follows (6.15.2) p. 383 of Rabiner-Huang
// Combine Segmental K-Means and Baum-Welch
{
	int  iCount = 0;
	double delta;
	double *gamma = SetVector(mN);
	double **xi = SetMatrix(mN, mN);

	mA->Start();// Allocate sums used to cumulate results from each sequence
	mB->Start();
	mPi->Start();

	for(int i=0;i<10;i++){
		delta = IterBaumWelch(obsSeq, gamma, xi);
		iCount++;
		cout<< endl << "BW iteration no. "<< iCount << endl;
		cout << "delta = " << delta <<endl<<endl;
	}
	// Baum-Welch loop starts here
	do  {	
		delta = IterSegmentalKMeans(obsSeq);

		iCount++;

		cout<< endl << "SKM iteration no. "<< iCount << endl;
		cout << "delta = " << delta <<endl<<endl;

	}
	while(delta > DELTA);

	mPi->End();
	mB->End();
	mA->End();

	cout << endl << "num iterations " << iCount << endl;
	//	cout << "logTotalProb: " << sumProbf << endl;
}

//===============================================================================


CObsSeq* CHMM::GenerateSequences(long nbSequences, long nbObs, int seed)
{
	int i;
	int anyState = 1;
	CObs* obsType = mB->PickObservation(anyState);// to pass observation type
	CObsSeq* obsSeq = new CObsSeq(obsType, nbSequences, nbObs);

	MyInitRand(seed);

	for(i=1;i<=nbSequences;i++){
		obsSeq->mObs[i] = GenerateObservations(nbObs);
	}
	return obsSeq;
}

//===============================================================================


CObs** CHMM::GenerateObservations(long T)
// Generate an observation sequence of length T using A, B and Pi
{
	int t = 1;
	int currentState;
	CObs** obs;

	obs = new CObs*[T+1];
	assert(obs != NULL);

	currentState = mPi->PickInitialState();
	mA->InitDuration(currentState);
	obs[1] = mB->PickObservation(currentState);

	for (t = 2; t <= T; t++) {
		currentState =  mA->PickNextState(currentState);
		obs[t] =  mB->PickObservation(currentState);
	}
	return obs;
}

//===============================================================================

void CHMM::PrintStatesAndExpectedObs(CObsSeq *obsSeq,
									 ostream& stateFile, ostream& bestObsFile)
// Print sequences of most probable states 
// and sequences of expected observations 
// corresponding to those states
{
	long i, j, t, T, nbSequences;
	double logProb;
 	int *q;// most probable state sequence
	CObs **stateToObsMap;// list of expected observations for each state
	CObs *expectedObs;

	obsSeq->PrintHeader(stateFile);
	obsSeq->PrintHeader(bestObsFile);
	
	stateToObsMap = mB->MapStateToObs();// expected observation for each state

	for(i=1;i<=obsSeq->GetNbSequences();i++){ // Loop over observation files:
		T = obsSeq->mNbObs[i];
		stateFile <<"T= "<< T << endl;
		bestObsFile <<"T= "<< T << endl;
		
  		q = SetIntVector(T);// best state sequence
		logProb = ViterbiLog(obsSeq->mObs[i], T, q);

		for (t=1; t <= T; t++){
			stateFile << q[t] << " ";
			
			expectedObs = stateToObsMap[q[t]];
			expectedObs->Print(bestObsFile);
		}
		stateFile << endl;
		bestObsFile << endl;
		delete [] q;
	}
#if 1
	for (j=1; j<= mN; j++){
		delete stateToObsMap[j];
	}
#endif
	delete [] stateToObsMap;
}

//===============================================================================

#if 0

CObsSeq* CHMM::ReadSequences(ifstream &inputSeqFile)
// Read observation file into a data structure to make training faster
{
	long i, j, T, nbSequences;
	char magicID[32];
	CObs* obs;
	CObsSeq* obsSeq = new CObsSeq;

	mB->ReadFileHeader(inputSeqFile);// P5 or P6

	inputSeqFile >> magicID;
	assert(strcmp(magicID, "nbSequences=")==0);
	inputSeqFile >> nbSequences;
	obsSeq->mNbSequences = nbSequences;
	obsSeq->mObsCount = 0;
	obsSeq->mObs = new CObs**[nbSequences+1];
	obsSeq->mNbObs = new long[nbSequences+1];

	for(i=1;i<=nbSequences;i++){
		cout <<"Sequence "<< i <<endl;
		inputSeqFile >> magicID;
		assert(strcmp(magicID, "T=")==0);

		inputSeqFile >> T;// nb of observations for each sequence
		obsSeq->mNbObs[i] =  T;
		obsSeq->mObsCount += T;
		obsSeq->mObs[i] = new CObs*[T+1];// This array is from 1 to T
		assert(obsSeq->mObs[i] != NULL);

		for(j=1; j <=T; j++){
			obs = mB->ReadObsFrom(inputSeqFile);// obs type depends on type of mB
			obsSeq->mObs[i][j] = obs;
		}
	}

	return obsSeq;
}

#endif

//===============================================================================

double CHMM::FindDistance(CObsSeq *obsSeq, ostream &outFile)
// Find product of probabilities for each sequence
{
	long i, T, nbSequences;

	double logProb;
	double sumProb = 0.0;
	double *scale;
	double **alpha, **beta;
	const boolean YESLOG = TRUE;
	long stepCount = 0;
	double distance;


	nbSequences = obsSeq->mNbSequences;
	outFile << "nbSequences= " << nbSequences << endl;

	for(i=1;i<=nbSequences;i++){ // Loop over observation files:
		T = obsSeq->mNbObs[i];
		stepCount += T;

		alpha = SetMatrix(T, mN);// different size every time
		beta = SetMatrix(T, mN);

		scale = SetVector(T);

		logProb = Forward(alpha, scale, obsSeq->mObs[i], T, YESLOG);

		sumProb += logProb;

		//    cout<<i<<". T = "<<T<< ", logProb = " <<logProb<<", -prob/step = "<<-logProb/T << endl;
		outFile << -logProb/T << endl;
		cout << -logProb/T << endl;

		delete [] alpha[1];
		delete [] alpha;
		delete [] beta[1];
		delete [] beta;
		delete [] scale;
	}

	cout << endl << "-Log Prob of all " << nbSequences << " sequences: "<< -sumProb << endl;
	cout << "Nb of steps: " << stepCount << endl;

	distance = - sumProb/ stepCount; // average distance per step
	cout << "Average Forward distance: " << distance << endl;

#if 0
	double averageProb = exp(-distance);
	cout << endl << "Average Forward prob = "<< averageProb << endl;
#endif
	return distance;

}

//===============================================================================

double CHMM::FindViterbiDistance(CObsSeq *obsSeq, ostream &outFile)
// Find for each sequence log prob corresponding to best state segmentation
{
	long i, T, nbSequences;

	double logProb;
	double sumProb = 0.0;
	int *q;// most probable state sequence
	long stepCount = 0;
	double distance;


	nbSequences = obsSeq->mNbSequences;
	outFile << "nbSequences= " << nbSequences << endl;

	for(i=1;i<=nbSequences;i++){ // Loop over observation files:
		T = obsSeq->mNbObs[i];
		stepCount += T;

		q = SetIntVector(T);// best state sequence
		logProb = ViterbiLog(obsSeq->mObs[i], T, q);
		sumProb += logProb;

		outFile << -logProb/T << endl;
		cout << -logProb/T << endl;
		delete [] q;
	}

	outFile << endl << "-Log Prob of all " << nbSequences << " sequences: "<< -sumProb << endl;
	outFile << "Nb of steps: " << stepCount << endl;
	cout << endl << "-Log Prob of all " << nbSequences << " sequences: "<< -sumProb << endl;
	cout << "Nb of steps: " << stepCount << endl;

	distance = - sumProb/ stepCount; // average distance per step
	outFile << "Average Viterbi distance: " << distance << endl;
	cout << "Average Viterbi distance: " << distance << endl;

#if 0
	double averageProb = exp(-distance);
	cout << endl << "Average Viterbi prob = "<< averageProb << endl;
#endif
	return distance;

}

//===============================================================================

double CHMM::FindCrossEntropyDistance(CObsSeq *obsSeq, ostream &outFile)
// Find for each sequence log prob corresponding to best state segmentation
{
	long i, T, nbSequences;

	const double kWeight = 1.0;
	double logProb, weightedLogProb;
	double sumProb = 0.0;
	int *q;// most probable state sequence
	long stepCount = 0;
	double distance;
	double logQToPiProb;


	nbSequences = obsSeq->mNbSequences;
	outFile << "nbSequences= " << nbSequences << endl;

	for(i=1;i<=nbSequences;i++){ // Loop over observation files:
		T = obsSeq->mNbObs[i];
		stepCount += T;

		q = SetIntVector(T);// best state sequence
		logProb = ViterbiLog(obsSeq->mObs[i], T, q);
		logQToPiProb = FindQToPiProb(T, q);
		weightedLogProb = (kWeight*logProb + logQToPiProb)/(1.0+kWeight);
		sumProb += weightedLogProb;

		outFile <<-weightedLogProb/T << endl;
		cout <<-logProb/T << ", "<< -logQToPiProb/T << endl;
		delete [] q;
	}

	cout << endl << "-Log Prob of all " << nbSequences << " sequences: "<< -sumProb << endl;
	cout << "Nb of steps: " << stepCount << endl;

	distance = - sumProb/ stepCount; // average distance per step
	cout << "Average cross-entropy distance: " << distance << endl;

#if 0
	double averageProb = exp(-distance);
	cout << endl << "Average cross-entropy prob = "<< averageProb << endl;
#endif
	return distance;

}

//===============================================================================

double CHMM::FindQToPiProb(long T, int *q)
// Find probability of seeing a Pi distribution given that we have a Qi distribution
{
	int i, t;
	double *probQ;
	double piProb, qProb;
	double qToPiProb = 0.0;

	probQ = SetVector(mN);// prob distribution of observed Qs
	SetToZero(probQ, mN);

	for (t = 1; t <= T; t++){
		probQ[q[t]]++;
	}
	NonZeroNormalizeRow(probQ, mN);

	for(i=1;i<=mN;i++){
		piProb = mPi->at(i);
		qProb = probQ[i];
		qToPiProb += piProb * log(qProb/piProb);
	}
	qToPiProb *= T;// each number of observations is T * mPi->at(i)

	delete [] probQ;

	return qToPiProb;
}

//===============================================================================

void CHMM::Print(ostream &outFile)
{
	mA->Print(outFile);
	mB->Print(outFile);
	mPi->Print(outFile);
}

//===============================================================================

//===============================================================================
//===============================================================================

