/*********************************************************************
MLDemos: A User-Friendly visualization toolkit for machine learning
Copyright (C) 2010  Basilio Noris
Contact: mldemos@b4silio.com

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free
Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*********************************************************************/
#include "public.h"
#include "clustererExample.h"

using namespace std;

void ClustererExample::Train(std::vector< fvec > samples)
{
	if(!samples.size()) return;
    dim = samples[0].size();

    nbClusters = 2; // this is usually initialized from the hyperparameters panel, or might be computed automatically
}

fvec ClustererExample::Test( const fvec &sample)
{
	fvec res;
	res.resize(nbClusters,0);

    // just for our testing, we do a dumb selection process
    float dot = sample*sample*50;
    int index = ((int)dot) % dim;
    res[index] = 1;

    return res;
}

const char *ClustererExample::GetInfoString()
{
    char *text = new char[1024];
    sprintf(text, "My Clusterer Example\n");
    sprintf(text, "\n");
    sprintf(text, "Training informations:\n");

    // here you can fill in whatever information you want

    return text;
}
