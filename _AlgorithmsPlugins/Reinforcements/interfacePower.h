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
#ifndef _INTERFACE_POWER_H_
#define _INTERFACE_POWER_H_

#include <vector>
#include <interfaces.h>
#include "ui_paramsPower.h"
#include <QDir>

class ReinforcementInterfacePower : public QObject, public ReinforcementInterface
{
	Q_OBJECT
    Q_INTERFACES(ReinforcementInterface)
private:
	QWidget *widget;
    Ui::ParametersPower *params;
public:
    ReinforcementInterfacePower();
	// virtual functions to manage the algorithm creation
    Reinforcement *GetReinforcement();

	// virtual functions to manage the GUI and I/O
    void DrawInfo(Canvas *canvas, QPainter &painter, Reinforcement *reinforcement){}
    void DrawModel(Canvas *canvas, QPainter &painter, Reinforcement *reinforcement){}
    void DrawGL(Canvas *canvas, GLWidget *glw, Reinforcement*reinforcement){}

    QString GetName(){return QString("POWER");}
	QString GetAlgoString();
    QString GetInfoFile(){return "RL.html";}
    QWidget *GetParameterWidget(){return widget;}
    void SetParams(Reinforcement *reinforcement);
	void SaveOptions(QSettings &settings);
	bool LoadOptions(QSettings &settings);
	void SaveParams(QTextStream &stream);
	bool LoadParams(QString name, float value);
    void SetParams(Reinforcement *reinforcement, fvec parameters);
    fvec GetParams();
    void GetParameterList(std::vector<QString> &parameterNames,
                                 std::vector<QString> &parameterTypes,
                                 std::vector< std::vector<QString> > &parameterValues);
public slots:
	void ChangeOptions();
};

#endif // _INTERFACE_POWER_H_
