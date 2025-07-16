#ifndef DISCRETEOBSPROB_H
#define DISCRETEOBSPROB_H
/* ************************************************************************ *
 * ************************************************************************ *

   File: discreteObsProb.h
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

class CDiscreteObsProb: public CObsProb{

public:
	CDiscreteObsProb(int nbSymbols, int nbStates);
		CDiscreteObsProb(std::ifstream &hmmFile, int nbStates);
    ~CDiscreteObsProb(void);
	void Start(void);
	void StartIter(void);
	void BWSum(double *gamma, CObs *obs);
	void SKMSum(int state, CObs *obs);
	double EndIter();
	void End();
	CObs*  PickObservation(int state);
	uChar* FindAverageStateGray(void);
	CObs** MapStateToObs(void);
	void Print(std::ostream &outFile);
    inline double at(int state, CObs *obs){return mThisB[state][((CIntObs*)obs)->Get()];}
  //	inline double logAt(int state, CObs *obs){return mLogB[state][((CIntObs*)obs)->Get()];};
  //	int GetM(void){return mM;};// see below
#if 1
	void ReadFileHeader(std::ifstream &inFile);
	void PrintFileHeader(std::ostream &outFile);
	CObs* ReadObsFrom(std::ifstream &inFile);
#endif

private:

  //  int mM;// nb of observation symbols; not useful as each component may have a different nb
  //  int mN;// nb of states now defined in parent class

  double **mThisB;
  double **mNextB;
  double **mLogB;
  double **mSum;
  
};

//===============================================================================
//===============================================================================

//===============================================================================
#endif
