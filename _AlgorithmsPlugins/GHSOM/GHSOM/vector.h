/***************************************************************************
                          vector.h  -  description
                             -------------------
    begin                : Thu Jan 27 2000
    copyright            : (C) 2000 by Michael Dittenbach
    email                : mbach@ifs.tuwien.ac.at
 ***************************************************************************/
#ifndef GHSOM_VECTOR_H
#define GHSOM_VECTOR_H

#include <string.h>
#include "dataitem.h"
//#include "neuronlayer.h"

class NeuronLayer;
class DataItem;

/**
  *@author Michael Dittenbach
  */

template <class T>
class GVector {
public: 
	GVector() : vectorSize(0), vectorArray(0){}
	~GVector() {removeAllElements();}
	GVector(const GVector &o)
	{
		vectorSize = o.vectorSize;
		vectorArray = new T*[vectorSize];
		memcpy(vectorArray, o.vectorArray, vectorSize*sizeof(T*));
	}
	GVector& operator=(const GVector& o)
	{
		if(&o == this) return *this;
		removeAllElements();
		vectorSize = o.vectorSize;
		vectorArray = new T*[vectorSize];
		memcpy(vectorArray, o.vectorArray, vectorSize*sizeof(T*));
		return *this;
	}

    /**  */
	void addElement(T *obj)
	{
		if(vectorArray)
		{
			T **newArray = new T*[vectorSize+1];
			memcpy(newArray, vectorArray, vectorSize*sizeof(T*));
			newArray[vectorSize] = obj;
			delete [] vectorArray;
			vectorArray = newArray;
		}
		else
		{
			vectorArray = new T*[1];
			vectorArray[0] = obj;
		}
		++vectorSize;
	}

public: // Public attributes
	/**  */
	T *elementAt(int pos)
	{
		if(pos >= 0 && pos < vectorSize)
			return vectorArray[pos];
		else return 0;
	}
	/**  */
	int size() {return vectorSize;}
	/**  */
	void removeAllElements()
	{
		delete [] vectorArray;
		vectorArray = 0;
		vectorSize = 0;
	}
private: // Private attributes
	/**  */
	int vectorSize;
	/**  */
	T** vectorArray;
};

void GHSOM_getTemplates();

#ifdef WIN32
template class GVector<char>;
template class GVector<float>;
template class GVector<DataItem>;
template class GVector<NeuronLayer>;
template class GVector<GVector<NeuronLayer> >;
#endif

#endif // GHSOM_VECTOR_H
