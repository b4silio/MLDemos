/***************************************************************************
                          neuron.cpp  -  description
                             -------------------
    begin                : Tue Jan 25 2000
    copyright            : (C) 2000 by Michael Dittenbach
    email                : mbach@ifs.tuwien.ac.at
 ***************************************************************************/


#include <cstdlib>
#include "neuron.h"
#include <stdio.h>
#include <iostream>
//#include "globals.h"
//#include "vector.h"
#include <math.h>

Neuron::Neuron(int ws,int ingid,int inlevel,int spx,int spy){
  weightsize = ws;
  weights = new float[weightsize];
  for (int i=0;i<weightsize;i++) {
    weights[i] = Globals::getRandom();
  }
  nl = NULL;
  weights = Globals::normVec(weights);
  representingDataItems = new Data_Vector();
  mapgid=ingid;
  maplevel=inlevel;
  mapspx=spx;
  mapspy=spy;
}

Neuron::Neuron(float *w,int ingid,int inlevel,int spx,int spy) {
  weights = w;
  weightsize = Globals::vectorlength;
  representingDataItems = new Data_Vector();
  nl = NULL;
  mapgid=ingid;
  maplevel=inlevel;
  mapspx=spx;
  mapspy=spy;
}

Neuron::~Neuron(){
    if(weights) delete [] weights;
    if(nl) delete nl;
    if(representingDataItems) delete representingDataItems;
    weights = 0;
    nl = 0;
    representingDataItems = 0;
}
/** alte funktion */
//void Neuron::adaptWeights(DataItem *di,float dist,float learnrate,int neighbourhood){
//	float n_influence = (-(1/(neighbourhood+0.5))*dist)+1;
//	for (int i=0;i<weightsize;i++) {
//			weights[i] += (learnrate * n_influence * (di->getDataVector()[i] - weights[i]));
//	}
//}
/** neue funktion */
void Neuron::adaptWeights(DataItem *di,float dist,float learnrate,float neighbourhood){
  float n_influence = learnrate * exp(-1.0*pow((dist/(2.0*pow((float)neighbourhood,2))),2));     //(-(1/(neighbourhood+0.5))*dist)+1;
  for (int i=0;i<weightsize;i++) {
    weights[i] = weights[i] + (n_influence * (di->getDataVector()[i] - weights[i]));
  }
}
/**  */
float Neuron::calcDist(DataItem *di){
	return Globals::calcQE(di->getDataVector(),weights);
}
/**  */
void Neuron::calcMQE(){
  if (representingDataItems->size()==0) {
    MQE = 0;
  } else {
    MQE = 0;
    for (int i=0; i<representingDataItems->size();i++) {
      MQE += Globals::calcQE(representingDataItems->elementAt(i)->getDataVector(),weights);
    }
    //MQE = MQE / (float) representingDataItems->size();
  }
}
/**  */
int Neuron::representsMultiDataItems(){
	return (representingDataItems->size());
}
/**  */
void Neuron::addRepresentingDataItem(DataItem *di){
	representingDataItems->addElement(di);
}
/**  */
void Neuron::clearRepresentingDataItems(){
	representingDataItems->removeAllElements();
}
/**  */
Data_Vector* Neuron::getRepresentingDataItems(){
	return representingDataItems;
}
/**  */
void Neuron::addMap(Data_Vector *dataItems,float MQE,int level,int sizeX,int sizeY,int posX,int posY){
	nl = new NeuronLayer(NULL,dataItems,MQE,level,sizeX,sizeY,posX,posY,NULL,NULL,NULL,NULL);
}
/**  */
void Neuron::addMap(Neuron *sn,float MQE,int level,int sizeX,int sizeY,int posX,int posY){
	nl = new NeuronLayer(sn,representingDataItems,MQE,level,sizeX,sizeY,posX,posY,NULL,NULL,NULL,NULL);
}
/**  */
void Neuron::addMap(Neuron *sn,float MQE,int level,int posX,int posY,float *ULweight, float *URweight, float *LLweight, float *LRweight){
  nl = new NeuronLayer(sn,representingDataItems,MQE,level,2,2,posX,posY,ULweight,URweight,LLweight,LRweight);
}
/**  */
NeuronLayer *Neuron::getMap(){
	return nl;
}
/**  */ /* ***************** max weight component ******************** */
/*String_Vector *Neuron::getLabels(int num){
	String_Vector *res;
	res = new String_Vector;
	
	float maxlabels[num];
	for (int i=0;i<num;i++) { maxlabels[i]=MAX_DOUBLE; }
	int maxlabelindex[num];
	for (int i=0;i<num;i++) { maxlabelindex[i]=0; }
	float max;
	int maxindex;
	int full = 0;
	float last = MAX_DOUBLE;

	while (full<num) {
		max = 0;
		maxindex = 0;
		for (int i=0;i<weightsize;i++) {
			if ((weights[i]>max) && (weights[i]<last)) {
				max = weights[i];
				maxindex = i;
			}
		}
		maxlabels[full] = max;
		maxlabelindex[full] = maxindex;
		last = max;
		full++;
 	
		int j=0;
		while ((j<weightsize) && (full<num)) {
			if ((weights[j] == max) && (j != maxindex)) {
				maxlabels[full] = weights[j];
				maxlabelindex[full] = j;
				full++;
			}
			j++;
		}
  }

	for (int i=0;i<num;i++) {
		res->addElement(Globals::currentDataLoader->getVectorDescription()[maxlabelindex[i]]);
	}
	return res;
*/
/*	float *errors;
	errors = (float*) calloc(weightsize,sizeof(float));
	for (int i=0;i<weightsize;i++) {	
		float featureerror;
		for (int di=0;di<representingDataItems->size();di++) {
			featureerror += representingDataItems->elementAt(di)->getDataVector()[i];
		}
		featureerror = featureerror / (float)representingDataItems->size();
		featureerror = featureerror - weights[i];
		if (featureerror < 0) { featureerror = 0-featureerror; }
		errors[i] = featureerror;
	}
	float min = MAX_DOUBLE;
	int minindex = 0;
	for (int i=0;i<weightsize;i++) {
		if (errors[i]<min) {
			bool allzero = true;
			for (int di=0;di<representingDataItems->size();di++) {
				if (representingDataItems->elementAt(di)->getDataVector()[i]==0) {
					allzero = false;
				}
			}
			if (allzero==false) {
				min = errors[i];
				minindex = i;
			}
		}
	}*/
//}

/* ************** Varianz ********************
String_Vector *Neuron::getLabels(int num){
	String_Vector *res;
	res = new String_Vector;
	
	float *variance;
	variance = (float*) calloc(weightsize,sizeof(float));
	float *variancemax;
	variancemax = (float*) calloc(weightsize,sizeof(float));
	for (int i=0;i<weightsize;i++) {	
		float featuremin=MAX_DOUBLE, featuremax=0;
		for (int di=0;di<representingDataItems->size();di++) {
			if (representingDataItems->elementAt(di)->getDataVector()[i] <= featuremin) {
				featuremin = representingDataItems->elementAt(di)->getDataVector()[i];
			}
			if (representingDataItems->elementAt(di)->getDataVector()[i] >= featuremax) {
				featuremax = representingDataItems->elementAt(di)->getDataVector()[i];
			}
		}
		variancemax[i] = featuremax;
		variance[i] = featuremax - featuremin;
	}

	float minlabels[num];
	for (int i=0;i<num;i++) { minlabels[i]=0; }
	int minlabelindex[num];
	for (int i=0;i<num;i++) { minlabelindex[i]=0; }
	float min;
	int minindex;
	int full = 0;
	float last = 0;

	while (full<num) {
		min = MAX_DOUBLE;
		minindex = 0;
		for (int i=0;i<weightsize;i++) {
			if ((variance[i]<min) && (variance[i]>last) && (variancemax[i]>0)) {
				min = variance[i];
				minindex = i;
			}
		}
		minlabels[full] = min;
		minlabelindex[full] = minindex;
		last = min;
		full++;
 	
		int j=0;
		while ((j<weightsize) && (full<num)) {
			if ((variance[j] == min) && (j != minindex) && (variancemax>0)) {
				minlabels[full] = variance[j];
				minlabelindex[full] = j;
				full++;
			}
			j++;
		}
  }

	for (int i=0;i<num;i++) {
		res->addElement(Globals::currentDataLoader->getVectorDescription()[minlabelindex[i]]);
	}
	return res;
}
*/ // LabelSOM
String_Vector *Neuron::getLabels(int num){
    String_Vector *res = new String_Vector;
	
    float *variance = new float[weightsize];
	for (int i=0;i<weightsize;i++) {	
		float featureerror = 0;
		for (int di=0;di<representingDataItems->size();di++) {
			featureerror += (weights[i] - representingDataItems->elementAt(di)->getDataVector()[i]) *
											(weights[i] - representingDataItems->elementAt(di)->getDataVector()[i]);
		}
		variance[i] = sqrt(featureerror)/representingDataItems->size();
	}

	float maxweightvalue=0;
	for (int i=0;i<weightsize;i++) {
		if (weights[i]>maxweightvalue) {
			maxweightvalue = weights[i];
		}
	}

	float minlabels[num];
	for (int i=0;i<num;i++) { minlabels[i]=0; }
	int minlabelindex[num];
	for (int i=0;i<num;i++) { minlabelindex[i]=0; }
	float min;
	int minindex;
	int full = 0;
	float last = 0;
	bool stop = false;

	while ((full<num) && (!stop)) {
		min = MAX_DOUBLE;
		minindex = 0;
		bool found = false;
		for (int i=0;i<weightsize;i++) {
			if ((variance[i]<min) && (variance[i]>last) && (weights[i]>(Globals::LABELS_THRESHOLD*maxweightvalue))) {
				min = variance[i];
				minindex = i;
				found = true;
			}
		}

		if ((!found) ) {
			stop = true;
		}

    if (!stop) {
			minlabels[full] = min;
			minlabelindex[full] = minindex;
			last = min;
			full++;
 	
			int j=0;
			while ((j<weightsize) && (full<num) && (!stop)) {
				if ((variance[j] == min) && (j != minindex) && (weights[j]>(Globals::LABELS_THRESHOLD*maxweightvalue))) {
					minlabels[full] = variance[j];
					minlabelindex[full] = j;
					full++;
				}
				j++;
			}
		}
  }
//	int to;
//  if (full==num) { to=full; } else { to=full-1; }
	for (int i=0;i<full;i++) {
        res->addElement(Globals::vectorDescription[minlabelindex[i]]);
	}
    delete [] variance;
	return res;
}
