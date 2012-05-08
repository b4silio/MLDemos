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
#include "reinforcementGA.h"
#include <QDebug>

using namespace std;

ReinforcementGA::ReinforcementGA()
    : mutation(0.01), cross(0.5), survival(0.3), population(50), trainer(0)
{
    dim = 2;
    maximum = directions = fvec();
}

ReinforcementGA::~ReinforcementGA()
{
    DEL(trainer);
}

void ReinforcementGA::SetParams(double mutation, double cross, double survival, int population)
{
    this->mutation = mutation;
    this->cross = cross;
    this->survival = survival;
    this->population = population;
}

void ReinforcementGA::Initialize(ReinforcementProblem *problem)
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

    DEL(trainer);
    trainer = new GATrain(population, dim, problem->quantizeType);
    trainer->AlphaMute(mutation);
    trainer->AlphaCross(cross);
    trainer->AlphaSurvivors(survival);
    trainer->Generate(population);
    evaluations = 0;
}

fvec ReinforcementGA::Update()
{
    if(bConverged) return maximum;
    FOR(i, trainer->Population().size()) visited.push_back(trainer->Population()[i].ToSample());
    evaluations += trainer->Population().size();

    dvec fitness(trainer->Population().size());
    FOR(i, trainer->Population().size())
    {
        fitness[i] = problem->GetReward(trainer->Population()[i].ToSample());
    }
    trainer->SetFitness(fitness);
    trainer->NextGen();
    maximum = trainer->Best().ToSample();
    maximumValue = trainer->BestFitness();
    history.push_back(maximum);
    historyValue.push_back(maximumValue);
    return maximum;
}

void ReinforcementGA::Draw(QPainter &painter)
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

    // first we draw the fitness function
    dvec fitness = trainer->Fitness();
    double maxValue = -DBL_MAX;
    FOR(i, historyValue.size()) maxValue = max(maxValue, historyValue[i]);
    //FOR(i, fitness.size()) maxValue = max(maxValue, fitness[i]);
    int valueLimit = 4;
    double upperBound = ((int)ceil(maxValue)/valueLimit + 1)*valueLimit;
    painter.setPen(QPen(Qt::red, 2));
    QPointF oldPoint;
    FOR(i, fitness.size())
    {
        QPointF point(i*graphW/fitness.size(), graphH*(1.f - (fitness[i]/upperBound)));
        point += QPointF(left + graphPad, top + graphPad);
        //painter.drawRect(left + point.x() + graphPad, top + graphPad + (graphH - point.y()), max(1,graphW/(int)fitness.size()), point.y());
        if(i) painter.drawLine(point, oldPoint);
        else
        {
            painter.drawText(point + QPointF(2,0), QString("%1").arg(fitness[i], 0, 'f', 2));
        }
        oldPoint = point;
    }
    // then the history values
    painter.setPen(QPen(Qt::green, 2));
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
    painter.setPen(QPen(Qt::black, 2));
    painter.drawLine(left + graphPad, top+graphPad, left + graphPad, top+graphPad + graphH);
    painter.drawLine(left + graphPad, top+graphPad+graphH, left + graphPad + graphW, top+graphPad + graphH);
    painter.drawText(left + graphPad, top + graphPad, QString("%1").arg(upperBound, 0, 'f', 1));
    painter.drawText(left + graphPad, top + graphPad*2 + graphH, QString("0"));
    font.setPointSize(9);
    painter.setFont(font);
    painter.setPen(Qt::red);
    painter.drawText(left, top, graphW/2, graphPad, Qt::AlignVCenter | Qt::AlignRight, "Individuals");
    //painter.drawText(left + graphPad + graphW/2 - 100, top + graphPad*2, "Individuals");
    painter.setPen(Qt::green);
    painter.drawText(left + graphPad + graphW/2, top, graphW/2, graphPad, Qt::AlignVCenter | Qt::AlignLeft, "Best");
    //painter.drawText(left + graphPad + graphW/2, top + graphPad*2, "Best");
    painter.setRenderHints(hints);
}


const char *ReinforcementGA::GetInfoString()
{
    char *text = new char[1024];
    sprintf(text, "Genetic Algorithm\n");
    return text;
}
