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
#include "mldemos.h"

void MLDemos::Drawing(fvec sample, int label)
{
    if (canvas->canvasType) return;
    int drawType = 0; // none
    if (drawToolbar->tabWidget->currentIndex() == 0) { // drawingTab
        if (drawToolbar->singleButton->isChecked()) drawType = 1;
        if (drawToolbar->sprayButton->isChecked()) drawType = 2;
        if (drawToolbar->spray3DButton->isChecked()) drawType = 9;
        if (drawToolbar->trajectoryButton->isChecked()) drawType = 6;
        if (drawToolbar->obstacleButton->isChecked()) drawType = 7;
        if (drawToolbar->paintButton->isChecked()) drawType = 8;
        if (drawToolbar->eraseButton->isChecked()) drawType = 3;
    } else if (drawToolbar->tabWidget->currentIndex() == 1) { // editTab
        if (drawToolbar->dragButton->isChecked()) drawType = -1;
        if (drawToolbar->moveButton->isChecked()) drawType = -2;
        if (drawToolbar->moveClassButton->isChecked()) drawType = -3;
        if (drawToolbar->extrudeButton->isChecked()) drawType = -4;
        if (drawToolbar->sprayClassButton->isChecked()) drawType = -5;
    } else if (drawToolbar->tabWidget->currentIndex() == 2) {
        if (drawToolbar->ellipseButton->isChecked()) drawType = 4;
        if (drawToolbar->lineButton->isChecked()) drawType = 5;
    }
    if (!drawType) return;

    if (drawType < 0) {
        Editing(-drawType, sample, label);
        return;
    }

    int speed = 6;
    bool bEmpty = canvas->data->GetCount() == 0;

    if (label) label = drawToolbar->classSpin->value();

    switch (drawType)
    {
    case 1: // single samples
        {
            // we don't want to draw too often
            if (drawTime.elapsed() < 50/speed) return; // msec elapsed since last drawing
            canvas->data->AddSample(sample, label);
            if (!selectedData.size()) selectedData.push_back(0);
        }
        break;
    case 2: // spray samples
    case 9: // spray 3D samples
        {
            // we don't want to draw too often
            if (drawTime.elapsed() < 200/speed) return; // msec elapsed since last drawing
            int type = drawToolbarContext1->randCombo->currentIndex();
            float s = drawToolbar->radiusSpin->value();
            float size = s*canvas->height();
            int count = drawToolbarContext1->spinCount->value();

            QPointF sampleCoords = canvas->toCanvasCoords(sample);
            // we generate the new data
            float variance = sqrtf(size*size/9.f*0.5f);
            int dim = canvas->data->GetDimCount();
            if (drawType == 9) dim = 3;
            int xIndex = canvas->xIndex;
            int yIndex = canvas->yIndex;
            float radius = 1.f;
            if (dim > 2) {
                fvec s1 = canvas->toSampleCoords(sampleCoords.x() - size, sampleCoords.y() - size);
                fvec s2 = canvas->toSampleCoords(sampleCoords.x() + size, sampleCoords.y() + size);
                radius = sqrtf((s1-s2)*(s1-s2))/2;
            }
            fvec newSample(2,0);
            FOR (i, count) {
                if (type == 0) { // uniform
                    newSample[0] = (rand()/(float)RAND_MAX - 0.5f)*size + sampleCoords.x();
                    newSample[1] = (rand()/(float)RAND_MAX - 0.5f)*size + sampleCoords.y();
                } else { // normal
                    newSample[0] = RandN((float)sampleCoords.x(), variance);
                    newSample[1] = RandN((float)sampleCoords.y(), variance);
                }
                fvec canvasSample = canvas->toSampleCoords(newSample[0],newSample[1]);
                while(canvasSample.size() < dim) canvasSample.push_back(0);
                if (dim > 2) {
                    FOR (d, dim) {
                        if (d == xIndex || d == yIndex) continue;
                        canvasSample[d] = (drand48()-0.5f)*radius;
                    }
                }
                canvas->data->AddSample(canvasSample, label);
                if (bEmpty) {
                    if (canvas->zooms.size() != dim) {
                        while(canvas->zooms.size() < dim) canvas->zooms.push_back(1.f);
                        while(canvas->center.size() < dim) canvas->center.push_back(0.f);
                    }
                }
                if (!selectedData.size()) selectedData.push_back(0);
            }
        }
        break;
    case 3: // erase
        {
            float s = drawToolbar->radiusSpin->value();
            float size = s*canvas->height();
            QPointF center = canvas->toCanvasCoords(sample);
            bool anythingDeleted = canvas->DeleteData(center, size/2);
            if (anythingDeleted) {
                drawTimer->Stop();
                drawTimer->Clear();
                QMutexLocker lock(&mutex);
                if (algo->dynamical && algo->dynamical->avoid) algo->dynamical->avoid->SetObstacles(canvas->data->GetObstacles());
                drawTimer->start(QThread::NormalPriority);
                canvas->ResetSamples();
            }
        }
        break;
    case 4: // ellipse
        {
            if (drawTime.elapsed() < 200/speed) return; // msec elapsed since last drawing
            float aX = drawToolbarContext2->spinSigmaX->value();
            float aY = drawToolbarContext2->spinSigmaY->value();
            float angle = -drawToolbarContext2->spinAngle->value()/180.f*PIf;
            int count = drawToolbarContext1->spinCount->value()+1;
            float sin_angle = sinf(angle);
            float cos_angle = cosf(angle);

            QPointF oldPoint, point;
            float startTheta = rand()/(float)RAND_MAX*2*PIf;
            for (float theta=0; theta < 2*PIf; theta += 2.f*PIf/count) {
                float X = aX * cosf(theta+startTheta);
                float Y = aY * sinf(theta+startTheta);

                float RX = + X * cos_angle + Y * sin_angle;
                float RY = - X * sin_angle + Y * cos_angle;

                fvec newSample;
                newSample.resize(2,0);
                newSample[0] = sample[0] + RX;
                newSample[1] = sample[1] + RY;
                if (theta==0) {
                    oldPoint = point;
                    continue;
                }
                canvas->data->AddSample(newSample, label);

                oldPoint = point;
            }
        }
        break;
    case 5: // line
        {
            if (drawTime.elapsed() < 200/speed) return; // msec elapsed since last drawing
            float aX = drawToolbarContext2->spinSigmaX->value();
            float angle = -drawToolbarContext2->spinAngle->value()/180.f*PIf;
            int count = drawToolbarContext1->spinCount->value();
            float sin_angle = sinf(angle);
            float cos_angle = cosf(angle);

            QPointF pStart, pStop;
            float x = cos_angle*aX;
            float y = sin_angle*aX;
            pStart = QPointF(sample[0] - x, sample[1] - y);
            pStop = QPointF(sample[0] + x, sample[1] + y);
            QPointF oldPoint = pStart;
            float start = (rand() / (float)RAND_MAX - 0.5) * (1/(float)count);
            FOR (i,count) {
                QPointF point = (pStop - pStart)*((i+1)/(float)count + start) + pStart;
                fvec newSample;
                newSample.resize(2);
                newSample[0] = point.x();
                newSample[1] = point.y();
                canvas->data->AddSample(newSample, label);
                oldPoint = point;
            }
        }
        break;
    case 6: // trajectory
        {
            if (trajectory.first == -1) { // we're starting a trajectory
                trajectory.first = canvas->data->GetCount();
            }
            // we don't want to draw too often
            canvas->data->AddSample(sample, label, _TRAJ);
            trajectory.second = canvas->data->GetCount()-1;
        }
        break;
    case 7: // obstacle
        {
            bNewObstacle = true;
            obstacle = Obstacle();
            obstacle.angle = drawToolbarContext3->spinAngle->value() / 180.f * PIf;
            obstacle.power[0] = drawToolbarContext3->spinPowerX->value();
            obstacle.power[1] = drawToolbarContext3->spinPowerY->value();
            obstacle.center = sample;
            obstacle.axes[0] = drawToolbarContext3->spinSigmaX->value();
            obstacle.axes[1] = drawToolbarContext3->spinSigmaY->value();
            obstacle.repulsion[0] = drawToolbarContext3->spinRepulsionX->value();
            obstacle.repulsion[1] = drawToolbarContext3->spinRepulsionX->value();
        }
        break;
    case 8: // paint rewards
        {
            float radius = drawToolbarContext4->spinRadius->value();
            float alpha = drawToolbarContext4->spinAlpha->value();
            canvas->PaintReward(sample, radius, label ? alpha : -alpha);
        }
        break;
    }
    canvas->repaint();
    drawTime.restart();
    ResetPositiveClass();
    if (bEmpty) {
        compare->params->outputDimCombo->setCurrentIndex(canvas->data->GetDimCount()-1);
        algo->optionsRegress->outputDimCombo->setCurrentIndex(canvas->data->GetDimCount()-1);
    }
    UpdateInfo();
}

void MLDemos::Editing(int editType, fvec position, int label)
{
    if (!selectedData.size()) // we're beginning our editing
    {
        QPointF center = canvas->toCanvasCoords(position);
        float s = drawToolbar->radiusSpin->value();
        float radius = s*canvas->height()/2;
        switch(editType)
        {
        case 1: // drag a single point
            {
                selectedData = canvas->SelectSamples(center, -1);
                selectionStart = position;
            }
            break;
        case 2: // drag multiple points
            {
                selectedData = canvas->SelectSamples(center, radius, &selectionWeights);
                selectionStart = position;
            }
            break;
        case 3: // drag multiple points of the same class
            {
                ivec closestSample = canvas->SelectSamples(center,-1);
                if (closestSample.size()) {
                    int closest = closestSample[0];
                    int closestLabel = canvas->data->GetLabel(closest);
                    selectedData.clear();
                    selectionWeights.clear();
                    FOR (i, canvas->data->GetCount()) {
                        if (canvas->data->GetLabel(i) != closestLabel) continue;
                        selectedData.push_back(i);
                        selectionWeights.push_back(1);
                    }
                    selectionStart = position;
                }
            }
            break;
        case 4: // extrude points across the vertical
            {
                selectionStart = position;
                selectedData = ivec(canvas->data->GetCount());
                selectionWeights = fvec(canvas->data->GetCount());
                int yIndex = canvas->yIndex;
                selectionStart[yIndex] = canvas->center[yIndex];
                FOR (i, selectedData.size()) selectedData[i] = i;
                // we need to generate a fixed "spacing" so that we can 'scale' the samples
                FOR (i, selectedData.size()) selectionWeights[i] = canvas->data->GetSample(i)[yIndex] + drand48()*2.f - 1.f;
            }
            break;
        case 5: // change sample labels
            {
                int newLabel = drawToolbar->classSpin->value();
                selectedData = canvas->SelectSamples(center, radius);
                FOR (i, selectedData.size()) {
                    canvas->data->SetLabel(selectedData[i], newLabel);
                }
                selectedData.clear();
                canvas->sampleColors.clear();
            }
            break;
        }
    }

    switch(editType)
    {
    case 1:
    case 2:
    case 3:
        {
            FOR (i, selectedData.size()) {
                fvec sample = canvas->data->GetSample(selectedData[i]);
                float weight = 1.f;
                if (selectionWeights.size()) {
                    weight = max(cosf(max(0.f, selectionWeights[i]-0.75f)*(M_PI*0.75)),0.f);
                }
                sample += (position - selectionStart)*weight;
                canvas->data->SetSample(selectedData[i], sample);
                canvas->sampleColors.clear();
                canvas->maps.model = QPixmap();
            }
            selectionStart = position;
        }
        break;
    case 4:
        {
            int yIndex = canvas->yIndex;
            float diff = (position - selectionStart)[yIndex];
            FOR (i, selectedData.size()) {
                fvec sample = canvas->data->GetSample(selectedData[i]);
                sample[yIndex] = diff * selectionWeights[i];
                canvas->data->SetSample(selectedData[i], sample);
            }
        }
        break;
    }

    canvas->repaint();
    canvas->ResetSamples();
    ResetPositiveClass();
    drawTime.restart();
}

void MLDemos::DrawingStopped()
{
    if (selectedData.size() || (canvas->sampleColors.size() && canvas->sampleColors.size() != canvas->data->GetCount())) {
        if (algo->classifier) algo->DrawClassifiedSamples(canvas, algo->classifier, algo->classifierMulti);
        if (algo->clusterer) algo->clusterers[algo->tabUsedForTraining]->Draw(canvas, algo->clusterer);
        if (algo->regressor) algo->regressors[algo->tabUsedForTraining]->Draw(canvas, algo->regressor);
        if (algo->dynamical) algo->dynamicals[algo->tabUsedForTraining]->Draw(canvas, algo->dynamical);
    }
    selectedData.clear();
    selectionStart = fvec();

    if (trajectory.first != -1) {
        // the last point is a duplicate, we take it out
        canvas->data->AddSequence(trajectory);
        canvas->drawnTrajectories = 0;
        trajectory.first = -1;
        canvas->repaint();
    }
    if (bNewObstacle) {
        bNewObstacle = false;
        canvas->data->AddObstacle(obstacle);
        canvas->repaint();
        if (algo->dynamical && algo->dynamical->avoid) {
            drawTimer->Stop();
            drawTimer->Clear();
            drawTimer->start(QThread::NormalPriority);
        }
    }
}

void MLDemos::DrawCrosshair()
{
    int drawType = 0;
    if (drawToolbar->tabWidget->currentIndex() == 0) { // drawingTab
        if (drawToolbar->singleButton->isChecked()) drawType = 1;
        if (drawToolbar->sprayButton->isChecked()) drawType = 2;
        if (drawToolbar->spray3DButton->isChecked()) drawType = 9;
        if (drawToolbar->trajectoryButton->isChecked()) drawType = 6;
        if (drawToolbar->obstacleButton->isChecked()) drawType = 7;
        if (drawToolbar->paintButton->isChecked()) drawType = 8;
        if (drawToolbar->eraseButton->isChecked()) drawType = 3;
    } else if (drawToolbar->tabWidget->currentIndex() == 1) { // editTab
        if (drawToolbar->dragButton->isChecked()) drawType = -1;
        if (drawToolbar->moveButton->isChecked()) drawType = -2;
        if (drawToolbar->moveClassButton->isChecked()) drawType = -3;
        if (drawToolbar->extrudeButton->isChecked()) drawType = -4;
        if (drawToolbar->sprayClassButton->isChecked()) drawType = -5;
    } else if (drawToolbar->tabWidget->currentIndex() == 2) { // shapeTab
        if (drawToolbar->ellipseButton->isChecked()) drawType = 4;
        if (drawToolbar->lineButton->isChecked()) drawType = 5;
    }

    if (!drawType || drawType == 1 || drawType == 6 || drawType == -1 || drawType == -3) {
        canvas->crosshair = QPainterPath();
        canvas->bNewCrosshair = false;
        return;
    }
    int type = drawToolbarContext1->randCombo->currentIndex();
    float aX = drawToolbarContext2->spinSigmaX->value();
    float aY = drawToolbarContext2->spinSigmaY->value();
    float angle = -drawToolbarContext2->spinAngle->value()/180.f*PIf;
    float s = drawToolbar->radiusSpin->value();
    //float s = drawToolbarContext1->spinSize->value();
    int size = (int)(s*canvas->height());
    int Size = canvas->height();

    QPainterPath cursor;

    float sin_angle = sinf(angle);
    float cos_angle = cosf(angle);

    switch (drawType)
    {
    case 5: // line
        {
            QPointF pStart, pStop;
            float x = cos_angle*aX;
            float y = sin_angle*aX;
            pStart = QPointF(- x*Size, - y*Size);
            pStop = QPointF(+ x*Size, + y*Size);
            cursor.moveTo(pStart);
            cursor.lineTo(pStop);
            canvas->crosshair = cursor;
            canvas->bNewCrosshair = false;
            return;
        }
        break;
    case 2: // spray
    case 3: // erase
    case 9: // spray 3D
    case -2: // move points
    case -5:  // spray class
        {
            cursor.addEllipse(QPoint(0,0),size/2,size/2);
            canvas->crosshair = cursor;
            canvas->bNewCrosshair = false;
            return;
        }
        break;
    case 7: // obstacles
        {
            Obstacle o;
            o.angle = drawToolbarContext3->spinAngle->value() / 180.f * PIf;
            o.axes.resize(2);
            o.axes[0] = drawToolbarContext3->spinSigmaX->value();
            o.axes[1] = drawToolbarContext3->spinSigmaY->value();
            o.power[0] = drawToolbarContext3->spinPowerX->value();
            o.power[1] = drawToolbarContext3->spinPowerY->value();
            o.repulsion[0] = drawToolbarContext3->spinRepulsionX->value();
            o.repulsion[1] = drawToolbarContext3->spinRepulsionY->value();
            o.center = fVec(0,0);
            canvas->crosshair = canvas->DrawObstacle(o);
            canvas->bNewCrosshair = false;
            return;
        }
        break;
    case 8: // paint
        {
            float radius = drawToolbarContext4->spinRadius->value();
            QPainterPath cursor;
            cursor.addEllipse(QPoint(0,0),radius,radius);
            canvas->crosshair = cursor;
            canvas->bNewCrosshair = false;
            return;
        }
        break;
    }

    QPointF oldPoint, point;
    for (float theta=0; theta < 2*PIf + 0.1; theta += 0.1f) {
        float X, Y;
        if (drawType == 2 || drawType == 3) {
            X = sqrtf(aX)/2 * cosf(theta);
            Y = sqrtf(aY)/2 * sinf(theta);
        } else {
            X = aX * cosf(theta);
            Y = aY * sinf(theta);
        }

        float RX = + X * cos_angle + Y * sin_angle;
        float RY = - X * sin_angle + Y * cos_angle;

        point = QPointF(RX*Size,RY*Size);
        if (theta==0) {
            cursor.moveTo(point);
            continue;
        }
        cursor.lineTo(point);
        oldPoint = point;
    }
    canvas->crosshair = cursor;
    canvas->bNewCrosshair = false;
}

void MLDemos::CrosshairChanged()
{
    canvas->bNewCrosshair = true;
    DrawCrosshair();
}


void MLDemos::TargetButton()
{
    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;

    mimeData->setText("Target");
    drag->setMimeData(mimeData);
    QPixmap pixmap(33,33);
    pixmap.fill();
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(Qt::black, 1.5));
    painter.setBrush(Qt::NoBrush);

    int pad = 4, radius = 8;
    painter.drawEllipse(QPoint(16,16), radius, radius);
    painter.setBrush(Qt::black);
    painter.drawLine(QPoint(16,16) - QPoint(radius,radius), QPoint(16,16) - QPoint(radius+pad,radius+pad));
    painter.drawLine(QPoint(16,16) + QPoint(radius,radius), QPoint(16,16) + QPoint(radius+pad,radius+pad));
    painter.drawLine(QPoint(16,16) - QPoint(radius,-radius), QPoint(16,16) - QPoint(radius+pad,-radius-pad));
    painter.drawLine(QPoint(16,16) + QPoint(radius,-radius), QPoint(16,16) + QPoint(radius+pad,-radius-pad));
    drag->setPixmap(pixmap);
    drag->setHotSpot(QPoint(pixmap.width()/2, pixmap.height()/2));

    // maximization only allows one target, so we take the others out
    if (algo->options->tabMax->isVisible()) {
        canvas->targets.clear();
        canvas->targetAge.clear();
        canvas->repaint();
    }
    Qt::DropAction dropAction = drag->exec();
}

void MLDemos::ClearTargets()
{
    if (!canvas->targets.size()) return;
    canvas->targets.clear();
    canvas->targetAge.clear();
    canvas->maps.animation = QPixmap();
    canvas->repaint();
}

void MLDemos::GaussianButton()
{
    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;

    mimeData->setText("Gaussian");
    mimeData->setColorData(QVariant(algo->optionsMaximize->varianceSpin->value()));
    drag->setMimeData(mimeData);
    QPixmap pixmap(33,33);
    pixmap.fill();
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(Qt::black, 1.5));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(QPoint(16,16), 12,12);
    painter.setBrush(Qt::black);
    painter.drawEllipse(QPoint(16,16), 1,1);
    drag->setPixmap(pixmap);
    drag->setHotSpot(QPoint(pixmap.width()/2, pixmap.height()/2));
    Qt::DropAction dropAction = drag->exec();
}

void MLDemos::GradientButton()
{
    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;

    mimeData->setText("Gradient");
    drag->setMimeData(mimeData);
    QPixmap pixmap(33,33);
    pixmap.fill();
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(Qt::black, 1.5));
    painter.setBrush(Qt::NoBrush);
    painter.drawLine(QPoint(4,16), QPoint(29,4));
    painter.drawLine(QPoint(4,16), QPoint(29,29));
    painter.drawLine(QPoint(29,4), QPoint(29,29));
    drag->setPixmap(pixmap);
    drag->setHotSpot(QPoint(pixmap.width()/2, pixmap.height()/2));
    Qt::DropAction dropAction = drag->exec();
}
