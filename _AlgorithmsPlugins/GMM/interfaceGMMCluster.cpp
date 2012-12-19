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
}

void ClustGMM::SetParams(Clusterer *clusterer)
{
	if(!clusterer) return;
	int clusters = params->gmmCount->value();
	int covType = params->gmmCovarianceCombo->currentIndex();
	int initType = params->gmmInitCombo->currentIndex();

	((ClustererGMM *)clusterer)->SetParams(clusters, covType, initType);
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
		gmm->getCovariance(i, bigSigma, false);
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
    float radius[3], rot[4*4];
    float* bigSigma = new float[dim*dim];
    float* bigMean = new float[dim];

    FOR(i, gmm->nstates)
    {

        GLuint list = glGenLists(1);
        glNewList(list, GL_COMPILE);

        glPushAttrib(GL_ALL_ATTRIB_BITS);

        glDisable( GL_TEXTURE_2D );
        glEnable( GL_LINE_SMOOTH );
        glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_ADD);

        glDisable(GL_LINE_STIPPLE); // dashed/ dotted lines

        glEnable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glEnable(GL_ALPHA_TEST);
        glShadeModel(GL_SMOOTH);

        gmm->getCovariance(i, bigSigma, false);
        gmm->getMean(i, bigMean);

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
        radius[0] =  sqrtf(eigenValues(xIndex));
        radius[1] =  sqrtf(eigenValues(yIndex));
        radius[2] =  zIndex >= 0 ? sqrtf(eigenValues(zIndex)) : 0.001;

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

        rot[0]  = x1; rot[0 + 1] = x2; rot[0 + 2] = x3; rot[0 + 3] = 0;
        rot[4]  = y1; rot[4 + 1] = y2; rot[4 + 2] = y3; rot[4 + 3] = 0;
        rot[8]  = z1; rot[8 + 1] = z2; rot[8 + 2] = z3; rot[8 + 3] = 0;
        rot[12] = 0;  rot[12 + 1] = 0 ; rot[12 + 2] = 0 ; rot[12 + 3] = 1;

        QColor color = SampleColor[(i+1)%SampleColorCnt];

        int steps = 30;
        float speed = 3.f;
        FOR(d, steps)
        {
            float mcolor[] = { color.redF()/2, color.greenF()/2, color.blueF()/2, (1.f - d/(float)steps)*expf(-(d/(float)steps)*speed)};
            glPushMatrix();
            glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mcolor);
            glTranslatef(mean[0], mean[1], mean[2]);
            glMultMatrixf(rot);
            glScalef(radius[0], radius[1], radius[2]);
            DrawStandardSphere(0.01 + d * 2.8f / steps);
            glPopMatrix();
        }

        glPopAttrib();

        glEndList();
        glw->drawSampleLists.push_back(list);
        glw->drawSampleListCenters[list] = mean;


        list = glGenLists(1);
        glNewList(list, GL_COMPILE);

        glPushAttrib(GL_ALL_ATTRIB_BITS);

        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);

        glDisable( GL_TEXTURE_2D );
        glEnable( GL_LINE_SMOOTH );
        glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_ADD);

        glPushMatrix();
        glTranslatef(mean[0], mean[1], mean[2]);
        glMultMatrixf(rot);
        glScalef(radius[0], radius[1], radius[2]);
        glColor3d(0,0,0);
        glLineWidth(2.f);
        glDisable(GL_LINE_STIPPLE); // dashed/ dotted lines
        DrawSphereIsolines(1);
        glLineWidth(0.5f);
        glEnable(GL_LINE_STIPPLE); // dashed/ dotted lines
        glLineStipple (1, 0xAAAA); // dash pattern AAAA: dots
        DrawSphereIsolines(2);
        glPopMatrix();
        glPopAttrib();

        glEndList();
        glw->drawSampleLists.push_back(list);
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
