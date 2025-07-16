/*********************************************************************
MLDemos: A User-Friendly visualization toolkit for machine learning
Copyright (C) 2010  Basilio Noris
Contact: mldemos@b4silio.com

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public License,
version 3 as published by the Free Software Foundation.

This library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free
Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*********************************************************************/
#ifndef _CLASSIFIER_EXAMPLE_H_
#define _CLASSIFIER_EXAMPLE_H_

#include <vector>
#include <map>
#include "classifier.h"

/**
  Classifier example implementing all the necessary functions from the interface
  */
class ClassifierExample : public Classifier
{
private:
    std::map<int,fvec> centers; // used as a simple examples in which we compute the distance to the center of each class
public:
    /**
      Constructor, instanciating everything that will be used
      */
    ClassifierExample(){}
    /**
      Deconstructor, deinstanciating everything that has been instanciated
      */
    ~ClassifierExample(){}

    /**
      The training function, called by the main program, all training should go here
      */
    void Train(std::vector< fvec > samples, ivec labels);

    /**
      Binary classification function, takes a sample, returns a real value, negative for class 0 and positive for class 1
      */
    float Test(const fvec &sample);

    /**
      Multi-Class classification function, takes a sample, returns a list of real values, with the likelihood of each class for the given sample
      */
    fvec TestMulti(const fvec &sample);

    /**
      Information string for the Algorithm Information and Statistics panel in the main program interface.
      Here you probably will put the number of parameters, the training time or anything else
      */
    const char *GetInfoString();

    /**
      Function to set the algorithm hyper-parameters, called prior to the training itself
      */
    void SetParams(double param1, int param2, bool param3){}
};

#endif // _CLASSIFIER_EXAMPLE_H_
