/*********************************************************************
MLDemos: A User-Friendly visualization toolkit for machine learning
Copyright (C) 2010  Basilio Noris
Contact: mldemos@b4silio.com

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free
Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*********************************************************************/
#include <QtGui>
#include <QWidget>
#include <QSize>
#include <QPixmap>
#include <QDebug>
#include <QMutexLocker>

#include "public.h"
#include "basicMath.h"
#include "drawTimer.h"

using namespace std;

DrawTimer::DrawTimer(Canvas *canvas, QMutex *mutex)
	: canvas(canvas),
	  refineLevel(0),
	  refineMax(10),
	  classifier(0),
	  regressor(0),
	  dynamical(0),
	  clusterer(0),
	  bRunning(false),
	  bPaused(false),
	  bColorMap(true),
	  mutex(mutex),
	  perm(0), w(0), h(0), dim(2)
{

}

DrawTimer::~DrawTimer()
{
    KILL(perm);
}

void DrawTimer::Stop()
{
	bRunning = false;
}

void DrawTimer::Clear()
{
	refineLevel = 0;
	w = canvas->width();
	h = canvas->height();
	drawMutex.lock();
	bigMap = QImage(QSize(w,h), QImage::Format_ARGB32);
	bigMap.fill(0xffffff);
	modelMap = QImage(QSize(w,h), QImage::Format_ARGB32);
	modelMap.fill(qRgba(255, 255, 255, 0));
	KILL(perm);
	perm = randPerm(w*h);
	drawMutex.unlock();
}

void DrawTimer::run()
{
	bRunning = true;
	while(bRunning)
	{
        if(!canvas || canvas->canvasType) break;
		if((!classifier || !(*classifier)) && (!regressor || !(*regressor)) && (!dynamical || !(*dynamical)) && (!clusterer || !(*clusterer)) && (!maximizer || !(*maximizer)))
		{
			//if(refineLevel) Clear();
			Clear();
			bRunning = false;
			return;
		}

		// we refine the current map
		Refine();
		// and we send the image to the canvas
		drawMutex.lock();
		//emit MapReady(bigMap);
		//if(dynamical && (*dynamical) || maximizer && (*maximizer) ) emit ModelReady(modelMap);
		if(maximizer && (*maximizer))
		{            
			emit ModelReady(modelMap);
			emit CurveReady();
			//canvas->SetModelImage(modelMap);
		}
		else
		{
			if(dynamical && (*dynamical))  emit bColorMap ? MapReady(bigMap) : MapReady(modelMap);
			else emit MapReady(bigMap);
		}
		drawMutex.unlock();
		//qApp->processEvents();
		this->msleep(50);
	}
	bRunning = false;
}

void DrawTimer::Refine()
{
	if(refineLevel > refineMax)
	{
		bRunning = false;
		return;
	}
	if(canvas->width() != w || canvas->height() != h)
	{
		Clear();
		return;
	}
	if(refineLevel == 0)
	{
		Clear();
		if(maximizer && (*maximizer))
		{
			refineMax = 100;
		}
		else
		{
			refineMax = 20;
		}
	}
	else
	{
		int count = (w*h) / refineMax;
		int start = count * (refineLevel-1);
		int stop = count * refineLevel;
		if(refineLevel == refineMax) stop = w*h; // we want to be sure we paint everything in the end

		if(maximizer && (*maximizer))
		{
			Maximization();
			if((*maximizer)->hasConverged()) refineLevel=refineMax+1;
			else refineLevel = 1;
			return;
		}

		TestFast(start,stop); // we finish the current batch
		if(dynamical && (*dynamical) && !bColorMap)
		{
			int cnt = 10000 / refineMax;
			int steps = 8;
			VectorsFast(cnt, steps);
		}
	}
	refineLevel++;
}

void DrawTimer::Vectors(int count, int steps)
{
	if(!bRunning || !mutex) return;
	mutex->lock();
	if(!(*dynamical)) return;
	float dT = (*dynamical)->dT;// * (dynamical->count/100.f);
	mutex->unlock();
	//float dT = 0.02f;
	fvec sample;
	sample.resize(2,0);
	int w = canvas->width();
	int h = canvas->height();
	QMutexLocker drawLock(&drawMutex);
	QPainter painter(&modelMap);
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.setRenderHint(QPainter::HighQualityAntialiasing, true);
	vector<Obstacle> obstacles = canvas->data->GetObstacles();

	QPointF oldPoint(-FLT_MAX,-FLT_MAX);
	QPointF oldPointUp(-FLT_MAX,-FLT_MAX);
	QPointF oldPointDown(-FLT_MAX,-FLT_MAX);
	FOR(i, count)
	{
		QPointF samplePre(rand()/(float)RAND_MAX * w, rand()/(float)RAND_MAX * h);
		sample = canvas->toSampleCoords(samplePre);
		float color = (rand()/(float)RAND_MAX*0.7f)*255.f;
		color = 0;
		QPointF oldPoint = canvas->toCanvasCoords(sample);
		FOR(j, steps)
		{
			if(!(*dynamical)) return;
			mutex->lock();
			fvec res = (*dynamical)->Test(sample);
			if((*dynamical)->avoid)
			{
				(*dynamical)->avoid->SetObstacles(obstacles);
				fvec newRes = (*dynamical)->avoid->Avoid(sample, res);
				res = newRes;
			}
			mutex->unlock();
			sample += res*dT;
			float speed = sqrtf(res[0]*res[0] + res[1]*res[1]);
			QPointF point = canvas->toCanvasCoords(sample);
			painter.setOpacity(speed);
			QColor c(color,color,color);
			painter.setPen(QPen(c, 0.25));
			painter.drawLine(point, oldPoint);
			oldPoint = point;
		}
	}
}

void DrawTimer::Maximization()
{
	if(!maximizer || !(*maximizer)) return;
	QMutexLocker lock(mutex);
	if(!bRunning) return;

	fvec sample = (*maximizer)->Test((*maximizer)->Maximum());

	double value = (*maximizer)->MaximumValue();
	if(value >= (*maximizer)->stopValue)
	{
		(*maximizer)->SetConverged(true);
	}
	if((*maximizer)->age >= (*maximizer)->maxAge)
	{
		(*maximizer)->SetConverged(true);
	}
	else (*maximizer)->age++;

	QMutexLocker drawLock(&drawMutex);
	int w = modelMap.width();
	int h = modelMap.height();
	if(modelMap.isNull() || !w || !h) return;
	modelMap.fill(qRgba(255, 255, 255, 0));

	QPainter painter(&modelMap);
	painter.setRenderHint(QPainter::Antialiasing, true);
    //painter.setRenderHint(QPainter::HighQualityAntialiasing, true);

	(*maximizer)->Draw(painter);

	QPointF point(sample[0]*w, sample[1]*h);
	painter.setPen(QPen(Qt::black, 1.5));
	painter.setBrush(Qt::NoBrush);
	painter.drawEllipse(point, 3, 3);
}

void DrawTimer::VectorsFast(int count, int steps)
{
	if(!(*dynamical)) return;
	if(!bRunning || !mutex) return;
	QPointF oldPoint(-FLT_MAX,-FLT_MAX);
	QPointF oldPointUp(-FLT_MAX,-FLT_MAX);
	QPointF oldPointDown(-FLT_MAX,-FLT_MAX);
	mutex->lock();
	float dT = (*dynamical)->dT;// * (dynamical->count/100.f);
	mutex->unlock();
	//float dT = 0.02f;
	fvec sample;
	sample.resize(2,0);
	int w = canvas->width();
	int h = canvas->height();
	QMutexLocker drawLock(&drawMutex);
	QPainter painter(&modelMap);
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.setRenderHint(QPainter::HighQualityAntialiasing, true);
	vector<Obstacle> obstacles = canvas->data->GetObstacles();
	FOR(i, count)
	{
		QPointF samplePre(rand()/(float)RAND_MAX * w, rand()/(float)RAND_MAX * h);
		sample = canvas->toSampleCoords(samplePre);
		float color = (rand()/(float)RAND_MAX*0.7f)*255.f;
		color = 0;
		QPointF oldPoint = canvas->toCanvasCoords(sample);
		FOR(j, steps)
		{
			if(!(*dynamical)) return;
			mutex->lock();
			fvec res = (*dynamical)->Test(sample);
			if((*dynamical)->avoid)
			{
				(*dynamical)->avoid->SetObstacles(obstacles);
				fvec newRes = (*dynamical)->avoid->Avoid(sample, res);
				res = newRes;
			}
			mutex->unlock();
			sample += res*dT;
			float speed = sqrtf(res[0]*res[0] + res[1]*res[1]);
			QPointF point = canvas->toCanvasCoords(sample);
			painter.setOpacity(speed);
			QColor c(color,color,color);
			painter.setPen(QPen(c, 0.25));
			painter.drawLine(point, oldPoint);
			oldPoint = point;
		}
	}
}

QColor DrawTimer::GetColor(Classifier *classifier, fvec sample)
{
    QColor c;
    if(classifier->IsMultiClass())
    {
        fvec val = classifier->TestMulti(sample);
        if(val.size() == 1)
        {
            float v = val[0];
            int color = fabs(v)*128;
            color = max(0,min(color, 255));
            if(v > 0) c = QColor(color,0,0);
            else c = QColor(color,color,color);
        }
        else
        {
            // we find the max
            int maxVal = 0;
            FOR(i, val.size()) if (val[maxVal] < val[i]) maxVal = i;
            val[maxVal] *= 3;
            float sum = 0;
            FOR(i, val.size()) sum += fabs(val[i]);
            sum = 1.f/sum;

            float r=0,g=0,b=0;
            FOR(j, val.size())
            {
                int index = classifier->inverseMap[j];
                r += SampleColor[index%SampleColorCnt].red()*val[j]*sum;
                g += SampleColor[index%SampleColorCnt].green()*val[j]*sum;
                b += SampleColor[index%SampleColorCnt].blue()*val[j]*sum;
            }
            c = QColor(max(0.f,min(255.f,r)),max(0.f,min(255.f,g)),max(0.f,min(255.f,b)));
        }
    }
    else
    {
        float v = classifier->Test(sample);
        int color = (int)(fabs(v)*128);
        color = max(0,min(color, 255));
        if(v > 0) c = QColor(color,0,0);
        else c = QColor(color,color,color);
    }
    return c;
}

void DrawTimer::TestFast(int start, int stop)
{
	if(stop < 0 || stop > w*h) stop = w*h;
	mutex->lock();
	int dim=canvas->data->GetDimCount();
	vector<Obstacle> obstacles = canvas->data->GetObstacles();
	mutex->unlock();
	if(dim > 2) return; // we dont want to draw multidimensional stuff, it's ... problematic
	fvec sample(dim);
	for (int i=start; i<stop; i++)
	{
		drawMutex.lock();
		int x = perm[i]%w;
		int y = perm[i]/w;
		if(x >= bigMap.width() || y >= bigMap.height()) continue;
		drawMutex.unlock();
		sample = canvas->fromCanvas(x,y);
		fvec val(dim);
		float v;
		QMutexLocker lock(mutex);
		if((*classifier))
		{
            QColor c = GetColor(*classifier, sample);
			drawMutex.lock();
			bigMap.setPixel(x,y,c.rgb());
			drawMutex.unlock();
		}
		else if(*regressor)
		{
			val = (*regressor)->Test(sample);
		}
		else if(*clusterer)
		{
			fvec res = (*clusterer)->Test(sample);
			float r=0,g=0,b=0;
			if(res.size() > 1)
			{
				FOR(i, res.size())
				{
					r += SampleColor[(i+1)%SampleColorCnt].red()*res[i];
					g += SampleColor[(i+1)%SampleColorCnt].green()*res[i];
					b += SampleColor[(i+1)%SampleColorCnt].blue()*res[i];
				}
			}
			else if(res.size())
			{
				r = (1-res[0])*255 + res[0]* 255;
				g = (1-res[0])*255;
				b = (1-res[0])*255;
			}
			if( r < 10 && g < 10 && b < 10) r = b = g = 255;
			QColor c = QColor(r,g,b);
			drawMutex.lock();
			bigMap.setPixel(x,y,c.rgb());
			drawMutex.unlock();
		}
		else if(*dynamical && bColorMap)
		{
			val = (*dynamical)->Test(sample);
			if((*dynamical)->avoid)
			{
				(*dynamical)->avoid->SetObstacles(obstacles);
				fVec newRes = (*dynamical)->avoid->Avoid(sample, val);
				val = newRes;
			}
			float speed = sqrtf(val[0]*val[0] + val[1]*val[1]);
			speed = min(1.f,speed);
			int hue = (int)((atan2(val[0], val[1]) / (2*M_PI) + 0.5) * 359);
			QColor color = QColor::fromHsv(hue, 255, 255);
			color.setRed(255*(1-speed) + color.red()*speed);
			color.setGreen(255*(1-speed) + color.green()*speed);
			color.setBlue(255*(1-speed) + color.blue()*speed);
			drawMutex.lock();
			bigMap.setPixel(x,y,color.rgb());
			drawMutex.unlock();
		}
	}
}
