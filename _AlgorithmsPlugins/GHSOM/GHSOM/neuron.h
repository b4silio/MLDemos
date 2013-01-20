/***************************************************************************
                          neuron.h  -  description
                             -------------------
    begin                : Tue Jan 25 2000
    copyright            : (C) 2000 by Michael Dittenbach
    email                : mbach@ifs.tuwien.ac.at
 ***************************************************************************/


#ifndef GHSOM_NEURON_H
#define GHSOM_NEURON_H

#include "neuronlayer.h"

#include "dataitem.h"
#include "globals.h"
#include "vector.h"
typedef GVector<DataItem> Data_Vector;
typedef GVector<char> String_Vector;
typedef GVector<float> Double_Vector;
class NeuronLayer;
/**
  *@author Michael Dittenbach
  */

class Neuron {
public: 
  Neuron(int ws,int ingid,int inlevel,int spx,int spy);
  Neuron(float *w,int ingid,int inlevel,int spx,int spy);
  ~Neuron();
  /**  */
  void adaptWeights(DataItem *di,float dist,float learnrate,float neighbourhood);
  /**  */
  float calcDist(DataItem *di);
  /**  */
  void calcMQE();
  /**  */
  void addRepresentingDataItem(DataItem *di);
  /**  */
  int representsMultiDataItems();
  /**  */
  NeuronLayer *getMap();
  /**  */
  void addMap(Neuron *sn,float MQE,int level,int sizeX,int sizeY,int posX,int posY);
  /**  */
  void addMap(Data_Vector *dataItems,float MQE,int level,int sizeX,int SizeY,int posX,int posY);
  /**  */
  void addMap(Neuron *sn,float MQE,int level,int posX,int posY,float *ULweight, float *URweight, float *LLweight, float *LRweight);
  /**  */
  Data_Vector *getRepresentingDataItems();
  /**  */
  void clearRepresentingDataItems();
  /**  */
  String_Vector *getLabels(int num);

  int mapgid,maplevel,mapspx,mapspy;
  float MQE;
  float *weights;
  int weightsize;
private:
	NeuronLayer *nl;
	Data_Vector *representingDataItems;
};

#endif
