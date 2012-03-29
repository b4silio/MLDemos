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
#include "interfaceMVM.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>
#include <QDebug>

using namespace std;

ClassMVM::ClassMVM()
{
    params = new Ui::ParametersMVM();
    params->setupUi(widget = new QWidget());
    connect(params->alphaSpin, SIGNAL(valueChanged(double)), this, SLOT(ChangeAlphas()));
    connect(params->alphaList, SIGNAL(currentRowChanged(int)), this, SLOT(SelectAlpha(int)));
    connect(params->samplesCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeSample()));
    connect(params->clearAlphaButton, SIGNAL(clicked()), this, SLOT(ClearAlphas()));
    connect(params->kernelTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeOptions()));
    ChangeOptions();
}

void ClassMVM::SelectAlpha(int index)
{
    if(!params->samplesCombo->count()) return;
    if(index >= indices.size()) return;
    params->samplesCombo->setCurrentIndex(indices[index]);
}

void ClassMVM::ChangeAlphas()
{
    if(!params->samplesCombo->count())
    {
        params->alphaList->clear();
        return;
    }
    int index = params->samplesCombo->currentIndex();
    int alphaIndex = -1;
    FOR(i, indices.size())
    {
        if(index == indices[i])
        {
            alphaIndex = i;
            break;
        }
    }
    float alpha = params->alphaSpin->value();
    if(alphaIndex != -1)
    {
        alphas[alphaIndex] = alpha;
    }
    else
    {
        indices.push_back(index);
        alphas.push_back(alpha);
        signs.push_back(labels[index] ? 1 : -1);
    }
    params->alphaList->clear();
    FOR(i, indices.size())
    {
        params->alphaList->addItem(QString("s%1 (%2): %3").arg(indices[i]+1).arg(signs[i]>0).arg(alphas[i], 0, 'f', 2));
    }
}

void ClassMVM::ChangeSample()
{
    if(!params->samplesCombo->count()) return;
    int index = params->samplesCombo->currentIndex();
    int alphaIndex = -1;
    FOR(i, indices.size())
    {
        if(index == indices[i])
        {
            alphaIndex = i;
            break;
        }
    }
    float alpha = 0;
    if(alphaIndex != -1)
    {
        alpha = alphas[alphaIndex];
    }

    bool bOld = params->alphaSpin->blockSignals(true);
    params->alphaSpin->setValue(alpha);
    params->alphaSpin->blockSignals(bOld);
}

void ClassMVM::SetSampleList(vector<fvec> samples, ivec labels)
{
    this->samples = samples;
    this->labels = labels;
    int oldIndex = params->samplesCombo->currentIndex();
    bool bOld = params->samplesCombo->blockSignals(true);
    params->samplesCombo->clear();
    int count = samples.size();
    int dim = samples.size() ? samples[0].size() : 2;
    FOR(i, count)
    {
        QString text = QString("%1 (%2):").arg(i+1).arg(labels[i]);
        FOR(d, min(2,dim)) text += QString("%1").arg(samples[i][d], 0, 'f', 3);
        params->samplesCombo->addItem(text);
    }
    if(oldIndex < count) params->samplesCombo->setCurrentIndex(oldIndex);
    else params->samplesCombo->setCurrentIndex(0);
    params->samplesCombo->blockSignals(bOld);
    params->statusLabel->setText(count ? "Manually select samples and alphas" : "Hit TRAIN before starting");
}

void ClassMVM::ClearAlphas()
{
    indices.clear();
    alphas.clear();
    signs.clear();
    bool bOld = params->alphaList->blockSignals(true);
    params->alphaList->clear();
    params->alphaList->blockSignals(bOld);

    bOld = params->alphaSpin->blockSignals(true);
    params->alphaSpin->setValue(0.);
    params->alphaSpin->blockSignals(bOld);
}

void ClassMVM::ChangeOptions()
{
    switch(params->kernelTypeCombo->currentIndex())
    {
    case 0: // linear
        params->kernelDegSpin->setEnabled(false);
        params->kernelDegSpin->setVisible(false);
        break;
    case 1: // poly
        params->kernelDegSpin->setEnabled(true);
        params->kernelDegSpin->setVisible(true);
        params->kernelWidthSpin->setEnabled(false);
        params->kernelWidthSpin->setVisible(false);
        break;
    case 2: // RBF
        params->kernelDegSpin->setEnabled(false);
        params->kernelDegSpin->setVisible(false);
        params->kernelWidthSpin->setEnabled(true);
        params->kernelWidthSpin->setVisible(true);
        break;
    }
}

QString ClassMVM::GetAlgoString()
{
    int kernelType = params->kernelTypeCombo->currentIndex();
    float kernelGamma = params->kernelWidthSpin->value();
    float kernelDegree = params->kernelDegSpin->value();

    QString algo = "MVM";
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
    }
    return algo;
}

void ClassMVM::SetParams(Classifier *classifier)
{
    if(!classifier) return;
    int kernelType = params->kernelTypeCombo->currentIndex();
    float kernelGamma = params->kernelWidthSpin->value();
    float kernelDegree = params->kernelDegSpin->value();

    ClassifierMVM *mvm = dynamic_cast<ClassifierMVM *>(classifier);
    if(!mvm) return;

    switch(kernelType)
    {
    case 0:
        mvm->kernel_type = LINEAR;
        break;
    case 1:
        mvm->kernel_type = POLY;
        break;
    case 2:
        mvm->kernel_type = RBF;
        break;
    }
    mvm->gamma = 1 / kernelGamma;
    mvm->degree = kernelDegree;
    mvm->indices = indices;
    mvm->alphas = alphas;
    FOR(i, alphas.size()) mvm->alphas[i] *= signs[i];
    mvm->manualSamples = samples;
    mvm->manualLabels = labels;
}

Classifier *ClassMVM::GetClassifier()
{
    Classifier *classifier = new ClassifierMVM();
    SetParams(classifier);
    return classifier;
}

void ClassMVM::DrawInfo(Canvas *canvas, QPainter &painter, Classifier *classifier)
{
    SetSampleList(canvas->data->GetSamples(), canvas->data->GetLabels());
    painter.setRenderHint(QPainter::Antialiasing);

    ClassifierMVM *mvm = dynamic_cast<ClassifierMVM*>(classifier);
    if(!mvm) return;

    int dim = canvas->data->GetDimCount();
    int xIndex = canvas->xIndex, yIndex = canvas->yIndex;
    // we want to draw the support vectors

    vector<fvec> samples = canvas->data->GetSamples();
    ivec labels = canvas->data->GetLabels();
    QFont font = painter.font();
    font.setPointSize(9);
    painter.setFont(font);
    FOR(i, mvm->indices.size())
    {
        int radius = 9;
        QPointF point = canvas->toCanvasCoords(samples[mvm->indices[i]]);
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(Qt::black, 6));
        painter.drawEllipse(point, radius, radius);
        painter.setPen(QPen(Qt::white,4));
        painter.drawEllipse(point, radius, radius);
        painter.setBrush(Qt::black);
        painter.setPen(Qt::white);
        painter.drawEllipse(QPointF(point.x()+radius+6+3, point.y()), 6, 6);
        painter.setPen(QPen(Qt::white,4));
        painter.drawText(point.x()+radius+3, point.y()-6, 12, 12, Qt::AlignCenter, QString("%1").arg(i+1));
    }
}

void ClassMVM::DrawModel(Canvas *canvas, QPainter &painter, Classifier *classifier)
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
    FOR(i, canvas->data->GetCount())
    {
        fvec sample = canvas->data->GetSample(i);
        int label = canvas->data->GetLabel(i);
        QPointF point = canvas->toCanvasCoords(canvas->data->GetSample(i));
        float response = classifier->Test(sample);
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
}

void ClassMVM::SaveOptions(QSettings &settings)
{
    settings.setValue("kernelDeg", params->kernelDegSpin->value());
    settings.setValue("kernelType", params->kernelTypeCombo->currentIndex());
    settings.setValue("kernelWidth", params->kernelWidthSpin->value());
}

bool ClassMVM::LoadOptions(QSettings &settings)
{
    if(settings.contains("kernelDeg")) params->kernelDegSpin->setValue(settings.value("kernelDeg").toFloat());
    if(settings.contains("kernelType")) params->kernelTypeCombo->setCurrentIndex(settings.value("kernelType").toInt());
    if(settings.contains("kernelWidth")) params->kernelWidthSpin->setValue(settings.value("kernelWidth").toFloat());
    ChangeOptions();
    return true;
}

void ClassMVM::SaveParams(QTextStream &file)
{
    file << "classificationOptions" << ":" << "kernelDeg" << " " << params->kernelDegSpin->value() << "\n";
    file << "classificationOptions" << ":" << "kernelType" << " " << params->kernelTypeCombo->currentIndex() << "\n";
    file << "classificationOptions" << ":" << "kernelWidth" << " " << params->kernelWidthSpin->value() << "\n";
}

bool ClassMVM::LoadParams(QString name, float value)
{
    if(name.endsWith("kernelDeg")) params->kernelDegSpin->setValue((int)value);
    if(name.endsWith("kernelType")) params->kernelTypeCombo->setCurrentIndex((int)value);
    if(name.endsWith("kernelWidth")) params->kernelWidthSpin->setValue(value);
    ChangeOptions();
    return true;
}
