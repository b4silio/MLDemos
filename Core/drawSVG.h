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
#ifndef _DRAWSVG_H_
#define _DRAWSVG_H_

#include <QtSvg>
#include <QThread>
#include "canvas.h"
#include "classifier.h"
#include "regressor.h"
#include "dynamical.h"
#include "clusterer.h"
#include "maximize.h"
#include "interfaces.h"
#include <QMutex>
#include <QMutexLocker>

class DrawSVG
{
private:
	u32 *perm;
	Canvas *canvas;
	int w, h;

public:
	DrawSVG(Canvas *canvas, QMutex *mutex);
	~DrawSVG();
	void Write(QString filename);
	void Vectors(int count, int steps, QPainter &painter);
	void VectorsFast(int count, int steps, QPainter &painter);
	void Maximization(QPainter &painter);
    void DrawClassificationSamples(Canvas *canvas, QPainter &painter, Classifier *classifier, std::vector<Classifier*> classifierMulti);

	Classifier *classifier;
    std::vector<Classifier *> classifierMulti;
	Regressor *regressor;
	Dynamical *dynamical;
	Clusterer *clusterer;
	Maximizer *maximizer;
    Projector *projector;

	ClassifierInterface *drawClass;
	RegressorInterface *drawRegr;
	DynamicalInterface *drawDyn;
	ClustererInterface *drawClust;
    ProjectorInterface *drawProj;

	QMutex *mutex;
};

#endif // _DRAWSVG_H_
