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
#include "maximize.h"
#include "reinforcement.h"
#include "projector.h"
#include "canvas.h"
#include "drawTimer.h"
#include <QtPlugin>
#include <QWidget>
#include <QSettings>
#include <QBitmap>
#include <QPainter>
#include <QUrl>
#include <QTextStream>

class ClassifierInterface
{
public:
	// virtual functions to manage the algorithm creation
	virtual Classifier *GetClassifier() = 0;
	virtual void DrawModel(Canvas *canvas, QPainter &painter, Classifier *classifier) = 0;
	virtual void DrawInfo(Canvas *canvas, QPainter &painter, Classifier *classifier) = 0;
    virtual ~ClassifierInterface(){}

	// virtual functions to manage the GUI and I/O
	virtual QString GetName() = 0;
	virtual QString GetAlgoString() = 0;
	virtual QString GetInfoFile() = 0;
	virtual QWidget *GetParameterWidget() = 0;
	virtual void SetParams(Classifier *classifier) = 0;
	virtual void SaveOptions(QSettings &settings) = 0;
	virtual bool LoadOptions(QSettings &settings) = 0;
	virtual void SaveParams(QTextStream &stream) = 0;
	virtual bool LoadParams(QString name, float value) = 0;

	// drawing function
	void Draw(Canvas *canvas, Classifier *classifier)
	{
		if(!classifier || !canvas) return;
		canvas->liveTrajectory.clear();
		int w = canvas->width();
		int h = canvas->height();

		{
			canvas->maps.model = QPixmap(w,h);
			QBitmap bitmap(w,h);
			bitmap.clear();
			canvas->maps.model.setMask(bitmap);
			canvas->maps.model.fill(Qt::transparent);
			QPainter painter(&canvas->maps.model);
            if(!canvas->canvasType) DrawModel(canvas, painter, classifier);
		}

		{
			canvas->maps.info = QPixmap(w,h);
			QBitmap bitmap(w,h);
			bitmap.clear();
			canvas->maps.info.setMask(bitmap);
			canvas->maps.info.fill(Qt::transparent);
			QPainter painter(&canvas->maps.info);
            if(!canvas->canvasType) DrawInfo(canvas, painter, classifier);
		}
		canvas->maps.confidence = QPixmap();
		canvas->repaint();
	}
};

class ClustererInterface
{
public:
	// virtual functions to manage the algorithm creation
	virtual Clusterer *GetClusterer() = 0;
	virtual void DrawInfo(Canvas *canvas, QPainter &painter, Clusterer *clusterer) = 0;
	virtual void DrawModel(Canvas *canvas, QPainter &painter, Clusterer *clusterer) = 0;
    virtual ~ClustererInterface(){}

	// virtual functions to manage the GUI and I/O
	virtual QString GetName() = 0;
	virtual QString GetAlgoString() = 0;
	virtual QString GetInfoFile() = 0;
	virtual QWidget *GetParameterWidget() = 0;
	virtual void SetParams(Clusterer *clusterer) = 0;
	virtual void SaveOptions(QSettings &settings) = 0;
	virtual bool LoadOptions(QSettings &settings) = 0;
	virtual void SaveParams(QTextStream &stream) = 0;
	virtual bool LoadParams(QString name, float value) = 0;

	void Draw(Canvas *canvas, Clusterer *clusterer)
	{
		if(!canvas || !clusterer) return;
		canvas->liveTrajectory.clear();
		int w = canvas->width();
		int h = canvas->height();
		{
			QPixmap modelPixmap(w, h);
			QBitmap bitmap(w,h);
			bitmap.clear();
			modelPixmap.setMask(bitmap);
			modelPixmap.fill(Qt::transparent);
			QPainter painter(&modelPixmap);
            if(!canvas->canvasType) DrawModel(canvas, painter, clusterer);
			canvas->maps.model = modelPixmap;
		}

		{
			QPixmap infoPixmap(w, h);
			QBitmap bitmap(w,h);
			bitmap.clear();
			infoPixmap.setMask(bitmap);
			infoPixmap.fill(Qt::transparent);
			QPainter painter(&infoPixmap);
            if(!canvas->canvasType) DrawInfo(canvas, painter, clusterer);
			canvas->maps.info = infoPixmap;
		}
		canvas->repaint();
	}
};

class RegressorInterface
{
public:
	// virtual functions to manage the algorithm creation
	virtual Regressor *GetRegressor() = 0;
	virtual void DrawInfo(Canvas *canvas, QPainter &painter, Regressor *regressor) = 0;
	virtual void DrawModel(Canvas *canvas, QPainter &painter, Regressor *regressor) = 0;
	virtual void DrawConfidence(Canvas *canvas, Regressor *regressor) = 0;
    virtual ~RegressorInterface(){}

	// virtual functions to manage the GUI and I/O
	virtual QString GetName() = 0;
	virtual QString GetAlgoString() = 0;
	virtual QString GetInfoFile() = 0;
	virtual QWidget *GetParameterWidget() = 0;
	virtual void SetParams(Regressor *regressor) = 0;
	virtual void SaveOptions(QSettings &settings) = 0;
	virtual bool LoadOptions(QSettings &settings) = 0;
	virtual void SaveParams(QTextStream &stream) = 0;
	virtual bool LoadParams(QString name, float value) = 0;

	void Draw(Canvas *canvas, Regressor *regressor)
	{
		if(!regressor || !canvas) return;
		canvas->liveTrajectory.clear();
		int w = canvas->width();
		int h = canvas->height();

		{
			canvas->maps.confidence = QPixmap(w,h);
			canvas->maps.model = QPixmap(w,h);
			QBitmap bitmap(w,h);
			bitmap.clear();
			canvas->maps.model.setMask(bitmap);
			canvas->maps.model.fill(Qt::transparent);
            QPainter painter(&canvas->maps.model);
            if(!canvas->canvasType) DrawModel(canvas, painter, regressor);
		}

		{
			QPixmap infoPixmap(w, h);
			QBitmap bitmap(w,h);
			bitmap.clear();
			infoPixmap.setMask(bitmap);
			infoPixmap.fill(Qt::transparent);
			QPainter painter(&infoPixmap);
            if(!canvas->canvasType) DrawInfo(canvas, painter, regressor);
			canvas->maps.info = infoPixmap;
		}

		DrawConfidence(canvas, regressor);
		canvas->repaint();
	}

};

class DynamicalInterface
{
public:
	// virtual functions to manage the algorithm creation
	virtual Dynamical *GetDynamical() = 0;
	virtual void DrawInfo(Canvas *canvas, QPainter &painter, Dynamical *dynamical) = 0;
	virtual void DrawModel(Canvas *canvas, QPainter &painter, Dynamical *dynamical) = 0;
    virtual ~DynamicalInterface(){}

	// virtual functions to manage the GUI and I/O
	virtual QString GetName() = 0;
	virtual QString GetAlgoString() = 0;
	virtual QString GetInfoFile() = 0;
	virtual QWidget *GetParameterWidget() = 0;
	virtual void SetParams(Dynamical *dynamical) = 0;
	virtual void SaveOptions(QSettings &settings) = 0;
	virtual bool LoadOptions(QSettings &settings) = 0;
	virtual void SaveParams(QTextStream &stream) = 0;
	virtual bool LoadParams(QString name, float value) = 0;
	virtual bool UsesDrawTimer() = 0;
    virtual void SaveModel(QString filename, Dynamical *dynamical){}
    virtual bool LoadModel(QString filename, Dynamical *dynamical){return true;}

	void Draw(Canvas *canvas, Dynamical *dynamical)
	{
		if(!dynamical || !canvas) return;
		int w = canvas->width();
		int h = canvas->height();
		canvas->maps.confidence = QPixmap(w,h);

		{
            QPixmap modelPixmap(w, h);
            QBitmap bitmap(w,h);
            bitmap.clear();
            modelPixmap.setMask(bitmap);
            modelPixmap.fill(Qt::transparent);

            QPainter painter(&modelPixmap);
            if(!canvas->canvasType) DrawModel(canvas, painter, dynamical);
            canvas->maps.model = modelPixmap;
        }

		{
			QPixmap infoPixmap(w, h);
			QBitmap bitmap(w,h);
			bitmap.clear();
			infoPixmap.setMask(bitmap);
			infoPixmap.fill(Qt::transparent);

			QPainter painter(&infoPixmap);
            if(!canvas->canvasType) DrawInfo(canvas, painter, dynamical);
			canvas->maps.info = infoPixmap;
		}
		canvas->repaint();
	}

};

class AvoidanceInterface
{
public:
	virtual ObstacleAvoidance *GetObstacleAvoidance() = 0;
    virtual ~AvoidanceInterface(){}
    virtual QString GetName() = 0;
	virtual QString GetAlgoString() = 0;
	virtual QString GetInfoFile() = 0;
	virtual QWidget *GetParameterWidget() = 0;
	virtual void SetParams(ObstacleAvoidance *avoid) = 0;
	virtual void SaveOptions(QSettings &settings) = 0;
	virtual bool LoadOptions(QSettings &settings) = 0;
	virtual void SaveParams(QTextStream &stream) = 0;
	virtual bool LoadParams(QString name, float value) = 0;
};

class MaximizeInterface
{
public:
	// virtual functions to manage the algorithm creation
	virtual Maximizer *GetMaximizer() = 0;
    virtual ~MaximizeInterface(){}

	// virtual functions to manage the GUI and I/O
	virtual QString GetName() = 0;
	virtual QString GetAlgoString() = 0;
	virtual QString GetInfoFile() = 0;
	virtual QWidget *GetParameterWidget() = 0;
	virtual void SetParams(Maximizer *maximizer) = 0;
	virtual void SaveOptions(QSettings &settings) = 0;
	virtual bool LoadOptions(QSettings &settings) = 0;
	virtual void SaveParams(QTextStream &stream) = 0;
	virtual bool LoadParams(QString name, float value) = 0;
};

class ReinforcementInterface
{
public:
    // virtual functions to manage the algorithm creation
    virtual Reinforcement *GetReinforcement() = 0;
    virtual void DrawInfo(Canvas *canvas, QPainter &painter, Reinforcement *reinforcement) = 0;
    virtual void DrawModel(Canvas *canvas, QPainter &painter, Reinforcement *reinforcement) = 0;
    virtual ~ReinforcementInterface(){}

    // virtual functions to manage the GUI and I/O
    virtual QString GetName() = 0;
    virtual QString GetAlgoString() = 0;
    virtual QString GetInfoFile() = 0;
    virtual QWidget *GetParameterWidget() = 0;
    virtual void SetParams(Reinforcement *reinforcement) = 0;
    virtual void SaveOptions(QSettings &settings) = 0;
    virtual bool LoadOptions(QSettings &settings) = 0;
    virtual void SaveParams(QTextStream &stream) = 0;
    virtual bool LoadParams(QString name, float value) = 0;
    // drawing function
    void Draw(Canvas *canvas, Reinforcement *reinforcement)
    {
        if(!reinforcement || !canvas) return;
        canvas->liveTrajectory.clear();
        int w = canvas->width();
        int h = canvas->height();

        {
            canvas->maps.model = QPixmap(w,h);
            QBitmap bitmap(w,h);
            bitmap.clear();
            canvas->maps.model.setMask(bitmap);
            canvas->maps.model.fill(Qt::transparent);
            QPainter painter(&canvas->maps.model);
            if(!canvas->canvasType) DrawModel(canvas, painter, reinforcement);
        }

        {
            canvas->maps.info = QPixmap(w,h);
            QBitmap bitmap(w,h);
            bitmap.clear();
            canvas->maps.info.setMask(bitmap);
            canvas->maps.info.fill(Qt::transparent);
            QPainter painter(&canvas->maps.info);
            if(!canvas->canvasType) DrawInfo(canvas, painter, reinforcement);
        }
        canvas->maps.confidence = QPixmap();
        canvas->repaint();
    }
};

class ProjectorInterface
{
public:
    // virtual functions to manage the algorithm creation
    virtual Projector *GetProjector() = 0;
    virtual void DrawInfo(Canvas *canvas, QPainter &painter, Projector *projector) = 0;
    virtual void DrawModel(Canvas *canvas, QPainter &painter, Projector *projector) = 0;
    virtual ~ProjectorInterface(){}

    // virtual functions to manage the GUI and I/O
    virtual QString GetName() = 0;
    virtual QString GetAlgoString() = 0;
    virtual QString GetInfoFile() = 0;
    virtual QWidget *GetParameterWidget() = 0;
    virtual void SetParams(Projector *projector) = 0;
    virtual void SaveOptions(QSettings &settings) = 0;
    virtual bool LoadOptions(QSettings &settings) = 0;
    virtual void SaveParams(QTextStream &stream) = 0;
    virtual bool LoadParams(QString name, float value) = 0;

    void Draw(Canvas *canvas, Projector *projector)
    {
        if(!canvas || !projector) return;
        canvas->liveTrajectory.clear();
        int w = canvas->width();
        int h = canvas->height();
        {
            QPixmap modelPixmap(w, h);
            QBitmap bitmap(w,h);
            bitmap.clear();
            modelPixmap.setMask(bitmap);
            modelPixmap.fill(Qt::transparent);
            QPainter painter(&modelPixmap);
            DrawModel(canvas, painter, projector);
            canvas->maps.model = modelPixmap;
        }

        {
            QPixmap infoPixmap(w, h);
            QBitmap bitmap(w,h);
            bitmap.clear();
            infoPixmap.setMask(bitmap);
            infoPixmap.fill(Qt::transparent);
            QPainter painter(&infoPixmap);
            DrawInfo(canvas, painter, projector);
            canvas->maps.info = infoPixmap;
        }
        canvas->repaint();
    }
};

class CollectionInterface
{
protected:
	std::vector<ClassifierInterface *> classifiers;
	std::vector<ClustererInterface *> clusterers;
	std::vector<RegressorInterface *> regressors;
	std::vector<DynamicalInterface *> dynamicals;
    std::vector<MaximizeInterface*> maximizers;
    std::vector<ReinforcementInterface *> reinforcements;
    std::vector<ProjectorInterface*> projectors;

public:
	virtual QString GetName() = 0;

    std::vector<ClassifierInterface *> GetClassifiers() {return classifiers;}
    std::vector<ClustererInterface *> GetClusterers() {return clusterers;}
    std::vector<RegressorInterface *> GetRegressors() {return regressors;}
    std::vector<DynamicalInterface *> GetDynamicals() {return dynamicals;}
    std::vector<MaximizeInterface *> GetMaximizers() {return maximizers;}
    std::vector<ReinforcementInterface *> GetReinforcements() {return reinforcements;}
    std::vector<ProjectorInterface *> GetProjectors() {return projectors;}

    virtual ~CollectionInterface()
	{
		FOR(i, classifiers.size()) if(classifiers[i]) delete classifiers[i];
		FOR(i, clusterers.size()) if(clusterers[i]) delete clusterers[i];
		FOR(i, regressors.size()) if(regressors[i]) delete regressors[i];
		FOR(i, dynamicals.size()) if(dynamicals[i]) delete dynamicals[i];
        FOR(i, maximizers.size()) if(maximizers[i]) delete maximizers[i];
        FOR(i, reinforcements.size()) if(reinforcements[i]) delete reinforcements[i];
        FOR(i, projectors.size()) if(projectors[i]) delete projectors[i];
    }
};

class InputOutputInterface
{
public:
    virtual ~InputOutputInterface(){}
	// signatures for plugin slots and signals (SLOT() and SIGNAL() functions)
	virtual const char* QueryClassifierSignal() = 0; // void QueryClassifier(std::vector<fvec> samples);
	virtual const char* QueryRegressorSignal() = 0; // void QueryRegressor(std::vector<fvec> samples);
	virtual const char* QueryDynamicalSignal() = 0; // void QueryDynamical(std::vector<fvec> samples);
	virtual const char* QueryClustererSignal() = 0; // void QueryClusterer(std::vector<fvec> samples);
    virtual const char* QueryMaximizerSignal() = 0; // void QueryMaximizer(std::vector<fvec> samples);
    virtual const char* QueryReinforcementSignal() = 0; // void QueryReinforcement(std::vector<fvec> samples);
    virtual const char* SetDataSignal() = 0; // void SetData(std::vector<fvec> samples, ivec labels, std::vector<ipair> trajectories);
	virtual const char* SetTimeseriesSignal() = 0; // void SetTimeseriesSignal(std::vector<TimeSerie> series);
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
Q_DECLARE_INTERFACE(MaximizeInterface, "com.MLDemos.MaximizeInterface/1.0")
Q_DECLARE_INTERFACE(ReinforcementInterface, "com.MLDemos.ReinforcementInterface/1.0")
Q_DECLARE_INTERFACE(ProjectorInterface, "com.MLDemos.ProjectorInterface/1.0")
Q_DECLARE_INTERFACE(CollectionInterface, "com.MLDemos.CollectionInterface/1.0")
Q_DECLARE_INTERFACE(InputOutputInterface, "com.MLDemos.InputOutputInterface/1.0")

#endif // _INTERFACES_H_
