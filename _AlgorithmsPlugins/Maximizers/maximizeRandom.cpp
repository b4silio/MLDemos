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
#include "maximizeRandom.h"
#include <QDebug>

using namespace std;

MaximizeRandom::MaximizeRandom()
{
	data = 0;
	dim = 2;
	maximum.resize(dim);
	FOR(d,dim) maximum[d] = rand()/(float)RAND_MAX;
	variance = 0;
}

MaximizeRandom::~MaximizeRandom()
{
	KILL(data);
}

void MaximizeRandom::SetParams(float variance)
{
	this->variance = variance;
}

void MaximizeRandom::Draw(QPainter &painter)
{
	painter.setPen(QPen(Qt::black, 1.5));
	painter.setBrush(Qt::NoBrush);
	FOR(i, visited.size())
	{
		QPointF point(visited[i][0]*w, visited[i][1]*h);
		painter.drawEllipse(point, 3, 3);
	}

	painter.setPen(QPen(Qt::black, 1.5));
	FOR(i, history.size()-1 )
    {
		QPointF point(history[i][0]*w, history[i][1]*h);
		QPointF pointNext(history[i+1][0]*w, history[i+1][1]*h);

		painter.setBrush(Qt::NoBrush);
		painter.drawLine(point, pointNext);
		painter.setBrush(QColor(255*(1-historyValue[i]), 255, 255*(1-historyValue[i])));
		painter.drawEllipse(point, 5, 5);
	}
	// we draw the current maximum
    QPointF point(history.back()[0]*w, history.back()[1]*h);
    painter.setBrush(QColor(255*(1.-historyValue.back()), 255, 255*(1.-historyValue.back())));
	painter.drawEllipse(point, 5, 5);

	if(variance > 0)
	{
		QPointF maxPoint(maximum[0]*w, maximum[1]*h);
		int radius = (int)(variance * max(w,h));
		painter.setBrush(Qt::NoBrush);
		painter.setPen(QPen(Qt::black,1.5));
		painter.drawEllipse(maxPoint, radius,radius);
		painter.setPen(QPen(Qt::black,0.5));
		painter.drawEllipse(maxPoint, radius*2,radius*2);
	}
}

std::vector<GLObject> MaximizeRandom::DrawGL()
{
    vector<GLObject> objects;
    GLObject o;
    o.objectType = "Samples,Maximizer,Lines,linestrip";
    o.style ="pointsize:10,width:2,dotted";
    FOR(i, history.size()-1)
    {
            fvec &sample = history[i];
            double value = historyValue[i]*0.5;
            o.vertices.push_back(QVector3D(sample[0]*2-1, value+0.02, sample[1]*2-1));
            o.colors.append(QVector3D(1,1,1));
    }
    o.vertices.push_back(QVector3D(history.back()[0]*2-1, historyValue.back()*0.5+0.02, history.back()[1]*2-1));
    o.colors.append(QVector3D(0,1,0));
    objects.push_back(o);
    o = GLObject();
    o.objectType = "Lines,Maximizer,linestrip";
    o.style ="dotted";
    // we generate a circle around the current maximum
    int segments = 32;
    float r = variance*2;
    for (float theta=0; theta <= M_PI*2.f; theta += (M_PI*2.f)/segments)
    {
        float x = cosf(theta)*r + history.back()[0];
        float y = sinf(theta)*r + history.back()[1];
        o.vertices.push_back(QVector3D(x*2.f-1.f,historyValue.back()*0.5+0.02,y*2.f-1.f));
    }
    objects.push_back(o);
    return objects;
}

void MaximizeRandom::Train(float *dataMap, fVec size, fvec startingPoint)
{
	w = size.x;
	h = size.y;
	if(data) delete [] data;
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

fvec MaximizeRandom::Test( const fvec &sample)
{
	if(bConverged) return maximum;
	fvec newSample;
	if(variance == 0) // we're doing random search
	{
		newSample.resize(dim);
		FOR(d, dim) newSample[d] = (rand()/(float)RAND_MAX);
	}
	else // we're doing random walk
	{
		newSample = sample;
		if(!sample.size()) newSample = maximum;

		fvec randSample; randSample.resize(dim);
		int tries = 64;
		bool bInsideLimits = true;
		do
		{
			randSample = newSample + RandN(dim, 0, variance);
			FOR(d, dim) bInsideLimits &= randSample[d] >= 0.f && randSample[d] <= 1.f;
			tries--;
		}while(tries && !bInsideLimits);
		newSample = randSample;
	}
	visited.push_back(newSample);
	float value = GetValue(newSample);
	evaluations++;
	if(value > maximumValue)
	{
		maximum = newSample;
		maximumValue = value;
		//qDebug() << "new maximum found at " << maximum[0] << " : " << maximum[1] << " \tvalue: " << value;
	}
	history.push_back(maximum);
	historyValue.push_back(GetValue(maximum));
	return newSample;
}

fvec MaximizeRandom::Test(const fVec &sample)
{
	return Test((fvec)sample);
}

const char *MaximizeRandom::GetInfoString()
{
	char *text = new char[1024];
	if(variance == 0) sprintf(text, "Random Search");
	else sprintf(text, "Random Walk\n");
	return text;
}
