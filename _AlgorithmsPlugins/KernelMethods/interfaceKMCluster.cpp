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
#include <QPixmap>
#include <QDebug>
#include <QBitmap>
#include <QPainter>
#include <qcontour.h>

using namespace std;

ClustKM::ClustKM()
{
    params = new Ui::ParametersKM();
    params->setupUi(widget = new QWidget());
    connect(params->kmeansMethodCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeOptions()));
    connect(params->kmeansNormCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeOptions()));
    connect(params->kernelTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeOptions()));
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

        params->kernelTypeCombo->setEnabled(false);
        params->kernelDegSpin->setEnabled(false);
        params->kernelDegSpin->setVisible(false);
        params->kernelWidthSpin->setEnabled(false);
        params->kernelWidthSpin->setVisible(false);
        break;
    case 1:
        params->kmeansBetaSpin->setVisible(true);
        params->kmeansBetaSpin->setEnabled(true);
        params->kmeansNormSpin->setVisible(false);
        params->kmeansNormSpin->setEnabled(false);
        params->kmeansNormCombo->setEnabled(false);

        params->kernelTypeCombo->setEnabled(false);
        params->kernelDegSpin->setEnabled(false);
        params->kernelDegSpin->setVisible(false);
        params->kernelWidthSpin->setEnabled(false);
        params->kernelWidthSpin->setVisible(false);
        break;
    case 2:
    {
        params->kmeansBetaSpin->setVisible(false);
        params->kmeansBetaSpin->setEnabled(false);
        params->kmeansNormSpin->setVisible(false);
        params->kmeansNormSpin->setEnabled(false);
        params->kmeansNormCombo->setEnabled(false);
        params->kernelTypeCombo->setEnabled(true);
        switch(params->kernelTypeCombo->currentIndex())
        {
        case 0: // linear
            params->param1Label->setText("");
            params->param2Label->setText("");
            params->kernelDegSpin->setEnabled(false);
            params->kernelDegSpin->setVisible(false);
            params->kernelWidthSpin->setEnabled(false);
            params->kernelWidthSpin->setVisible(false);
            break;
        case 1: // poly
            params->kernelDegSpin->setEnabled(true);
            params->kernelDegSpin->setVisible(true);
            params->kernelWidthSpin->setEnabled(false);
            params->kernelWidthSpin->setVisible(false);
//            params->kernelDegSpin->setDecimals(1);
//            params->kernelDegSpin->setRange(1,100);
//            params->kernelDegSpin->setSingleStep(1);
//            params->kernelWidthSpin->setRange(-999,999);
//            params->kernelWidthSpin->setSingleStep(0.1);
//            params->param1Label->setText("Degree");
//            params->param2Label->setText("Offset");
            break;
        case 2: // RBF
            params->kernelDegSpin->setEnabled(false);
            params->kernelDegSpin->setVisible(false);
            params->kernelWidthSpin->setEnabled(true);
            params->kernelWidthSpin->setVisible(true);
//            params->kernelWidthSpin->setRange(0.001,999);
//            params->kernelWidthSpin->setSingleStep(0.01);
//            params->param2Label->setText("Width");
            break;
        }
    }
        break;
    }
}

void ClustKM::SetParams(Clusterer *clusterer)
{
    if(!clusterer) return;
    int method = params->kmeansMethodCombo->currentIndex();
    int clusters = params->kmeansClusterSpin->value();
    if(method == 2)
    {
        float kernelWidth = params->kernelWidthSpin->value();
        int kernelDegree = params->kernelDegSpin->value();
        int kernelType = params->kernelTypeCombo->currentIndex();
        float kernelOffset = params->kernelWidthSpin->value();
        ClustererKKM *clust = dynamic_cast<ClustererKKM*>(clusterer);
        if(!clust) return;
        clust->SetParams(clusters, kernelType, kernelWidth, kernelDegree, kernelOffset);
    }
    else
    {
        int power = params->kmeansNormSpin->value();
        int metrictype = params->kmeansNormCombo->currentIndex();
        float beta = params->kmeansBetaSpin->value();
        if (metrictype < 3) power = metrictype;
        ClustererKM *clust = dynamic_cast<ClustererKM*>(clusterer);
        if(!clust) return;
        clust->SetParams(clusters, method, beta, power);
    }
}

Clusterer *ClustKM::GetClusterer()
{
    Clusterer *clusterer;
    int method = params->kmeansMethodCombo->currentIndex();
    if(method == 2)
    {
        clusterer = new ClustererKKM();
    }
    else
    {
        clusterer = new ClustererKM();
    }
    SetParams(clusterer);
    return clusterer;
}

void ClustKM::DrawInfo(Canvas *canvas, QPainter &painter, Clusterer *clusterer)
{
    if(!canvas || !clusterer) return;
    painter.setRenderHint(QPainter::Antialiasing);
    ClustererKM * _kmeans = dynamic_cast<ClustererKM*>(clusterer);
    if(_kmeans) // standardk-means!
    {
        KMeansCluster *kmeans = _kmeans->kmeans;
        painter.setBrush(Qt::NoBrush);
        FOR(i, kmeans->GetClusters())
        {
            fvec mean = kmeans->GetMean(i);
            QPointF point = canvas->toCanvasCoords(mean);

            QColor color = SampleColor[(i+1)%SampleColorCnt];
            painter.setPen(QPen(Qt::black, 12));
            painter.drawEllipse(point, 8, 8);
            painter.setPen(QPen(color,4));
            painter.drawEllipse(point, 8, 8);
        }
        return;
    }
    ClustererKKM * _kkmeans = dynamic_cast<ClustererKKM*>(clusterer);
    if(!_kkmeans) return;
    int W = painter.viewport().width();
    int H = painter.viewport().height();
    int w = 129;
    int h = 129;
    int nbClusters = _kkmeans->NbClusters();
    double **valueList = new double*[nbClusters];
    FOR(i, nbClusters)
    {
        valueList[i] = new double[w*h];
    }
    int dim = canvas->data->GetDimCount();
    FOR(k, nbClusters)
    {
        FOR(i, w)
        {
            FOR(j, h)
            {
                int x = i*W/w;
                int y = j*H/h;
                fvec sample = canvas->fromCanvas(x,y);
                double value = _kkmeans->TestScore(sample, k);
                // to avoid some numerical weird stuff
                value = value*1000.;
                valueList[k][j*w + i] = value;
            }
        }
    }
    FOR(k, nbClusters)
    {

        QContour contour(valueList[k], w, h);
        contour.plotColor = SampleColor[(k+1)%SampleColorCnt];
        contour.plotThickness = 2;
        double vmin, vmax;
        contour.GetLimits(vmin, vmax);
        vmin += (vmax - vmin)/4; // we take out the smallest levels to avoid numerical issues
        contour.SetLimits(vmin, vmax);
        contour.Paint(painter, 10);
        /*
        QImage image(w,h,QImage::Format_RGB32);
        double vmin, vmax;
        contour.GetLimits(vmin, vmax);
        double vdiff = vmax - vmin;
        FOR(i, w)
        {
            FOR(j, h)
            {
                int value = (int)((valueList[k][j*w + i]-vmin)/vdiff*255);
                value = max(0,min(255,value));
                image.setPixel(i,j, qRgb((int)value,value,value));
            }
        }
        QPixmap contourPixmap = QPixmap::fromImage(image).scaled(512,512, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        QLabel *lab = new QLabel();
        lab->setPixmap(contourPixmap);
        lab->show();
        */
        delete [] valueList[k];
        valueList[k] = 0;
    }
    delete [] valueList;
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
        painter.setBrush(QColor(max(0.f,min(255.f,r)),max(0.f,min(255.f,g)),max(0.f,min(255.f,b))));
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
    settings.setValue("kernelDeg", params->kernelDegSpin->value());
    settings.setValue("kernelType", params->kernelTypeCombo->currentIndex());
    settings.setValue("kernelWidth", params->kernelWidthSpin->value());
}

bool ClustKM::LoadOptions(QSettings &settings)
{
    if(settings.contains("kmeansBeta")) params->kmeansBetaSpin->setValue(settings.value("kmeansBeta").toFloat());
    if(settings.contains("kmeansCluster")) params->kmeansClusterSpin->setValue(settings.value("kmeansCluster").toFloat());
    if(settings.contains("kmeansMethod")) params->kmeansMethodCombo->setCurrentIndex(settings.value("kmeansMethod").toInt());
    if(settings.contains("kmeansPower")) params->kmeansNormSpin->setValue(settings.value("kmeansPower").toFloat());
    if(settings.contains("kmeansNormCombo")) params->kmeansNormCombo->setCurrentIndex(settings.value("kmeansNormCombo").toInt());
    if(settings.contains("kernelDeg")) params->kernelDegSpin->setValue(settings.value("kernelDeg").toFloat());
    if(settings.contains("kernelType")) params->kernelTypeCombo->setCurrentIndex(settings.value("kernelType").toInt());
    if(settings.contains("kernelWidth")) params->kernelWidthSpin->setValue(settings.value("kernelWidth").toFloat());
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
    file << "clusterOptions" << ":" << "kernelDeg" << " " << params->kernelDegSpin->value() << "\n";
    file << "clusterOptions" << ":" << "kernelType" << " " << params->kernelTypeCombo->currentIndex() << "\n";
    file << "clusterOptions" << ":" << "kernelWidth" << " " << params->kernelWidthSpin->value() << "\n";
}

bool ClustKM::LoadParams(QString name, float value)
{
    if(name.endsWith("kmeansBeta")) params->kmeansBetaSpin->setValue(value);
    if(name.endsWith("kmeansCluster")) params->kmeansClusterSpin->setValue((int)value);
    if(name.endsWith("kmeansMethod")) params->kmeansMethodCombo->setCurrentIndex((int)value);
    if(name.endsWith("kmeansPower")) params->kmeansNormSpin->setValue((int)value);
    if(name.endsWith("kmeansNormCombo")) params->kmeansNormCombo->setCurrentIndex((int)value);
    if(name.endsWith("kernelDeg")) params->kernelDegSpin->setValue((int)value);
    if(name.endsWith("kernelType")) params->kernelTypeCombo->setCurrentIndex((int)value);
    if(name.endsWith("kernelWidth")) params->kernelWidthSpin->setValue(value);
    ChangeOptions();
    return true;
}
