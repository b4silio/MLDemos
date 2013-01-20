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
#ifndef _INTERFACEMVMCLASSIFIER_H_
#define _INTERFACEMVMCLASSIFIER_H_

#include <vector>
#include <interfaces.h>
#include "classifierMVM.h"
#include "ui_paramsMVM.h"

class ClassMVM : public QObject, public ClassifierInterface
{
    Q_OBJECT
    Q_INTERFACES(ClassifierInterface)
private:
    QWidget *widget;
    Ui::ParametersMVM *params;
    ivec indices;
    fvec alphas;
    ivec signs;
    std::vector<fvec> samples;
    ivec labels;
public:
    ClassMVM();
    // virtual functions to manage the algorithm creation
    Classifier *GetClassifier();
    void DrawModel(Canvas *canvas, QPainter &painter, Classifier *classifier);
    void DrawInfo(Canvas *canvas, QPainter &painter, Classifier *classifier);
    void DrawGL(Canvas *canvas, GLWidget *glw, Classifier *classifier){}

    // virtual functions to manage the GUI and I/O
    QString GetName(){return QString("Manual Vector Machine");}
    QString GetAlgoString();
    QString GetInfoFile(){return "mvm.html";}
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
    void ChangeOptions();
    void ChangeAlphas();
    void ChangeSample();
    void ClearAlphas();
    void SelectAlpha(int index);
    void SetSampleList(std::vector<fvec> samples, ivec labels);
};

#endif // _INTERFACEMVMCLASSIFIER_H_
