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
#include "basicMath.h"
#include "reinforcementPower.h"
#include <QDebug>

using namespace std;

ReinforcementPower::ReinforcementPower()
{
	dim = 2;
	maximum.resize(dim);
	lastSigma.resize(dim,0);
    FOR(d,dim) maximum[d] = drand48();
    maximumValue = -FLT_MAX;
	variance = 0;
	k = 10;
}

ReinforcementPower::~ReinforcementPower()
{
}

void ReinforcementPower::SetParams(int k, float variance, bool bAdaptive)
{
	this->variance = variance;
	this->k = k;
	this->bAdaptive = bAdaptive;
	lastSigma = fvec();
	lastSigma.resize(dim, variance*variance);
}

void ReinforcementPower::Draw(QPainter &painter)
{
}

void ReinforcementPower::Initialize(ReinforcementProblem *problem)
{
    this->problem = problem;
    bConverged = false;
    dim = problem->gridSize*problem->gridSize;
    directions = problem->directions;
    lastSigma.resize(dim, variance*variance);

    best.clear();
    history.clear();
    historyValue.clear();

    maximum = directions;
    float value = problem->GetReward(maximum);
    maximumValue = 0;
    history.push_back(maximum);
    HistoryValue().push_back(value);
    evaluations = 0;
}

fvec ReinforcementPower::Update()
{
	if(bConverged) return maximum;

    if(maximum.size() < dim)
    {
        maximum.resize(dim);
    }
    fvec newSample = maximum;
    int quantizeType = problem->quantizeType;
	if(bAdaptive && best.size() <= k)
	{
		fvec sigma;sigma.resize(dim,variance);
		while(best.size() < k)
		{
            switch(quantizeType)
            {
            case 0:
            {
                fvec randSample = newSample + RandN(dim, 0, variance);
                FOR(d, dim)
                {
                    while(randSample[d] < 0) randSample[d] += 2*M_PI;
                    while(randSample[d] > 2*M_PI) randSample[d] -= 2*M_PI;
                }
                newSample = randSample;
            }
                break;
            case 1:
            {
                FOR(d, dim) newSample[d] = (int)(newSample[d] + rand()%8) % 9;
            }
                break;
            case 2:
            {
                FOR(d, dim) newSample[d] = (int)(newSample[d] + rand()%4) % 5;
            }
                break;
            }
            visited.push_back(newSample);
            float value = problem->GetReward(newSample);
			evaluations++;
            best.push_back(make_pair(value, make_pair(newSample, sigma)));
		}
		std::sort(best.begin(), best.end());
	}

    FOR(d, dim)
	{
        switch(quantizeType)
        {
        case 0:
        {
            fvec randSample; randSample.resize(dim);
            if(bAdaptive) randSample = newSample + RandN(dim, 0.f, sqrtf(lastSigma[d]));
            else randSample = newSample + RandN(dim, 0, variance);
            FOR(d, dim)
            {
                while(randSample[d] < 0) randSample[d] += 2*M_PI;
                while(randSample[d] > 2*M_PI) randSample[d] -= 2*M_PI;
            }
            newSample = randSample;
        }
            break;
        case 1:
        {
            if(bAdaptive) FOR(d, dim) newSample[d] = (int)(newSample[d] + rand()%(int)(8*lastSigma[d])) % 9;
            else FOR(d, dim) newSample[d] = (int)(newSample[d] + rand()%8) % 9;
        }
            break;
        case 2:
        {
            if(bAdaptive) FOR(d, dim) newSample[d] = (int)(newSample[d] + rand()%(int)(4*lastSigma[d])) % 5;
            else FOR(d, dim) newSample[d] = (int)(newSample[d] + rand()%4) % 5;
        }
            break;
        }
	}

    float value = problem->GetReward(newSample);
	evaluations++;
	visited.push_back(newSample);

	if(!bAdaptive)
	{
        fvec sigma; sigma.resize(dim, variance);
		if(best.size() < k)
		{
			best.push_back(make_pair(value, make_pair(newSample, sigma)));
			std::sort(best.begin(), best.end());
		}
		else if(value > best[0].first)
		{
			best[0] = make_pair(value, make_pair(newSample, sigma));
			std::sort(best.begin(), best.end());
		}

        fvec newMaximum(dim,0);
		maximumValue = 0;
		FOR(i, best.size())
		{
            float value = best[i].first;
            fvec sample = best[i].second.first;
            newMaximum += (sample*value);
            maximumValue += value;
        }
		if(maximumValue == 0)
		{
            newMaximum = newSample;
		}
		else
		{
            newMaximum /= maximumValue;
		}
        switch(quantizeType)
        {
        case 0:
            FOR(d, dim) newMaximum[d] = max(0.f, min((float)M_PI,newMaximum[d]));
            break;
        case 1:
            FOR(d, dim) newMaximum[d] = (int)(newMaximum[d])%9;
            break;
        case 2:
            FOR(d, dim) newMaximum[d] = (int)(newMaximum[d])%5;
            break;
        }
        float value = problem->GetReward(newMaximum);
        if(value > maximumValue)
        {
            maximum = newMaximum;
            maximumValue = value;
            qDebug() << "maximum value" << value;
            history.push_back(newMaximum);
            historyValue.push_back(value);
        }
        newSample = newMaximum;
    }
	else
	{
		fvec current = newSample;
        fvec newMaximum = fvec(); newMaximum.resize(dim,0);
		fvec totalMaximum; totalMaximum.resize(dim,0);
		fvec delta;delta.resize(dim,0);
		fvec varianceSum;varianceSum.resize(dim,0);
		fvec totalVarianceSum;totalVarianceSum.resize(dim,0);
		FOR(i, best.size())
		{
			FOR(d, dim)
			{
				float delta = best[i].second.first[d] - current[d];
				float var = best[i].second.second[d];
				float reward = best[i].first;
                newMaximum[d] += delta*reward / var;
				totalMaximum[d] +=  reward / var;

				varianceSum[d] += delta*delta*reward;
				totalVarianceSum[d] += reward;
			}
		}
        FOR(d, dim) newMaximum[d] = current[d] + newMaximum[d]/totalMaximum[d];
        switch(quantizeType)
        {
        case 0:
            FOR(d, dim) newMaximum[d] = max(0.f, min((float)M_PI,newMaximum[d]));
            break;
        case 1:
            FOR(d, dim) newMaximum[d] = (int)(newMaximum[d])%9;
            break;
        case 2:
            FOR(d, dim) newMaximum[d] = (int)(newMaximum[d])%5;
            break;
        }
        float value = problem->GetReward(newMaximum);
        if(value > maximumValue)
        {
            maximum = newMaximum;
            maximumValue = value;
            qDebug() << "maximum value" << value;
            history.push_back(newMaximum);
            historyValue.push_back(value);
        }
        newSample = newMaximum;

		fvec sigma; sigma.resize(2, variance);
		FOR(d, dim) sigma[d] = varianceSum[d] / totalVarianceSum[d];
		if(value > best[0].first)
		{
			best[0] = make_pair(value, make_pair(newSample, sigma));
			std::sort(best.begin(), best.end());
		}
		lastSigma = sigma;
	}
	return newSample;
}

const char *ReinforcementPower::GetInfoString()
{
	char *text = new char[1024];
	sprintf(text, "POWER\n");
	return text;
}
