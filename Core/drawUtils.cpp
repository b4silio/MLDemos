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
#include "basicMath.h"
#include <QBitmap>
using namespace std;

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

	FOR(d, rocdata.size())
	{
        int minCol = 128;
		int color = (rocdata.size() == 1) ? 255 : (255 - minCol)*(rocdata.size() - d -1)/(rocdata.size()-1) + minCol;
		color = 255 - color;

		std::vector<f32pair> data = rocdata[d];
		if(!data.size()) continue;
		std::sort(data.begin(), data.end());

		std::vector<fvec> allData;
		FOR(i, data.size())
		{
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
			fVec val;
			float fmeasure = 0;
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

        painter.setPen(QPen(QColor(color,color,color), 1.f));

		fVec pt1, pt2;
		FOR(i, allData.size()-1)
		{
			pt1 = fVec(allData[i][0]*size.width(), allData[i][1]*size.height());
			pt2 = fVec(allData[i+1][0]*size.width(), allData[i+1][1]*size.height());
			painter.drawLine(QPointF(pt1.x+PAD, pt1.y+PAD),QPointF(pt2.x+PAD, pt2.y+PAD));
		}
		pt1 = fVec(0,size.width());
		painter.drawLine(QPointF(pt1.x+PAD, pt1.y+PAD),QPointF(pt2.x+PAD, pt2.y+PAD));

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
	QBitmap bitmap;
	bitmap.clear();
	boxplot.setMask(bitmap);
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
			quartLow, quartHi;
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
	QBitmap bitmap;
	bitmap.clear();
	histogram.setMask(bitmap);
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
    QBitmap bitmap;
    bitmap.clear();
    rawData.setMask(bitmap);
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
