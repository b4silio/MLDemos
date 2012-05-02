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
#include "maximizePower.h"
#include <QDebug>

using namespace std;

MaximizePower::MaximizePower()
{
	data = 0;
	dim = 2;
	maximum.resize(dim);
	lastSigma.resize(dim,0);
	FOR(d,dim) maximum[d] = rand()/(float)RAND_MAX;
	variance = 0;
	k = 10;
}

MaximizePower::~MaximizePower()
{
	KILL(data);
}

void MaximizePower::SetParams(int k, float variance, bool bAdaptive)
{
	this->variance = variance;
	this->k = k;
	this->bAdaptive = bAdaptive;
	lastSigma = fvec();
	lastSigma.resize(dim, variance*variance);
}

void MaximizePower::Draw(QPainter &painter)
{
	painter.setPen(QPen(Qt::black, 1.5));
	painter.setBrush(Qt::NoBrush);
	FOR(i, visited.size())
	{
		QPointF point(visited[i][0]*w, visited[i][1]*h);
		painter.drawEllipse(point, 3, 3);
	}

	FOR(i, history.size()-1 )
	{
		QPointF point(history[i][0]*w, history[i][1]*h);
		QPointF pointNext(history[i+1][0]*w, history[i+1][1]*h);

		painter.setBrush(Qt::NoBrush);
		painter.drawLine(point, pointNext);
		painter.setBrush(QColor(255,255,255));
		painter.drawEllipse(point, 4, 4);
	}
	// we draw the current maximum
	QPointF point(history[history.size()-1][0]*w, history[history.size()-1][1]*h);
	painter.setBrush(QColor(0,255,0));
	painter.drawEllipse(point, 5, 5);

	painter.setBrush(QColor(0,255,0));
	FOR(i, best.size())
	{
		QPointF point(best[i].second.first[0]*w, best[i].second.first[1]*h);
		painter.drawEllipse(point, 3,3);
	}

	// draw the current sigma
	if(variance > 0)
	{
		QPointF maxPoint(maximum[0]*w, maximum[1]*h);
		QPointF radius(sqrtf(lastSigma[0])*w, sqrtf(lastSigma[1])*h);
		painter.setBrush(Qt::NoBrush);
		painter.setPen(QPen(Qt::black,1.5));
		painter.drawEllipse(maxPoint, radius.x(),radius.y());
		painter.setPen(QPen(Qt::black,0.5));
		painter.drawEllipse(maxPoint, radius.x()*2,radius.y()*2);
	}
}

void MaximizePower::Train(float *dataMap, fVec size, fvec startingPoint)
{
	w = size.x;
	h = size.y;
	if(data) delete [] data;
	best.clear();
	history.clear();
	historyValue.clear();
	data = new float[w*h];
	memcpy(data, dataMap, w*h*sizeof(float));
	bConverged = false;
	if(startingPoint.size())
	{
		maximum = startingPoint;
		float value = GetValue(startingPoint);
		maximumValue = value;
		history.push_back(maximum);
		HistoryValue().push_back(value);
		//qDebug() << "Starting maximization at " << maximum[0] << " " << maximum[1];
	}
	evaluations = 0;
}

fvec MaximizePower::Test( const fvec &sample)
{
	if(bConverged) return maximum;

	fvec newSample = sample;
	if(!sample.size()) newSample = maximum;

	if(bAdaptive && best.size() <= k)
	{
		fvec sigma;sigma.resize(dim,variance);
		while(best.size() < k)
		{
			fvec randSample;randSample.resize(dim);
			FOR(d, dim)
			{
				int tries = 64;
				do
				{
					randSample[d] = newSample[d] + RandN(0.f, variance);
				} while(randSample[d] < 0 || randSample[d] > 1.f || tries-- > 0);
			}
			visited.push_back(randSample);
			float value = GetValue(randSample);
			evaluations++;
			best.push_back(make_pair(value, make_pair(randSample, sigma)));
		}
		std::sort(best.begin(), best.end());
	}

	fvec randSample; randSample.resize(dim);

	FOR(d, dim)
	{
		int tries = 64;
		do
		{
			if(bAdaptive) randSample[d] = newSample[d] + RandN(0.f, sqrtf(lastSigma[d]));
			else randSample[d] = newSample[d] + RandN(0.f, variance);
		} while(randSample[d] < 0 || randSample[d] > 1.f || tries-- > 0);
		if(randSample[d] != randSample[d]) // nan!
		{
			randSample[d] = drand48();
		}
	}

	newSample = randSample;
	float value = GetValue(randSample);
	evaluations++;
	visited.push_back(newSample);

	if(!bAdaptive)
	{
		fvec sigma; sigma.resize(2, variance);
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

		maximum = fvec(); maximum.resize(dim,0);
		maximumValue = 0;
		FOR(i, best.size())
		{
			FOR(d, dim)
			{
				float value = best[i].first;
				fvec sample = best[i].second.first;
				maximum += (sample*value);
				maximumValue += value;
			}
		}
		if(maximumValue == 0)
		{
			maximum = newSample;
		}
		else
		{
			maximum /= maximumValue;
		}
		FOR(d, dim) maximum[d] = max(0.f, min(1.f,maximum[d]));
		history.push_back(maximum);
		historyValue.push_back(GetValue(maximum));
		//newSample = maximum;
	}
	else
	{
		fvec current = newSample;
		maximum = fvec(); maximum.resize(dim,0);
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
				maximum[d] += delta*reward / var;
				totalMaximum[d] +=  reward / var;

				varianceSum[d] += delta*delta*reward;
				totalVarianceSum[d] += reward;
			}
		}
		FOR(d, dim) maximum[d] = current[d] + maximum[d]/totalMaximum[d];
		FOR(d, dim) maximum[d] = max(0.f, min(1.f,maximum[d]));
		history.push_back(maximum);
		historyValue.push_back(GetValue(maximum));

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

fvec MaximizePower::Test(const fVec &sample)
{
	return Test((fvec)sample);
}

const char *MaximizePower::GetInfoString()
{
	char *text = new char[1024];
	sprintf(text, "POWER\n");
	return text;
}
