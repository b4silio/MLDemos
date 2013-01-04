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
#include "interfaceProjections.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>
#include <QDebug>

using namespace std;

ClassProjections::ClassProjections()
{
    params = new Ui::ParametersProjections();
    params->setupUi(widget = new QWidget());
    projectionWindow = NULL;
    canvas = NULL;
    classifier = NULL;
    classifierType = 0;
    connect(params->projectionButton, SIGNAL(clicked()), this, SLOT(ShowProjection()));
    connect(params->linearTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(OptionsChanged()));
    OptionsChanged();
}

void ClassProjections::OptionsChanged()
{
    bool bKernelVisible = params->linearTypeCombo->currentIndex() == 4; // kernel pca
    params->kernelDegSpin->setVisible(bKernelVisible);
    params->kernelTypeCombo->setVisible(bKernelVisible);
    params->kernelWidthSpin->setVisible(bKernelVisible);
    params->labelDegree->setVisible(bKernelVisible);
    params->labelWidth->setVisible(bKernelVisible);
    params->labelkernel->setVisible(bKernelVisible);
}

void ClassProjections::SetParams(Classifier *classifier)
{
    if(!classifier) return;
    int type = params->linearTypeCombo->currentIndex();
    classifierType = type;
    if(type != 4) ((ClassifierLinear *)classifier)->SetParams(type);
    else
    {
        int kernelType = params->kernelTypeCombo->currentIndex();
        float kernelWidth = params->kernelWidthSpin->value();
        int kernelDegree = params->kernelDegSpin->value();
        float kernelOffset = (kernelType == 3) ? params->kernelDegSpin->value() : params->kernelWidthSpin->value();
        ((ClassifierKPCA *)classifier)->SetParams(kernelType, kernelDegree, kernelWidth, kernelOffset);
    }
}

fvec ClassProjections::GetParams()
{
    int type = params->linearTypeCombo->currentIndex();
    int kernelType = params->kernelTypeCombo->currentIndex();
    float kernelWidth = params->kernelWidthSpin->value();
    int kernelDegree = params->kernelDegSpin->value();

    fvec par(4);
    par[0] = type;
    par[1] = kernelType;
    par[2] = kernelWidth;
    par[3] = kernelDegree;
    return par;
}

void ClassProjections::SetParams(Classifier *classifier, fvec parameters)
{
    if(!classifier) return;
    int type = parameters.size() > 0 ? parameters[0] : 1;
    int kernelType = parameters.size() > 1 ? parameters[1] : 0;
    float kernelWidth = parameters.size() > 2 ? parameters[2] : 0;
    int kernelDegree = parameters.size() > 3 ? parameters[3] : 0;
    float kernelOffset =  (kernelType == 3) ? kernelDegree : kernelWidth;
    if(type == 4)
        ((ClassifierKPCA *)classifier)->SetParams(kernelType, kernelDegree, kernelWidth, kernelOffset);
    else ((ClassifierLinear *)classifier)->SetParams(type);
}

void ClassProjections::GetParameterList(std::vector<QString> &parameterNames,
                             std::vector<QString> &parameterTypes,
                             std::vector< std::vector<QString> > &parameterValues)
{
    parameterNames.push_back("Projection Type");
    parameterNames.push_back("Kernel Type");
    parameterNames.push_back("Kernel Width");
    parameterNames.push_back("Kernel Degree");
    parameterTypes.push_back("List");
    parameterTypes.push_back("List");
    parameterTypes.push_back("Real");
    parameterTypes.push_back("Integer");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("PCA");
    parameterValues.back().push_back("Means-Only LDA");
    parameterValues.back().push_back("LDA");
    parameterValues.back().push_back("Fisher LDA");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("Linear");
    parameterValues.back().push_back("Poly");
    parameterValues.back().push_back("RBF");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("0.00000001f");
    parameterValues.back().push_back("9999999999");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("1");
    parameterValues.back().push_back("150");
}

QString ClassProjections::GetAlgoString()
{
    int type = params->linearTypeCombo->currentIndex();
    switch(type)
    {
    case 0:
        return "PCA";
    case 1:
        return "Means-Only";
    case 2:
        return "LDA";
    case 3:
        return "Fisher-LDA";
    case 4:
        return "Kernel PCA";
    case 5:
        return "Naive Bayes";
    default:
        return "None";
    }
}

Classifier *ClassProjections::GetClassifier()
{
    int type = params->linearTypeCombo->currentIndex();
    if(type == 4)
    {
        classifier = new ClassifierKPCA();
    }
    else{
        classifier = new ClassifierLinear();
    }
    SetParams(classifier);
    return classifier;
}

void ClassProjections::DrawInfo(Canvas *canvas, QPainter &painter, Classifier *classifier)
{
    if(!canvas || !classifier) return;
    if(!this->canvas) this->canvas = canvas;
    if(canvas->canvasType) return;
    painter.setRenderHint(QPainter::Antialiasing);
    ClassifierLinear *linear = (ClassifierLinear*)classifier;

    if(linear->GetType() < 4) // PCA, LDA, Fisher
    {
        fvec pt[5];
        QPointF point[4];
        FOR(i,5) pt[i].resize(2,0);
        pt[0][0]=1;pt[0][1]=0;
        pt[1][0]=-1;pt[1][1]=0;
        pt[2][0]=0;pt[2][1]=0;
        FOR(i, 3) pt[i] = linear->Project(pt[i]);
        point[0] = canvas->toCanvasCoords(pt[0]);
        point[1] = canvas->toCanvasCoords(pt[1]);
        painter.setPen(QPen(Qt::black, 2));
        painter.drawLine(point[0], point[1]);
    }
    if(projectionWindow && projectionWindow->isVisible()) ShowProjection();
}

void ClassProjections::DrawModel(Canvas *canvas, QPainter &painter, Classifier *classifier)
{
    if(!classifier || !canvas) return;
    if(!this->canvas) this->canvas = canvas;
    if(canvas->canvasType) return;
    painter.setRenderHint(QPainter::Antialiasing, true);

    int posClass = 1;
    bool bUseMinMax = true;
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

void ClassProjections::ShowProjection()
{
    //if(projectionWindow && projectionWindow->isVisible())
    //{
    //	projectionWindow->hide();
    //	return;
    //}
    if(!classifier || !canvas) return;
    // we project all the data into a new image
    int w = canvas->width()/2;
    if(classifierType == 4) w = canvas->width();
    int h = canvas->height()/2;
    QPixmap projectionPixmap(w, h);
    projectionPixmap.fill();
    vector<fvec> samples = canvas->data->GetSamples();
    if(!samples.size()) return;
    ivec labels = canvas->data->GetLabels();

    QPainter painter(&projectionPixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);

    if(classifierType < 4) // PCA, LDA, Fisher
    {
        fvec pt[5];
        QPointF point[4];
        FOR(i,5) pt[i].resize(2,0);
        pt[0][0]=1;pt[0][1]=0;
        pt[1][0]=-1;pt[1][1]=0;
        pt[2][0]=0;pt[2][1]=0;

        FOR(i, 3)
        {
            pt[i] = ((ClassifierLinear *)classifier)->Project(pt[i]);
        }

        point[0] = canvas->toCanvasCoords(pt[0])*0.5f;
        point[1] = canvas->toCanvasCoords(pt[1])*0.5f;
        painter.setPen(QPen(Qt::black, 2));
        painter.drawLine(point[0], point[1]);

        // draw lines
        FOR(i, samples.size())
        {
            fvec sample = samples[i];
            int label = labels[i];
            fvec newSample = ((ClassifierLinear *)classifier)->Project(sample);

            QPointF point = canvas->toCanvasCoords(newSample)/canvas->width()*w;
            QPointF original = canvas->toCanvasCoords(samples[i])/canvas->width()*w;
            painter.setPen(QPen(QColor(0,0,0,128), 0.2));
            painter.drawLine(original, point);
        }

        // draw samples
        FOR(i, samples.size())
        {
            fvec sample = samples[i];
            int label = labels[i];
            fvec newSample = ((ClassifierLinear *)classifier)->Project(sample);

            QPointF point = canvas->toCanvasCoords(newSample)/canvas->width()*w;
            QPointF original = canvas->toCanvasCoords(samples[i])/canvas->width()*w;
            Canvas::drawSample(painter, point, 6, label);
            Canvas::drawSample(painter, original, 6, label);
        }
    }
    if(classifierType == 6) // ICA
    {
        int dim = samples[0].size();
        fvec meanAll; meanAll.resize(dim, 0);
        FOR(i, samples.size())
        {
            meanAll += samples[i];
        }
        meanAll /= samples.size();

        vector<fvec> projected(samples.size());
        FOR(i, samples.size()) projected[i] = ((ClassifierLinear *)classifier)->Project(samples[i]);
        // draw lines
        vector<QPointF> points(samples.size());
        vector<QPointF> originals(samples.size());
        QPointF pointsMean(0,0), originalsMean(0,0);
        fvec mins(2, FLT_MAX), maxes(2, -FLT_MAX);
        FOR(i, samples.size())
        {
            fvec& sample = samples[i];
            fvec& newSample = projected[i];

            QPointF point = canvas->toCanvasCoords(newSample);
            point = QPointF(point.x()/canvas->width(), point.y()/canvas->height());
            mins[0] = min(mins[0], (float)point.x());
            maxes[0] = max(maxes[0], (float)point.x());
            mins[1] = min(mins[1], (float)point.y());
            maxes[1] = max(maxes[1], (float)point.y());
            QPointF original = canvas->toCanvasCoords(sample-meanAll);
            original = QPointF(original.x()/canvas->width(), original.y()/canvas->height());
            points[i] = point;
            originals[i] = original;
            pointsMean += point;
            originalsMean += original;
        }
        pointsMean /= samples.size();
        originalsMean /= samples.size();
        qDebug() << "means" << pointsMean << originalsMean;
        FOR(i, samples.size())
        {
            QPointF original = originals[i] - originalsMean;
            original.setX(original.x()*w*0.5 + w*0.25);
            original.setY(original.y()*h + h*0.5);
            QPointF point = points[i];
            point = QPointF((point.x()-mins[0])/(maxes[0]-mins[0]) - 0.5f, (point.y()-mins[1])/(maxes[1]-mins[1]) - 0.5f);
            point.setX(point.x()*w*0.4 + w*0.75);
            point.setY(point.y()*h*0.8 + h*0.5);
            if(original.x() > w/2) continue;
            painter.setPen(QPen(QColor(0,0,0,40), 0.2));
            painter.drawLine(original, point);
        }
        FOR(i, samples.size())
        {
            QPointF original = originals[i] - originalsMean;
            original.setX(original.x()*w*0.5 + w*0.25);
            original.setY(original.y()*h + h*0.5);
            QPointF point = points[i];
            point = QPointF((point.x()-mins[0])/(maxes[0]-mins[0]) - 0.5f, (point.y()-mins[1])/(maxes[1]-mins[1]) - 0.5f);
            point.setX(point.x()*w*0.4 + w*0.75);
            point.setY(point.y()*h*0.8 + h*0.5);
            Canvas::drawSample(painter, point, 6, labels[i]);
            if(original.x() > w/2) continue;
            Canvas::drawSample(painter, original, 6, labels[i]);
        }

        painter.setPen(QPen(Qt::black, 2.f));
        painter.drawLine(QPointF(w*0.5f, 0), QPointF(w*0.5f, h));
    }
    if(classifierType == 4) // KPCA
    {
        ClassifierKPCA *kpca = (ClassifierKPCA*)classifier;

        vector<fvec> results = kpca->GetResults();
        ivec labels = kpca->GetLabels();

        vector<fvec> samples = kpca->GetSamples();

        fvec meanAll(samples[0].size(), 0);
        FOR(i, samples.size())
        {
            meanAll += samples[i];
        }
        meanAll /= samples.size();

        // draw lines
        FOR(i, samples.size())
        {
            fvec sample = samples[i];
            int label = labels[i];
            fvec newSample = results[i];

            QPointF point = canvas->toCanvasCoords(newSample)/canvas->width()*w*0.5 + QPointF(w/4, -h*0.5);
            QPointF original = canvas->toCanvasCoords(samples[i]-meanAll)/canvas->width()*w*0.5;
            painter.setPen(QPen(QColor(0,0,0,40), 0.2));
            painter.drawLine(original, point);
        }

        // draw samples
        FOR(i, samples.size())
        {
            fvec sample = samples[i];
            int label = labels[i];
            fvec newSample = results[i];

            QPointF point = canvas->toCanvasCoords(newSample)/canvas->width()*w*0.5 + QPointF(w/4, -h*0.5);
            QPointF original = canvas->toCanvasCoords(samples[i]-meanAll)/canvas->width()*w*0.5;
            Canvas::drawSample(painter, point, 6, label);
            Canvas::drawSample(painter, original, 6, label);
        }

        painter.setPen(QPen(Qt::black, 2.f));
        painter.drawLine(QPointF(w*0.5f, 0), QPointF(w*0.5f, h));
    }

    if(!projectionWindow) projectionWindow = new QLabel("Projections");
    projectionWindow->resize(projectionPixmap.size());
    projectionWindow->setPixmap(projectionPixmap);
    projectionWindow->show();
}

void ClassProjections::SaveOptions(QSettings &settings)
{
    settings.setValue("linearType", params->linearTypeCombo->currentIndex());
    settings.setValue("kernelDeg", params->kernelDegSpin->value());
    settings.setValue("kernelType", params->kernelTypeCombo->currentIndex());
    settings.setValue("kernelWidth", params->kernelWidthSpin->value());
}

bool ClassProjections::LoadOptions(QSettings &settings)
{
    if(settings.contains("linearType")) params->linearTypeCombo->setCurrentIndex(settings.value("linearType").toInt());
    if(settings.contains("kernelDeg")) params->kernelDegSpin->setValue(settings.value("kernelDeg").toFloat());
    if(settings.contains("kernelType")) params->kernelTypeCombo->setCurrentIndex(settings.value("kernelType").toInt());
    if(settings.contains("kernelWidth")) params->kernelWidthSpin->setValue(settings.value("kernelWidth").toFloat());
    return true;
}

void ClassProjections::SaveParams(QTextStream &file)
{
    file << "classificationOptions" << ":" << "linearType" << " " << params->linearTypeCombo->currentIndex() << "\n";
    file << "classificationOptions" << ":" << "kernelDeg" << " " << params->kernelDegSpin->value() << "\n";
    file << "classificationOptions" << ":" << "kernelType" << " " << params->kernelTypeCombo->currentIndex() << "\n";
    file << "classificationOptions" << ":" << "kernelWidth" << " " << params->kernelWidthSpin->value() << "\n";
}

bool ClassProjections::LoadParams(QString name, float value)
{
    if(name.endsWith("linearType")) params->linearTypeCombo->setCurrentIndex((int)value);
    if(name.endsWith("kernelDeg")) params->kernelDegSpin->setValue((int)value);
    if(name.endsWith("kernelType")) params->kernelTypeCombo->setCurrentIndex((int)value);
    if(name.endsWith("kernelWidth")) params->kernelWidthSpin->setValue(value);
    return true;
}
