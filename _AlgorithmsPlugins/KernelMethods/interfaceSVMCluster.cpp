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
#include "interfaceSVMCluster.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>

using namespace std;

ClustSVM::ClustSVM()
{
    params = new Ui::ParametersClust();
    params->setupUi(widget = new QWidget());
    connect(params->kernelTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeOptions()));
    ChangeOptions();
}

void ClustSVM::ChangeOptions()
{
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
    }
}

void ClustSVM::SetParams(Clusterer *clusterer)
{
    if(!clusterer) return;
    SetParams(clusterer, GetParams());
}

fvec ClustSVM::GetParams()
{
    float svmNu = params->svmCSpin->value();
    int kernelType = params->kernelTypeCombo->currentIndex();
    float kernelGamma = params->kernelWidthSpin->value();
    float kernelDegree = params->kernelDegSpin->value();

    fvec par(4);
    par[0] = svmNu;
    par[1] = kernelType;
    par[2] = kernelGamma;
    par[3] = kernelDegree;
    return par;
}

void ClustSVM::SetParams(Clusterer *clusterer, fvec parameters)
{
    if(!clusterer) return;
    float svmNu = parameters.size() > 0 ? parameters[0] : 1;
    int kernelType = parameters.size() > 1 ? parameters[1] : 0;
    float kernelGamma = parameters.size() > 2 ? parameters[2] : 0;
    int kernelDegree = parameters.size() > 3 ? parameters[3] : 0;

    ClustererSVR *svm = dynamic_cast<ClustererSVR *>(clusterer);
    if(!svm) return;
    svm->param.nu = svmNu;
    svm->param.kernel_type = kernelType;
    svm->param.gamma = 1 / kernelGamma;
    svm->param.degree = kernelDegree;
}

void ClustSVM::GetParameterList(std::vector<QString> &parameterNames,
                                std::vector<QString> &parameterTypes,
                                std::vector< std::vector<QString> > &parameterValues)
{
    parameterNames.push_back("Nu");
    parameterNames.push_back("Kernel Type");
    parameterNames.push_back("Kernel Width");
    parameterNames.push_back("Kernel Degree");
    parameterTypes.push_back("Real");
    parameterTypes.push_back("List");
    parameterTypes.push_back("Real");
    parameterTypes.push_back("Integer");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("0.00000001f");
    parameterValues.back().push_back("1.0f");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("Linear");
    parameterValues.back().push_back("Poly");
    parameterValues.back().push_back("RBF");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("0.00000001f");
    parameterValues.back().push_back("9999999");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("1");
    parameterValues.back().push_back("150");
}

Clusterer *ClustSVM::GetClusterer()
{
    Clusterer *clusterer = 0;
    clusterer = new ClustererSVR();
    SetParams(clusterer);
    return clusterer;
}

void ClustSVM::DrawInfo(Canvas *canvas, QPainter &painter, Clusterer *clusterer)
{
    if(!canvas || !clusterer) return;

    painter.setRenderHint(QPainter::Antialiasing);
    if(!dynamic_cast<ClustererSVR*>(clusterer)) return;

    // we want to draw the support vectors
    svm_model *svm = dynamic_cast<ClustererSVR*>(clusterer)->GetModel();
    painter.setBrush(Qt::NoBrush);
    if(svm)
    {
        f32 sv[2];
        FOR(i, svm->l)
        {
            FOR(j, 2)
            {
                sv[j] = (f32)svm->SV[i][j].value;
            }
            int radius = 11;
            QPointF point = canvas->toCanvasCoords(sv[0],sv[1]);
            if(abs((*svm->sv_coef)[i]) == svm->param.C)
            {
                painter.setPen(QPen(Qt::black, 4));
                painter.drawEllipse(point, radius, radius);
                painter.setPen(Qt::white);
                painter.drawEllipse(point, radius, radius);
            }
            else
            {
                painter.setPen(Qt::black);
                painter.drawEllipse(point, radius, radius);
            }
        }
    }
}

void ClustSVM::DrawModel(Canvas *canvas, QPainter &painter, Clusterer *clusterer)
{
    painter.setRenderHint(QPainter::Antialiasing);

    FOR(i, canvas->data->GetSamples().size())
    {
        fvec sample = canvas->data->GetSample(i);
        QPointF point = canvas->toCanvasCoords(sample);
        fvec res = clusterer->Test(sample);
        float r=0,g=0,b=0;
        if(res.size() > 1)
        {
            FOR(j, res.size())
            {
                r += SampleColor[(j+1)%SampleColorCnt].red()*res[j];
                g += SampleColor[(j+1)%SampleColorCnt].green()*res[j];
                b += SampleColor[(j+1)%SampleColorCnt].blue()*res[j];
            }
        }
        else if(res.size())
        {
            r = (1-res[0])*255 + res[0]* 255;
            g = (1-res[0])*255;
            b = (1-res[0])*255;
        }
        painter.setBrush(QColor(r,g,b));
        painter.setPen(Qt::black);
        painter.drawEllipse(point,5,5);
    }
}

void ClustSVM::SaveOptions(QSettings &settings)
{
    settings.setValue("kernelDeg", params->kernelDegSpin->value());
    settings.setValue("kernelType", params->kernelTypeCombo->currentIndex());
    settings.setValue("kernelWidth", params->kernelWidthSpin->value());
    settings.setValue("svmNu", params->svmCSpin->value());
}

bool ClustSVM::LoadOptions(QSettings &settings)
{
    if(settings.contains("kernelDeg")) params->kernelDegSpin->setValue(settings.value("kernelDeg").toFloat());
    if(settings.contains("kernelType")) params->kernelTypeCombo->setCurrentIndex(settings.value("kernelType").toInt());
    if(settings.contains("kernelWidth")) params->kernelWidthSpin->setValue(settings.value("kernelWidth").toFloat());
    if(settings.contains("svmNu")) params->svmCSpin->setValue(settings.value("svmNu").toFloat());
    ChangeOptions();
    return true;
}

void ClustSVM::SaveParams(QTextStream &file)
{
    file << "clusterOptions" << ":" << "kernelDeg" << " " << params->kernelDegSpin->value() << "\n";
    file << "clusterOptions" << ":" << "kernelType" << " " << params->kernelTypeCombo->currentIndex() << "\n";
    file << "clusterOptions" << ":" << "kernelWidth" << " " << params->kernelWidthSpin->value() << "\n";
    file << "clusterOptions" << ":" << "svmNu" << " " << params->svmCSpin->value() << "\n";
}

bool ClustSVM::LoadParams(QString name, float value)
{
    if(name.endsWith("kernelDeg")) params->kernelDegSpin->setValue((int)value);
    if(name.endsWith("kernelType")) params->kernelTypeCombo->setCurrentIndex((int)value);
    if(name.endsWith("kernelWidth")) params->kernelWidthSpin->setValue(value);
    if(name.endsWith("svmNu")) params->svmCSpin->setValue(value);
    ChangeOptions();
    return true;
}
