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
#include <QMutexLocker>

#include "public.h"
#include "basicMath.h"
#include "drawTimer.h"

using namespace std;

DrawTimer::DrawTimer(Canvas *canvas, QMutex *mutex)
    : refineLevel(0),
      refineMax(10),
      perm(0),
      canvas(canvas),
      w(0), h(0),
      dim(2),
      classifier(0),
      regressor(0),
      dynamical(0),
      clusterer(0),
      maximizer(0),
      reinforcement(0),
      mutex(mutex),
      glw(0),
      bPaused(false),
      bRunning(false),
      bColorMap(true),
      maximumVisitedCount(0)
{}

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
    //if(!perm || w != canvas->width() || h != canvas->height())
    //{
    //    KILL(perm);
    //    perm = randPerm(w*h);
    //}
    w = canvas->width();
    h = canvas->height();
    drawMutex.lock();
    KILL(perm);
    perm = randPerm(w*h);
    bigMap = QImage(QSize(w,h), QImage::Format_ARGB32);
    bigMap.fill(0xffffff);
    modelMap = QImage(QSize(w,h), QImage::Format_ARGB32);
    modelMap.fill(qRgba(255, 255, 255, 0));
    drawMutex.unlock();
    /*
    glw->mutex->lock();
    FOR(i, glw->objects.size())
    {
        if(glw->objects[i].objectType.contains("Dynamize"))
        {
            glw->objects.erase(glw->objects.begin() + i);
            i--;
        }
    }
    glw->mutex->unlock();
    */
}

void DrawTimer::run()
{
    bRunning = true;
    while(bRunning)
    {
        if(!canvas || canvas->canvasType > 1) break;
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
                if(dynamical && (*dynamical))
                {
                    if(!bColorMap) emit MapReady(modelMap);
                    else
                    {
                        emit MapReady(bigMap);
                        /*
                        QPainter painter(&modelMap);
                        painter.setRenderHint(QPainter::Antialiasing);
                        painter.setCompositionMode(QPainter::CompositionMode_Multiply);
                        painter.drawImage(QPointF(0,0),bigMap);
                        painter.end();
                        emit MapReady(modelMap);
                        */
                    }
                }
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

        vector< vector<fvec> > trajectories(targets.size());
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
            trajectories[i] = targetTrajectory;
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

        vector< vector<fvec> > trajectories(targets.size());
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
            trajectories[i] = targetTrajectory;
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
    if(refineLevel > refineMax) {
        refineLevel = -1;
        return;
    }
    if(canvas->width() != w || canvas->height() != h) {
        Clear();
        return;
    }
    bool bRefined = true;
    if(refineLevel == 0) {
        Clear();
        if((maximizer && (*maximizer)) || (reinforcement && (*reinforcement))) {
            refineMax = 100;
        }
        if(dynamical && (*dynamical)) {
            refineMax = 32;
            drawMutex.lock();
            if(bColorMap) modelMap.fill(qRgba(0,0,0,255));
            drawMutex.unlock();
        } else {
            refineMax = 32;
        }
    } else {
        int count = (w*h) / refineMax;
        int start = count * (refineLevel-1);
        int stop = count * refineLevel;
        if(refineLevel == refineMax) stop = w*h; // we want to be sure we paint everything in the end

        if(maximizer && (*maximizer)) {
            Maximization();
            if((*maximizer)->hasConverged()) refineLevel=refineMax+1;
            else refineLevel = 1;
            return;
        }

        if(reinforcement && (*reinforcement)) {
            Reinforce();
            if((*reinforcement)->hasConverged()) refineLevel=refineMax+1;
            else refineLevel = 1;
            return;
        }

        mutex->lock();
        int dim = canvas->data->GetDimCount();
        if(inputDims.size() == 2) dim = inputDims.size();
        mutex->unlock();

        if(dim == 2) {
            bRefined &= TestFast(start,stop); // we finish the current batch
            if(dynamical && (*dynamical)) {
                int cnt = 10000 / refineMax;
                int steps = 8;
                bRefined &= VectorsFast(cnt, steps);
            }
        }
        /*
        else if(dim == 3 && canvas->canvasType == 1)
        {
            if(dynamical && (*dynamical))
            {
                //int cnt = 4096 / refineMax;
                int cnt = 512 / refineMax;
                int steps = 64;
                bRefined &= VectorsGL(cnt, steps);
            }
        }
        */
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
    vector<Obstacle> obstacles = canvas->data->GetObstacles();
    mutex->unlock();
    //float dT = 0.02f;
    fvec sample(2,0);
    int w = canvas->width();
    int h = canvas->height();
    QMutexLocker drawLock(&drawMutex);
    QPainter painter(&modelMap);
    painter.setRenderHint(QPainter::Antialiasing, true);

    FOR(i, count)
    {
        QPointF samplePre(rand()/(float)RAND_MAX * w, rand()/(float)RAND_MAX * h);
        sample = canvas->toSampleCoords(samplePre);
        float color = 0; // 255 - (rand()/(float)RAND_MAX*0.7f)*255.f;
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
            painter.setOpacity(1 - speed);
            QColor c(color,color,color);
            painter.setPen(QPen(c, 0.25));
            painter.drawLine(point, oldPoint);
            oldPoint = point;
        }
    }
    return true;
}

bool DrawTimer::VectorsGL(int count, int steps)
{
    if(!bRunning || !mutex) return false;
    float dT=0.001;
    mutex->lock();
    if(!(*dynamical)) return false;
    dT = (*dynamical)->dT*2; // in 3d we want longer 'trails'
    int dim = canvas->data->GetDimCount();
    if(dim != 3) return false;
    int xInd = canvas->xIndex;
    int yInd = canvas->yIndex;
    int zInd = canvas->zIndex;
    vector<fvec> samples = canvas->data->GetSamples();
    vector<Obstacle> obstacles = canvas->data->GetObstacles();
    mutex->unlock();

    fvec sample(dim,0);
    float minv=FLT_MAX, maxv=-FLT_MAX;
    FOR(i, samples.size())
    {
        FOR(d, dim)
        {
            minv = min(minv, samples[i][d]);
            maxv = max(maxv, samples[i][d]);
        }
    }
    float diff = maxv-minv;
    minv = minv - diff*0.5f;
    maxv = maxv + diff*0.5f;
    diff = maxv - minv;

    glw->mutex->lock();
    vector<GLObject> objects = glw->objects;
    vector<bool> objectAlive = glw->objectAlive;
    glw->mutex->unlock();

    int oIndex = -1;
    GLObject o;
    FOR(i, objects.size())
    {
        if(!objectAlive[i]) continue;
        if(objects[i].objectType.contains("Dynamize"))
        {
            oIndex = i;
            o = objects[i];
            break;
        }
    }
    if(oIndex == -1)
    {
        o.objectType = "Dynamize,Lines";
        o.style = QString("fading:%1").arg(steps);
    }

    FOR(i, count)
    {
        /*
        int index = prevCount + i;
        int x = index % 16;
        int y = (index / 16) % 16;
        int z = (index / 256);
        sample[xInd] = (x/16.f)*diff + minv;
        sample[yInd] = (y/16.f)*diff + minv;
        sample[zInd] = (z/16.f)*diff + minv;
        */
        FOR(d, dim) sample[d] = drand48()*diff+ minv;

        fvec oldSample = sample;
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
            o.vertices.append(QVector3D(oldSample[xInd],oldSample[yInd], zInd >= 0 && zInd < dim ? oldSample[zInd] : 0));
            o.vertices.append(QVector3D(sample[xInd],sample[yInd], zInd >= 0 && zInd < dim ? sample[zInd] : 0));
            oldSample = sample;
        }
    }
    glw->mutex->lock();
    //if(oIndex != -1) glw->objects[oIndex] = o;
    //else glw->objects.push_back(o);
    if(oIndex != -1) glw->killList.push_back(oIndex);
    glw->AddObject(o);
    glw->mutex->unlock();
    return true;
}

void DrawTimer::Maximization()
{
    if(!maximizer || !(*maximizer)) return;
    QMutexLocker lock(mutex);
    if(!bRunning) return;

    fvec maxSample = (*maximizer)->Test((*maximizer)->Maximum());
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

    // we paint the visited history
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

    if(glw && canvas->canvasType == 1)
    {
        glw->mutex->lock();
        vector<GLObject> objects = glw->objects;
        glw->mutex->unlock();
        int oIndex = -1;
        FOR(i, objects.size())
        {
            if(objects[i].objectType.contains("Samples") &&
                    objects[i].objectType.contains("Maximization"))
            {
                oIndex = i;
                break;
            }
        }
        GLObject o;
        if(oIndex != -1) o = objects[oIndex];
        else
        {
            o.objectType = "Samples,Maximization";
            o.style ="pointsize:6,dotted";
        }
        if(o.colors.size()) o.colors.back() = QVector4D(0,0,0,1);
        // we replace all past points as history

        for(int i=maximumVisitedCount; i<visited.size(); i++)
        {
            fvec &sample = visited[i];
            // we want to paint the last visited points
            double value = (*maximizer)->GetValue(sample);
            value = max(0.,min(1.,value))*0.5;
            o.vertices.push_back(QVector3D(sample[0]*2-1, value+0.02, sample[1]*2-1));
            o.colors.push_back(QVector4D(0,0,0,1));
        }
        vector<GLObject> oList = (*maximizer)->DrawGL();

        glw->mutex->lock();
        if(oIndex != -1) glw->killList.push_back(oIndex);
        glw->AddObject(o);

        FOR(i, glw->objects.size())
        {
            if(!glw->objectAlive[i]) continue;
            if(glw->objects[i].objectType.contains("Maximizer"))
            {
                glw->killList.push_back(i);
            }
        }
        FOR(i, oList.size())
        {
            glw->AddObject(oList[i]);
        }
        glw->mutex->unlock();
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
    vector<Obstacle> obstacles = canvas->data->GetObstacles();
    FOR(i, count)
    {
        QPointF samplePre(rand()/(float)RAND_MAX * w, rand()/(float)RAND_MAX * h);
        sample = canvas->toSampleCoords(samplePre);
        float color = bColorMap ? 255 : 0;
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

QColor DrawTimer::GetColor(Classifier *classifier, fvec sample, std::vector<Classifier*> *classifierMulti, ivec sourceDims)
{
    if(sourceDims.size())
    {
        fvec newSample(sourceDims.size());
        FOR(d, sourceDims.size()) newSample[d] = sample[d];
        sample = newSample;
    }

    QColor c;
    if(classifier->IsMultiClass())
    {
        fvec val = classifier->TestMulti(sample);
        if(!val.size()) return QColor(0,0,0);
        else if(val.size() == 1)
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
                int index = (classifier->inverseMap[j]%SampleColorCnt);
                r += SampleColor[index].red()*val[j]*sum;
                g += SampleColor[index].green()*val[j]*sum;
                b += SampleColor[index].blue()*val[j]*sum;
            }
            r = max(0.f, min(255.f, r));
            g = max(0.f, min(255.f, g));
            b = max(0.f, min(255.f, b));
            c = QColor(r,g,b);
        }
    }
    else
    {
        if(classifierMulti && (*classifierMulti).size())
        {
            // we find the max
            int maxClass = 0;
            float maxRes = -FLT_MAX;
            fvec val((*classifierMulti).size(),0);
            FOR(j, (*classifierMulti).size())
            {
                float res = (*classifierMulti)[j]->Test(sample);
                val[j] = res;
                if(res > maxRes)
                {
                    maxRes = res;
                    maxClass = j;
                }
            }
            val[maxClass] *= 3;
            float sum = 0;
            FOR(i, val.size()) sum += fabs(val[i]);
            sum = 1.f/sum;

            float r=0,g=0,b=0;
            FOR(j, val.size())
            {
                //int index = j%SampleColorCnt;
                int index = classifier->inverseMap[j]%SampleColorCnt;
                r += SampleColor[index].red()*val[j]*sum;
                g += SampleColor[index].green()*val[j]*sum;
                b += SampleColor[index].blue()*val[j]*sum;
            }
            r = max(0.f, min(255.f, r));
            g = max(0.f, min(255.f, g));
            b = max(0.f, min(255.f, b));
            c = QColor(r,g,b);
        }
        else
        {
            float v = classifier->Test(sample);
            int color = (int)(fabs(v)*128);
            color = max(0,min(color, 255));
            if(v > 0) c = QColor(color,0,0);
            else c = QColor(color,color,color);
        }
    }
    return c;
}

inline void fromCanvas(fvec &sample, const float x, const float y,
                       const int height, const int width,
                       const float zxh, const float zyh,
                       int xIndex, int yIndex, const fvec &center, const bool bRestrictedDims)
{
    sample = center;
    if(bRestrictedDims) {
        sample[0] += (x - width*0.5f)*zxh;
        sample[1] += (-y + height*0.5f)*zyh;
    } else {
        sample[xIndex] += (x - width*0.5f)*zxh;
        sample[yIndex] += (-y + height*0.5f)*zyh;
    }
}

bool DrawTimer::TestFast(int start, int stop)
{
    if(stop < 0 || stop > w*h) stop = w*h;
    mutex->lock();
    int dim=canvas->data->GetDimCount();
    vector<Obstacle> obstacles = canvas->data->GetObstacles();
    int xIndex = canvas->xIndex;
    int yIndex = canvas->yIndex;
    bool bRestrictedDims = false;
    if(inputDims.size() == 2 && xIndex == inputDims.front() && yIndex == inputDims.back()) {
        bRestrictedDims = true;
        dim = 2;
    }
    int cheight = canvas->height();
    int cwidth = canvas->width();
    float zxh = 1.f / (canvas->zoom*canvas->zooms[xIndex]*cheight);
    float zyh = 1.f / (canvas->zoom*canvas->zooms[yIndex]*cheight);
    fvec center = canvas->center;
    if(bRestrictedDims) {
        fvec newCenter(2,0);
        newCenter[0] = center[xIndex];
        newCenter[1] = center[yIndex];
        center = newCenter;
    }
    mutex->unlock();
    if(dim > 2) return false; // we dont want to draw multidimensional stuff, it's ... problematic
    fvec sampleMatrix(dim*(stop-start));
    vector<fvec> samples(stop-start);
    vector<int> X(stop-start);
    vector<int> Y(stop-start);
    FOR(i, stop-start) {
        drawMutex.lock();
        if(!perm) perm = randPerm(w*h);
        int x = perm[i+start]%w;
        int y = perm[i+start]/w;
        if(x >= bigMap.width() || y >= bigMap.height())
        {
            drawMutex.unlock();
            continue;
        }
        drawMutex.unlock();
        X[i] = x;
        Y[i] = y;
        fromCanvas(samples[i], x, y, cheight, cwidth, zxh, zyh, xIndex, yIndex, center, bRestrictedDims);
    }

    FOR(i, stop-start) {
        fvec& sample = samples[i];
        int x = X[i];
        int y = Y[i];

        QMutexLocker lock(mutex);
        if((*classifier)) {
            QColor c = GetColor(*classifier, sample, classifierMulti);
            drawMutex.lock();
            bigMap.setPixel(x,y,c.rgb());
            drawMutex.unlock();
        } else if(*regressor) {
            //fvec val = (*regressor)->Test(sample);
        } else if(*clusterer) {
            fvec res = (*clusterer)->Test(sample);
            float r=0,g=0,b=0;
            if(res.size() > 1) {
                FOR(j, res.size()) {
                    r += SampleColor[(j+1)%SampleColorCnt].red()*res[j];
                    g += SampleColor[(j+1)%SampleColorCnt].green()*res[j];
                    b += SampleColor[(j+1)%SampleColorCnt].blue()*res[j];
                }
            } else if(res.size()) {
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
        } else if(*dynamical && bColorMap) {
            QColor color;
            fvec val = (*dynamical)->Test(sample);
            if((*dynamical)->avoid) {
                (*dynamical)->avoid->SetObstacles(obstacles);
                fVec newRes = (*dynamical)->avoid->Avoid(sample, val);
                val = newRes;
            }
            float speed = sqrtf(val[0]*val[0] + val[1]*val[1]);
            speed = min(1.f,speed);
            const int colorStyle = 1;
            if(colorStyle == 0) {// velocity as colors
                int hue = (int)((atan2(val[0], val[1]) / (2*M_PI) + 0.5) * 359);
                hue = max(0, min(359,hue));
                color = QColor::fromHsv(hue, 255, 255);
                color.setRed(255*(1-speed) + color.red()*speed);
                color.setGreen(255*(1-speed) + color.green()*speed);
                color.setBlue(255*(1-speed) + color.blue()*speed);
            } else if(colorStyle == 1) {// speed as color
                color = QColor(Canvas::GetColorMapValue(speed, 2));
            }
            drawMutex.lock();
            bigMap.setPixel(x,y,color.rgb());
            drawMutex.unlock();
        }
    }
    return true;
}
