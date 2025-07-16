/* ************************************************************************ *
 * ************************************************************************ *

   File: gammaProb.C
   The class CGammaProb defines operations
   for a gamma distribution

  * ************************************************************************ *

   Authors: Daniel DeMenthon & Marc Vuilleumier
   Date:  4-18-99 

 * ************************************************************************ *

   Modification Log:
		5-30-99: 
		Added mStd, mMaxDuration;
		Incomplete Gamma function used to compute prob. of staying in state
		or leaving state. 
		Precompute probs. of leaving and staying
		6-8-99: Pass maxDuration, typically size of image to use as array size
		Also, limit the max of value computed by Newton-Raphson to maxDuration
		6-21-99:
		return mMaxDuration if divisions by zero attempted in Newton/Raphson

 * ************************************************************************ *
   Log for new ideas:
 * ************************************************************************ *
 					Copyright 1999
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
#include "gammaProb.h"
using namespace std;

//===============================================================================

#define GSL_PI 3.14159265358979323844
#define GSL_E  2.71828182845904523536
#define GSL_LOGINFINITY 300.0

//===============================================================================

CGammaProb::CGammaProb(int maxDuration)
{
        mMaxDuration = maxDuration;
	mSmallMaxDuration = mMaxDuration;
	mHistogram = new int[mMaxDuration];
	assert(mHistogram != NULL);
	for(int i=0;i<mMaxDuration;i++){
		mHistogram[i] = 0;
	}
	mStayProb = SetVector(mSmallMaxDuration);
	mLeaveProb = SetVector(mSmallMaxDuration);
	mLogStayProb = SetVector(mSmallMaxDuration);
	mLogLeaveProb = SetVector(mSmallMaxDuration);
}

//===============================================================================

CGammaProb::~CGammaProb(void)
{
	delete [] mHistogram;
	delete [] mStayProb;
	delete [] mLeaveProb;
	delete [] mLogStayProb;
	delete [] mLogLeaveProb;
}

//===============================================================================

void CGammaProb::ResetSums(void)
{
	mDurationSum = 0;
	mDurationSquareSum = 0;
	mSumCount = 0;
//   mIsDefined = false; do not reset here
	for(int i=0;i<mMaxDuration;i++) mHistogram[i] = 0;
}

//===============================================================================

void CGammaProb::SKMSum(int duration)
{
	mDurationSum += duration;
	mDurationSquareSum += duration * duration;
	mSumCount++;
#if 1 // for debugging only
	if(duration<mMaxDuration){
		mHistogram[duration]++;
	}
#endif
}

//===============================================================================

void CGammaProb::Read(ifstream &hmmFile)
{
	  mIsDefined = false;
	  hmmFile >> mMean;
	  hmmFile >> mStd; 
	  mVariance = mStd * mStd;
	  
	  FindParameters();

	  Print(cerr);
	  if(mIsDefined == false){
	    cerr << "Gamma not defined"<< endl;
	  }
}

//===============================================================================


void CGammaProb::Print(ostream &outFile)
{
	  outFile << mMean << " " << mStd;
#if 0
	  if(!mIsDefined) outFile <<" * ";// debug
#endif
	  outFile << endl;
}

//===============================================================================

void CGammaProb::ProcessSums(void)
{
	mIsDefined = false;// may be switched to true in FindParameters
	if(mSumCount <2){
		mMean = 0;
		mVariance = 0;
		mStd = 0;// defined for printing
		return;
	}
	mMean = double(mDurationSum)/mSumCount;
	mVariance = double(mDurationSquareSum)/mSumCount - mMean * mMean;
	mStd = sqrt(mVariance);
	
	FindParameters();
}

//===============================================================================

void CGammaProb::FindParameters(void)
{
	mIsDefined = false;// will be switched if tests are passed
	if(mMean <= 0.0 || mVariance <= 0.0){
		return;
	}
	mAlpha = mMean/mVariance;
	mP = mMean * mMean / mVariance;
#if 0
	if(mAlpha<=1 || mP<0){
#else // why limit model to alpha >1? mAlpha is scaling of gamma density
	if(mP<=0){
#endif
		return;
	}
	mConstant = mP * log(mAlpha) - LogGamma(mP);// to speed up distribution estimate
	mDurationAtPeak = (int)floor((mP - 1)/mAlpha);
#ifdef BURSHTEIN
	if(mDurationAtPeak<=0){// we can't calculate the log, so mIsDefined = false
		return;
	}
	mLogPeak = LogGammaDensity(mDurationAtPeak);
#endif
	mSmallMaxDuration = FindMaxDuration();
	ReassignArrays();
	
	mIsDefined = true;
	FindDurationProbs();
}

//===============================================================================

void CGammaProb::ReassignArrays(void)
{
		delete [] mStayProb;
		delete [] mLeaveProb;
		delete [] mLogStayProb;
		delete [] mLogLeaveProb;
		mStayProb = SetVector(mSmallMaxDuration);
		mLeaveProb = SetVector(mSmallMaxDuration);
		mLogStayProb = SetVector(mSmallMaxDuration);
		mLogLeaveProb = SetVector(mSmallMaxDuration);
}

//===============================================================================

int CGammaProb::FindMaxDuration(void)
// Rightmost non-zero density position
// Use 5 Newton-Raphson iterations to find the 1.0e-8 point
  // Hopefully the result is less than mMaxDuration (image size)
{
   double f, df, x;
   int i;
   int maxDur;

   if(mP>1){// peak exists
     x = mDurationAtPeak + mStd;
   }
   else{
     x = 1;
   }

   for(i=1;i<=5;i++){
	if(x<=0) return mMaxDuration;// so we don't take the log
        f = - mAlpha * x + (mP - 1) * log(x) + 18.42 + mConstant;// - 18.42 is log(1.0e-8)
	if(x == 0.0) return mMaxDuration;
	df = - mAlpha + (mP - 1) / x;
	x = x - f / df;
   }
   maxDur = int(1 + floor(x));
   if(maxDur>mMaxDuration || maxDur<2){
		cerr<< "Max duration out of range: " << maxDur << endl;
		return mMaxDuration;
   }
   return maxDur;
}

//===============================================================================

#ifdef BURSHTEIN

void CGammaProb::FindDurationProbs(void)
// Compute duration penalties to correct the Viterbi log prob
// using David Burshtein's paper
// " Robust Parametric Modeling of Durations in HMMs"
// Don't forget subtracting log(1-aii) when using mLogLeaveProb
{
	int i;
	double dxpeak, dx, prevdx, pstay, pleave;
	double stayProb, leaveProb;
	dxpeak = exp(mLogPeak);
	prevdx = GammaDensity(1);
	for(i=1;i<=mSmallMaxDuration;i++){
		dx = GammaDensity(i+1);
		if(i<mDurationAtPeak){
			pstay = 1.0;// prob of staying is 1
			pleave = prevdx;
		}
		else{// duration>=durationAtPeak
			pstay = dx / prevdx;// Marc had it the other way around
			pleave = dxpeak;
		}
		stayProb = pstay / (pstay + pleave);
		leaveProb = pleave / (pstay + pleave);
		mStayProb[i] = stayProb;
		mLeaveProb[i] = leaveProb;
		assert(stayProb>0);
		assert(leaveProb>0);
		mLogStayProb[i] = log(stayProb);
		mLogLeaveProb[i] = log(leaveProb);
		prevdx = dx;
	}
}

#endif

//===============================================================================

void CGammaProb::FindDurationProbs(void)
// Find probs of staying in and leaving from a state
// using Marc Vuilleumier's incomplete Gamma technique
{
	int i;
 	double dx, prevdx, denom, stayProb, leaveProb;
	
	prevdx = 0;
	for(i=1;i<=mSmallMaxDuration;i++){
		dx = CumulDistribution(i);
#if 0 // debugging
	double a, b, c, sum;// debugging
		a = dx - prevdx;
		b = GammaDensity(i);
		if(i>1){
		  c = GammaDensity(i-1);
		}
#endif
		denom = 1-prevdx;
		if(denom>0){	   	
			stayProb = (1-dx) / (1-prevdx);
			leaveProb = (dx - prevdx) / (1-prevdx);
			mStayProb[i] = stayProb;
			mLeaveProb[i] = leaveProb;
			assert(stayProb>=0);
			assert(leaveProb>=0);
			if(stayProb==0){
			  mLogStayProb[i] = -1000.0;
			}else{
			  mLogStayProb[i] = log(stayProb);
			}
			if(leaveProb==0){
			  mLogLeaveProb[i] = -1000.0;
			}
			else{
			  mLogLeaveProb[i] = log(leaveProb);
			}
// 			sum = exp(mLogStayProb[i]) + exp(mLogLeaveProb[i]);
// 			if(fabs(sum - 1.0)> 0.001){
// 			  cout << i << " " << sum << endl;
// 			}
		}
		else{// denom = 0
			mStayProb[i] = 1.0;
			mLeaveProb[i] = 0.0;

			mLogStayProb[i] = 0.0;// ln(1)
			mLogLeaveProb[i] = -1000.0;// len(0)
		}
		prevdx = dx;
	}
}

//===============================================================================

double CGammaProb::GetStayProb(int duration)
{
  if(duration>= mSmallMaxDuration){
    return 0.0;// prob. of staying is zero
  }
//  else{
    return mStayProb[duration];
}

//===============================================================================

double CGammaProb::GetLeaveProb(int duration)
{
  if(duration>= mSmallMaxDuration){
     return 1.0;// prob. of leaving is 1
  }
//  else{
     return mLeaveProb[duration];
}

//===============================================================================

double CGammaProb::GetLogStayProb(int duration)
{
  if(duration>= mSmallMaxDuration){
    return -1000.0;// prob. of staying is zero, log is -INF
  }
//  else{
    return mLogStayProb[duration];
}

//===============================================================================

double CGammaProb::GetLogLeaveProb(int duration)
{
  if(duration>= mSmallMaxDuration){
     return 0.0;// prob. of leaving is 1, log is zero
  }
//  else{
     return mLogLeaveProb[duration];
}

//===============================================================================

double CGammaProb::LogGamma(double xx)
// Log of Gamma function, from Numerical Recipes
{
	double x, y, tmp, ser;
	int j;

	y=x=xx;
	tmp=x+5.5;
	tmp -= (x+0.5)*log(tmp);
	ser=1.000000000190015;
	for (j=0;j<=5;j++) {
		ser += gCoef[j]/++y;
	}
	return -tmp+log(2.5066282746310005*ser/x);
}

//===============================================================================

double CGammaProb::LogGammaDensity(double x)
// Log of gamma density
{
	double logGammaProb;

	assert(x>0);
	logGammaProb = mConstant - mAlpha * x + (mP-1) * log(x);

	return logGammaProb;
}

//===============================================================================

double CGammaProb::GammaDensity(double x)
// Log of gamma density
{
	double density = exp(LogGammaDensity(x));
	if(density < 1.0e-100) density = 1.0e-100;
	return density;
}

//===============================================================================

double CGammaProb::FindRandomDuration(void)
// Gamma deviate from gsl library
// mAlpha is just a scaling factor, 
// as can be seen by comparing gsl's and Burshtein's expressions
// for the Gamma probability distribution
{
	int na;
	double a = mP;// map to notations of gsl
	na = int(floor (a));
	double randomGamma, scaledRandomGamma;
	if (a == na){
		randomGamma = SampleInt(na);
	}
	else if (na == 0){
		randomGamma = SampleFrac(a);
	}
	else{
		randomGamma = SampleInt(na) + SampleFrac(a-na);
	}
	scaledRandomGamma = randomGamma/mAlpha;
	
	return scaledRandomGamma;
}

//===============================================================================

double CGammaProb::SampleInt(int a)
{
	if (a < 12){
		int i;
		double prod = 1.0;
		for (i = 0; i < a; ++i){
			prod *= LocalRand();
		}
		if (prod == 0){
			return GSL_LOGINFINITY;
		}
		else{
			return -log (prod);
		}
	}
	else{
		return SampleLarge ((double) a);
	}
}

//===============================================================================

double CGammaProb::SampleFrac(double a)
// This is exercise 16 from Knuth; see page 135,
// and the solution is on page 551.
{
	double p, q, x, u, v;
	
	p = GSL_E / (a + GSL_E);
	do{
		u = LocalRand();
		do{
			v = LocalRand();
		}while (v == 0);
		if (u < p){
			x = exp ((1 / a) * log (v));
			q = exp (-x);
		}
		else{
			x = 1 - log (v);
			q = exp ((a - 1) * log (x));
		}
	}while (LocalRand() >= q);
	return x;
}

//===============================================================================

double CGammaProb::SampleLarge(double a)
// Works only if a>1, and is most efficient if a is large
// This algorithm, reported in Knuth, is attributed to Ahrens.  
// A faster one, we are told, can be found in: 
// J. H. Ahrens and U. Dieter, Computing 12 (1974) 223-246.
{

	double sqa, x, y, v;
	sqa = sqrt (2 * a - 1);
	do{
		do{
			y = tan (GSL_PI * LocalRand());
			x = sqa * y + a - 1;
		}while (x <= 0);
		v = LocalRand();
	}while (v>(1+y*y)*exp((a-1)*log(x/(a-1))-sqa*y));

	return x;
}

//===============================================================================

double CGammaProb::GammaFrac(double x)
// Compute the incomplete gamma using a continuous fraction"
// From Press et al. p. 219"
{
	double gamFrac;
	double fpmin, an, b, c, d, delta, h;
	fpmin = 1.0e-30;
	b = mAlpha * x + 1.0 - mP;
	c = 1.0 / fpmin;
	d = 1.0 / b;
	h = d;
	int i;
	for(i=1; i<=1000; i++){// maximal number of iterations
		an = i * (mP - i);
		b = b + 2.0;
		d = an * d + b;
		if(fabs(d) < fpmin) d = fpmin;
		c = an / c + b;
		if(fabs(c) < fpmin) c = fpmin;
		d = 1.0 / d;
		delta = d * c;
		h = h * delta;
		if(fabs(delta - 1.0) < 3.0e-7){
			gamFrac = h * x * exp(LogGammaDensity(x));
			return gamFrac;
		}
	}
	cerr << "mP is too large, more than 1000 iterations needed." << endl;
	return gamFrac;// still OK if error large?
}

//===============================================================================

double CGammaProb::GammaSeries(double x)
// Compute the incomplete gamma using a serie"
// From Press et al. pp. 218-219"
{
	int i;
	double gamSer;
	double xx, ap, sum, delta;
	xx = mAlpha * x;
	ap = mP;
	delta = sum = 1.0 / mP;

	for(i=1; i<=1000; i++){// maximal number of iterations
		ap = ap + 1.0;
		delta = delta * xx / ap;
		sum = sum + delta;
		if(fabs(delta)< (fabs(sum) * 3.0e-7)){
			gamSer = sum * x * exp(LogGammaDensity(x));
			return gamSer;
		}
	}
	cerr << "mP is too large, more than 1000 iterations needed." << endl;
	return gamSer;// still OK if error large?
}

//===============================================================================

double CGammaProb::CumulDistribution(double x)
// Compute the cumulative probability distribution at the given point
{
	double gamDis;
	
	if((mAlpha * x) < (mP + 1.0)){
		gamDis = GammaSeries(x);
	}
	else{
		gamDis = 1.0 - GammaFrac(x);
	}
	return gamDis;
}

//===============================================================================
//===============================================================================

