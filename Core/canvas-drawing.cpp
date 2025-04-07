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
#include <QTransform>
#include <iostream>

#include "expose.h"
#include "public.h"
#include "basicMath.h"
#include "canvas.h"
#include "drawUtils.h"

using namespace std;

void Canvas::paintEvent(QPaintEvent *event)
{
    if(bDrawing) return;
    if(canvasType != 0) return; // we only draw if we're actually on the canvas
    bDrawing = true;
    QPainter painter(this);
    PaintBufferedCanvas(painter);
    bDrawing = false;
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

void Canvas::PaintBufferedCanvas(QPainter &painter, bool bSvg)
{
    bool bHighDPI = qApp->devicePixelRatio() > 1;
    if(bHighDPI && data->GetSamples().size() > 0 && data->GetSamples().size() < 250) {
        PaintSequentialCanvas(painter, bSvg);
        return;
    }
    bool bUseTimer = false;
    QElapsedTimer timer;
    if(bUseTimer) timer.start();;
    painter.setBackgroundMode(Qt::OpaqueMode);
    painter.setBackground(Qt::white);
    painter.fillRect(geometry(),Qt::white);
    painter.setRenderHint(QPainter::Antialiasing);

    if(bDisplayMap && !maps.confidence.isNull())
    {
        painter.drawPixmap(geometry(), maps.confidence);
    }
    painter.setBackgroundMode(Qt::TransparentMode);

    if(bDisplaySamples) {
        DrawRewards();
        if(!maps.reward.isNull()) {
            painter.setBackgroundMode(Qt::OpaqueMode);
            painter.drawPixmap(geometry(), maps.reward);
            painter.setBackgroundMode(Qt::TransparentMode);
        }
        if(bSvg) {
            DrawSamples(painter);
            DrawObstacles(painter);
        } else {
            DrawSamples();
            painter.drawPixmap(geometry(), maps.samples);
            DrawObstacles();
            painter.drawPixmap(geometry(), maps.obstacles);
        }
    }
    if(bDisplayTrajectories && data->GetSequences().size() > 0) {
        if(bSvg) {
            DrawTrajectories(painter);
        } else {
            DrawTrajectories();
            painter.drawPixmap(geometry(), maps.trajectories);
        }
    }
    if(bDisplayTrajectories && targets.size()) {
        DrawTargets(painter);
    }
    if(bDisplayTimeSeries && data->GetTimeSeries().size() > 0) {
        if(bSvg) {
        } else {
            DrawTimeseries();
            painter.drawPixmap(geometry(), maps.timeseries);
        }
    }
    if(!bSvg && bDisplayLearned) {
        if(maps.model.isNull()) {
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
        painter.drawPixmap(geometry(), maps.model);
    }
    if(!maps.animation.isNull()) {
        painter.drawPixmap(geometry(), maps.animation);
    }
    if(!bSvg && bDisplayInfo && !maps.info.isNull()) {
        painter.drawPixmap(geometry(), maps.info);
    }
    if(!bSvg && bShowCrosshair) {
        if(bNewCrosshair) emit DrawCrosshair();
        painter.drawPath(crosshair.translated(mouse));
        if(liveTrajectory.size()) DrawLiveTrajectory(painter);
    }
    if(bDisplayGrid) {
        if(bSvg || bHighDPI) {
            DrawAxes(painter);
        } else {
            if(maps.grid.isNull()) RedrawAxes();
            painter.drawPixmap(geometry(), maps.grid);
        }
    }
    if(bDisplayLegend) {
        DrawLegend(painter);
    }
    if(bUseTimer) qDebug() << timer.nsecsElapsed()/1000 << "µsec/t" << "Drawing Buffered Canvas";
}

void Canvas::PaintSequentialCanvas(QPainter &painter, bool bSvg)
{
    bool bUseTimer = false;
    QElapsedTimer timer;
    if(bUseTimer) timer.start();
    painter.setBackgroundMode(Qt::OpaqueMode);
    painter.setBackground(Qt::white);
    painter.fillRect(geometry(),Qt::white);
    painter.setRenderHint(QPainter::Antialiasing);

    if(bDisplayMap && !maps.confidence.isNull())
    {
        painter.drawPixmap(geometry(), maps.confidence);
    }

    if(bDisplaySamples) {
        if(!maps.reward.isNull()) {
            painter.drawPixmap(geometry(), maps.reward);
        }
        DrawSamples(painter);
        DrawObstacles(painter);
    }
    if(bDisplayTrajectories && data->GetSequences().size() > 0) {
        DrawTrajectories(painter);
        if(targets.size()) DrawTargets(painter);
    }
    if(bDisplayTimeSeries && data->GetTimeSeries().size() > 0) {
        if(bSvg) {
        } else {
            DrawTimeseries();
            painter.drawPixmap(geometry(), maps.timeseries);
        }
    }
    if(!bSvg && bDisplayLearned) {
        DrawSampleColors(painter);
        if(!maps.model.isNull()) {
            painter.setBackgroundMode(Qt::TransparentMode);
            painter.drawPixmap(geometry(), maps.model);
            painter.setBackgroundMode(Qt::OpaqueMode);
        }
    }
    if(!maps.animation.isNull()) {
        painter.drawPixmap(geometry(), maps.animation);
    }
    if(!bSvg && bDisplayInfo && !maps.info.isNull()) {
        painter.drawPixmap(geometry(), maps.info);
    }
    if(!bSvg && bShowCrosshair) {
        if(bNewCrosshair) emit DrawCrosshair();
        painter.setPen(Qt::black);
        painter.setBrush(Qt::NoBrush);
        painter.drawPath(crosshair.translated(mouse));
        if(liveTrajectory.size()) DrawLiveTrajectory(painter);
    }
    if(bDisplayGrid) {
        DrawAxes(painter);
    }
    if(bDisplayLegend) {
        DrawLegend(painter);
    }
    if(bUseTimer) qDebug() << timer.nsecsElapsed()/1000 << "µsec/t" << "Drawing Sequential Canvas";
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
        maps.reward.fill(Qt::white);
    }

    QImage image(w, h, QImage::Format_ARGB32);
    image.fill(qRgb(255,255,255));
    fVec pos(position.x()/(float)w, position.y()/(float)h);
    fVec point;
    float invSigma = 1./(variance*variance);
    //float a = invSigma*sqrtf(2*PIf);
    float value;
    float minVal = 1e30, maxVal = -1e30;
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
    if(!canvasType) PaintBufferedCanvas(painter);
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

void Canvas::DrawLegend(QPainter &painter)
{
    int w = painter.viewport().width();
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
            painter.drawText(textRect, Qt::AlignRight | Qt::AlignVCenter, text);
            painter.drawLine(rect.x(), y, rect.x()-4, y);
        }
        painter.setPen(QPen(Qt::black, 1));
        painter.drawRect(rect);
        painter.setRenderHint(QPainter::Antialiasing, true);
    }
    else // we draw the samples legend
    {
        if(data->GetCount())
        {
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
            FORIT(labelList, int, bool)
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
            FORIT(labelList, int, bool)
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
}

void Canvas::DrawAxes(QPainter &painter)
{
    if(xIndex == yIndex) return DrawAxes1D(painter);

    int w = width();
    int h = height();
    int pad = 20;
    // we find out how 'big' the space is
    QRectF bounding = canvasRect();
    float xSpan = bounding.width();

    // we need to find a nice "round" square size
    float xSquareSize = xSpan / 10;
    float logXSquare = log10(xSquareSize);
    int digits = logXSquare > 0 ? ceilf(logXSquare)-1 : -ceilf(-logXSquare);
    float xSize = pow(10, digits);

    int sizeDigits = -digits;
    // we want ~10 lines per dimension
    int lineCount = xSpan / xSize;
    while(lineCount > 20) {
        xSize *= 2.5; // 0.25 0.5 0.75
        lineCount = xSpan / xSize;
        sizeDigits = -digits+1;
        if(lineCount > 20) {
            xSize *= 2; // 0.5 1.0 1.5
            lineCount = xSpan / xSize;
            sizeDigits = -digits;
        }
    }
    sizeDigits = max(0, sizeDigits);

    float ySize = xSize;

    float xStart = int(bounding.x()/xSize)*xSize;
    float yStart = int(bounding.y()/ySize)*ySize;

    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(Qt::black, 0.5, Qt::DashLine));
    painter.setFont(QFont("Lucida Grande", 9));
    QFontMetrics fm = painter.fontMetrics();

    if(!data->bProjected && data->IsCategorical(xIndex)) {
        int cnt = data->categorical[xIndex].size();
        FOR(i, cnt) {
            float canvasX = toCanvasCoords(i,0).x();
            if(canvasX < 0 || canvasX > w) continue;
            painter.drawLine(canvasX, 0, canvasX, h);
            string name = data->GetCategorical(xIndex, i);
            painter.drawText(canvasX, h-5, QString(name.c_str()));
        }
    } else {
        for(float x=xStart; x<bounding.x() + bounding.width(); x+= xSize) {
            if(fabs(x) < 1e-6) x = 0;
            float canvasX = toCanvasCoords(x, 0).x();
            painter.drawLine(canvasX, 0, canvasX, h);
            QString text = QString("%1").arg(x, 0, 'f', sizeDigits);
            int textW = fm.horizontalAdvance(text);
            QRect rect(canvasX-textW/2,h-2-fm.height(),textW, fm.height());
            painter.drawText(rect, Qt::AlignCenter, text);
        }
    }

    if(!data->bProjected && data->IsCategorical(yIndex)) {
        int cnt = data->categorical[yIndex].size();
        FOR(i, cnt) {
            float canvasY = toCanvasCoords(0,i).y();
            if(canvasY < 0 || canvasY > w) continue;
            painter.drawLine(0, canvasY, w, canvasY);
            string name = data->GetCategorical(yIndex, i);
            painter.drawText(2, canvasY, QString(name.c_str()));
        }
    } else {
        for(float y=yStart; y<bounding.y() + bounding.height(); y+= ySize) {
            if(fabs(y) < 1e-6) y = 0;
            float canvasY = toCanvasCoords(0, y).y();
            painter.drawLine(0, canvasY, w, canvasY);
            QString text = QString("%1").arg(y, 0, 'f', sizeDigits);
            int textW = fm.horizontalAdvance(text);
            QRect rect(2,canvasY-fm.height()/2,textW, fm.height());
            painter.drawText(rect, Qt::AlignCenter, text);
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

void Canvas::DrawAxes1D(QPainter &painter)
{
    int w = width();
    int h = height();

    int pad = 20;
    int base = h/2+10;
    painter.setPen(QPen(Qt::black, 1));
    painter.drawLine(pad, base, w-2*pad,base);
    painter.drawLine(pad, base-5, pad, base+5);
    painter.drawLine(w-2*pad, base-5, w-2*pad, base+5);
    QString xlabel = QString("Dimension %1").arg(xIndex+1);
    if(xIndex < dimNames.size()) {
        xlabel = dimNames[xIndex];
    }
    painter.setPen(QPen(Qt::black, 0.5));
    painter.drawText(0, base + 30, w, 20, Qt::AlignTop | Qt::AlignHCenter, xlabel);
    return;
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
    if(data->GetCount() == 0)
    {
        int w = width();
        int h = height();
        maps.samples = QPixmap(w,h);
        maps.samples.fill(Qt::transparent);
        drawnSamples = 0;
        return;
    }
    if(!maps.samples.isNull() && drawnSamples == data->GetCount()) return;
    if(drawnSamples > data->GetCount()) drawnSamples = 0;

    if(drawnSamples==0 || maps.samples.isNull())
    {
        int w = width();
        int h = height();
        maps.samples = QPixmap(w,h);
        maps.samples.fill(Qt::transparent);
        drawnSamples = 0;
    }
    QPainter painter(&maps.samples);
    painter.setRenderHint(QPainter::Antialiasing, true);
    for(int i=drawnSamples; i<data->GetCount(); i++)
    {
        if(data->GetFlag(i) == _TRAJ) continue;
        int label = data->GetLabel(i);
        fvec sample = data->GetSample(i);
        QPointF point = toCanvasCoords(sample);
        if(xIndex == yIndex) point.setY(height()/2);
        Canvas::drawSample(painter, point, (data->GetFlag(i)==_TRAJ)?5:radius, bDisplaySingle ? 0 : label);
    }
    drawnSamples = data->GetCount();
}

void Canvas::DrawTargets(QPainter &painter)
{
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(Qt::black, 1.5));
    FOR(i, targets.size()) {
        QPointF point = toCanvasCoords(targets[i]);
        QPointF delta1 = QPointF(1,1);
        QPointF delta2 = QPointF(1,-1);
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

        QTransform scalingTransform;
        QPointF t = toCanvasCoords(data->GetObstacle(i).center);
        scalingTransform.scale(data->GetObstacle(i).repulsion[0], data->GetObstacle(i).repulsion[1]);
        obstaclePath = scalingTransform.map(obstaclePath);
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
            painter.setPen(QPen(Qt::black, 0.5));
            QPointF point = toCanvasCoords(pt);
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
    fvec oldPt = liveTrajectory[0];
    int count = liveTrajectory.size();
    FOR(j, count-1)
    {
        fvec pt = liveTrajectory[j+1];
        if(!pt.size()) break;
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
