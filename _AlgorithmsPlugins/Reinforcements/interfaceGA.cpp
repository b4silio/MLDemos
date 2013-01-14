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
#include "interfaceGA.h"
#include "reinforcementGA.h"
#include <QPixmap>
#include <QDebug>
#include <QBitmap>
#include <QPainter>
#include <datasetManager.h>
#include <canvas.h>

using namespace std;

ReinforcementInterfaceGA::ReinforcementInterfaceGA()
{
	params = new Ui::ParametersGA();
	params->setupUi(widget = new QWidget());
}

void ReinforcementInterfaceGA::SetParams(Reinforcement *reinforcement)
{
    if(!reinforcement) return;
	double mutation = params->mutationSpin->value();
	double cross = params->crossSpin->value();
	double survival = params->survivalSpin->value();
	int population = params->populationSpin->value();
    ((ReinforcementGA *)reinforcement)->SetParams(mutation, cross, survival, population);
}

fvec ReinforcementInterfaceGA::GetParams()
{
    double mutation = params->mutationSpin->value();
    double cross = params->crossSpin->value();
    double survival = params->survivalSpin->value();
    int population = params->populationSpin->value();

    fvec par(4);
    par[0] = mutation;
    par[1] = cross;
    par[2] = survival;
    par[3] = population;
    return par;
}

void ReinforcementInterfaceGA::SetParams(Reinforcement *reinforcement, fvec parameters)
{
    int i=0;
    double mutation = parameters.size() > i ? parameters[i] : 0; i++;
    double cross = parameters.size() > i ? parameters[i] : 0.1;
    double survival = parameters.size() > i ? parameters[i] : 10;
    int population = parameters.size() > i ? parameters[i] : false;

    ((ReinforcementGA *)reinforcement)->SetParams(mutation, cross, survival, population);
}

void ReinforcementInterfaceGA::GetParameterList(std::vector<QString> &parameterNames,
                             std::vector<QString> &parameterTypes,
                             std::vector< std::vector<QString> > &parameterValues)
{
    parameterNames.push_back("Mutation Rate");
    parameterNames.push_back("Crossover Rate");
    parameterNames.push_back("Survival Rate");
    parameterNames.push_back("Population Size");
    parameterTypes.push_back("Real");
    parameterTypes.push_back("Real");
    parameterTypes.push_back("Real");
    parameterTypes.push_back("Integer");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("0.0000001f");
    parameterValues.back().push_back("1.f");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("0.0000001f");
    parameterValues.back().push_back("1.f");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("0.0000001f");
    parameterValues.back().push_back("1.f");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("1");
    parameterValues.back().push_back("999999");
}

QString ReinforcementInterfaceGA::GetAlgoString()
{
	double mutation = params->mutationSpin->value();
	double cross = params->crossSpin->value();
	double survival = params->survivalSpin->value();
	int population = params->populationSpin->value();
	QString	algo = QString("GA %1 %2 %3 %4").arg(population).arg(mutation).arg(cross).arg(survival);
	return algo;
}

Reinforcement *ReinforcementInterfaceGA::GetReinforcement()
{
    Reinforcement *reinforcement = new ReinforcementGA();
    SetParams(reinforcement);
    return reinforcement;
}

void ReinforcementInterfaceGA::SaveOptions(QSettings &settings)
{
	settings.setValue("populationSpin", params->populationSpin->value());
	settings.setValue("mutationSpin", params->mutationSpin->value());
	settings.setValue("crossSpin", params->crossSpin->value());
	settings.setValue("survivalSpin", params->survivalSpin->value());
}

bool ReinforcementInterfaceGA::LoadOptions(QSettings &settings)
{
	if(settings.contains("populationSpin")) params->populationSpin->setValue(settings.value("populationSpin").toFloat());
	if(settings.contains("mutationSpin")) params->mutationSpin->setValue(settings.value("mutationSpin").toFloat());
	if(settings.contains("crossSpin")) params->crossSpin->setValue(settings.value("crossSpin").toFloat());
	if(settings.contains("survivalSpin")) params->survivalSpin->setValue(settings.value("survivalSpin").toFloat());
	return true;
}

void ReinforcementInterfaceGA::SaveParams(QTextStream &file)
{
	file << "maximizationOptions" << ":" << "populationSpin" << " " << params->populationSpin->value() << "\n";
	file << "maximizationOptions" << ":" << "mutationSpin" << " " << params->mutationSpin->value() << "\n";
	file << "maximizationOptions" << ":" << "crossSpin" << " " << params->crossSpin->value() << "\n";
	file << "maximizationOptions" << ":" << "survivalSpin" << " " << params->survivalSpin->value() << "\n";
}

bool ReinforcementInterfaceGA::LoadParams(QString name, float value)
{
	if(name.endsWith("populationSpin")) params->populationSpin->setValue((int)value);
	if(name.endsWith("mutationSpin")) params->mutationSpin->setValue((float)value);
	if(name.endsWith("crossSpin")) params->crossSpin->setValue((float)value);
	if(name.endsWith("survivalSpin")) params->survivalSpin->setValue((float)value);
	return true;
}
