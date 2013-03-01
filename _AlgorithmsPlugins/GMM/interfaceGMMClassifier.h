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
#ifndef _INTERFACEGMMCLASSIFIER_H_
#define _INTERFACEGMMCLASSIFIER_H_

#include <vector>
#include <interfaces.h>
#include "classifierGMM.h"
#include "ui_paramsGMM.h"
#include "marginalwidget.h"

class ClassGMM : public QObject, public ClassifierInterface
{
	Q_OBJECT
	Q_INTERFACES(ClassifierInterface)
private:
	Ui::ParametersGMM *params;
    QWidget *widget;
    MarginalWidget *marginalWidget;
public:
    ClassGMM();
    ~ClassGMM();
    // virtual functions to manage the algorithm creation
	Classifier *GetClassifier();
	void DrawInfo(Canvas *canvas, QPainter &painter, Classifier *classifier);
    void DrawGL(Canvas *canvas, GLWidget *glw, Classifier *classifier);

	// virtual functions to manage the GUI and I/O
    QString GetName(){return QString("Gaussian Mixture Model");}
	QString GetAlgoString();
    QString GetInfoFile(){return "gmm.html";}
    bool UsesDrawTimer(){return true;}
    QWidget *GetParameterWidget(){return widget;}
	void SetParams(Classifier *classifier);
	void SaveOptions(QSettings &settings);
	bool LoadOptions(QSettings &settings);
	void SaveParams(QTextStream &stream);
	bool LoadParams(QString name, float value);

    void SetParams(Classifier *classifier, fvec parameters);
    fvec GetParams();
    void GetParameterList(std::vector<QString> &parameterNames,
                                 std::vector<QString> &parameterTypes,
                                 std::vector< std::vector<QString> > &parameterValues);

public slots:
    void ShowMarginals();
};

#endif // _INTERFACEGMMCLASSIFIER_H_
