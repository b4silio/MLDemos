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
#include "drawUtils.h"
#include "maximizeDonut.h"
#include <QDebug>
#include "mvnpdf.h"

using namespace std;

MaximizeDonut::MaximizeDonut()
{
	data = 0;
	dim = 2;
	maximum.resize(dim);
	lastSigma.resize(dim,0);
	FOR(d,dim) maximum[d] = rand()/(float)RAND_MAX;
	variance = 0;
	k = 10;
}

MaximizeDonut::~MaximizeDonut()
{
	KILL(data);
}

void MaximizeDonut::SetParams(int k, float variance, bool bAdaptive)
{
	this->fingerprint = variance * variance;
	this->variance = 0.2;
	this->k = k;
	this->bAdaptive = bAdaptive;
	lastSigma = fvec();
	lastSigma.resize(dim*(dim-1), 0);
	FOR(d,dim)
	{
		lastSigma[d] = variance;
	}
}

void MaximizeDonut::Draw(QPainter &painter)
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

	fvec sigma; sigma.resize(3);
	// we draw all the fingertips
	FOR(i, best.size())
	{
		sigma[0] = best[i].second.second[0];
		sigma[1] = best[i].second.second[1];
		sigma[2] = best[i].second.second[3];
		if(sigma[0] == sigma[0] && sigma[1] == sigma[1] && sigma[2] == sigma[2])
		{
			painter.setBrush(Qt::NoBrush);
			painter.setPen(QPen(Qt::black,.6));
			DrawEllipse(&best[i].second.first[0], &sigma[0], 1.f, &painter, QSize(w,h));
			painter.setPen(QPen(Qt::black,.2));
			DrawEllipse(&best[i].second.first[0], &sigma[0], 2.f, &painter, QSize(w,h));
		}
	}
	// and we draw the current search zone
	sigma[0] = lastSigma[0];
	sigma[1] = lastSigma[1];
	sigma[2] = lastSigma[3];
	if(sigma[0] == sigma[0] && sigma[1] == sigma[1] && sigma[2] == sigma[2])
	{
		painter.setBrush(Qt::NoBrush);
		painter.setPen(QPen(Qt::green,1.5));
		DrawEllipse(&maximum[0], &sigma[0], 1.f, &painter, QSize(w,h));
		painter.setPen(QPen(Qt::green,0.5));
		DrawEllipse(&maximum[0], &sigma[0], 2.f, &painter, QSize(w,h));
	}
}

void MaximizeDonut::Train(float *dataMap, fVec size, fvec startingPoint)
{
	w = size.x;
	h = size.y;
	KILL(data);
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

fvec MaximizeDonut::Generate(fvec sample, fvec sigma, float uniform)
{
	fvec newSample; newSample.resize(dim);

	if(uniform)
	{
		//FOR(d, dim) newSample[d] = drand48();
		newSample = sample + RandN(dim, 0, variance);
		return newSample;
	}
	bool bIsInside = false;
	int tries = 100;
	do
	{
		double u = 0.;
		double accept = 1.;
		int attempts = 0;
		do
		{
			// we do the rejection sampling
			// step 1: generate a sample from the positive distribution
			newSample = mvnRandN(sample, sigma);
			// step 2: test if we need to reject
			u = drand48();
			accept = 1.;
			FOR(i, best.size())
			{
				fvec mean = best[i].second.first;
				fvec sigma = best[i].second.second;
				float value = best[i].first;
				float pdf = mvnPdf(newSample, mean, sigma);
				accept *= 1. - pdf*(1-value);
			}
			attempts++;
		}
		while(attempts < 100 && u >= accept);
		//qDebug() << "generated after " << attempts << " attempts";

		bIsInside = true;
		FOR(d, dim) bIsInside &= newSample[d] >= 0.f && newSample[d] <=1.f;
	} while(tries-- && !bIsInside);

	return newSample;
}

fvec MaximizeDonut::GetBestMean()
{
	fvec mean; mean.resize(dim,0);
	float meanValue = 0;
	FOR(i, best.size())
	{
		float value = best[i].first;
		fvec sample = best[i].second.first;
		mean += (sample*value);
		meanValue += value;
	}
	if(meanValue != 0) mean /= meanValue;
	return mean;
}

fvec MaximizeDonut::GetBestSigma(fvec mean)
{
	fvec sigma; sigma.resize(dim*dim,0);
	float totalReward = 0;
	FOR(i, best.size())
	{
		float reward = best[i].first;
		fvec diff = best[i].second.first - mean;
		FOR(x, dim)
		{
			FOR(y, dim)
			{
				sigma[y*dim + x] += reward*diff[x]*diff[y];
			}
		}
		totalReward += reward;
	}
	FOR(d, sigma.size()) sigma[d] /= totalReward;
	//FOR(d, sigma.size()) sigma[d] /= best.size();
	return sigma;
}

QImage MaximizeDonut::DrawMap()
{
	int size = 200;
	QImage image(QSize(size,size), QImage::Format_ARGB32);
	fvec sample; sample.resize(2);
	FOR(i, size)
	{
		sample[0] = i/(float)size;
		FOR(j, size)
		{
			sample[1] = j/(float)size;
			float accept = 1.f;
			float positive = mvnPdf(sample, maximum, lastSigma);

			FOR(k, best.size())
			{
				fvec mean = best[k].second.first;
				fvec sigma = best[k].second.second;
				float value = mvnPdf(sample, mean, sigma);
				positive *= 1.f - min(1.f,value);
			}

			int val = max(0.f,min(255.f,(positive)*20));
			image.setPixel(i,j,qRgb(val,val,val));
		}
	}
	return image;
}

fvec MaximizeDonut::Test( const fvec &sample)
{
	if(bConverged) return maximum;

	fvec sigma; sigma.resize(dim*dim, 0);
	FOR(d, dim) sigma[d*dim + d] = fingerprint;

	fvec newSample = sample;
	if(!sample.size()) newSample = maximum;

	if(best.size() <= k)
	{
		while(best.size() < k)
		{
			fvec randSample = Generate(newSample, lastSigma, true);
			visited.push_back(randSample);
			float value = GetValue(randSample);
			evaluations++;
			if(bAdaptive)
			{
				FOR(d, dim) sigma[d*dim + d] = (1-value + 0.0001)*fingerprint;
			}
			best.push_back(make_pair(value, make_pair(randSample, sigma)));
		}
		std::sort(best.begin(), best.end());
	}

	// here we generate some samples
	fvec randSample = Generate(newSample, lastSigma);

	newSample = randSample;
	float value = GetValue(randSample);
	evaluations++;
	visited.push_back(newSample);
	if(bAdaptive)
	{
		FOR(d, dim*dim)
		{
			float sign = lastSigma[d] > 0 ? 1 : -1;
			sigma[d] = (1-value + 0.0001)*fingerprint*sign*sqrtf(fabs(lastSigma[d]));
		}
		//FOR(d, dim) sigma[d*dim + d] = (1-value + 0.0001)*fingerprint*sqrt(lastSigma[d*dim + d]);
	}
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

	maximum = GetBestMean();
	maximumValue = GetValue(maximum);
	lastSigma = GetBestSigma(maximum);
	FOR(d, lastSigma.size()) lastSigma[d] *= 2.f;

	//qDebug() << "generating map";
	//QImage map = DrawMap();
	//map.save("../../../map.png");
	//qDebug() << "map completed";

//	qDebug() << "mu: " << maximum[0] << " " << maximum[1];
//	qDebug() << "sigma: " << lastSigma[0] << " " << lastSigma[1] << " " << lastSigma[3];

	FOR(d, dim) maximum[d] = max(0.f, min(1.f,maximum[d]));
	history.push_back(maximum);
	historyValue.push_back(GetValue(maximum));

	return newSample;
}

fvec MaximizeDonut::Test(const fVec &sample)
{
	return Test((fvec)sample);
}

const char *MaximizeDonut::GetInfoString()
{
	char *text = new char[1024];
	sprintf(text, "POWER\n");
	return text;
}
