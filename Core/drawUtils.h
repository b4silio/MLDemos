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
#ifndef _DRAW_UTILS_H_
#define _DRAW_UTILS_H_

#include "public.h"
#include "mymaths.h"
#include "basicMath.h"
#include "canvas.h"
#include "roc.h"
#include <QPainter>

#include "glwidget.h"
#include "classifier.h"
#include "clusterer.h"
#include "regressor.h"
#include "reinforcement.h"
#include "projector.h"
#include "dynamical.h"
#include "maximize.h"

void DrawEllipse(float *mean, float *sigma, float rad, QPainter *painter, QSize size);
void DrawEllipse(float *mean, float *sigma, float rad, QPainter *painter, Canvas *canvas);
void DrawArrow( const QPointF &ppt, const QPointF &pt, double sze, QPainter &painter);
QColor ColorFromVector(fvec a);
QPixmap RocImage(std::vector< std::vector<f32pair> > rocdata, std::vector<const char *> roclabels, QSize size);
QPixmap BoxPlot(std::vector<fvec> allData, QSize size, float maxVal=-FLT_MAX, float minVal=FLT_MAX);
QPixmap Histogram(std::vector<fvec> allData, QSize size, float maxVal=-FLT_MAX, float minVal=FLT_MAX);
QPixmap RawData(std::vector<fvec> allData, QSize size, float maxVal=-FLT_MAX, float minVal=FLT_MAX);


void Draw2DDynamical(Canvas *canvas, Dynamical *dynamical);
void Draw3DClassifier(GLWidget *glw, Classifier *classifier);
void Draw3DRegressor(GLWidget *glw, Regressor *regressor);
void Draw3DClusterer(GLWidget *glw, Clusterer *clusterer);
void Draw3DMaximizer(GLWidget *glw, Maximizer *maximizer);
void Draw3DDynamical(GLWidget *glw, Dynamical *dynamical, int displayStyle);
void Draw3DProjector(GLWidget *glw, Projector *projector);
void Draw3DReinforcement(GLWidget *glw, Reinforcement *reinforcement);

#endif // _DRAW_UTILS_H_
