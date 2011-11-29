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
#include "interfaceSVMRegress.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>

using namespace std;

RegrSVM::RegrSVM()
{
	params = new Ui::ParametersRegr();
	params->setupUi(widget = new QWidget());
    connect(params->svmTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeOptions()));
    connect(params->kernelTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeOptions()));
}

void RegrSVM::ChangeOptions()
{
	params->svmCLabel->setText("C");
    params->svmPSpin->setRange(0.0001, 1.0);
	params->svmPSpin->setSingleStep(0.01);
	params->svmPSpin->setDecimals(4);
	params->svmCSpin->setEnabled(true);
	params->svmCSpin->setRange(0.1, 9999.9);
	params->svmCSpin->setDecimals(1);
	switch(params->svmTypeCombo->currentIndex())
	{
	case 0: // C-SVM
		params->svmEpsLabel->setText("eps");
        params->svmPSpin->setRange(0.0001, 100.0);
        break;
	case 1: // Nu-SVM
		params->svmEpsLabel->setText("Nu");
		break;
	case 2: // RVM
		params->svmCSpin->setEnabled(false);
		params->svmEpsLabel->setText("eps");
		break;
	case 3: // SOGP
		params->svmEpsLabel->setText("Noise");
		params->svmCLabel->setText("Capacity");
		params->svmCSpin->setRange(-1, 500);
		params->svmCSpin->setDecimals(0);
		params->svmPSpin->setRange(0.001, 1.0);
		params->svmPSpin->setSingleStep(0.01);
		params->svmPSpin->setDecimals(3);
		break;
	case 4:
		params->svmEpsLabel->setText("Tolerance");
		params->svmCLabel->setText("Capacity");
		params->svmCSpin->setRange(0, 1000);
		params->svmCSpin->setDecimals(0);
		params->svmPSpin->setRange(0.0001, 1.0);
		params->svmPSpin->setSingleStep(0.001);
		params->svmPSpin->setDecimals(4);
		break;
	}
    switch(params->kernelTypeCombo->currentIndex())
    {
    case 0: // linear
        params->kernelDegSpin->setEnabled(false);
        break;
    case 1: // poly
        params->kernelDegSpin->setEnabled(true);
        params->kernelWidthSpin->setEnabled(true);
        break;
    case 2: // RBF
        params->kernelDegSpin->setEnabled(false);
        params->kernelWidthSpin->setEnabled(true);
        break;
    }
}

void RegrSVM::SetParams(Regressor *regressor)
{
	if(!regressor) return;
	int kernelMethod = params->svmTypeCombo->currentIndex();
	float svmC = params->svmCSpin->value();
	int kernelType = params->kernelTypeCombo->currentIndex();
	float kernelGamma = params->kernelWidthSpin->value();
	float kernelDegree = params->kernelDegSpin->value();
	float svmP = params->svmPSpin->value();

	if(kernelMethod == 2) // rvm
	{
		RegressorRVM *rvm = (RegressorRVM*)regressor;
		rvm->SetParams(svmP, kernelType, kernelGamma, kernelDegree);
	}
	else if(kernelMethod == 3) // sogp
	{
		RegressorGPR *gpr = (RegressorGPR*)regressor;
		int capacity = svmC;
		double kernelNoise = svmP;
		gpr->SetParams(kernelGamma, kernelNoise, capacity, kernelType, kernelDegree);
	}
	else if(kernelMethod == 4 ) // KRLS
	{
		RegressorKRLS *krls = (RegressorKRLS*)regressor;
		int capacity = svmC;
		double epsilon = svmP;
		krls->SetParams(epsilon, capacity, kernelType, kernelGamma, kernelDegree);
	}
	else
	{
		RegressorSVR *svm = (RegressorSVR*)regressor;
		switch(kernelMethod)
		{
		case 0:
			svm->param.svm_type = EPSILON_SVR;
			break;
		case 1:
			svm->param.svm_type = NU_SVR;
			break;
		}
		switch(kernelType)
		{
		case 0:
			svm->param.kernel_type = LINEAR;
			break;
		case 1:
			svm->param.kernel_type = POLY;
			break;
		case 2:
			svm->param.kernel_type = RBF;
			break;
		}
		svm->param.C = svmC;
		svm->param.nu = svmP;
		svm->param.p = svmP;
		svm->param.gamma = 1 / kernelGamma;
		svm->param.degree = kernelDegree;
	}
}

QString RegrSVM::GetAlgoString()
{
	int kernelMethod = params->svmTypeCombo->currentIndex();
	float svmC = params->svmCSpin->value();
	int kernelType = params->kernelTypeCombo->currentIndex();
	float kernelGamma = params->kernelWidthSpin->value();
	float kernelDegree = params->kernelDegSpin->value();
	float svmP = params->svmPSpin->value();

	QString algo;
	switch(kernelMethod)
	{
	case 0:
		algo += "eps-SVM";
		algo += QString(" %1 %2").arg(svmC).arg(svmP);
		break;
	case 1:
		algo += "nu-SVM";
		algo += QString(" %1 %2").arg(svmC).arg(svmP);
		break;
	case 2:
		algo += "RVM";
		algo += QString(" %1").arg(svmP);
		break;
	case 3:
		algo += "SOGP";
		algo += QString(" %1 %2").arg(svmC).arg(svmP);
		break;
	case 4:
		algo += "KRLS";
		algo += QString(" %1 %2").arg(svmC).arg(svmP);
		break;
	}
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

Regressor *RegrSVM::GetRegressor()
{
	int svmType = params->svmTypeCombo->currentIndex();
	Regressor *regressor = 0;
	switch(svmType)
	{
	case 2:
		regressor = new RegressorRVM();
		break;
	case 3:
		regressor = new RegressorGPR();
		break;
	case 4:
		regressor = new RegressorKRLS();
		break;
	default:
		regressor = new RegressorSVR();
		break;
	}
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

void RegrSVM::DrawInfo(Canvas *canvas, QPainter &painter, Regressor *regressor)
{
	painter.setRenderHint(QPainter::Antialiasing);
    int xIndex = canvas->xIndex;
    int yIndex = canvas->yIndex;
    if(regressor->type == REGR_RVM || regressor->type == REGR_KRLS)
	{
		vector<fvec> sv = (regressor->type == REGR_KRLS) ?
				((RegressorKRLS*)regressor)->GetSVs() :
				((RegressorRVM*)regressor)->GetSVs();
		int radius = 9;
		painter.setBrush(Qt::NoBrush);
		FOR(i, sv.size())
		{
			QPointF point = canvas->toCanvasCoords(sv[i]);
			painter.setPen(QPen(Qt::black,6));
			painter.drawEllipse(point, radius, radius);
			painter.setPen(QPen(Qt::white,3));
			painter.drawEllipse(point, radius, radius);
		}
	}
	else if(regressor->type == REGR_SVR)
	{
		// we want to draw the support vectors
		svm_model *svm = ((RegressorSVR*)regressor)->GetModel();
		if(svm)
		{
			painter.setBrush(Qt::NoBrush);
			std::vector<fvec> samples = canvas->data->GetSamples();
            int dim = canvas->data->GetDimCount();
            fvec sv(2,0);
			FOR(i, svm->l)
			{
                sv[0] = (f32)svm->SV[i][xIndex].value;
				FOR(j, samples.size())
				{
                    if(sv[0] == samples[j][xIndex])
					{
                        sv[1] = samples[j][yIndex];
						break;
					}
				}
				int radius = 7;
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
	else if(regressor->type == REGR_GPR)
	{
		RegressorGPR * gpr = (RegressorGPR*)regressor;
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
}

void RegrSVM::DrawConfidence(Canvas *canvas, Regressor *regressor)
{
	if(regressor->type == REGR_GPR)
	{
		RegressorGPR *gpr = (RegressorGPR *)regressor;
		if(gpr->sogp)
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
	else canvas->maps.confidence = QPixmap();
}

void RegrSVM::DrawModel(Canvas *canvas, QPainter &painter, Regressor *regressor)
{
	painter.setRenderHint(QPainter::Antialiasing, true);
	int w = canvas->width();
	int h = canvas->height();
    int xIndex = canvas->xIndex;
    fvec sample = canvas->toSampleCoords(0,0);
    int dim = sample.size();
    if(dim > 2) return;
	if(regressor->type == REGR_KRLS || regressor->type == REGR_RVM)
	{
		canvas->maps.confidence = QPixmap();
		int steps = w;
		QPointF oldPoint(-FLT_MAX,-FLT_MAX);
		FOR(x, steps)
		{
			sample = canvas->toSampleCoords(x,0);
			fvec res = regressor->Test(sample);
			if(res[0] != res[0]) continue;
            QPointF point = canvas->toCanvasCoords(sample[xIndex], res[0]);
			if(x)
			{
				painter.setPen(QPen(Qt::black, 1));
				painter.drawLine(point, oldPoint);
				painter.setPen(QPen(Qt::black, 0.5));
				//				painter.drawLine(point+QPointF(0,eps*h), oldPoint+QPointF(0,eps*h));
				//				painter.drawLine(point-QPointF(0,eps*h), oldPoint-QPointF(0,eps*h));
			}
			oldPoint = point;
		}
	}
	else if(regressor->type == REGR_SVR)
	{
		canvas->maps.confidence = QPixmap();
		svm_parameter params = ((RegressorSVR *)regressor)->param;

		float eps = params.p;
		if(params.svm_type == NU_SVR) eps = ((RegressorSVR *)regressor)->GetModel()->eps[0];
		eps = fabs((canvas->toCanvasCoords(eps,0) - canvas->toCanvasCoords(0,0)).x());

		int steps = w;
		QPointF oldPoint(-FLT_MAX,-FLT_MAX);
		FOR(x, steps)
		{
			sample = canvas->toSampleCoords(x,0);
            int dim = sample.size();
			fvec res = regressor->Test(sample);
			if(res[0] != res[0]) continue;
            QPointF point = canvas->toCanvasCoords(sample[xIndex], res[0]);
			if(x)
			{
				painter.setPen(QPen(Qt::black, 1));
				painter.drawLine(point, oldPoint);
				painter.setPen(QPen(Qt::black, 0.5));
				painter.drawLine(point+QPointF(0,eps), oldPoint+QPointF(0,eps));
				painter.drawLine(point-QPointF(0,eps), oldPoint-QPointF(0,eps));
			}
			oldPoint = point;
		}
	}
	else if(regressor->type == REGR_GPR)
	{
		RegressorGPR *gpr = (RegressorGPR *)regressor;
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
}

void RegrSVM::SaveOptions(QSettings &settings)
{
	settings.setValue("kernelDeg", params->kernelDegSpin->value());
	settings.setValue("kernelType", params->kernelTypeCombo->currentIndex());
	settings.setValue("kernelWidth", params->kernelWidthSpin->value());
	settings.setValue("svmC", params->svmCSpin->value());
	settings.setValue("svmP", params->svmPSpin->value());
	settings.setValue("svmType", params->svmTypeCombo->currentIndex());
}

bool RegrSVM::LoadOptions(QSettings &settings)
{
	if(settings.contains("kernelDeg")) params->kernelDegSpin->setValue(settings.value("kernelDeg").toFloat());
	if(settings.contains("kernelType")) params->kernelTypeCombo->setCurrentIndex(settings.value("kernelType").toInt());
	if(settings.contains("kernelWidth")) params->kernelWidthSpin->setValue(settings.value("kernelWidth").toFloat());
	if(settings.contains("svmC")) params->svmCSpin->setValue(settings.value("svmC").toFloat());
	if(settings.contains("svmP")) params->svmPSpin->setValue(settings.value("svmP").toFloat());
	if(settings.contains("svmType")) params->svmTypeCombo->setCurrentIndex(settings.value("svmType").toInt());
	return true;
}

void RegrSVM::SaveParams(QTextStream &file)
{
	file << "regressionOptions" << ":" << "kernelDeg" << " " << params->kernelDegSpin->value() << "\n";
	file << "regressionOptions" << ":" << "kernelType" << " " << params->kernelTypeCombo->currentIndex() << "\n";
	file << "regressionOptions" << ":" << "kernelWidth" << " " << params->kernelWidthSpin->value() << "\n";
	file << "regressionOptions" << ":" << "svmC" << " " << params->svmCSpin->value() << "\n";
	file << "regressionOptions" << ":" << "svmP" << " " << params->svmPSpin->value() << "\n";
	file << "regressionOptions" << ":" << "svmType" << " " << params->svmTypeCombo->currentIndex() << "\n";
}

bool RegrSVM::LoadParams(QString name, float value)
{
	if(name.endsWith("kernelDeg")) params->kernelDegSpin->setValue((int)value);
	if(name.endsWith("kernelType")) params->kernelTypeCombo->setCurrentIndex((int)value);
	if(name.endsWith("kernelWidth")) params->kernelWidthSpin->setValue(value);
	if(name.endsWith("svmC")) params->svmCSpin->setValue(value);
	if(name.endsWith("svmP")) params->svmPSpin->setValue(value);
	if(name.endsWith("svmType")) params->svmTypeCombo->setCurrentIndex((int)value);
	return true;
}
