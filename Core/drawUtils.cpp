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
#include "drawUtils.h"
#include "glUtils.h"
#include "basicMath.h"
#include <QBitmap>
#include <QDebug>
#include "qcontour.h"
#include "kmeans.h"
#include <jacgrid/jacgrid.h>
using namespace std;

#define FOUR(a) {a;a;a;a;}
#define RES 256
void DrawEllipse(float *mean, float *sigma, float rad, QPainter *painter, QSize size)
{
    if(mean[0] != mean[0] || mean[1] != mean[1]) return; // nan
    float a = sigma[0], b = sigma[1], c = sigma[2];
    float L[4];
    L[0] = a; L[1] = 0; L[2] = b; L[3] = sqrtf(c*a-b*b);
    if(L[3] != L[3]) L[3] = 0;
    FOR(i,4) L[i] /= sqrtf(a);

    const int segments = 64;
    float oldX = FLT_MAX, oldY = FLT_MAX;
    for (float theta=0; theta <= PIf*2.f; theta += (PIf*2.f)/segments)
    {
        float x = cosf(theta)*rad;
        float y = sinf(theta)*rad;
        float nx = L[0]*x;
        float ny = L[2]*x + L[3]*y;
        nx += mean[0];
        ny += mean[1];
        if(oldX != FLT_MAX)
        {
            painter->drawLine(
                        QPointF(nx*size.width(),ny*size.height()),
                        QPointF(oldX*size.width(),oldY*size.height())
                        );
        }
        oldX = nx;
        oldY = ny;
    }
}

void DrawEllipse(float *mean, float *sigma, float rad, QPainter *painter, Canvas *canvas)
{
    if(mean[0] != mean[0] || mean[1] != mean[1]) return; // nan
    float a = sigma[0], b = sigma[1], c = sigma[2];
    float L[4];
    L[0] = a; L[1] = 0; L[2] = b; L[3] = sqrtf(c*a-b*b);
    if(L[3] != L[3]) L[3] = 0;
    FOR(i,4) L[i] /= sqrtf(a);

    const int segments = 64;
    float oldX = FLT_MAX, oldY = FLT_MAX;
    for (float theta=0; theta <= PIf*2.f; theta += (PIf*2.f)/segments)
    {
        float x = cosf(theta)*rad;
        float y = sinf(theta)*rad;
        float nx = L[0]*x;
        float ny = L[2]*x + L[3]*y;
        nx += mean[0];
        ny += mean[1];
        if(oldX != FLT_MAX)
        {
            painter->drawLine(canvas->toCanvasCoords(nx,ny), canvas->toCanvasCoords(oldX, oldY));
        }
        oldX = nx;
        oldY = ny;
    }
}

void DrawArrow( const QPointF &ppt, const QPointF &pt, double sze, QPainter &painter)
{
    QPointF pd, pa, pb;
    double tangent;

    pd = ppt - pt;
    if (pd.x() == 0 && pd.y() == 0)
        return;
    tangent = atan2 ((double) pd.y(), (double) pd.x());
    pa.setX(sze * cos (tangent + PIf / 7.f) + pt.x());
    pa.setY(sze * sin (tangent + PIf / 7.f) + pt.y());
    pb.setX(sze * cos (tangent - PIf / 7.f) + pt.x());
    pb.setY(sze * sin (tangent - PIf / 7.f) + pt.y());
    //-- connect the dots...
    painter.drawLine(pt, ppt);
    painter.drawLine(pt, pa);
    painter.drawLine(pt, pb);
}

QColor ColorFromVector(fvec a)
{
    // angle is between 0 and 1;
    float angle = atan2(a[0], a[1]) / (2*PIf) + 0.5f;
    vector<fvec> colors;
#define Col2Col(r,g,b) {fvec c;c.resize(3); c[0] = r;c[1] = g;c[2] = b; colors.push_back(c);}

    Col2Col(0,0,255);
    Col2Col(255,0,255);
    Col2Col(255,0,0);
    Col2Col(255,255,0);
    Col2Col(0,255,0);
    Col2Col(0,255,255);

    // find where the angle fits in the color list
    int index = (int)(angle*(colors.size())) % colors.size();
    fvec c1 = colors[index];
    fvec c2 = colors[(index+1)%colors.size()];

    // compute the ratio between c1 and c2
    float remainder = angle*(colors.size()) - (int)(angle*(colors.size()));
    fvec c3 = c1*(1-remainder) + c2*remainder;
    return QColor(c3[0],c3[1],c3[2]);
}

QPixmap RocImage(std::vector< std::vector<f32pair> > rocdata, std::vector<const char *> roclabels, QSize size)
{
    QPixmap pixmap(size);
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    int w = pixmap.width(), h = pixmap.height();
    int PAD = 16;

    QFont font = painter.font();
    font.setPointSize(12);
    font.setBold(true);
    painter.setFont(font);

    bool bMultiClass = false;
    FOR(d, rocdata.size())
    {
        FOR(i, rocdata[d].size())
        {
            if(rocdata[d][i].second > 1)
            {
                bMultiClass = true;
                break;
            }
        }
        if(bMultiClass) break;
    }

    FOR(d, rocdata.size())
    {
        int minCol = 128;
        int color = (rocdata.size() == 1) ? 255 : (255 - minCol)*(rocdata.size() - d -1)/(rocdata.size()-1) + minCol;
        color = 255 - color;

        std::vector<fvec> allData;
        std::vector<f32pair> data = rocdata[d];
        if(!data.size()) continue;

        if(bMultiClass)
        {
            std::map<int,int> countPerClass;
            std::map<int,int> truePerClass;
            std::map<int,int> falsePerClass;
            FOR(i, data.size())
            {
                countPerClass[data[i].second]++;
                if(data[i].first != data[i].second) falsePerClass[data[i].first]++;
                else truePerClass[data[i].first]++;
            }

            float macroFMeasure = 0.f, microFMeasure = 0.f;
            int microTP = 0, microFP = 0, microTN = 0, microCount = 0;
            float microPrecision = 0.f;
            float microRecall = 0.f;
            FORIT(countPerClass, int, int)
            {
                int c = it->first;
                int tp = truePerClass[c];
                int fp = falsePerClass[c];
                int fn = countPerClass[c] - tp;
                int tn = countPerClass[c] - fp;
                int count = it->second;
                microTP += tp;
                microFP += fp;
                microTN += tn;
                microCount += count;
                float precision = tp / float(tp + fp);
                float recall = tp / float(count);
                macroFMeasure += 2*precision*recall/(precision+recall);
            }
            macroFMeasure /= countPerClass.size();
            microPrecision = microTP / float(microTP + microFP);
            microRecall = microTP / float(microCount);
            microFMeasure = 2*microPrecision*microRecall/(microPrecision + microRecall);

            fVec val = fVec(microFP/float(microFP+microTN), 1 - microRecall);
            float fmeasure = microFMeasure;

            fvec dat;
            dat.push_back(val.x);
            dat.push_back(val.y);
            dat.push_back(0.5);
            dat.push_back(fmeasure);
            allData.push_back(dat);
        }
        else
        {
            std::sort(data.begin(), data.end());
            FOR(i, data.size())
            {
                fVec val;
                float fmeasure = 0;
                float thresh = data[i].first;
                u32 tp = 0, fp = 0;
                u32 fn = 0, tn = 0;

                FOR(j, data.size())
                {
                    if(data[j].second == 1)
                    {
                        if(data[j].first >= thresh) tp++;
                        else fn++;
                    }
                    else
                    {
                        if(data[j].first >= thresh) fp++;
                        else tn++;
                    }
                }
                if((fp+tn)>0 && (tp+fn)>0 && (tp+fp)>0)
                {
                    val=fVec(fp/float(fp+tn), 1 - tp/float(tp+fn));
                    float precision = tp / float(tp+fp);
                    float recall = tp /float(tp+fn);
                    fmeasure = tp == 0 ? 0 : 2 * (precision * recall) / (precision + recall);
                }
                fvec dat;
                dat.push_back(val.x);
                dat.push_back(val.y);
                dat.push_back(data[i].first);
                dat.push_back(fmeasure);
                allData.push_back(dat);
            }
        }

        painter.setPen(QPen(QColor(color,color,color), 1.f));

        if(allData.size() > 1)
        {
            fVec pt1, pt2;
            FOR(i, allData.size()-1)
            {
                pt1 = fVec(allData[i][0]*size.width(), allData[i][1]*size.height());
                pt2 = fVec(allData[i+1][0]*size.width(), allData[i+1][1]*size.height());
                painter.drawLine(QPointF(pt1.x+PAD, pt1.y+PAD),QPointF(pt2.x+PAD, pt2.y+PAD));
            }
            pt1 = fVec(0,size.width());
            painter.drawLine(QPointF(pt1.x+PAD, pt1.y+PAD),QPointF(pt2.x+PAD, pt2.y+PAD));
        }
        else if(allData.size())
        {
            fVec point;
            point = fVec(allData[0][0]*size.width(), allData[0][1]*size.height());
            painter.drawLine(QPointF(PAD, size.height()+PAD), QPointF(point.x+PAD, point.y+PAD));
            painter.drawLine(QPointF(point.x+PAD, point.y+PAD), QPointF(size.width()+PAD, PAD));
        }

        if(d < roclabels.size())
        {
            QPointF pos(3*size.width()/4,size.height() - (d+1)*16);
            painter.drawText(pos,QString(roclabels[d]));
        }
    }

    font = painter.font();
    font.setPointSize(10);
    font.setBold(false);
    font.setCapitalization(QFont::SmallCaps);
    painter.setFont(font);
    painter.setPen(Qt::black);
    painter.drawText(0, 0, size.width(), 16, Qt::AlignCenter, "False Positives");
    painter.translate(0, size.height());
    painter.rotate(-90);
    painter.drawText(0,0, size.height(), 16, Qt::AlignCenter, "True Positives");

    return pixmap;
}

QPixmap BoxPlot(std::vector<fvec> allData, QSize size, float maxVal, float minVal)
{
    QPixmap boxplot(size);
    if(!allData.size()) return boxplot;
    //QBitmap bitmap;
    //bitmap.clear();
    //boxplot.setMask(bitmap);
    boxplot.fill(Qt::transparent);
    QPainter painter(&boxplot);

    //	painter.setRenderHint(QPainter::Antialiasing);

    FOR(d,allData.size())
    {
        fvec data = allData[d];
        if(!data.size()) continue;
        FOR(i, data.size()) maxVal = max(maxVal, data[i]);
        FOR(i, data.size()) minVal = min(minVal, data[i]);
    }
    if(minVal == maxVal)
    {
        minVal = minVal/2;
        minVal = minVal*3/2;
    }

    FOR(d,allData.size())
    {
        int minCol = 70;
        int color = (allData.size() == 1) ? minCol : (255-minCol) * d / allData.size() + minCol;

        fvec data = allData[d];
        if(!data.size()) continue;
        int hpad = 15 + (d*size.width()/(allData.size()));
        int pad = -16;
        int res = size.height()+2*pad;
        int nanCount = 0;
        FOR(i, data.size()) if(data[i] != data[i]) nanCount++;

        float mean = 0;
        float sigma = 0;
        FOR(i, data.size()) if(data[i]==data[i]) mean += data[i] / (data.size()-nanCount);
        FOR(i, data.size()) if(data[i]==data[i]) sigma += powf(data[i]-mean,2);
        sigma = sqrtf(sigma/(data.size()-nanCount));

        float edge = minVal;
        float delta = maxVal - minVal;

        float top, bottom, median, quartLow, quartHi;
        vector<float> outliers;
        vector<float> sorted;

        if(data.size() > 1)
        {
            if(sigma==0)
            {
                sorted = data;
            }
            else
            {
                // we look for outliers using the 3*sigma rule
                FOR(i, data.size())
                {
                    if(data[i]!=data[i]) continue;
                    if (data[i] - mean < 3*sigma)
                        sorted.push_back(data[i]);
                    else outliers.push_back(data[i]);
                }
            }
            if(!sorted.size()) return boxplot;
            sort(sorted.begin(), sorted.end());
            int count = sorted.size();
            int half = count/2;
            bottom = sorted[0];
            top = sorted[sorted.size()-1];

            median = count%2 ? sorted[half] : (sorted[half] + sorted[half - 1])/2;
            if(count < 4)
            {
                quartLow = bottom;
                quartHi = top;
            }
            else
            {
                quartLow = half%2 ? sorted[half/2] : (sorted[half/2] + sorted[half/2 - 1])/2;
                quartHi = half%2 ? sorted[half*3/2] : (sorted[half*3/2] + sorted[half*3/2 - 1])/2;
            }
        }
        else
        {
            top = bottom = median = quartLow = quartHi = data[0];
        }

        QPointF bottomPoint = QPointF(0, size.height() - (int)((bottom-edge)/delta*res) + pad);
        QPointF topPoint = QPointF(0, size.height() - (int)((top-edge)/delta*res) + pad);
        QPointF medPoint = QPointF(0, size.height() - (int)((median-edge)/delta*res) + pad);
        QPointF lowPoint = QPointF(0, size.height() - (int)((quartLow-edge)/delta*res) + pad);
        QPointF highPoint = QPointF(0, size.height() - (int)((quartHi-edge)/delta*res) + pad);

        painter.setPen(Qt::black);
        painter.drawLine(QPointF(hpad+35, bottomPoint.y()),	QPointF(hpad+65, bottomPoint.y()));

        painter.setPen(Qt::black);
        painter.drawLine(QPointF(hpad+35, topPoint.y()), QPointF(hpad+65, topPoint.y()));

        painter.setPen(Qt::black);
        painter.drawLine(QPointF(hpad+50, bottomPoint.y()),	QPointF(hpad+50, topPoint.y()));

        painter.setBrush(QColor(color,color,color));
        painter.drawRect(hpad+30, lowPoint.y(), 40, highPoint.y() - lowPoint.y());

        painter.setPen(Qt::black);
        painter.drawLine(QPointF(hpad+30, medPoint.y()),	QPointF(hpad+70, medPoint.y()));

        const char *longFormat = "%.3f";
        const char *shortFormat = "%.0f";
        const char *format = (maxVal - minVal) > 100 ? shortFormat : longFormat;
        painter.setPen(Qt::black);
        char text[255];
        sprintf(text, format, median);
        painter.drawText(QPointF(hpad-8,medPoint.y()+6), QString(text));
        sprintf(text, format, top);
        painter.drawText(QPointF(hpad+36,topPoint.y()-6), QString(text));
        sprintf(text, format, bottom);
        painter.drawText(QPointF(hpad+36,bottomPoint.y()+12), QString(text));
    }
    return boxplot;
}

QPixmap Histogram(std::vector<fvec> allData, QSize size, float maxVal, float minVal)
{
    QPixmap histogram(size);
    if(!allData.size()) return histogram;
    //QBitmap bitmap;
    //bitmap.clear();
    //histogram.setMask(bitmap);
    histogram.fill(Qt::transparent);
    QPainter painter(&histogram);

    //	painter.setRenderHint(QPainter::Antialiasing);

    FOR(d,allData.size())
    {
        fvec data = allData[d];
        if(!data.size()) continue;
        FOR(i, data.size()) if(data[i]==data[i]) maxVal = max(maxVal, data[i]);
        FOR(i, data.size()) if(data[i]==data[i]) minVal = min(minVal, data[i]);
    }
    if(minVal == maxVal)
    {
        minVal = minVal/2;
        minVal = minVal*3/2;
    }

    FOR(d,allData.size())
    {
        int minCol = 70;
        int color = (allData.size() == 1) ? minCol : (255-minCol) * d / allData.size() + minCol;

        fvec data = allData[d];
        if(!data.size()) continue;
        int hpad = 15 + (d*size.width()/(allData.size()));
        int pad = -16;
        int res = size.height()+2*pad;

        int nanCount = 0;
        FOR(i, data.size()) if(data[i] != data[i]) nanCount++;
        float mean = 0;
        float sigma = 0;
        FOR(i, data.size()) if(data[i]==data[i]) mean += data[i] / (data.size()-nanCount);
        FOR(i, data.size()) if(data[i]==data[i]) sigma += powf(data[i]-mean,2);
        sigma = sqrtf(sigma/(data.size()-nanCount));

        float edge = minVal;
        float delta = maxVal - minVal;
        float bottom = 0;

        QPointF bottomPoint = QPointF(0, size.height() - (int)((bottom-edge)/delta*res) + pad);
        QPointF topPoint = QPointF(0, size.height() - (int)((mean-edge)/delta*res) + pad);
        QPointF plusPoint = QPointF(0, size.height() - (int)((mean+sigma-edge)/delta*res) + pad);
        QPointF minusPoint = QPointF(0, size.height() - (int)((mean-sigma-edge)/delta*res) + pad);

        painter.setPen(Qt::black);
        painter.drawLine(QPointF(hpad+35, bottomPoint.y()),	QPointF(hpad+65, bottomPoint.y()));

        painter.setPen(Qt::black);
        painter.drawLine(QPointF(hpad+35, topPoint.y()), QPointF(hpad+65, topPoint.y()));

        painter.setBrush(QColor(color,color,color));
        painter.drawRect(hpad+30, topPoint.y(), 40, bottomPoint.y()-topPoint.y());

        painter.setPen(Qt::black);
        painter.drawLine(QPointF(hpad+50, plusPoint.y()), QPointF(hpad+50, minusPoint.y()));

        painter.setPen(Qt::black);
        painter.drawLine(QPointF(hpad+40, plusPoint.y()),	QPointF(hpad+60, plusPoint.y()));

        painter.setPen(Qt::black);
        painter.drawLine(QPointF(hpad+40, minusPoint.y()),	QPointF(hpad+60, minusPoint.y()));

        const char *longFormat = "%.3f";
        const char *shortFormat = "%.0f";
        const char *format = (maxVal - minVal) > 10 ? shortFormat : longFormat;
        painter.setPen(Qt::black);
        char text[255];
        sprintf(text, format, mean);
        painter.drawText(QPointF(hpad-8,topPoint.y()+6), QString(text));
        sprintf(text, format, mean+sigma);
        painter.drawText(QPointF(hpad+36,plusPoint.y()-6), QString(text));
        sprintf(text, format, mean-sigma);
        painter.drawText(QPointF(hpad+36,minusPoint.y()+12), QString(text));
    }
    return histogram;
}

QPixmap RawData(std::vector<fvec> allData, QSize size, float maxVal, float minVal)
{
    QPixmap rawData(size);
    if(!allData.size()) return rawData;
    //QBitmap bitmap;
    //bitmap.clear();
    //rawData.setMask(bitmap);
    rawData.fill(Qt::transparent);
    QPainter painter(&rawData);

    painter.setRenderHint(QPainter::Antialiasing);

    FOR(d,allData.size())
    {
        fvec data = allData[d];
        if(!data.size()) continue;
        FOR(i, data.size()) if(data[i]==data[i]) maxVal = max(maxVal, data[i]);
        FOR(i, data.size()) if(data[i]==data[i]) minVal = min(minVal, data[i]);
    }
    if(minVal == maxVal)
    {
        minVal = minVal/2;
        minVal = minVal*3/2;
    }

    FOR(d,allData.size())
    {
        int minCol = 70;
        int color = (allData.size() == 1) ? minCol : (255-minCol) * d / allData.size() + minCol;

        fvec data = allData[d];
        if(!data.size()) continue;
        int hpad = 15 + (d*size.width()/(allData.size()));
        int hsize = (size.width()/allData.size() - 15);
        int pad = -16;
        int res = size.height()+2*pad;
        int nanCount = 0;
        FOR(i, data.size()) if(data[i] != data[i]) nanCount++;

        float mean = 0;
        float sigma = 0;
        FOR(i, data.size()) if(data[i]==data[i]) mean += data[i] / (data.size()-nanCount);
        FOR(i, data.size()) if(data[i]==data[i]) sigma += powf(data[i]-mean,2);
        sigma = sqrtf(sigma/(data.size()-nanCount));

        float edge = minVal;
        float delta = maxVal - minVal;

        QPointF topPoint = QPointF(0, size.height() - (int)((mean-edge)/delta*res) + pad);
        QPointF plusPoint = QPointF(0, size.height() - (int)((mean+sigma-edge)/delta*res) + pad);
        QPointF minusPoint = QPointF(0, size.height() - (int)((mean-sigma-edge)/delta*res) + pad);

        FOR(i, data.size())
        {
            QPointF point = QPointF(hpad + (drand48() - 0.5)*hsize/2 + hsize/2, size.height() - (int)((data[i]-edge)/delta*res) + pad);
            painter.setPen(QPen(Qt::black, 0.5));
            painter.setBrush(QColor(color,color,color));
            painter.drawEllipse(point, 5, 5);
        }
        const char *longFormat = "%.3f";
        const char *shortFormat = "%.0f";
        const char *format = (maxVal - minVal) > 10 ? shortFormat : longFormat;
        painter.setPen(Qt::black);
        char text[255];
        sprintf(text, format, mean);
        painter.drawText(QPointF(hpad-8,topPoint.y()+6), QString(text));
        sprintf(text, format, mean+sigma);
        painter.drawText(QPointF(hpad-8,plusPoint.y()-6), QString(text));
        sprintf(text, format, mean-sigma);
        painter.drawText(QPointF(hpad-8,minusPoint.y()+12), QString(text));
    }
    return rawData;
}

void Draw3DRegressor(GLWidget *glw, Regressor *regressor)
{
    // we get the boundaries of our axes
    vector<fvec> samples = glw->canvas->data->GetSamples();
    int dim = glw->canvas->data->GetDimCount();
    fvec mins(dim, FLT_MAX), maxes(dim, -FLT_MAX);
    FOR(i, samples.size())
    {
        FOR(d, dim)
        {
            mins[d] = min(mins[d], samples[i][d]);
            maxes[d] = max(maxes[d], samples[i][d]);
        }
    }
    fvec center = (maxes + mins)*0.5f;
    fvec dists = (maxes - mins)*0.5f;
    float maxDist = dists[0];
    FOR(d, dim) maxDist = max(dists[d], maxDist);
    dists = fvec(dim, maxDist);
    // we double them just to be safe
    mins = center - dists;
    maxes = center + dists;

    // and now we draw a nice grid
    int xInd = 0, yInd = 1, zInd = regressor->outputDim;
    if(regressor->outputDim == yInd) yInd = 2;
    else if(regressor->outputDim == xInd) xInd = 2;
    int xSteps = 128, ySteps = 128;
    fvec point(dim,0);
    fvec gridPoints(xSteps*ySteps);
    qDebug() << "Generating regression surface";
    FOR(y, ySteps)
    {
        point[yInd] = y/(float)ySteps*(maxes[yInd]-mins[yInd]) + mins[yInd];
        FOR(x, xSteps)
        {
            point[xInd] = x/(float)xSteps*(maxes[xInd]-mins[xInd]) + mins[xInd];
            // we get the value of the regressor at the coordinates in the meshgrid
            fvec res = regressor->Test(point);
            gridPoints[x+y*xSteps] = res[0];
        }
    }
    qDebug() << "Creating GLObject structure";
    GLObject o = GenerateMeshGrid(gridPoints, xSteps, mins, maxes, xInd, yInd, zInd);
    qDebug() << "Done.";
    o.style = "smooth,transparent";
    o.style += QString(",isolines:%1").arg(zInd);
    o.style += ",blurry:3,color:1.0:1.0:1.0:0.4";
    glw->mutex->lock();
    glw->AddObject(o);
    glw->mutex->unlock();
}

void Draw3DClassifier(GLWidget *glw, Classifier *classifier)
{
    // we get the boundaries of our axes
    vector<fvec> samples = glw->canvas->data->GetSamples();
    int dim = glw->canvas->data->GetDimCount();
    fvec mins(dim, FLT_MAX), maxes(dim, -FLT_MAX);
    int xIndex = glw->canvas->xIndex;
    int yIndex = glw->canvas->yIndex;
    int zIndex = glw->canvas->zIndex;
    if(zIndex < 0 || zIndex > dim) return;
    FOR(i, samples.size())
    {
        FOR(d, dim)
        {
            mins[d] = min(mins[d], samples[i][d]);
            maxes[d] = max(maxes[d], samples[i][d]);
        }
    }
    fvec center = (maxes + mins)*0.5f;
    fvec dists = (maxes - mins)*0.5f;
    float maxDist = dists[0];
    FOR(d, dim) maxDist = max(dists[d], maxDist);
    dists = fvec(dim, maxDist);
    // we double them just to be safe
    mins = center - dists*2;
    maxes = center + dists*2;

    bool bMultiClass = false;
    // and now we draw a volume
    int steps = 64;
    fvec sample(dim);
    float *minVals = new float[steps];
    float *maxVals = new float[steps];
    double *values = new double[steps*steps*steps];
    printf("Generating volumetric data: ");
    FOR(y, steps)
    {
        //        values[y] = new double[steps*steps];
        minVals[y] = FLT_MAX;
        maxVals[y] = -FLT_MAX;
        sample[yIndex] = y/(float)(steps)*(maxes[yIndex]-mins[yIndex]) + mins[yIndex];
        FOR(z, steps)
        {
            sample[zIndex] = z/(float)steps*(maxes[zIndex]-mins[zIndex]) + mins[zIndex];
            FOR(x, steps)
            {
                sample[xIndex] = x/(float)steps*(maxes[xIndex]-mins[xIndex]) + mins[xIndex];
                if(classifier->IsMultiClass())
                {
                    fvec res = classifier->TestMulti(sample);
                    if(res.size() == 1)
                    {
                        values[x + (y + z*steps)*steps] = res[0];
                    }
                    else
                    {
                        // we mostly want to know if the sample is close to the boundary
                        float maxVal = res[0];
                        int maxInd = 0;
                        FOR(d, res.size())
                        {
                            if(maxVal < res[d])
                            {
                                maxInd = d;
                                maxVal = res[d];
                            }
                        }
                        // we keep the class with the highest score
                        values[x + (y + z*steps)*steps] = maxInd;
                        bMultiClass = true;
                    }
                }
                else
                {
                    float res = classifier->Test(sample);
                    values[x + (y + z*steps)*steps] = res;
                }
            }
        }
    }
    printf("done.\n");
    fflush(stdout);

    if(bMultiClass)
    {
        int classCount = DatasetManager::GetClassCount(glw->canvas->data->GetLabels());
        FOR(c,classCount-1)
        {
            gridT valueGrid(0.f, steps, steps, steps);
            /*
            FOR(i, steps*steps*steps)
            {
                if(values[i] == c) valueGrid[i] = 1.f;
                else
                {
                    valueGrid[i] = 2.f;
                    for(int c2=c+1; c2<classCount; c2++)
                        if(values[i] == c2) valueGrid[i] = -1.f;
                }
            }
            */
            FOR(i, steps*steps*steps) valueGrid[i] = values[i] == c ? 1.f : -1.f;
            FOR(d, 3) valueGrid.unit[d] = (maxes[d] - mins[d])/steps;   /* length of a single edge in each dimension*/
            FOR(d, 3) valueGrid.size[d] = maxes[d] - mins[d];           /* length of entire grid in each dimension */
            FOR(d, 3) valueGrid.org[d] = mins[d];                       /* the origin of the grid i.e. coords of (0,0,0) */
            FOR(d, 3) valueGrid.center[d] = (maxes[d] + mins[d])/2;     /* coords of center of grid */

            surfaceT surf;
            float surfThreshold = 0.f;
            unsigned int surfIType = JACSurfaceTypes::SURF_CONTOUR;
            printf("Generating isosurfaces: ");
            fflush(stdout);
            JACMakeSurface(surf, surfIType, valueGrid, surfThreshold);
            JACSmoothSurface(surf);
            JACSmoothSurface(surf);
            JACSmoothSurface(surf);
            //surf.Reduce(0.05);
            printf("done.\n");
            fflush(stdout);

            printf("Generating mesh: ");

            GLObject o;

            std::vector<float> &vertices = surf.vertices;
            //std::vector<float> &normals = surf.normals;
            for (int i=0; i<surf.nconn; i += 3)
            {
                int index = surf.triangles[i];
                o.vertices.append(QVector3D(vertices[index*3],vertices[index*3+1],vertices[index*3+2]));
                index = surf.triangles[i+1];
                o.vertices.append(QVector3D(vertices[index*3],vertices[index*3+1],vertices[index*3+2]));
                index = surf.triangles[i+2];
                o.vertices.append(QVector3D(vertices[index*3],vertices[index*3+1],vertices[index*3+2]));
            }

            QColor color = SampleColor[(c+1)%SampleColorCnt];
            o.objectType = "Surfaces";
            o.style = "smooth,transparent,blurry";
            o.style += QString("color:%1:%2:%3:0.4").arg(color.redF()).arg(color.greenF()).arg(color.blueF());
            o.style += QString(",offset:%1").arg(c*0.5f,0,'f',2);
            glw->mutex->lock();
            glw->AddObject(o);
            glw->mutex->unlock();
            printf("done.\n");
            fflush(stdout);
        }
    }
    else
    {
        gridT valueGrid(0.f, steps, steps, steps);
        FOR(i, steps*steps*steps) valueGrid[i] = values[i];
        FOR(d, 3) valueGrid.unit[d] = (maxes[d] - mins[d])/steps;   /* length of a single edge in each dimension*/
        FOR(d, 3) valueGrid.size[d] = maxes[d] - mins[d];           /* length of entire grid in each dimension */
        FOR(d, 3) valueGrid.org[d] = mins[d];                       /* the origin of the grid i.e. coords of (0,0,0) */
        FOR(d, 3) valueGrid.center[d] = (maxes[d] + mins[d])/2;     /* coords of center of grid */

        surfaceT surf;
        float surfThreshold = 0.f;
        unsigned int surfIType = JACSurfaceTypes::SURF_CONTOUR;
        printf("Generating isosurfaces: ");
        fflush(stdout);
        JACMakeSurface(surf, surfIType, valueGrid, surfThreshold);
        JACSmoothSurface(surf);
        JACSmoothSurface(surf);
        JACSmoothSurface(surf);
        //surf.Reduce(0.05);
        printf("done.\n");
        fflush(stdout);

        printf("Generating mesh: ");

        GLObject o;

        std::vector<float> &vertices = surf.vertices;
        //std::vector<float> &normals = surf.normals;
        for (int i=0; i<surf.nconn; i += 3)
        {
            int index = surf.triangles[i];
            o.vertices.append(QVector3D(vertices[index*3],vertices[index*3+1],vertices[index*3+2]));
            index = surf.triangles[i+1];
            o.vertices.append(QVector3D(vertices[index*3],vertices[index*3+1],vertices[index*3+2]));
            index = surf.triangles[i+2];
            o.vertices.append(QVector3D(vertices[index*3],vertices[index*3+1],vertices[index*3+2]));
        }

        o.objectType = "Surfaces";
        o.style = "smooth,transparent,blurry:1,color:0:0:0:0.3";
        glw->mutex->lock();
        glw->AddObject(o);
        glw->mutex->unlock();
        printf("done.\n");
        fflush(stdout);
    }
    delete [] minVals;
    delete [] maxVals;
    delete [] values;
}

void Draw3DClusterer(GLWidget *glw, Clusterer *clusterer)
{
    // we get the boundaries of our axes
    vector<fvec> samples = glw->canvas->data->GetSamples();
    int dim = glw->canvas->data->GetDimCount();
    fvec mins(dim, FLT_MAX), maxes(dim, -FLT_MAX);
    int xIndex = glw->canvas->xIndex;
    int yIndex = glw->canvas->yIndex;
    int zIndex = glw->canvas->zIndex;
    if(zIndex < 0 || zIndex > dim) return;
    FOR(i, samples.size())
    {
        FOR(d, dim)
        {
            mins[d] = min(mins[d], samples[i][d]);
            maxes[d] = max(maxes[d], samples[i][d]);
        }
    }
    fvec center = (maxes + mins)*0.5f;
    fvec dists = (maxes - mins)*0.5f;
    float maxDist = dists[0];
    FOR(d, dim) maxDist = max(dists[d], maxDist);
    dists = fvec(dim, maxDist);
    // we double them just to be safe
    mins = center - dists*2;
    maxes = center + dists*2;

    // and now we draw a volume
    int steps = 64;
    fvec sample(dim);
    float *minVals = new float[steps];
    float *maxVals = new float[steps];
    double *values = new double[steps*steps*steps];
    printf("Generating volumetric data: ");
    bool bOneClass = true;
    int clusterCount = clusterer->NbClusters();
    FOR(y, steps)
    {
        //        values[y] = new double[steps*steps];
        minVals[y] = FLT_MAX;
        maxVals[y] = -FLT_MAX;
        sample[yIndex] = y/(float)(steps)*(maxes[yIndex]-mins[yIndex]) + mins[yIndex];
        FOR(z, steps)
        {
            sample[zIndex] = z/(float)steps*(maxes[zIndex]-mins[zIndex]) + mins[zIndex];
            FOR(x, steps)
            {
                sample[xIndex] = x/(float)steps*(maxes[xIndex]-mins[xIndex]) + mins[xIndex];
                fvec res = clusterer->Test(sample);
                if(res.size() == 1) values[x + (y + z*steps)*steps] = res[0];
                else
                {

                    float maxVal = res[0];
                    int maxInd = 0;
                    FOR(d, res.size())
                    {
                        if(maxVal < res[d])
                        {
                            maxInd = d;
                            maxVal = res[d];
                        }
                    }
                    // we keep the class with the highest score
                    values[x + (y + z*steps)*steps] = maxInd;
                    bOneClass = false;
                }
            }
        }
    }
    printf("done.\n");
    fflush(stdout);

    if(!bOneClass)
    {
        FOR(c,clusterCount-1)
        {
            gridT valueGrid(0.f, steps, steps, steps);
            FOR(i, steps*steps*steps)
            {
                if(values[i] == c) valueGrid[i] = 1.f;
                else
                {
                    valueGrid[i] = 2.f;
                    for(int c2=c+1; c2<clusterCount; c2++)
                        if(values[i] == c2) valueGrid[i] = -1.f;
                }
            }
            FOR(d, 3) valueGrid.unit[d] = (maxes[d] - mins[d])/steps;   /* length of a single edge in each dimension*/
            FOR(d, 3) valueGrid.size[d] = maxes[d] - mins[d];           /* length of entire grid in each dimension */
            FOR(d, 3) valueGrid.org[d] = mins[d];                       /* the origin of the grid i.e. coords of (0,0,0) */
            FOR(d, 3) valueGrid.center[d] = (maxes[d] + mins[d])/2;     /* coords of center of grid */

            surfaceT surf;
            float surfThreshold = 0.f;
            unsigned int surfIType = JACSurfaceTypes::SURF_CONTOUR;
            printf("Generating isosurfaces: ");
            fflush(stdout);
            JACMakeSurface(surf, surfIType, valueGrid, surfThreshold);
            JACSmoothSurface(surf);
            JACSmoothSurface(surf);
            JACSmoothSurface(surf);
            //surf.Reduce(0.05);
            printf("done.\n");
            fflush(stdout);

            printf("Generating mesh: ");

            GLObject o;

            std::vector<float> &vertices = surf.vertices;
            //std::vector<float> &normals = surf.normals;
            for (int i=0; i<surf.nconn; i += 3)
            {
                int index = surf.triangles[i];
                o.vertices.append(QVector3D(vertices[index*3],vertices[index*3+1],vertices[index*3+2]));
                index = surf.triangles[i+1];
                o.vertices.append(QVector3D(vertices[index*3],vertices[index*3+1],vertices[index*3+2]));
                index = surf.triangles[i+2];
                o.vertices.append(QVector3D(vertices[index*3],vertices[index*3+1],vertices[index*3+2]));
            }

            QColor color = SampleColor[(c+1)%SampleColorCnt];
            o.objectType = "Surfaces";
            o.style = "smooth,transparent,blurry:1";
            o.style += QString(",color:%1:%2:%3:0.4").arg(color.redF()).arg(color.greenF()).arg(color.blueF());
            o.style += QString(",offset:%1").arg((float)c,0,'f',2);
            glw->mutex->lock();
            glw->AddObject(o);
            glw->mutex->unlock();
            printf("done.\n");
            fflush(stdout);
        }
    }
    else
    {
        gridT valueGrid(0.f, steps, steps, steps);
        FOR(i, steps*steps*steps) valueGrid[i] = values[i];
        FOR(d, 3) valueGrid.unit[d] = (maxes[d] - mins[d])/steps;   /* length of a single edge in each dimension*/
        FOR(d, 3) valueGrid.size[d] = maxes[d] - mins[d];           /* length of entire grid in each dimension */
        FOR(d, 3) valueGrid.org[d] = mins[d];                       /* the origin of the grid i.e. coords of (0,0,0) */
        FOR(d, 3) valueGrid.center[d] = (maxes[d] + mins[d])/2;     /* coords of center of grid */

        surfaceT surf;
        float surfThreshold = 0.f;
        unsigned int surfIType = JACSurfaceTypes::SURF_CONTOUR;
        printf("Generating isosurfaces: ");
        fflush(stdout);
        JACMakeSurface(surf, surfIType, valueGrid, surfThreshold);
        JACSmoothSurface(surf);
        JACSmoothSurface(surf);
        JACSmoothSurface(surf);
        //surf.Reduce(0.05);
        printf("done.\n");
        fflush(stdout);

        printf("Generating mesh: ");

        GLObject o;

        std::vector<float> &vertices = surf.vertices;
        //std::vector<float> &normals = surf.normals;
        for (int i=0; i<surf.nconn; i += 3)
        {
            int index = surf.triangles[i];
            o.vertices.append(QVector3D(vertices[index*3],vertices[index*3+1],vertices[index*3+2]));
            index = surf.triangles[i+1];
            o.vertices.append(QVector3D(vertices[index*3],vertices[index*3+1],vertices[index*3+2]));
            index = surf.triangles[i+2];
            o.vertices.append(QVector3D(vertices[index*3],vertices[index*3+1],vertices[index*3+2]));
        }

        o.objectType = "Surfaces";
        o.style = "smooth,transparent,blurry:1,color:0:0:0:0.3";
        glw->mutex->lock();
        glw->AddObject(o);
        glw->mutex->unlock();
        printf("done.\n");
        fflush(stdout);
    }
}

struct Streamline
{
    std::vector<fvec> trajectory;
    int cluster;
    int length;
public:
    Streamline() : length(0), cluster(0) {}
    Streamline(std::vector<fvec> trajectory)
        : trajectory(trajectory), length(trajectory.size()), cluster(0) {}
    fvec &operator[](int i) {return trajectory[i];}
    fvec &operator()(int i) {return trajectory[i];}
    void push_back(const fvec point) {trajectory.push_back(point); length++;}
    void clear() {trajectory.clear(); length = 0; cluster = 0;}
    int size() {return length;}
    fvec &back() {return trajectory.back();}
    fvec &front() {return trajectory.front();}
};

inline float StreamDistance(Streamline &s, vector<fvec> &mean)
{
    float res = 0;
    int dim = mean[0].size();
    int length = min((int)mean.size(), s.length);
    FOR(i, length)
    {
        FOR(d, dim) res += (mean[i][d]-s[i][d])*(mean[i][d]-s[i][d]);
    }
    return res;
}

ivec ClusterStreams(std::vector<Streamline> streams, int nbClusters, int maxIterations=5)
{
    if(!streams.size()) return ivec();
    int count = streams.size();
    nbClusters = min(count, nbClusters);
    vector< vector<fvec> > means(nbClusters); // the clusters 'mean' trajectories
    ivec clusters(count); // the responsiblity (which cluster each stream belongs to

    // first thing we do is to invert all streams so that we can follow them backwards
    FOR(i, count)
    {
        std::reverse(streams[i].trajectory.begin(), streams[i].trajectory.end());
    }

    // we initialize by picking actual trajectories
    FOR(i, nbClusters)
    {
        int index = i * count / nbClusters;
        means[i] = streams[index].trajectory;
    }

    FOR(it, maxIterations)
    {
        // we recompute the responsiblity for each cluster
        qDebug() << "E Step" << it;
        ivec newClusters(count,0);
        FOR(i, count)
        {
            float minDist = FLT_MAX;
            int minInd = 0;
            FOR(j, nbClusters)
            {
                float dist = StreamDistance(streams[i], means[j]);
                if(dist < minDist)
                {
                    minInd = j;
                    minDist = dist;
                }
            }
            newClusters[i] = minInd;
        }
        if(clusters == newClusters) break; // we've converged!
        clusters = newClusters;

        // and now we compute the new means
        qDebug() << "M Step" << it;
        ivec counts(nbClusters,0);
        vector<ivec> meanCounts(nbClusters);
        FOR(i, count)
        {
            int c = clusters[i];
            if(!counts[c])
            {
                means[c] = streams[i].trajectory;
                meanCounts[c].resize(streams[i].length,1);
            }
            else
            {
                FOR(j, streams[i].size())
                {
                    if(j < means[c].size())
                    {
                        means[c][j] += streams[i][j];
                        meanCounts[c][j]++;
                    }
                    else
                    {
                        means[c].push_back(streams[i][j]);
                        meanCounts[c].push_back(1);
                    }
                }
            }
            counts[c]++;
        }
        FOR(c, nbClusters)
        {
            FOR(j, means[c].size())
            {
                means[c][j] /= meanCounts[c][j];
            }
        }
    }
    return clusters;
}

unsigned int tessIndices[8][3] = {{0,1,2},{0,2,3},{0,3,4},{0,4,1},{5,2,1},{5,3,2},{5,4,3},{5,1,4}};
float tessVerts[6][3] = {{0,0,-1},{1,0,0},{0,-1,0},{-1,0,0},{0,1,0},{0,0,1}};
void normalize_vert(float *a)
{
    float d=sqrtf(a[0]*a[0]+a[1]*a[1]+a[2]*a[2]);
    d = 1.f/d;
    a[0]*=d; a[1]*=d; a[2]*=d;
}

void draw_recursive_tri(float *a,float *b,float *c,unsigned int div, vector<fvec> &vertices)
{
    if (div==0)
    {
        fvec v(3);
        v[0] = (a[0]+b[0]+c[0])/3.f;
        v[1] = (a[1]+b[1]+c[1])/3.f;
        v[2] = (a[2]+b[2]+c[2])/3.f;
        vertices.push_back(v);
    }
    else
    {
        register unsigned int i;
        float ab[3],ac[3],bc[3];
        for (i=0; i<3; i++)
        {
            ab[i]=(a[i]+b[i])/2.0f;
            ac[i]=(a[i]+c[i])/2.0f;
            bc[i]=(b[i]+c[i])/2.0f;
        }
        normalize_vert(ab);
        normalize_vert(ac);
        normalize_vert(bc);
        draw_recursive_tri(a,ab,ac,div-1,vertices);
        draw_recursive_tri(b,bc,ab,div-1,vertices);
        draw_recursive_tri(c,ac,bc,div-1,vertices);
        draw_recursive_tri(ab,bc,ac,div-1,vertices);
    }
}

float **tessellatedSphere(unsigned int detail)
{
    vector<fvec> vertices;
    FOR(i, 8)
        draw_recursive_tri
                (
                    tessVerts[tessIndices[i][0]],
                    tessVerts[tessIndices[i][1]],
                    tessVerts[tessIndices[i][2]],
                    detail,vertices
                    );
    float **tess = new float*[vertices.size()];
    FOR(i, vertices.size())
    {
        tess[i] = new float[3];
        tess[i][0] = vertices[i][0];
        tess[i][1] = vertices[i][1];
        tess[i][2] = vertices[i][2];
    }
    return tess;
}

// level 1: 32, level 2: 128
float **tesssphere = 0;
int tesssize = 32;
inline int binFromVector(float *v)
{
    if(!tesssphere) tesssphere = tessellatedSphere(1);
    int bin = 0;
    float minDist = FLT_MAX;
    FOR(i, tesssize)
    {
        float *t = tesssphere[i];
        float dist = (t[0]-v[0])*(t[0]-v[0]) + (t[1]-v[1])*(t[1]-v[1]) + (t[2]-v[2])*(t[2]-v[2]);
        if(dist < minDist)
        {
            minDist = dist;
            bin = i;
        }
    }
    return bin;
}

fvec ComputeDynamicalEntropy(Dynamical *dynamical, fvec mins, fvec maxes, int gridSteps = 64, int hSteps = 16)
{
    // we begin by generating a dense grid of values
    qDebug() << "dumping vectors to memory";
    vector<fvec> grid(gridSteps*gridSteps*gridSteps);
    fvec sample(3);
    FOR(z, gridSteps)
    {
        sample[2] = z / (float)gridSteps * (maxes[2]-mins[2]) + mins[2];
        FOR(y, gridSteps)
        {
            sample[1] = y / (float)gridSteps * (maxes[1]-mins[1]) + mins[1];
            FOR(x, gridSteps)
            {
                sample[0] = x / (float)gridSteps * (maxes[0]-mins[0]) + mins[0];
                fvec res = dynamical->Test(sample);
//                res = res/sqrtf(res*res); // we normalize it
                grid[x + (y + z*gridSteps)*gridSteps] = res;
            }
        }
    }

    if(!tesssphere) tesssphere = tessellatedSphere(1);

    // bins: binCount*binCount*binCount
    int binCount = tesssize;
    int ratio = gridSteps/hSteps;
    fvec H(hSteps*hSteps*hSteps);

    FOR(i, hSteps)
    {
        FOR(j, hSteps)
        {
            FOR(k, hSteps)
            {
                int bins[32];
                FOR(d, 32) bins[d] = 0;
                // we get the histogram for the current subcube
                FOR(z,ratio)
                {
                    FOR(y,ratio)
                    {
                        FOR(x,ratio)
                        {
                            float *val = &grid[(x + k*ratio) + (y+j*ratio + (z+i*ratio)*gridSteps)*gridSteps][0];
                            int bin = binFromVector(val);
                            bins[bin] += 1;
                        }
                    }
                }
                float sum = ratio*ratio*ratio;
                float entropy = 0;
                FOR(d, binCount)
                {
                    if(bins[d] == 0) continue;
                    float p = bins[d]/sum;
                    float e = p*log2(p);
                    entropy -= e;
                }
                H[k + (j + i*hSteps)*hSteps] = entropy;
            }
        }
    }
    return H;
}

GLObject DrawEntropyField(fvec H, float minv, float maxv, int hSteps)
{
    qDebug() << "drawing entropy field";
    GLObject o;
    o.objectType = "Dynamize,Surfaces,quads";
    o.style = "smooth";
    float minH = FLT_MAX, maxH = -FLT_MAX;
    FOR(i, hSteps*hSteps*hSteps)
    {
        minH = min(minH, H[i]);
        maxH = max(maxH, H[i]);
    }

    FOR(i, hSteps)
    {
        float z = i/(float)hSteps*(maxv-minv) + minv;
        FOR(j, hSteps)
        {
            float y = j/(float)hSteps*(maxv-minv) + minv;
            FOR(k, hSteps)
            {
                float v = H[k + (j + i*hSteps)*hSteps];
                v = (v-minH)/(maxH-minH);
                if(v < 0.01) continue;
                QColor color(Canvas::GetColorMapValue(v,2));
                float x = k/(float)hSteps*(maxv-minv) + minv;
                float r = 0.02;
                o.vertices.push_back(QVector3D(x-r,y-r,z-r));
                o.vertices.push_back(QVector3D(x+r,y-r,z-r));
                o.vertices.push_back(QVector3D(x+r,y+r,z-r));
                o.vertices.push_back(QVector3D(x-r,y+r,z-r));
                FOUR(o.normals.push_back(QVector3D(0,0,1)));
                FOUR(o.colors.push_back(QVector4D(color.redF(), color.greenF(), color.blueF(), 1.f)));
                o.vertices.push_back(QVector3D(x-r,y-r,z+r));
                o.vertices.push_back(QVector3D(x+r,y-r,z+r));
                o.vertices.push_back(QVector3D(x+r,y+r,z+r));
                o.vertices.push_back(QVector3D(x-r,y+r,z+r));
                FOUR(o.normals.push_back(QVector3D(0,0,-1)));
                FOUR(o.colors.push_back(QVector4D(color.redF(), color.greenF(), color.blueF(), 1.f)));
                o.vertices.push_back(QVector3D(x-r,y-r,z-r));
                o.vertices.push_back(QVector3D(x-r,y-r,z+r));
                o.vertices.push_back(QVector3D(x-r,y+r,z+r));
                o.vertices.push_back(QVector3D(x-r,y+r,z-r));
                FOUR(o.normals.push_back(QVector3D(1,0,0)));
                FOUR(o.colors.push_back(QVector4D(color.redF(), color.greenF(), color.blueF(), 1.f)));
                o.vertices.push_back(QVector3D(x+r,y-r,z-r));
                o.vertices.push_back(QVector3D(x+r,y-r,z+r));
                o.vertices.push_back(QVector3D(x+r,y+r,z+r));
                o.vertices.push_back(QVector3D(x+r,y+r,z-r));
                FOUR(o.normals.push_back(QVector3D(-1,0,0)));
                FOUR(o.colors.push_back(QVector4D(color.redF(), color.greenF(), color.blueF(), 1.f)));
                o.vertices.push_back(QVector3D(x-r,y-r,z-r));
                o.vertices.push_back(QVector3D(x-r,y-r,z+r));
                o.vertices.push_back(QVector3D(x+r,y-r,z+r));
                o.vertices.push_back(QVector3D(x+r,y-r,z-r));
                FOUR(o.normals.push_back(QVector3D(0,1,0)));
                FOUR(o.colors.push_back(QVector4D(color.redF(), color.greenF(), color.blueF(), 1.f)));
                o.vertices.push_back(QVector3D(x-r,y+r,z-r));
                o.vertices.push_back(QVector3D(x-r,y+r,z+r));
                o.vertices.push_back(QVector3D(x+r,y+r,z+r));
                o.vertices.push_back(QVector3D(x+r,y+r,z-r));
                FOUR(o.normals.push_back(QVector3D(0,-1,0)));
                FOUR(o.colors.push_back(QVector4D(color.redF(), color.greenF(), color.blueF(), 1.f)));
            }
        }
    }
    return o;
}

GLObject DrawStreamTubes(vector<Streamline> streams, float diff, float maxSpeed, int xInd, int yInd, int zInd)
{
    GLObject o;
    o.objectType = "Dynamize,Surfaces,quads";
    o.style = "smooth";
    FOR(i, streams.size())
    {
        if(streams[i].length < 2) continue;
        int dim = streams[i][0].size();
        float radius = diff*0.001;
        QVector3D p1, p2;
        vector<QVector3D> oldCircle;
        vector<QVector3D> oldNormals;
        QVector3D oldP, pos;
        float oldSpeed = 0.f;
        FOR(j, streams[i].length-1)
        {
            QVector3D d;
            p1 = QVector3D(streams[i][j][xInd],streams[i][j][yInd],zInd >= 0 && zInd < dim ? streams[i][j][zInd] : 0);
            p2 = QVector3D(streams[i][j+1][xInd],streams[i][j+1][yInd],zInd >= 0 && zInd < dim ? streams[i][j+1][zInd] : 0);
            QVector3D dn = (p2-p1);
            d = dn.normalized();
            float speed = dn.length();
            if(j==0)
            {
                if(d.z() != 0)
                {
                    float x=drand48(), y=drand48();
                    float z=-(x*d.x() + y*d.y())/d.z();
                    pos = QVector3D(x,y,z).normalized();
                }
                else
                {
                    float z=drand48(), y=drand48();
                    float x=-(y*d.y() + z*d.z())/d.x();
                    pos = QVector3D(x,y,z).normalized();
                }
            }
            else
            {
                float dDot = QVector3D::dotProduct(d, pos);
                pos = (pos - d*dDot).normalized();
            }

            // we want to generate a circle orthogonal to the segment direction
            int segs=10;
            QQuaternion quat(M_PI, d);
            //QQuaternion quat(M_PI/(float)segs, d);
            vector<QVector3D> circle(segs);
            vector<QVector3D> normals(segs);
            FOR(s, segs)
            {
                circle[s] = pos*radius*(speed*100.f);
                normals[s] = pos;
                pos = quat.rotatedVector(pos).normalized();
            }
            if(j > 0)
            {
                // we look for the closest point to each of the circle

                int offset = 0;

                float minDist=FLT_MAX;
                int minInd = 0;
                FOR(s, segs)
                {
                    QVector3D v = circle[0]-oldCircle[s];
                    float dist = v.lengthSquared();
                    if(dist < minDist)
                    {
                        minInd = s;
                        minDist = dist;
                    }
                }
                offset = minInd;
                QColor cVal(Canvas::GetColorMapValue((speed+oldSpeed)*0.5/maxSpeed,2)); // jet color
                QVector4D color(cVal.redF(),cVal.greenF(),cVal.blueF(),1);
                //QVector4D color(d.x(),d.y(),d.z(),1);

                FOR(s, segs)
                {
                    int i1 = s%segs;
                    int i2 = (s+1)%segs;
                    int oi1 = (s+offset)%segs;
                    int oi2 = (s+1+offset)%segs;
                    o.vertices.append(oldCircle[oi1] + p1);
                    o.vertices.append(circle[i1] + p2);
                    o.vertices.append(circle[i2] + p2);
                    o.vertices.append(oldCircle[oi2] + p1);
                    o.normals.append(oldNormals[i1]);
                    o.normals.append(normals[i1]);
                    o.normals.append(normals[i2]);
                    o.normals.append(oldNormals[i2]);
                    o.colors.append(color);
                    o.colors.append(color);
                    o.colors.append(color);
                    o.colors.append(color);
                }
            }
            oldP = p1;
            oldCircle = circle;
            oldNormals = normals;
            oldSpeed = speed;
        }
    }
    return o;
}

GLObject DrawStreamRibbon(vector<Streamline> streams, Dynamical *dynamical, vector<Obstacle> obstacles, float diff, float maxSpeed, int xInd, int yInd, int zInd)
{
    GLObject o;
    o.objectType = "Dynamize,Surfaces,quads";
    o.style = "smooth";
    FOR(i, streams.size())
    {
        if(streams[i].length < 2) continue;
        int dim = streams[i][0].size();
        float radius = diff*0.05;
        QVector3D p1, p2, q1, q2;
        QVector3D pos;

        // we generate the twin trajectory
        vector<fvec> twin;
        fvec sample(3);
        p1 = QVector3D(streams[i][0][xInd],streams[i][0][yInd],zInd >= 0 && zInd < dim ? streams[i][0][zInd] : 0);
        p2 = QVector3D(streams[i][1][xInd],streams[i][1][yInd],zInd >= 0 && zInd < dim ? streams[i][1][zInd] : 0);
        QVector3D d = (p2-p1).normalized();
        if(d.z() != 0)
        {
            float x=drand48(), y=drand48();
            float z=-(x*d.x() + y*d.y())/d.z();
            pos = QVector3D(x,y,z).normalized();
        }
        else
        {
            float z=drand48(), y=drand48();
            float x=-(y*d.y() + z*d.z())/d.x();
            pos = QVector3D(x,y,z).normalized();
        }
        q1 = pos + p1;
        sample[xInd] = pos.x()*radius + p1.x();
        sample[yInd] = pos.y()*radius + p1.y();
        if(zInd >= 0 && zInd < dim) sample[zInd] = pos.z()*radius + p1.z();
        twin.push_back(sample);
        FOR(j, streams[i].length-1)
        {
            fvec res = dynamical->Test(sample);
            if(dynamical->avoid)
            {
                dynamical->avoid->SetObstacles(obstacles);
                fvec newRes = dynamical->avoid->Avoid(sample, res);
                res = newRes;
            }
            sample += res*dynamical->dT;
            fvec diff = sample - streams[i][j];
            float norm = sqrtf(diff*diff);
            if(norm > radius)
            {
                sample = streams[i][j] + diff/norm*radius;
            }
            twin.push_back(sample);
        }

        float oldSpeed = 0;
        FOR(j, streams[i].length-1)
        {
            p1 = QVector3D(streams[i][j][xInd],streams[i][j][yInd],zInd >= 0 && zInd < dim ? streams[i][j][zInd] : 0);
            p2 = QVector3D(streams[i][j+1][xInd],streams[i][j+1][yInd],zInd >= 0 && zInd < dim ? streams[i][j+1][zInd] : 0);
            q1 = QVector3D(twin[j][xInd],twin[j][yInd],zInd >= 0 && zInd < dim ? twin[j][zInd] : 0);
            q2 = QVector3D(twin[j+1][xInd],twin[j+1][yInd],zInd >= 0 && zInd < dim ? twin[j+1][zInd] : 0);
            float speed = (p2-p1).length();
            speed += (q2-q1).length();
            speed /= 2;

            QColor cVal(Canvas::GetColorMapValue((speed+oldSpeed)*0.5/maxSpeed,2)); // jet color
            QVector4D color(cVal.redF(),cVal.greenF(),cVal.blueF(),1);

            //QVector4D color(d1.x(),d1.y(),d1.z(),1);

            o.vertices.append(p1);
            o.vertices.append(p2);
            o.vertices.append(q2);
            o.vertices.append(q1);
            o.colors.append(color);
            o.colors.append(color);
            o.colors.append(color);
            o.colors.append(color);
        }
    }
    return o;
}

GLObject DrawStreamLines(vector<Streamline> streams, int xInd, int yInd, int zInd)
{
    GLObject o;
    o.objectType = "Dynamize,Lines";
    o.style = "";
//    o.style = QString("fading:%1").arg(steps);
    FOR(i, streams.size())
    {
        if(!streams[i].length) continue;
        int dim = streams[i][0].size();
        QColor c = SampleColor[streams[i].cluster%(SampleColorCnt-1)+1];
        FOR(j, streams[i].length-1)
        {
            o.vertices.append(QVector3D(streams[i][j][xInd],streams[i][j][yInd],zInd >= 0 && zInd < dim ? streams[i][j][zInd] : 0));
            o.vertices.append(QVector3D(streams[i][j+1][xInd],streams[i][j+1][yInd],zInd >= 0 && zInd < dim ? streams[i][j+1][zInd] : 0));
            o.colors.append(QVector4D(c.redF(), c.greenF(), c.blueF(),1));
            o.colors.append(QVector4D(c.redF(), c.greenF(), c.blueF(),1));
        }
    }
    return o;
}

void Draw3DDynamical(GLWidget *glw, Dynamical *dynamical, int displayStyle)
{
    if(!dynamical) return;
    int dim = glw->canvas->data->GetDimCount();
    if(dim != 3) return;
    float dT = dynamical->dT*2; // in 3d we want longer 'trails'
    int xInd = glw->canvas->xIndex;
    int yInd = glw->canvas->yIndex;
    int zInd = glw->canvas->zIndex;
    if(zInd < 0 || zInd >= dim) return;
    vector<fvec> samples = glw->canvas->data->GetSamples();
    vector< vector<fvec> > trajectories = glw->canvas->data->GetTrajectories(glw->canvas->trajectoryResampleType, glw->canvas->trajectoryResampleCount, glw->canvas->trajectoryCenterType, dT, true);
    vector<Obstacle> obstacles = glw->canvas->data->GetObstacles();

    fvec sample(dim,0);
    float minv=FLT_MAX, maxv=-FLT_MAX;
    FOR(i, samples.size())
    {
        FOR(d, dim)
        {
            minv = min(minv, samples[i][d]);
            maxv = max(maxv, samples[i][d]);
        }
    }
    float diff = maxv-minv;
    minv = minv - diff*0.5;
    maxv = maxv + diff*0.5;
    diff = maxv - minv;

    qDebug() << "computing field entropy";
    int gridSteps = 80;
    int hSteps = 20;
    int hRatio = gridSteps/hSteps;
    fvec H = ComputeDynamicalEntropy(dynamical, fvec(dim,minv), fvec(dim,maxv), gridSteps, hSteps);
    vector< pair<float,int> > hList(H.size());
    float hSum = 0;
    float hmin = FLT_MAX, hmax = -FLT_MAX;
    FOR(i, H.size())
    {
        hmin = min(hmin, H[i]);
        hmax = max(hmax, H[i]);
    }

    FOR(i, H.size())
    {
        hList[i] = make_pair((H[i]-hmin)/(hmax-hmin), i);
        hSum += hList[i].first;
    }
    //sort(hList.begin(), hList.end(), std::greater< pair<float,int> >());
    sort(hList.begin(), hList.end()); // smallest to largest
    reverse(hList.begin(), hList.end());

    /*
    FOR(i, hList.size())
    {
        qDebug() << "h values ["<< i <<"]:" << hList[i].first;
    }
    */
    // we generate seed points proportionally to the field entropy
    int seedlings = 80;
    vector<fvec> seeds(seedlings);
    float voxelSize = (maxv-minv)/(float)hSteps;
    FOR(s, seedlings)
    {
        float r = drand48()*hSum;
        float sum = 0;
        FOR(i, hList.size())
        {
            sum += hList[i].first;
            if(r < sum || i==hList.size()-1)
            {
                int index = hList[i].second;
                int x = index%hSteps;
                int y = (index/hSteps)%hSteps;
                int z = index/(hSteps*hSteps);
                // we generate a random sample inside the selected voxel
                sample[0] = drand48()*voxelSize + (x/(float)hSteps*(maxv-minv) + minv);
                sample[1] = drand48()*voxelSize + (y/(float)hSteps*(maxv-minv) + minv);
                sample[2] = drand48()*voxelSize + (z/(float)hSteps*(maxv-minv) + minv);
                seeds[s] = sample;
                break;
            }
        }
    }

    fvec mins(3,minv), maxes(3,maxv), origin=(maxes+mins)*0.5f;
    // we generate the trajectories
    int steps = 400;
    float maxSpeed = -FLT_MAX;
    vector<Streamline> streams(seeds.size());
    FOR(i, seeds.size())
    {
        sample = seeds[i];
        Streamline s;
        s.push_back(sample);
        FOR(j, steps)
        {
            fvec res = dynamical->Test(sample);
            if(dynamical->avoid)
            {
                dynamical->avoid->SetObstacles(obstacles);
                fvec newRes = dynamical->avoid->Avoid(sample, res);
                res = newRes;
            }
            float speed = sqrtf((res*dT)*(res*dT));
            if(speed > maxSpeed) maxSpeed = speed;
            sample += res*dT;
            if(speed < 1e-4) break;
            if(sqrtf((sample - origin)*(sample - origin)) > diff*0.7) break;
            s.push_back(sample);
        }
        s.cluster = i;
        streams[i] = s;
    }

    GLObject o;
    switch(displayStyle)
    {
    case 0: // entropy field
        o = DrawEntropyField(H, minv, maxv, hSteps);
        break;
    case 1: // tubes
        o = DrawStreamTubes(streams, diff, maxSpeed, xInd, yInd, zInd);
        break;
    case 2: // ribbons
        o = DrawStreamRibbon(streams, dynamical, obstacles, diff, maxSpeed, xInd, yInd, zInd);
        break;
    case 3: // Animation
        o = DrawStreamLines(streams, xInd, yInd, zInd);
        break;
    }

    // we replace the old vector field (if there is one) with the new one
    glw->mutex->lock();
    int oInd = -1;
    FOR(i, glw->objects.size())
    {
        if(!glw->objectAlive[i]) continue;
        if(glw->objects[i].objectType.contains("Dynamize"))
        {
            oInd = i;
            break;
        }
    }
    if(oInd != -1) glw->killList.push_back(oInd);
    glw->AddObject(o);
    glw->mutex->unlock();
}

void Draw3DMaximizer(GLWidget *glw, Maximizer *maximizer){}
void Draw3DProjector(GLWidget *glw, Projector *projector){}
void Draw3DReinforcement(GLWidget *glw, Reinforcement *reinforcement){}

QLabel *label = 0;
void Draw2DDynamical(Canvas *canvas, Dynamical *dynamical)
{
    int w = canvas->width();
    int h = canvas->height();
    // we start by generating random noise
    QImage pixels(w,h,QImage::Format_RGB32);
    FOR(i, w*h)
    {
        int x = i%w;
        int y = i/w;
        float value = max(0.f,min(1.f,RandN(0.5f,0.5f)));
        pixels.setPixel(x,y,qRgb(value*255,value*255,value*255));
    }

    // now we "process" the noise by iteratively applying the DS to it
    QPainter painter(&pixels);
    painter.setRenderHint(QPainter::Antialiasing);
    int iterations = 4;
    float dT = 0.004;
    vector<Obstacle> obstacles = canvas->data->GetObstacles();
    qDebug() << "processing noise";
    FOR(i, iterations)
    {
        qDebug() << "iteration" << i;
        FOR(y, h)
        {
            FOR(x, w)
            {
                QPoint point(x,y);
                QRgb val = pixels.pixel(point);
                fvec sample = canvas->fromCanvas(x,y);
                fvec res = dynamical->Test(sample);
                if(dynamical->avoid)
                {
                    dynamical->avoid->SetObstacles(obstacles);
                    fvec newRes = dynamical->avoid->Avoid(sample, res);
                    res = newRes;
                }
                sample += res*dT;
                QPointF point2 = canvas->toCanvasCoords(sample);
                painter.setPen(QColor(val));
                painter.drawLine(point, point2);
                //if(point.x() < 0 || point.x() >= w || point.y() < 0 || point.y() >= h) continue;
                //pixels.setPixel(point.x(), point.y(), val);
            }
        }
    }

    QPixmap pixmap = QPixmap::fromImage(pixels);
//    QPixmap pixmap = QPixmap::fromImage(pixels).scaled(w*2, h*2,Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    if(!label)
    {
        label = new QLabel();
        label->setScaledContents(true);
    }
    label->setPixmap(pixmap);
    label->show();
}
