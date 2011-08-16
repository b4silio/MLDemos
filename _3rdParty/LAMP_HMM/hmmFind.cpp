/* ************************************************************************ *
 * ************************************************************************ *
   File: hmmMain.C

   From file of sequences of observations, find HMM model.

  * ************************************************************************ *

   Authors: Daniel DeMenthon & Marc Vuilleumier
   Date:   February 22, 1999

 * ************************************************************************ *
   Modification Log:
   January 5, 2003: Process observation vectors with discrete pdfs 
                    and different nbSymbols for each components
   January 9, 2003: Read input data from a config file
   

 * ************************************************************************ *
   Log for new ideas:
 * ************************************************************************ *
               Copyright 2003 Daniel DeMenthon and LAMP
               Language and Media Processing (LAMP)
               Center for Automation Research
               University of Maryland
               College Park, MD  20742
 * ************************************************************************ *
 * ************************************************************************ */
 
//===============================================================================

//===============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <iostream.h>
#include <iomanip.h>
#include <fstream.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "utils.h"
#include "obs.h"
#include "obsSeq.h"
#include "obsProb.h"
#include "discreteObsProb.h"
#include "gaussianObsProb.h"
#include "vectorObsProb.h"
#include "stateTrans.h"
#include "plainStateTrans.h"
#include "gammaProb.h"
#include "explicitDurationTrans.h"
#include "initStateProb.h"
#include "hmm.h"

//===============================================================================

//===============================================================================

void PrintUsage(void);

void readConfigFile(char *configName, char *sequenceName, char *hmmInputName, char *outputName, int *nbDimensions, 
                    int **ptrToListPtr,  int *nbStates, int *seed, 
                    int *isExplicitDur, int *isGaussian, int *whichEMMethod, int *skipLearning);

//===============================================================================
// Example of call on Mac OS X:
// hmmFind.app/Contents/MacOS/hmmFind  /Users/daniel/C-CODE/HMM2Movies/Source/configFileExample.txt
//
//===============================================================================

int main (int argc, char *argv[])
{
    int nbDimensions, nbSymbols, nbStates, seed;
    int *listNbSymbols;
    int **ptrToListPtr; // array of dimensions is generated in function, we need a pointer to it
    ptrToListPtr = new int*; // space must be allocated for returning the pointer to the array
    
    int isExplicitDur, isGaussian, whichEMMethod, skipLearning, readHMMFile;
    char configName[256];
    char sequenceName[256];
    char outputName[256];
    char hmmInputName[256];
    char hmmOutputName[256];
    char stateOutputName[256];
    char expectedObsOutputName[256];
    char distanceOutputName[256];
    char magicID[32];

#if 1
    if (argc != 2) {
        PrintUsage();
        exit (1);
    }
#else // debugging
    argv[1] = "/Users/daniel/C-CODE/HMM2Movies/Source/configFileExample.txt";
#endif
    sprintf(configName, "%s", argv[1]);
    
    readConfigFile(configName, sequenceName, hmmInputName, outputName, 
                &nbDimensions, ptrToListPtr, &nbStates, &seed, 
                &isExplicitDur, &isGaussian, &whichEMMethod, &skipLearning);
    listNbSymbols = *ptrToListPtr;
                
    sprintf(hmmOutputName,"%s%s",outputName, ".hmm");
    sprintf(stateOutputName,"%s%s",outputName, ".sta");
    sprintf(expectedObsOutputName,"%s%s",outputName, ".obs");
    sprintf(distanceOutputName,"%s%s",outputName, ".obs");
    
    readHMMFile = !(strcmp(hmmInputName, "0") == 0); // user put zero as input name if no input HMM
            
    
    // 1. Define hmm
    
    ifstream hmmFile;
    
    if (readHMMFile){
        hmmFile.open(hmmInputName);
        if(hmmFile==NULL){
            cerr << "HMM file not found. Exiting..."<<endl;
            exit(-1);
        }
        hmmFile >> magicID;
        cout << "** Reading HMM from file **" << endl;
        assert(strcmp(magicID, "N=")==0);
        hmmFile >> nbStates;// nb of states; this overwrites nbStates returned by readConfigFile
        
    }
    cout << endl; 
    cout << "** Nb of States (N) is "<< nbStates << " **" << endl;
    cout << "** Nb of components in observation vectors (nbDimensions) is "<< nbDimensions << " **" << endl;
    cout << "** Nb of symbols (M) used for each component: "<< endl;
    for (int i=1;i<=nbDimensions;i++){
        cout << listNbSymbols[i] << endl;
    }
    cout << endl;
    
    CStateTrans *a;
    CObsProb *b;
    CInitStateProb *pi;
    CHMM *learnedHMM;
    CObs *obsType;
    
    
    if (isExplicitDur){
        cout << "** EXPLICIT DURATIONS **"<< endl;
        if (readHMMFile){
            a = new CExplicitDurationTrans(hmmFile, nbStates);
        }
        else{
            a = new CExplicitDurationTrans(nbStates, seed);
        }
    }
    else{
        cout << "** PLAIN STATE TRANSITIONS **"<< endl;
        if (readHMMFile){
            a = new CPlainStateTrans(hmmFile, nbStates);
        }
        else{
            a = new CPlainStateTrans(nbStates, seed);
         }
    }
            
    if (nbDimensions == 1){
        nbSymbols = listNbSymbols[1];
        obsType = new CIntObs;
        if (isGaussian){
            cout << "** GAUSSIAN PROB, SCALAR OBS **" << endl;
            if (readHMMFile){
                b = new CGaussianObsProb(hmmFile, nbStates);
            }
            else{
                b = new CGaussianObsProb(nbSymbols, nbStates);
            }
        }
        else{ // discrete (histograms)
            cout << "** DISCRETE PROB, SCALAR OBS **"<< endl;
            if (readHMMFile){
                b = new CDiscreteObsProb(hmmFile, nbStates);
            }
            else{
                b = new CDiscreteObsProb(nbSymbols, nbStates);
            }
        }
    }
    else if (nbDimensions > 1){// observation vectors
        obsType = new CVectorObs(nbDimensions);
        if (isGaussian){// Gaussian
            cout << "** GAUSSIAN PROBS FOR VECTOR OBS **" << endl;
        }
        else{ // discrete (histograms)
            cout << "** DISCRETE PROBS FOR VECTOR OBS **" << endl;
        }
        if (readHMMFile){
            b = new CVectorObsProb(hmmFile, nbStates, nbDimensions, isGaussian);
        }
        else{
            b = new CVectorObsProb(listNbSymbols, nbStates, nbDimensions, isGaussian);
        }
    }
    else{
        cerr << "** NUMBER OF DIMENSIONS CANNOT BE SET TO ZERO **" << endl;
        exit(-1);
    }
    
    
    if (readHMMFile){
        pi = new CInitStateProb(hmmFile, nbStates);
    }
    else{
        pi = new CInitStateProb(nbStates);
    }
    
    learnedHMM = new CHMM(a, b, pi);
    
    // 2. Read observation sequence into data structure
    
            CObsSeq *obsSeq;
            ifstream sequenceFile(sequenceName);
            assert(sequenceFile != NULL);
    //	obsSeq = learnedHMM->ReadSequences(sequenceFile);
            obsSeq = new CObsSeq(obsType, sequenceFile);
            sequenceFile.close();
            
    
    // 3. Generate HMM from file of observation sequences
    
        
    if(! skipLearning){
        switch (whichEMMethod){
            case 1:
                cout << "** BAUM WELCH LEARNING **" << endl;
                learnedHMM->LearnBaumWelch(obsSeq);
                break;
            case 2:
                cout << "** SEGMENTAL K-MEANS LEARNING **" << endl;
                learnedHMM->LearnSegmentalKMeans(obsSeq);
                break;
            case 3:
                cout << "** HYBRID LEARNING **" << endl;
                learnedHMM->LearnHybridSKM_BW(obsSeq);
                break;
            default:
                cerr << " ** WRONG INPUT IN CONFIG FILE **" << endl;
                exit(1);
        }
    // print and store HMM results
        ofstream hmmOutput(hmmOutputName);
        learnedHMM->Print(hmmOutput);
        hmmOutput.close();
        learnedHMM->Print(cout);
    }
    else{ // skip learning step
    }
    
    ofstream stateOutput(stateOutputName);
    ofstream expectedObsOutput(expectedObsOutputName);
    ofstream distanceOutput(distanceOutputName);
    
    learnedHMM->PrintStatesAndExpectedObs(obsSeq, stateOutput, expectedObsOutput);
    stateOutput.close();
    expectedObsOutput.close();
    // Distances to each sequence in obsSeq and total distance are stored in distanceOutput file
    double normalizedLogProb = learnedHMM->FindViterbiDistance(obsSeq, distanceOutput);
    // Store normalizedLogProb in another file for classification 
    distanceOutput.close();
    
    delete learnedHMM;
    delete pi;
    delete b;
    delete a;
    //delete obsSeq;// check with purify
            
    return 0;
}

//===============================================================================

void PrintUsage(void)
{
  cout << "Usage: hmmFind configFileName" << endl;
  cout << "Example: hmmFind configFileExample.txt"  << endl;
  cout << "configFile contains comment lines and required input data, which are:" << endl;
  cout << "Name for sequence file" << endl;
  cout << "containing number of sequences and observation sequences," << endl;
  cout << "Switch skipLearning that is 0 if we are learning a new HMM" << endl;
  cout << "and 1 for classification by finding how well inputHMM predicts sequence file." << endl;
  cout << "Name for file containing input HMM" << endl;
  cout << "used as an initialization for finding HMM of data if skipLearning is 0" << endl;
  cout << "and used for finding distance to data if skipLearning is 1." << endl;
  cout << "When HMM is used as a classification method, distances of different HMMs to a data set" << endl;
  cout << "are computed with skipLearning set, and compared." << endl;
  cout << "Name for prefix used to produce four output files,"<<endl;
  cout << "prefix.hmm: computed HMM model" << endl;
  cout << "prefix.sta: sequences of most probable states using Viterbi" << endl;
  cout << "prefix.obs: sequences of expected observations for each probable states" << endl;
  cout << "prefix.dis: distance of result HMM or input HMM to each of the sequences" << endl;
  cout << "Input data in config file also include feature vector dimension, nbs of symbols and nb of states," << endl;
  cout << "and switches that specify the type of modeling to be used." << endl;

}

//===============================================================================
//===============================================================================
