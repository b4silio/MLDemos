/***************************************************************************
                          globals.h  -  description
                             -------------------
    begin                : Tue Jan 25 2000
    copyright            : (C) 2000 by Michael Dittenbach
    email                : mbach@ifs.tuwien.ac.at
 ***************************************************************************/

#ifndef GHSOM_GLOBALS_H
#define GHSOM_GLOBALS_H

#define VERSION 1.6

#include <time.h>
#include "neuron.h"
#include "neuronlayer.h"
#include "vector.h"
#include "dataitem.h"
#include "dataloader.h"
class Neuron;
class NeuronLayer;


typedef GVector<DataItem> Data_Vector;
typedef GVector<GVector<NeuronLayer> > Vector_Vector;
typedef GVector<NeuronLayer> Layer_Vector;

/**
  *@author Michael Dittenbach
  */

class Globals {
public:
	static int vectorlength;
	static int numofallvecs;
	static int HTML_GID;
	static float TAU_1;
	static float INITIAL_LEARNRATE;
	static float MIN_LEARNRATE;
	static int INITIAL_NEIGHBOURHOOD;
	static int MIN_NEIGHBOURHOOD;
	static float NEIGHBOURHOOD_FALLOFF;
	static int MAX_CYCLES;
	static int EXPAND_CYCLES;
//	static int MAX_REPRESENTING;
	static float TAU_2;
	static const char *HTML_PREFIX;
	static const char *DATAFILE_EXTENSION;
	static Data_Vector *dataItems;
	static NeuronLayer *hfm;
	static unsigned int randomSeed;
	static bool isTrained;
	static Vector_Vector *layers;
	static clock_t startTime;
	static clock_t stopTime;
	static float trainingTime;
	static char *inputFile;
	static char *descriptionFile;
	static DataLoader *currentDataLoader;
    static char **vectorDescription;
	static char *savePath;
	static bool printMQE;
	static int normInputVectors;
	static bool SAVE_AS_HTML;
	static bool SAVE_AS_SOMLIB;
	static int INITIAL_X_SIZE;
	static int INITIAL_Y_SIZE;
	static int LABELS_NUM;
	static bool LABELS_ONLY;
	static float LABELS_THRESHOLD;
	static bool ORIENTATION;
	static char *MQE0_FILE;
	static float NR;

	static void setRandom(unsigned int seed);
	static float getRandom();
	static float *normVec(float *vec);
	static Data_Vector* normIntervalVector(Data_Vector *vec);
	static float calcQE(float *v1, float *v2);
	static float *meanWeights(float *w1,float *w2);
	static float *meanVector(Data_Vector *dItems,int dataLength);
	static void addLayer(int level,NeuronLayer *nl);
	static void initHFM();
	static void trainHFM();
	static NeuronLayer *getFirstLayerMap();
	static Layer_Vector *getLayerAt(int level);
	static int getIntRandom();
	static float *vectorDiff(float *a, float *b);
	static float *vectorDiffMean(float *a, float *b);
	static float *vectorAdd(float *a, float *b);
	static float *vectorAddMean(float *a, float *b);
	static float *vectorAdd3(float *a,float *b, float *c);
	static float *vectorAdd3Mean(float *a,float *b, float *c);

	Globals();
	~Globals();
  /**  */
  static void saveHFMAs(int what);
  /**  */
  /**  */
};

//static Globals *glob = new Globals();

#define MAX_DOUBLE 999999999
#define HTML 0
#define ASCII 1
#define SOMLIB 2
#define NORM_NONE 0
#define NORM_LENGTH 1
#define NORM_INTERVAL 2
//typedef enum {FALSE=0, TRUE} boolean;

#endif
