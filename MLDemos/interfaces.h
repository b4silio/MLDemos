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
#ifndef _INTERFACES_H_
#define _INTERFACES_H_

#include <vector>
#include <fstream>
#include "classifier.h"
#include "clusterer.h"
#include "regressor.h"
#include "dynamical.h"
#include "canvas.h"
#include "drawTimer.h"
#include <QtPlugin>
#include <QWidget>
#include <QSettings>

class ClassifierInterface
{
public:
	// virtual functions to manage the algorithm creation
	virtual Classifier *GetClassifier() = 0;
	virtual void DrawInfo(Canvas *canvas, Classifier *classifier) = 0;
	virtual void Draw(Canvas *canvas, Classifier *classifier) = 0;

	// virtual functions to manage the GUI and I/O
	virtual QString GetName() = 0;
	virtual QWidget *GetParameterWidget() = 0;
	virtual void SetParams(Classifier *classifier) = 0;
	virtual void SaveOptions(QSettings &settings) = 0;
	virtual bool LoadOptions(QSettings &settings) = 0;
	virtual void SaveParams(std::ofstream &stream) = 0;
	virtual bool LoadParams(char *line, float value) = 0;
	virtual bool UsesDrawTimer() = 0;
};

class ClustererInterface
{
public:
	// virtual functions to manage the algorithm creation
	virtual Clusterer *GetClusterer() = 0;
	virtual void DrawInfo(Canvas *canvas, Clusterer *clusterer) = 0;
	virtual void Draw(Canvas *canvas, Clusterer *clusterer) = 0;

	// virtual functions to manage the GUI and I/O
	virtual QString GetName() = 0;
	virtual QWidget *GetParameterWidget() = 0;
	virtual void SetParams(Clusterer *clusterer) = 0;
	virtual void SaveOptions(QSettings &settings) = 0;
	virtual bool LoadOptions(QSettings &settings) = 0;
	virtual void SaveParams(std::ofstream &stream) = 0;
	virtual bool LoadParams(char *line, float value) = 0;
};

class RegressorInterface
{
public:
	// virtual functions to manage the algorithm creation
	virtual Regressor *GetRegressor() = 0;
	virtual void DrawInfo(Canvas *canvas, Regressor *regressor) = 0;
	virtual void Draw(Canvas *canvas, Regressor *regressor) = 0;

	// virtual functions to manage the GUI and I/O
	virtual QString GetName() = 0;
	virtual QWidget *GetParameterWidget() = 0;
	virtual void SetParams(Regressor *regressor) = 0;
	virtual void SaveOptions(QSettings &settings) = 0;
	virtual bool LoadOptions(QSettings &settings) = 0;
	virtual void SaveParams(std::ofstream &stream) = 0;
	virtual bool LoadParams(char *line, float value) = 0;
};

class DynamicalInterface
{
public:
	// virtual functions to manage the algorithm creation
	virtual Dynamical *GetDynamical() = 0;
	virtual void DrawInfo(Canvas *canvas, Dynamical *dynamical) = 0;
	virtual void Draw(Canvas *canvas, Dynamical *dynamical) = 0;

	// virtual functions to manage the GUI and I/O
	virtual QString GetName() = 0;
	virtual QWidget *GetParameterWidget() = 0;
	virtual void SetParams(Dynamical *dynamical) = 0;
	virtual void SaveOptions(QSettings &settings) = 0;
	virtual bool LoadOptions(QSettings &settings) = 0;
	virtual void SaveParams(std::ofstream &stream) = 0;
	virtual bool LoadParams(char *line, float value) = 0;
	virtual bool UsesDrawTimer() = 0;
};

class AvoidanceInterface
{
public:
	virtual ObstacleAvoidance *GetObstacleAvoidance() = 0;
	virtual QString GetName() = 0;
	virtual QWidget *GetParameterWidget() = 0;
	virtual void SetParams(ObstacleAvoidance *avoid) = 0;
	virtual void SaveOptions(QSettings &settings) = 0;
	virtual bool LoadOptions(QSettings &settings) = 0;
	virtual void SaveParams(std::ofstream &stream) = 0;
	virtual bool LoadParams(char *line, float value) = 0;
};

class CollectionInterface
{
public:
	virtual std::vector<ClassifierInterface *> GetClassifiers() = 0;
	virtual std::vector<ClustererInterface *> GetClusterers() = 0;
	virtual std::vector<RegressorInterface *> GetRegressors() = 0;
	virtual std::vector<DynamicalInterface *> GetDynamicals() = 0;
};

class InputOutputInterface
{
public:
	// signatures for plugin slots and signals (SLOT() and SIGNAL() functions)
	virtual const char* QueryClassifierSignal() = 0; // void QueryClassifier(std::vector<fvec> samples);
	virtual const char* QueryRegressorSignal() = 0; // void QueryRegressor(std::vector<fvec> samples);
	virtual const char* QueryDynamicalSignal() = 0; // void QueryDynamical(std::vector<fvec> samples);
	virtual const char* QueryClustererSignal() = 0; // void QueryClusterer(std::vector<fvec> samples);
	virtual const char* SetDataSignal() = 0; // void SetData(std::vector<fvec> samples, ivec labels, std::vector<ipair> trajectories);
	virtual const char* FetchResultsSlot() = 0; // void FetchResults(std::vector<fvec> results);
	virtual QObject *object() = 0; // trick to get access to the QObject interface for signals and slots
	virtual const char* DoneSignal() = 0; // void Done(QObject *);

	virtual QString GetName() = 0;
	virtual void Start() = 0;
	virtual void Stop() = 0;
};

Q_DECLARE_INTERFACE(ClassifierInterface, "com.MLDemos.ClassifierInterface/1.0")
Q_DECLARE_INTERFACE(ClustererInterface, "com.MLDemos.ClustererInterface/1.0")
Q_DECLARE_INTERFACE(RegressorInterface, "com.MLDemos.RegressorInterface/1.0")
Q_DECLARE_INTERFACE(DynamicalInterface, "com.MLDemos.DynamicalInterface/1.0")
Q_DECLARE_INTERFACE(AvoidanceInterface, "com.MLDemos.AvoidInterface/1.0")
Q_DECLARE_INTERFACE(CollectionInterface, "com.MLDemos.CollectionInterface/1.0")
Q_DECLARE_INTERFACE(InputOutputInterface, "com.MLDemos.InputOutputInterface/1.0")

#endif // _INTERFACES_H_
