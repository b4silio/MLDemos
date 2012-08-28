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
#include "maximizeSwarm.h"
#include <QDebug>
#include <optimization_test_functions.h>

using namespace std;

MaximizeSwarm::MaximizeSwarm()
    : particleCount(20), pso(0)
{
    dim = 2;
    maximum.resize(dim);
    FOR(d,dim) maximum[d] = rand()/(float)RAND_MAX;
}

MaximizeSwarm::~MaximizeSwarm()
{
    KILL(data);
}

void MaximizeSwarm::SetParams(int particleCount, float mutation, bool inertia, float inertiaInit, float inertiaFinal, float particleConfidence, float swarmConfidence)
{
    this->particleCount = particleCount;
    this->mutation = mutation;
    this->inertia = inertia;
    this->inertiaInit = inertiaInit;
    this->inertiaFinal = inertiaFinal;
    this->particleConfidence = particleConfidence;
    this->swarmConfidence = swarmConfidence;
}

void MaximizeSwarm::Draw(QPainter &painter)
{
    painter.setPen(QPen(Qt::black, 1.5));
    painter.setBrush(Qt::NoBrush);
    FOR(i, visited.size())
    {
        QPointF point(visited[i][0]*w, visited[i][1]*h);
        painter.drawEllipse(point, 3, 3);
    }

    if(pso)
    {
        double ** swarm = pso->swarm;
        int count = pso->swarmCount;
        FOR(i, count)
        {
            double *particle = swarm[i];
            double x = (particle[0] - pso->getLBound()(0))/(pso->getUBound()(0)-pso->getLBound()(0));
            double y = (particle[1] - pso->getLBound()(1))/(pso->getUBound()(1)-pso->getLBound()(1));

            //int radius = 2 + weights[i]*5;
            int radius = 4;
            painter.setBrush(Qt::green);
            painter.drawEllipse(QPointF(x*w, y*h), radius, radius);
        }
    }

    // draw all the particles visited in the past
    FOR(i, pso->evaluationHistory.size())
    {
        pair<int,int> sample = pso->evaluationHistory[i];
        QPointF point(sample.first, sample.second);
        int radius = 3;
        painter.setBrush(Qt::NoBrush);
        painter.setPen(Qt::black);
        painter.drawEllipse(point, radius, radius);
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
    float val = 255*(1-historyValue[history.size()-1]);
    QColor color(min(255.f,max(0.f,val)), 255, min(255.f,max(0.f,val)));
    painter.setBrush(color);
    painter.drawEllipse(point, 5, 5);
}

/*
//SINGLE OBJECTIVE OPTIMIZATION PROBLEMS
Eigen::VectorXd rastragin(Eigen::VectorXd& x);
Eigen::VectorXd schwefel(Eigen::VectorXd& x);
Eigen::VectorXd griewangk(Eigen::VectorXd& x);
Eigen::VectorXd griewangk_constrained(Eigen::VectorXd& x);
Eigen::VectorXd f_1disolated(Eigen::VectorXd& x);
Eigen::VectorXd f_1disolated2(Eigen::VectorXd& x);
*/

void MaximizeSwarm::Train(float *dataMap, fVec size, fvec startingPoint)
{
    w = size.x;
    h = size.y;
    if(data) delete [] data;
    data = new float[w*h];
    memcpy(data, dataMap, w*h*sizeof(float));
    //FOR(i, w*h) data[i] = 1.f - data[i];
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
        historyValue.push_back(1-value);
        //qDebug() << "Starting maximization at " << maximum[0] << " " << maximum[1];
    }

    if(pso)
    {
        pso->kill();
        DEL(pso);
    }

    int constraintCount=0;
    int iterationCount=9999;
    evaluations = 0;

    pso = new PSO(dim,constraintCount,iterationCount,particleCount,Eigen::VectorXd::Constant(dim,0.),Eigen::VectorXd::Constant(dim,1.));
    pso->SetData(data, w, h);
    pso->setProblemName("Data");
    pso->setMutationProbability(mutation);
    if(inertia)
    {
        pso->setInitialInertia(inertiaInit);
        pso->setFinalInertia(inertiaFinal);
    }
    pso->setSelfConfidence(particleConfidence);
    pso->setSwarmConfidence(swarmConfidence);
    pso->init();
}

fvec MaximizeSwarm::Test( const fvec &sample)
{
    if(bConverged) return maximum;

    pso->optimizeOnce();
    Eigen::MatrixXd current = pso->getOptimalSolutions();
    Eigen::MatrixXd values = pso->getOptimalValues();
    maximum.resize(2);
    FOR(d, dim) maximum[d] = (current(0,d) - pso->getLBound()(d))/(pso->getUBound()(d)-pso->getLBound()(d));
    //qDebug() << "maximum: " << maximum[0] << " " << maximum[1];
    maximumValue = values(0,0);
    history.push_back(maximum);
    historyValue.push_back(1-maximumValue);

    evaluations = pso->modelEvaluationsCount;
    return sample;
}

fvec MaximizeSwarm::Test(const fVec &sample)
{
    return Test((fvec)sample);
}

const char *MaximizeSwarm::GetInfoString()
{
    char *text = new char[1024];
    sprintf(text, "Particles Swarm Optimization\n");
    return text;
}
