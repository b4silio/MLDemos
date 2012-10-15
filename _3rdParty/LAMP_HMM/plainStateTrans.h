#ifndef PLAINSTATETRANS_H
#define PLAINSTATETRANS_H
/* ************************************************************************ *
 * ************************************************************************ *

   File: plainStateTrans.h
   The class CPlainStateTrans defines operations
   for the transition matrix A with implicit duration probablilities

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

class CPlainStateTrans: public CStateTrans{

public:

  CPlainStateTrans(int nbStates, int seed);
  CPlainStateTrans(int nbStates, int seed, int mode);
  CPlainStateTrans(std::ifstream &hmmFile, int nbStates);
  ~CPlainStateTrans(void);
  void Start(void);
  void StartIter(void);
  double EndIter();
  void End(void);
  void BWSum(double **xi);
  void SKMSum(int thisState, int nextState);
  inline void InitDuration(int thisState){};// useful only in CExplicitDurationTrans
  inline void InitViterbiDurations(int thisState){};// useful only in CExplicitDurationTrans
  inline void UpdateViterbiDurations(int thisState, int nextState){};
  int PickNextState(int presentState);
  void Print(std::ostream &outFile);
  inline double at(int i, int j){return mThisA[i][j];};
  inline double logAt(int i, int j){return mLogA[i][j];};

private:

  //  int mN;// nb of states defined in parent class

  double **mThisA;// A matrix
  double **mNextA;
  double **mLogA;// log terms of A matrix
  double **mSum;
  
};

//===============================================================================
//===============================================================================

//===============================================================================
#endif
