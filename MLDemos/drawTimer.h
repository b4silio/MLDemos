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
#ifndef _DRAWTIMER_H_
#define _DRAWTIMER_H_

#include <QWidget>
#include <QThread>
#include "canvas.h"
#include "classifier.h"
#include "regressor.h"
#include "dynamical.h"
#include "clusterer.h"
#include <QMutex>
#include <QMutexLocker>

class DrawTimer : public QThread
{
	Q_OBJECT
private:
	int refineLevel;
	int refineMax;
	QImage bigMap;
	QImage modelMap;
	u32 *perm;
	Canvas *canvas;
	int w, h;

public:
	DrawTimer(Canvas *canvas, QMutex *mutex);
	~DrawTimer();
	void run();
	void Refine();
	void Clear();
	void Test(int start, int stop);
	void TestFast(int start, int stop);
	void Vectors(int count, int steps);
	void VectorsFast(int count, int steps);
	void stop();

	Classifier **classifier;
	Regressor **regressor;
	Dynamical **dynamical;
	Clusterer **clusterer;

	QMutex *mutex;
	bool bPaused;
	bool bRunning;
	bool bStopping;

	signals:
	void MapReady(QImage image);
	void ModelReady(QImage image);
};

#endif // _DRAWTIMER_H_
