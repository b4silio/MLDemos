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
#include "interfaceSVMCluster.h"
#include "basicOpenCV.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>

using namespace std;

ClustSVM::ClustSVM()
{
	params = new Ui::ParametersClust();
	params->setupUi(widget = new QWidget());
}

void ClustSVM::SetParams(Clusterer *clusterer)
{
	if(!clusterer) return;
	float type = params->svmTypeCombo->currentIndex();
	float svmC = params->svmCSpin->value();
	int kernelType = params->kernelTypeCombo->currentIndex();
	float kernelGamma = params->kernelWidthSpin->value();
	float kernelDegree = params->kernelDegSpin->value();
	int clusters = params->kernelClusterSpin->value();

	if(type == 0) // One-Class SVM
	{
		ClustererSVR *svm = ((ClustererSVR*) clusterer);
		svm->param.nu = svmC;
		svm->param.gamma = 1 / kernelGamma;
		svm->param.degree = kernelDegree;
	}
	else if(type == 1) // kernel k-means
	{
		ClustererKKM *kkm = ((ClustererKKM*)clusterer);
		kkm->SetParams(clusters, kernelType, kernelGamma, kernelDegree);
	}
}

Clusterer *ClustSVM::GetClusterer()
{
	int type = params->svmTypeCombo->currentIndex();
	Clusterer *clusterer = 0;

	switch(type)
	{
	case 0:
		clusterer = new ClustererSVR();
		break;
	case 1:
		clusterer = new ClustererKKM();
		break;
	}
	SetParams(clusterer);
	return clusterer;
}

void ClustSVM::DrawInfo(Canvas *canvas, Clusterer *clusterer)
{
	if(!canvas || !clusterer) return;
	int w = canvas->width();
	int h = canvas->height();
	QPixmap infoPixmap(w, h);
	QBitmap bitmap(w,h);
	bitmap.clear();
	infoPixmap.setMask(bitmap);
	infoPixmap.fill(Qt::transparent);

	QPainter painter(&infoPixmap);
	painter.setRenderHint(QPainter::Antialiasing);
	if(clusterer->type == CLUS_SVR)
	{
		// we want to draw the support vectors
		svm_model *svm = ((ClustererSVR*)clusterer)->GetModel();
		if(svm)
		{
			f32 sv[2];
			FOR(i, svm->l)
			{
				FOR(j, 2)
				{
					sv[j] = (f32)svm->SV[i][j].value;
				}
				int radius = 11;
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
	canvas->infoPixmap = infoPixmap;
}

void ClustSVM::Draw(Canvas *canvas, Clusterer *clusterer)
{
	if(!canvas || !clusterer) return;
	canvas->liveTrajectory.clear();
	DrawInfo(canvas, clusterer);
	int w = canvas->width();
	int h = canvas->height();

	bool bDrawConfidence = canvas->bDisplayMap;

	if(bDrawConfidence)
	{
		IplImage *image = NULL;
		image = cvCreateImage(cvSize(w,h), 8, 3);
		cvSet(image, CV_RGB(255,255,255));

		fvec sample;
		sample.resize(2,0);
		FOR(y, image->height)
		{
			FOR(x, image->width)
			{
				sample = canvas->toSampleCoords(x,y);
				fvec res = clusterer->Test(sample);
				float r=0,g=0,b=0;
				if(res.size() > 1)
				{
					FOR(i, res.size())
					{
						r += CV::color[(i+1)%CV::colorCnt].val[0]*res[i];
						g += CV::color[(i+1)%CV::colorCnt].val[1]*res[i];
						b += CV::color[(i+1)%CV::colorCnt].val[2]*res[i];
					}
				}
				else if(res.size())
				{
					r = (1-res[0])*255 + res[0]* 255;
					g = (1-res[0])*255;
					b = (1-res[0])*255;
				}
				image->imageData[y*image->widthStep + x*3 + 2] = (u8)r;
				image->imageData[y*image->widthStep + x*3 + 1] = (u8)g;
				image->imageData[y*image->widthStep + x*3 + 0] = (u8)b;
			}
		}

		IplImage *big = cvCreateImage(cvSize(canvas->width(), canvas->height()),8,3);
		cvResize(image, big, CV_INTER_CUBIC);
		canvas->confidencePixmap = Canvas::toPixmap(big);
		IMKILL(image);
		IMKILL(big);
	}
	else
	{
		canvas->confidencePixmap = QPixmap();
	}

	QPixmap modelPixmap(w, h);
	QBitmap bitmap(w,h);
	bitmap.clear();
	modelPixmap.setMask(bitmap);
	modelPixmap.fill(Qt::transparent);
	QPainter painter(&modelPixmap);
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
				r += CV::color[(j+1)%CV::colorCnt].val[0]*res[j];
				g += CV::color[(j+1)%CV::colorCnt].val[1]*res[j];
				b += CV::color[(j+1)%CV::colorCnt].val[2]*res[j];
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
	canvas->modelPixmap = modelPixmap;
	canvas->repaint();
}

void ClustSVM::SaveOptions(QSettings &settings)
{
	settings.setValue("kernelDeg", params->kernelDegSpin->value());
	settings.setValue("kernelType", params->kernelTypeCombo->currentIndex());
	settings.setValue("kernelWidth", params->kernelWidthSpin->value());
	settings.setValue("svmC", params->svmCSpin->value());
	settings.setValue("svmType", params->svmTypeCombo->currentIndex());
	settings.setValue("kernelCluster", params->kernelClusterSpin->value());
}

bool ClustSVM::LoadOptions(QSettings &settings)
{
	if(settings.contains("kernelDeg")) params->kernelDegSpin->setValue(settings.value("kernelDeg").toFloat());
	if(settings.contains("kernelType")) params->kernelTypeCombo->setCurrentIndex(settings.value("kernelType").toInt());
	if(settings.contains("kernelWidth")) params->kernelWidthSpin->setValue(settings.value("kernelWidth").toFloat());
	if(settings.contains("svmType")) params->svmTypeCombo->setCurrentIndex(settings.value("svmType").toInt());
	if(settings.contains("svmC")) params->svmCSpin->setValue(settings.value("svmC").toFloat());
	if(settings.contains("kernelCluster")) params->kernelClusterSpin->setValue(settings.value("kernelCluster").toFloat());
	return true;
}

void ClustSVM::SaveParams(std::ofstream &file)
{
	file << "clusterOptions" << ":" << "kernelDeg" << " " << params->kernelDegSpin->value() << std::endl;
	file << "clusterOptions" << ":" << "kernelType" << " " << params->kernelTypeCombo->currentIndex() << std::endl;
	file << "clusterOptions" << ":" << "kernelWidth" << " " << params->kernelWidthSpin->value() << std::endl;
	file << "clusterOptions" << ":" << "svmC" << " " << params->svmCSpin->value() << std::endl;
	file << "clusterOptions" << ":" << "svmType" << " " << params->svmTypeCombo->currentIndex() << std::endl;
	file << "clusterOptions" << ":" << "kernelCluster" << " " << params->kernelClusterSpin->value() << std::endl;
}

bool ClustSVM::LoadParams(char *line, float value)
{
	if(endsWith(line,"kernelDeg")) params->kernelDegSpin->setValue((int)value);
	if(endsWith(line,"kernelType")) params->kernelTypeCombo->setCurrentIndex((int)value);
	if(endsWith(line,"kernelWidth")) params->kernelWidthSpin->setValue(value);
	if(endsWith(line,"svmType")) params->svmTypeCombo->setCurrentIndex((int)value);
	if(endsWith(line,"svmC")) params->svmCSpin->setValue(value);
	if(endsWith(line,"kernelCluster")) params->kernelClusterSpin->setValue((int)value);
	return true;
}
