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
    model = new LWPR_Object(dim-1,1);
	model->setInitD(initD);
	model->setInitAlpha(initAlpha);
	model->wGen(wGen);

	dvec x;
	dvec y;
    x.resize(dim-1);
	y.resize(1);
	FOR(i, samples.size())
	{
        FOR(d, dim-1) x[d] = samples[i][d];
        if(outputDim != -1 && outputDim < dim-1)
        {
            x[outputDim] = samples[i][dim-1];
            y[0] = samples[i][outputDim];
        }
        else y[0] = samples[i][dim-1];
		model->update(x,y);
	}
}

fvec RegressorLWPR::Test( const fvec &sample)
{
	fvec res;
	res.resize(2,0);
	if(!model) return res;
	float sigma = 0;
    int dim = sample.size();
    dvec x(dim-1);
    FOR(d, dim-1) x[d] = sample[d];
    if(outputDim != -1 && outputDim < dim-1)
    {
        x[outputDim] = sample[dim-1];
    }
    dvec y = model->predict(x, sigma);
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
	char *text = new char[1024];
	sprintf(text, "Locally Weighted Projection Regression\n");
	if(!model) return text;
	sprintf(text, "%sGeneration Threshold: %f\n", text, model->wGen());
	sprintf(text, "%sLambda (start: %f end: %f)\n", text, model->initLambda(), model->finalLambda());
	sprintf(text, "%sPenalty: %f\n", text, model->penalty());
        sprintf(text, "%sReceptive Fields: %d\n", text, model->numRFS()[0]);
	return text;
}
