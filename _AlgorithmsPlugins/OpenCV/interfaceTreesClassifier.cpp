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
#include "interfaceTreesClassifier.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>
#include <QDebug>

using namespace std;

ClassTrees::ClassTrees()
    : displayLabel(0)
{
    params = new Ui::ParametersTrees();
	params->setupUi(widget = new QWidget());
    connect(params->displayButton, SIGNAL(clicked()), this, SLOT(DisplayTrees()));
}

ClassTrees::~ClassTrees()
{
    DEL(displayLabel);
}

void ClassTrees::DisplayTrees()
{
    if(!displayLabel)
    {
        displayLabel = new QLabel();
        displayLabel->setScaledContents(true);
    }
    if(!treePixmap.isNull())
    {
        displayLabel->setPixmap(treePixmap);
        displayLabel->setGeometry(displayLabel->geometry().x(), displayLabel->geometry().y(),
                                  treePixmap.width(), treePixmap.height());
        displayLabel->show();
    }
}

void ClassTrees::SetParams(Classifier *classifier)
{
	if(!classifier) return;
    ClassifierTrees *trees = dynamic_cast<ClassifierTrees *>(classifier);
    if(!trees) return;
    bool bBalanceClasses= params->balanceClassesCheck->isChecked();
    int minSampleCount = params->sampleCountSpin->value();
    int maxDepth = params->maxDepthSpin->value();
    int maxTrees = params->maxTreesSpin->value();
    float accuracyTolerance = params->accuracySpin->value();
    trees->SetParams(bBalanceClasses, minSampleCount, maxDepth, maxTrees, accuracyTolerance);
}

QString ClassTrees::GetAlgoString()
{
    bool bBalanceClasses= params->balanceClassesCheck->isChecked();
    int minSampleCount = params->sampleCountSpin->value();
    int maxDepth = params->maxDepthSpin->value();
    int maxTrees = params->maxTreesSpin->value();
    float accuracyTolerance = params->accuracySpin->value();

    QString algo = QString("RForest: T%1").arg(maxTrees);
    algo += QString(" D:%1").arg(maxDepth);
    algo += QString(" S:%1").arg(minSampleCount);
    algo += QString(" A:%1").arg(accuracyTolerance);
    algo += QString(" %1").arg(bBalanceClasses ? "Bal" : "Unbal");
    return algo;
}

Classifier *ClassTrees::GetClassifier()
{
    ClassifierTrees *classifier = new ClassifierTrees();
	SetParams(classifier);
    params->importanceList->clear();
	return classifier;
}

void ClassTrees::DrawModel(Canvas *canvas, QPainter &painter, Classifier *classifier)
{
	if(!classifier || !canvas) return;
	painter.setRenderHint(QPainter::Antialiasing, true);

	int posClass = 1;

	float resMin = FLT_MAX;
	float resMax = -FLT_MAX;
    std::vector<fvec> samples = canvas->data->GetSamples();

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
                if(classifier->classMap[label] == posClass) Canvas::drawSample(painter, point, 9, 1);
                else Canvas::drawCross(painter, point, 6, 2);
            }
            else
            {
                if(classifier->classMap[label] != posClass) Canvas::drawSample(painter, point, 9, 0);
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

void ClassTrees::DrawInfo(Canvas *canvas, QPainter &painter, Classifier *classifier)
{
    if(!classifier || !canvas) return;
    painter.setRenderHint(QPainter::Antialiasing, true);

    bool bUseMinMax = false;
    bUseMinMax = true;

    float resMin = FLT_MAX;
    float resMax = -FLT_MAX;
    std::vector<fvec> samples = canvas->data->GetSamples();
    ivec labels = canvas->data->GetLabels();

    ClassifierTrees *trees = dynamic_cast<ClassifierTrees*>(classifier);
    if(!trees) return;
    treePixmap = trees->treePixmap;
    if(params->displayButton->isChecked()) DisplayTrees();
    fvec importance = trees->GetImportance();
    params->importanceList->clear();
    FOR(i, importance.size())
    {
        params->importanceList->addItem(QString("Dim %1: %2%").arg(i+1).arg(importance[i]*100, 0, 'f', 1));
    }

    /*
    fvec weights = trees->GetErrorWeights();
    //qDebug() << "weights: ";
    FOR(i, weights.size())
    {
        //qDebug() << " " << weights[i];
        QPointF point = canvas->toCanvasCoords(boost->samples[i]);
        float response = classifier->Test(boost->samples[i]);
        int radius = max(3.f,min(10*tanh(weights[i]), 20.f));
        //int radius = max(1.f,min(9*sqrtf(weights[i]), 20.f));
        if(boost->labels[i] == 1)
        {
            painter.setBrush(Qt::red);
            painter.setPen(Qt::black);
            painter.drawEllipse(point, radius, radius);
            if(response < 0)
            {
                painter.setBrush(Qt::NoBrush);
                painter.setPen(QPen(Qt::white,2));
                painter.drawEllipse(point, max(3,radius-2), max(3,radius-2));
            }
        }
        else if(boost->labels[i] != 1)
        {
            painter.setBrush(Qt::white);
            painter.setPen(Qt::black);
            painter.drawEllipse(point, radius, radius);
            if(response >= 0)
            {
                painter.setBrush(Qt::NoBrush);
                painter.setPen(QPen(Qt::red,2));
                painter.drawEllipse(point, max(3,radius-2), max(3,radius-2));
            }
        }
    }
    */
}

void ClassTrees::SaveOptions(QSettings &settings)
{
    settings.setValue("balanceClasses", params->balanceClassesCheck->isChecked());
    settings.setValue("sampleCount", params->sampleCountSpin->value());
    settings.setValue("maxDepth", params->maxDepthSpin->value());
    settings.setValue("maxTrees", params->maxTreesSpin->value());
    settings.setValue("accuracy", params->accuracySpin->value());
}

bool ClassTrees::LoadOptions(QSettings &settings)
{
    if(settings.contains("balanceClasses")) params->balanceClassesCheck->setChecked(settings.value("balanceClasses").toBool());
    if(settings.contains("sampleCount")) params->sampleCountSpin->setValue(settings.value("sampleCount").toInt());
    if(settings.contains("maxDepth")) params->maxDepthSpin->setValue(settings.value("maxDepth").toInt());
    if(settings.contains("maxTrees")) params->maxTreesSpin->setValue(settings.value("maxTrees").toInt());
    if(settings.contains("accuracy")) params->accuracySpin->setValue(settings.value("accuracy").toFloat());
    return true;
}

void ClassTrees::SaveParams(QTextStream &file)
{
    file << "classificationOptions" << ":" << "balanceClasses" << " " << params->balanceClassesCheck->isChecked() << "\n";
    file << "classificationOptions" << ":" << "sampleCount" << " " << params->sampleCountSpin->value() << "\n";
    file << "classificationOptions" << ":" << "maxDepth" << " " << params->maxDepthSpin->value() << "\n";
    file << "classificationOptions" << ":" << "maxTrees" << " " << params->maxTreesSpin->value() << "\n";
    file << "classificationOptions" << ":" << "accuracy" << " " << params->accuracySpin->value() << "\n";
}

bool ClassTrees::LoadParams(QString name, float value)
{
    if(name.endsWith("balanceClasses")) params->balanceClassesCheck->setChecked((bool)value);
    if(name.endsWith("sampleCount")) params->sampleCountSpin->setValue((int)value);
    if(name.endsWith("maxDepth")) params->maxDepthSpin->setValue((int)value);
    if(name.endsWith("maxTrees")) params->maxTreesSpin->setValue((int)value);
    if(name.endsWith("accuracy")) params->accuracySpin->setValue((float)value);
    return true;
}
