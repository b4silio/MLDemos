/***************************************************************************
                          vector.h  -  description
                             -------------------
    begin                : Thu Jan 27 2000
    copyright            : (C) 2000 by Michael Dittenbach
    email                : mbach@ifs.tuwien.ac.at
 ***************************************************************************/
#ifndef GHSOM_VECTOR_H
#define GHSOM_VECTOR_H


#include "dataitem.h"
//#include "neuronlayer.h"

class NeuronLayer;
class DataItem;

/**
  *@author Michael Dittenbach
  */

template <class T>
class Vector {
public: 
	Vector();
	~Vector();
    Vector(const Vector &o);
    Vector& operator=(const Vector& o);

    /**  */
  void addElement(T *obj);
public: // Public attributes
  /**  */
  T *elementAt(int pos);
  /**  */
  int size();
  /**  */
  void removeAllElements();
private: // Private attributes
  /**  */
  int vectorSize;
  /**  */
  T** vectorArray;
};

void GHSOM_getTemplates();

template class Vector<char>;
template class Vector<float>;
template class Vector<DataItem>;
template class Vector<NeuronLayer>;
template class Vector<Vector<NeuronLayer> >;

#endif // GHSOM_VECTOR_H
