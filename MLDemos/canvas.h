/*********************************************************************
MLDemos: A User-Friendly visualization toolkit for machine learning
Copyright (C) 2010  Basilio Noris
Contact: mldemos@b4silio.com

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public License,
version 3 as published by the Free Software Foundation.

This library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free
Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*********************************************************************/
#ifndef _CANVAS_H_
#define _CANVAS_H_

#include "datasetManager.h"
#include "mymaths.h"
#include <QWidget>
#include <map>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPainter>
#include <QPainterPath>

class Canvas : public QWidget
{
	Q_OBJECT

public:
	Canvas(QWidget *parent);
	~Canvas();

	bool DeleteData(QPointF center, float radius);
	static bool bCrossesAsDots;
	void DrawSamples();
	void DrawObstacles();
	void DrawTrajectories();
	void DrawLiveTrajectory(QPainter &painter);
	void ResetSamples(){drawnSamples = 0; drawnTrajectories = 0;};
	void FitToData();
	void RedrawAxes();
	bool bDrawing;
	QPainterPath DrawObstacle(Obstacle o);
	fVec center;
	float zoom;

protected:
	void paintEvent(QPaintEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void wheelEvent(QWheelEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void enterEvent(QEvent *event);
	void focusOutEvent(QFocusEvent *event);
	void leaveEvent(QEvent *event);

public:
	DatasetManager *data;
	QPixmap confidencePixmap;
	QPixmap modelPixmap;
	QPixmap infoPixmap;
	QPixmap gridPixmap;
	QPainterPath crosshair;
	QPixmap samplesPixmap;
	QPixmap trajectoriesPixmap;
	QPixmap obstaclesPixmap;
	QImage qimg;
	bool bDisplayMap, bDisplayInfo, bDisplaySingle, bDisplaySamples, bDisplayTrajectories, bDisplayLearned, bDisplayGrid;
	bool bShowCrosshair, bNewCrosshair;
	int trajectoryCenterType, trajectoryResampleType, trajectoryResampleCount;
	QPoint mouse, mouseAnchor;
	fVec fromCanvas(QPointF point);
	fVec fromCanvas(float x, float y);
	QPointF toCanvasCoords(float x, float y);
	QPointF toCanvasCoords(fvec sample);
	QPointF toCanvas(fVec sample);
	fvec toSampleCoords(QPointF point);
	fvec toSampleCoords(float x, float y);
	fvec canvasTopLeft();
	fvec canvasBottomRight();
	QRectF canvasRect();
	void SetZoom(float zoom);
	float GetZoom(){return zoom;};
	void SetCenter(fVec center);
	fvec GetCenter(){return center;};

	std::map<int,fvec> centers;
	int drawnSamples;
	int drawnTrajectories;
	std::vector<fvec> liveTrajectory;

	bool SaveScreenshot(QString filename);
	QPixmap GetScreenshot();

public slots:
	void ResizeEvent();
	void SetConfidenceMap(QImage image);
	void SetModelImage(QImage image);

signals:
	void DrawCrosshair();
	void Drawing(fvec sample, int label);
        void CanvasMoveEvent();
	void Released();
	void Navigation(fvec sample);

public:
	static QPixmap toPixmap(IplImage *src)
	{
		QPixmap pixmap;
		if(src->nChannels == 4)
		{
			pixmap = QPixmap::fromImage(QImage((const unsigned char *)src->imageData,src->width, src->height, QImage::Format_RGB32)).copy();
		}
		else
		{
			IplImage *image = cvCreateImage(cvGetSize(src),8,4);
			cvCvtColor(src, image, src->nChannels==1 ? CV_GRAY2BGRA : CV_BGR2BGRA);
			QImage qimg = QImage((const unsigned char *)image->imageData, image->width, image->height, QImage::Format_RGB32);
			pixmap = QPixmap::fromImage(qimg).copy();
			cvReleaseImage(&image);
		}
		return pixmap;
	}
	static void drawCross(QPainter &painter, QPointF point, float radius, int label)
	{
		float x = point.x();
		float y = point.y();
		QColor c1 = Qt::white;
		QColor c2 = Qt::black;

		switch(label)
		{
		case 0:
			c1 = Qt::white;
			c2 = Qt::black;
			break;
		case 1:
			c1 = Qt::black;
			c2 = Qt::white;
			break;
		case 2:
			c1 = Qt::red;
			c2 = Qt::black;
			break;
		case 3:
			c1 = Qt::blue;
			c2 = Qt::white;
			break;
		default:
			srand(label);
			c1 = QColor(rand()%255,rand()%255,rand()%255);
			c2 = Qt::black;
			break;
		}

		QPen pen = painter.pen();
		pen.setColor(c1);pen.setWidth(3);
		painter.setPen(pen);
		painter.drawLine(QPointF(x-radius, y-radius), QPointF(x+radius, y+radius));
		painter.drawLine(QPointF(x+radius, y-radius), QPointF(x-radius, y+radius));
		pen.setColor(c2);pen.setWidth(1);
		painter.setPen(pen);
		painter.drawLine(QPointF(x-radius, y-radius), QPointF(x+radius, y+radius));
		painter.drawLine(QPointF(x+radius, y-radius), QPointF(x-radius, y+radius));
	}

	static void drawSample(QPainter &painter, QPointF point, float radius, int label)
	{
		float x = point.x();
		float y = point.y();

		if(label == 0)
		{
			//		radius = 10;
			painter.setBrush(Qt::white);
			painter.setPen(Qt::black);
			painter.drawEllipse(QRectF(x-radius/2.,y-radius/2.,radius,radius));
		}
		else if(label == 1)
		{
			//		radius = 10;
			painter.setBrush(Qt::black);
			painter.setPen(Qt::white);
			painter.drawEllipse(QRectF(x-radius/2.,y-radius/2.,radius,radius));
		}
		else if(label == 2)
		{
			// 		radius = 10;
			painter.setBrush(Qt::red);
			painter.setPen(Qt::black);
			painter.drawEllipse(QRectF(x-radius/2.,y-radius/2.,radius,radius));
		}
		else if(label == 3)
		{
			// 		radius = 10;
			painter.setBrush(Qt::blue);
			painter.setPen(Qt::white);
			painter.drawEllipse(QRectF(x-radius/2.,y-radius/2.,radius,radius));
		}
		else
		{
			// 		radius = 10;
			srand(label);
			painter.setBrush(QColor(rand()%255,rand()%255,rand()%255));
			painter.setPen(Qt::black);
			painter.drawEllipse(QRectF(x-radius/2.,y-radius/2.,radius,radius));
		}
	}
};

#endif // _CANVAS_H_
