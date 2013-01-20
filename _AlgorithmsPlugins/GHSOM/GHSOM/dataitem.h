/***************************************************************************
                          dataitem.h  -  description
                             -------------------
    begin                : Tue Jan 25 2000
    copyright            : (C) 2000 by Michael Dittenbach
    email                : mbach@ifs.tuwien.ac.at
 ***************************************************************************/



#ifndef GHSOM_DATAITEM_H
#define GHSOM_DATAITEM_H

/**
  *@author Michael Dittenbach
  */

class DataItem {
public: 
    DataItem(char *inid, float *indataVector, int inlength);
    ~DataItem();
    /**  */
    float *getDataVector();
    /**  */
    char *getId();
    /**  */
    int getLength();
    DataItem(const DataItem& o);
    DataItem& operator=(const DataItem& o);
private:
    char *id;
    float *dataVector;
    int length;
};

#endif
