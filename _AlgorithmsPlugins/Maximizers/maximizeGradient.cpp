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
#include "maximizeGradient.h"
#include <QDebug>

using namespace std;

MaximizeGradient::MaximizeGradient()
{
	data = 0;
	dim = 2;
	maximum.resize(dim);
	FOR(d,dim) maximum[d] = rand()/(float)RAND_MAX;
	strength = 0.1;
	unmoving = 0;
	adaptive = true;
}

MaximizeGradient::~MaximizeGradient()
{
	KILL(data);
}

void MaximizeGradient::SetParams(float strength, bool adaptive)
{
	this->strength = strength;
	this->adaptive = adaptive;
}

void MaximizeGradient::Draw(QPainter &painter)
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
		painter.setBrush(Qt::white);
		painter.drawEllipse(point, 4, 4);
	}
	// we draw the current maximum
	QPointF point(history[history.size()-1][0]*w, history[history.size()-1][1]*h);
	painter.setBrush(QColor(255*(1-historyValue[history.size()-1]), 255, 255*(1-historyValue[history.size()-1])));
	painter.drawEllipse(point, 5, 5);
}

std::vector<GLObject> MaximizeGradient::DrawGL()
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
            o.colors.push_back(QVector3D(1,1,1));
    }
    o.vertices.push_back(QVector3D(history.back()[0]*2-1, historyValue.back()*0.5+0.02, history.back()[1]*2-1));
    o.colors.append(QVector3D(0,1,0));
    return objects;
}

void MaximizeGradient::Train(float *dataMap, fVec size, fvec startingPoint)
{
	w = size.x;
	h = size.y;
	if(data) delete [] data;
	data = new float[w*h];
	memcpy(data, dataMap, w*h*sizeof(float));
	bConverged = false;
	if(!startingPoint.size())
	{
		startingPoint.resize(dim);
		FOR(d, dim) startingPoint[d] = drand48();
	}
	unmoving = 0;
	maximum = startingPoint;
	float value = GetValue(startingPoint);
	maximumValue = value;
	history.push_back(maximum);
	HistoryValue().push_back(value);
	evaluations = 0;
	//qDebug() << "Starting maximization at " << maximum[0] << " " << maximum[1];
}

fvec MaximizeGradient::Test( const fvec &sample)
{
	if(bConverged) return maximum;
	fvec newSample;

	newSample = sample;
	if(!sample.size()) newSample = maximum;

	int xIndex = newSample[0]*w;
	int yIndex = newSample[1]*h;

	float delta = 0.003;
	float value = GetValue(newSample);
	evaluations++;
	// we compute the values of the gradient in the 9 directions around
	float values[8];
	fVec v[8];
	int searchType = 1;
	switch(searchType)
	{
	case 2: // 8 directions
		values[0] = GetValue(newSample + fVec(-delta	,-delta));
		values[2] = GetValue(newSample + fVec(delta	,-delta));
		values[5] = GetValue(newSample + fVec(-delta	,+delta));
		values[7] = GetValue(newSample + fVec(delta	,+delta));
		v[0] = fVec(-1,-1)*(values[0]-value);
		v[2] = fVec( 1,-1)*(values[2]-value);
		v[5] = fVec(-1, 1)*(values[5]-value);
		v[7] = fVec( 1, 1)*(values[7]-value);
		evaluations += 4;
	case 1: // 4 directions
		values[1] = GetValue(newSample + fVec(0		,-delta));
		values[3] = GetValue(newSample + fVec(-delta	,0));
		v[1] = fVec( 0,-1)*(values[1]-value);
		v[3] = fVec(-1, 0)*(values[3]-value);
		evaluations += 2;
	case 0: // 2 directions
		values[4] = GetValue(newSample + fVec(delta	,0));
		values[6] = GetValue(newSample + fVec(0		,+delta));
		v[4] = fVec( 1, 0)*(values[4]-value);
		v[6] = fVec( 0, 1)*(values[6]-value);
		evaluations += 2;
	}

	fVec gradient;
	FOR(i, 8) gradient += v[i];
	gradient /= (float)(1<<(searchType+1));
	if(!adaptive) // we just use the strength as factor for moving
	{
		gradient.normalize();
		gradient *= strength*0.1;
	}
	else // we use the actual value of gradient to decide how much to move
	{
		gradient *= strength/delta*0.1;
	}

	fvec oldSample = newSample;
	newSample += gradient;

	if(oldSample[0]*w == newSample[0]*w && oldSample[1]*h == newSample[1]*h) // we're not moving anymore!
	{
		unmoving++;
		if(unmoving > 10)
		{
			bConverged = true;
			return newSample;
		}
	}
	else unmoving=0;

	visited.push_back(newSample);
	value = GetValue(newSample);
	if(value > maximumValue)
	{
		maximum = newSample;
		maximumValue = value;
		history.push_back(maximum);
		historyValue.push_back(value);
	}
	return newSample;
}

fvec MaximizeGradient::Test(const fVec &sample)
{
	return Test((fvec)sample);
}

const char *MaximizeGradient::GetInfoString()
{
	char *text = new char[1024];
	sprintf(text, "Gradient Ascent");
	return text;
}
