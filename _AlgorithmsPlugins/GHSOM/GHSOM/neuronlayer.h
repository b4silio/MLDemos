/***************************************************************************
                          neuronlayer.h  -  description
                             -------------------
    begin                : Tue Jan 25 2000
    copyright            : (C) 2000 by Michael Dittenbach
    email                : mbach@ifs.tuwien.ac.at
 ***************************************************************************/



#ifndef GHSOM_NEURONLAYER_H
#define GHSOM_NEURONLAYER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// inserted by mmx
#include <math.h>
// inserted by mmx - end
#include "neuron.h"
#include "vector.h"
#include "globals.h"
typedef GVector<DataItem> Data_Vector;
typedef GVector<char> String_Vector;
class Neuron;
/**
  *@author Michael Dittenbach
  */

class NeuronLayer {
public:
  int gid;
  float superMQE;
	
  NeuronLayer(Neuron *sn,Data_Vector *dataItems,float superMQE,int level,int initialSizeX,int initialSizeY,int posX,int posY, float *ULweight, float *URweight, float *LLweight, float *LRweight);
	~NeuronLayer();
  /**  */
  NeuronLayer* getLayer1Map();
  /**  */
  void calcLayer0MQE();
  /**  */
  void calcMQE();
  /**  */
  void insertColumn(int pos);
  /**  */
  void train();
  /**  */
  void saveAs(int what);

  Neuron ***getMap(){return neuronMap;}
  int getX(){return x;}
  int getY(){return y;}
  Neuron *getSuperNeuron(){return superNeuron;}

private:
  Data_Vector *dataItems;
  int *superPos;
  Neuron *superNeuron;
  float MQE;
  int *MQENeuron;
  float stupidity;
  float ini_learnrate;
  float learnrate;
  float min_learnrate;
  float ini_neighbourhood;
  float neighbourhood;
  float min_neighbourhood;
  float neighbourhood_falloff;
  int level;
  int dataLength;
  int x;
  int y;
  float STRETCH_PARAM_LEARN;
  float STRETCH_PARAM_NEIGHB;
  Neuron ***neuronMap;
  int currentCycle;
private: // Private methods
  /**  */
  void adaptWeights(int *winner, DataItem *di);
  /**  */
  void testDataItems();
  /**  */
  int *getMaxDissNeighbour(int *n);
  /**  */
  void saveAsHTML();
  /**  */
  void insertRow(int pos);
  /**  */
  void saveAsSOMLib();
  /**  */
  void getNewWeights(int xPos, int yPos, float *UL, float *UR, float *LL, float *LR);
};

#endif
