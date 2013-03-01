/* ************************************************************************ *
 * ************************************************************************ *
   File: readConfigFile.C

   Parameters used for hmm are defined in a file
   Comment lines are started with #
   Numbers are separated with commas or spaces
   Rows containing numbers are preceded with characters specifying what the row contains
   
  * ************************************************************************ *

   Authors: Daniel DeMenthon
   Date:   January 2003

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
using namespace std;

#define MAX_LINE_SIZE 32768

void readConfigFile(char *configName, char *sequenceName, char *hmmInputName, char *outputName, int *nbDimensions, 
                    int **ptrToListPtr,  int *nbStates, int *seed, 
                    int *isExplicitDur, int *isGaussian, int *whichEMMethod, int *skipLearning)
{
	long i;
        
	char buffer[MAX_LINE_SIZE];
	char* token;
	char minus[] = ", \n\r"; // tokens that divide numbers
        int state = -1; // state is changed every time a label is recognized; this has nothing to do with HMM states
        int *listNbSymbols;

#if 0        			
	ifstream fp(configName);
        assert(fp != NULL);
#endif
        FILE* fp = fopen(configName, "r");
        if (fp == NULL) {
                    cerr << "** config file was not found **" << endl;
                    exit(-1);
        }
	while(fgets(buffer, MAX_LINE_SIZE, fp) != NULL) {
		if (buffer[0] != '#') { /* comments start with # */
				token = strtok(buffer, minus);
				while (token != NULL) {
                                        if (strcmp(token, "sequenceName=") == 0){
                                            state = 0;
                                        }
                                        else if (strcmp(token, "hmmInputName=") == 0){
                                            state = 1;
                                        }
                                        else if (strcmp(token, "outputName=") == 0){
                                            state = 2;
                                        }
					else if (strcmp(token, "nbDimensions=") == 0){
                                            state = 3;
                                         }
                                        else if (strcmp(token, "nbSymbols=") == 0){
                                            state = 4;
                                        }
                                         else if (strcmp(token, "nbStates=") == 0){
                                            state = 5;
                                        }
                                       else if (strcmp(token, "seed=") == 0){
                                            state = 6;
                                        }
                                        else if (strcmp(token, "explicitDuration=") == 0){
                                            state = 7;
                                        }
                                        else if (strcmp(token, "gaussianDistribution=") == 0){
                                            state = 8;
                                        }
                                        else if (strcmp(token, "EMMethod=") == 0){
                                            state = 9;
                                        }
                                        else if (strcmp(token, "skipLearning=") == 0){
                                            state = 10;
                                        }
                                        else{
                                            switch(state){
                                                case 0:
                                                    sprintf(sequenceName, "%s", token);
                                                    break;
                                                case 1:
                                                    sprintf(hmmInputName, "%s", token);
                                                    break;
                                                case 2:
                                                    sprintf(outputName, "%s", token);
                                                    break;
                                                case 3:
                                                    *nbDimensions = atoi(token);
                                                    break;
                                                case 4:
                                                   listNbSymbols = new int[(*nbDimensions)+1];
                                                    *ptrToListPtr = listNbSymbols;
                                                    for (i=1;i<=*nbDimensions;i++){
                                                        listNbSymbols[i] = atoi(token);
                                                        token = strtok(NULL, minus);
                                                    }
                                                    break;
                                                case 5:
                                                    *nbStates = atoi(token);
                                                    break;
                                                 case 6:
                                                    *seed = atoi(token);
                                                    break;
                                                case 7:
                                                    *isExplicitDur = atoi(token); // 0 or 1
                                                    break;
                                                case 8:
                                                    *isGaussian = atoi(token); // 0 or 1
                                                    break;
                                                case 9:
                                                    *whichEMMethod = atoi(token); // 1 to 3
                                                    break;
                                                case 10:
                                                    *skipLearning = atoi(token); // 0 or 1
                                                    break;
                                                default:
                                                    cerr << " ** WRONG INPUT WHEN EDITING CONFIG FILE **" << endl;
                                                exit(1);
                                        }// switch
                                    }// if
                                    token = strtok(NULL, minus); // read next token
				}// end while != NULL
		}// end if buffer
	}// end while
}	
