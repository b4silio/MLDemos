/***************************************************************************
                          dataitem.cpp  -  description
                             -------------------
    begin                : Tue Jan 25 2000
    copyright            : (C) 2000 by Michael Dittenbach
    email                : mbach@ifs.tuwien.ac.at
 ***************************************************************************/


#include <cstdlib>
#include <cstring>
#include "dataitem.h"

DataItem::DataItem(char *inid, float *indataVector, int inlength)
    : id(0), dataVector(0)
{
    id = new char[150];
    dataVector = new float[inlength];
    strcpy(id, inid);
    memcpy(dataVector, indataVector, inlength*sizeof(float));
	length = inlength;
}
DataItem::~DataItem(){
    delete [] id;
    delete [] dataVector;
    id = NULL;
    dataVector = NULL;
    length = 0;
}

DataItem::DataItem(const DataItem& o)
{
    id = new char[150];
    dataVector = new float[o.length];
    strcpy(id, o.id);
    memcpy(dataVector, o.dataVector, o.length*sizeof(float));
    length = o.length;
}

DataItem& DataItem::operator=(const DataItem& o)
{
    if(&o == this) return *this;
    if(id) delete [] id;
    if(dataVector) delete [] dataVector;
    id = new char[150];
    dataVector = new float[o.length];
    strcpy(id, o.id);
    memcpy(dataVector, o.dataVector, o.length*sizeof(float));
    length = o.length;
    return *this;
}

/**  */
float *DataItem::getDataVector(){
	return dataVector;
}
/**  */
char *DataItem::getId(){
    return id;
}
/**  */
int DataItem::getLength(){
    return length;
}
