#ifndef VECTOROBSPROB_H
#define VECTOROBSPROB_H
/* ************************************************************************ *
 * ************************************************************************ *

   File: vectorObsProb.h
   The class CVectorObsProb defines operations 
   for the observation distribution B
   when observations are vectors with independent components

  * ************************************************************************ *

  * ************************************************************************ *

   Authors: Daniel DeMenthon & Marc Vuilleumier
   Date:  2-27-99 

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


//===============================================================================
#include <iostream>

class CVectorObsProb: public CObsProb{
	
public:
	CVectorObsProb(int nbSymbols, int nbStates, int nbComponents, int isGaussian);
	CVectorObsProb(int *listNbSymbols, int nbStates, int nbComponents, int isGaussian);
	CVectorObsProb(int nbSymbols, int nbStates, int nbComponents, CObs **centroids, CObs **meanSquares);
	CVectorObsProb(std::ifstream &hmmFile, int nbStates, int nbComponents, int isGaussian);
	CVectorObsProb(void);
	~CVectorObsProb(void);
	void Start(void);
	void StartIter(void);
	void BWSum(double *gamma, CObs *obs);
	void SKMSum(int state, CObs *obs);
	double EndIter();
	void End();
	CObs* PickObservation(int state);
	CObs** MapStateToObs(void);
	void Print(std::ostream &outFile);
	double at(int state, CObs *obs);
	int GetM(void){std::cout<<"Wrong class used"<<std::endl; return 0;};
	CObsProb **GetComponents(){return mComponentProb;}
	int GetDimension(){return mDimension;}
#if 1
	CObs* ReadObsFrom(std::ifstream &inFile);
	void ReadFileHeader(std::ifstream &inFile);
	void PrintFileHeader(std::ostream &outFile);
#endif

private:
	void InitParameters(void);
	
private:
  	int mDimension;
	//	CGaussianObsProb **mComponentProb;// array of ptrs to gaussian probs, size = mDimension
	CObsProb **mComponentProb;// array of ptrs to discrete or gaussian probs, size = mDimension
};

//===============================================================================
//===============================================================================

//===============================================================================
#endif
