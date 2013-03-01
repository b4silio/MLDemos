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
#include "interfaceSEDSDynamic.h"
#include "drawUtils.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>
#include <QDebug>

using namespace std;

DynamicSEDS::DynamicSEDS()
{
	params = new Ui::ParametersSEDS();
	params->setupUi(widget = new QWidget());
    connect(params->sedsConstraintCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(OptionsChanged()));
}

DynamicSEDS::~DynamicSEDS()
{
    delete params;
}

void DynamicSEDS::OptionsChanged()
{
	int constraintCriterion = params->sedsConstraintCombo->currentIndex();
	//params->minorIterationCount->setEnabled(constraintCriterion);
}

void DynamicSEDS::SetParams(Dynamical *dynamical)
{
	if(!dynamical) return;

	int clusters = params->sedsCount->value();
	bool bPrior = params->sedsCheckPrior->isChecked();
    bool bMu = params->sedsCheckMu->isChecked();
	bool bSigma = params->sedsCheckSigma->isChecked();
	int objectiveType = params->sedsObjectiveCombo->currentIndex();
    int optimizationType = params->sedsOptimizationCombo->currentIndex();
	int maxIteration = params->iterationCount->value();
	int constraintCriterion = params->sedsConstraintCombo->currentIndex();

    DynamicalSEDS *seds = dynamic_cast<DynamicalSEDS *>(dynamical);
    if(!seds) return;

    seds->SetParams(clusters, bPrior, bMu, bSigma, objectiveType, maxIteration, constraintCriterion, optimizationType);
    seds->displayLabel = params->graphLabel;

	//float penalty = params->sedsPenaltySpin->value();
	//int maxMinorIteration = params->minorIterationCount->value();
	//((DynamicalSEDS *)dynamical)->SetParams(clusters, penalty, bPrior, bMu, bSigma, objectiveType, maxIteration);
	//((DynamicalSEDS *)dynamical)->SetParams(clusters, penalty, bPrior, bMu, bSigma, objectiveType, maxIteration, maxMinorIteration, constraintCriterion);
}

fvec DynamicSEDS::GetParams()
{
    int clusters = params->sedsCount->value();
    bool bPrior = params->sedsCheckPrior->isChecked();
    bool bMu = params->sedsCheckMu->isChecked();
    bool bSigma = params->sedsCheckSigma->isChecked();
    int objectiveType = params->sedsObjectiveCombo->currentIndex();
//    int optimizationType = params->sedsOptimizationCombo->currentIndex();
    int maxIteration = params->iterationCount->value();
    int constraintCriterion = params->sedsConstraintCombo->currentIndex();

    int i=0;
    fvec par(7);
    par[i++] = clusters;
    par[i++] = bPrior;
    par[i++] = bMu;
    par[i++] = bSigma;
    par[i++] = objectiveType;
//    par[i++] = optimizationType;
    par[i++] = maxIteration;
    par[i++] = constraintCriterion;
    return par;
}

void DynamicSEDS::SetParams(Dynamical *dynamical, fvec parameters)
{
    if(!dynamical) return;
    int i=0;
    int clusters = parameters.size() > i ? parameters[i] : 1; i++;
    bool bPrior = parameters.size() > i ? parameters[i] : 1; i++;
    bool bMu = parameters.size() > i ? parameters[i] : 1; i++;
    bool bSigma = parameters.size() > i ? parameters[i] : 1; i++;
    int objectiveType = parameters.size() > i ? parameters[i] : 1; i++;
//    int optimizationType = parameters.size() > i ? parameters[i] : 1; i++;
    int optimizationType = 0;
    int maxIteration = parameters.size() > i ? parameters[i] : 1; i++;
    int constraintCriterion = parameters.size() > i ? parameters[i] : 1; i++;
    DynamicalSEDS *seds = dynamic_cast<DynamicalSEDS *>(dynamical);
    if(!seds) return;

    seds->SetParams(clusters, bPrior, bMu, bSigma, objectiveType, maxIteration, constraintCriterion, optimizationType);
    seds->displayLabel = params->graphLabel;
}

void DynamicSEDS::GetParameterList(std::vector<QString> &parameterNames,
                             std::vector<QString> &parameterTypes,
                             std::vector< std::vector<QString> > &parameterValues)
{
    parameterNames.clear();
    parameterTypes.clear();
    parameterValues.clear();
    parameterNames.push_back("Components");
    parameterNames.push_back("Optimize Prior");
    parameterNames.push_back("Optimize Mu");
    parameterNames.push_back("Optimize Sigma");
    parameterNames.push_back("Objective Function");
    parameterNames.push_back("Max Iterations");
    parameterNames.push_back("Constraint Criterion");
    parameterTypes.push_back("Integer");
    parameterTypes.push_back("List");
    parameterTypes.push_back("List");
    parameterTypes.push_back("List");
    parameterTypes.push_back("List");
    parameterTypes.push_back("Integer");
    parameterTypes.push_back("List");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("1");
    parameterValues.back().push_back("99999");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("False");
    parameterValues.back().push_back("True");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("False");
    parameterValues.back().push_back("True");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("False");
    parameterValues.back().push_back("True");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("False");
    parameterValues.back().push_back("True");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("MSE");
    parameterValues.back().push_back("Likelihood");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("1");
    parameterValues.back().push_back("99999");
}

Dynamical *DynamicSEDS::GetDynamical()
{
	DynamicalSEDS *dynamical = new DynamicalSEDS();
	SetParams(dynamical);
	return dynamical;
}

void DynamicSEDS::DrawInfo(Canvas *canvas, QPainter &painter, Dynamical *dynamical)
{
	if(!canvas || !dynamical) return;
	painter.setRenderHint(QPainter::Antialiasing);

	float resize = ((DynamicalSEDS*)dynamical)->resizeFactor;
    Gmm *gmm = ((DynamicalSEDS*)dynamical)->gmm;
    int xIndex = canvas->xIndex;
    int yIndex = canvas->yIndex;
    int dim = gmm->dim;
    float mean[2];
    float sigma[3];
    painter.setBrush(Qt::NoBrush);
    FOR(i, gmm->nstates)
    {
        float* bigSigma = new float[dim*dim];
        float* bigMean = new float[dim];
        gmm->getCovariance(i, bigSigma, false);
        sigma[0] = bigSigma[xIndex*dim + xIndex];
        sigma[1] = bigSigma[yIndex*dim + xIndex];
        sigma[2] = bigSigma[yIndex*dim + yIndex];
        gmm->getMean(i, bigMean);
        mean[0] = bigMean[xIndex];
        mean[1] = bigMean[yIndex];
        delete [] bigSigma;
        delete [] bigMean;

        fvec endpoint = ((DynamicalSEDS*)dynamical)->endpoint;
        FOR(j,2) mean[j] = mean[j]/resize + endpoint[j];
        FOR(j,3) sigma[j] /= resize*resize;

        painter.setPen(QPen(Qt::black, 1));
        DrawEllipse(mean, sigma, 1, &painter, canvas);
        painter.setPen(QPen(Qt::black, 0.5));
        DrawEllipse(mean, sigma, 2, &painter, canvas);
        QPointF point = canvas->toCanvasCoords(mean[0],mean[1]);
        painter.setPen(QPen(Qt::black, 4));
        painter.drawEllipse(point, 2, 2);
        painter.setPen(QPen(Qt::white, 2));
        painter.drawEllipse(point, 2, 2);
    }
}

void DynamicSEDS::SaveOptions(QSettings &settings)
{
	settings.setValue("sedsCount", params->sedsCount->value());
	//settings.setValue("sedsPenalty", params->sedsPenaltySpin->value());
    //settings.setValue("sedsObjective", params->sedsObjectiveCombo->currentIndex());
	settings.setValue("sedsPrior", params->sedsCheckPrior->isChecked());
	settings.setValue("sedsMu", params->sedsCheckMu->isChecked());
    settings.setValue("sedsSigma", params->sedsCheckSigma->isChecked());
    settings.setValue("sedsUniform", params->sedsCheckUniform->isChecked());
    settings.setValue("sedsConstraintCombo", params->sedsConstraintCombo->currentIndex());
    settings.setValue("sedsOptimizationCombo", params->sedsOptimizationCombo->currentIndex());
    settings.setValue("iterationCount", params->iterationCount->value());
	//settings.setValue("minorIterationCount", params->minorIterationCount->value());
}

bool DynamicSEDS::LoadOptions(QSettings &settings)
{
	if(settings.contains("sedsCount")) params->sedsCount->setValue(settings.value("sedsCount").toInt());
	//if(settings.contains("sedsPenalty")) params->sedsPenaltySpin->setValue(settings.value("sedsPenalty").toFloat());
    //if(settings.contains("sedsObjective")) params->sedsObjectiveCombo->setCurrentIndex(settings.value("sedsObjective").toInt());
	if(settings.contains("sedsPrior")) params->sedsCheckPrior->setChecked(settings.value("sedsPrior").toBool());
	if(settings.contains("sedsMu")) params->sedsCheckMu->setChecked(settings.value("sedsMu").toBool());
	if(settings.contains("sedsSigma")) params->sedsCheckSigma->setChecked(settings.value("sedsSigma").toBool());
    if(settings.contains("sedsUniform")) params->sedsCheckUniform->setChecked(settings.value("sedsUniform").toBool());
    if(settings.contains("sedsConstraintCombo")) params->sedsConstraintCombo->setCurrentIndex(settings.value("sedsConstraintCombo").toInt());
    if(settings.contains("sedsOptimizationCombo")) params->sedsOptimizationCombo->setCurrentIndex(settings.value("sedsOptimizationCombo").toInt());
    if(settings.contains("iterationCount")) params->iterationCount->setValue(settings.value("iterationCount").toInt());
	//if(settings.contains("minorIterationCount")) params->minorIterationCount->setValue(settings.value("minorIterationCount").toInt());
	return true;
}

void DynamicSEDS::SaveParams(QTextStream &file)
{
	file << "dynamicalOptions" << ":" << "sedsCount" << " " << params->sedsCount->value() << "\n";
	//file << "dynamicalOptions" << ":" << "sedsPenalty" << " " << params->sedsPenaltySpin->value() << "\n";
    //file << "dynamicalOptions" << ":" << "sedsObjective" << " " << params->sedsObjectiveCombo->currentIndex() << "\n";
	file << "dynamicalOptions" << ":" << "sedsPrior" << " " << params->sedsCheckPrior->isChecked() << "\n";
	file << "dynamicalOptions" << ":" << "sedsMu" << " " << params->sedsCheckMu->isChecked() << "\n";
	file << "dynamicalOptions" << ":" << "sedsSigma" << " " << params->sedsCheckSigma->isChecked() << "\n";
    file << "dynamicalOptions" << ":" << "sedsUniform" << " " << params->sedsCheckUniform->isChecked() << "\n";
    file << "dynamicalOptions" << ":" << "sedsConstraintCombo" << " " << params->sedsConstraintCombo->currentIndex() << "\n";
    file << "dynamicalOptions" << ":" << "sedsOptimizationCombo" << " " << params->sedsOptimizationCombo->currentIndex() << "\n";
    file << "dynamicalOptions" << ":" << "iterationCount" << " " << params->iterationCount->value() << "\n";
	//file << "dynamicalOptions" << ":" << "minorIterationCount" << " " << params->minorIterationCount->value() << "\n";
}

bool DynamicSEDS::LoadParams(QString name, float value)
{
	if(name.endsWith("sedsCount")) params->sedsCount->setValue((int)value);
	//if(name.endsWith("sedsPenalty")) params->sedsPenaltySpin->setValue(value);
    //if(name.endsWith("sedsObjective")) params->sedsObjectiveCombo->setCurrentIndex((int)value);
    if(name.endsWith("sedsPrior")) params->sedsCheckPrior->setChecked((int)value);
    if(name.endsWith("sedsMu")) params->sedsCheckMu->setChecked((int)value);
	if(name.endsWith("sedsSigma")) params->sedsCheckSigma->setChecked((int)value);
    if(name.endsWith("sedsUniform")) params->sedsCheckUniform->setChecked((int)value);
    if(name.endsWith("sedsConstraintCombo")) params->sedsConstraintCombo->setCurrentIndex((int)value);
    if(name.endsWith("sedsOptimizationCombo")) params->sedsOptimizationCombo->setCurrentIndex((int)value);
    if(name.endsWith("iterationCount")) params->iterationCount->setValue((int)value);
	//if(name.endsWith("minorIterationCount")) params->minorIterationCount->setValue((int)value);
	return true;
}

Q_EXPORT_PLUGIN2(mld_SEDS, DynamicSEDS)
