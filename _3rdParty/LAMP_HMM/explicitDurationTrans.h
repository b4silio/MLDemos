#ifndef EXPLICITDUATIONTRANS_H
#define EXPLICITDUATIONTRANS_H
/* ************************************************************************ *
 * ************************************************************************ *

   File: explicitDurationTrans.h
   The class CExplicitDurationTrans defines operations
   for the state transition matrix A
   when the probability of staying in each state is computed explicitely

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

class CExplicitDurationTrans: public CStateTrans{

public:

  CExplicitDurationTrans(int nbStates, int seed);
  CExplicitDurationTrans(std::ifstream &hmmFile, int nbStates);
  ~CExplicitDurationTrans(void);
  void Start(void);
  void StartIter(void);
  double EndIter();
  void End(void);
  void BWSum(double **xi){" *** use CPlainStateTrans for BW ***";};
  void SKMSum(int thisState, int nextState);
  void InitDuration(int thisState);
  void InitViterbiDurations(int thisState);
  void UpdateViterbiDurations(int thisState, int nextState);
  int PickNextState(int presentState);
  int PickNextState0(int presentState);
  void Print(std::ostream &outFile);
  inline double at(int i, int j){return mThisA[i][j];};
  double logAt(int i, int j);

  double DurationPenalty(int *q, int T);

private:

	int PickRandomState(double *rowA);

private:
  //  int mN;// nb of states defined in parent class

  double **mThisA;// A matrix
  double **mNextA;
  double **mLogA;
  double **mSum;
  
  CGammaProb *mGammaProb;// array of gamma distributions
  int *mViterbiDuration;
  int mDuration, mRandomGammaDuration;
  double *mRowA;
};

//===============================================================================
//===============================================================================

//===============================================================================
#endif
