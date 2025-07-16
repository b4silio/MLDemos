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

MLDemos::MLDemos(QString filename, QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags),
      canvas(0),glw(0),vis(0),
      gridSearch(0),
      bIsRocNew(true),
      bIsCrossNew(true),
      compare(0),
      trajectory(ipair(-1,-1)),
      bNewObstacle(false)
{
    QApplication::setWindowIcon(QIcon(":/MLDemos/logo.png"));
    ui.setupUi(this);
    setAcceptDrops(true);

#ifdef QT_OPENGL_ES_2
    qDebug() << "OpenGL ES 2";
#else
    qDebug() << "OpenGL 2.1";
#endif

    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    connect(ui.actionExit, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(ui.actionAbout, SIGNAL(triggered()), this, SLOT(ShowAbout()));
    connect(ui.actionClearData, SIGNAL(triggered()), this, SLOT(ClearData()));
    connect(ui.actionClearModel, SIGNAL(triggered()), this, SLOT(Clear()));
    connect(ui.actionEditDataset, SIGNAL(triggered()), this, SLOT(ShowDataEditor()));
    connect(ui.actionNew, SIGNAL(triggered()), this, SLOT(ClearData()));
    connect(ui.actionSave, SIGNAL(triggered()), this, SLOT(SaveData()));
    connect(ui.actionLoad, SIGNAL(triggered()), this, SLOT(LoadData()));
    connect(ui.actionImportData, SIGNAL(triggered()), this, SLOT(ImportData()));
    connect(ui.actionExportOutput, SIGNAL(triggered()), this, SLOT(ExportOutput()));
    connect(ui.actionAML_Class, SIGNAL(triggered()), this, SLOT(ReloadPlugins()));
    //connect(ui.actionExportAnimation, SIGNAL(triggered()), this, SLOT(ExportAnimation()));
    connect(ui.actionExport_SVG, SIGNAL(triggered()), this, SLOT(ExportSVG()));
    ui.actionImportData->setShortcut(QKeySequence(tr("Ctrl+I")));

    initDialogs();
    initToolBars();

    LoadLayoutOptions();
    SetTextFontSize();

    plugin->LoadPlugins();
    LoadAlgorithmsOptions();

    this->show();

    DisplayOptionsChanged();
    UpdateInfo();
    FitToData();
    AlgoChanged();

    canvas->repaint();

    canvas->ResizeEvent();
    CanvasMoveEvent();
    CanvasTypeChanged();
    CanvasOptionsChanged();
    Display3DOptionsChanged();
    ResetPositiveClass();
    algo->ClusterChanged();
    DrawToolsChanged();
    ChangeInfoFile();
    drawTime.start();
    if (filename != "") Load(filename);
}

MLDemos::~MLDemos()
{
    Clear();
    SaveLayoutOptions();
    DEL(algo);
    DEL(glw);
    DEL(vis);
    DEL(manualSelection);
    DEL(inputDimensions);
    DEL(showStats);
    DEL(viewOptions);
    DEL(drawToolbar);
    //DEL(drawToolbarWidget);
    DEL(drawToolbarContext1);
    DEL(drawToolbarContext2);
    DEL(drawToolbarContext3);
    DEL(drawToolbarContext4);
    DEL(drawContext1Widget);
    DEL(drawContext2Widget);
    DEL(drawContext3Widget);
    DEL(drawContext4Widget);
    DEL(displayDialog);
    DEL(aboutPanel);
    DEL(aboutDialog);
    DEL(statsDialog);
    DEL(manualSelectDialog);
    DEL(inputDimensionsDialog);
    DEL(drawTimer);
    DEL(gridSearch);
    DEL(import);
    DEL(generator);
    DEL(dataEdit);
    DEL(compare);
    DEL(canvas);
}

void MLDemos::closeEvent(QCloseEvent *event)
{
    qApp->quit();
}

void MLDemos::resizeEvent(QResizeEvent *event)
{
    if (!canvas) return;
    if (canvas->canvasType == 2) {
        QSizePolicy policy = vis->sizePolicy();
        policy.setHorizontalPolicy(QSizePolicy::Preferred);
        policy.setVerticalPolicy(QSizePolicy::Preferred);
        vis->setSizePolicy(policy);
        vis->setMinimumSize(ui.canvasWidget->size());
        vis->resize(ui.canvasWidget->size());
        //CanvasOptionsChanged();
    } else if (canvas->canvasType == 1) { // 3D View
        QSizePolicy policy = glw->sizePolicy();
        policy.setHorizontalPolicy(QSizePolicy::Preferred);
        policy.setVerticalPolicy(QSizePolicy::Preferred);
        glw->setSizePolicy(policy);
        glw->setMinimumSize(ui.canvasWidget->size());
        glw->resize(ui.canvasWidget->size());
    }
    canvas->ResizeEvent();
    CanvasOptionsChanged();
    CanvasMoveEvent();
}
