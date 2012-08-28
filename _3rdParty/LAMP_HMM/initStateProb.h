#ifndef INITSTATEPROB_H
#define INITSTATEPROB_H
/* ************************************************************************ *
 * ************************************************************************ *

   File: initStateProb.h
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

class CInitStateProb{

public:
  CInitStateProb(int nbStates);
  //Addition of Simis for random and "first" initial state vectors
  CInitStateProb(int nbStates, int mode);
  CInitStateProb(std::ifstream &hmmFile, int nbStates);
  ~CInitStateProb(void);
  void Start(void);
  void StartIter(void);
  void BWSum(double *gamma);
  void SKMSum(int state);
  double EndIter(void);
  void End(void);
  int PickInitialState(void);
  void Print(std::ostream &outFile);
  inline double at(int i){return mThisPi[i];};
  inline double logAt(int i){return mLogPi[i];};

private:

  int mN;// nb of states

  double *mThisPi;
  double *mNextPi;
  double *mLogPi;
  double *mSum;
  
};

//===============================================================================
//===============================================================================

//===============================================================================
#endif
