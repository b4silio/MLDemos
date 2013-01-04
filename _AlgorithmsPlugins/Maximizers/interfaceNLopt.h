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
#ifndef INTERFACENLOPT_H
#define INTERFACENLOPT_H

#include <QObject>
#include <interfaces.h>
#include "ui_paramsNLopt.h"

class MaximizeInterfaceNLopt : public QObject, public MaximizeInterface
{
    Q_OBJECT
    Q_INTERFACES(MaximizeInterface)

private:
    QWidget *widget;
    Ui::ParametersNLopt *params;
public:
    MaximizeInterfaceNLopt();
    Maximizer *GetMaximizer();

    QString GetName(){return QString("Gradient Free");}
    QString GetAlgoString();
    QString GetInfoFile(){return "maximizeNLopt.html";}
    QWidget *GetParameterWidget(){return widget;}
    void SetParams(Maximizer *maximizer);
    void SaveOptions(QSettings &settings);
    bool LoadOptions(QSettings &settings);
    void SaveParams(QTextStream &stream);
    bool LoadParams(QString name, float value);
    void SetParams(Maximizer *maximizer, fvec parameters);
    fvec GetParams();
    void GetParameterList(std::vector<QString> &parameterNames,
                                 std::vector<QString> &parameterTypes,
                                 std::vector< std::vector<QString> > &parameterValues);
};

#endif // INTERFACENLOPT_H
