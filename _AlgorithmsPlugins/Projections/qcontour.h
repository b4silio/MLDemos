#ifndef QCONTOUR_H
#define QCONTOUR_H

#include "contours.h"
#include <QPainter>

// implementation of the CRaster class for contour lines creation
class ValueMap:public CRaster
{
    double *values;
public:
    int w, h;
    ValueMap(double *values=0, int w=0, int h=0):values(values), w(w), h(h){}
    double value(double x,double y){return values && w ? values[int(y)*w + int(x)] : 0;}
    SPoint upper_bound(){return SPoint(w-1,h-1);}
    SPoint lower_bound(){return SPoint(0,0);}
};

/**
    QContour class takes as input an array of data (pixel) values,
    and draws the corresponding contour lines onto a given painter device
*/
class QContour
{
    ValueMap valueMap;
    double vmin, vmax;
    double meanValue(int xStart, int xEnd, int yStart, int yEnd);
    double meanValue(QRect rect);
public:
    QContour(double *values, int w, int h);
    void Paint(QPainter &painter, int levels);
};

#endif // QCONTOUR_H
