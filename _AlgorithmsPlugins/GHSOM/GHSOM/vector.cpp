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

void GHSOM_getTemplates()
{
	GVector<char> v1;
	GVector<float> v2;
	GVector<DataItem> v3;
	GVector<NeuronLayer> v4;
	GVector<GVector<NeuronLayer> > v5;
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
