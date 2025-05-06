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
#ifndef _PCAPROJECTOR_H_
#define _PCAPROJECTOR_H_

#include "public.h"
#include "ui_PCAFaces.h"
#include "widget.h"
#include "sampleManager.h"
#include "eigenFaces.h"
#include <QMutex>
#include <QMutexLocker>
#include <opencv2/videoio.hpp>

namespace Ui
{
    class EigenVectorWidget;
}

class PCAProjector : public QObject
{
    Q_OBJECT

    Ui::PCAFacesDialog *    options;
    EigenFaces              eig;
    cv::Mat                 image, display;
    cv::Mat                 samples;
    QNamedWindow *          imageWindow;
    QNamedWindow *          samplesWindow;
    SampleManager           sm;
    QPoint                  start;
    QRect                   selection;
    bool                    bFromWebcam;
    cv::VideoCapture        grabber;
    QMutex                  imageMutex;
    int                     timerID;
    Ui::EigenVectorWidget*  eigenDisplay;
    QWidget*                eigenDisplayWidget;
    QPixmap                 eigenVecPixmap, eigenValPixmap;

    void mouseCallBack(int x,int y,int flags,int params);

    void SetImage(cv::Mat image);
    void RefreshDataset();
    void FixLabels(SampleManager &sm);
public:
    PCAProjector(Ui::PCAFacesDialog *options);
    ~PCAProjector();
    void timerEvent(QTimerEvent *event);
    std::pair<std::vector<fvec>,ivec> GetData();

protected:
    bool eventFilter(QObject* obj, QEvent* evt);

signals:
    void Update();
public slots:
    void LoadImage();
    void FromClipboard();
    void FromWebcam();
    void AddImage();
    void LoadDataset();
    void SaveDataset();
    void AddDataset();
    void ClearDataset();
    void DrawEigen();
    void ToClipboard();

    void SelectionStart(QMouseEvent *event);
    void SelectionStop(QMouseEvent *event);
    void SelectionResize(QMouseEvent *event);

    void DatasetClick(QMouseEvent *event);
    void DragImage(QDragEnterEvent *event);
    void DropImage(QDropEvent *event);
    void DragDataset(QDragEnterEvent *event);
    void DropDataset(QDropEvent *event);
    //private slots:
    // void on_eigenButton_clicked();
};

#endif // _PCAPROJECTOR_H_
