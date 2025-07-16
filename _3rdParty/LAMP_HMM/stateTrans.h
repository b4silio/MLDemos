#ifndef STATETRANS_H
#define STATETRANS_H
/* ************************************************************************ *
 * ************************************************************************ *

   File: stateTrans.h
   The class CStateTrans defines operations
   for the transition matrix A

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

class CStateTrans{

public:
  virtual void Start(void)=0;
  virtual void StartIter(void)=0;
  virtual double EndIter()=0;
  virtual void End(void)=0;
  virtual void BWSum(double **)=0;
  virtual void SKMSum(int, int)=0;
  virtual void SKMSum(int, int, int){};// used for 3D state transitions
  virtual void InitDuration(int thisState)=0;
  virtual void InitViterbiDurations(int thisState)=0;
  virtual void UpdateViterbiDurations(int thisState, int nextState)=0;
  virtual int PickNextState(int)=0;
//  virtual int PickNextState(int, int){};// used for 3D state transitions
//virtual int PickNextState(int, int)=0;// used for 3D state transitions
  virtual void Print(std::ostream &)=0;
  virtual double at(int , int)=0;
  virtual double logAt(int , int)=0;
//  virtual double logAt(int , int, int){};// used for 3D state transitions
//virtual double logAt(int , int, int)=0;// used for 3D state transitions

  int GetN(void){return mN;};

protected:

  int mN;// nb of states
  
};

//===============================================================================
//===============================================================================

//===============================================================================
#endif
