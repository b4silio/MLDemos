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
#include "interfaceMRVMClassifier.h"
#include "classifierMRVM.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>
#include <QDebug>

using namespace std;

ClassMRVM::ClassMRVM()
{
    params = new Ui::ParametersMRVM();
    params->setupUi(widget = new QWidget());
    ardLabel = 0;
    connect(params->svmTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeOptions()));
    connect(params->optimizeCheck, SIGNAL(clicked()), this, SLOT(ChangeOptions()));
    ChangeOptions();
}

ClassMRVM::~ClassMRVM()
{
    delete params;
    DEL(ardLabel);
}

void ClassMRVM::ChangeOptions()
{
    int C = params->svmCSpin->value();
    params->svmCSpin->setRange(0.0001, 1.0);
    params->svmCSpin->setSingleStep(0.0001);
    params->svmCSpin->setDecimals(4);
    params->optimizeCheck->setVisible(true);
    if(C > 1) params->svmCSpin->setValue(0.001);
    switch(params->svmTypeCombo->currentIndex())
    {
    case 0: // C-SVM
        params->svmCSpin->setRange(0.1, 9999);
        params->svmCSpin->setSingleStep(1);
        params->svmCSpin->setDecimals(1);
        params->svmCSpin->setValue(C);
        if(params->svmCSpin->value() < 1) params->svmCSpin->setValue(100);
        params->svmTypeLabel->setText("C");
        break;
    case 1: // Nu-SVM
        params->svmTypeLabel->setText("Nu");
        break;
    }
}


QString ClassMRVM::GetAlgoString()
{
    double C = params->svmCSpin->value();
    float kernelGamma = params->kernelWidthSpin->value();
    bool bOptimize = params->optimizeCheck->isChecked();

    QString algo;
    algo += "MRVM ";
    switch(params->svmTypeCombo->currentIndex())
    {
    case 0: // C-SVM
        algo += "C";
        algo += QString(" %1").arg(C);
        break;
    case 1: // Nu-SVM
        algo += "Nu";
        algo += QString(" %1").arg(C);
        break;
    }
    algo += QString(" RBF %1").arg(kernelGamma);
    if(bOptimize) algo += QString(" Opt");
    return algo;
}

void ClassMRVM::SetParams(Classifier *classifier)
{
    if(!classifier) return;
    int svmType = params->svmTypeCombo->currentIndex();
    float svmC = params->svmCSpin->value();
    float kernelGamma = params->kernelWidthSpin->value();
    bool bOptimize = params->optimizeCheck->isChecked();

    ClassifierMRVM *svm = dynamic_cast<ClassifierMRVM *>(classifier);
    if(svm)
    {
        switch(svmType)
        {
        case 0:
            svm->param.svm_type = C_SVC;
            break;
        case 1:
            svm->param.svm_type = NU_SVC;
            break;
        }
        svm->param.kernel_type = RBF;
        svm->param.C = svm->param.nu = svmC;
        svm->param.gamma = 1 / kernelGamma;
        svm->param.coef0 = 0;
        svm->bOptimize = bOptimize;
    }
}

fvec ClassMRVM::GetParams()
{
    int svmType = params->svmTypeCombo->currentIndex();
    float svmC = params->svmCSpin->value();
    float kernelGamma = params->kernelWidthSpin->value();
    bool bOptimize = params->optimizeCheck->isChecked();

    fvec par(4);
    par[0] = svmType;
    par[1] = svmC;
    par[2] = kernelGamma;
    par[3] = bOptimize;
    return par;
}

void ClassMRVM::SetParams(Classifier *classifier, fvec parameters)
{
    if(!classifier) return;
    int svmType = params->svmTypeCombo->currentIndex();
    float svmC, kernelGamma;
    bool bOptimize;

    svmType = parameters.size() > 0 ? parameters[0] : 0;
    svmC = parameters.size() > 1 ? parameters[1] : 1;
    kernelGamma = parameters.size() > 2 ? parameters[2] : 0.1;
    bOptimize = parameters.size() > 3 ? parameters[3] : 0;

    ClassifierMRVM *svm = dynamic_cast<ClassifierMRVM *>(classifier);
    if(svm)
    {
        switch(svmType)
        {
        case 0:
            svm->param.svm_type = C_SVC;
            break;
        case 1:
            svm->param.svm_type = NU_SVC;
            break;
        }
        svm->param.kernel_type = RBF;
        svm->param.C = svm->param.nu = svmC;
        svm->param.gamma = 1. / kernelGamma;
        svm->param.coef0 = 0;
        svm->bOptimize = bOptimize;
    }
}

void ClassMRVM::GetParameterList(std::vector<QString> &parameterNames,
                                std::vector<QString> &parameterTypes,
                                std::vector< std::vector<QString> > &parameterValues)
{
    parameterNames.push_back("SVM Type");
    parameterNames.push_back("Penalty (C) / Nu");
    parameterNames.push_back("Kernel Width");
    parameterNames.push_back("Optimize Kernel");
    parameterTypes.push_back("List");
    parameterTypes.push_back("Real");
    parameterTypes.push_back("Real");
    parameterTypes.push_back("List");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("Epsilon-SVM");
    parameterValues.back().push_back("Nu-SVM");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("1");
    parameterValues.back().push_back("999999999");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("0.00000001f");
    parameterValues.back().push_back("9999999");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("False");
    parameterValues.back().push_back("True");
}

Classifier *ClassMRVM::GetClassifier()
{
    Classifier *classifier = new ClassifierMRVM();
    SetParams(classifier);
    ardKernel.clear();
    ardNames.clear();
    return classifier;
}

void ClassMRVM::DrawInfo(Canvas *canvas, QPainter &painter, Classifier *classifier)
{
    ardKernel.clear();
    ardNames.clear();
    painter.setRenderHint(QPainter::Antialiasing);
    if(dynamic_cast<ClassifierMRVM*>(classifier))
    {
        int dim = canvas->data->GetDimCount();
        // we want to draw the support vectors
        ClassifierMRVM *theSVM = dynamic_cast<ClassifierMRVM*>(classifier);
        svm_model *svm = theSVM->GetModel();
        MRVM *mrvm = &theSVM->mrvm;
        fvec centerSample = canvas->toSampleCoords(canvas->width()/2, canvas->height()/2);
        fvec centerPlusSample = canvas->toSampleCoords(canvas->width()/2 + 1, canvas->height()/2);
        float pixelSize = sqrtf((centerSample-centerPlusSample)*(centerSample-centerPlusSample));
        if(svm)
        {
            if(svm->param.kernel_type == RBFWEIGH)
            {
                ardKernel.resize(svm->param.kernel_dim,1.f);
                FOR(d, svm->param.kernel_dim) ardKernel[d] = svm->param.kernel_weight[d];
                ardNames = canvas->dimNames;
                //if(ardLabel && ardLabel->isVisible()) DisplayARDKernel();
            }
            fvec sample(dim,0);
            FOR(i, svm->l)
            {
                if(canvas->sourceDims.size())
                {
                    FOR(d, canvas->sourceDims.size()) sample[canvas->sourceDims[d]] = svm->SV[i][d].value;
                }
                else
                {
                    FOR(d, dim) sample[d] = (f32)svm->SV[i][d].value;
                }
                int radius = 9;
                QPointF point = canvas->toCanvasCoords(sample);
                if((*svm->sv_coef)[i] < 0 != theSVM->bInvert)
                {
                    painter.setPen(QPen(Qt::black, 6));
                    painter.drawEllipse(point, radius, radius);
                    painter.setPen(QPen(Qt::white,4));
                    painter.drawEllipse(point, radius, radius);
                }
                else
                {
                    painter.setPen(QPen(Qt::black, 6));
                    painter.drawEllipse(point, radius, radius);
                    painter.setPen(QPen(Qt::red,4));
                    painter.drawEllipse(point, radius, radius);
                }
                float r = mrvm->betas[i]/mrvm->gamma/pixelSize;
                painter.setPen(QPen(Qt::black, 2, Qt::DotLine));
                painter.drawEllipse(point, r, r);
                painter.setPen(QPen(Qt::black, 1, Qt::DotLine));
                painter.drawEllipse(point, r*2, r*2);
            }
        }
    }
}

void ClassMRVM::DrawGL(Canvas *canvas, GLWidget *glw, Classifier *classifier)
{
    int xInd = canvas->xIndex;
    int yInd = canvas->yIndex;
    int zInd = canvas->zIndex;
    GLObject o;
    o.objectType = "Samples";
    o.style = "rings,pointsize:24";
    vector<fvec> svs;
    int dim = canvas->data->GetDimCount();
    if(dynamic_cast<ClassifierMRVM*>(classifier))
    {
        // we want to draw the support vectors
        svm_model *svm = dynamic_cast<ClassifierMRVM*>(classifier)->GetModel();
        if(svm)
        {
            fvec sv(dim);
            FOR(i, svm->l)
            {
                FOR(d, dim) sv[d] = svm->SV[i][d].value;
                svs.push_back(sv);
            }
        }
    }
    FOR(i, svs.size())
    {
        o.vertices.append(QVector3D(svs[i][xInd],svs[i][yInd], zInd>=0 && zInd<dim ? svs[i][zInd] : 0));
        o.colors.append(QVector4D(0,0,0,1));
    }
    glw->mutex->lock();
    glw->AddObject(o);
    glw->mutex->unlock();
}

void ClassMRVM::SaveOptions(QSettings &settings)
{
    settings.setValue("kernelWidth", params->kernelWidthSpin->value());
    settings.setValue("svmC", params->svmCSpin->value());
    settings.setValue("svmType", params->svmTypeCombo->currentIndex());
    settings.setValue("optimizeCheck", params->optimizeCheck->isChecked());
}

bool ClassMRVM::LoadOptions(QSettings &settings)
{
    if(settings.contains("kernelWidth")) params->kernelWidthSpin->setValue(settings.value("kernelWidth").toFloat());
    if(settings.contains("svmC")) params->svmCSpin->setValue(settings.value("svmC").toFloat());
    if(settings.contains("svmType")) params->svmTypeCombo->setCurrentIndex(settings.value("svmType").toInt());
    if(settings.contains("optimizeCheck")) params->optimizeCheck->setChecked(settings.value("optimizeCheck").toInt());
    ChangeOptions();
    return true;
}

void ClassMRVM::SaveParams(QTextStream &file)
{
    file << "classificationOptions" << ":" << "kernelWidth" << " " << params->kernelWidthSpin->value() << "\n";
    file << "classificationOptions" << ":" << "svmC" << " " << params->svmCSpin->value() << "\n";
    file << "classificationOptions" << ":" << "svmType" << " " << params->svmTypeCombo->currentIndex() << "\n";
    file << "classificationOptions" << ":" << "optimizeCheck" << " " << params->optimizeCheck->isChecked() << "\n";
}

bool ClassMRVM::LoadParams(QString name, float value)
{
    if(name.endsWith("kernelWidth")) params->kernelWidthSpin->setValue(value);
    if(name.endsWith("svmC")) params->svmCSpin->setValue(value);
    if(name.endsWith("svmType")) params->svmTypeCombo->setCurrentIndex((int)value);
    if(name.endsWith("optimizeCheck")) params->optimizeCheck->setChecked((int)value);
    ChangeOptions();
    return true;
}
