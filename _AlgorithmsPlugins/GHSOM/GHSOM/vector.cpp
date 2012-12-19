/***************************************************************************
                          vector.cpp  -  description
                             -------------------
    begin                : Thu Jan 27 2000
    copyright            : (C) 2000 by Michael Dittenbach
    email                : mbach@ifs.tuwien.ac.at
 ***************************************************************************/

#include <cstdlib>
#include "vector.h"
#include "dataitem.h"
#include "neuronlayer.h"
#include <cstring>

class DataItem;
class NeuronLayer;

template <class T>
Vector<T>::Vector(){
	vectorSize = 0;
	vectorArray = NULL;
}

template <class T>
Vector<T>::~Vector(){
	removeAllElements();
}

template <class T>
Vector<T>::Vector(const Vector &o)
{
    vectorSize = o.vectorSize;
    vectorArray = new T*[vectorSize];
    memcpy(vectorArray, o.vectorArray, vectorSize*sizeof(T*));
}

template <class T>
Vector<T>& Vector<T>::operator=(const Vector& o)
{
    if(&o == this) return *this;
    removeAllElements();
    vectorSize = o.vectorSize;
    vectorArray = new T*[vectorSize];
    memcpy(vectorArray, o.vectorArray, vectorSize*sizeof(T*));
    return *this;
}

/**  */
template <class T>
void Vector<T>::addElement(T *obj){
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

template <class T>
T* Vector<T>::elementAt(int pos){
    if(pos >= 0 && pos < vectorSize)
	return vectorArray[pos];
    else return NULL;
}
/**  */
template <class T>
int Vector<T>::size(){
	return vectorSize;
}
/**  */
template <class T>
void Vector<T>::removeAllElements(){
    delete [] vectorArray;
	vectorArray = NULL;
	vectorSize = 0;
}

void GHSOM_getTemplates()
{
    Vector<char> v1;
    Vector<float> v2;
    Vector<DataItem> v3;
    Vector<NeuronLayer> v4;
    Vector<Vector<NeuronLayer> > v5;
    char a0 = 'a';
    float b0 = 1.f;
    v1.addElement(&a0);
    v2.addElement(&b0);
    v3.addElement(Globals::dataItems->elementAt(0));
    v4.addElement(Globals::hfm);
    v5.addElement(&v4);
    char *a = v1.elementAt(0);
    float *b = v2.elementAt(0);
    DataItem *c = v3.elementAt(0);
    NeuronLayer *d = v4.elementAt(0);
    NeuronLayer *e = v5.elementAt(0)->elementAt(0);
    int x = v1.size() + v2.size() + v3.size() + v4.size() + v5.size();
    v1.removeAllElements();
    v2.removeAllElements();
    v3.removeAllElements();
    v4.removeAllElements();
    v5.removeAllElements();
}
