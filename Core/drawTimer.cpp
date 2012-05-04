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
      maximizer(0),
      reinforcement(0),
	  bRunning(false),
	  bPaused(false),
	  bColorMap(true),
	  mutex(mutex),
      perm(0), w(0), h(0), dim(2), maximumVisitedCount(0)
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
    maximumVisitedCount = 0;
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
        if((!classifier || !(*classifier)) &&
                (!regressor || !(*regressor)) &&
                (!dynamical || !(*dynamical)) &&
                (!clusterer || !(*clusterer)) &&
                (!maximizer || !(*maximizer)) &&
                (!reinforcement || !(*reinforcement)))
		{
			Clear();
			bRunning = false;
			return;
		}

		// we refine the current map
		Refine();

        // we animate
        Animate();

        if(refineLevel >= 0)
        {
            // and we send the image to the canvas
            drawMutex.lock();
            if(maximizer && (*maximizer))
            {
                emit ModelReady(modelMap);
                emit MapReady(bigMap);
                emit CurveReady();
            }
            else if (reinforcement && (*reinforcement))
            {
                emit ModelReady(modelMap);
                //emit CurveReady();
            }
            else
            {
                if(dynamical && (*dynamical))  emit bColorMap ? MapReady(bigMap) : MapReady(modelMap);
                else emit MapReady(bigMap);
            }
            drawMutex.unlock();
        }
        else if (!dynamical || !(*dynamical) || !reinforcement || (*reinforcement)) // no animations to be done
        {
            break;
        }

        // wait a while
        this->msleep(40);
	}
	bRunning = false;
}

void DrawTimer::Animate()
{
    mutex->lock();
    drawMutex.lock();
    if(dynamical && (*dynamical) && canvas->targets.size()) // we need to animate the targets
    {
        float dT = (*dynamical)->dT;// * (dynamical->count/100.f);
        int w = canvas->width(), h = canvas->height();
        vector<Obstacle> obstacles = canvas->data->GetObstacles();
        vector<fvec> targets = canvas->targets;
        ivec ages = canvas->targetAge;
        drawMutex.unlock();
        if((*dynamical)->avoid) (*dynamical)->avoid->SetObstacles(obstacles);

        vector< vector<fvec> > trajectories;
        // animate each target
        FOR(i, targets.size())
        {
            ages[i]++;
            if(ages[i] > 400) ages[i] = 0; // we restart

            vector<fvec> targetTrajectory(ages[i]+1);
            fvec sample = targets[i];
            targetTrajectory[0] = sample;
            FOR(j, ages[i])
            {
                fvec res = (*dynamical)->Test(sample);
                if((*dynamical)->avoid)
                {
                    fvec newRes = (*dynamical)->avoid->Avoid(sample, res);
                    sample += newRes*dT;
                }
                else sample += res*dT;
                targetTrajectory[j+1] = sample;
            }
            if(ages[i] > 2)
            {
                fvec diff = targetTrajectory[ages[i]] - targetTrajectory[ages[i]-1];
                float speed = 0;
                FOR(d, diff.size()) speed += diff[d]*diff[d];
                speed = sqrtf(speed);
                if(speed <= 1e-5) ages[i] = 0;
            }
            trajectories.push_back(targetTrajectory);
        }
        mutex->unlock();

        // we update the ages and create the trajectories
        QList<QPainterPath> paths;
        QList<QPointF> startPoints;
        QList<QPointF> endPoints;
        drawMutex.lock();
        FOR(i, targets.size())
        {
            QPainterPath path;
            QPointF point;
            FOR(j, trajectories[i].size())
            {
                point = canvas->toCanvasCoords(trajectories[i][j]);
                if(!j)
                {
                    path.moveTo(point);
                    startPoints.push_back(point);
                }
                else path.lineTo(point);
            }
            paths.push_back(path);
            endPoints.push_back(point);
            canvas->targetAge[i] = ages[i];
        }
        drawMutex.unlock();

        // and now we paint
        if(animationImage.isNull() || animationImage.width() != w || animationImage.height() != h)
        {
            animationImage = QImage(w,h,QImage::Format_ARGB32);
        }
        animationImage.fill(Qt::transparent);
        QPainter painter(&animationImage);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(Qt::red, 3, Qt::SolidLine));
        FOR(i, paths.size())
        {
            painter.drawPath(paths[i]);
        }
        painter.setPen(QPen(Qt::blue, 2));
        FOR(i, startPoints.size())
        {
            painter.drawEllipse(startPoints[i], 8, 8);
        }
        FOR(i, endPoints.size())
        {
            painter.drawEllipse(endPoints[i], 8, 8);
        }
        emit(AnimationReady(animationImage));
    }
    else if(reinforcement && (*reinforcement))
    {
        int w = canvas->width(), h = canvas->height();
        vector<fvec> targets = canvas->targets;
        ivec ages = canvas->targetAge;
        drawMutex.unlock();

        vector< vector<fvec> > trajectories;
        // animate each target
        FOR(i, targets.size())
        {
            ages[i]++;
            if(ages[i] > 30) ages[i] = 0; // we restart

            vector<fvec> targetTrajectory(ages[i]+1);
            QPointF samplePoint = canvas->toCanvasCoords(targets[i]);
            fvec sample(2);
            sample[0] = samplePoint.x() / w;
            sample[1] = samplePoint.y() / h;
            targetTrajectory[0] = sample;
            targetTrajectory[0][0] *= w;
            targetTrajectory[0][1] *= h;
            FOR(j, ages[i])
            {
                sample = reinforcementProblem->PerformAction(sample);
                targetTrajectory[j+1] = sample;
                targetTrajectory[j+1][0] *= w;
                targetTrajectory[j+1][1] *= h;
            }
            if(ages[i] > 2)
            {
                fvec diff = targetTrajectory[ages[i]] - targetTrajectory[ages[i]-1];
                float speed = 0;
                FOR(d, diff.size()) speed += diff[d]*diff[d];
                speed = sqrtf(speed);
                if(speed <= 1e-5) ages[i] = 0;
            }
            trajectories.push_back(targetTrajectory);
        }
        mutex->unlock();

        // we update the ages and create the trajectories
        QList<QPainterPath> paths;
        QList<QPointF> startPoints;
        QList<QPointF> endPoints;
        drawMutex.lock();
        FOR(i, trajectories.size())
        {
            QPainterPath path;
            QPointF point;
            FOR(j, trajectories[i].size())
            {
                point = QPointF(trajectories[i][j][0],trajectories[i][j][1]);
                if(!j)
                {
                    path.moveTo(point);
                    startPoints.push_back(point);
                }
                else path.lineTo(point);
            }
            paths.push_back(path);
            endPoints.push_back(point);
            canvas->targetAge[i] = ages[i];
        }
        drawMutex.unlock();

        // and now we paint
        if(animationImage.isNull() || animationImage.width() != w || animationImage.height() != h)
        {
            animationImage = QImage(w,h,QImage::Format_ARGB32);
        }
        animationImage.fill(Qt::transparent);
        QPainter painter(&animationImage);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(Qt::black, 2, Qt::DashLine));
        FOR(i, paths.size())
        {
            painter.drawPath(paths[i]);
        }
        painter.setPen(QPen(Qt::blue, 2));
        FOR(i, startPoints.size())
        {
            painter.drawEllipse(startPoints[i], 8, 8);
        }
        FOR(i, endPoints.size())
        {
            painter.drawEllipse(endPoints[i], 8, 8);
        }
        emit(AnimationReady(animationImage));
    }
    else
    {
        mutex->unlock();
        drawMutex.unlock();
    }
}

void DrawTimer::Refine()
{
    if(refineLevel < 0) return;
    if(refineLevel > refineMax)
	{
        refineLevel = -1;
        return;
	}
	if(canvas->width() != w || canvas->height() != h)
	{
		Clear();
		return;
	}
    bool bRefined = true;
	if(refineLevel == 0)
	{
		Clear();
        if(maximizer && (*maximizer) || reinforcement && (*reinforcement))
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

        if(reinforcement && (*reinforcement))
        {
            Reinforce();
            if((*reinforcement)->hasConverged()) refineLevel=refineMax+1;
            else refineLevel = 1;
            return;
        }

        bRefined &= TestFast(start,stop); // we finish the current batch
		if(dynamical && (*dynamical) && !bColorMap)
		{
			int cnt = 10000 / refineMax;
			int steps = 8;
            bRefined &= VectorsFast(cnt, steps);
		}
	}
    if(bRefined) refineLevel++;
}

bool DrawTimer::Vectors(int count, int steps)
{
    if(!bRunning || !mutex) return false;
    float dT=0.001;
    mutex->lock();
    if(!(*dynamical)) return false;
    dT = (*dynamical)->dT;// * (dynamical->count/100.f);
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
            if(!(*dynamical)) return false;
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
    return true;
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
    if((*maximizer)->age == 1) maximumVisitedCount = 0;

    QMutexLocker drawLock(&drawMutex);
    int w = modelMap.width();
    int h = modelMap.height();
    if(modelMap.isNull() || !w || !h) return;
    modelMap.fill(qRgba(255, 255, 255, 0));

    QPainter painter(&modelMap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    //painter.setRenderHint(QPainter::HighQualityAntialiasing, true);
    (*maximizer)->Draw(painter);

    // we draw the current maximum value on the legend
    int barW = 20;
    QRect legendRect(w - barW - 32, 40, barW, 256);
    int y = (1.-value) * legendRect.height() + legendRect.y();

    painter.setPen(QPen(Qt::black, 2));
    painter.drawEllipse(QPointF(legendRect.x()-3, y), 2, 2);
    painter.drawEllipse(QPointF(legendRect.x()+legendRect.width()+3, y), 2, 2);
    painter.setPen(QPen(Qt::black, 2));
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.drawLine(legendRect.x(), y, legendRect.x()+legendRect.width(), y);

    QPainter painter2(&bigMap);
    w = bigMap.width(), h = bigMap.height();
    painter2.setRenderHint(QPainter::Antialiasing, true);
    painter2.setPen(Qt::NoPen);
    vector<fvec> visited = (*maximizer)->Visited();
    for(int i=maximumVisitedCount; i<visited.size(); i++)
    {
        fvec &sample = visited[i];
        // we want to paint the last visited points
        double value = (*maximizer)->GetValue(sample);
        value = 255*max(0.,min(1.,value));
        QPointF point(sample[0]*w, sample[1]*h);
        painter2.setBrush(QColor(255,255-value, 255-value));
        painter2.drawEllipse(point, 8, 8);
    }
    maximumVisitedCount = visited.size();
}

void DrawTimer::Reinforce()
{
    if(!reinforcement || !(*reinforcement)) return;
    QMutexLocker lock(mutex);
    if(!bRunning) return;

    // we do ten new iterations
    int displayIterations = reinforcementProblem->displayIterationsCount;
    FOR(i, displayIterations)
    {
        reinforcementProblem->tempDirections = (*reinforcement)->Update();
        reinforcementProblem->directions = (*reinforcement)->Maximum();
    }

    // we draw the current model
    QMutexLocker drawLock(&drawMutex);
    int w = modelMap.width();
    int h = modelMap.height();
    if(modelMap.isNull() || !w || !h) return;
    modelMap.fill(qRgba(255, 255, 255, 0));

    QPainter painter(&modelMap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    //painter.setRenderHint(QPainter::HighQualityAntialiasing, true);
    reinforcementProblem->Draw(painter);
    (*reinforcement)->Draw(painter);
}

bool DrawTimer::VectorsFast(int count, int steps)
{
    if(!(*dynamical)) return false;
    if(!bRunning || !mutex) return false;
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
            if(!(*dynamical)) return false;
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
    return true;
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
            r = max(0.f, min(255.f, r));
            g = max(0.f, min(255.f, g));
            b = max(0.f, min(255.f, b));
            c = QColor(r,g,b);
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

bool DrawTimer::TestFast(int start, int stop)
{
	if(stop < 0 || stop > w*h) stop = w*h;
    int dim=2;
    vector<Obstacle> obstacles;
    mutex->lock();
    dim=canvas->data->GetDimCount();
    obstacles = canvas->data->GetObstacles();
    mutex->unlock();
    if(dim > 2) return false; // we dont want to draw multidimensional stuff, it's ... problematic
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
            r = max(0.f,min(255.f, r));
            g = max(0.f,min(255.f, g));
            b = max(0.f,min(255.f, b));
            QColor c(r,g,b);
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
            hue = max(0, min(359,hue));
            QColor color = QColor::fromHsv(hue, 255, 255);
            color.setRed(255*(1-speed) + color.red()*speed);
            color.setGreen(255*(1-speed) + color.green()*speed);
            color.setBlue(255*(1-speed) + color.blue()*speed);
            drawMutex.lock();
            bigMap.setPixel(x,y,color.rgb());
            drawMutex.unlock();
        }
    }
    return true;
}
