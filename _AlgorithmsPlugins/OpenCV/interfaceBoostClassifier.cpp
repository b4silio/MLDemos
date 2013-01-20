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
#include "interfaceBoostClassifier.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>
#include <QDebug>

using namespace std;

ClassBoost::ClassBoost()
{
	params = new Ui::ParametersBoost();
	params->setupUi(widget = new QWidget());
    connect(params->boostLearnerType, SIGNAL(currentIndexChanged(int)), this, SLOT(OptionsChanged()));
    OptionsChanged();
}

void ClassBoost::OptionsChanged()
{
    params->svmCountLabel->setVisible(false);
    params->svmCountSpin->setVisible(false);
    if(params->boostLearnerType->currentIndex() == 5) // svm
    {
        params->svmCountLabel->setVisible(true);
        params->svmCountSpin->setVisible(true);
    }
}

void ClassBoost::SetParams(Classifier *classifier)
{
	if(!classifier) return;
    ClassifierBoost *boost = dynamic_cast<ClassifierBoost *>(classifier);
    if(!boost) return;
    int weakCount = params->boostCountSpin->value();
    int weakType = params->boostLearnerType->currentIndex();
    int boostType = params->boostType->currentIndex();
    int svmCount = params->svmCountSpin->value();
    boost->SetParams(weakCount, weakType, boostType, svmCount);
}

fvec ClassBoost::GetParams()
{
    int weakCount = params->boostCountSpin->value();
    int weakType = params->boostLearnerType->currentIndex();
    int boostType = params->boostType->currentIndex();
    int svmCount = params->svmCountSpin->value();

    fvec par(4);
    par[0] = weakCount;
    par[1] = weakType;
    par[2] = boostType;
    par[3] = svmCount;
    return par;
}

void ClassBoost::SetParams(Classifier *classifier, fvec parameters)
{
    if(!classifier) return;
    int weakCount= parameters.size() > 0 ? parameters[0] : 1;
    int weakType = parameters.size() > 1 ? parameters[1] : 0;
    int boostType = parameters.size() > 2 ? parameters[2] : 0;
    int svmCount = parameters.size() > 3 ? parameters[3] : 10;

    ClassifierBoost *boost = dynamic_cast<ClassifierBoost *>(classifier);
    if(!boost) return;
    boost->SetParams(weakCount, weakType, boostType, svmCount);
}

void ClassBoost::GetParameterList(std::vector<QString> &parameterNames,
                             std::vector<QString> &parameterTypes,
                             std::vector< std::vector<QString> > &parameterValues)
{
    parameterNames.clear();
    parameterTypes.clear();
    parameterValues.clear();
    parameterNames.push_back("Weak Learner Count");
    parameterNames.push_back("Weak Learner Type");
    parameterNames.push_back("Boosting Variant");
    parameterNames.push_back("Weak Learner Param");
    parameterTypes.push_back("Integer");
    parameterTypes.push_back("List");
    parameterTypes.push_back("List");
    parameterTypes.push_back("Integer");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("1");
    parameterValues.back().push_back("999999");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("Decision Stump");
    parameterValues.back().push_back("Random Projection");
    parameterValues.back().push_back("Random Rectangle");
    parameterValues.back().push_back("Random Circle");
    parameterValues.back().push_back("Random Gaussians");
    parameterValues.back().push_back("Random SVM");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("Discrete");
    parameterValues.back().push_back("Real");
    parameterValues.back().push_back("Logit");
    parameterValues.back().push_back("Gentle");
}

QString ClassBoost::GetAlgoString()
{
	int weakCount = params->boostCountSpin->value();
	int weakType = params->boostLearnerType->currentIndex();
    int svmCount = params->svmCountSpin->value();
	QString algo = QString("Boost %1").arg(weakCount);
	switch(weakType)
	{
    case 0:
        algo += " Stump";
        break;
    case 1:
        algo += " Proj";
        break;
    case 2:
		algo += " Rect";
		break;
    case 3:
        algo += " Circ";
        break;
    case 4:
        algo += " GMM";
        break;
    case 5:
        algo += QString(" SVM %1").arg(svmCount);
        break;
    }
	return algo;
}

Classifier *ClassBoost::GetClassifier()
{
	ClassifierBoost *classifier = new ClassifierBoost();
	SetParams(classifier);
	return classifier;
}

void ClassBoost::DrawModel(Canvas *canvas, QPainter &painter, Classifier *classifier)
{
	if(!classifier || !canvas) return;
	painter.setRenderHint(QPainter::Antialiasing, true);

	int posClass = 1;
	bool bUseMinMax = false;
    bUseMinMax = true;

	float resMin = FLT_MAX;
	float resMax = -FLT_MAX;
    std::vector<fvec> samples = canvas->data->GetSamples();

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

void ClassBoost::DrawInfo(Canvas *canvas, QPainter &painter, Classifier *classifier)
{
    if(!classifier || !canvas) return;
    painter.setRenderHint(QPainter::Antialiasing, true);

    bool bUseMinMax = false;
    bUseMinMax = true;

    float resMin = FLT_MAX;
    float resMax = -FLT_MAX;
    std::vector<fvec> samples = canvas->data->GetSamples();
    ivec labels = canvas->data->GetLabels();

    ClassifierBoost *boost = dynamic_cast<ClassifierBoost*>(classifier);
    if(!boost) return;
    fvec weights = boost->GetErrorWeights();
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
}

void ClassBoost::SaveOptions(QSettings &settings)
{
	settings.setValue("boostCount", params->boostCountSpin->value());
    settings.setValue("boostType", params->boostType->currentIndex());
    settings.setValue("boostLearnerType", params->boostLearnerType->currentIndex());
    settings.setValue("svmCount", params->svmCountSpin->value());
}

bool ClassBoost::LoadOptions(QSettings &settings)
{
    if(settings.contains("boostCount")) params->boostCountSpin->setValue(settings.value("boostCount").toFloat());
    if(settings.contains("boostType")) params->boostType->setCurrentIndex(settings.value("boostType").toInt());
    if(settings.contains("boostLearnerType")) params->boostLearnerType->setCurrentIndex(settings.value("boostLearnerType").toInt());
    if(settings.contains("svmCount")) params->svmCountSpin->setValue(settings.value("svmCount").toFloat());
    return true;
}

void ClassBoost::SaveParams(QTextStream &file)
{
	file << "classificationOptions" << ":" << "boostCount" << " " << params->boostCountSpin->value() << "\n";
    file << "classificationOptions" << ":" << "boostType" << " " << params->boostType->currentIndex() << "\n";
    file << "classificationOptions" << ":" << "boostLearnerType" << " " << params->boostLearnerType->currentIndex() << "\n";
    file << "classificationOptions" << ":" << "svmCount" << " " << params->svmCountSpin->value() << "\n";
}

bool ClassBoost::LoadParams(QString name, float value)
{
	if(name.endsWith("boostCount")) params->boostCountSpin->setValue((int)value);
    if(name.endsWith("boostType")) params->boostType->setCurrentIndex((int)value);
    if(name.endsWith("boostLearnerType")) params->boostLearnerType->setCurrentIndex((int)value);
    if(name.endsWith("svmCount")) params->svmCountSpin->setValue((int)value);
    return true;
}
