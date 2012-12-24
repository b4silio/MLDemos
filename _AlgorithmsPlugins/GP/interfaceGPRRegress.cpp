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
    connect(params->optimizeCheck, SIGNAL(clicked()), this, SLOT(ChangeOptions()));
    ChangeOptions();
}

void RegrGPR::ChangeOptions()
{
    bool bSparse = params->sparseCheck->isChecked();

    params->capacitySpin->setVisible(bSparse);
    params->labelCapacity->setVisible(bSparse);
    params->optimizeCombo->setVisible(params->optimizeCheck->isChecked());

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

void RegrGPR::SetParams(Regressor *regressor)
{
    if(!regressor) return;
    RegressorGPR *gpr = dynamic_cast<RegressorGPR*>(regressor);
    if(!gpr) return;

    int kernelType = params->kernelTypeCombo->currentIndex();
    float kernelGamma = params->kernelWidthSpin->value();
    float kernelDegree = params->kernelDegSpin->value();
    int capacity = params->capacitySpin->value();
    if(!params->sparseCheck->isChecked()) capacity = -1;
    double kernelNoise = params->noiseSpin->value();
    bool bOptimize = params->optimizeCheck->isChecked();
    bool bUseLikelihood = params->optimizeCombo->currentIndex() == 0;

    gpr->SetParams(kernelGamma, kernelNoise, capacity, kernelType, kernelDegree, bOptimize, bUseLikelihood);
}

fvec RegrGPR::GetParams()
{
    int kernelType = params->kernelTypeCombo->currentIndex();
    float kernelGamma = params->kernelWidthSpin->value();
    float kernelDegree = params->kernelDegSpin->value();
    int capacity = params->capacitySpin->value();
    bool bSparse = params->sparseCheck->isChecked();
    double kernelNoise = params->noiseSpin->value();
    bool bOptimize = params->optimizeCheck->isChecked();
    bool bUseLikelihood = params->optimizeCombo->currentIndex() == 0;
    fvec par(7);
    par[0] = kernelType;
    par[1] = kernelGamma;
    par[2] = kernelDegree;
    par[3] = capacity;
    par[4] = kernelNoise;
    par[5] = bOptimize;
    par[6] = bUseLikelihood;
    return par;
}

void RegrGPR::SetParams(Regressor *regressor, fvec parameters)
{
    if(!regressor) return;
    RegressorGPR *gpr = dynamic_cast<RegressorGPR*>(regressor);
    if(!gpr) return;

    int i = 0;
    int kernelType = parameters.size() > i ? parameters[i] : 0; i++;
    float kernelGamma = parameters.size() > i ? parameters[i] : 0; i++;
    float kernelDegree = parameters.size() > i ? parameters[i] : 0; i++;
    int capacity = parameters.size() > i ? parameters[i] : 0; i++;
    bool bSparse = parameters.size() > i ? parameters[i] : 0; i++;
    double kernelNoise = parameters.size() > i ? parameters[i] : 0; i++;
    bool bOptimize = parameters.size() > i ? parameters[i] : 0; i++;
    bool bUseLikelihood = parameters.size() > i ? parameters[i] : 0; i++;
    if(bSparse) capacity = -1;
    gpr->SetParams(kernelGamma, kernelNoise, capacity, kernelType, kernelDegree, bOptimize, bUseLikelihood);
}

void RegrGPR::GetParameterList(std::vector<QString> &parameterNames,
                             std::vector<QString> &parameterTypes,
                             std::vector< std::vector<QString> > &parameterValues)
{
    parameterNames.clear();
    parameterTypes.clear();
    parameterValues.clear();
    parameterNames.push_back("kernelType");
    parameterNames.push_back("kernelGamma");
    parameterNames.push_back("kernelDegree");
    parameterNames.push_back("capacity");
    parameterNames.push_back("bSparse");
    parameterNames.push_back("kernelNoise");
    parameterNames.push_back("bOptimize");
    parameterNames.push_back("bUseLikelihood");
    parameterTypes.push_back("List");
    parameterTypes.push_back("Real");
    parameterTypes.push_back("Integer");
    parameterTypes.push_back("Integer");
    parameterTypes.push_back("List");
    parameterTypes.push_back("Real");
    parameterTypes.push_back("List");
    parameterTypes.push_back("List");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("Linear");
    parameterValues.back().push_back("Poly");
    parameterValues.back().push_back("RBF");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("0.00000001f");
    parameterValues.back().push_back("99999999999");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("1");
    parameterValues.back().push_back("150");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("1");
    parameterValues.back().push_back("9999999");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("False");
    parameterValues.back().push_back("True");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("0.0000000001f");
    parameterValues.back().push_back("999999999999");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("False");
    parameterValues.back().push_back("True");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("False");
    parameterValues.back().push_back("True");
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
    painter.setPen(QPen(Qt::red,3));
    FOR(i, gpr->GetBasisCount())
    {
        fvec basis = gpr->GetBasisVector(i);
        fvec testBasis(dim+1);
        FOR(d, dim) testBasis[d] = basis[d];
        fvec res = gpr->Test(testBasis);
        // we draw the basis circle
        float conf = basis[dim + xIndex];
        QPointF pt1 = canvas->toCanvasCoords(basis[xIndex],res[0]);
        painter.drawEllipse(pt1, radius, radius);
        // and the arrow of the direction
        QPointF pt2 = pt1 + QPointF(0,(conf>0 ? 1 : -1)*radius);
        QPointF pt3 = pt2 + QPointF(0,(conf>0 ? 1 : -1)*25*(0.5+min(2.f,fabs(conf)/5.f)));
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
    QPainterPath path, pathUp, pathDown;
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
            path.lineTo(point);
            pathUp.lineTo(pointUp);
            pathDown.lineTo(pointDown);
        }
        else
        {
            path.moveTo(point);
            pathUp.moveTo(pointUp);
            pathDown.moveTo(pointDown);
        }
        oldPoint = point;
        oldPointUp = pointUp;
        oldPointDown = pointDown;
    }
    painter.setBackgroundMode(Qt::TransparentMode);
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(Qt::black, 1));
    painter.drawPath(path);
    painter.setPen(QPen(Qt::black, 0.5, Qt::DashLine));
    painter.drawPath(pathUp);
    painter.drawPath(pathDown);
}

void RegrGPR::SaveOptions(QSettings &settings)
{
    settings.setValue("kernelDeg", params->kernelDegSpin->value());
    settings.setValue("kernelType", params->kernelTypeCombo->currentIndex());
    settings.setValue("kernelWidth", params->kernelWidthSpin->value());
    settings.setValue("capacitySpin", params->capacitySpin->value());
    settings.setValue("noiseSpin", params->noiseSpin->value());
    settings.setValue("sparseCheck", params->sparseCheck->isChecked());
    settings.setValue("optimizeCheck", params->optimizeCheck->isChecked());
    settings.setValue("optimizeCombo", params->optimizeCombo->currentIndex());
}

bool RegrGPR::LoadOptions(QSettings &settings)
{
    if(settings.contains("kernelDeg")) params->kernelDegSpin->setValue(settings.value("kernelDeg").toFloat());
    if(settings.contains("kernelType")) params->kernelTypeCombo->setCurrentIndex(settings.value("kernelType").toInt());
    if(settings.contains("kernelWidth")) params->kernelWidthSpin->setValue(settings.value("kernelWidth").toFloat());
    if(settings.contains("capacity")) params->capacitySpin->setValue(settings.value("capacitySpin").toInt());
    if(settings.contains("noiseSpin")) params->noiseSpin->setValue(settings.value("noiseSpin").toFloat());
    if(settings.contains("sparseCheck")) params->sparseCheck->setChecked(settings.value("sparseCheck").toBool());
    if(settings.contains("optimizeCheck")) params->optimizeCheck->setChecked(settings.value("optimizeCheck").toBool());
    if(settings.contains("optimizeCombo")) params->optimizeCombo->setCurrentIndex(settings.value("optimizeCombo").toInt());
    return true;
}

void RegrGPR::SaveParams(QTextStream &file)
{
    file << "regressionOptions" << ":" << "kernelDeg" << " " << params->kernelDegSpin->value() << "\n";
    file << "regressionOptions" << ":" << "kernelType" << " " << params->kernelTypeCombo->currentIndex() << "\n";
    file << "regressionOptions" << ":" << "kernelWidth" << " " << params->kernelWidthSpin->value() << "\n";
    file << "regressionOptions" << ":" << "capacitySpin" << " " << params->capacitySpin->value() << "\n";
    file << "regressionOptions" << ":" << "noiseSpin" << " " << params->noiseSpin->value() << "\n";
    file << "regressionOptions" << ":" << "sparseCheck" << " " << params->sparseCheck->isChecked() << "\n";
    file << "regressionOptions" << ":" << "optimizeCheck" << " " << params->optimizeCheck->isChecked() << "\n";
    file << "regressionOptions" << ":" << "optimizeCombo" << " " << params->optimizeCombo->currentIndex() << "\n";
}

bool RegrGPR::LoadParams(QString name, float value)
{
    if(name.endsWith("kernelDeg")) params->kernelDegSpin->setValue((int)value);
    if(name.endsWith("kernelType")) params->kernelTypeCombo->setCurrentIndex((int)value);
    if(name.endsWith("kernelWidth")) params->kernelWidthSpin->setValue(value);
    if(name.endsWith("capacitySpin")) params->capacitySpin->setValue(value);
    if(name.endsWith("noiseSpin")) params->noiseSpin->setValue(value);
    if(name.endsWith("sparseCheck")) params->sparseCheck->setChecked((int)value);
    if(name.endsWith("optimizeCheck")) params->optimizeCheck->setChecked((int)value);
    if(name.endsWith("optimizeCombo")) params->optimizeCombo->setCurrentIndex((int)value);
    return true;
}
