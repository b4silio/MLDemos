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

struct Pixmaps
{
	int w, h;
	QPixmap confidence;
	QPixmap reward;
	QPixmap model;
	QPixmap info;
	QPixmap grid;
	QPixmap samples;
	QPixmap trajectories;
	QPixmap obstacles;
	QPixmap timeseries;
    QPixmap animation;
    void clear() {
		confidence = QPixmap();
		reward = QPixmap();
		model = QPixmap();
		info = QPixmap();
		grid = QPixmap();
		samples = QPixmap();
		trajectories = QPixmap();
		obstacles = QPixmap();
		timeseries = QPixmap();
        animation = QPixmap();
    }
};

class Canvas : public QWidget
{
	Q_OBJECT

public:
	Canvas(QWidget *parent);
	~Canvas();

    static bool bCrossesAsDots;
    bool DeleteData(QPointF center, float radius);
    ivec SelectSamples(QPointF center, float radius, fvec *weights=0);
    void DrawSamples();
	void DrawObstacles();
	void DrawTrajectories();
	void DrawTimeseries();
	void DrawRewards();
	void DrawObstacles(QPainter &painter);
	void DrawTrajectories(QPainter &painter);
	void DrawSamples(QPainter &painter);
    void DrawSampleColors(QPainter &painter);
	void DrawTargets(QPainter &painter);
	void DrawLiveTrajectory(QPainter &painter);
    void DrawLegend(QPainter &painter);
    void DrawAxes(QPainter &painter);
    void ResetSamples(){drawnSamples = 0; drawnTrajectories = 0; drawnTimeseries = 0;}
	void FitToData();
	void RedrawAxes();
    void SetCanvasType(int);

    void PaintGaussian(QPointF position, double variance);
	void PaintReward(fvec sample, float radius, float shift);
    void PaintGradient(QPointF position);
    bool bDrawing;
	QPainterPath DrawObstacle(Obstacle o);
    fvec center;
	float zoom;
	fvec zooms;
    fvec mins, maxes;
    int xIndex, yIndex, zIndex;
	std::vector<fvec> targets;
    ivec targetAge;
    int canvasType;
    std::vector<QColor> sampleColors;
    QStringList dimNames;
    std::map<int,QString> classNames;
	QString GetClassString(int classNumber);

protected:
	void paintEvent(QPaintEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void wheelEvent(QWheelEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void enterEvent(QEvent *event);
	void focusOutEvent(QFocusEvent *event);
	void leaveEvent(QEvent *event);
	void dragEnterEvent(QDragEnterEvent *);
	void dropEvent(QDropEvent *);

public:
	DatasetManager *data;
	Pixmaps maps;
    QPixmap& confidencePixmap(){return maps.confidence;}
    QPixmap& rewardPixmap(){return maps.reward;}
    QPixmap& modelPixmap(){return maps.model;}
    QPixmap& infoPixmap(){return maps.info;}
    QPixmap& gridPixmap(){return maps.grid;}
    QPixmap& samplesPixmap(){return maps.samples;}
    QPixmap& trajectoriesPixmap(){return maps.trajectories;}
    QPixmap& obstaclesPixmap(){return maps.obstacles;}
	QImage qimg;
	QPainterPath crosshair;
	bool bDisplayMap, bDisplayInfo, bDisplaySingle, bDisplaySamples;
    bool bDisplayTrajectories, bDisplayLearned, bDisplayGrid, bDisplayTimeSeries, bDisplayLegend;
	bool bShowCrosshair, bNewCrosshair;
	int trajectoryCenterType, trajectoryResampleType, trajectoryResampleCount;
	QPoint mouse, mouseAnchor;
	fvec fromCanvas(QPointF point);
	fvec fromCanvas(float x, float y);
	QPointF toCanvasCoords(float x, float y);
	QPointF toCanvasCoords(fvec sample);
	QPointF toCanvas(fVec sample);
	fvec toSampleCoords(QPointF point);
	fvec toSampleCoords(float x, float y);
	fvec canvasTopLeft();
	fvec canvasBottomRight();
	QRectF canvasRect();
	void SetZoom(float zoom);
	void SetZoom(fvec zooms);
	float GetZoom(){return zoom;}
	void SetCenter(fvec center);
	fvec GetCenter(){return center;}
    void SetDim(int xIndex, int yIndex, int zIndex=0);

	std::map<int,fvec> centers;
	int drawnSamples;
	int drawnTrajectories;
	int drawnTimeseries;
	std::vector<fvec> liveTrajectory;

    void PaintStandard(QPainter &painter, bool bSvg=false);
    void PaintMultivariate(QPainter &painter, int type);
    void PaintVariable(QPainter &painter, int type, fvec params);

	bool SaveScreenshot(QString filename);
	QPixmap GetScreenshot();

public slots:
    void Clear();
	void ResizeEvent();
	void SetConfidenceMap(QImage image);
	void SetModelImage(QImage image);
    void SetAnimationImage(QImage animation);

signals:
	void DrawCrosshair();
	void Drawing(fvec sample, int label);
	void CanvasMoveEvent();
	void Released();
	void Navigation(fvec sample);

public:
	/*
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
 */
	static void drawCross(QPainter &painter, QPointF point, float radius, int label)
	{
		float x = point.x();
		float y = point.y();

		QColor c1 = SampleColor[label%SampleColorCnt];
		QColor c2 = Qt::black;
		if(label == 1)
		{
			c1 = Qt::black;
			c2 = Qt::white;
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

		QColor color = SampleColor[label%SampleColorCnt];
		QColor edge = Qt::black;
        if(label == -1)
        {
            color = Qt::black;
            edge = Qt::white;
        }
		//		radius = 10;
        if(painter.brush().color() != color) painter.setBrush(color);
        if(painter.pen().color() != edge) painter.setPen(edge);
		painter.drawEllipse(QRectF(x-radius/2.,y-radius/2.,radius,radius));

	}

    static QRgb GetColorMapValue(float value, int colorscheme);
};

#endif // _CANVAS_H_
