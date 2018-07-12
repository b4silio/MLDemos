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

    //fvec sample = toSampleCoords(x,y);
    //int label = 0;
    //if(event->button()==Qt::LeftButton) label = 1;
    //if(event->button()==Qt::RightButton) label = 0;

    if(canvasType == 0) {
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

void Canvas::wheelEvent(QWheelEvent *event)
{
    if(canvasType) return;
    if(event->modifiers() == Qt::ShiftModifier)
    {
        zooms[xIndex] += event->delta()/1000.f;
        if(zooms[xIndex] < 0.001) zooms[xIndex] == 0.001;
        //qDebug() << "zooms[" << xIndex << "]: " << zooms[xIndex];

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
    if(event->modifiers() == Qt::AltModifier)
    {
        if(event->buttons() == Qt::LeftButton || event->buttons() == Qt::RightButton) {
            fVec d = (fromCanvas(mouseAnchor.x(), mouseAnchor.y()) - fromCanvas(x,y));
            if(d.x == 0 && d.y == 0) return;
            SetCenter(center + d);
            mouseAnchor = event->pos();
            bShowCrosshair = false;
            emit CanvasMoveEvent();
        }
        bShowCrosshair = false;
        repaint();
        return;
    }
    bShowCrosshair = true;

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
        }
    }
    return anythingDeleted;
}
