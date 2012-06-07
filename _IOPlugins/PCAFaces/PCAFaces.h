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
#ifndef _INTERFACEPCAFACES_H_
#define _INTERFACEPCAFACES_H_

#include <vector>
#include <interfaces.h>
#include <QTimerEvent>
#include "ui_PCAFaces.h"
#include "pcaprojector.h"

class PCAFaces : public QObject, public InputOutputInterface
{
	Q_OBJECT
	Q_INTERFACES(InputOutputInterface)
public:
	const char* QueryClassifierSignal() {return SIGNAL(QueryClassifier(std::vector<fvec>));}
	const char* QueryRegressorSignal() {return SIGNAL(QueryRegressor(std::vector<fvec>));}
	const char* QueryDynamicalSignal() {return SIGNAL(QueryDynamical(std::vector<fvec>));}
	const char* QueryClustererSignal() {return SIGNAL(QueryClusterer(std::vector<fvec>));}
    const char* QueryMaximizerSignal() {return SIGNAL(QueryMaximizer(std::vector<fvec>));}
    const char* QueryReinforcementSignal() {return SIGNAL(QueryReinforcement(std::vector<fvec>));}
    const char* SetDataSignal() {return SIGNAL(SetData(std::vector<fvec>, ivec, std::vector<ipair>, bool));}
	const char* SetTimeseriesSignal() {return SIGNAL(SetTimeseries(std::vector<TimeSerie>));}
	const char* FetchResultsSlot() {return SLOT(FetchResults(std::vector<fvec>));}
	const char* DoneSignal() {return SIGNAL(Done(QObject *));}
    QObject *object(){return this;}
    QString GetName(){return "PCA Faces";}

	void Start();
	void Stop();

	Ui::PCAFacesDialog *gui;
	QDialog *guiDialog;
    PCAProjector *projector;
	PCAFaces();
	~PCAFaces();
signals:
	void Done(QObject *);
    void SetData(std::vector<fvec> samples, ivec labels, std::vector<ipair> trajectories, bool bProjected);
	void SetTimeseries(std::vector<TimeSerie> series);
	void QueryClassifier(std::vector<fvec> samples);
	void QueryRegressor(std::vector<fvec> samples);
	void QueryDynamical(std::vector<fvec> samples);
	void QueryClusterer(std::vector<fvec> samples);
    void QueryMaximizer(std::vector<fvec> samples);
    void QueryReinforcement(std::vector<fvec> samples);
public slots:
	void FetchResults(std::vector<fvec> results);
	void Closing();
	void Updating();
};

#endif // _INTERFACEPCAFACES_H_

