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
#include "interfaceGMMRegress.h"
#include "drawUtils.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>
#include <MathLib/MathLib.h>
#include <glUtils.h>

using namespace std;

RegrGMM::RegrGMM()
{
	params = new Ui::ParametersGMMRegr();
	params->setupUi(widget = new QWidget());
    marginalWidget = new MarginalWidget();
    connect(params->marginalsButton, SIGNAL(clicked()), this, SLOT(ShowMarginals()));
}

RegrGMM::~RegrGMM()
{
    delete params;
    delete marginalWidget;
}

void RegrGMM::SetParams(Regressor *regressor)
{
	if(!regressor) return;
    SetParams(regressor, GetParams());
}

fvec RegrGMM::GetParams()
{
    fvec par(3);
    par[0] = params->gmmCount->value();
    par[1] = params->gmmCovarianceCombo->currentIndex();
    par[2] = params->gmmInitCombo->currentIndex();
    return par;
}

void RegrGMM::SetParams(Regressor *regressor, fvec parameters)
{
    if(!regressor) return;
    int clusters = parameters.size() > 0 ? parameters[0] : 1;
    int covType = parameters.size() > 1 ? parameters[1] : 0;
    int initType = parameters.size() > 2 ? parameters[2] : 0;
    ((RegressorGMR *)regressor)->SetParams(clusters, covType, initType);
}

void RegrGMM::GetParameterList(std::vector<QString> &parameterNames,
                             std::vector<QString> &parameterTypes,
                             std::vector< std::vector<QString> > &parameterValues)
{
    parameterNames.clear();
    parameterTypes.clear();
    parameterValues.clear();
    parameterNames.push_back("Components Count");
    parameterNames.push_back("Covariance Type");
    parameterNames.push_back("Initialization Type");
    parameterTypes.push_back("Integer");
    parameterTypes.push_back("List");
    parameterTypes.push_back("List");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("1");
    parameterValues.back().push_back("999");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("Full");
    parameterValues.back().push_back("Diagonal");
    parameterValues.back().push_back("Spherical");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("Random");
    parameterValues.back().push_back("Uniform");
    parameterValues.back().push_back("K-Means");
}

void RegrGMM::ShowMarginals()
{
    marginalWidget->Show();
}

QString RegrGMM::GetAlgoString()
{
	int clusters = params->gmmCount->value();
	int covType = params->gmmCovarianceCombo->currentIndex();
	int initType = params->gmmInitCombo->currentIndex();
	QString algo = QString("GMM %1").arg(clusters);
	switch(covType)
	{
	case 0:
		algo += " Ful";
		break;
	case 1:
		algo += " Dia";
		break;
	case 2:
		algo += " Sph";
		break;
	}
	switch(initType)
	{
	case 0:
		algo += " Rnd";
		break;
	case 1:
		algo += " Uni";
		break;
	case 2:
		algo += " K-M";
		break;
	}
	return algo;
}

Regressor *RegrGMM::GetRegressor()
{
	RegressorGMR *regressor = new RegressorGMR();
	SetParams(regressor);
	return regressor;
}

void RegrGMM::DrawInfo(Canvas *canvas, QPainter &painter, Regressor *regressor)
{
	if(!canvas || !regressor) return;
	painter.setRenderHint(QPainter::Antialiasing);

	RegressorGMR * gmr = (RegressorGMR*)regressor;
	Gmm *gmm = gmr->gmm;
	int dim = canvas->data->GetCount() ? canvas->data->GetSample(0).size() : gmm->dim;
    int outputDim = regressor->outputDim;
	int xIndex = canvas->xIndex;
	int yIndex = canvas->yIndex;
    if(outputDim != -1 && outputDim < dim-1);
    {
        if(xIndex == dim-1) xIndex = outputDim;
        else if (xIndex == outputDim) xIndex = dim-1;
        if(yIndex == dim-1) yIndex = outputDim;
        else if(yIndex == outputDim) yIndex = dim-1;
    }
    float mean[2];
	float sigma[3];
	painter.setBrush(Qt::NoBrush);
	FOR(i, gmm->nstates)
	{
		float* bigSigma = new float[dim*dim];
		float* bigMean = new float[dim];
        gmm->getCovariance(i, bigSigma);
		sigma[0] = bigSigma[xIndex*dim + xIndex];
		sigma[1] = bigSigma[yIndex*dim + xIndex];
		sigma[2] = bigSigma[yIndex*dim + yIndex];
		gmm->getMean(i, bigMean);
		mean[0] = bigMean[xIndex];
		mean[1] = bigMean[yIndex];
		delete [] bigSigma;
		delete [] bigMean;

		painter.setPen(QPen(Qt::black, 1));
		DrawEllipse(mean, sigma, 1, &painter, canvas);
		painter.setPen(QPen(Qt::black, 0.5));
		DrawEllipse(mean, sigma, 2, &painter, canvas);
		QPointF point = canvas->toCanvasCoords(mean[0],mean[1]);
		painter.setPen(QPen(Qt::black, 4));
		painter.drawEllipse(point, 2, 2);
		painter.setPen(QPen(Qt::white, 2));
		painter.drawEllipse(point, 2, 2);
	}

    vector<fvec> samples = canvas->data->GetSamples();
    fvec minv (dim, FLT_MAX);
    fvec maxv (dim, -FLT_MAX);
    FOR(i, samples.size())
    {
        FOR(d, dim)
        {
            minv[d] = min(minv[d], samples[i][d]);
            maxv[d] = max(maxv[d], samples[i][d]);
        }
    }
    fvec diff = maxv-minv;
    minv -= diff/4;
    maxv += diff/4;

    std::vector<fvec> limits(dim);
    FOR(d, dim)
    {
        fvec pair(2);
        pair[0] = minv[d];
        pair[1] = maxv[d];
        limits[d] = pair;
    }
    vector<fvec> marginals(dim);
    vector< vector<fvec> >marginalGmm(dim);

    float* bigSigma = new float[dim*dim];
    float* bigMean = new float[dim];
    FOR ( d, dim )
    {
        int steps = 200;
        marginals[d].resize(steps,0);
        marginalGmm[d].resize(steps);
        FOR ( j, steps )
        {
            float likelihood = 0;
            float s = j/(float)(steps)*(maxv[d]-minv[d]) + minv[d];
            FOR ( i, gmm->nstates ) {
                gmm->getCovariance(i, bigSigma);
                gmm->getMean(i, bigMean);
                float mu = bigMean[d];
                float sigma = bigSigma[d + d*dim];
                float prior = gmm->getPrior(i);

                float diff = s-mu;
                float lik = 1./(sqrtf(2*M_PI*sigma))*expf(-(diff*diff)*0.5/sigma);
                likelihood += lik*prior;
                marginalGmm[d][j].push_back(max(0.f,lik*prior));
            }
            marginals[d][j] = likelihood;
        }
    }
    delete [] bigSigma;
    delete [] bigMean;
    marginalWidget->SetDimensions(dim, canvas->dimNames);
    marginalWidget->SetMarginals(marginals, marginalGmm, limits);
}

void RegrGMM::DrawConfidence(Canvas *canvas, Regressor *regressor)
{
	int w = canvas->width();
	int h = canvas->height();

	RegressorGMR *gmr = ((RegressorGMR *)regressor);
    int outputDim = regressor->outputDim;

	QImage density(QSize(256,256), QImage::Format_RGB32);
	density.fill(0);
	// we draw a density map for the probability
	fvec sample;
	sample.resize(2,0);
	float sigma[4];
	for (int i=0; i < density.width(); i++)
	{
		for (int j=0; j< density.height(); j++)
		{
			sample = canvas->toSampleCoords(i*w/density.width(),j*h/density.height());
            int dim = sample.size();
            if(outputDim != -1 && outputDim < dim)
            {
                float tmp = sample[outputDim];
                sample[outputDim] = sample[dim-1];
                sample[dim-1] = tmp;
            }
			float val = gmr->gmm->pdf(&sample[0]);
			int color = min(255,(int)(128 + val*10));
			density.setPixel(i,j, qRgb(color,color,color));
		}
	}
	canvas->maps.confidence = QPixmap::fromImage(density.scaled(QSize(w,h),Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

void RegrGMM::DrawModel(Canvas *canvas, QPainter &painter, Regressor *regressor)
{
	if(!regressor || !canvas) return;
	int w = canvas->width();
	int h = canvas->height();
	painter.setRenderHint(QPainter::Antialiasing, true);
    int xIndex = canvas->xIndex;
    int yIndex = canvas->yIndex;
    int outputDim = regressor->outputDim;

	int steps = w;
	fvec sample;sample.resize(2, 0);
	painter.setBrush(Qt::NoBrush);
    QPainterPath path, pathUp, pathDown, pathUpUp, pathDownDown;
	FOR(x, steps)
	{
        sample = canvas->toSampleCoords(x, 0);
        int dim = sample.size();
        if(dim > 2) continue;
        if(outputDim==-1) outputDim = dim-1;
        fvec res = regressor->Test(sample);
		if(res[0] != res[0] || res[1] != res[1]) continue;
        sample[outputDim] = res[0];
        QPointF point = canvas->toCanvasCoords(sample);
        sample[outputDim] = res[0]+res[1];
        QPointF pointUp = canvas->toCanvasCoords(sample);
        pointUp.setX(0);
        pointUp.setY(pointUp.y() - point.y());
        sample[outputDim] = res[0]-res[1];
        QPointF pointDown = canvas->toCanvasCoords(sample);
        //pointDown = -pointUp;
        pointDown.setX(0);
        pointDown.setY(pointDown.y() - point.y());
		if(x)
		{
            path.lineTo(point);
            pathUp.lineTo(pointUp);
            pathUpUp.lineTo(point + 2*pointUp);
            pathDown.lineTo(point + pointDown);
            pathDownDown.lineTo(point + 2*pointDown);
        }
        else
        {
            path.moveTo(point);
            pathUp.moveTo(point + pointUp);
            pathUpUp.moveTo(point + 2*pointUp);
            pathDown.moveTo(point + pointDown);
            pathDownDown.moveTo(point + 2*pointDown);
        }
	}
    painter.setPen(QPen(Qt::black, 1));
    painter.drawPath(path);
    painter.setPen(QPen(Qt::black, 0.5));
    painter.drawPath(pathUp);
    painter.drawPath(pathDown);
    painter.setPen(QPen(Qt::black, 0.25));
    painter.drawPath(pathUpUp);
    painter.drawPath(pathDownDown);
}

void RegrGMM::DrawGL(Canvas *canvas, GLWidget *glw, Regressor *regressor)
{
    if(!canvas || !glw || !regressor) return;
    int dim = canvas->data->GetDimCount();
    int xIndex = canvas->xIndex;
    int yIndex = canvas->yIndex;
    int zIndex = canvas->zIndex;
    if(canvas->zIndex >= dim) zIndex = -1;

    if(!dynamic_cast<RegressorGMR*>(regressor)) return;
    Gmm* gmr = dynamic_cast<RegressorGMR*>(regressor)->gmm;

    fvec mean(3);
    float eigVal[3], rot[4*4];
    float* bigSigma = new float[dim*dim];
    float* bigMean = new float[dim];
    int outputDim = ((RegressorGMR*)regressor)->outputDim;

    FOR(i, gmr->nstates)
    {
        gmr->getCovariance(i, bigSigma, false);
        gmr->getMean(i, bigMean);
        float prior = gmr->getPrior(i);

        // Very Important! We need to swap the dimensions to get the proper output dim!
        if(outputDim >= 0 && outputDim < dim-1)
        {
            float val = bigMean[dim-1];
            bigMean[dim-1] = bigMean[outputDim];
            bigMean[outputDim] = val;

            FOR(y, dim-1)
            {
                FOR(x,y)
                {
                    if(y == outputDim)
                    {
                        float val = bigSigma[(dim-1)*dim + x];
                        bigSigma[(dim-1)*dim + x] = bigSigma[y*dim + x];
                        bigSigma[y*dim + x] = val;
                        bigSigma[x*dim + y] = val;
                        bigSigma[x*dim + (dim-1)] = bigSigma[(dim-1)*dim + x];
                    }
                    else if(x == outputDim)
                    {
                        float val = bigSigma[(dim-1)*dim + y];
                        bigSigma[(dim-1)*dim + y] = bigSigma[y*dim + x];
                        bigSigma[y*dim + x] = val;
                        bigSigma[x*dim + y] = val;
                        bigSigma[y*dim + (dim-1)] = bigSigma[(dim-1)*dim + y];
                    }
                }
            }
            val = bigSigma[outputDim*dim + outputDim];
            bigSigma[outputDim*dim + outputDim] = bigSigma[(dim-1)*dim + (dim-1)];
            bigSigma[(dim-1)*dim + (dim-1)] = val;
        }

        mean[0] = bigMean[xIndex];
        mean[1] = bigMean[yIndex];
        mean[2] = zIndex >= 0 ? bigMean[zIndex] : 0;

        MathLib::Matrix m(dim, dim);
        FOR(d1, dim)
        {
            FOR(d2, dim)
            {
                m(d1,d2) = bigSigma[d1*dim + d2];
            }
        }
        MathLib::Vector eigenValues(dim);
        MathLib::Matrix eigenVectors(dim, dim);
        m.EigenValuesDecomposition(eigenValues, eigenVectors);

        // we get the scaling parameters
        eigVal[0] =  sqrtf(eigenValues(xIndex));
        eigVal[1] =  sqrtf(eigenValues(yIndex));
        eigVal[2] =  zIndex >= 0 ? sqrtf(eigenValues(zIndex)) : 0.001;

        // we get the angles
        float norm;
        float x1,y1,z1;
        x1 = eigenVectors(xIndex,xIndex);
        y1 = eigenVectors(xIndex,yIndex);
        z1 = zIndex >= 0 ? eigenVectors(xIndex,zIndex) : 0;
        norm = sqrtf(x1*x1 + y1*y1 + z1*z1);
        x1 /= norm; y1 /= norm; z1 /= norm;

        float x2,y2,z2;
        x2 = eigenVectors(yIndex,xIndex);
        y2 = eigenVectors(yIndex,yIndex);
        z2 = zIndex >= 0 ? eigenVectors(yIndex,zIndex) : 0;
        norm = sqrtf(x2*x2 + y2*y2 + z2*z2);
        x2 /= norm; y2 /= norm; z2 /= norm;

        float x3,y3,z3;
        x3 = zIndex >= 0 ? eigenVectors(zIndex,xIndex) : 0;
        y3 = zIndex >= 0 ? eigenVectors(zIndex,yIndex) : 0;
        z3 = zIndex >= 0 ? eigenVectors(zIndex,zIndex) : 1;
        norm = sqrtf(x3*x3 + y3*y3 + z3*z3);
        x3 /= norm; y3 /= norm; z3 /= norm;

        float eigVec[9];
        eigVec[0] = x1; eigVec[1] = x2; eigVec[2] = x3;
        eigVec[3] = y1; eigVec[4] = y2; eigVec[5] = y3;
        eigVec[6] = z1; eigVec[7] = z2; eigVec[8] = z3;

        GLuint list= DrawGaussian(&mean[0], eigVal, eigVec, prior, false, 0.8f, 0.8f, 0.8f);
        glw->drawSampleLists.push_back(list);
        glw->drawSampleListCenters[list] = mean;
        list= DrawGaussian(&mean[0], eigVal, eigVec);
        glw->drawSampleLists.push_back(list);
    }

    delete [] bigSigma;
    delete [] bigMean;
}

void RegrGMM::SaveOptions(QSettings &settings)
{
	settings.setValue("gmmCount", params->gmmCount->value());
	settings.setValue("gmmCovariance", params->gmmCovarianceCombo->currentIndex());
	settings.setValue("gmmInit", params->gmmInitCombo->currentIndex());
}

bool RegrGMM::LoadOptions(QSettings &settings)
{
	if(settings.contains("gmmCount")) params->gmmCount->setValue(settings.value("gmmCount").toFloat());
	if(settings.contains("gmmCovariance")) params->gmmCovarianceCombo->setCurrentIndex(settings.value("gmmCovariance").toInt());
	if(settings.contains("gmmInit")) params->gmmInitCombo->setCurrentIndex(settings.value("gmmInit").toInt());
	return true;
}

void RegrGMM::SaveParams(QTextStream &file)
{
	file << "regressionOptions" << ":" << "gmmCount" << " " << params->gmmCount->value() << "\n";
	file << "regressionOptions" << ":" << "gmmCovariance" << " " << params->gmmCovarianceCombo->currentIndex() << "\n";
	file << "regressionOptions" << ":" << "gmmInit" << " " << params->gmmInitCombo->currentIndex() << "\n";
}

bool RegrGMM::LoadParams(QString name, float value)
{
	if(name.endsWith("gmmCount")) params->gmmCount->setValue((int)value);
	if(name.endsWith("gmmCovariance")) params->gmmCovarianceCombo->setCurrentIndex((int)value);
	if(name.endsWith("gmmInit")) params->gmmInitCombo->setCurrentIndex((int)value);
	return true;
}
