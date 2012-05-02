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
#include <drawUtils.h>
#include "reinforcementRandom.h"
#include <QDebug>

using namespace std;

ReinforcementRandom::ReinforcementRandom()
{
	data = 0;
	dim = 2;
	variance = 0;
    gridSize = 4;
    bSingleDim = false;
    directions.resize(gridSize*gridSize);
    if(quantizeType) FOR(d, gridSize*gridSize) directions[d] = rand()%(quantizeType==1 ? 8 : 4);
    else FOR(d, gridSize*gridSize) directions[d] = drand48()*2*M_PI;
    maximum = directions;
}

ReinforcementRandom::~ReinforcementRandom()
{
	KILL(data);
}

void ReinforcementRandom::SetParams(float variance, int policyType, int gridSize, bool bSingleDim, int quantizeType)
{
	this->variance = variance;
    this->policyType = policyType;
    this->gridSize = gridSize;
    this->bSingleDim = bSingleDim;
    this->quantizeType = quantizeType;
    directions.resize(gridSize*gridSize);
}

void ReinforcementRandom::Draw(QPainter &painter)
{
    int w = painter.viewport().width(), h = painter.viewport().height();
    int xSize = w / gridSize, ySize = h / gridSize;
    painter.setPen(QPen(Qt::black, 1));
	painter.setBrush(Qt::NoBrush);
    switch(policyType)
    {
    case 0:
    {
        // we draw a grid of the different 'cases'
        FOR(i, gridSize*gridSize)
        {
            int xIndex = i%gridSize;
            int yIndex = i/gridSize;
            int x = xIndex*w/gridSize, y = yIndex*h/gridSize;
            painter.setPen(QPen(Qt::black, 0.5));
            painter.drawRect(x+xSize*0.05, y+ySize*0.05, xSize*.9, ySize*.9);
            painter.setPen(QPen(Qt::gray, 1));
            QPointF point1, point2;
            if(quantizeType)
            {
                fvec s(2,0);
                s[0] = xIndex/(float)gridSize;
                s[1] = yIndex/(float)gridSize;
                fvec s2 = NextStep(s, directions);
                point1 = QPointF(s2[0]-s[0], s2[1]-s[1]);
                float len = sqrtf(point1.x()*point1.x()+point1.y()*point1.y());
                point1 = QPointF(point1.x()/len, point1.y()/len);
                s2 = NextStep(s, maximum);
                point2 = QPointF(s2[0]-s[0], s2[1]-s[1]);
                len = sqrtf(point2.x()*point2.x()+point2.y()*point2.y());
                point2 = QPointF(point2.x()/len, point2.y()/len);
            }
            else
            {
                point1 = QPointF(cosf(directions[i]), sinf(directions[i]));
                point2 = QPointF(cosf(maximum[i]), sinf(maximum[i]));
            }
            DrawArrow(QPointF(x+xSize/2 - point1.x()*xSize/5, y+ySize/2 - point1.y()*ySize/5),
                      QPointF(x+xSize/2 + point1.x()*xSize/5, y+ySize/2 + point1.y()*ySize/5), xSize/10., painter);
            //painter.setBrush(Qt::gray);
            //painter.drawEllipse(QPointF(x+xSize/2 + point.x()*xSize/5, y+ySize/2 + point.y()*ySize/5), 3, 3);

            painter.setPen(QPen(Qt::black, 1));
            DrawArrow(QPointF(x+xSize/2 - point2.x()*xSize/5, y+ySize/2 - point2.y()*ySize/5),
                      QPointF(x+xSize/2 + point2.x()*xSize/5, y+ySize/2 + point2.y()*ySize/5), xSize/10., painter);
            //painter.setBrush(Qt::black);
            //painter.drawEllipse(QPointF(x+xSize/2 + point.x()*xSize/5, y+ySize/2 + point.y()*ySize/5), 3, 3);
        }
    }
        break;
    }
}

fvec ReinforcementRandom::NextStep(fvec sample, fvec directions)
{
    int index = 0;
    int mult = 0;
    int xIndex = sample[0]*gridSize;
    int yIndex = sample[1]*gridSize;
    // the policy direction at state (xIndex,yIndex)
    float direction = directions[yIndex*gridSize + xIndex]; // direction, in radians
    // we move one case in the direction determined by the policy
    switch(quantizeType)
    {
        case 0:
        {
            sample[0] += cosf(direction)*(0.5f/gridSize);
            sample[1] += sinf(direction)*(0.5f/gridSize);
        }
            break;
        case 1:
        {
            switch((int)direction)
            {
            case 0:
                sample[0] += 1./gridSize;
            break;
            case 1:
                sample[0] += 1./gridSize;
                sample[1] += 1./gridSize;
            break;
            case 2:
                sample[1] += 1./gridSize;
            break;
            case 3:
                sample[0] -= 1./gridSize;
                sample[1] += 1./gridSize;
            break;
            case 4:
                sample[0] -= 1./gridSize;
            break;
            case 5:
                sample[0] -= 1./gridSize;
                sample[1] -= 1./gridSize;
            break;
            case 6:
                sample[1] -= 1./gridSize;
            break;
            case 7:
                sample[0] += 1./gridSize;
                sample[1] -= 1./gridSize;
            break;
            }
        }
            break;
        case 2:
        {
            switch((int)direction)
            {
            case 0:
                sample[0] += 1./gridSize;
            break;
            case 1:
                sample[1] += 1./gridSize;
            break;
            case 2:
                sample[0] -= 1./gridSize;
            break;
            case 3:
                sample[1] -= 1./gridSize;
            break;
            }
        }
            break;
    }

    // we bound the space to a 0-1 range
    sample[0] = min(1.f, max(0.f, sample[0]));
    sample[1] = min(1.f, max(0.f, sample[1]));
    return sample;
}

fvec ReinforcementRandom::PerformAction(fvec sample)
{
    return NextStep(sample, directions);
}

float ReinforcementRandom::GetReward()
{
    return GetReward(directions);
}

float ReinforcementRandom::GetReward(fvec directions)
{
    fvec backup = this->directions;
    this->directions = directions;
    fvec sample(dim);
    float fullReward = 0;
    FOR(i, gridSize*gridSize)
    {
        sample[0] = (i%gridSize + 0.5f)/(float)gridSize;
        sample[1] = (i/gridSize + 0.5f)/(float)gridSize;
        fullReward += GetSimulationValue(sample);
    }
    fullReward /= (gridSize*gridSize);
    this->directions = backup;
    return fullReward;
}

void ReinforcementRandom::Train(float *dataMap, fVec size, fvec startingPoint)
{
	w = size.x;
	h = size.y;
	if(data) delete [] data;
	data = new float[w*h];
	memcpy(data, dataMap, w*h*sizeof(float));
	bConverged = false;

    if(quantizeType) FOR(d, gridSize*gridSize) directions[d] = rand()%(quantizeType==1?8:4);
    else FOR(d, gridSize*gridSize) directions[d] = drand48()*2*M_PI;

    if(startingPoint.size())
	{
		maximum = startingPoint;
        float value = GetReward(maximum);
        maximumValue = 0;
		history.push_back(maximum);
		HistoryValue().push_back(value);
		//qDebug() << "Starting maximization at " << maximum[0] << " " << maximum[1];
	}
	evaluations = 0;
}

fvec ReinforcementRandom::Test( const fvec &sample)
{
	if(bConverged) return maximum;
	fvec newSample;
	if(variance == 0) // we're doing random search
	{
        newSample.resize(gridSize*gridSize);
        if(quantizeType) FOR(d, gridSize*gridSize) newSample[d] = rand() % (quantizeType==1?8:4);
        else FOR(d, gridSize*gridSize) newSample[d] = drand48()*2*M_PI;
	}
	else // we're doing random walk
	{
        if(sample.size() < gridSize*gridSize) newSample = directions;
        else newSample = sample;
//		newSample = sample;
//		if(!sample.size()) newSample = maximum;

        fvec randSample; randSample.resize(gridSize*gridSize);
		int tries = 64;
		bool bInsideLimits = true;
        if(bSingleDim)
        {
            // we randomly pick one dimension
            int index = rand()%(gridSize*gridSize);
            switch(quantizeType)
            {
            case 0:
            {
                newSample[index] += RandN(1, 0, variance)[0];
                while(newSample[index] < 0) newSample[index] += 2*M_PI;
                while(newSample[index] > 2*M_PI) newSample[index] -= 2*M_PI;
            }
                break;
            case 1:
            {
                newSample[index] = (int)(newSample[index] + rand()%((int)(variance*8))) % 8;
            }
                break;
            case 2:
            {
                newSample[index] = (int)(newSample[index] + rand()%((int)(variance*4))) % 4;
            }
                break;
            }
        }
        else
        {
            switch(quantizeType)
            {
            case 0:
            {
                randSample = newSample + RandN(gridSize*gridSize, 0, variance);
                FOR(d, gridSize*gridSize)
                {
                    while(randSample[d] < 0) randSample[d] += 2*M_PI;
                    while(randSample[d] > 2*M_PI) randSample[d] -= 2*M_PI;
                    bInsideLimits &= randSample[d] >= 0.f && randSample[d] <= 2*M_PI;
                }
                newSample = randSample;
            }
                break;
            case 1:
            {
                FOR(d, gridSize*gridSize) newSample[d] = (int)(newSample[d] + rand()%((int)(variance*8))) % 8;
            }
                break;
            case 2:
            {
                FOR(d, gridSize*gridSize) newSample[d] = (int)(newSample[d] + rand()%((int)(variance*4))) % 4;
            }
                break;
            }
        }
	}
	visited.push_back(newSample);
    float value = GetReward(newSample);
	evaluations++;
	if(value > maximumValue)
	{
        maximum = newSample;
		maximumValue = value;
        qDebug() << "new maximum found at " << maximum[0] << " \tvalue: " << value;
	}
    directions = maximum;
	history.push_back(maximum);
    historyValue.push_back(maximumValue);
	return newSample;
}

fvec ReinforcementRandom::Test(const fVec &sample)
{
	return Test((fvec)sample);
}

const char *ReinforcementRandom::GetInfoString()
{
	char *text = new char[1024];
	if(variance == 0) sprintf(text, "Random Search");
	else sprintf(text, "Random Walk\n");
	return text;
}
