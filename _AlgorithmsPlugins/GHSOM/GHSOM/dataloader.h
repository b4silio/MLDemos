/***************************************************************************
                          dataloader.h  -  description
                             -------------------
    begin                : Mon Jan 31 2000
    copyright            : (C) 2000 by Michael Dittenbach
    email                : mbach@ifs.tuwien.ac.at
 ***************************************************************************/



#ifndef GHSOM_DATALOADER_H
#define GHSOM_DATALOADER_H

#include "dataitem.h"
#include "vector.h"
typedef GVector<DataItem> Data_Vector;
/**
  *@author Michael Dittenbach
  */

class DataLoader {
public: 
	DataLoader();
	~DataLoader();
  /**  */
  void readVectorDescription(char* dfName);
  /**  */
  int &getDataLength();
  /**  */
  int &getNumVectors();
  /**  */
  char** getVectorDescription();
  /**  */
  void setVectorDescription(char **v);
  /**  */
  bool providesVectorDescription();
  /**  */
  Data_Vector* readDataItems(char *ifName);
private:
	int dataLength;
	int numVectors;
	char **vectorDescription;
};

#endif
