#include "qcontour.h"
#include <float.h>
#include <math.h>
#include <QLabel>
#include <QDebug>

QContour::QContour(double *values, int w, int h)
    : valueMap(ValueMap(values,w,h)), plotColor(Qt::green), plotThickness(2), style(Qt::SolidLine)
{
    vmin = DBL_MAX;
    vmax = -DBL_MAX;
    if(values)
    {
        for(int i=0; i<w; i++)
        {
            for(int j=0; j<h; j++)
            {
                double v = values[j*w+i];
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

    if(vmax-vmin < 1e-10)
    {
        double vdiff = 1e-10;
        double vcenter = (vmax-vmin)*0.5f;
        vmax = vcenter + vdiff*0.5f;
        vmin = vcenter - vdiff*0.5f;
    }
}

double QContour::meanValue(int xStart, int xEnd, int yStart, int yEnd)
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

double QContour::meanValue(QRect rect)
{
    return meanValue(rect.x(), rect.x()+rect.width(), rect.y(), rect.y()+rect.height());
}

void QContour::ShowValueImage()
{
    int w = valueMap.w, h = valueMap.h;
    QImage image(w,h,QImage::Format_RGB32);
    double vdiff = vmax - vmin;
    for(int i=0; i<w; i++)
    {
        for(int j=0; j<h; j++)
        {
            int value = (int)((valueMap.value(i, j)-vmin)/vdiff*255);
            value = max(0,min(255,value));
            image.setPixel(i,j, qRgb((int)value,value,value));
        }
    }
    QPixmap contourPixmap = QPixmap::fromImage(image).scaled(512,512, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    QLabel *lab = new QLabel();
    lab->setPixmap(contourPixmap);
    lab->show();
}

void QContour::Paint(QPainter &painter, int levels, int zoom)
{
    if(vmin == vmax) return;
    CContourMap map;
    levels = (int)(levels*sqrt(zoom));
    map.generate_levels(vmin,vmax,levels);
    map.contour(&valueMap);
    //map.dump();
    map.consolidate(); // connect all the lines
    //map.dump();
    int w = valueMap.w, h = valueMap.h;
    int hmo = h-1;// we take out the last line as it will mess up the computations
    int wmo = w-1;// we take out the last line as it will mess up the computations
    int W = painter.viewport().width();
    int H = painter.viewport().height();

    QList<QPainterPath> paths;
    QList<double> altitudes;
    for(int i=0; i<levels; i++)
    {
        CContourLevel *level = map.level(i);
        if(!level || !level->contour_lines) continue;
        for(int j=0; j<level->contour_lines->size(); j++)
        {
            CContour *line = level->contour_lines->at(j);
            vector<SPoint> points = line->contourPoints();
            QPointF oldPoint;
            int emptyCount = 0;
            QPainterPath path;
            for(int p=0; p<points.size(); p++)
            {
                QPointF point(points[p].x*W/(w-2), points[p].y*H/(h-2));
                if(p)
                {
                    path.lineTo(point);
//                    painter.drawLine(point, oldPoint);
                    QPointF diff = point - oldPoint;
                    if(diff.x() == 0 || diff.y() == 0) emptyCount++;
                }
                else path.moveTo(point);
                if(emptyCount > points.size()*0.2f) break;
                oldPoint = point;                
            }
            if(emptyCount / (float) points.size() > 0.2) continue; // we dont want stuff that has loads of empty segments
            paths.push_back(path);
        }
        altitudes.push_back(map.alt(i));
    }

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(Qt::NoBrush);
    for(int i=0; i<min(paths.size(),300); i++)
    {
        painter.setPen(QPen(plotColor, 0.25 + i/(double)paths.size()*plotThickness, style));
        painter.drawPath(paths.at(i));
        //qDebug() << "path length:" << paths.at(i).length();
    }

    if(bDrawColorbar)
    {
        // we draw the colorbar
        painter.setRenderHint(QPainter::Antialiasing, false);
        painter.setOpacity(0.8);
        int barW = 20;
        QRect rect(W - barW - 16, 16, barW, H-32);
        painter.setBrush(Qt::NoBrush);
        for(int i=0; i<rect.height(); i++)
        {
            double v = (1. - i/(double)rect.height())*255.;
            v = max(0.,min(255.,v));
            painter.setPen(QColor(v,v,v));
            painter.drawLine(rect.x(), rect.y() + i, rect.x() + rect.width(), rect.y() + i);
        }

        // we draw the altitude lines
        for(int i=0; i<altitudes.size(); i++)
        {
            double v = altitudes[i];
            v = (v-vmin)/(vmax-vmin);
            int y = (int)(v*rect.height()) + 16;
            painter.setPen(QPen(Qt::green, 3.25 - i/(double)altitudes.size()*3));
            painter.drawLine(rect.x(), y, rect.x()+rect.width(), y);
        }

        const double multiplier = 1000.; // this is to avoid numerical instabilities

        // we draw the values on the colorbar
        QFontMetrics fm = painter.fontMetrics();
        QFont font = painter.font();
        font.setPointSize(9);
        painter.setFont(font);
        painter.setOpacity(1);
        int steps = 10;
        bool bShortFormat = true;
        if(fabs(vmax/multiplier) < 1e-6 && fabs(vmin/multiplier) < 1e-6) bShortFormat = false;
        for(int i=0; i<steps+1; i++)
        {
            double v = (1.f - i/(double)steps);
            double value = (v*(vmax-vmin) + vmin)/multiplier;
            QString text;
            if(bShortFormat) text = QString("%1").arg(value, 0, 'g', 4);
            else text = QString("%1").arg(value, 0, 'f', 4);
            int y = rect.y() + i*rect.height()/steps;
            QRect boundingRect = fm.boundingRect(text);
            QRect textRect = QRect(rect.x()-boundingRect.width()-6, y - boundingRect.height()/2, boundingRect.width(), boundingRect.height());
            // to decide if we write stuff in white or black
            QRect testRect = QRect(textRect.x()*w/W, textRect.y()*h/H, textRect.width()*w/W, textRect.height()*h/H);
            double mean = meanValue(testRect);
            mean = (mean-vmin)/(vmax-vmin);
            painter.setPen((mean>0.5)? Qt::black : Qt::white);
            painter.drawText(textRect, Qt::AlignRight + Qt::AlignVCenter, text);
            painter.drawLine(rect.x(), y, rect.x()-4, y);
        }

        // we decide if we write stuff in white or black
        double mean = meanValue(QRect((rect.x()-40)*w/W, rect.y()*h/H, (40-6)*w/W, rect.height()*h/H));
        painter.setPen(QPen(mean>0.5?Qt::black:Qt::white, 1));
        painter.drawRect(rect);
    }

}
