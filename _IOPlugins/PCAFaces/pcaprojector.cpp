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
#include "public.h"
#include "pcaprojector.h"
#include "basicOpenCV.h"
#include <QString>
#include <QMouseEvent>
#include <QDebug>
#include <QFileDialog>
#include <QMimeData>
#include <QUrl>
#include <QClipboard>
#include <QMessageBox>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

using namespace std;
using cv::Mat;

PCAProjector::PCAProjector( Ui::PCAFacesDialog *options )
    : options(options), samples(0), start(QPoint(-1,-1)), grabber(0), bFromWebcam(true), timerID(0)
{
    eigenVectorLabel = NULL;
    eigenValueLabel = NULL;
    imageWindow = new QNamedWindow("image", false, options->imageWidget);
    samplesWindow = new QNamedWindow("samples", false, options->dataWidget);
    selection = QRect(0,0,256,256);
    image = cv::Mat(256,256,CV_8UC3);
    display = cv::Mat(256,256,CV_8UC3);
    samples = cvCreateImage(cvSize(380,340),8,3);
    cvSet(samples, CV_RGB(255,255,255));
    connect(imageWindow, SIGNAL(MousePressEvent(QMouseEvent *)), this, SLOT(SelectionStart(QMouseEvent *)));
    connect(imageWindow, SIGNAL(MouseMoveEvent(QMouseEvent *)), this, SLOT(SelectionResize(QMouseEvent *)));
    connect(imageWindow, SIGNAL(MouseReleaseEvent(QMouseEvent *)), this, SLOT(SelectionStop(QMouseEvent *)));
    connect(samplesWindow, SIGNAL(MouseReleaseEvent(QMouseEvent *)), this, SLOT(DatasetClick(QMouseEvent *)));
    connect(imageWindow, SIGNAL(DragEnterEvent(QDragEnterEvent *)), this, SLOT(DragImage(QDragEnterEvent *)));
    connect(imageWindow, SIGNAL(DropEvent(QDropEvent *)), this, SLOT(DropImage(QDropEvent *)));
    connect(samplesWindow, SIGNAL(DragEnterEvent(QDragEnterEvent *)), this, SLOT(DragDataset(QDragEnterEvent *)));
    connect(samplesWindow, SIGNAL(DropEvent(QDropEvent *)), this, SLOT(DropDataset(QDropEvent *)));

    connect(options->webcamButton, SIGNAL(clicked()), this, SLOT(FromWebcam()));
    connect(options->loadImgButton, SIGNAL(clicked()), this, SLOT(LoadImage()));
    connect(options->clipboardButton, SIGNAL(clicked()), this, SLOT(FromClipboard()));
    connect(options->addButton, SIGNAL(clicked()), this, SLOT(AddImage()));
    connect(options->loadDatasetButton, SIGNAL(clicked()), this, SLOT(LoadDataset()));
    connect(options->saveDatasetButton, SIGNAL(clicked()), this, SLOT(SaveDataset()));
    connect(options->clearButton, SIGNAL(clicked()), this, SLOT(ClearDataset()));
    connect(options->eigenButton, SIGNAL(clicked()), this, SLOT(DrawEigen()));

    imageWindow->setAcceptDrops(true);
    samplesWindow->setAcceptDrops(true);
    samplesWindow->repaint();

    image.setTo(cv::Scalar(255,255,255));
    imageMutex.lock();
    SetImage(image);
    imageMutex.unlock();
    grabber.open(0);
    timerID = startTimer(1000/30); // 30fps
}

PCAProjector::~PCAProjector()
{
    if(timerID != -1) killTimer(timerID);
    timerID = -1;
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    if(grabber.isOpened()) grabber.release();
    IMKILL(samples);
    DEL(imageWindow);
    DEL(samplesWindow);
    DEL(eigenVectorLabel);
    DEL(eigenValueLabel);
}

void PCAProjector::timerEvent(QTimerEvent *event)
{
    if(!bFromWebcam) return;
    imageMutex.lock();
    Mat frame;
    grabber >> frame;
    SetImage(frame);
    imageMutex.unlock();
}


void PCAProjector::DrawEigen()
{
    if(sm.GetCount() >= 3){
        EigenFaces eig;
        eig.Learn(sm.GetSampleMats(), sm.GetLabels());
        SampleManager eigVecs;
        eigVecs.AddSamples(eig.GetEigenVectorsImages());
        IplImage *image = eigVecs.GetSampleImage();
        if(!eigenVectorLabel) eigenVectorLabel = new QLabel();
        eigenVectorLabel->setScaledContents(true);
        eigenVectorLabel->setPixmap(QNamedWindow::toPixmap(image));
        eigenVectorLabel->show();

        IplImage *eigValsImg = eig.DrawEigenVals();
        if(!eigenValueLabel) eigenValueLabel = new QLabel();
        eigenValueLabel->setScaledContents(true);
        eigenValueLabel->setPixmap(QNamedWindow::toPixmap(eigValsImg));
        eigenValueLabel->show();

        //cvNamedWindow("Eigen Vectors");
        //cvShowImage("Eigen Vectors", image);
        eigVecs.Clear();
        IMKILL(image);
        IMKILL(eigValsImg);
    }else{
        QMessageBox msgBox;
        msgBox.setText("Load/Import data first! (at least 3 samples)");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
    }
}

pair<vector<fvec>,ivec> PCAProjector::GetData()
{
    pair<vector<fvec>,ivec> data;
    if(sm.GetCount() < 2) return data;

    int e1 = options->spinE1->value()-1;
    int e2 = options->spinE2->value()-1;
    int count = options->eigenCountSpin->value();

    vector<cv::Mat> allSamples = sm.GetSampleMats();
    vector<cv::Mat> sourceSamples;
    ivec sourceLabels;
    FOR(i, sm.GetCount())
    {
        if(sm.GetLabel(i) == 0) continue;
        sourceSamples.push_back(allSamples[i]);
        sourceLabels.push_back(sm.GetLabel(i));
    }
    if(sourceSamples.size() < 4) return data;
    count = min(count, (int)sourceSamples.size()-2);
    e1 = min(count, e1);
    e2 = min(count, e2);

    // we want at least one class to be 0, to avoid problems afterwards
    //FixLabels(sm);
    // we do the data projection here
    EigenFaces eig;
    eig.Learn(sourceSamples, sourceLabels);
    int dim = max(count, max(e1, e2)+1);
    vector<float *> projections = eig.GetProjections(dim, true);
    if(!projections.size()) return data;
    // the projections are normalized on a space 0-1, we want to add a bit of edges
    vector<fvec> samples;
    samples.resize(projections.size());
    fvec sample(count);
    FOR(i, projections.size())
    {
        if(count == 2)
        {
            sample[0] = projections[i][e1]*0.9f + 0.05f;
            sample[1] = projections[i][e2]*0.9f + 0.05f;
        }
        else
            FOR(d, count) sample[d] = projections[i][d]*0.9f + 0.05f;
        samples[i] = sample;
        delete [] projections[i];
    }
    projections.clear();

    data.first = samples;
    data.second = sourceLabels;
    //data.first = samples;
    //data.second = sm.GetLabels();
    return data;
}

void PCAProjector::FromWebcam()
{
    bFromWebcam = true;
    if(!grabber.isOpened()) grabber.open(0);
}

void PCAProjector::SetImage( cv::Mat img )
{
    if(img.empty())
    {
        // we want a blank image
        image = cv::Mat(240, 320, CV_8UC3);
        image.setTo(cv::Scalar(255,255,255));
        imageWindow->ShowImage(image);
        imageWindow->repaint();
        bFromWebcam = false;
        if(grabber.isOpened()) grabber.release();
        return;
    }
    //float ratio = img->width / (float)img->height;
    if(img.cols != image.cols || img.rows != image.rows || img.channels() != image.channels())
    {
        int size = max(img.cols, img.rows);
        image = cv::Mat(size,size, CV_8UC3);
        image.setTo(cv::Scalar(255,255,255));
        cv::Rect rect((size-img.cols)/2,(size-img.rows)/2, img.cols, img.rows);
        Mat selection = image(rect);
        cv::resize(img, selection,cv::Size(rect.width, rect.height),0,0,cv::INTER_CUBIC);
    }
    cv::resize(image, display, cv::Size(display.cols, display.rows), 0, 0, cv::INTER_CUBIC);
    cv::rectangle(display, cv::Rect(selection.x(),selection.y(),selection.width(), selection.height()), cv::Scalar(0,0,0),3);
    cv::rectangle(display, cv::Rect(selection.x(),selection.y(),selection.width(), selection.height()), cv::Scalar(255,255,255),1);
    IplImage displayImg(display);
    imageWindow->ShowImage(&displayImg);
    imageWindow->repaint();
}

void PCAProjector::RefreshDataset()
{
    IplImage *dataset = sm.GetSampleImage();
    if(!dataset)
    {
        options->samplesLabel->setText(QString("Samples: %1").arg(sm.GetCount()));
        cvSet(samples, CV_RGB(255,255,255));
        samplesWindow->ShowImage(samples);
        samplesWindow->repaint();
        return;
    }
    float ratio = dataset->width / (float)dataset->height;
    cvSet(samples, CV_RGB(255,255,255));
    CvRect rect = cvRect(0,0,samples->width,samples->height/ratio);
    if(ratio != 1) ROI(samples, rect);
    cvResize(dataset, samples, CV_INTER_CUBIC);
    unROI(samples);
    samplesWindow->ShowImage(samples);
    int posCount=0, negCount=0;
    FOR(i, sm.GetCount())
    {
        if(sm.GetLabel(i) == 0) posCount++;
        else negCount++;
    }
    options->samplesLabel->setText(QString("Samples: %1").arg(sm.GetCount()));
    options->eigenCountSpin->setRange(2, max(2,sm.GetCount()-1));
    samplesWindow->repaint();
    emit(Update());
}

void PCAProjector::SelectionStart(QMouseEvent *event)
{
    start = event->pos();
}

void PCAProjector::SelectionStop(QMouseEvent *event)
{
    if(event->pos() == start)
    {
        selection = QRect(0,0,256,256);
        cv::resize(image, display, cv::Size(display.cols, display.rows), 0, 0, cv::INTER_CUBIC);
        cv::rectangle(display, cv::Rect(selection.x(),selection.y(),selection.width(), selection.height()), cv::Scalar(0,0,0),3);
        cv::rectangle(display, cv::Rect(selection.x(),selection.y(),selection.width(), selection.height()), cv::Scalar(255,255,255),1);
        imageWindow->ShowImage(display);
        imageWindow->repaint();
    }
    start = QPoint(-1,-1);
}

void PCAProjector::SelectionResize(QMouseEvent *event)
{
    if(start.x() == -1) return;
    int size = max(
                   min(max(0,event->pos().x()),255)-start.x(),
                   min(max(0,event->pos().y()),255)-start.y());
    if(size + start.x() > 255) size = 255 - start.x();
    if(size + start.y() > 255) size = 255 - start.y();
    selection = QRect(start.x(), start.y(), size, size);

    cv::resize(image, display, cv::Size(display.cols, display.rows), 0, 0, cv::INTER_CUBIC);
    cv::rectangle(display, cv::Rect(selection.x(),selection.y(),selection.width(), selection.height()), cv::Scalar(0,0,0),3);
    cv::rectangle(display, cv::Rect(selection.x(),selection.y(),selection.width(), selection.height()), cv::Scalar(255,255,255),1);
    imageWindow->ShowImage(display);
    imageWindow->repaint();
}

void PCAProjector::DatasetClick(QMouseEvent *event)
{
    if(!sm.GetCount()) return;
    int x = event->pos().x();
    int y = event->pos().y();

    // we need to know the conversion ratio between display and real samples
    IplImage *sampleImage = sm.GetSampleImage();
    if(!sampleImage) return;
    CvSize realSize = cvGetSize(sampleImage);
    CvSize size = cvGetSize(samples);
    float ratio = realSize.width/(float)realSize.height;
    x = x*realSize.width/size.width;
    y = (int)(y*realSize.height*ratio/size.height);

    int index = sm.GetIndexAt(x,y);
    if(event->button() == Qt::RightButton)
    {
        int label = max((int)sm.GetLabel(index)-1, 0);
        if(event->modifiers() == Qt::ShiftModifier)
        {
            for(int i=index; i<sm.GetCount(); i++) sm.SetLabel(i, label);
        }
        else sm.SetLabel(index, label);
    }
    else if (event->button() == Qt::LeftButton)
    {
        int label = min((int)sm.GetLabel(index)+1, 255);
        if(event->modifiers() == Qt::ShiftModifier)
        {
            for(int i=index; i<sm.GetCount(); i++) sm.SetLabel(i, label);
        }
        else sm.SetLabel(index, label);
    }
    if(event->modifiers() == Qt::ControlModifier && event->button() == Qt::LeftButton)
    {
        sm.RemoveSample(index);
    }
    RefreshDataset();
}

void PCAProjector::DragImage(QDragEnterEvent *event)
{
    if(!event->mimeData()->hasUrls()) return;
    QList<QUrl> urls = event->mimeData()->urls();
    for(int i=0; i<urls.size(); i++)
    {
        QString filename = urls[i].path();
        if(filename.toLower().endsWith(".png") || filename.toLower().endsWith(".jpg"))
        {
            event->acceptProposedAction();
            return;
        }
    }
}

void PCAProjector::DragDataset(QDragEnterEvent *event)
{
    if(!event->mimeData()->hasUrls()) return;
    QList<QUrl> urls = event->mimeData()->urls();
    for(int i=0; i<urls.size(); i++)
    {
        QString filename = urls[i].path();
        if(filename.toLower().endsWith(".png"))
        {
            event->acceptProposedAction();
            return;
        }
    }
}

void PCAProjector::DropImage(QDropEvent *event)
{
    if(!event->mimeData()->hasUrls()) return;
    FOR(i, event->mimeData()->urls().length())
    {
        QString filename = event->mimeData()->urls()[i].toLocalFile();
        if(filename.toLower().endsWith(".png") || filename.toLower().endsWith(".jpg"))
        {
            QFile file(filename);
            if (!file.open(QIODevice::ReadOnly)) continue;
            file.close();
            cv::Mat img = cv::imread(filename.toStdString().c_str());
            imageMutex.lock();
            SetImage(img);
            imageMutex.unlock();
            bFromWebcam = false;
            if(grabber.isOpened()) grabber.release();
            break;
        }
    }
    event->acceptProposedAction();
}

void PCAProjector::DropDataset(QDropEvent *event)
{
    if(!event->mimeData()->hasUrls()) return;
    FOR(i, event->mimeData()->urls().length())
    {
        QString filename = event->mimeData()->urls()[i].toLocalFile();
        if(filename.toLower().endsWith(".png"))
        {
            QFile file(filename);
            if (!file.open(QIODevice::ReadOnly)) return;
            file.close();
            sm.Load(filename.toLatin1());
            RefreshDataset();
        }
    }
    event->acceptProposedAction();
}

void PCAProjector::LoadImage()
{
    QString filename = QFileDialog::getOpenFileName(imageWindow, tr("Load Image"), "", tr("Images (*.png *.jpg)"));
    if(filename.isEmpty()) return;
    if(!filename.endsWith(".png") && !filename.endsWith(".jpg")) filename += ".png";
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) return;
    file.close();

    bFromWebcam = false;
    if(grabber.isOpened()) grabber.release();
    Mat img = cv::imread(filename.toStdString().c_str());
    if(img.empty()) return;
    imageMutex.lock();
    SetImage(img);
    imageMutex.unlock();
    bFromWebcam = false;
}

void PCAProjector::FromClipboard()
{
    QClipboard *clipboard = QApplication::clipboard();
    if(clipboard != NULL){

        const QMimeData *mimeData = clipboard->mimeData();

        if (mimeData->hasImage()) {

            QImage image = clipboard->image();
            IplImage *img = QNamedWindow::cvxCopyQImage(image);
            Mat imgMat = cv::cvarrToMat(img);

            if(!imgMat.empty()){
                imageMutex.lock();
                SetImage(imgMat);
                imageMutex.unlock();
                bFromWebcam = false;
                if(grabber.isOpened()) grabber.release();
            }
        } else if (mimeData->hasUrls()) {
            FOR(i, clipboard->mimeData()->urls().length())
            {
                QString filename = clipboard->mimeData()->urls()[i].toLocalFile();
                if(filename.toLower().endsWith(".png") || filename.toLower().endsWith(".jpg"))
                {
                    Mat img = cv::imread(filename.toStdString().c_str());
                    if(img.empty()) break;
                    imageMutex.lock();
                    SetImage(img);
                    imageMutex.unlock();
                    bFromWebcam = false;
                    if(grabber.isOpened()) grabber.release();
                    break;
                }
            }
        }else {
            std::cout<< "invalid data type" << std::endl;
        }
    }else{
        std::cout<< "clipboard is NULL" << std::endl;
    }
}

void PCAProjector::AddImage()
{
    CvRect rect = cvRect(selection.x(), selection.y(), selection.width(), selection.height());
    float ratio = image.cols/(float)display.cols;
    rect.x *= ratio;
    rect.y *= ratio;
    rect.width *= ratio;
    rect.height *= ratio;
    if(!rect.width || !rect.height) return;
    if(rect.width < 0)
    {
        rect.width = abs(rect.width);
        rect.x -= rect.width;
    }
    if(rect.height < 0)
    {
        rect.height = abs(rect.height);
        rect.y -= rect.height;
    }
    if(rect.x < 0) rect.x = 0;
    if(rect.y < 0) rect.y = 0;
    if(rect.x+rect.width > image.cols) rect.width = image.cols - rect.x;
    if(rect.y+rect.height > image.rows) rect.height= image.rows - rect.y;
    IplImage imageIpl(image);
    sm.AddSample(&imageIpl, rect);
    RefreshDataset();
}

void PCAProjector::LoadDataset()
{
    QString filename = QFileDialog::getOpenFileName(imageWindow, tr("Load Dataset"), "", tr("Dataset Images (*.png)"));
    if(filename.isEmpty()) return;
    if(!filename.endsWith(".png")) filename += ".png";
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) return;
    file.close();
    sm.Load(filename.toLatin1().data());
    RefreshDataset();
}

void PCAProjector::SaveDataset()
{
    if(!sm.GetCount()) return;
    QString filename = QFileDialog::getSaveFileName(samplesWindow, tr("Save Dataset"), "", tr("Dataset Images (*.png)"));
    if(filename.isEmpty()) return;
    if(!filename.endsWith(".png")) filename += ".png";
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) return;
    file.close();
    sm.Save(filename.toLatin1());
}

void PCAProjector::AddDataset()
{
    QString filename = QFileDialog::getOpenFileName(imageWindow, tr("Add Dataset"), "", tr("Dataset Images (*.png)"));
    if(filename.isEmpty()) return;
    if(!filename.endsWith(".png")) filename += ".png";
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) return;
    file.close();
    SampleManager newSm;
    newSm.Load(filename.toLatin1());
    sm.AddSamples(newSm);
    RefreshDataset();
}

void PCAProjector::ClearDataset()
{
    sm.Clear();
    RefreshDataset();
}

void PCAProjector::FixLabels(SampleManager &sm)
{
    if(!sm.GetCount())
    {
        return;
    }
    vector< pair<int,int> > couples;
    FOR(i, sm.GetCount())
    {
        int label = sm.GetLabel(i);
        bool bExists = false;
        FOR(j, couples.size())
        {
            if(label == couples[j].first)
            {
                bExists = true;
                break;
            }
        }
        if(bExists) continue;
        couples.push_back(pair<int,int>(label,couples.size()));
    }
    FOR(i, couples.size())
    {
        if(couples[i].first == couples[i].second) continue; // nothing to change here!
        FOR(j, sm.GetCount())
        {
            if(sm.GetLabel(j) == couples[i].first) sm.SetLabel(j,(u8)couples[i].second);
        }
    }
}
