/* ************************************************************************ *
 * ************************************************************************ *
   File:checkDUrationDist.C

  Verify that the durations given by CGammaProb::FindRandomDuration(void)
  are distributed in a gamma distribution.

  * ************************************************************************ *

   Authors: Daniel DeMenthon
   Date:   April 28, 1999

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
//#include <console.h>
using namespace std;

//===============================================================================

int main (int argc, char **argv)
{
	int i;
	int duration;
	int maxCount = 2000;
	double mean, variance, newMean, newVariance, ranseed;
	
	CGammaProb *gammaProb = new CGammaProb();
	
	//argc = ccommand(&argv);// open window to enter arguments on Mac
	if (argc != 3) {
	  cout << "enter mean and variance" << endl;
	  exit (1);
	}
	mean = atof(argv[1]);
	variance = atof(argv[2]);
//   randseed = atoi(argv[3]);
//   gammaProb->gsl_ran_seed (randseed);
	gammaProb->SetMean(mean);
	gammaProb->SetVariance(variance);
	gammaProb->FindParameters();
	
	gammaProb->ResetSums();
	
	for(i=0; i<maxCount; i++){
		duration = gammaProb->FindRandomDuration();
		cout << duration << " ";
		if(i % 40 == 0) cout << endl;
		gammaProb->SKMSum(duration);
	}
	gammaProb->ProcessSums();
	newMean = gammaProb->GetMean();
	newVariance = gammaProb->GetVariance();
	
	cout << endl << "Input mean and variance " << mean << " " << variance << endl;
	cout <<"Output mean and variance " << newMean << " " << newVariance << endl;
}
