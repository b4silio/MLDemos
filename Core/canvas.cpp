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

void Canvas::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/plain")) event->acceptProposedAction();
}

void Canvas::dropEvent(QDropEvent *event)
{
    if(event->mimeData()->text() == "Target")
    {
        QPointF position = event->pos();
        //qDebug() << "Dropping Target at coordinates: " << position;
        targets.push_back(toSampleCoords(position.x(), position.y()));
        targetAge.push_back(0);
    }
    else if(event->mimeData()->text() == "Gaussian")
    {
        QPointF position = event->pos();
        double variance = event->mimeData()->colorData().toDouble();
        PaintGaussian(position, variance);
    }
    else if(event->mimeData()->text() == "Gradient")
    {
        QPointF position = event->pos();
        PaintGradient(position);
    }
    event->acceptProposedAction();
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

void Canvas::PaintStandard(QPainter &painter, bool bSvg)
{
    painter.setBackgroundMode(Qt::OpaqueMode);
    painter.setBackground(Qt::white);

    painter.fillRect(geometry(),Qt::white);

    if(bDisplayMap)
    {
        if(!maps.confidence.isNull()) painter.drawPixmap(geometry(), maps.confidence);
    }
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);

    if(bDisplaySamples)
    {
        DrawRewards();
        if(!maps.reward.isNull())
        {
            painter.setBackgroundMode(Qt::OpaqueMode);
            painter.drawPixmap(geometry(), maps.reward);
        }
        if(bSvg)
        {
            painter.setBackgroundMode(Qt::TransparentMode);
            DrawSamples(painter);
            DrawObstacles(painter);
        }
        else
        {
            DrawSamples();
            painter.setBackgroundMode(Qt::TransparentMode);
            painter.drawPixmap(geometry(), maps.samples);
            DrawObstacles();
            painter.drawPixmap(geometry(), maps.obstacles);
        }
    }
    if(bDisplayTrajectories)
    {
        if(bSvg)
        {
            DrawTrajectories(painter);
        }
        else
        {
            DrawTrajectories();
            painter.setBackgroundMode(Qt::TransparentMode);
            painter.drawPixmap(geometry(), maps.trajectories);
        }
        if(targets.size()) DrawTargets(painter);
    }
    if(bDisplayTimeSeries)
    {
        if(bSvg)
        {

        }
        else
        {
            DrawTimeseries();
            painter.setBackgroundMode(Qt::TransparentMode);
            painter.drawPixmap(geometry(), maps.timeseries);
        }
    }
    if(!bSvg && bDisplayLearned)
    {
        if(maps.model.isNull())
        {
            int w = width();
            int h = height();
            maps.model = QPixmap(w,h);
            //QBitmap bitmap(w,h);
            //bitmap.clear();
            //maps.model.setMask(bitmap);
            maps.model.fill(Qt::transparent);
            QPainter painter(&maps.model);
            DrawSampleColors(painter);
        }
        painter.setBackgroundMode(Qt::TransparentMode);
        painter.drawPixmap(geometry(), maps.model);
    }
    if(!maps.animation.isNull())
    {
        painter.setBackgroundMode(Qt::TransparentMode);
        painter.drawPixmap(geometry(), maps.animation);
    }
    if(!bSvg && bDisplayInfo && !maps.info.isNull())
    {
        painter.setBackgroundMode(Qt::TransparentMode);
        painter.drawPixmap(geometry(), maps.info);
    }
    if(!bSvg && bShowCrosshair)
    {
        if(bNewCrosshair) emit DrawCrosshair();
        painter.setBackgroundMode(Qt::TransparentMode);
        painter.drawPath(crosshair.translated(mouse));
        if(liveTrajectory.size()) DrawLiveTrajectory(painter);
    }
    if(bDisplayGrid)
    {
        if(bSvg)
        {
            painter.setBackgroundMode(Qt::TransparentMode);
            DrawAxes(painter);
        }
        else
        {
            if(maps.grid.isNull()) RedrawAxes();
            painter.setBackgroundMode(Qt::TransparentMode);
            painter.drawPixmap(geometry(), maps.grid);
        }
    }
    if(bDisplayLegend)
    {
        painter.setBackgroundMode(Qt::TransparentMode);
        DrawLegend(painter);
    }
}

void Canvas::PaintMultivariate(QPainter &painter, int type)
{
    painter.setBackgroundMode(Qt::OpaqueMode);
    painter.setBackground(Qt::white);

    painter.fillRect(geometry(),Qt::white);

    std::pair<fvec,fvec> bounds = data->GetBounds();

    if(bDisplaySamples)
    {
        if(maps.samples.isNull())
        {
            int w = width();
            int h = height();
            maps.samples = QPixmap(w,h);
            //QBitmap bitmap(w,h);
            //bitmap.clear();
            //maps.samples.setMask(bitmap);
            maps.samples.fill(Qt::transparent);
            Expose::DrawData(maps.samples, data->GetSamples(), data->GetLabels(), data->GetFlags(), type, data->bProjected, dimNames, bounds);
        }
        painter.setBackgroundMode(Qt::TransparentMode);
        painter.drawPixmap(geometry(), maps.samples);
    }
    if(bDisplayTrajectories && (type != 1 && type != 3))
    {
        if(maps.trajectories.isNull())
        {
            int w = width();
            int h = height();
            maps.trajectories = QPixmap(w,h);
            //QBitmap bitmap(w,h);
            //bitmap.clear();
            //maps.trajectories.setMask(bitmap);
            maps.trajectories.fill(Qt::transparent);
            Expose::DrawTrajectories(maps.trajectories, data->GetTrajectories(trajectoryResampleType, trajectoryResampleCount, trajectoryCenterType, 0.1, true), data->GetLabels(), type, 0, bounds);
        }
        painter.setBackgroundMode(Qt::TransparentMode);
        painter.drawPixmap(geometry(), maps.trajectories);
    }
    if(bDisplayLearned)
    {
        if(maps.model.isNull() && sampleColors.size())
        {
            int w = width();
            int h = height();
            maps.model = QPixmap(w,h);
            //QBitmap bitmap(w,h);
            //bitmap.clear();
            //maps.model.setMask(bitmap);
            maps.model.fill(Qt::transparent);
            Expose::DrawData(maps.model, data->GetSamples(), sampleColors, data->GetFlags(), type, data->bProjected, true, dimNames);
        }
        painter.setBackgroundMode(Qt::TransparentMode);
        painter.drawPixmap(geometry(), maps.model);
    }
    if(bDisplayInfo && !maps.info.isNull())
    {
        //painter.setBackgroundMode(Qt::TransparentMode);
        //painter.drawPixmap(geometry(), maps.info);
    }
    if(bDisplayGrid)
    {
        if(maps.grid.isNull())
        {
        }
        painter.setBackgroundMode(Qt::TransparentMode);
        painter.drawPixmap(geometry(), maps.grid);
    }
}

void Canvas::PaintVariable(QPainter &painter, int type, fvec params)
{
    painter.setBackgroundMode(Qt::OpaqueMode);
    painter.setBackground(Qt::white);
    painter.fillRect(geometry(),Qt::white);

    if(maps.samples.isNull())
    {
        int w = width();
        int h = height();
        maps.samples = QPixmap(w,h);
        //QBitmap bitmap(w,h);
        //bitmap.clear();
        //maps.samples.setMask(bitmap);
        maps.samples.fill(Qt::transparent);
        Expose::DrawVariableData(maps.samples, data->GetSamples(), data->GetLabels(), type, params, data->bProjected);
    }
    painter.setBackgroundMode(Qt::TransparentMode);
    painter.drawPixmap(geometry(), maps.samples);

    if(maps.trajectories.isNull())
    {
        int w = width();
        int h = height();
        maps.trajectories = QPixmap(w,h);
        //QBitmap bitmap(w,h);
        //bitmap.clear();
        //maps.trajectories.setMask(bitmap);
        maps.trajectories.fill(Qt::transparent);
    }
    painter.setBackgroundMode(Qt::TransparentMode);
    painter.drawPixmap(geometry(), maps.trajectories);

    if(maps.model.isNull() && sampleColors.size())
    {
        int w = width();
        int h = height();
        maps.model = QPixmap(w,h);
        //QBitmap bitmap(w,h);
        //bitmap.clear();
        //maps.model.setMask(bitmap);
        maps.model.fill(Qt::transparent);
        Expose::DrawVariableData(maps.model, data->GetSamples(), sampleColors, type, params, data->bProjected);
    }
    painter.setBackgroundMode(Qt::TransparentMode);
    painter.drawPixmap(geometry(), maps.model);

}
void Canvas::paintEvent(QPaintEvent *event)
{
    if(bDrawing) return;
    bDrawing = true;
    QPainter painter(this);
    if(!canvasType) PaintStandard(painter); // we only draw if we're actually on the canvas
    /*
    else if(canvasType <= 5) PaintMultivariate(painter, canvasType-2); // 0: standard, 1: 3d, so we take out 2
    else
    {
        fvec params;
        params.push_back(xIndex);
        params.push_back(yIndex);
        params.push_back(zIndex);
        PaintVariable(painter, canvasType-6, params);
    }
    */
    bDrawing = false;
}

QPointF Canvas::toCanvasCoords(fvec sample)
{
    if(!sample.size()) return QPointF(0,0);
    if(sample.size() < center.size()) sample.resize(center.size());
    //else if(sample.size() > center.size()) center = fvec(sample.size(), 0);
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

void Canvas::SetDim(int xIndex, int yIndex, int zIndex)
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

void Canvas::DrawLegend(QPainter &painter)
{
    int w = painter.viewport().width(), h = painter.viewport().height();
    QFont font = painter.font();
    font.setPointSize(10);
    painter.setFont(font);

    // we draw the reward colorbar
    if(!maps.reward.isNull())
    {
        // we draw the colorbar
        painter.setRenderHint(QPainter::Antialiasing, false);
        painter.setOpacity(0.8);
        int barW = 20;
        QRect rect(w - barW - 32, 40, barW, 256);
        painter.setBrush(Qt::NoBrush);
        for(int i=0; i<rect.height(); i++)
        {
            float v = (1.f - i/(float)rect.height())*255.f;
            v = max(0.f, min(255.f, v));
            painter.setPen(QColor(255,255-v,255-v));
            painter.drawLine(rect.x(), rect.y() + i, rect.x() + rect.width(), rect.y() + i);
        }

        // we draw the values on the colorbar
        painter.setOpacity(1);
        int steps = 4;
        float vmax = 1.0;
        float vmin = 0.0;
        for(int i=0; i<steps+1; i++)
        {
            float v = (1.f - i/(float)steps);
            QString text = QString("%1").arg(v*(vmax-vmin) + vmin, 0, 'f', 2);
            int y = rect.y() + i*rect.height()/steps;
            QRect textRect = QRect(rect.x()-40, y - 10, 40-6, 20);
            painter.setPen(Qt::black);
            painter.drawText(textRect, Qt::AlignRight + Qt::AlignVCenter, text);
            painter.drawLine(rect.x(), y, rect.x()-4, y);
        }
        painter.setPen(QPen(Qt::black, 1));
        painter.drawRect(rect);
    }
    else // we draw the samples legend
    {
        if(!data->GetCount()) return;
        std::map<int,bool> labelList;
        ivec labels = data->GetLabels();
        FOR(i, labels.size())
        {
            labelList[labels[i]] = true;
        }
        painter.setPen(QPen(Qt::black, 1));
        // we need to know the size of the legend rectangle
        int rectWidth = 0;
        QFontMetrics fm = painter.fontMetrics();
        for(map<int,bool>::iterator it=labelList.begin(); it != labelList.end(); it++)
        {
            QString className = GetClassString(it->first);
            QRect rect = fm.boundingRect(className);
            rectWidth = max(rectWidth, rect.width());
        }
        rectWidth += 10; // we add the sample size;

        int x = w - rectWidth - 40, y = 40;
        painter.setRenderHint(QPainter::Antialiasing, false);
        painter.drawRect(x-10,y-10, rectWidth+12, 20*labelList.size());
        painter.setRenderHint(QPainter::Antialiasing, true);
        for(map<int,bool>::iterator it=labelList.begin(); it != labelList.end(); it++)
        {
            int label = it->first;
            QPointF point(x, y);
            drawSample(painter, point, 10, label);
            QString className = GetClassString(label);
            painter.drawText(x + 8, point.y()+3, className);
            //painter.drawText(QRect(x + 4, point.y()-10, 70, 20), Qt::AlignLeft + Qt::AlignCenter, className);
            y += 20;
        }
    }
}

void Canvas::DrawAxes(QPainter &painter)
{
    int w = width();
    int h = height();
    // we find out how 'big' the space is
    QRectF bounding = canvasRect();
    // we round up the size to the closest decimal
    float scale = bounding.height();
    float scaleRatio = scale / bounding.width();
    if(scaleRatio > 1000 || 1.f/scaleRatio > 1000) scale = (bounding.height() + bounding.width()) / 2;
    if(scale <= 1e-5) return;
    float mult = 1;
    if(scale > 10)
    {
        while(scale / mult > 10 && mult != 0) mult *= 2.5f; // we want at most 10 lines to draw
    }
    else
    {
        while(scale / mult < 5 && mult != 0) mult /= 2.f; // we want at least 5 lines to draw
    }
    if(mult == 0) mult = 1;

    // we now have the measure of the ticks, we can draw this
    painter.setBackgroundMode(Qt::TransparentMode);
    painter.setRenderHint(QPainter::Antialiasing, true);
    //painter.setRenderHint(QPainter::TextAntialiasing);
    painter.setBrush(Qt::NoBrush);
    painter.setFont(QFont("Lucida Grande", 9));
    painter.setPen(QPen(Qt::black, 0.5, Qt::DotLine));
    // we draw the grid lines
    int minGridWidth = 32;
    if(!data->bProjected && data->IsCategorical(xIndex))
    {
        int cnt = data->categorical[xIndex].size();
        FOR(i, cnt)
        {
            float canvasX = toCanvasCoords(i,0).x();
            if(canvasX < 0 || canvasX > w) continue;
            painter.drawLine(canvasX, 0, canvasX, h);
        }
    }
    else
    {
        int cnt = 0;
        if ( bounding.width() < 0 ) {
            for(float x = (int)(bounding.x()/mult)*mult; x > bounding.x() + bounding.width(); x -= mult) cnt++;
        } else {
            for(float x = (int)(bounding.x()/mult)*mult; x < bounding.x() + bounding.width(); x += mult) cnt++;
        }
        if(!cnt) mult = minGridWidth/(float)w;
        else if(w/cnt < minGridWidth) mult *= (float)minGridWidth*cnt/w;
        for(float x = (int)(bounding.x()/mult)*mult; x < bounding.x() + bounding.width(); x += mult)
        {
            float canvasX = toCanvasCoords(x,0).x();
            if(canvasX < 0 || canvasX > w) continue;
            painter.drawLine(canvasX, 0, canvasX, h);
        }
    }
    painter.setPen(QPen(Qt::black, 0.5, Qt::DotLine));

    if(!data->bProjected && data->IsCategorical(yIndex))
    {
        int cnt = data->categorical[yIndex].size();
        FOR(i, cnt)
        {
            float canvasY = toCanvasCoords(0,i).y();
            if(canvasY < 0 || canvasY > w) continue;
            painter.drawLine(0, canvasY, w, canvasY);
        }
    }
    else
    {
        int cnt = 0;
        if ( bounding.width() < 0 ) {
            for(float y = (int)(bounding.y()/mult)*mult; y > bounding.y() + bounding.height(); y -= mult) cnt++;
        } else {
            for(float y = (int)(bounding.y()/mult)*mult; y < bounding.y() + bounding.height(); y += mult) cnt++;
        }
        if(!cnt) mult = minGridWidth/(float)w;
        if(w/cnt < minGridWidth) mult *= (float)minGridWidth*cnt/w;
        for(float y = (int)(bounding.y()/mult)*mult; y < bounding.y() + bounding.height(); y += mult)
        {
            float canvasY = toCanvasCoords(0,y).y();
            if(canvasY < 0 || canvasY > w) continue;
            painter.drawLine(0, canvasY, w, canvasY);
        }
    }
    // we draw the tick values
    painter.setPen(QPen(Qt::black, 0.5));
    if(!data->bProjected && data->IsCategorical(xIndex))
    {
        int cnt = data->categorical[xIndex].size();
        FOR(i, cnt)
        {
            string name = data->GetCategorical(xIndex, i);
            float canvasX = toCanvasCoords(i,0).x();
            if(canvasX < 0 || canvasX > w) continue;
            painter.drawText(canvasX, h-5, QString(name.c_str()));
        }
    }
    else
    {
        float gridW = 0;
        for(float x = (int)(bounding.x()/mult)*mult; x < bounding.x() + bounding.width(); x += mult)
        {
            float canvasX = toCanvasCoords(x,0).x();
            if(gridW == 0)
            {
                float x2 = toCanvasCoords(x + mult, 0).x();
                gridW = x2 - canvasX;
            }
            if(canvasX < 0 || canvasX > w) continue;
            float val = (int)(x/mult)*mult;
            QString s;
            if(mult >= 1) s = QString("%1").arg(val, 0, 'f', 0);
            else if(mult >= 0.1) s = QString("%1").arg(val, 0, 'f', 1);
            else if(mult >= 0.01) s = QString("%1").arg(val, 0, 'f', 2);
            else s = QString("%1").arg(val);
            painter.drawText(canvasX, h-15, max((float)minGridWidth,gridW), 10, Qt::AlignLeft | Qt::AlignBottom, s);
        }
    }
    // we now have the measure of the ticks, we can draw this
    painter.setPen(QPen(Qt::black, 0.5));
    if(!data->bProjected && data->IsCategorical(yIndex))
    {
        int cnt = data->categorical[yIndex].size();
        FOR(i, cnt)
        {
            string name = data->GetCategorical(yIndex, i);
            float canvasY = toCanvasCoords(0,i).y();
            if(canvasY < 0 || canvasY > w) continue;
            painter.drawText(2, canvasY, QString(name.c_str()));
        }
    }
    else
    {
        for(float y = (int)(bounding.y()/mult)*mult; y < bounding.y() + bounding.height(); y += mult)
        {
            float canvasY = toCanvasCoords(0,y).y();
            if(canvasY < 0 || canvasY > w) continue;
            float val = (int)(y/mult)*mult;
            QString s;
            if(mult >= 1) s = QString("%1").arg(val, 0, 'f', 0);
            else if(mult >= 0.1) s = QString("%1").arg(val, 0, 'f', 1);
            else if(mult >= 0.01) s = QString("%1").arg(val, 0, 'f', 2);
            else s = QString("%1").arg(val);
            painter.drawText(2, canvasY, s);
        }
    }

    // we get the dimension names
    QFont font = painter.font();
    font.setPointSize(12);
    painter.setFont(font);
    if(xIndex < dimNames.size())
    {
        QString xlabel = dimNames[xIndex];
        painter.setPen(QPen(Qt::black, 0.5));
        painter.drawText(w/2 - 100, h + 10, 200, 10, Qt::AlignTop | Qt::AlignHCenter, xlabel);
    }
    if(yIndex < dimNames.size())
    {
        QString ylabel = dimNames[yIndex];
        painter.setPen(QPen(Qt::black, 0.5));
        painter.rotate(-90);
        painter.drawText(-h/2-100, -20, 200, 10, Qt::AlignTop | Qt::AlignHCenter, ylabel);
        painter.rotate(90);
    }
}

void Canvas::RedrawAxes()
{
    int w = width();
    int h = height();
    maps.grid = QPixmap(w,h);
    //QBitmap bitmap(w,h);
    //bitmap.clear();
    //maps.grid.setMask(bitmap);
    maps.grid.fill(Qt::transparent);

    QPainter painter(&maps.grid);
    DrawAxes(painter);
}

void Canvas::DrawSamples(QPainter &painter)
{
    int radius = 10;
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    for(int i=0; i<data->GetCount(); i++)
    {
        if(data->GetFlag(i) == _TRAJ) continue;
        int label = data->GetLabel(i);
        QPointF point = toCanvasCoords(data->GetSample(i));
        Canvas::drawSample(painter, point, (data->GetFlag(i)==_TRAJ)?5:radius, bDisplaySingle ? 0 : label);
    }
}

void Canvas::DrawSampleColors(QPainter &painter)
{
    int radius = 10;
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    for(int i=0; i<data->GetCount(); i++)
    {
        if(i >= sampleColors.size()) continue;
        QColor color = sampleColors[i];
        QPointF point = toCanvasCoords(data->GetSample(i));
        painter.setBrush(color);
        painter.setPen(Qt::black);
        painter.drawEllipse(QRectF(point.x()-radius/2.,point.y()-radius/2.,radius,radius));
    }
}

void Canvas::DrawSamples()
{
    int radius = 10;
    if(!data->GetCount())
    {
        int w = width();
        int h = height();
        maps.samples = QPixmap(w,h);
        //QBitmap bitmap(w,h);
        //bitmap.clear();
        //maps.samples.setMask(bitmap);
        maps.samples.fill(Qt::transparent);
        drawnSamples = 0;
        return;
    }
    if(drawnSamples == data->GetCount()) return;
    if(drawnSamples > data->GetCount()) drawnSamples = 0;

    if(!drawnSamples || maps.samples.isNull())
    {
        int w = width();
        int h = height();
        maps.samples = QPixmap(w,h);
        //QBitmap bitmap(w,h);
        //bitmap.clear();
        //maps.samples.setMask(bitmap);
        maps.samples.fill(Qt::transparent);
        drawnSamples = 0;
        //maps.model = QPixmap(w,h);
        //maps.model.setMask(bitmap);
        //maps.model.fill(Qt::transparent);
    }
    QPainter painter(&maps.samples);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    for(int i=drawnSamples; i<data->GetCount(); i++)
    {
        if(data->GetFlag(i) == _TRAJ) continue;
        int label = data->GetLabel(i);
        fvec sample = data->GetSample(i);
        QPointF point = toCanvasCoords(sample);
        Canvas::drawSample(painter, point, (data->GetFlag(i)==_TRAJ)?5:radius, bDisplaySingle ? 0 : label);
    }
    drawnSamples = data->GetCount();
}

void Canvas::DrawTargets(QPainter &painter)
{
    painter.setRenderHint(QPainter::Antialiasing, true);
    FOR(i, targets.size())
    {
        QPointF point = toCanvasCoords(targets[i]);
        QPointF delta1 = QPointF(1,1);
        QPointF delta2 = QPointF(1,-1);
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(Qt::black, 1.5));
        int r = 8, p = 2;
        painter.drawEllipse(point,r,r);
        painter.drawLine(point+delta1*r, point+delta1*r+delta1*p);
        painter.drawLine(point-delta1*r, point-delta1*r-delta1*p);
        painter.drawLine(point+delta2*r, point+delta2*r+delta2*p);
        painter.drawLine(point-delta2*r, point-delta2*r-delta2*p);
    }
}

QPainterPath Canvas::DrawObstacle(Obstacle o)
{
    QPointF point;
    float aX = o.axes[0];
    float aY = o.axes[1];
    float angle = o.angle;
    float pX = o.power[0];
    float pY = o.power[1];
    QPainterPath obstaclePath;
    QPointF firstPoint;
    // first we draw the obstacle
    for(float theta=-PIf; theta < PIf + 0.1; theta += 0.1f)
    {
        float X, Y;
        X = aX * cosf(theta);
        Y = aY * (theta>=0?1.f:-1.f) * powf((1-powf(cosf(theta),2.f*pX)),1./(2*pY));

        float RX = + X * cosf(angle) - Y * sinf(angle);
        float RY = + X * sinf(angle) + Y * cosf(angle);

        point = QPointF(RX*(zoom*zooms[xIndex]*height()),-RY*(zoom*zooms[yIndex]*height()));
        if (theta==-PIf) {
            firstPoint = point;
            obstaclePath.moveTo(point);
        } else {
            obstaclePath.lineTo(point);
        }
    }
    obstaclePath.lineTo(firstPoint);
    return obstaclePath;
}

void Canvas::DrawObstacles(QPainter &painter)
{
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    // we draw the obstacles
    if(!data->GetObstacles().size()) return;
    QList<QPainterPath> paths;
    QList<QPainterPath> safeties;
    FOR(i, data->GetObstacles().size())
    {
        QPainterPath obstaclePath = DrawObstacle(data->GetObstacle(i));
        obstaclePath.translate(toCanvasCoords(data->GetObstacle(i).center));
        paths.push_back(obstaclePath);
        obstaclePath = DrawObstacle(data->GetObstacle(i));

        QMatrix scalingMatrix;
        QPointF t = toCanvasCoords(data->GetObstacle(i).center);
        scalingMatrix.scale(data->GetObstacle(i).repulsion[0], data->GetObstacle(i).repulsion[1]);
        obstaclePath = scalingMatrix.map(obstaclePath);
        obstaclePath.translate(toCanvasCoords(data->GetObstacle(i).center));
        safeties.push_back(obstaclePath);
    }
    FOR(i, paths.size())
    {
        painter.setBrush(Qt::white);
        painter.setPen(QPen(Qt::black, 1,Qt::SolidLine));
        painter.drawPath(paths[i]);
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(Qt::black, 1,Qt::DotLine));
        painter.drawPath(safeties[i]);
    }
}

void Canvas::DrawObstacles()
{
    int w = width();
    int h = height();
    maps.obstacles = QPixmap(w,h);
    //QBitmap bitmap(w,h);
    //bitmap.clear();
    //maps.obstacles.setMask(bitmap);
    maps.obstacles.fill(Qt::transparent);

    QPainter painter(&maps.obstacles);
    DrawObstacles(painter);
}

void Canvas::DrawRewards()
{
    return;
    int w = width();
    int h = height();
    maps.reward= QPixmap(w,h);
    //QBitmap bitmap(w,h);
    //bitmap.clear();
    //maps.reward.setMask(bitmap);
    maps.reward.fill(Qt::transparent);

    if(!data->GetReward()->rewards) return;

    QPainter painter(&maps.reward);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);

    int radius = 10;
    int stepsW = w/radius;
    int stepsH = h/radius;
    //int radius = min(w,h) / steps;
    // we draw the rewards
    QColor color;
    fvec sample(2);
    FOR(i, stepsH)
    {
        float y = i/(float)stepsH*h;
        FOR(j, stepsW)
        {
            float x = j/(float)stepsW*w;
            float value = data->GetReward()->ValueAt(toSampleCoords(x,y));
            if(value > 0) color = QColor(255, 255 - (int)(max(0.f,min(1.f, value)) * 255), 255 - (int)(max(0.f,min(1.f, value)) * 255));
            else color = QColor(255 - (int)(max(0.f,min(1.f, -value)) * 255),255 - (int)(max(0.f,min(1.f, -value)) * 255),255);
            painter.setBrush(color);
            painter.setPen(Qt::black);
            painter.drawEllipse(QRectF(x-radius/2.,y-radius/2.,radius,radius));
        }
    }
}

void Canvas::DrawTrajectories(QPainter &painter)
{
    int w = width();
    int h = height();
    int count = data->GetCount();

    bool bDrawing = false;

    vector<ipair> sequences = data->GetSequences();
    int start=0, stop=0;
    if(data->GetFlag(count-1) == _TRAJ)
    {
        if(sequences.size()) stop = sequences[sequences.size()-1].second;
        if(stop < count-1) // there's an unfinished trajectory
        {
            stop++;
            for(start=count-1; start >= stop && data->GetFlag(start) == _TRAJ; start--);
            sequences.push_back(ipair(start+(sequences.size() ? 1 : 0),count-1));
            bDrawing = true;
        }
    }

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);

    vector<fvec> samples = data->GetSamples();

    map<int,int> counts;
    centers.clear();
    if(trajectoryCenterType)
    {
        FOR(i, sequences.size())
        {
            int index = sequences[i].first;
            if(trajectoryCenterType==1) // end
            {
                index = sequences[i].second;
            }
            int label = data->GetLabel(index);
            if(!centers.count(label))
            {
                fvec center(2,0);
                centers[label] = center;
                counts[label] = 0;
            }
            centers[label] += samples[index];
            counts[label]++;
        }

        for(map<int,int>::iterator p = counts.begin(); p!=counts.end(); ++p)
        {
            int label = p->first;
            centers[label] /= p->second;
        }
    }

    // do the interpolation
    vector< vector<fvec> > trajectories;
    vector<fvec> diffs;
    ivec trajLabels;
    FOR(i, sequences.size())
    {
        start = sequences[i].first;
        stop = sequences[i].second;
        int label = data->GetLabel(start);
        fvec diff(2,0);
        if(trajectoryCenterType && (i < sequences.size()-1 || !bDrawing))
        {
            diff = centers[label] - samples[trajectoryCenterType==1?stop:start];
        }
        vector<fvec> trajectory(stop-start+1);
        int pos = 0;
        for (int j=start; j<=stop; j++)
        {
            trajectory[pos++] = samples[j] + diff;
        }
        switch (trajectoryResampleType)
        {
        case 0: // do nothing
            break;
        case 1: // uniform resampling
            {
                if(i < sequences.size()-1 || !bDrawing)
                {
                    trajectory = interpolate(trajectory, trajectoryResampleCount);
                }
            }
            break;
        case 2: // spline resampling
            {
                if(i < sequences.size()-1 || !bDrawing)
                {
                    trajectory = interpolateSpline(trajectory, trajectoryResampleCount);
                }
            }
            break;
        }
        trajectories.push_back(trajectory);
        trajLabels.push_back(data->GetLabel(start));
    }

    // let's draw the trajectories
    FOR(i, trajectories.size())
    {
        fvec oldPt = trajectories[i][0];
        int count = trajectories[i].size();
        int label = trajLabels[i];
        FOR(j, count-1)
        {
            fvec pt = trajectories[i][j+1];
            painter.setPen(QPen(Qt::black, 0.5));
            painter.drawLine(toCanvasCoords(pt), toCanvasCoords(oldPt));
            if(j<count-2) Canvas::drawSample(painter, toCanvasCoords(pt), 5, bDisplaySingle ? 0 : label);
            oldPt = pt;
        }
        painter.setBrush(Qt::NoBrush);
        painter.setPen(Qt::green);
        painter.drawEllipse(toCanvasCoords(trajectories[i][0]), 5, 5);
        if(!bDrawing)
        {
            painter.setPen(Qt::red);
            painter.drawEllipse(toCanvasCoords(trajectories[i][count-1]), 5, 5);
        }
    }
}

void Canvas::DrawTrajectories()
{
    int w = width();
    int h = height();
    int count = data->GetCount();
    if(!count || (!data->GetSequences().size() && (data->GetFlag(count-1) != _TRAJ)))
    {
        maps.trajectories = QPixmap(w,h);
        //QBitmap bitmap(w,h);
        //bitmap.clear();
        //maps.trajectories.setMask(bitmap);
        maps.trajectories.fill(Qt::transparent);
        drawnTrajectories = 0;
    }

    bool bDrawing = false;

    vector<ipair> sequences = data->GetSequences();
    int start=0, stop=0;
    if(data->GetFlag(count-1) == _TRAJ)
    {
        if(sequences.size()) stop = sequences.back().second;
        if(stop < count-1) // there's an unfinished trajectory
        {
            stop++;
            for(start=count-1; start >= stop && data->GetFlag(start) == _TRAJ; start--);
            sequences.push_back(ipair(start+(sequences.size() ? 1 : 0),count-1));
            bDrawing = true;
        }
    }
    if(!bDrawing && drawnTrajectories == sequences.size()) return;
    if(drawnTrajectories > sequences.size()) drawnTrajectories = 0;

    if(!drawnTrajectories || maps.trajectories.isNull())
    {
        maps.trajectories = QPixmap(w,h);
        //QBitmap bitmap(w,h);
        //bitmap.clear();
        //maps.trajectories.setMask(bitmap);
        maps.trajectories.fill(Qt::transparent);
        drawnTrajectories = 0;
    }

    QPainter painter(&maps.trajectories);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);

    ivec trajLabels(sequences.size());
    FOR(i, sequences.size())
    {
        trajLabels[i] = data->GetLabel(sequences[i].first);
    }
    vector< vector<fvec> > trajectories = data->GetTrajectories(trajectoryResampleType, trajectoryResampleCount, trajectoryCenterType, 0.1, true);
    if(bDrawing)
    {
        vector<fvec> trajectory(sequences.back().second-sequences.back().first);
        for(int i=sequences.back().first; i<sequences.back().second; i++)
        {
            trajectory[i-sequences.back().first] = data->GetSample(i);
        }
        if(trajectory.size()) trajectories.push_back(trajectory);
    }
    // let's draw the trajectories
    for(int i=drawnTrajectories; i<trajectories.size(); i++)
    {
        fvec oldPt = trajectories[i][0];
        int count = trajectories[i].size();
        int label = trajLabels[i];
        FOR(j, count-1)
        {
            fvec pt = trajectories[i][j+1];
            int dim = pt.size();
            float x = pt[xIndex];
            float y = pt[yIndex];
            painter.setPen(QPen(Qt::black, 0.5));
            QPointF point = toCanvasCoords(pt);
            QPointF oldPoint = toCanvasCoords(oldPt);
            painter.drawLine(point, toCanvasCoords(oldPt));
            if(j<count-2) Canvas::drawSample(painter, point, 5, bDisplaySingle ? 0 : label);
            oldPt = pt;
        }
        painter.setBrush(Qt::NoBrush);
        painter.setPen(Qt::green);
        painter.drawEllipse(toCanvasCoords(trajectories[i][0]), 5, 5);
        if(!bDrawing)
        {
            painter.setPen(Qt::red);
            painter.drawEllipse(toCanvasCoords(trajectories[i][count-1]), 5, 5);
        }
    }
    drawnTrajectories = !bDrawing ? sequences.size() : sequences.size()-1;
}

void Canvas::DrawLiveTrajectory(QPainter &painter)
{
    if(!liveTrajectory.size() || !liveTrajectory[0].size()) return;
    int w = width();
    int h = height();
    fvec oldPt = liveTrajectory[0];
    int count = liveTrajectory.size();
    FOR(j, count-1)
    {
        fvec pt = liveTrajectory[j+1];
        if(!pt.size()) break;
        int label = 1;
        if(false && bDisplayMap)
        {
            painter.setPen(QPen(Qt::white, 3));
            painter.drawLine(toCanvasCoords(pt), toCanvasCoords(oldPt));
            painter.setPen(QPen(Qt::black, 1));
            painter.drawLine(toCanvasCoords(pt), toCanvasCoords(oldPt));
        }
        else
        {
            painter.setPen(QPen(Qt::magenta, 2));
            painter.drawLine(toCanvasCoords(pt), toCanvasCoords(oldPt));
        }
        //if(j<count-2) Canvas::drawSample(painter, QPoint(pt[0]*w, pt[1]*h), 5, label);
        oldPt = pt;
    }
    painter.setBrush(Qt::NoBrush);
    painter.setPen(Qt::green);
    painter.drawEllipse(toCanvasCoords(liveTrajectory[0]), 5, 5);
    painter.setPen(Qt::red);
    painter.drawEllipse(toCanvasCoords(liveTrajectory[count-1]), 5, 5);
}

void Canvas::DrawTimeseries()
{
    int w = width();
    int h = height();
    if(!drawnTimeseries || maps.timeseries.isNull())
    {
        maps.timeseries = QPixmap(w,h);
        //QBitmap bitmap(w,h);
        //bitmap.clear();
        //maps.timeseries.setMask(bitmap);
        maps.timeseries.fill(Qt::transparent);
        drawnTimeseries = 0;
    }

    vector<TimeSerie> timeseries = data->GetTimeSeries();
    if((!timeseries.size() && drawnTimeseries) || (timeseries.size() == drawnTimeseries)) return;

    if(drawnTimeseries > timeseries.size()) drawnTimeseries = 0;

    QPainter painter(&maps.timeseries);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);

    //qDebug() << "drawing: " << timeseries.size() << "series";
    // we draw all the timeseries, each with its own color
    for(int i=drawnTimeseries; i < timeseries.size(); i++)
    {
        painter.setPen(QPen(SampleColor[i%(SampleColorCnt-1)+1],0.5));
        TimeSerie &t = timeseries[i];
        if(t.size() < 2) continue;
        QPointF p0,p1;
        float count = t.timestamps.size();
        p0 = toCanvasCoords(t.timestamps[0] / count, t.data[0][yIndex-1]);
        FOR(j, t.size()-1)
        {
            float value = t.data[j+1][yIndex-1];
            p1 = toCanvasCoords(t.timestamps[j+1] / count, value);
            if(t.timestamps[j] == -1 || t.timestamps[j+1] == -1) continue;
            painter.drawLine(p0, p1);
            p0 = p1;
        }
    }
    drawnTimeseries = timeseries.size();
}

void Canvas::ResizeEvent()
{
    if(!canvasType && (width() != parentWidget()->width() || height() != parentWidget()->height())) resize(parentWidget()->size());
    bNewCrosshair = true;
    if(!maps.reward.isNull())
    {
        QPixmap newReward(width(), height());
        newReward = maps.reward.scaled(newReward.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
    if(!canvasType) RedrawAxes();
}

void Canvas::mousePressEvent( QMouseEvent *event )
{
    int x = event->x();
    int y = event->y();

    fvec sample = toSampleCoords(x,y);

    int label = 0;
    if(event->button()==Qt::LeftButton) label = 1;
    if(event->button()==Qt::RightButton) label = 0;

    if(canvasType == 0)
    {
        if(event->modifiers()==Qt::AltModifier)
        {
            mouseAnchor = event->pos();
            return;
        }
        emit Drawing(sample, label);
    }
}

void Canvas::mouseReleaseEvent( QMouseEvent *event )
{
    int x = event->x();
    int y = event->y();

    fvec sample = toSampleCoords(x,y);

    int label = 0;
    if(event->button()==Qt::LeftButton) label = 1;
    if(event->button()==Qt::RightButton) label = 0;

    if(canvasType == 0)
    {
        mouseAnchor = QPoint(-1,-1);
        if(x > 0 && x < width() && y>0 && y<height()) bShowCrosshair = true;
        //emit Drawing(sample, label);
        emit Released();
    }
}

void Canvas::enterEvent(QEvent *event)
{
    bShowCrosshair = true;
    repaint();
}

void Canvas::focusOutEvent(QFocusEvent *event)
{
    bShowCrosshair = false;
    bNewCrosshair = true;
    repaint();
}

void Canvas::leaveEvent(QEvent *event)
{
    bShowCrosshair = false;
    bNewCrosshair = true;
    //mouseAnchor = QPoint(-1,-1);
    repaint();
}
void Canvas::Clear()
{
    maps.grid = QPixmap();
    maps.model = QPixmap();
    maps.confidence = QPixmap();
    maps.info = QPixmap();
    maps.obstacles = QPixmap();
    maps.trajectories = QPixmap();
    maps.samples = QPixmap();
    ResetSamples();
    bNewCrosshair = true;
    repaint();
}

void Canvas::wheelEvent(QWheelEvent *event)
{
    if(canvasType) return;
    if(event->modifiers() == Qt::ShiftModifier)
    {
        zooms[xIndex] += event->delta()/1000.f;
        qDebug() << "zooms[" << xIndex << "]: " << zooms[xIndex];

        maps.grid = QPixmap();
        maps.model = QPixmap();
        maps.confidence = QPixmap();
        //maps.reward = QPixmap();
        maps.info = QPixmap();
        ResetSamples();
        bNewCrosshair = true;
        repaint();

        emit(Navigation(fVec(-1,0.001)));
        return;
    }
    float d = 0;
    if (event->delta() > 100) d = 1;
    if (event->delta() < 100) d = -1;
    if(d!=0) emit Navigation(fVec(-1,d));
}

void Canvas::mouseMoveEvent( QMouseEvent *event )
{
    if(canvasType) return;
    int x = event->x();
    int y = event->y();
    mouse = QPoint(x,y);
    fvec sample = toSampleCoords(x,y);

    if(mouseAnchor.x() == -1) mouseAnchor = event->pos();
    // we navigate in our environment
    if(event->modifiers() == Qt::AltModifier && event->buttons() == Qt::LeftButton)
    {
        fVec d = (fromCanvas(mouseAnchor) - fromCanvas(event->pos()));
        qDebug() << "mouse" << event->pos() << "anchor" << mouseAnchor << "diff:" << d.x << d.y;
        if(d.x == 0 && d.y == 0) return;
        SetCenter(center + d);
        mouseAnchor = event->pos();
        bShowCrosshair = false;
        emit CanvasMoveEvent();
        return;
    }

    if(event->buttons() != Qt::LeftButton && event->buttons() != Qt::RightButton )
    {
        emit Navigation(sample);
        repaint();
    }
    else
    {
        int label = 0;
        if(event->buttons()==Qt::LeftButton) label = 1;
        if(event->buttons()==Qt::RightButton) label = 0;
        emit Drawing(sample, label);
    }
}

bool Canvas::SaveScreenshot( QString filename )
{
    QPixmap screenshot = GetScreenshot();
    return screenshot.save(filename);
}

QPixmap Canvas::GetScreenshot()
{
    QPixmap screenshot(width(), height());
    QPainter painter(&screenshot);
    bool tmp = bShowCrosshair;
    bShowCrosshair = false;
    painter.setBackgroundMode(Qt::OpaqueMode);
    painter.setBackground(Qt::white);
    if(!canvasType) PaintStandard(painter);
    else if(canvasType <= 5) PaintMultivariate(painter, canvasType-2); // 0: standard, 1: 3D View, so we take out 2
    else
    {
        fvec params;
        params.push_back(xIndex);
        params.push_back(yIndex);
        params.push_back(zIndex);
        PaintVariable(painter, canvasType-6, params);
    }
    bShowCrosshair = tmp;
    return screenshot;
}

ivec Canvas::SelectSamples(QPointF center, float radius , fvec *weights)
{
    ivec selection;
    if(weights) (*weights).clear();
    int closest = 0;
    float minDist = FLT_MAX;
    FOR(i, data->GetCount())
    {
        QPointF dataPoint = toCanvasCoords(data->GetSample(i));
        QPointF point = this->mapToParent(QPoint(dataPoint.x(), dataPoint.y()));
        point -= center;
        float dist = point.x()*point.x() + point.y()*point.y();
        if(radius > 0)
        {
            if(!weights)
            {
                if(sqrtf(dist) < radius) selection.push_back(i);
            }
            else
            {
                if(sqrtf(dist) < radius*1.5f)
                {
                    selection.push_back(i);
                    float weight = sqrtf(dist)/radius;
                    (*weights).push_back(weight);
                }
            }
        }
        else
        {
            if(dist < minDist)
            {
                closest = i;
                minDist = dist;
            }
        }
    }
    if(radius < 0)
    {
        selection.push_back(closest);
    }
    return selection;
}

bool Canvas::DeleteData( QPointF center, float radius )
{
    bool anythingDeleted = false;
    FOR (i, data->GetCount()) {
        QPointF dataPoint = toCanvasCoords(data->GetSample(i));
        QPointF point = this->mapToParent(QPoint(dataPoint.x(), dataPoint.y()));
        point -= center;
        if (sqrt(point.x()*point.x() + point.y()*point.y()) < radius) {
            anythingDeleted = true;
            data->RemoveSample(i);
            i--;
        }
    }
    FOR (i, data->GetObstacles().size()) {
        QPointF obstaclePoint= toCanvasCoords(data->GetObstacle(i).center);
        QPointF point = this->mapToParent(QPoint(obstaclePoint.x(), obstaclePoint.y()));
        point -= center;
        if (sqrt(point.x()*point.x() + point.y()*point.y()) < radius) {
            anythingDeleted = true;
            data->RemoveObstacle(i);
            i--;
        }
    }
    FOR (i, targets.size()) {
        QPointF targetPoint= toCanvasCoords(targets[i]);
        QPointF point = this->mapToParent(QPoint(targetPoint.x(), targetPoint.y()));
        point -= center;
        if(sqrt(point.x()*point.x() + point.y()*point.y()) < radius)
        {
            anythingDeleted = true;
            targets.erase(targets.begin() + i);
            targetAge.erase(targetAge.begin() + i);
            i--;
            anythingDeleted = true;
        }
    }
    return anythingDeleted;
}

void Canvas::PaintReward(fvec sample, float radius, float shift)
{
    int w = width();
    int h = height();
    if (maps.reward.isNull()) {
        maps.reward = QPixmap(w,h);
        //QBitmap bitmap(w,h);
        //bitmap.clear();
        //maps.reward.setMask(bitmap);
        maps.reward.fill(Qt::transparent);
        maps.reward.fill(Qt::white);
    }
    QPainter painter(&maps.reward);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

    QPointF center = toCanvasCoords(sample);
    QRadialGradient gradient( center, radius*.75);
    if (shift > 0) {
        gradient.setColorAt(0, QColor(255,0,0,shift*255));
        gradient.setColorAt(1, QColor(255,0,0,0));
    } else {
        gradient.setColorAt(0, QColor(255,255,255,-shift*255));
        gradient.setColorAt(1, QColor(255,255,255,0));
    }
    painter.setBrush(gradient);
    //if(shift > 0) painter.setBrush(QColor(255,0,0,shift*255));
    //else painter.setBrush(QColor(255,255,255,-shift*255));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(toCanvasCoords(sample), radius, radius);
}

void Canvas::PaintGaussian(QPointF position, double variance)
{
    int w = width();
    int h = height();
    if (maps.reward.isNull()) {
        maps.reward = QPixmap(w,h);
        //QBitmap bitmap(w,h);
        //bitmap.clear();
        //maps.reward.setMask(bitmap);
        maps.reward.fill(Qt::transparent);
        maps.reward.fill(Qt::white);
    }

    QImage image(w, h, QImage::Format_ARGB32);
    image.fill(qRgb(255,255,255));
    fVec pos(position.x()/(float)w, position.y()/(float)h);
    fVec point;
    float invSigma = 1./(variance*variance);
    float a = invSigma*sqrtf(2*PIf);
    float value;
    float minVal = 1e30, maxVal = -1e30;
    qDebug() << "gaussian dropped at position " << position;
    FOR (i, w) {
        point.x = i/(float)w;
        FOR (j, h) {
            point.y = j/(float)h;
            value = (pos - point).lengthSquared();
            value = expf(-0.5*value*invSigma);
            value = (1.f - value);
            if(value < minVal) minVal = value;
            if(value > maxVal) maxVal = value;
            int color = 255.f*value;
            //			if(color > 255) color = 255;
            //			if(color < -255) color = 0;
            //int color = min(255, max(0, (int)(255.f*(1.f - value))));
            image.setPixel(i,j,qRgba(255, color, color, 255));
        }
    }
    QPainter painter(&maps.reward);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setCompositionMode(QPainter::CompositionMode_Darken);
    painter.drawPixmap(0,0,w,h,QPixmap::fromImage(image));
}

void Canvas::PaintGradient(QPointF position)
{
    int w = width();
    int h = height();
    if(maps.reward.isNull())
    {
        maps.reward = QPixmap(w,h);
        //QBitmap bitmap(w,h);
        //bitmap.clear();
        //maps.reward.setMask(bitmap);
        maps.reward.fill(Qt::transparent);
        maps.reward.fill(Qt::white);
    }
    QPainter painter(&maps.reward);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

    QPointF center(w/2.f, h/2.f);
    QPointF opposite = center - (position-center);
    QLinearGradient gradient(opposite, position);
    gradient.setColorAt(0, QColor(255,255,255,255));
    gradient.setColorAt(1, QColor(255,0,0,255));
    painter.setBrush(gradient);
    painter.setPen(Qt::NoPen);
    painter.drawRect(maps.reward.rect());
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
