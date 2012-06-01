#ifndef GAMMAPROB_H
#define GAMMAPROB_H
/* ************************************************************************ *
 * ************************************************************************ *

   File: gammaProb.C
   The class CGammaProb defines operations
   for a gamma distribution 
   and is used to compute state duration probabilities

  * ************************************************************************ *

   Authors: Daniel DeMenthon & Marc Vuilleumier
   Date:  4-18-99 

 * ************************************************************************ *

   Modification Log:
	

 * ************************************************************************ *
   Log for new ideas:
 * ************************************************************************ *
 					Copyright
               Language and Media Processing
               Center for Automation Research
               University of Maryland
               College Park, MD  20742
 * ************************************************************************ *
 * ************************************************************************ */
 

//===============================================================================
const double gCoef[6]={76.18009173,-86.50532033,24.01409822,
		-1.231739516,0.120858003e-2,-0.536382e-5};// used to compute Gamma function
				
//===============================================================================

class CGammaProb{

public:
	CGammaProb(int maxDuration);
	CGammaProb(void){};
	~CGammaProb(void);
	inline boolean IsDefined(void){return mIsDefined;};
	inline void IsDefined(boolean flag){mIsDefined = flag;};
	inline int GetDurationAtPeak(void){return mDurationAtPeak;};
	void ResetSums(void);
	void Read(std::ifstream &hmmFile);
	void Print(std::ostream &outFile);
	void FindParameters(void);
	void SKMSum(int duration);
	void ProcessSums(void);
        int FindMaxDuration(void);
	void FindDurationProbs(void);
	double LogGammaDensity(double x);
	double GammaDensity(double x);
	double FindRandomDuration(void);
	
	void SetMean(double mean){mMean = mean;};
	void SetVariance(double variance){mVariance = variance;};
	double GetMean(void){return mMean;};
	double GetVariance(void){return mVariance;};
	double GetStd(void){return mStd;};
	double GetStayProb(int duration);
	double GetLeaveProb(int duration);
	double GetLogStayProb(int duration);
	double GetLogLeaveProb(int duration);
	
private:
	void ReassignArrays(void);
	double LogGamma(double xx);
	double SampleInt(int a);
	double SampleFrac (double a);
	double SampleLarge (double a);
	double GammaFrac(double x);
	double GammaSeries(double x);
	double CumulDistribution(double x);
	
private:
  	boolean mIsDefined;
	long mDurationSquareSum;
	long mDurationSum;
	long mSumCount;
	double mMean;
	double mVariance, mStd;
	double mAlpha;
	double mP;
	double mConstant;// p*log(alpha)-LogGamma(p)
	int mDurationAtPeak;
  	int mSmallMaxDuration, mMaxDuration;// duration over which the density is zero
  	double *mLogStayProb, *mLogLeaveProb;
  	double *mStayProb, *mLeaveProb;
	double mLogPeak;
	int *mHistogram;// histogram of durations, for debugging
};

//===============================================================================
//===============================================================================

//===============================================================================
#endif
