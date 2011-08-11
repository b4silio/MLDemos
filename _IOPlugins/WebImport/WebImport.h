/*********************************************************************
MLDemos: A User-Friendly visualization toolkit for machine learning
Copyright (C) 2011 Chrstophe Paccolat
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
// http://stackoverflow.com/users/14065/martin
// http://stackoverflow.com/questions/1120140/csv-parser-in-c
// http://www.gamedev.net/topic/603211-get-type-of-string-in-c/
// http://www.gamedev.net/user/78572-rip-off/


#ifndef WEBIMPORT_H_INCLUDED
#define WEBIMPORT_H_INCLUDED

//using namespace std;

#include "parser.h"
#include <interfaces.h>
//#include <QTimerEvent>
#include "ui_WebImport.h"
//#include "public.h"
//#include "widget.h"
//#include <QMutex>
//#include <QMutexLocker>
//#include <QApplication>

 #include <QFileDialog>

class WebImport : public QObject, public InputOutputInterface
{
	Q_OBJECT
	Q_INTERFACES(InputOutputInterface)

public:
	const char* QueryClassifierSignal() {return SIGNAL(QueryClassifier(std::vector<fvec>));}
	const char* QueryRegressorSignal() {return SIGNAL(QueryRegressor(std::vector<fvec>));}
	const char* QueryDynamicalSignal() {return SIGNAL(QueryDynamical(std::vector<fvec>));}
	const char* QueryClustererSignal() {return SIGNAL(QueryClusterer(std::vector<fvec>));}
	const char* QueryMaximizerSignal() {return SIGNAL(QueryMaximizer(std::vector<fvec>));}
	const char* SetDataSignal() {return SIGNAL(SetData(std::vector<fvec>, ivec, std::vector<ipair>));}
	const char* FetchResultsSlot() {return SLOT(FetchResults(std::vector<fvec>));}
	const char* DoneSignal() {return SIGNAL(Done(QObject *));}
	QObject *object(){return this;};
	QString GetName(){return "WebImport";};

	void Start();
	void Stop();

	Ui::WebImportDialog *gui;
	QDialog *guiDialog;
        CSVParser *inputParser;
	WebImport();
	~WebImport();
signals:
	void Done(QObject *);
	void SetData(std::vector<fvec> samples, ivec labels, std::vector<ipair> trajectories);
	void QueryClassifier(std::vector<fvec> samples);
	void QueryRegressor(std::vector<fvec> samples);
	void QueryDynamical(std::vector<fvec> samples);
	void QueryClusterer(std::vector<fvec> samples);
	void QueryMaximizer(std::vector<fvec> samples);
public slots:
	void FetchResults(std::vector<fvec> results);
	void Closing();
        void Updating();
private slots:
        void on_loadImgButton_released();
        void on_loadImgButton_clicked();
};


#endif // WEBIMPORT_H_INCLUDED
