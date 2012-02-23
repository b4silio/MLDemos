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
#include "interfaceKMCluster.h"
#include "basicOpenCV.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>

using namespace std;

ClustKM::ClustKM()
{
    params = new Ui::ParametersKM();
    params->setupUi(widget = new QWidget());
    connect(params->kmeansMethodCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeOptions()));
    connect(params->kmeansNormCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeOptions()));
}

void ClustKM::ChangeOptions()
{
    int method = params->kmeansMethodCombo->currentIndex();
    switch(method)
    {
    case 0:
        params->kmeansBetaSpin->setVisible(false);
        params->kmeansBetaSpin->setEnabled(false);
        params->kmeansNormCombo->setEnabled(true);
        params->kmeansNormSpin->setEnabled(params->kmeansNormCombo->currentIndex() == 3);
        params->kmeansNormSpin->setVisible(params->kmeansNormCombo->currentIndex() == 3);
        break;
    case 1:
        params->kmeansBetaSpin->setVisible(true);
        params->kmeansBetaSpin->setEnabled(true);
        params->kmeansNormSpin->setVisible(false);
        params->kmeansNormSpin->setEnabled(false);
        params->kmeansNormCombo->setEnabled(false);
        break;
    }
}

void ClustKM::SetParams(Clusterer *clusterer)
{
    if(!clusterer) return;
    int clusters = params->kmeansClusterSpin->value();
    int power = params->kmeansNormSpin->value();
    int metrictype = params->kmeansNormCombo->currentIndex();
    float beta = params->kmeansBetaSpin->value();
    int method = params->kmeansMethodCombo->currentIndex();
    if (metrictype < 3) power = metrictype;
    ((ClustererKM *)clusterer)->SetParams(clusters, method, beta, power);
}

Clusterer *ClustKM::GetClusterer()
{
    ClustererKM *clusterer = new ClustererKM();
    SetParams(clusterer);
    return clusterer;
}

void ClustKM::DrawInfo(Canvas *canvas, QPainter &painter, Clusterer *clusterer)
{
    if(!canvas || !clusterer) return;
    painter.setRenderHint(QPainter::Antialiasing);
    ClustererKM * _kmeans = (ClustererKM*)clusterer;
    KMeansCluster *kmeans = _kmeans->kmeans;
    painter.setBrush(Qt::NoBrush);
    FOR(i, kmeans->GetClusters())
    {
        fvec mean = kmeans->GetMean(i);
        QPointF point = canvas->toCanvasCoords(mean);
        CvScalar color = CV::color[(i+1)%CV::colorCnt];
        painter.setPen(QPen(Qt::black, 12));
        painter.drawEllipse(point, 8, 8);
        painter.setPen(QPen(QColor(color.val[0],color.val[1],color.val[2]),4));
        painter.drawEllipse(point, 8, 8);
        //painter.setPen(QPen(Qt::white, 2));
        //painter.drawEllipse(point, 8, 8);
    }
}

void ClustKM::DrawModel(Canvas *canvas, QPainter &painter, Clusterer *clusterer)
{
    if(!canvas || !clusterer) return;
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

void ClustKM::SaveOptions(QSettings &settings)
{
    settings.setValue("kmeansBeta", params->kmeansBetaSpin->value());
    settings.setValue("kmeansCluster", params->kmeansClusterSpin->value());
    settings.setValue("kmeansMethod", params->kmeansMethodCombo->currentIndex());
    settings.setValue("kmeansPower", params->kmeansNormSpin->value());
    settings.setValue("kmeansNormCombo", params->kmeansNormCombo->currentIndex());
}

bool ClustKM::LoadOptions(QSettings &settings)
{
    if(settings.contains("kmeansBeta")) params->kmeansBetaSpin->setValue(settings.value("kmeansBeta").toFloat());
    if(settings.contains("kmeansCluster")) params->kmeansClusterSpin->setValue(settings.value("kmeansCluster").toFloat());
    if(settings.contains("kmeansMethod")) params->kmeansMethodCombo->setCurrentIndex(settings.value("kmeansMethod").toInt());
    if(settings.contains("kmeansPower")) params->kmeansNormSpin->setValue(settings.value("kmeansPower").toFloat());
    if(settings.contains("kmeansNormCombo")) params->kmeansNormCombo->setCurrentIndex(settings.value("kmeansNormCombo").toInt());
    ChangeOptions();
    return true;
}

void ClustKM::SaveParams(QTextStream &file)
{
    file << "clusterOptions" << ":" << "kmeansBeta" << " " << params->kmeansBetaSpin->value() << "\n";
    file << "clusterOptions" << ":" << "kmeansCluster" << " " << params->kmeansClusterSpin->value() << "\n";
    file << "clusterOptions" << ":" << "kmeansMethod" << " " << params->kmeansMethodCombo->currentIndex() << "\n";
    file << "clusterOptions" << ":" << "kmeansPower" << " " << params->kmeansNormSpin->value() << "\n";
    file << "clusterOptions" << ":" << "kmeansNormCombo" << " " << params->kmeansNormCombo->currentIndex() << "\n";
}

bool ClustKM::LoadParams(QString name, float value)
{
    if(name.endsWith("kmeansBeta")) params->kmeansBetaSpin->setValue(value);
    if(name.endsWith("kmeansCluster")) params->kmeansClusterSpin->setValue((int)value);
    if(name.endsWith("kmeansMethod")) params->kmeansMethodCombo->setCurrentIndex((int)value);
    if(name.endsWith("kmeansPower")) params->kmeansNormSpin->setValue((int)value);
    if(name.endsWith("kmeansNormCombo")) params->kmeansNormCombo->setCurrentIndex((int)value);
    ChangeOptions();
    return true;
}
