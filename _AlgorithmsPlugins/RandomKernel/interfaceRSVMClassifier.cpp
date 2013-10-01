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
#include "interfaceRSVMClassifier.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>
#include <QDebug>

using namespace std;

ClassRSVM::ClassRSVM()
{
    params = new Ui::Parameters();
    params->setupUi(widget = new QWidget());
    connect(params->randomTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeOptions()));
    connect(params->kernelTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeOptions()));

    ChangeOptions();
}

ClassRSVM::~ClassRSVM()
{
    delete params;
}

void ClassRSVM::ChangeOptions()
{
    int C = params->svmCSpin->value();
    params->svmCSpin->setRange(0.0001, 1.0);
    params->svmCSpin->setSingleStep(0.0001);
    params->svmCSpin->setDecimals(4);

    if(C > 1) params->svmCSpin->setValue(0.001);

    switch(params->randomTypeCombo->currentIndex())
    {
    case 0: // Fourier
        break;
    case 1: // Nystrom

        break;
    }
    params->kernelRankSpin->setVisible(true);
    params->kernelWidthSpin->setVisible(true);
}

void ClassRSVM::DisplayARDKernel()
{

}

QString ClassRSVM::GetAlgoString()
{
    double C = params->svmCSpin->value();
    int randomType = params->randomTypeCombo->currentIndex();
    int kernelRank = params->kernelRankSpin->value();
    int kernelType = params->kernelTypeCombo->currentIndex();
    float kernelWidith = params->kernelWidthSpin->value();

    QString algo;
    algo += "Rand SVM";
    switch(randomType)
    {
    case RANDOM_FOURIER:
        algo += " Rank";
        algo += QString(" %1").arg(kernelRank);
        algo += " Fourier";
        algo += QString(" %1").arg(C);
        break;
    case RANDOM_NYSTROM:
        algo += QString(" Nystrom %1").arg(kernelRank);
        break;
    }

    switch(kernelType)
    {
    case RAND_KERNEL_RBF:
        algo += " RBF";
        algo += QString(" %1").arg(kernelWidith);
        break;
    }

    return algo;
}

void ClassRSVM::SetParams(Classifier *classifier)
{
    if(!classifier)
    {
        return;
    }
    //assign parameters to classifier
    int nRandomType = params->randomTypeCombo->currentIndex();
    int nKernelType = params->kernelTypeCombo->currentIndex();
    float fSvmC = params->svmCSpin->value();
    float kernelGamma = params->kernelWidthSpin->value();
    int nRandomRank = params->kernelRankSpin->value();

    ClassifierRSVM *rsvm = dynamic_cast<ClassifierRSVM *>(classifier);
    if(rsvm)
    {
        switch(nRandomType)
        {
        case 0:
            rsvm->kernelParms.eRandFeatureType = RANDOM_FOURIER;

            break;
        case 1:
            rsvm->kernelParms.eRandFeatureType = RANDOM_NYSTROM;

            break;
        }
        switch(nKernelType)
        {
        case 0:
            rsvm->kernelParms.eRandKernelType = RAND_KERNEL_RBF;
        }

        rsvm->kernelParms.nFeatureDim = nRandomRank;
        rsvm->kernelParms.fGamma = 1 / kernelGamma;
        rsvm->param.C = fSvmC;
    }

    return;
}

fvec ClassRSVM::GetParams()
{
    fvec par(4);
    int nRandomType = params->randomTypeCombo->currentIndex();
    int nKernelType = params->kernelTypeCombo->currentIndex();
    float fSvmC = params->svmCSpin->value();
    float kernelGamma = params->kernelWidthSpin->value();
    int nRandomRank = params->kernelRankSpin->value();

    par[0] = fSvmC;
    par[1] = nKernelType;
    par[2] = nRandomRank;
    par[3] = kernelGamma;

    return par;
}

void ClassRSVM::SetParams(Classifier *classifier, fvec parameters)
{
    if(!classifier) return;
    int nRandFeatType = params->randomTypeCombo->currentIndex();
    ClassifierRSVM *rsvm = dynamic_cast<ClassifierRSVM*>(classifier);
    if(rsvm)
    {
        rsvm->param.C = parameters[0];
        rsvm->kernelParms.eRandFeatureType = nRandFeatType;
        rsvm->kernelParms.eRandKernelType = parameters[1];
        rsvm->kernelParms.nFeatureDim = parameters[2];
        rsvm->kernelParms.fGamma = parameters[3];
    }
    return;
}

void ClassRSVM::GetParameterList(std::vector<QString> &parameterNames,
                                std::vector<QString> &parameterTypes,
                                std::vector< std::vector<QString> > &parameterValues)
{
    return;
}

Classifier *ClassRSVM::GetClassifier()
{
    Classifier *classifier = NULL;
    classifier = new ClassifierRSVM();
    SetParams(classifier);
    return classifier;
}

void ClassRSVM::DrawInfo(Canvas *canvas, QPainter &painter, Classifier *classifier)
{
    /*
    ardKernel.clear();
    ardNames.clear();
    painter.setRenderHint(QPainter::Antialiasing);
    ClassifierRSVM* classifier_rsvm = NULL;
    if(classifier_rsvm = dynamic_cast<ClassifierRSVM*>(classifier))
    {

    }
    */
    //NOT_IMPLEMENTED_YET
    return;
}

void ClassRSVM::DrawGL(Canvas *canvas, GLWidget *glw, Classifier *classifier)
{
    return;
}

void ClassRSVM::SaveOptions(QSettings &settings)
{
    settings.setValue("randomType", params->randomTypeCombo->currentIndex());
    settings.setValue("kernelType", params->kernelTypeCombo->currentIndex());
    settings.setValue("kernelWidth", params->kernelWidthSpin->value());
    settings.setValue("kernelRank", params->kernelRankSpin->value());
    settings.setValue("svmC", params->svmCSpin->value());
    return;
}

bool ClassRSVM::LoadOptions(QSettings &settings)
{
    if(settings.contains("randomType"))
        params->randomTypeCombo->setCurrentIndex(settings.value("randomType").toInt());
    if(settings.contains("kernelType"))
        params->kernelTypeCombo->setCurrentIndex(settings.value("kernelType").toInt());
    if(settings.contains("kernelWidth"))
        params->kernelWidthSpin->setValue(settings.value("kernelWidth").toFloat());
    if(settings.contains("kernelRank"))
        params->kernelRankSpin->setValue(settings.value("kernelRank").toInt());
    if(settings.contains("svmC"))
        params->svmCSpin->setValue(settings.value("svmC").toFloat());
    return true;
}

void ClassRSVM::SaveParams(QTextStream &file)
{
    file << "classificationOptions" << ":" << "randomType" << params->randomTypeCombo->currentIndex() << "\n";
    file << "classificationOptions" << ":" << "kernelType" << params->kernelTypeCombo->currentIndex() << "\n";
    file << "classificationOptions" << ":" << "kernelWidth" << params->kernelWidthSpin->value() << "\n";
    file << "classificationOptions" << ":" << "kernelRank" << params->kernelRankSpin->value() << "\n";
    file << "classificationOptions" << ":" << "svmC" << params->svmCSpin->value() << "\n";
    return;
}

bool ClassRSVM::LoadParams(QString name, float value)
{
    if(name.endsWith("randomType"))
        params->randomTypeCombo->setCurrentIndex((int) value);
    if(name.endsWith("kernelType"))
        params->kernelTypeCombo->setCurrentIndex((int) value);
    if(name.endsWith("kernelWidth"))
        params->kernelWidthSpin->setValue(value);
    if(name.endsWith("kernelRank"))
        params->kernelRankSpin->setValue((int) value);
    if(name.endsWith("svmC"))
        params->svmCSpin->setValue(value);
    return true;
}
