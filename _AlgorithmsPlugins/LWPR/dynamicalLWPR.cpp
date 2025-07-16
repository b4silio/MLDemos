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
#include "dynamicalLWPR.h"

using namespace std;

DynamicalLWPR::DynamicalLWPR()
: initD(50), initAlpha(250), wGen(0.2), model(0)
{
	type = DYN_LWPR;
}

void DynamicalLWPR::Train(std::vector< std::vector<fvec> > trajectories, ivec labels)
{
	if(!trajectories.size()) return;
	int count = trajectories[0].size();
	if(!count) return;
	dim = trajectories[0][0].size();
	// we forget about time and just push in everything
	vector<fvec> samples;
	FOR(i, trajectories.size())
	{
		FOR(j, trajectories[i].size())
		{
			samples.push_back(trajectories[i][j]);
		}
	}
	if(!samples.size()) return;
	dim = samples[0].size() / 2;
	DEL(model);
	model = new LWPR_Object(dim,dim);
	model->setInitD(initD);
	model->setInitAlpha(initAlpha);
	model->wGen(wGen);

	dvec x;
	dvec y;
	x.resize(dim);
	y.resize(dim);
	FOR(i, samples.size())
	{
		FOR(d,dim) x[d] = samples[i][d];
		FOR(d,dim) y[d] = samples[i][dim + d];
		model->update(x,y);
	}
}

std::vector<fvec> DynamicalLWPR::Test( const fvec &sample, const int count)
{
	fvec start = sample;
	dim = sample.size();
	std::vector<fvec> res;
	res.resize(count);
	FOR(i, count) res[i].resize(dim,0);
	if(!model) return res;
    dvec x(dim,0);
	fvec velocity; velocity.resize(dim,0);
	FOR(i, count)
	{
		res[i] = start;
		start += velocity*dT;

		FOR(d, dim) x[d] = start[d];
		dvec y = model->predict(x);
		FOR(d, dim) velocity[d] = y[d];
	}
	return res;
}

fvec DynamicalLWPR::Test( const fvec &sample)
{
	int dim = sample.size();
    fvec res(dim,0);
	if(!model) return res;
    dvec x(dim,0);
	FOR(d, dim) x[d] = sample[d];
	dvec y = model->predict(x);
	FOR(d, dim) res[d] = y[d];
	return res;
}

fVec DynamicalLWPR::Test( const fVec &sample)
{
	int dim = 2;
	fVec res;
	if(!model) return res;
	dvec x;
	x.resize(dim,0);
	FOR(d, dim) x[d] = sample._[d];
	dvec y = model->predict(x);
	FOR(d, dim) res[d] = y[d];
	return res;
}

void DynamicalLWPR::SetParams(double initD, double initAlpha, double wGen)
{
	this->initD = initD;
	this->initAlpha = initAlpha;
	this->wGen = wGen;
}

const char *DynamicalLWPR::GetInfoString()
{
	char *text = new char[1024];
	sprintf(text, "Locally Weighted Projection Regression\n");
	sprintf(text, "%sGeneration Threshold: %f\n", text, model->wGen());
	sprintf(text, "%sLambda (start: %f end: %f)\n", text, model->initLambda(), model->finalLambda());
	sprintf(text, "%sPenalty: %f\n", text, model->penalty());
	sprintf(text, "%sReceptive Fields: %d\n", text, model->numRFS()[0]);
	return text;
}
