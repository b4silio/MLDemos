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
#include <algorithm>
#include "reinforcementDP.h"
#include <QDebug>

using namespace std;

ReinforcementDP::ReinforcementDP()
{
	dim = 2;
	variance = 0;
    bBatchUpdate = false;
    maximum = directions = fvec();
}

ReinforcementDP::~ReinforcementDP()
{
}

void ReinforcementDP::SetParams(float variance, bool bBatchUpdate)
{
	this->variance = variance;
    this->bBatchUpdate = bBatchUpdate;
}

void ReinforcementDP::Initialize(ReinforcementProblem *problem)
{
    this->problem = problem;
	bConverged = false;
    dim = problem->gridSize*problem->gridSize;
    directions = problem->directions;

    maximum = directions;
    float value = problem->GetReward(maximum);
    maximumValue = 0;
    history.push_back(maximum);
    historyValue.push_back(value);
	evaluations = 0;
}

fvec ReinforcementDP::Update()
{
	if(bConverged) return maximum;
    int quantizeType = problem->quantizeType;
    int gridSize = problem->gridSize;
    fvec newSample = maximum;
    fvec &stateValues = problem->stateValues;

    // for each tested state, we go through the whole thing and update the closest basis function
    FOR(x, gridSize)
    {
        FOR(y, gridSize)
        {
            int index = y*gridSize + x;
            // we look for the action values for each action
            float Q[8];
            Q[0] = x < gridSize-1 ? stateValues[(x+1) + (y)*gridSize] : 0.f;
            Q[1] = x < gridSize-1 && y < gridSize -1 ? stateValues[(x+1) + (y+1)*gridSize] : 0.f;
            Q[2] = y < gridSize-1 ? stateValues[(x) + (y+1)*gridSize] : 0.f;
            Q[3] = x>0 && y < gridSize-1 ? stateValues[(x-1) + (y+1)*gridSize] : 0.f;
            Q[4] = x>0 ? stateValues[(x-1) + (y)*gridSize] : 0.f;
            Q[5] = x>0 && y>0 ? stateValues[(x-1) + (y-1)*gridSize] : 0.f;
            Q[6] = y>0 ? stateValues[(x) + (y-1)*gridSize] : 0.f;
            Q[7] = x < gridSize-1 && y>0 ? stateValues[(x+1) + (y-1)*gridSize] : 0.f;
            int maxIndex=0;
            float maxVal = Q[0];
            FOR(i, 8)
            {
                if(maxVal < Q[i])
                {
                    maxVal = Q[i];
                    maxIndex = i;
                }
                if(quantizeType==2) i++;
            }
            switch(quantizeType)
            {
            case 0:
            {
                float angle=maxIndex*M_PI*2/8;
                newSample[index] = angle;
                //newSample[index] = newSample[index]*(1.f-variance) + maxIndex*M_PI*2/8*variance; // we compute the new direction in radians
            }
                break;
            case 1:
                newSample[index] = maxIndex;
                break;
            case 2:
                newSample[index] = maxIndex/2;
                break;
            }
        }
    }

	visited.push_back(newSample);
    float value = problem->GetReward(newSample);
	evaluations++;
    if(value > maximumValue)
	{
        maximum = newSample;
		maximumValue = value;
	}
    directions = maximum;
    history.push_back(maximum);
    historyValue.push_back(maximumValue);
    newSample = maximum;
    return newSample;
}

void ReinforcementDP::Draw(QPainter &painter)
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

const char *ReinforcementDP::GetInfoString()
{
	char *text = new char[1024];
    if(variance == 0) sprintf(text, "DP Search");
    else sprintf(text, "DP Walk\n");
	return text;
}
