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
#include "regressorExample.h"

using namespace std;

void RegressorExample::Train(std::vector< fvec > samples, ivec labels)
{
	if(!samples.size()) return;
    dim = samples[0].size();

    // outputdim is the dimension that we want to estimate, and it might NOT be the last one in the dataset
    if(outputDim != -1 && outputDim < dim-1)
    {
        // we need to swap the current last dimension with the desired output
        FOR(i, samples.size())
        {
            float val = samples[i][dim-1];
            samples[i][dim-1] = samples[i][outputDim];
            samples[i][outputDim] = val;
        }
    }

    // here you will train the regressor with your data
}

fvec RegressorExample::Test( const fvec &sample)
{
    fvec res;
    res.resize(2,0);
    res[0] = drand48()-0.5; // the regression estimation
    res[1] = drand48()*0.1; // the regression confidence (0 if you don't have one)
	return res;
}

const char *RegressorExample::GetInfoString()
{
    char *text = new char[1024];
    sprintf(text, "My Regressor Example\n");
    sprintf(text, "\n");
    sprintf(text, "Training information:\n");

    // here you can fill in whatever information you want

    return text;
}
