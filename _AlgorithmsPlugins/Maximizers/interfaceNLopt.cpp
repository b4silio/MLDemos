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
