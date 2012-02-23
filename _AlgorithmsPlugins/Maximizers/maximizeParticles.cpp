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
#include "maximizeParticles.h"
#include <QDebug>

using namespace std;

MaximizeParticles::MaximizeParticles()
    : particleCount(20)
{
    dim = 2;
    maximum.resize(dim);
    FOR(d,dim) maximum[d] = rand()/(float)RAND_MAX;
}

MaximizeParticles::~MaximizeParticles()
{
    KILL(data);
}

void MaximizeParticles::SetParams(int particleCount, float variance, bool bAdaptive)
{
    this->particleCount = particleCount;
    this->variance = variance;
    this->bAdaptive = bAdaptive;
    this->particleCount = particleCount;
}

void MaximizeParticles::Draw(QPainter &painter)
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

    // draw the current particles
    FOR(i, particles.size())
    {
        fvec sample = particles[i];
        QPointF point(sample[0]*w, sample[1]*h);
        int radius = 2 + weights[i]*5;
        painter.setBrush(Qt::green);
        painter.drawEllipse(point, radius, radius);
    }
    // we draw the current maximum
    QPointF point(history[history.size()-1][0]*w, history[history.size()-1][1]*h);
    painter.setBrush(QColor(255*(1-historyValue[history.size()-1]), 255, 255*(1-historyValue[history.size()-1])));
    painter.drawEllipse(point, 5, 5);
}

void MaximizeParticles::Train(float *dataMap, fVec size, fvec startingPoint)
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
        int xIndex = startingPoint[0]*w;
        int yIndex = startingPoint[1]*h;
        int index = min(w*h, max(0, yIndex*w + xIndex));
        float value = data[index];
        maximumValue = value;
        history.push_back(maximum);
        historyValue.push_back(value);
        //qDebug() << "Starting maximization at " << maximum[0] << " " << maximum[1];
    }
    particles.clear();
    weights.clear();
    fvec sample; sample.resize(dim);
    FOR(i, particleCount)
    {
        FOR(d, dim) sample[d] = drand48();
        particles.push_back(sample);
        weights.push_back(1.f/particleCount);
    }
    evaluations = 0;
}

fvec MaximizeParticles::Test( const fvec &sample)
{
    if(bConverged) return maximum;

    float decay = 0.2f;
    float totalWeights= 0;
    FOR(i, particleCount)
    {
        // first we guess the next pose for each particle
        particles[i] += RandN(dim, 0, variance*variance);
        // we compute the weights
        weights[i] = weights[i] *(1-decay) + GetValue(particles[i])*decay;
        totalWeights += weights[i];
        evaluations++;
        visited.push_back(particles[i]);
    }

    // we sort the particles by weight
    vector< pair<double, u32> > fits;
    FOR(i, weights.size()) fits.push_back(pair<double, u32>(weights[i], i));
    sort(fits.begin(), fits.end(), greater< pair<double,u32> >());
    vector<fvec> newParticles;
    fvec newWeights;
    FOR(i, fits.size())
    {
        newParticles.push_back(particles[fits[i].second]);
        newWeights.push_back(weights[fits[i].second]);
    }
    particles = newParticles;
    weights = newWeights;

    // we compute the result
    maximum = particles[0];
    maximumValue = weights[0];

    if(bAdaptive)
    {
        float decay = 0.1;
        variance = variance*(1-decay) + (1 - maximumValue*maximumValue)*0.1*decay;
    }

    /*
 maximum.clear();
 maximum.resize(dim,0);
 FOR(i, particleCount)
 {
  FOR(d, dim) maximum[d] = weights[i] * particles[i][d];
 }
*/

    history.push_back(maximum);
    historyValue.push_back(maximumValue);

    // we normalize probabilities by weight value
    double weightSum = 0;
    FOR(i, weights.size())  weightSum += weights[i];
    dvec probs;
    double weightCounter = 0;
    FOR(i, weights.size())
    {
        weightCounter += weights[i]/weightSum;
        probs.push_back(weightCounter);
    }

    // we resample the particles
    newParticles.clear();
    newWeights.clear();
    FOR(i, particleCount)
    {
        double r = drand48();
        u32 j=0;
        for (j=0; j<probs.size() && r > probs[j]; j++);
        u32 index = j<probs.size() ? j : 0;
        newParticles.push_back(particles[index]);
        newWeights.push_back(weights[index]);
    }
    particles = newParticles;
    weights = newWeights;

    // if particles have gone wild we resample them
    float degeneracyThreshold = 0.000000001;
    FOR(i, particleCount)
    {
        if( weights[i] < degeneracyThreshold )
        {
            FOR(d, dim) particles[i][d] = drand48();
            weights[i] = 1.f / particleCount;
        }
    }
    return maximum;
}

fvec MaximizeParticles::Test(const fVec &sample)
{
    return Test((fvec)sample);
}

char *MaximizeParticles::GetInfoString()
{
    char *text = new char[1024];
    sprintf(text, "Genetic Algorithm\n");
    return text;
}
