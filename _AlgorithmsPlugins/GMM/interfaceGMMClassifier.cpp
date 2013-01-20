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
#include "interfaceGMMClassifier.h"
#include "drawUtils.h"
#include <basicMath.h>
#include <QPixmap>
#include <QBitmap>
#include <QPainter>
#include <QDebug>
#include <MathLib/MathLib.h>
#include <glUtils.h>

using namespace std;

ClassGMM::ClassGMM()
{
    params = new Ui::ParametersGMM();
    params->setupUi(widget = new QWidget());
}

void ClassGMM::SetParams(Classifier *classifier)
{
    if(!classifier) return;
    SetParams(classifier, GetParams());
}

fvec ClassGMM::GetParams()
{
    fvec par(3);
    par[0] = params->gmmCount->value();
    par[1] = params->gmmCovarianceCombo->currentIndex();
    par[2] = params->gmmInitCombo->currentIndex();
    return par;
}

void ClassGMM::SetParams(Classifier *classifier, fvec parameters)
{
    if(!classifier) return;
    int clusters = parameters.size() > 0 ? parameters[0] : 1;
    int covType = parameters.size() > 1 ? parameters[1] : 0;
    int initType = parameters.size() > 2 ? parameters[2] : 0;
    ((ClassifierGMM *)classifier)->SetParams(clusters, covType, initType);
}

void ClassGMM::GetParameterList(std::vector<QString> &parameterNames,
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

QString ClassGMM::GetAlgoString()
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

Classifier *ClassGMM::GetClassifier()
{
    ClassifierGMM *classifier = new ClassifierGMM();
    SetParams(classifier);
    return classifier;
}

void ClassGMM::DrawInfo(Canvas *canvas, QPainter &painter, Classifier *classifier)
{
    if(!canvas || !classifier) return;
    painter.setRenderHint(QPainter::Antialiasing);

    ClassifierGMM * gmm = (ClassifierGMM*)classifier;
    vector<Gmm*> gmms = gmm->gmms;
    if(!gmms.size()) return;
    int xIndex = canvas->xIndex;
    int yIndex = canvas->yIndex;
    int dim = gmms[0]->dim;
    float mean[2];
    float sigma[3];
    painter.setBrush(Qt::NoBrush);
    FOR(g, gmms.size())
    {
        FOR(i, gmms[g]->nstates)
        {
            float* bigSigma = new float[dim*dim];
            float* bigMean = new float[dim];
            gmms[g]->getCovariance(i, bigSigma);
            sigma[0] = bigSigma[xIndex*dim + xIndex];
            sigma[1] = bigSigma[yIndex*dim + xIndex];
            sigma[2] = bigSigma[yIndex*dim + yIndex];
            gmms[g]->getMean(i, bigMean);
            mean[0] = bigMean[xIndex];
            mean[1] = bigMean[yIndex];
            delete [] bigSigma;
            delete [] bigMean;
            //FOR(j,4) sigma[j] = sqrt(sigma[j]);
            painter.setPen(QPen(Qt::black, 1));
            DrawEllipse(mean, sigma, 1, &painter, canvas);
            painter.setPen(QPen(Qt::black, 0.5));
            DrawEllipse(mean, sigma, 2, &painter, canvas);
            QPointF point = canvas->toCanvasCoords(mean[0],mean[1]);
            QColor color = SampleColor[classifier->inverseMap[g]%SampleColorCnt];
            painter.setPen(QPen(Qt::black, 12));
            painter.drawEllipse(point, 6, 6);
            painter.setPen(QPen(color,4));
            painter.drawEllipse(point, 6, 6);
        }
    }
}

void ClassGMM::DrawModel(Canvas *canvas, QPainter &painter, Classifier *classifier)
{
    int posClass = 1;
    // we draw the samples
    painter.setRenderHint(QPainter::Antialiasing, true);
    FOR(i, canvas->data->GetCount())
    {
        fvec sample = canvas->data->GetSample(i);
        int label = canvas->data->GetLabel(i);
        QPointF point = canvas->toCanvasCoords(canvas->data->GetSample(i));
        fvec res = classifier->TestMulti(sample);
        if(res.size()==1)
        {
            float response = res[0];
            if(response > 0)
            {
                if(classifier->classMap[label] == posClass) Canvas::drawSample(painter, point, 9, 1);
                else Canvas::drawCross(painter, point, 6, 2);
            }
            else
            {
                if(classifier->classMap[label] != posClass) Canvas::drawSample(painter, point, 9, 0);
                else Canvas::drawCross(painter, point, 6, 0);
            }
        }
        else
        {
            int max = 0;
            for(int i=1; i<res.size(); i++) if(res[max] < res[i]) max = i;
            int resp = classifier->inverseMap[max];
            if(label == resp) Canvas::drawSample(painter, point, 9, label);
            else Canvas::drawCross(painter, point, 6, label);
        }
    }
}

void DrawGaussian(float *mean, float *sigma, float rad, int plane)
{
    if(mean[0] != mean[0] || mean[1] != mean[1]) return; // nan
    float a = sigma[0], b = sigma[1], c = sigma[2];
    float L[4];
    L[0] = a; L[1] = 0; L[2] = b; L[3] = sqrtf(c*a-b*b);
    if(L[3] != L[3]) L[3] = 0;
    FOR(i,4) L[i] /= sqrtf(a);

    glBegin(GL_LINE_LOOP);
    const int segments = 64;
    for (float theta=0; theta <= PIf*2.f; theta += (PIf*2.f)/segments)
    {
        float x = cosf(theta)*rad;
        float y = sinf(theta)*rad;
        float nx = L[0]*x;
        float ny = L[2]*x + L[3]*y;
        float nz = 0;
        switch(plane)
        {
        case 0: // x-y
            glVertex3f(nx + mean[0], ny + mean[1], nz + mean[2]);
            break;
        case 1: // x-z
            glVertex3f(nx + mean[0], nz + mean[1], ny + mean[2]);
            break;
        case 2: // y-z
            glVertex3f(nz + mean[0], nx + mean[1], ny + mean[2]);
            break;
        }
    }
    glEnd();
}

void ClassGMM::DrawGL(Canvas *canvas, GLWidget *glw, Classifier *classifier)
{
    if(!canvas || !glw || !classifier) return;
    int dim = canvas->data->GetDimCount();
    int xIndex = canvas->xIndex;
    int yIndex = canvas->yIndex;
    int zIndex = canvas->zIndex;
    if(canvas->zIndex >= dim) zIndex = -1;

    ClassifierGMM * gmm = (ClassifierGMM*)classifier;
    vector<Gmm*> gmms = gmm->gmms;
    if(!gmms.size()) return;

    float* bigSigma = new float[dim*dim];
    float* bigMean = new float[dim];
    FOR(g, gmms.size())
    {
        FOR(i, gmms[g]->nstates)
        {
            gmms[g]->getCovariance(i, bigSigma, false);
            gmms[g]->getMean(i, bigMean);
            float prior = gmms[g]->getPrior(i);

            fvec mean(3);
            mean[0] = bigMean[xIndex];
            mean[1] = bigMean[yIndex];
            mean[2] = zIndex >= 0 ? bigMean[zIndex] : 0;

            MathLib::Matrix m(dim, dim);
            for(int d1=0; d1<dim; d1++)
            {
                for(int d2=0; d2<dim; d2++)
                {
                    m(d1,d2) = bigSigma[d1*dim + d2];
                }
            }
            MathLib::Vector eigenValues(dim);
            MathLib::Matrix eigenVectors(dim, dim);
            m.EigenValuesDecomposition(eigenValues, eigenVectors);

            // we get the scaling parameters
            float eigVal[3];
            eigVal[0] =  sqrtf(eigenValues(xIndex));
            eigVal[1] =  sqrtf(eigenValues(yIndex));
            eigVal[2] =  zIndex >= 0 ? sqrtf(eigenValues(zIndex)) : 0.001;

            // we get the eigenvectors for the 3 dimensions we are drawing
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

            QColor color = SampleColor[classifier->inverseMap[g]%SampleColorCnt];

            GLuint list= DrawGaussian(&mean[0], eigVal, eigVec, prior, false, color.redF(), color.greenF(), color.blueF());
            glw->drawSampleLists.push_back(list);
            glw->drawSampleListCenters[list] = mean;
            list= DrawGaussian(&mean[0], eigVal, eigVec);
            glw->drawSampleLists.push_back(list);
        }
    }

   /*
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_POINT_SPRITE);
    glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glPointSize(16.0);
    glBegin(GL_POINTS);
    FOR(g, gmms.size())
    {
        FOR(i, gmms[g]->nstates)
        {
            gmms[g]->getMean(i, bigMean);

            mean[0] = bigMean[xIndex];
            mean[1] = bigMean[yIndex];
            mean[2] = zIndex >= 0 ? bigMean[zIndex] : 0;
            QColor c = SampleColor[classifier->inverseMap[g]%SampleColorCnt];
            glColor3f(c.redF(), c.greenF(), c.blueF());
            glVertex3f(mean[0],mean[1],mean[2]);
        }
    }
    glEnd();
    glPopAttrib();
    */
    delete [] bigSigma;
    delete [] bigMean;

    /*
    glColor3d(0,0,0);
    glEnable(GL_LINE_STIPPLE); // enable dashed/ dotted lines
    glLineWidth(0.5f); // line width
    glLineStipple (1, 0xAAAA); // dash pattern AAAA: dots
    glBegin(GL_LINES);
    glEnd();
    */
}

void ClassGMM::SaveOptions(QSettings &settings)
{
    settings.setValue("gmmCount", params->gmmCount->value());
    settings.setValue("gmmCovariance", params->gmmCovarianceCombo->currentIndex());
    settings.setValue("gmmInit", params->gmmInitCombo->currentIndex());
}

bool ClassGMM::LoadOptions(QSettings &settings)
{
    if(settings.contains("gmmCount")) params->gmmCount->setValue(settings.value("gmmCount").toFloat());
    if(settings.contains("gmmCovariance")) params->gmmCovarianceCombo->setCurrentIndex(settings.value("gmmCovariance").toInt());
    if(settings.contains("gmmInit")) params->gmmInitCombo->setCurrentIndex(settings.value("gmmInit").toInt());
    return true;
}

void ClassGMM::SaveParams(QTextStream &file)
{
    file << "classificationOptions" << ":" << "gmmCount" << " " << params->gmmCount->value() << "\n";
    file << "classificationOptions" << ":" << "gmmCovariance" << " " << params->gmmCovarianceCombo->currentIndex() << "\n";
    file << "classificationOptions" << ":" << "gmmInit" << " " << params->gmmInitCombo->currentIndex() << "\n";
}

bool ClassGMM::LoadParams(QString name, float value)
{
    if(name.endsWith("gmmCount")) params->gmmCount->setValue((int)value);
    if(name.endsWith("gmmCovariance")) params->gmmCovarianceCombo->setCurrentIndex((int)value);
    if(name.endsWith("gmmInit")) params->gmmInitCombo->setCurrentIndex((int)value);
    return true;
}
