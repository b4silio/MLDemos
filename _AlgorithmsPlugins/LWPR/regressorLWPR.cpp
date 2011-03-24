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
#include "basicOpenCV.h"
#include "regressorLWPR.h"

using namespace std;

RegressorLWPR::RegressorLWPR()
: initD(50), initAlpha(250), wGen(0.2), model(0)
{
	type = REGR_LWPR;
}

void RegressorLWPR::Train(std::vector< fvec > samples, ivec labels)
{
	if(!samples.size()) return;
	int dim = samples[0].size();
	DEL(model);
	model = new LWPR_Object(1,1);
	model->setInitD(initD);
	model->setInitAlpha(initAlpha);
	model->wGen(wGen);

	dvec x;
	dvec y;
	x.resize(1);
	y.resize(1);
	FOR(i, samples.size())
	{
		x[0] = samples[i][0];
		y[0] = samples[i][1];
		model->update(x,y);
	}
}

fvec RegressorLWPR::Test( const fvec &sample)
{
	fvec res;
	res.resize(2,0);
	if(!model) return res;
	float estimate;
	float sigma = 0;
	dvec x;
	x.push_back(sample[0]);
	dvec y = model->predict(x);
	res[0] = y[0];
	res[1] = sigma;
	return res;
}

void RegressorLWPR::SetParams(double initD, double initAlpha, double wGen)
{
	this->initD = initD;
	this->initAlpha = initAlpha;
	this->wGen = wGen;
}

char *RegressorLWPR::GetInfoString()
{
	char *text = new char[255];
	sprintf(text, "Locally Weighted Projection Regression\n");
	if(!model) return text;
	sprintf(text, "%sGeneration Threshold: %f\n", text, model->wGen());
	sprintf(text, "%sLambda (start: %f end: %f)\n", text, model->initLambda(), model->finalLambda());
	sprintf(text, "%sPenalty: %f\n", text, model->penalty());
        sprintf(text, "%sReceptive Fields: %d\n", text, model->numRFS()[0]);
	return text;
}
