#ifndef QCONTOUR_H
#define QCONTOUR_H

#include "contours.h"
#include <QPainter>

// implementation of the CRaster class for contour lines creation
class ValueMap:public CRaster
{
    float *values;
public:
    int w, h;
    ValueMap(float *values=0, int w=0, int h=0):values(values), w(w), h(h){}
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
    float vmin, vmax;
    float meanValue(int xStart, int xEnd, int yStart, int yEnd);
    float meanValue(QRect rect);
public:
    QContour(float *values, int w, int h);
    void Paint(QPainter &painter, int levels);
};

#endif // QCONTOUR_H
