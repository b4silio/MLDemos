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
#include "interfaceRVMClassifier.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>
#include <QDebug>

using namespace std;

ClassRVM::ClassRVM()
{
    params = new Ui::ParametersRVM();
    params->setupUi(widget = new QWidget());
    connect(params->kernelTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeOptions()));
    ChangeOptions();
}

void ClassRVM::ChangeOptions()
{
    int C = params->svmCSpin->value();
    if(C > 1) params->svmCSpin->setValue(0.001);
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

QString ClassRVM::GetAlgoString()
{
    double C = params->svmCSpin->value();
    int kernelType = params->kernelTypeCombo->currentIndex();
    float kernelGamma = params->kernelWidthSpin->value();
    float kernelDegree = params->kernelDegSpin->value();

    QString algo = QString("RVM %1").arg(C);
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
    return algo;
}

void ClassRVM::SetParams(Classifier *classifier)
{
    if(!classifier) return;
    float svmC = params->svmCSpin->value();
    int kernelType = params->kernelTypeCombo->currentIndex();
    float kernelGamma = params->kernelWidthSpin->value();
    float kernelDegree = params->kernelDegSpin->value();


    ClassifierRVM *rvm = dynamic_cast<ClassifierRVM *>(classifier);
    if(rvm) rvm->SetParams(svmC, kernelType, kernelGamma, kernelDegree);
}

Classifier *ClassRVM::GetClassifier()
{
    Classifier *classifier = 0;
    classifier = new ClassifierRVM();
    SetParams(classifier);
    return classifier;
}

void ClassRVM::DrawInfo(Canvas *canvas, QPainter &painter, Classifier *classifier)
{
    painter.setRenderHint(QPainter::Antialiasing);

    if(!dynamic_cast<ClassifierRVM*>(classifier)) return;
    // we want to draw the support vectors
    vector<fvec> sv = dynamic_cast<ClassifierRVM*>(classifier)->GetSVs();
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

void ClassRVM::DrawModel(Canvas *canvas, QPainter &painter, Classifier *classifier)
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
                if(label == posClass) Canvas::drawSample(painter, point, 9, 1);
                else Canvas::drawCross(painter, point, 6, 2);
            }
            else
            {
                if(label != posClass) Canvas::drawSample(painter, point, 9, 0);
                else Canvas::drawCross(painter, point, 6, 0);
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

void ClassRVM::SaveOptions(QSettings &settings)
{
    settings.setValue("kernelDeg", params->kernelDegSpin->value());
    settings.setValue("kernelType", params->kernelTypeCombo->currentIndex());
    settings.setValue("kernelWidth", params->kernelWidthSpin->value());
    settings.setValue("svmC", params->svmCSpin->value());
}

bool ClassRVM::LoadOptions(QSettings &settings)
{
    if(settings.contains("kernelDeg")) params->kernelDegSpin->setValue(settings.value("kernelDeg").toFloat());
    if(settings.contains("kernelType")) params->kernelTypeCombo->setCurrentIndex(settings.value("kernelType").toInt());
    if(settings.contains("kernelWidth")) params->kernelWidthSpin->setValue(settings.value("kernelWidth").toFloat());
    if(settings.contains("svmC")) params->svmCSpin->setValue(settings.value("svmC").toFloat());
    ChangeOptions();
    return true;
}

void ClassRVM::SaveParams(QTextStream &file)
{
    file << "classificationOptions" << ":" << "kernelDeg" << " " << params->kernelDegSpin->value() << "\n";
    file << "classificationOptions" << ":" << "kernelType" << " " << params->kernelTypeCombo->currentIndex() << "\n";
    file << "classificationOptions" << ":" << "kernelWidth" << " " << params->kernelWidthSpin->value() << "\n";
    file << "classificationOptions" << ":" << "svmC" << " " << params->svmCSpin->value() << "\n";
}

bool ClassRVM::LoadParams(QString name, float value)
{
    if(name.endsWith("kernelDeg")) params->kernelDegSpin->setValue((int)value);
    if(name.endsWith("kernelType")) params->kernelTypeCombo->setCurrentIndex((int)value);
    if(name.endsWith("kernelWidth")) params->kernelWidthSpin->setValue(value);
    if(name.endsWith("svmC")) params->svmCSpin->setValue(value);
    ChangeOptions();
    return true;
}
