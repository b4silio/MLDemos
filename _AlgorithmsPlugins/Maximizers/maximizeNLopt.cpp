/*********************************************************************
MLDemos: A User-Friendly visualization toolkit for machine learning
Copyright (C) 2010  Basilio Noris
Contact: mldemos@b4silio.com

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public License,
version 3 as published by the Free Software Foundation.

This library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free
Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*********************************************************************/
#include "maximizeNLopt.h"
#include <nlopt/nlopt.hpp>
#include <QDebug>

using namespace std;

int MaximizeNlopt::evaluationFrame = 0;
vector<fvec> MaximizeNlopt::evaluationList;

MaximizeNlopt::MaximizeNlopt()
    : type(0), step(0.1)
{
    data = 0;
    dim = 2;
    maximum.resize(dim);
    evaluationFrame = 0;
    FOR(d,dim) maximum[d] = rand()/(float)RAND_MAX;
}

MaximizeNlopt::~MaximizeNlopt()
{
    KILL(data);
}

void MaximizeNlopt::SetParams(int type, float step)
{
    this->type = type;
    this->step = step;
}

void MaximizeNlopt::Draw(QPainter &painter)
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
    painter.setBrush(QColor(255*(1-historyValue.back()), 255, 255*(1-historyValue.back())));
    painter.drawEllipse(point, 5, 5);
}

std::vector<GLObject> MaximizeNlopt::DrawGL()
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
    return objects;
}

struct OptData
{
    int dim;
    int w, h;
    float *data;
};

double objectiveFunction(unsigned n, const double *x, double *gradient /* NULL if not needed */, void *func_data)
{
    OptData *data = (OptData*)func_data;
    fvec sample(data->dim);
    FOR(d, data->dim) sample[d] = x[d];
    MaximizeNlopt::evaluationList.push_back(sample);

    double objective = Maximizer::GetValue(sample, data->data, data->w, data->h);
    if(gradient)
    {
        double *dx = new double[n];
        double delta = 1e-2;
        FOR(i, n)
        {
            memcpy(dx, x, n*sizeof(double));
            dx[i] += delta;
            double dError = Maximizer::GetValue(sample, data->data, data->w, data->h);
            gradient[i] = (dError - objective)/delta;
        }
        delete [] dx;
    }

    return objective;
}

void MaximizeNlopt::Train(float *dataMap, fVec size, fvec startingPoint)
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
    maximum = startingPoint;
    float value = GetValue(startingPoint);
    maximumValue = value;
    history.push_back(maximum);
    historyValue.push_back(value);

    evaluations = 0;
    evaluationFrame = 0;
    evaluationList.clear();

    OptData *data = new OptData;
    data->data = this->data;
    data->w = this->w;
    data->h = this->h;
    data->dim = dim;

    int optDim = dim;

    nlopt::opt opt;
    switch(type)
    {
    case 0:
        opt = nlopt::opt(nlopt::LN_AUGLAG, optDim);
        break;
    case 1:
        opt = nlopt::opt(nlopt::LN_BOBYQA, optDim);
        break;
    case 2:
        opt = nlopt::opt(nlopt::LN_COBYLA, optDim);
        break;
    case 3:
        opt = nlopt::opt(nlopt::LN_NELDERMEAD, optDim);
        break;
    case 4:
        opt = nlopt::opt(nlopt::LN_NEWUOA, optDim);
        break;
    case 5:
        opt = nlopt::opt(nlopt::LN_PRAXIS, optDim);
        break;
    case 6:
        opt = nlopt::opt(nlopt::LN_SBPLX, optDim);
        break;
    }

    opt.set_max_objective(objectiveFunction, (void*)data);
    opt.set_maxeval(500);
    vector<double> lowerBounds(optDim, 0);
    vector<double> upperBounds(optDim, 1);
    opt.set_lower_bounds(lowerBounds);
    opt.set_upper_bounds(upperBounds);
    opt.set_initial_step(step);
    opt.set_xtol_abs(0.001);

    vector<double> x(optDim), xOpt;

    FOR(d, dim) x[d] = startingPoint[d];

    try
    {
        xOpt = opt.optimize(x);
    }
    catch(std::exception e)
    {
        qDebug() << "caught exception while optimizing";
    }
    delete data;
}

fvec MaximizeNlopt::Test( const fvec &sample)
{
    fvec newSample = sample;
    float newValue=-FLT_MAX;
    float maxValue = GetValue(maximum);
    if(evaluationFrame < evaluationList.size())
    {
        newSample = evaluationList[evaluationFrame];
        newValue = GetValue(newSample);
        ++evaluationFrame;
    }
    if(newValue >= maxValue) maximum = newSample;
    visited.push_back(newSample);
    history.push_back(maximum);
    historyValue.push_back(maxValue);
    return newSample;
}

fvec MaximizeNlopt::Test(const fVec &sample)
{
    return Test((fvec)sample);
}

const char *MaximizeNlopt::GetInfoString()
{
    char *text = new char[1024];
    sprintf(text, "Gradient-Free Maximization");
    return text;
}

