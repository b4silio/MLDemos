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
#include "interfaceGPRRegress.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>

using namespace std;

RegrGPR::RegrGPR()
{
    params = new Ui::ParametersGPRregr();
    params->setupUi(widget = new QWidget());
    connect(params->kernelTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeOptions()));
    connect(params->sparseCheck, SIGNAL(clicked()), this, SLOT(ChangeOptions()));
}

void RegrGPR::ChangeOptions()
{
    bool bSparse = params->sparseCheck->isChecked();

    params->capacitySpin->setEnabled(bSparse);
    params->noiseSpin->setEnabled(bSparse);

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

void RegrGPR::SetParams(Regressor *regressor)
{
    if(!regressor) return;
    RegressorGPR *gpr = dynamic_cast<RegressorGPR*>(regressor);
    if(!gpr) return;

    int kernelType = params->kernelTypeCombo->currentIndex();
    float kernelGamma = params->kernelWidthSpin->value();
    float kernelDegree = params->kernelDegSpin->value();
    int capacity = params->capacitySpin->value();
    double kernelNoise = params->noiseSpin->value();

    gpr->SetParams(kernelGamma, kernelNoise, capacity, kernelType, kernelDegree);
}

QString RegrGPR::GetAlgoString()
{
    float capacity = params->capacitySpin->value();
    float noise = params->noiseSpin->value();
    int kernelType = params->kernelTypeCombo->currentIndex();
    float kernelGamma = params->kernelWidthSpin->value();
    float kernelDegree = params->kernelDegSpin->value();

    QString algo;
    algo += "SOGP";
    algo += QString(" %1 %2").arg(capacity).arg(noise);
    switch(kernelType)
    {
    case 0:
        algo += " L";
        break;
    case 1:
        algo += QString(" P %1").arg(kernelDegree);
        break;
    case 2:
        algo += QString(" R %1").arg(kernelGamma);
        break;
    }
    return algo;
}

Regressor *RegrGPR::GetRegressor()
{
    Regressor *regressor = 0;
    regressor = new RegressorGPR();
    SetParams(regressor);
    return regressor;
}

void DrawArrow( const QPointF &ppt, const QPointF &pt, double sze, QPainter &painter)
{
    QPointF pd, pa, pb;
    double tangent;

    pd = ppt - pt;
    if (pd.x() == 0 && pd.y() == 0)
        return;
    tangent = atan2 ((double) pd.y(), (double) pd.x());
    pa.setX(sze * cos (tangent + M_PI / 7.f) + pt.x());
    pa.setY(sze * sin (tangent + M_PI / 7.f) + pt.y());
    pb.setX(sze * cos (tangent - M_PI / 7.f) + pt.x());
    pb.setY(sze * sin (tangent - M_PI / 7.f) + pt.y());
    //-- connect the dots...
    painter.drawLine(pt, ppt);
    painter.drawLine(pt, pa);
    painter.drawLine(pt, pb);
}

void RegrGPR::DrawInfo(Canvas *canvas, QPainter &painter, Regressor *regressor)
{
    RegressorGPR * gpr = dynamic_cast<RegressorGPR*>(regressor);
    if(!gpr) return;
    painter.setRenderHint(QPainter::Antialiasing);
    int xIndex = canvas->xIndex;
    int yIndex = canvas->yIndex;

    int radius = 8;
    int dim = canvas->data->GetDimCount()-1;
    painter.setBrush(Qt::NoBrush);
    FOR(i, gpr->GetBasisCount())
    {
        fvec basis = gpr->GetBasisVector(i);
        fvec testBasis(dim+1);
        FOR(d, dim) testBasis[d] = basis[d];
        fvec res = gpr->Test(testBasis);
        QPointF pt1 = canvas->toCanvasCoords(basis[xIndex],res[0]);
        QPointF pt2 = pt1 + QPointF(0,(basis[dim + xIndex]>0 ? 1 : -1)*radius);
        QPointF pt3 = pt2 + QPointF(0,(basis[dim + xIndex]>0 ? 1 : -1)*50);
        painter.setPen(QPen(Qt::red,3));
        painter.drawEllipse(pt1, radius, radius);
        painter.setPen(QPen(Qt::red,min(4.f,max(fabs(basis[dim + xIndex])/5,0.5f))));
        DrawArrow(pt2,pt3,10,painter);
    }
}

void RegrGPR::DrawConfidence(Canvas *canvas, Regressor *regressor)
{

    RegressorGPR *gpr = dynamic_cast<RegressorGPR *>(regressor);
    if(gpr && gpr->sogp)
    {
        int w = canvas->width();
        int h = canvas->height();
        int dim = canvas->data->GetDimCount()-1;
        int outputDim = regressor->outputDim;
        int xIndex = canvas->xIndex;
        int yIndex = canvas->yIndex;
        Matrix _testout;
        ColumnVector _testin(dim);
        QImage density(QSize(256,256), QImage::Format_RGB32);
        density.fill(0);
        // we draw a density map for the probability
        for (int i=0; i < density.width(); i++)
        {
            fvec sampleIn = canvas->toSampleCoords(i*w/density.width(),0);
            FOR(d, dim) _testin(d+1) = sampleIn[d];
            if(outputDim != -1 && outputDim < dim) _testin(outputDim+1) = sampleIn[dim];
            double sigma;
            _testout = gpr->sogp->predict(_testin, sigma);
            sigma = sigma*sigma;
            float testout = _testout(1,1);
            for (int j=0; j< density.height(); j++)
            {
                fvec sampleOut = canvas->toSampleCoords(i*w/density.width(),j*h/density.height());
                float val = gpr->GetLikelihood(testout, sigma, sampleOut[yIndex]);
                int color = min(255,(int)(128 + val*20));
                density.setPixel(i,j, qRgb(color,color,color));
            }
        }
        canvas->maps.confidence = QPixmap::fromImage(density.scaled(QSize(w,h),Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    }
    else canvas->maps.confidence = QPixmap();
}

void RegrGPR::DrawModel(Canvas *canvas, QPainter &painter, Regressor *regressor)
{
    painter.setRenderHint(QPainter::Antialiasing, true);
    int w = canvas->width();
    int h = canvas->height();
    int xIndex = canvas->xIndex;
    fvec sample = canvas->toSampleCoords(0,0);
    int dim = sample.size();
    if(dim > 2) return;

    RegressorGPR * gpr = dynamic_cast<RegressorGPR*>(regressor);
    if(!gpr) return;

    int steps = w;
    QPointF oldPoint(-FLT_MAX,-FLT_MAX);
    QPointF oldPointUp(-FLT_MAX,-FLT_MAX);
    QPointF oldPointDown(-FLT_MAX,-FLT_MAX);
    FOR(x, steps)
    {
        sample = canvas->toSampleCoords(x,0);
        fvec res = regressor->Test(sample);
        if(res[0] != res[0] || res[1] != res[1]) continue;
        QPointF point = canvas->toCanvasCoords(sample[xIndex], res[0]);
        QPointF pointUp = canvas->toCanvasCoords(sample[xIndex],res[0] + res[1]);
        QPointF pointDown = canvas->toCanvasCoords(sample[xIndex],res[0] - res[1]);
        if(x)
        {
            painter.setPen(QPen(Qt::black, 1));
            painter.drawLine(point, oldPoint);
            painter.setPen(QPen(Qt::black, 0.5));
            painter.drawLine(pointUp, oldPointUp);
            painter.drawLine(pointDown, oldPointDown);
        }
        oldPoint = point;
        oldPointUp = pointUp;
        oldPointDown = pointDown;
    }
}

void RegrGPR::SaveOptions(QSettings &settings)
{
    settings.setValue("kernelDeg", params->kernelDegSpin->value());
    settings.setValue("kernelType", params->kernelTypeCombo->currentIndex());
    settings.setValue("kernelWidth", params->kernelWidthSpin->value());
    settings.setValue("capacitySpin", params->capacitySpin->value());
    settings.setValue("noiseSpin", params->noiseSpin->value());
}

bool RegrGPR::LoadOptions(QSettings &settings)
{
    if(settings.contains("kernelDeg")) params->kernelDegSpin->setValue(settings.value("kernelDeg").toFloat());
    if(settings.contains("kernelType")) params->kernelTypeCombo->setCurrentIndex(settings.value("kernelType").toInt());
    if(settings.contains("kernelWidth")) params->kernelWidthSpin->setValue(settings.value("kernelWidth").toFloat());
    if(settings.contains("capacity")) params->capacitySpin->setValue(settings.value("capacitySpin").toInt());
    if(settings.contains("noiseSpin")) params->noiseSpin->setValue(settings.value("noiseSpin").toFloat());
    return true;
}

void RegrGPR::SaveParams(QTextStream &file)
{
    file << "regressionOptions" << ":" << "kernelDeg" << " " << params->kernelDegSpin->value() << "\n";
    file << "regressionOptions" << ":" << "kernelType" << " " << params->kernelTypeCombo->currentIndex() << "\n";
    file << "regressionOptions" << ":" << "kernelWidth" << " " << params->kernelWidthSpin->value() << "\n";
    file << "regressionOptions" << ":" << "capacitySpin" << " " << params->capacitySpin->value() << "\n";
    file << "regressionOptions" << ":" << "noiseSpin" << " " << params->noiseSpin->value() << "\n";
}

bool RegrGPR::LoadParams(QString name, float value)
{
    if(name.endsWith("kernelDeg")) params->kernelDegSpin->setValue((int)value);
    if(name.endsWith("kernelType")) params->kernelTypeCombo->setCurrentIndex((int)value);
    if(name.endsWith("kernelWidth")) params->kernelWidthSpin->setValue(value);
    if(name.endsWith("capacitySpin")) params->capacitySpin->setValue(value);
    if(name.endsWith("noiseSpin")) params->noiseSpin->setValue(value);
    return true;
}
