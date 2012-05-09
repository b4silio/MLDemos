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
    bSingleDim = true;
    bAdaptive = true;
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
    lastSigma.resize(dim, variance);
}

void ReinforcementPower::Draw(QPainter &painter)
{
    int w = painter.viewport().width(), h = painter.viewport().height();
    int graphW = 200, graphH = 100, graphPad = 10;
    int top = h - 10 - (graphH + 2*graphPad);
    int left = 10;
    QPainter::RenderHints hints = painter.renderHints();
    painter.setRenderHint(QPainter::Antialiasing, false);
    QFont font = painter.font();
    font.setPointSize(9);
    painter.setFont(font);

    // we draw the rectangle behind
    painter.setOpacity(1);
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(Qt::black, 2));
    painter.drawRect(left, top, graphW + 2*graphPad, graphH + 2*graphPad);
    painter.setOpacity(0.6);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::white);
    painter.drawRect(left, top, graphW + 2*graphPad, graphH + 2*graphPad);
    painter.setOpacity(1);
    painter.setBrush(Qt::black);
    painter.setPen(Qt::black);

    // we draw the values
    double maxValue = -DBL_MAX;
    FOR(i, historyValue.size()) maxValue = max(maxValue, historyValue[i]);
    int valueLimit = 4;
    double upperBound = ((int)ceil(maxValue)/valueLimit + 1)*valueLimit;
    painter.setPen(QPen(Qt::black, 2));
    QPointF oldPoint;
    FOR(i, graphW)
    {
        int index = i*historyValue.size()/graphW;
        QPointF point(i, graphH*(1.f - (historyValue[index]/upperBound)));
        point += QPointF(left + graphPad, top + graphPad);
        if(i) painter.drawLine(point, oldPoint);
        if(i==graphW-1)
        {
            painter.drawText(point + QPointF(-20,0), QString("%1").arg(historyValue.back(), 0, 'f', 2));
        }
        oldPoint = point;
    }
    // we draw the axes
    painter.setPen(QPen(Qt::black, 2));
    painter.drawLine(left + graphPad, top+graphPad, left + graphPad, top+graphPad + graphH);
    painter.drawLine(left + graphPad, top+graphPad+graphH, left + graphPad + graphW, top+graphPad + graphH);
    painter.drawText(left + graphPad, top + graphPad, QString("%1").arg(upperBound, 0, 'f', 1));
    painter.drawText(left + graphPad, top + graphPad*2 + graphH, QString("0"));
    font.setPointSize(9);
    painter.setFont(font);
    painter.drawText(left, top, graphPad*2 + graphW, graphPad, Qt::AlignCenter, "Maximum Reward");
    painter.setRenderHints(hints);
}

void ReinforcementPower::Initialize(ReinforcementProblem *problem)
{
    this->problem = problem;
    bConverged = false;
    dim = problem->gridSize*problem->gridSize;
    directions = problem->directions;
    lastSigma.resize(dim, variance);

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

    int d = rand()%dim;
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
                fvec randSample = newSample;
                if(bSingleDim)
                {
                    randSample[d] + RandN(0.f, variance);
                    while(randSample[d] < 0) randSample[d] += 2*M_PI;
                    while(randSample[d] > 2*M_PI) randSample[d] -= 2*M_PI;
                }
                else
                {
                    randSample += RandN(dim, 0, variance);
                    FOR(d, dim)
                    {
                        while(randSample[d] < 0) randSample[d] += 2*M_PI;
                        while(randSample[d] > 2*M_PI) randSample[d] -= 2*M_PI;
                    }
                }
                newSample = randSample;
            }
                break;
            case 1:
            {
                if(bSingleDim)
                {
                    newSample[d] = (int)(newSample[d] + rand()%8) % 9;
                }
                else FOR(d, dim) newSample[d] = (int)(newSample[d] + rand()%8) % 9;
            }
                break;
            case 2:
            {
                if(bSingleDim)
                {
                    newSample[d] = (int)(newSample[d] + rand()%4) % 5;
                }
                else FOR(d, dim) newSample[d] = (int)(newSample[d] + rand()%4) % 5;
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

    switch(quantizeType)
    {
    case 0:
    {
        fvec randSample = newSample;
        if(bSingleDim)
        {
            if(bAdaptive) randSample[d] = newSample[d] + RandN(0.f, sqrtf(lastSigma[d]));
            else randSample[d] = newSample[d] + RandN(0.f, variance);
            while(randSample[d] < 0) randSample[d] += 2*M_PI;
            while(randSample[d] > 2*M_PI) randSample[d] -= 2*M_PI;
        }
        else
        {
            if(bAdaptive) FOR(d, dim) randSample[d] = newSample[d] + RandN(0.f, sqrtf(lastSigma[d]));
            else randSample = newSample + RandN(dim, 0, variance);
            FOR(d, dim)
            {
                while(randSample[d] < 0) randSample[d] += 2*M_PI;
                while(randSample[d] > 2*M_PI) randSample[d] -= 2*M_PI;
            }
        }
        newSample = randSample;
    }
        break;
    case 1:
    {
        if(bSingleDim)
        {
            int noise = bAdaptive ? (int)(8*lastSigma[d]) : 8;
            if(noise) newSample[d] = (int)(newSample[d] + rand()%noise) % 9;
        }
        else
        {
            FOR(d, dim)
            {
                int noise = bAdaptive ? (int)(8*lastSigma[d]) : 8;
                if(!noise) continue;
                newSample[d] = (int)(newSample[d] + rand()%noise) % 9;
            }
        }
    }
        break;
    case 2:
    {
        if(bSingleDim)
        {
            int noise = bAdaptive ? (int)(8*lastSigma[d]) : 4;
            if(noise) newSample[d] = (int)(newSample[d] + rand()%noise) % 5;
        }
        else
        {
            FOR(d, dim)
            {
                int noise = bAdaptive ? (int)(4*lastSigma[d]) : 4;
                if(!noise) continue;
                newSample[d] = (int)(newSample[d] + rand()%noise) % 5;
            }
        }
    }
        break;
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
            if(bSingleDim) newMaximum[d] = max(0.f, min((float)M_PI,newMaximum[d]));
            else FOR(d, dim) newMaximum[d] = max(0.f, min((float)M_PI,newMaximum[d]));
            break;
        case 1:
            if(bSingleDim) newMaximum[d] = (int)(newMaximum[d])%9;
            else FOR(d, dim) newMaximum[d] = (int)(newMaximum[d])%9;
            break;
        case 2:
            if(bSingleDim) newMaximum[d] = (int)(newMaximum[d])%5;
            else FOR(d, dim) newMaximum[d] = (int)(newMaximum[d])%5;
            break;
        }
        float value = problem->GetReward(newMaximum);
        maximum = best.back().second.first;
        maximumValue = best.back().first;
        history.push_back(maximum);
        historyValue.push_back(maximumValue);
        /*
        if(value > maximumValue)
        {
            maximum = newMaximum;
            maximumValue = value;
            qDebug() << "maximum value" << value;
            history.push_back(newMaximum);
            historyValue.push_back(value);
        }
        */
        newSample = newMaximum;
    }
    else // bAdaptive==true
	{
		fvec current = newSample;
        fvec newMaximum(dim,0.f);
        fvec totalMaximum(dim,0.f);
        fvec varianceSum(dim,0);
        fvec totalVarianceSum(dim,0);
		FOR(i, best.size())
		{
            if(bSingleDim)
            {
                float delta = best[i].second.first[d] - current[d];
                float var = best[i].second.second[d];
                float reward = best[i].first;
                newMaximum[d] += delta*reward / var;
                totalMaximum[d] +=  reward / var;

                varianceSum[d] += delta*delta*reward;
                totalVarianceSum[d] += reward;
            }
            else
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
        }

        if(bSingleDim) newMaximum[d] = current[d] + newMaximum[d]/totalMaximum[d];
        else FOR(d, dim) newMaximum[d] = current[d] + newMaximum[d]/totalMaximum[d];

        // sanity check!
        switch(quantizeType)
        {
        case 0:
            if(bSingleDim) newMaximum[d] = max(0.f, min((float)M_PI,newMaximum[d]));
            else FOR(d, dim) newMaximum[d] = max(0.f, min((float)M_PI,newMaximum[d]));
            break;
        case 1:
            if(bSingleDim) newMaximum[d] = (int)(newMaximum[d])%9;
            else FOR(d, dim) newMaximum[d] = (int)(newMaximum[d])%9;
            break;
        case 2:
            if(bSingleDim) newMaximum[d] = (int)(newMaximum[d])%5;
            else FOR(d, dim) newMaximum[d] = (int)(newMaximum[d])%5;
            break;
        }
        /*
        float valueMax = problem->GetReward(newMaximum);
        if(valueMax > maximumValue)
        {
            maximum = newMaximum;
            maximumValue = value;
            qDebug() << "maximum value" << value;
            history.push_back(newMaximum);
            historyValue.push_back(value);
        }
        */
        fvec sigma = lastSigma;
        //fvec sigma; sigma.resize(dim, variance);
        if(bSingleDim)
        {
            if(totalVarianceSum[d] != 0) sigma[d] = varianceSum[d] / totalVarianceSum[d];
        }
        else
        {
            FOR(d, dim)
            {
                if(totalVarianceSum[d] == 0) continue;
                sigma[d] = varianceSum[d] / totalVarianceSum[d];
            }
        }
		lastSigma = sigma;
        if(value > best[0].first)
        {
            best[0] = make_pair(value, make_pair(newSample, sigma));
            std::sort(best.begin(), best.end());
        }
        maximum = best.back().second.first;
        maximumValue = best.back().first;
        history.push_back(maximum);
        historyValue.push_back(maximumValue);
        //newSample = newMaximum;
	}
	return newSample;
}

const char *ReinforcementPower::GetInfoString()
{
	char *text = new char[1024];
	sprintf(text, "POWER\n");
	return text;
}
