#include "qcontour.h"
#include <float.h>
#include <QDebug>

QContour::QContour(float *values, int w, int h)
    : valueMap(ValueMap(values,w,h))
{
    vmin = FLT_MAX;
    vmax = -FLT_MAX;
    if(values)
    {
        for(int i=0; i<w; i++)
        {
            for(int j=0; j<h; j++)
            {
                float v = values[j*w+i];
                if(vmin > v) vmin = v;
                if(vmax < v) vmax = v;
            }
        }
    }
    if(vmax==vmin)
    {
        vmax += 0.1f;
        vmin -= 0.1f;
    }
}

float QContour::meanValue(int xStart, int xEnd, int yStart, int yEnd)
{
    int w = valueMap.w, h = valueMap.h;

    if(xStart>=w) xStart = w-1;
    if(yStart>=h) yStart = h-1;
    if(xEnd>w) xEnd = w;
    if(yEnd>h) yEnd = h;
    if(xStart<0) xStart = 0;
    if(yStart<0) yStart = 0;
    if(xEnd==xStart) xEnd = xStart+1;
    if(yEnd==yStart) yEnd = yStart+1;

    double accumulator = 0;
    int cnt=0;
    for(int i=xStart; i<xEnd; i++)
    {
        for(int j=yStart; j<yEnd; j++)
        {
            accumulator += valueMap.value(i,j);
            cnt++;
        }
    }
    accumulator /= cnt;
    return accumulator;
}

float QContour::meanValue(QRect rect)
{
    return meanValue(rect.x(), rect.x()+rect.width(), rect.y(), rect.y()+rect.height());
}

void QContour::Paint(QPainter &painter, int levels)
{
    CContourMap map;
    map.generate_levels(vmin,vmax,levels);
    map.contour(&valueMap);
    //map.dump();
    map.consolidate(); // connect all the lines
    //map.dump();
    int w = valueMap.w, h = valueMap.h;
    int hmo = h-2;// we take out the last line as it will mess up the computations
    int wmo = w-2;// we take out the last line as it will mess up the computations
    int W = painter.viewport().width();
    int H = painter.viewport().height();

    QList<QPainterPath> paths;
    QList<float> altitudes;
    for(int i=0; i<levels; i++)
    {
        CContourLevel *level = map.level(i);
        if(!level || !level->contour_lines) continue;
        QPainterPath path;
        for(int j=0; j<level->contour_lines->size(); j++)
        {
            CContour *line = level->contour_lines->at(j);
            vector<SPoint> points = line->contourPoints();
            QPointF oldPoint;
            for(int p=0; p<points.size(); p++)
            {
                QPointF point(points[p].x*W/wmo, points[p].y*H/hmo);
                if(p)
                {
                    path.lineTo(point);
//                    painter.drawLine(point, oldPoint);
                }
                else path.moveTo(point);
                oldPoint = point;
            }
            paths.push_back(path);
        }
        altitudes.push_back(map.alt(i));
    }

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(Qt::green, 2));
    painter.setBrush(Qt::NoBrush);
    for(int i=0; i<paths.size(); i++)
    {
        painter.setPen(QPen(Qt::green, 0.25 + i/(float)paths.size()*3));
        painter.drawPath(paths.at(i));
    }

    // we draw the colorbar
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setOpacity(0.8);
    int barW = 20;
    QRect rect(W - barW - 16, 16, barW, H-32);
    painter.setBrush(Qt::NoBrush);
    for(int i=0; i<rect.height(); i++)
    {
        float v = (1.f - i/(float)rect.height())*255.f;
        painter.setPen(QColor(v,v,v));
        painter.drawLine(rect.x(), rect.y() + i, rect.x() + rect.width(), rect.y() + i);
    }

    // we draw the altitude lines
    for(int i=0; i<altitudes.size(); i++)
    {
        float v = altitudes[i];
        v = (v-vmin)/(vmax-vmin);
        int y = (int)(v*rect.height());
        painter.setPen(QPen(Qt::green, 3.25 - i/(float)altitudes.size()*3));
        painter.drawLine(rect.x(), y, rect.x()+rect.width(), y);
    }

    // we draw the values on the colorbar
    QFont font = painter.font();
    font.setPointSize(9);
    painter.setFont(font);
    painter.setOpacity(1);
    int steps = 10;
    for(int i=0; i<steps+1; i++)
    {
        float v = (1.f - i/(float)steps);
        QString text = QString("%1").arg(v*(vmax-vmin) + vmin, 0, 'f', 2);
        int y = rect.y() + i*rect.height()/steps;
        QRect textRect = QRect(rect.x()-40, y - 10, 40-6, 20);
        // to decide if we write stuff in white or black
        QRect testRect = QRect(textRect.x()*w/W, textRect.y()*h/H, textRect.width()*w/W, textRect.height()*h/H);
        float mean = meanValue(testRect);
        mean = (mean-vmin)/(vmax-vmin);
        painter.setPen((mean>0.5)? Qt::black : Qt::white);
        painter.drawText(textRect, Qt::AlignRight + Qt::AlignVCenter, text);
        painter.drawLine(rect.x(), y, rect.x()-4, y);
    }

    // we decide if we write stuff in white or black
    float mean = meanValue(QRect((rect.x()-40)*w/W, rect.y()*h/H, (40-6)*w/W, rect.height()*h/H));
    painter.setPen(QPen(mean>0.5?Qt::black:Qt::white, 1));
    painter.drawRect(rect);

}
