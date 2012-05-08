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
	dim = 2;
	variance = 0;
    bSingleDim = false;
    maximum = directions = fvec();
}

ReinforcementRandom::~ReinforcementRandom()
{
}

void ReinforcementRandom::SetParams(float variance, bool bSingleDim)
{
	this->variance = variance;
    this->bSingleDim = bSingleDim;
}

void ReinforcementRandom::Initialize(ReinforcementProblem *problem)
{
    this->problem = problem;
	bConverged = false;
    dim = problem->gridSize*problem->gridSize;
    directions = problem->directions;

    maximum = directions;
    float value = problem->GetReward(maximum);
    maximumValue = 0;
    history.push_back(maximum);
    HistoryValue().push_back(value);
	evaluations = 0;
}

fvec ReinforcementRandom::Update()
{
	if(bConverged) return maximum;
    int quantizeType = problem->quantizeType;
	fvec newSample;
	if(variance == 0) // we're doing random search
	{
        newSample.resize(directions.size());
        if(quantizeType) FOR(d, directions.size()) newSample[d] = rand() % (quantizeType==1?9:5);
        else FOR(d, directions.size()) newSample[d] = drand48()*2*M_PI;
	}
	else // we're doing random walk
    {
        newSample = directions;

        fvec randSample; randSample.resize(directions.size());
		bool bInsideLimits = true;
        if(bSingleDim)
        {
            // we randomly pick one dimension
            int index = rand()%(directions.size());
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
                newSample[index] = (int)(newSample[index] + rand()%8) % 9;
            }
                break;
            case 2:
            {
                newSample[index] = (int)(newSample[index] + rand()%4) % 5;
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
                randSample = newSample + RandN(directions.size(), 0, variance);
                FOR(d, directions.size())
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
                FOR(d, directions.size()) newSample[d] = (int)(newSample[d] + rand()%8) % 9;
            }
                break;
            case 2:
            {
                FOR(d, directions.size()) newSample[d] = (int)(newSample[d] + rand()%4) % 5;
            }
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
        //qDebug() << "new maximum found at " << maximum[0] << " \tvalue: " << value;
	}
    directions = maximum;
	history.push_back(maximum);
    historyValue.push_back(maximumValue);
	return newSample;
}

void ReinforcementRandom::Draw(QPainter &painter)
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

const char *ReinforcementRandom::GetInfoString()
{
	char *text = new char[1024];
	if(variance == 0) sprintf(text, "Random Search");
	else sprintf(text, "Random Walk\n");
	return text;
}
