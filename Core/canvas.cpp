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
#include <QtGui>
#include <QWidget>
#include <QSize>
#include <QPixmap>
#include <QPainter>
#include <QPen>
#include <QImage>
#include <QFontMetrics>
#include <iostream>

#include "expose.h"
#include "public.h"
#include "basicMath.h"
#include "canvas.h"
#include "drawUtils.h"

using namespace std;

//DatasetManager Canvas::data;
bool Canvas::bCrossesAsDots = true;

Canvas::Canvas(QWidget *parent)
    : QWidget(parent),
      bDisplayMap(false),
      bDisplayInfo(false),
      bDisplaySingle(false),
      bDisplaySamples(true),
      bDisplayTrajectories(true),
      bDisplayTimeSeries(true),
      bDisplayLearned(true),
      bDisplayGrid(true),
      bDisplayLegend(true),
      crosshair(QPainterPath()),
      bShowCrosshair(false),
      bNewCrosshair(true),
      trajectoryCenterType(0),
      trajectoryResampleType(1),
      trajectoryResampleCount(100),
      liveTrajectory(vector<fvec>()),
      centers(map<int,fvec>()),
      drawnSamples(0),
      drawnTrajectories(0),
      drawnTimeseries(0),
      mouseAnchor(QPoint(-1,-1)),
      bDrawing(false),
      zoom(1.f),
      zooms(2,1.f),
      center(2,0),
      xIndex(0), yIndex(1), zIndex(-1),
      canvasType(0),
      data(new DatasetManager())
{
    resize(640,480);
    setAcceptDrops(true);

    setMouseTracking(true);
    setCursor(Qt::CrossCursor);
    setBackgroundRole(QPalette::Base);
    setMouseTracking(true);

    QPalette p(palette());
    p.setColor(backgroundRole(), Qt::white);
    setPalette(p);
    show();
}

Canvas::~Canvas()
{
    DEL(data);
}

void Canvas::SetConfidenceMap(QImage image)
{
    maps.confidence = QPixmap::fromImage(image);
    repaint();
}

void Canvas::SetModelImage(QImage image)
{
    maps.model = QPixmap::fromImage(image);
    repaint();
}

void Canvas::SetAnimationImage(QImage animation)
{
    maps.animation = QPixmap::fromImage(animation);
    repaint();
}

void Canvas::SetCanvasType(int type)
{
    if(canvasType || type)
    {
        maps.model = QPixmap();
        maps.info = QPixmap();
    }
    maps.samples = QPixmap();
    maps.trajectories = QPixmap();
    maps.grid = QPixmap();
    canvasType = type;
    ResetSamples();
    bNewCrosshair = true;
}

QPointF Canvas::toCanvasCoords(fvec sample)
{
    if(!sample.size()) return QPointF(0,0);
    if(sample.size() == 2 && center.size() > 2) {
        fvec newSample = fvec(center.size(), 0);
        newSample[xIndex] = sample[0];
        newSample[yIndex] = sample[1];
        sample = newSample;
    } else if(sample.size() < center.size()) sample.resize(center.size());
    sample -= center;
    QPointF point(sample[xIndex]*(zoom*zooms[xIndex]*height()),sample[yIndex]*(zoom*zooms[yIndex]*height()));
    point += QPointF(width()/2, height()/2);
    point.setY(height()-point.y());
    return point;
}

QPointF Canvas::toCanvas(fVec sample)
{
    sample -= center;
    QPointF point(sample[xIndex]*(zoom*zooms[xIndex]*height()),sample[yIndex]*(zoom*zooms[yIndex]*height()));
    point += QPointF(width()/2, height()/2);
    point.setY(height()-point.y());
    return point;
}

QPointF Canvas::toCanvasCoords(float x, float y)
{
    x -= center[xIndex];
    y -= center[yIndex];
    QPointF point(x*(zoom*zooms[xIndex]*height()),y*(zoom*zooms[yIndex]*height()));
    point += QPointF(width()/2, height()/2);
    point.setY(height() - point.y());
    return point;
}

fvec Canvas::fromCanvas(QPointF point)
{
    int dim = data->GetDimCount();
    fvec sample(dim);
    point.setY(height()-point.y());
    point -= QPointF(width()/2.f,height()/2.f);
    sample[xIndex] = point.x()/(zoom*zooms[xIndex]*height());
    sample[yIndex] = point.y()/(zoom*zooms[yIndex]*height());
    sample += center;
    return sample;
}

fvec Canvas::fromCanvas(float x, float y)
{
    if(!data) return fvec(2,0);
    int dim = data->GetDimCount();
    fvec sample(dim);
    y = height() - y;
    x -= width()/2.f;
    y -= height()/2.f;
    sample[xIndex] = x/(zoom*zooms[xIndex]*height());
    sample[yIndex] = y/(zoom*zooms[yIndex]*height());
    sample += center;
    return sample;
}

fvec Canvas::toSampleCoords(QPointF point)
{
    int dim = data->GetDimCount();
    fvec sample(dim);
    point.setY(height() - point.y());
    point -= QPointF(width()/2.f,height()/2.f);
    sample[xIndex] = point.x()/(zoom*zooms[xIndex]*height());
    sample[yIndex] = point.y()/(zoom*zooms[yIndex]*height());
    sample += center;
    return sample;
}

fvec Canvas::toSampleCoords(float x, float y)
{
    int dim = data->GetDimCount();
    fvec sample(dim);
    y = height() - y;
    x -= width()/2.f;
    y -= height()/2.f;
    sample[xIndex] = x/(zoom*zooms[xIndex]*height());
    sample[yIndex] = y/(zoom*zooms[yIndex]*height());
    sample += center;
    return sample;
}

fvec Canvas::canvasTopLeft()
{
    return toSampleCoords(0,height()-1);
}

fvec Canvas::canvasBottomRight()
{
    return toSampleCoords(width()-1,0);
}

QRectF Canvas::canvasRect()
{
    fvec tl = canvasTopLeft();
    fvec br = canvasBottomRight();
    return QRectF(tl[xIndex], tl[yIndex], (br-tl)[xIndex], (br-tl)[yIndex]);
}

void Canvas::SetZoom(float zoom)
{
    if(this->zoom == zoom) return;
    this->zoom = zoom;
    //	int dim = data->GetDimCount();
    //	zooms = fvec(dim,1.f);
    maps.grid = QPixmap();
    maps.model = QPixmap();
    maps.confidence = QPixmap();
    //maps.reward = QPixmap();
    maps.info = QPixmap();
    ResetSamples();
    bNewCrosshair = true;
    //repaint();
}

void Canvas::SetZoom(fvec zooms)
{
    if(this->zooms == zooms) return;
    this->zooms = zooms;
    zoom = 1.f;
    maps.grid = QPixmap();
    maps.model = QPixmap();
    maps.confidence = QPixmap();
    //maps.reward = QPixmap();
    maps.info = QPixmap();
    ResetSamples();
    bNewCrosshair = true;
    //repaint();
}

void Canvas::SetCenter(fvec center)
{
    if(this->center == center) return;
    this->center = center;
    maps.grid = QPixmap();
    maps.model = QPixmap();
    maps.confidence = QPixmap();
    //maps.reward = QPixmap();
    maps.info = QPixmap();
    ResetSamples();
    bNewCrosshair = true;
    //repaint();
}

bool Canvas::SetDim(int xIndex, int yIndex, int zIndex)
{
    bool bChanged = false;
    if(this->xIndex != xIndex)
    {
        bChanged = true;
        this->xIndex = xIndex;
    }
    if(this->yIndex != yIndex)
    {
        bChanged = true;
        this->yIndex = yIndex;
    }
    this->zIndex = zIndex;
    if(bChanged)
    {
        maps.grid = QPixmap();
        maps.model = QPixmap();
        maps.confidence = QPixmap();
        //maps.reward = QPixmap();
        maps.info = QPixmap();
        ResetSamples();
        bNewCrosshair = true;
        //repaint();
    }
    return bChanged;
}

void Canvas::FitToData()
{
    if(!data->GetCount() && !data->GetTimeSeries().size())
    {
        center = fvec(2,0);
        zooms = fvec(2, 1.f);
        SetZoom(1);
        //qDebug() << "nothing to fit";
        return;
    }
    int dim = data->GetDimCount();
    center = fvec(dim,0);
    if(data->GetCount() == 1)
    {
        center = data->GetSample(0);
        zooms = fvec(dim,1.f);
        SetZoom(1);
        return;
    }
    //qDebug() << "fit to data, dim: " << dim;

    // we go through all the data and find the boundaries
    std::pair<fvec,fvec> bounds = data->GetBounds();
    fvec mins = bounds.first, maxes = bounds.second;
    FOR(d, mins.size())
    {
        if(maxes[d] - mins[d] > 1e6)
        {
            mins[d] = 0;
            maxes[d] = 1;
        }
    }

    vector<fvec> samples = data->GetSamples();

    vector<TimeSerie>& series = data->GetTimeSeries();
    FOR(i, series.size())
    {
        TimeSerie& serie = series[i];
        mins[0] = 0;
        maxes[0] = 1;
        center[0] = 0.5f;
        FOR(j, serie.size())
        {
            int dim = serie[j].size();
            FOR(d,dim)
            {
                if(mins[d+1] > serie[j][d]) mins[d+1] = serie[j][d];
                if(maxes[d+1] < serie[j][d]) maxes[d+1] = serie[j][d];
            }
        }
    }
    fvec diff = maxes - mins;
    // we add 10% to the edges
    mins -= diff*.05f;
    maxes += diff*.05f;
    diff = maxes - mins;

    FOR(d, diff.size())
    {
        if(diff[d] == 0) diff[d] = 1e-6;
    }

    center = mins + diff/2;

    /*
    float diffX = diff[xIndex]*1.04; // add a small margin
    float diffY = diff[yIndex]*1.04; // add a small margin
    float aspectRatio = width() / (float)height();
    diffX /= aspectRatio;
    SetZoom(min(1/diffY,1/diffX));
    */

    zooms = fvec(dim, 1.f);
    FOR(d, dim) zooms[d] = 1.f / diff[d];
    SetZoom(1.f);
}

void Canvas::ResizeEvent()
{
    if(canvasType != 0) return;
    if(width() != parentWidget()->width() || height() != parentWidget()->height()) resize(parentWidget()->size());
    drawnSamples = 0;
    maps.samples = QPixmap();
    maps.model = QPixmap();

    bNewCrosshair = true;
    if(!maps.reward.isNull())
    {
        QPixmap newReward(width(), height());
        newReward = maps.reward.scaled(newReward.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
    if(!canvasType) RedrawAxes();
}

QString Canvas::GetClassString(int classNumber)
{
    QString className = QString("Class %1").arg(classNumber);
    if (classNames.count(classNumber)) {
        QString name = classNames[classNumber];
        if (name.length() < 3) name = "Class " + name;
        return name;
    }
    return className;
}

QRgb Canvas::GetColorMapValue(float value, int colorscheme=2)
{
    float r, g, b;
    switch(colorscheme)
    {
    default:
    case 0:
        {
            r = value;
            g = 0;
            b = 0;
        }
        break;
    case 1: // autumn
        {
            r = value;
            g = value*0.6;
            b = value*0.2;
        }
        break;
    case 2: // jet
        {
            float Red = 0, Green = 0, Blue = 0;

            if (value < 0.5f) Red = value * 2;
            else Red = (1.0f - value) * 2;

            if (value >= 0.3f && value < 0.8f) Green = (value - 0.3f) * 2;
            else if (value < 0.3f) Green = (0.3f - value) * 2;
            else Green = (1.3f - value) * 2;

            if (value >= 0.5f) Blue = (value - 0.5f) * 2;
            else Blue = (0.5f - value) * 2;

            r = Red;
            g = Green;
            b = Blue;
        }
        break;
    case 3: // grayscale
        {
            r = value;
            g = value;
            b = value;
        }
        break;
    }
    return qRgb(r*255,g*255,b*255);
}
