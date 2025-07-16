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
#ifndef _CLUSTERER_EXAMPLE_H_
#define _CLUSTERER_EXAMPLE_H_

#include <vector>
#include <clusterer.h>

/**
  Clusterer example implementing all the necessary functions from the interface
  */
class ClustererExample : public Clusterer
{
public:
    /**
      Constructor, instanciating everything that will be used
      */
    ClustererExample(){}
    /**
      Deconstructor, deinstanciating everything that has been instanciated
      */
    ~ClustererExample(){}

    /**
      The training function, called by the main program, all training should go here
      */
    void Train(std::vector< fvec > samples);

    /**
      The testing function, returns a vector of size nbClusters, with the contribution/weight of the point for each cluster
      */
	fvec Test( const fvec &sample);

    /**
      Information string for the Algorithm Information and Statistics panel in the main program interface.
      Here you probably will put the number of parameters, the training time or anything else
      */
    const char *GetInfoString();

    /**
      Function to set the algorithm hyper-parameters, called prior to the training itself
      */
    void SetParams(float param1, int param2, bool param3){}
};

#endif // _CLUSTERER_EXAMPLE_H_
