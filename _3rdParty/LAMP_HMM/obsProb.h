#ifndef OBSPROB_H
#define OBSPROB_H
/* ************************************************************************ *
 * ************************************************************************ *

   File: obsProb.h
   The class CObsProb defines operations for the observation distribution B

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

class CObsProb{

  public:
   CObsProb(void){};
	virtual void Start(void) = 0;
	virtual void StartIter(void) = 0;
	virtual void BWSum(double *gamma, CObs *obs) = 0;
	virtual void SKMSum(int state, CObs *obs) = 0;
	virtual double EndIter() = 0;
	virtual void End() = 0;
	virtual CObs* PickObservation(int state) = 0;
	virtual void Print(std::ostream &outFile) = 0;
	virtual CObs** MapStateToObs(void) = 0;
	virtual double at(int state, CObs *obs) = 0;
//	virtual double logAt(int state, CObs *obs) = 0;
      
	int GetM(void){return mM;};
	int GetN(void){return mN;};
#if 0
	virtual CObs* ReadObsFrom(std::ifstream &inFile) = 0;
	virtual void ReadFileHeader(std::ifstream &inFile) = 0;
	virtual void PrintFileHeader(std::ostream &outFile) = 0;
#endif

  protected:
	int mM;// nb of symbols, assumed separated by an increment of 1
	int mN;// nb of states
};

//===============================================================================
//===============================================================================

//===============================================================================
#endif
