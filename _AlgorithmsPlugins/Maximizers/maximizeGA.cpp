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
#include "maximizeGA.h"
#include <QDebug>

using namespace std;

MaximizeGA::MaximizeGA()
    : mutation(0.01), cross(0.5), survival(0.3), population(50), trainer(0)
{
    dim = 2;
    maximum.resize(dim);
    FOR(d,dim) maximum[d] = rand()/(float)RAND_MAX;
}

MaximizeGA::~MaximizeGA()
{
    KILL(data);
    DEL(trainer);
}

void MaximizeGA::SetParams(double mutation, double cross, double survival, int population)
{
    this->mutation = mutation;
    this->cross = cross;
    this->survival = survival;
    this->population = population;
}

void MaximizeGA::Draw(QPainter &painter)
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

    if(trainer)
    {
        // draw the current population
        FOR(i, trainer->Population().size())
        {
            fvec sample = trainer->Population()[i].ToSample();
            QPointF point(sample[0]*w, sample[1]*h);
            painter.setBrush(Qt::green);
            painter.drawEllipse(point, 3, 3);
        }
    }
    // we draw the current maximum
    QPointF point(history[history.size()-1][0]*w, history[history.size()-1][1]*h);
    painter.setBrush(QColor(255*(1-historyValue[history.size()-1]), 255, 255*(1-historyValue[history.size()-1])));
    painter.drawEllipse(point, 5, 5);
}

void MaximizeGA::Train(float *dataMap, fVec size, fvec startingPoint)
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
        historyValue.push_back(value);
        //qDebug() << "Starting maximization at " << maximum[0] << " " << maximum[1];
    }
    DEL(trainer);
    trainer = new GATrain(data, w, h, population, dim);
    trainer->AlphaMute(mutation);
    trainer->AlphaCross(cross);
    trainer->AlphaSurvivors(survival);
    trainer->Generate(population);
    evaluations = 0;
}

fvec MaximizeGA::Test( const fvec &sample)
{
    if(bConverged) return maximum;
    FOR(i, trainer->Population().size()) visited.push_back(trainer->Population()[i].ToSample());
    evaluations += trainer->Population().size();
    trainer->NextGen();
    maximum = trainer->Best().ToSample();
    maximumValue = trainer->BestFitness();
    history.push_back(maximum);
    historyValue.push_back(maximumValue);
    return maximum;
}

fvec MaximizeGA::Test(const fVec &sample)
{
    return Test((fvec)sample);
}

char *MaximizeGA::GetInfoString()
{
    char *text = new char[1024];
    sprintf(text, "Genetic Algorithm\n");
    return text;
}
