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
#include "interfaceSVMClassifier.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>
#include <QDebug>

using namespace std;

ClassSVM::ClassSVM()
{
    params = new Ui::Parameters();
    params->setupUi(widget = new QWidget());
    connect(params->svmTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeOptions()));
    connect(params->kernelTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeOptions()));
    ChangeOptions();
}

void ClassSVM::ChangeOptions()
{
    int C = params->svmCSpin->value();
    params->maxSVSpin->setVisible(false);
    params->labelMaxSV->setVisible(false);
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
        if(params->kernelTypeCombo->count() < 4) params->kernelTypeCombo->addItem("Sigmoid");
        break;
    case 1: // Nu-SVM
        params->svmTypeLabel->setText("Nu");
        if(params->kernelTypeCombo->count() < 4) params->kernelTypeCombo->addItem("Sigmoid");
        break;
    case 2: // Pegasos
        params->optimizeCheck->setVisible(false);
        params->svmTypeLabel->setText("lambda");
        params->maxSVSpin->setVisible(true);
        params->labelMaxSV->setVisible(true);
        if(params->kernelTypeCombo->count() > 3) params->kernelTypeCombo->removeItem(3);
        break;
    }
    switch(params->kernelTypeCombo->currentIndex())
    {
    case 0: // linear
        params->kernelDegSpin->setVisible(false);
        params->labelDegree->setVisible(false);
        params->kernelWidthSpin->setVisible(false);
        params->labelWidth->setVisible(false);
        break;
    case 1: // poly
        params->kernelDegSpin->setVisible(true);
        params->labelDegree->setVisible(true);
        params->kernelWidthSpin->setVisible(false);
        params->labelWidth->setVisible(false);
        break;
    case 2: // RBF
        params->kernelDegSpin->setVisible(false);
        params->labelDegree->setVisible(false);
        params->kernelWidthSpin->setVisible(true);
        params->labelWidth->setVisible(true);
        break;
    case 3: // SIGMOID
        params->kernelDegSpin->setEnabled(false);
        params->labelDegree->setVisible(false);
        params->kernelWidthSpin->setEnabled(true);
        params->labelWidth->setVisible(true);
        break;
    }
}

QString ClassSVM::GetAlgoString()
{
    double C = params->svmCSpin->value();
    int sv = params->maxSVSpin->value();
    int kernelType = params->kernelTypeCombo->currentIndex();
    float kernelGamma = params->kernelWidthSpin->value();
    float kernelDegree = params->kernelDegSpin->value();
    bool bOptimize = params->optimizeCheck->isChecked();

    QString algo;
    switch(params->svmTypeCombo->currentIndex())
    {
    case 0: // C-SVM
        algo += "C-SVM";
        algo += QString(" %1").arg(C);
        break;
    case 1: // Nu-SVM
        algo += "Nu-SVM";
        algo += QString(" %1").arg(C);
        break;
    case 2: // Pegasos
        algo += "Pegasos";
        algo += QString(" %1 %2").arg(C).arg(sv);
        break;
    }
    switch(kernelType)
    {
    case 0:
        algo += " Lin";
        break;
    case 1:
        algo += QString(" Pol %1").arg(kernelDegree);
        break;
    case 2:
        algo += QString(" RBF %1").arg(kernelGamma);
        break;
    case 3:
        algo += QString(" Sig %1").arg(kernelGamma);
        break;
    }
    if(bOptimize) algo += QString(" Opt");
    return algo;
}

void ClassSVM::SetParams(Classifier *classifier)
{
    if(!classifier) return;
    int svmType = params->svmTypeCombo->currentIndex();
    int maxSV = params->maxSVSpin->value();
    float svmC = params->svmCSpin->value();
    int kernelType = params->kernelTypeCombo->currentIndex();
    float kernelGamma = params->kernelWidthSpin->value();
    float kernelDegree = params->kernelDegSpin->value();
    bool bOptimize = params->optimizeCheck->isChecked();

    ClassifierPegasos *pegasos = dynamic_cast<ClassifierPegasos *>(classifier);
    if(pegasos) pegasos->SetParams(svmC, max(2,(int)maxSV), kernelType, kernelGamma, kernelDegree);

    ClassifierSVM *svm = dynamic_cast<ClassifierSVM *>(classifier);
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
        switch(kernelType)
        {
        case 0:
            svm->param.kernel_type = LINEAR;
            break;
        case 1:
            svm->param.kernel_type = POLY;
            break;
        case 2:
            svm->param.kernel_type = RBF;
            break;
        case 3:
            svm->param.kernel_type = SIGMOID;
            break;
        }
        svm->param.C = svm->param.nu = svmC;
        svm->param.gamma = 1 / kernelGamma;
        svm->param.coef0 = 0;
        svm->param.degree = kernelDegree;
        svm->bOptimize = bOptimize;
    }

}

Classifier *ClassSVM::GetClassifier()
{
    int svmType = params->svmTypeCombo->currentIndex();
    Classifier *classifier = 0;
    switch(svmType)
    {
    case 2:
        classifier = new ClassifierPegasos();
        break;
    default:
        classifier = new ClassifierSVM();
        break;
    }
    SetParams(classifier);
    return classifier;
}

void ClassSVM::DrawInfo(Canvas *canvas, QPainter &painter, Classifier *classifier)
{
    painter.setRenderHint(QPainter::Antialiasing);

    if(dynamic_cast<ClassifierPegasos*>(classifier))
    {
        // we want to draw the support vectors
        vector<fvec> sv = dynamic_cast<ClassifierPegasos*>(classifier)->GetSVs();
        int radius = 9;
        FOR(i, sv.size())
        {
            QPointF point = canvas->toCanvasCoords(sv[i]);
            painter.setPen(QPen(Qt::black,6));
            painter.drawEllipse(point, radius, radius);
            painter.setPen(QPen(Qt::white,4));
            painter.drawEllipse(point, radius, radius);
        }
    }
    else if(dynamic_cast<ClassifierSVM*>(classifier))
    {
        int dim = canvas->data->GetDimCount();
        int xIndex = canvas->xIndex, yIndex = canvas->yIndex;
        // we want to draw the support vectors
        svm_model *svm = dynamic_cast<ClassifierSVM*>(classifier)->GetModel();
        if(svm)
        {
            f32 sv[2];
            FOR(i, svm->l)
            {
                FOR(j, 2)
                {
                    sv[j] = (f32)svm->SV[i][j].value;
                }
                int radius = 9;
                QPointF point = canvas->toCanvasCoords(sv[xIndex], sv[yIndex]);
                if(abs((*svm->sv_coef)[i]) == svm->param.C)
                {
                    painter.setPen(QPen(Qt::black, 6));
                    painter.drawEllipse(point, radius, radius);
                    painter.setPen(QPen(Qt::white,4));
                    painter.drawEllipse(point, radius, radius);
                }
                else
                {
                    painter.setPen(QPen(Qt::white, 6));
                    painter.drawEllipse(point, radius, radius);
                    painter.setPen(QPen(Qt::black,4));
                    painter.drawEllipse(point, radius, radius);
                }
            }
        }
    }
}

void ClassSVM::DrawModel(Canvas *canvas, QPainter &painter, Classifier *classifier)
{
    int posClass = 1;
    bool bUseMinMax = false;

    float resMin = FLT_MAX;
    float resMax = -FLT_MAX;
    if(bUseMinMax)
    {
        // TODO: get the min and max for all samples
        std::vector<fvec> samples = canvas->data->GetSamples();
        FOR(i, samples.size())
        {
            float val = classifier->Test(samples[i]);
            if(val > resMax) resMax = val;
            if(val < resMin) resMin = val;
        }
        if(resMin == resMax) resMin -= 3;
    }

    bool bNegatives = dynamic_cast<ClassifierPegasos*>(classifier);

    // we draw the samples
    painter.setRenderHint(QPainter::Antialiasing, true);
    map<int,int>& classes = classifier->classes;
    FOR(i, canvas->data->GetCount())
    {
        fvec sample = canvas->data->GetSample(i);
        int label = canvas->data->GetLabel(i);
        QPointF point = canvas->toCanvasCoords(canvas->data->GetSample(i));
        fvec res = classifier->TestMulti(sample);
        if(res.size() == 1)
        {
            float response = res[0];
            if(response > 0)
            {
                if(bNegatives)
                {
                    if(label == posClass) Canvas::drawSample(painter, point, 9, 1);
                    else Canvas::drawCross(painter, point, 6, 2);
                }
                else
                {
                    if(classifier->classMap[label] == posClass) Canvas::drawSample(painter, point, 9, 1);
                    else Canvas::drawCross(painter, point, 6, 2);
                }
            }
            else
            {
                if(bNegatives)
                {
                    if(label != posClass) Canvas::drawSample(painter, point, 9, 0);
                    else Canvas::drawCross(painter, point, 6, 0);
                }
                else
                {
                    if(classifier->classMap[label] != posClass) Canvas::drawSample(painter, point, 9, 0);
                    else Canvas::drawCross(painter, point, 6, 0);
                }
            }
        }
        else
        {
            int max = 0;
            for(int i=1; i<res.size(); i++) if(res[max] < res[i]) max = i;
            int resp = classifier->inverseMap[max];
            if(label == resp) Canvas::drawSample(painter, point, 9, label);
            else Canvas::drawCross(painter, point, 6, resp);
        }
    }
}

void ClassSVM::SaveOptions(QSettings &settings)
{
    settings.setValue("kernelDeg", params->kernelDegSpin->value());
    settings.setValue("kernelType", params->kernelTypeCombo->currentIndex());
    settings.setValue("kernelWidth", params->kernelWidthSpin->value());
    settings.setValue("svmC", params->svmCSpin->value());
    settings.setValue("svmType", params->svmTypeCombo->currentIndex());
    settings.setValue("optimizeCheck", params->optimizeCheck->isChecked());
    settings.setValue("maxSVSpin", params->maxSVSpin->value());
}

bool ClassSVM::LoadOptions(QSettings &settings)
{
    if(settings.contains("kernelDeg")) params->kernelDegSpin->setValue(settings.value("kernelDeg").toFloat());
    if(settings.contains("kernelType")) params->kernelTypeCombo->setCurrentIndex(settings.value("kernelType").toInt());
    if(settings.contains("kernelWidth")) params->kernelWidthSpin->setValue(settings.value("kernelWidth").toFloat());
    if(settings.contains("svmC")) params->svmCSpin->setValue(settings.value("svmC").toFloat());
    if(settings.contains("svmType")) params->svmTypeCombo->setCurrentIndex(settings.value("svmType").toInt());
    if(settings.contains("optimizeCheck")) params->optimizeCheck->setChecked(settings.value("optimizeCheck").toInt());
    if(settings.contains("maxSVSpin")) params->maxSVSpin->setValue(settings.value("maxSVSpin").toInt());
    ChangeOptions();
    return true;
}

void ClassSVM::SaveParams(QTextStream &file)
{
    file << "classificationOptions" << ":" << "kernelDeg" << " " << params->kernelDegSpin->value() << "\n";
    file << "classificationOptions" << ":" << "kernelType" << " " << params->kernelTypeCombo->currentIndex() << "\n";
    file << "classificationOptions" << ":" << "kernelWidth" << " " << params->kernelWidthSpin->value() << "\n";
    file << "classificationOptions" << ":" << "svmC" << " " << params->svmCSpin->value() << "\n";
    file << "classificationOptions" << ":" << "svmType" << " " << params->svmTypeCombo->currentIndex() << "\n";
    file << "classificationOptions" << ":" << "optimizeCheck" << " " << params->optimizeCheck->isChecked() << "\n";
    file << "classificationOptions" << ":" << "maxSVSpin" << " " << params->maxSVSpin->value() << "\n";
}

bool ClassSVM::LoadParams(QString name, float value)
{
    if(name.endsWith("kernelDeg")) params->kernelDegSpin->setValue((int)value);
    if(name.endsWith("kernelType")) params->kernelTypeCombo->setCurrentIndex((int)value);
    if(name.endsWith("kernelWidth")) params->kernelWidthSpin->setValue(value);
    if(name.endsWith("svmC")) params->svmCSpin->setValue(value);
    if(name.endsWith("svmType")) params->svmTypeCombo->setCurrentIndex((int)value);
    if(name.endsWith("optimizeCheck")) params->optimizeCheck->setChecked((int)value);
    if(name.endsWith("maxSVSpin")) params->maxSVSpin->setValue((int)value);
    ChangeOptions();
    return true;
}
