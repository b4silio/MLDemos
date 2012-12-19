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
}

void RegrGMM::SetParams(Regressor *regressor)
{
	if(!regressor) return;
	int clusters = params->gmmCount->value();
	int covType = params->gmmCovarianceCombo->currentIndex();
	int initType = params->gmmInitCombo->currentIndex();

	((RegressorGMR *)regressor)->SetParams(clusters, covType, initType);
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
		gmm->getCovariance(i, bigSigma, false);
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
    float radius[3], rot[4*4];
    float* bigSigma = new float[dim*dim];
    float* bigMean = new float[dim];

    FOR(i, gmr->nstates)
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

        gmr->getCovariance(i, bigSigma, false);
        gmr->getMean(i, bigMean);

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

        int steps = 30;
        float speed = 3.f;
        FOR(d, steps)
        {
            float mcolor[] = { 0.8f, 0.8f, 0.8f, (1.f - d/(float)steps)*expf(-(d/(float)steps)*speed)};
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
