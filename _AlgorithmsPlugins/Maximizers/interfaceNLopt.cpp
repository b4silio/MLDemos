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
#include <QPixmap>
#include <QBitmap>
#include <QPainter>
#include <datasetManager.h>
#include <canvas.h>
#include "interfaceNLopt.h"
#include "maximizeNLopt.h"

using namespace std;

MaximizeInterfaceNLopt::MaximizeInterfaceNLopt()
{
    params = new Ui::ParametersNLopt();
    params->setupUi(widget = new QWidget());
}

void MaximizeInterfaceNLopt::SetParams(Maximizer *maximizer)
{
    if(!maximizer) return;
    MaximizeNlopt *nlopt = dynamic_cast<MaximizeNlopt*>(maximizer);
    if(!nlopt) return;
    int type = params->algorithmCombo->currentIndex();
    float step = params->stepSpin->value();
    nlopt->SetParams(type, step);
}

fvec MaximizeInterfaceNLopt::GetParams()
{
    int type = params->algorithmCombo->currentIndex();
    float step = params->stepSpin->value();

    fvec par(2);
    par[0] = type;
    par[1] = step;
    return par;
}

void MaximizeInterfaceNLopt::SetParams(Maximizer *maximizer, fvec parameters)
{
    int i=0;
    int type = parameters.size() > i ? parameters[i] : 0; i++;
    float step = parameters.size() > i ? parameters[i] : 0.1;

    MaximizeNlopt *nlopt = dynamic_cast<MaximizeNlopt*>(maximizer);
    if(!nlopt) return;
    nlopt->SetParams(type, step);
}

void MaximizeInterfaceNLopt::GetParameterList(std::vector<QString> &parameterNames,
                             std::vector<QString> &parameterTypes,
                             std::vector< std::vector<QString> > &parameterValues)
{
    parameterNames.push_back("Algorithm Type");
    parameterNames.push_back("Initial Stepsize");
    parameterTypes.push_back("List");
    parameterTypes.push_back("Real");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("Augmented Lagrangian");
    parameterValues.back().push_back("BOBYQUA");
    parameterValues.back().push_back("COBYLA");
    parameterValues.back().push_back("Nelder-Mead");
    parameterValues.back().push_back("NEWUOA");
    parameterValues.back().push_back("Praxis");
    parameterValues.back().push_back("SubPlex");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("0.0000001f");
    parameterValues.back().push_back("9999999.f");
}

Maximizer *MaximizeInterfaceNLopt::GetMaximizer()
{
    Maximizer *maximizer = new MaximizeNlopt();
    SetParams(maximizer);
    return maximizer;
}

QString MaximizeInterfaceNLopt::GetAlgoString()
{
    QString algo;
    switch(params->algorithmCombo->currentIndex())
    {
    case 0:
        algo += QString("AUGLAG");
        break;
    case 1:
        algo += QString("BOBYQA");
        break;
    case 2:
        algo += QString("COBYLA");
        break;
    case 3:
        algo += QString("NELDERMEAD");
        break;
    case 4:
        algo += QString("NEWUOA");
        break;
    case 5:
        algo += QString("PRAXIS");
        break;
    case 6:
        algo += QString("SBPLX");
        break;
    default:
        return QString("Gradient-Free method");
    }
    algo += QString(" %1").arg(params->stepSpin->value());
    return algo;
}

void MaximizeInterfaceNLopt::SaveOptions(QSettings &settings)
{
    settings.setValue("algorithmCombo", params->algorithmCombo->currentIndex());
}

bool MaximizeInterfaceNLopt::LoadOptions(QSettings &settings)
{
    if(settings.contains("algorithmCombo")) params->algorithmCombo->setCurrentIndex(settings.value("algorithmCombo").toInt());
    return true;
}

void MaximizeInterfaceNLopt::SaveParams(QTextStream &file)
{
    file << "maximizationOptions:" << "algorithmCombo" << " " << params->algorithmCombo->currentIndex() << "\n";
}

bool MaximizeInterfaceNLopt::LoadParams(QString name, float value)
{
    if(name.endsWith("algorithmCombo")) params->algorithmCombo->setCurrentIndex((int)value);
    return true;
}
