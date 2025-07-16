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
#include "dynamicalExample.h"

using namespace std;

void DynamicalExample::Train(std::vector< std::vector<fvec> > trajectories, ivec labels)
{
	if(!trajectories.size()) return;
	int count = trajectories[0].size();
	if(!count) return;
	dim = trajectories[0][0].size()/2;

    // depending on the algorithm, you might want to do this or not

    // we forget about time and just push in everything in a list
	vector<fvec> samples;
	FOR(i, trajectories.size())
	{
		FOR(j, trajectories[i].size())
		{
			samples.push_back(trajectories[i][j]);
		}
	}
	if(!samples.size()) return;

    // here you will do your training part
}

std::vector<fvec> DynamicalExample::Test( const fvec &sample, int count)
{
    fvec position = sample;
	dim = sample.size();
	std::vector<fvec> res;
	res.resize(count);
	FOR(i, count) res[i].resize(dim,0);

	fvec velocity; velocity.resize(dim,0);
	FOR(i, count)
	{
        // we store the current position
        res[i] = position;
        // we compute the new velocity
        velocity = Test(position);
        // and we move the position accordingly
        position += velocity*dT;
	}
	return res;
}

fvec DynamicalExample::Test( const fvec &sample)
{
	dim = sample.size();
	fvec res; res.resize(dim, 0);

    // here we just return a random velocity
    res[0] = drand48()-0.5;
    res[1] = drand48()-0.5;

    return res;
}

const char *DynamicalExample::GetInfoString()
{
	char *text = new char[1024];
    sprintf(text, "My Dynamical Example\n");
    sprintf(text, "\n");
    sprintf(text, "Training information:\n");

    // here you can fill in whatever information you want

    return text;
}
