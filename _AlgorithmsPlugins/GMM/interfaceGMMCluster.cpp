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
#include "interfaceGMMCluster.h"
#include "drawUtils.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>
#include <MathLib/MathLib.h>
#include <glUtils.h>

using namespace std;

ClustGMM::ClustGMM()
{
	params = new Ui::ParametersGMMClust();
	params->setupUi(widget = new QWidget());
    marginalWidget = new MarginalWidget();
    connect(params->marginalsButton, SIGNAL(clicked()), this, SLOT(ShowMarginals()));
}

ClustGMM::~ClustGMM()
{
    delete params;
    delete marginalWidget;
}

void ClustGMM::SetParams(Clusterer *clusterer)
{
	if(!clusterer) return;
    SetParams(clusterer, GetParams());
}

fvec ClustGMM::GetParams()
{
    fvec par(3);
    par[0] = params->gmmCount->value();
    par[1] = params->gmmCovarianceCombo->currentIndex();
    par[2] = params->gmmInitCombo->currentIndex();
    return par;
}

void ClustGMM::SetParams(Clusterer *clusterer, fvec parameters)
{
    if(!clusterer) return;
    int clusters = parameters.size() > 0 ? parameters[0] : 1;
    int covType = parameters.size() > 1 ? parameters[1] : 0;
    int initType = parameters.size() > 2 ? parameters[2] : 0;
    ((ClustererGMM *)clusterer)->SetParams(clusters, covType, initType);
}

void ClustGMM::GetParameterList(std::vector<QString> &parameterNames,
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

void ClustGMM::ShowMarginals()
{
    marginalWidget->Show();
}

Clusterer *ClustGMM::GetClusterer()
{
	ClustererGMM *clusterer = new ClustererGMM();
	SetParams(clusterer);
	return clusterer;
}

void ClustGMM::DrawInfo(Canvas *canvas, QPainter &painter, Clusterer *clusterer)
{
	if(!canvas || !clusterer) return;
	painter.setRenderHint(QPainter::Antialiasing);
	int xIndex = canvas->xIndex;
	int yIndex = canvas->yIndex;

	ClustererGMM * _gmm = (ClustererGMM*)clusterer;
	Gmm *gmm = _gmm->gmm;
	int dim = gmm->dim;
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

		painter.setPen(QPen(Qt::black, 2));
		DrawEllipse(mean, sigma, 1, &painter, canvas);
		painter.setPen(QPen(Qt::black, 1));
		DrawEllipse(mean, sigma, 2, &painter, canvas);
		QPointF point = canvas->toCanvasCoords(mean[0], mean[1]);
		QColor color = SampleColor[(i+1)%SampleColorCnt];
		painter.setPen(QPen(Qt::black, 12));
		painter.drawEllipse(point, 8, 8);
		painter.setPen(QPen(color,4));
		painter.drawEllipse(point, 8, 8);
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
    std::vector<fvec> marginals(dim);
    std::vector< std::vector<fvec> > marginalGmm(dim);

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

void ClustGMM::DrawModel(Canvas *canvas, QPainter &painter, Clusterer *clusterer)
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

void ClustGMM::DrawGL(Canvas *canvas, GLWidget *glw, Clusterer *clusterer)
{
    if(!canvas || !glw || !clusterer) return;
    int dim = canvas->data->GetDimCount();
    int xIndex = canvas->xIndex;
    int yIndex = canvas->yIndex;
    int zIndex = canvas->zIndex;
    if(canvas->zIndex >= dim) zIndex = -1;

    if(!dynamic_cast<ClustererGMM*>(clusterer)) return;
    Gmm* gmm = dynamic_cast<ClustererGMM*>(clusterer)->gmm;

    fvec mean(3);
    float eigVal[3], rot[4*4];
    float* bigSigma = new float[dim*dim];
    float* bigMean = new float[dim];

    FOR(i, gmm->nstates)
    {
        gmm->getCovariance(i, bigSigma, false);
        gmm->getMean(i, bigMean);
        float prior = 1.f;

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

        QColor color = SampleColor[(i+1)%SampleColorCnt];

        float eigVec[9];
        eigVec[0] = x1; eigVec[1] = x2; eigVec[2] = x3;
        eigVec[3] = y1; eigVec[4] = y2; eigVec[5] = y3;
        eigVec[6] = z1; eigVec[7] = z2; eigVec[8] = z3;

        pair<QVector<QVector3D>, QMatrix4x4> verts = DrawGaussian(1.f, &mean[0], eigVal, eigVec);
        GLObject o;
        o.vertices = verts.first;
        o.normals = verts.first; // on a sphere we have vertices == normals!
        o.model = verts.second;
        o.objectType = "Surfaces,quads";
        o.style = "smooth,transparent";
        o.style += QString(",color:%1:%2:%3:0.5").arg(color.redF()).arg(color.greenF()).arg(color.blueF());
        glw->mutex->lock();
        glw->AddObject(o);

        verts = DrawGaussianLines(1.f, &mean[0], eigVal, eigVec);
        o.objectType = "Lines";
        o.style = "dotted,width:3";
        o.vertices = verts.first;
        o.normals.clear();
        o.model = verts.second;
        glw->AddObject(o);

        verts = DrawGaussianLines(2.f, &mean[0], eigVal, eigVec);
        o.objectType = "Lines";
        o.style = "dotted,width:1";
        o.vertices = verts.first;
        o.normals.clear();
        o.model = verts.second;
        glw->AddObject(o);
        glw->mutex->unlock();

        //GLuint list = DrawGaussian(&mean[0], eigVal, eigVec, prior, false, color.redF(), color.greenF(), color.blueF());
        //glw->drawSampleLists.push_back(list);
        //glw->drawSampleListCenters[list] = mean;
        /*
        GLuint list= DrawGaussian(&mean[0], eigVal, eigVec); // we draw the wireframe version
        glw->drawSampleLists.push_back(list);
        */
    }

    delete [] bigSigma;
    delete [] bigMean;
}

void ClustGMM::SaveOptions(QSettings &settings)
{
	settings.setValue("gmmCount", params->gmmCount->value());
	settings.setValue("gmmCovariance", params->gmmCovarianceCombo->currentIndex());
	settings.setValue("gmmInit", params->gmmInitCombo->currentIndex());
}

bool ClustGMM::LoadOptions(QSettings &settings)
{
	if(settings.contains("gmmCount")) params->gmmCount->setValue(settings.value("gmmCount").toFloat());
	if(settings.contains("gmmCovariance")) params->gmmCovarianceCombo->setCurrentIndex(settings.value("gmmCovariance").toInt());
	if(settings.contains("gmmInit")) params->gmmInitCombo->setCurrentIndex(settings.value("gmmInit").toInt());
	return true;
}

void ClustGMM::SaveParams(QTextStream &file)
{
	file << "clusterOptions" << ":" << "gmmCount" << " " << params->gmmCount->value() << "\n";
	file << "clusterOptions" << ":" << "gmmCovariance" << " " << params->gmmCovarianceCombo->currentIndex() << "\n";
	file << "clusterOptions" << ":" << "gmmInit" << " " << params->gmmInitCombo->currentIndex() << "\n";
}

bool ClustGMM::LoadParams(QString name, float value)
{
	if(name.endsWith("gmmCount")) params->gmmCount->setValue((int)value);
	if(name.endsWith("gmmCovariance")) params->gmmCovarianceCombo->setCurrentIndex((int)value);
	if(name.endsWith("gmmInit")) params->gmmInitCombo->setCurrentIndex((int)value);
	return true;
}
