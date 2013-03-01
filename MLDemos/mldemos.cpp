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
#include <QDebug>
#include <QtGui>
#include <fstream>
#include <QPixmap>
#include <QUrl>
#include <QBitmap>
#include <QSettings>
#include <QFileDialog>
#include <QLayout>
#include "basicMath.h"
#include "drawSVG.h"
#include <iostream>
#include <sstream>
#include <matio/matio.h>
#include "optimization_test_functions.h"

MLDemos::MLDemos(QString filename, QWidget *parent, Qt::WFlags flags)
    : QMainWindow(parent, flags),
      canvas(0),glw(0),vis(0),
      gridSearch(0),
      classifier(0),
      regressor(0),
      dynamical(0),
      clusterer(0),
      maximizer(0),
      reinforcement(0),
      projector(0),
      bIsRocNew(true),
      bIsCrossNew(true),
      compare(0),
      trajectory(ipair(-1,-1)),
      bNewObstacle(false),
      tabUsedForTraining(0)
{
    QApplication::setWindowIcon(QIcon(":/MLDemos/logo.png"));
    ui.setupUi(this);
    setAcceptDrops(true);

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
    connect(ui.actionExportAnimation, SIGNAL(triggered()), this, SLOT(ExportAnimation()));
    connect(ui.actionExport_SVG, SIGNAL(triggered()), this, SLOT(ExportSVG()));
    ui.actionImportData->setShortcut(QKeySequence(tr("Ctrl+I")));

    initDialogs();

    initToolBars();

    initPlugins();

    LoadLayoutOptions();
    SetTextFontSize();
    ShowToolbar();
    this->show();

    DisplayOptionsChanged();
    UpdateInfo();
    FitToData();
    AlgoChanged();
    if(!classifiers.size()) algorithmOptions->tabWidget->setTabEnabled(0,false);
    if(!clusterers.size()) algorithmOptions->tabWidget->setTabEnabled(1,false);
    if(!regressors.size()) algorithmOptions->tabWidget->setTabEnabled(2,false);
    if(!projectors.size()) algorithmOptions->tabWidget->setTabEnabled(3,false);
    if(!dynamicals.size()) algorithmOptions->tabWidget->setTabEnabled(4,false);
    if(!maximizers.size()) algorithmOptions->tabWidget->setTabEnabled(5,false);

    algorithmWidget->setFixedSize(636,220);
    canvas->repaint();

    canvas->ResizeEvent();
    CanvasMoveEvent();
    CanvasTypeChanged();
    CanvasOptionsChanged();
    Display3DOptionsChanged();
    ResetPositiveClass();
    ClusterChanged();
    ChangeInfoFile();
    drawTime.start();
    if(filename != "") Load(filename);
}

void MLDemos::initToolBars()
{
    actionNew = new QAction(QIcon(":/MLDemos/icons/new.png"), tr("&New"), this);
    actionNew->setShortcut(QKeySequence(tr("Ctrl+N")));
    actionNew->setStatusTip(tr("Clear Everything"));

    actionSave = new QAction(QIcon(":/MLDemos/icons/save.png"), tr("&Save"), this);
    actionSave->setShortcut(QKeySequence(tr("Ctrl+S")));
    actionSave->setStatusTip(tr("Save Data"));

    actionLoad = new QAction(QIcon(":/MLDemos/icons/load.png"), tr("&Load"), this);
    actionLoad->setShortcut(QKeySequence(tr("Ctrl+L")));
    actionLoad->setStatusTip(tr("Load Data"));

    actionAlgorithms = new QAction(QIcon(":/MLDemos/icons/algorithms.png"), tr("&Algorithms"), this);
    actionAlgorithms->setShortcut(QKeySequence(tr("C")));
    actionAlgorithms->setStatusTip(tr("Algorithm Options"));
    actionAlgorithms->setCheckable(true);

    actionCompare = new QAction(QIcon(":/MLDemos/icons/compare.png"), tr("&Compare"), this);
    actionCompare->setShortcut(QKeySequence(tr("M")));
    actionCompare->setStatusTip(tr("Compare Algorithms"));
    actionCompare->setCheckable(true);

    actionGridsearch = new QAction(QIcon(":/MLDemos/icons/gridsearch.png"), tr("&Gridsearch"), this);
    actionGridsearch->setShortcut(QKeySequence(tr("G")));
    actionGridsearch->setStatusTip(tr("Grid Search Parameters"));
    actionGridsearch->setCheckable(true);

    actionDrawSamples = new QAction(QIcon(":/MLDemos/icons/draw.png"), tr("&Drawing"), this);
    actionDrawSamples->setShortcut(QKeySequence(tr("W")));
    actionDrawSamples->setStatusTip(tr("Show Sample Drawing Options"));
    actionDrawSamples->setCheckable(true);

    actionAddData = new QAction(QIcon(":/MLDemos/icons/adddata.png"), tr("Add Data"), this);
    actionAddData->setShortcut(QKeySequence(tr("A")));
    actionAddData->setStatusTip(tr("Add new data"));
    actionAddData->setCheckable(true);

    actionClearModel = new QAction(QIcon(":/MLDemos/icons/clearmodel.png"), tr("Clear Model"), this);
    actionClearModel->setShortcut(QKeySequence(tr("Shift+X")));
    actionClearModel->setStatusTip(tr("Clear current model"));

    actionClearData = new QAction(QIcon(":/MLDemos/icons/cleardata.png"), tr("Clear Data"), this);
    actionClearData->setShortcut(QKeySequence(tr("X")));
    actionClearData->setStatusTip(tr("Clear data (Keep models)"));

    actionClearAll = new QAction(QIcon(":/MLDemos/icons/clearall.png"), tr("Clear All"), this);
    actionClearAll->setShortcut(QKeySequence(tr("X")));
    actionClearAll->setStatusTip(tr("Clear data and model"));

    actionScreenshot = new QAction(QIcon(":/MLDemos/icons/screenshot.png"), tr("Save Screenshot"), this);
    actionScreenshot->setShortcut(QKeySequence(tr("Alt+S")));
    actionScreenshot->setStatusTip(tr("Save the current image to disk"));

    actionDisplayOptions = new QAction(QIcon(":/MLDemos/icons/display.png"), tr("Display &Options"), this);
    actionDisplayOptions->setShortcut(QKeySequence(tr("O")));
    actionDisplayOptions->setStatusTip(tr("Show Display Options"));
    actionDisplayOptions->setCheckable(true);

    actionShowStats = new QAction(QIcon(":/MLDemos/icons/stats.png"), tr("Info/Statistics"), this);
    actionShowStats->setShortcut(QKeySequence(tr("I")));
    actionShowStats->setStatusTip(tr("Display Algorithm Information and Data Statistics"));
    actionShowStats->setCheckable(true);

    connect(actionAlgorithms, SIGNAL(triggered()), this, SLOT(ShowAlgorithmOptions()));
    connect(actionCompare, SIGNAL(triggered()), this, SLOT(ShowOptionCompare()));
    connect(actionDrawSamples, SIGNAL(triggered()), this, SLOT(ShowSampleDrawing()));
    connect(actionDisplayOptions, SIGNAL(triggered()), this, SLOT(ShowOptionDisplay()));
    connect(actionAddData, SIGNAL(triggered()), this, SLOT(ShowAddData()));
    connect(actionGridsearch, SIGNAL(triggered()), this, SLOT(ShowGridSearch()));
    connect(generator, SIGNAL(finished(int)), this, SLOT(HideAddData()));
    connect(actionClearData, SIGNAL(triggered()), this, SLOT(ClearData()));
    connect(actionClearAll, SIGNAL(triggered()), this, SLOT(ClearAll()));
    connect(actionClearModel, SIGNAL(triggered()), this, SLOT(Clear()));
    connect(actionScreenshot, SIGNAL(triggered()), this, SLOT(Screenshot()));
    connect(actionNew, SIGNAL(triggered()), this, SLOT(ClearData()));
    connect(actionSave, SIGNAL(triggered()), this, SLOT(SaveData()));
    connect(actionLoad, SIGNAL(triggered()), this, SLOT(LoadData()));
    connect(actionShowStats, SIGNAL(triggered()), this, SLOT(ShowStatsDialog()));

    /*
 connect(actionClearData, SIGNAL(triggered()), this, SLOT(ClearData()));
 connect(actionClearModel, SIGNAL(triggered()), this, SLOT(Clear()));
 connect(actionNew, SIGNAL(triggered()), this, SLOT(ClearData()));
 connect(actionSave, SIGNAL(triggered()), this, SLOT(SaveData()));
 connect(actionLoad, SIGNAL(triggered()), this, SLOT(LoadData()));
 */

    toolBar = addToolBar("Tools");
    toolBar->setObjectName("MainToolBar");
    toolBar->setMovable(false);
    toolBar->setFloatable(false);
    toolBar->setIconSize(QSize(32,32));
    toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);

    toolBar->addAction(actionNew);
    toolBar->addAction(actionLoad);
    toolBar->addAction(actionSave);
    toolBar->addSeparator();
    toolBar->addAction(actionAlgorithms);
    toolBar->addAction(actionCompare);
    toolBar->addAction(actionGridsearch);
    toolBar->addSeparator();
    toolBar->addAction(actionClearModel);
    toolBar->addAction(actionClearData);
    toolBar->addAction(actionClearAll);
    toolBar->addSeparator();
    toolBar->addAction(actionDrawSamples);
    toolBar->addAction(actionAddData);
    toolBar->addSeparator();
    toolBar->addAction(actionScreenshot);
    toolBar->addAction(actionDisplayOptions);
    toolBar->addAction(actionShowStats);
    toolBar->setVisible(true);

    connect(toolBar, SIGNAL(topLevelChanged(bool)), this, SLOT(ShowToolbar()));
    connect(ui.actionShow_Toolbar, SIGNAL(triggered()), this, SLOT(ShowToolbar()));
    connect(ui.actionSmall_Icons, SIGNAL(triggered()), this, SLOT(ShowToolbar()));
    connect(ui.canvasTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(CanvasTypeChanged()));
    connect(ui.canvasX1Spin, SIGNAL(valueChanged(int)), this, SLOT(DisplayOptionsChanged()));
    connect(ui.canvasX2Spin, SIGNAL(valueChanged(int)), this, SLOT(DisplayOptionsChanged()));
    connect(ui.canvasX3Spin, SIGNAL(valueChanged(int)), this, SLOT(DisplayOptionsChanged()));

    QSize iconSize(24,24);
    drawToolbar->singleButton->setIcon(QIcon(":/MLDemos/icons/brush.png"));
    drawToolbar->singleButton->setIconSize(iconSize);
    drawToolbar->singleButton->setText("");
    drawToolbar->singleButton->setAutoExclusive(true);
    drawToolbar->sprayButton->setIcon(QIcon(":/MLDemos/icons/airbrush.png"));
    drawToolbar->sprayButton->setIconSize(iconSize);
    drawToolbar->sprayButton->setText("");
    drawToolbar->sprayButton->setAutoExclusive(true);
    drawToolbar->spray3DButton->setIcon(QIcon(":/MLDemos/icons/airbrush3D.png"));
    drawToolbar->spray3DButton->setIconSize(iconSize);
    drawToolbar->spray3DButton->setText("");
    drawToolbar->spray3DButton->setAutoExclusive(true);
    drawToolbar->eraseButton->setIcon(QIcon(":/MLDemos/icons/erase.png"));
    drawToolbar->eraseButton->setIconSize(iconSize);
    drawToolbar->eraseButton->setText("");
    drawToolbar->eraseButton->setAutoExclusive(true);
    drawToolbar->trajectoryButton->setIcon(QIcon(":/MLDemos/icons/trajectory.png"));
    drawToolbar->trajectoryButton->setIconSize(iconSize);
    drawToolbar->trajectoryButton->setText("");
    drawToolbar->trajectoryButton->setAutoExclusive(true);
    drawToolbar->lineButton->setIcon(QIcon(":/MLDemos/icons/line.png"));
    drawToolbar->lineButton->setIconSize(iconSize);
    drawToolbar->lineButton->setText("");
    drawToolbar->lineButton->setAutoExclusive(true);
    drawToolbar->ellipseButton->setIcon(QIcon(":/MLDemos/icons/ellipse.png"));
    drawToolbar->ellipseButton->setIconSize(iconSize);
    drawToolbar->ellipseButton->setText("");
    drawToolbar->ellipseButton->setAutoExclusive(true);
    drawToolbar->paintButton->setIcon(QIcon(":/MLDemos/icons/bigbrush.png"));
    drawToolbar->paintButton->setIconSize(iconSize);
    drawToolbar->paintButton->setText("");
    drawToolbar->paintButton->setAutoExclusive(true);
    drawToolbar->obstacleButton->setIcon(QIcon(":/MLDemos/icons/obstacle.png"));
    drawToolbar->obstacleButton->setIconSize(iconSize);
    drawToolbar->obstacleButton->setText("");
    drawToolbar->obstacleButton->setAutoExclusive(true);
    drawToolbar->dragButton->setAutoExclusive(true);
    drawToolbar->dragButton->setIcon(QIcon(":/MLDemos/icons/drag.png"));
    drawToolbar->dragButton->setIconSize(iconSize);
    drawToolbar->dragButton->setText("");
    drawToolbar->moveButton->setAutoExclusive(true);
    drawToolbar->moveButton->setIcon(QIcon(":/MLDemos/icons/move.png"));
    drawToolbar->moveButton->setIconSize(iconSize);
    drawToolbar->moveButton->setText("");
    drawToolbar->moveClassButton->setAutoExclusive(true);
    drawToolbar->moveClassButton->setIcon(QIcon(":/MLDemos/icons/move-class.png"));
    drawToolbar->moveClassButton->setIconSize(iconSize);
    drawToolbar->moveClassButton->setText("");
    drawToolbar->extrudeButton->setAutoExclusive(true);
    drawToolbar->extrudeButton->setIcon(QIcon(":/MLDemos/icons/extrude.png"));
    drawToolbar->extrudeButton->setIconSize(iconSize);
    drawToolbar->extrudeButton->setText("");
    drawToolbar->dimPlusButton->setAutoExclusive(true);
    drawToolbar->dimPlusButton->setIcon(QIcon(":/MLDemos/icons/dimplus.png"));
    drawToolbar->dimPlusButton->setIconSize(iconSize);
    drawToolbar->dimPlusButton->setText("");
    drawToolbar->dimLessButton->setAutoExclusive(true);
    drawToolbar->dimLessButton->setIcon(QIcon(":/MLDemos/icons/dimless.png"));
    drawToolbar->dimLessButton->setIconSize(iconSize);
    drawToolbar->dimLessButton->setText("");
    drawToolbar->sprayClassButton->setAutoExclusive(true);
    drawToolbar->sprayClassButton->setIcon(QIcon(":/MLDemos/icons/sprayclass.png"));
    drawToolbar->sprayClassButton->setIconSize(iconSize);
    drawToolbar->sprayClassButton->setText("");
    connect(drawToolbar->dimPlusButton, SIGNAL(clicked()), this, SLOT(DimPlus()));
    connect(drawToolbar->dimLessButton, SIGNAL(clicked()), this, SLOT(DimLess()));
}

void MLDemos::initDialogs()
{
    drawToolbar = new Ui::DrawingToolbar();
    drawToolbarContext1 = new Ui::DrawingToolbarContext1();
    drawToolbarContext2 = new Ui::DrawingToolbarContext2();
    drawToolbarContext3 = new Ui::DrawingToolbarContext3();
    drawToolbarContext4 = new Ui::DrawingToolbarContext4();

    drawToolbar->setupUi(drawToolbarWidget = new QWidget());
    drawToolbarContext1->setupUi(drawContext1Widget = new QWidget());
    drawToolbarContext2->setupUi(drawContext2Widget = new QWidget());
    drawToolbarContext3->setupUi(drawContext3Widget = new QWidget());
    drawToolbarContext4->setupUi(drawContext4Widget = new QWidget());

    connect(qApp, SIGNAL(focusChanged(QWidget *,QWidget *)),this,SLOT(FocusChanged(QWidget *,QWidget *)));

    drawToolbar->sprayButton->setContextMenuPolicy(Qt::CustomContextMenu);
    drawToolbar->ellipseButton->setContextMenuPolicy(Qt::CustomContextMenu);
    drawToolbar->lineButton->setContextMenuPolicy(Qt::CustomContextMenu);
    drawToolbar->eraseButton->setContextMenuPolicy(Qt::CustomContextMenu);
    drawToolbar->obstacleButton->setContextMenuPolicy(Qt::CustomContextMenu);
    drawToolbar->paintButton->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(drawToolbar->sprayButton, SIGNAL(customContextMenuRequested( const QPoint& )), this, SLOT(ShowContextMenuSpray(const QPoint &)));
    connect(drawToolbar->ellipseButton, SIGNAL(customContextMenuRequested( const QPoint& )), this, SLOT(ShowContextMenuEllipse(const QPoint &)));
    connect(drawToolbar->lineButton, SIGNAL(customContextMenuRequested( const QPoint& )), this, SLOT(ShowContextMenuLine(const QPoint &)));
    connect(drawToolbar->eraseButton, SIGNAL(customContextMenuRequested( const QPoint& )), this, SLOT(ShowContextMenuErase(const QPoint &)));
    connect(drawToolbar->obstacleButton, SIGNAL(customContextMenuRequested( const QPoint& )), this, SLOT(ShowContextMenuObstacle(const QPoint &)));
    connect(drawToolbar->paintButton, SIGNAL(customContextMenuRequested( const QPoint& )), this, SLOT(ShowContextMenuReward(const QPoint &)));

    displayOptions = new Ui::viewOptionDialog();
    aboutPanel = new Ui::aboutDialog();
    showStats = new Ui::statisticsDialog();
    manualSelection = new Ui::ManualSelection();
    inputDimensions = new Ui::InputDimensions();

    displayOptions->setupUi(displayDialog = new QDialog());
    aboutPanel->setupUi(aboutDialog = new QDialog());
    showStats->setupUi(statsDialog = new QDialog());
    manualSelection->setupUi(manualSelectDialog = new QDialog());
    inputDimensions->setupUi(inputDimensionsDialog = new QDialog());
    rocWidget = new QNamedWindow("ROC Curve", false, showStats->rocWidget);

    connect(showStats->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(StatsChanged()));
    connect(rocWidget, SIGNAL(ResizeEvent(QResizeEvent *)), this, SLOT(StatsChanged()));
    connect(manualSelection->sampleList, SIGNAL(itemSelectionChanged()), this, SLOT(ManualSelectionChanged()));
    connect(manualSelection->clearSelectionButton, SIGNAL(clicked()), this, SLOT(ManualSelectionClear()));
    connect(manualSelection->invertSelectionButton, SIGNAL(clicked()), this, SLOT(ManualSelectionInvert()));
    connect(manualSelection->removeSampleButton, SIGNAL(clicked()), this, SLOT(ManualSelectionRemove()));
    connect(manualSelection->randomizeSelectionButton, SIGNAL(clicked()), this, SLOT(ManualSelectionRandom()));
    connect(inputDimensions->dimList, SIGNAL(itemSelectionChanged()), this, SLOT(InputDimensionsChanged()));
    connect(inputDimensions->clearSelectionButton, SIGNAL(clicked()), this, SLOT(InputDimensionsClear()));
    connect(inputDimensions->invertSelectionButton, SIGNAL(clicked()), this, SLOT(InputDimensionsInvert()));
    connect(inputDimensions->randomizeSelectionButton, SIGNAL(clicked()), this, SLOT(InputDimensionsRandom()));

    connect(displayOptions->clipboardButton, SIGNAL(clicked()), this, SLOT(ToClipboard()));
    connect(displayOptions->gridCheck, SIGNAL(clicked()), this, SLOT(Display3DOptionsChanged()));
    connect(displayOptions->zoomFitButton, SIGNAL(clicked()), this, SLOT(FitToData()));
    connect(displayOptions->mapCheck, SIGNAL(clicked()), this, SLOT(DisplayOptionsChanged()));
    connect(displayOptions->modelCheck, SIGNAL(clicked()), this, SLOT(DisplayOptionsChanged()));
    connect(displayOptions->infoCheck, SIGNAL(clicked()), this, SLOT(DisplayOptionsChanged()));
    connect(displayOptions->samplesCheck, SIGNAL(clicked()), this, SLOT(DisplayOptionsChanged()));
    connect(displayOptions->spinZoom, SIGNAL(valueChanged(double)), this, SLOT(DisplayOptionsChanged()));
    connect(displayOptions->legendCheck, SIGNAL(clicked()), this, SLOT(DisplayOptionsChanged()));
    connect(displayOptions->check3DSamples, SIGNAL(clicked()), this, SLOT(Display3DOptionsChanged()));
    connect(displayOptions->check3DWireframe, SIGNAL(clicked()), this, SLOT(Display3DOptionsChanged()));
    connect(displayOptions->check3DSurfaces, SIGNAL(clicked()), this, SLOT(Display3DOptionsChanged()));
    connect(displayOptions->check3DTransparency, SIGNAL(clicked()), this, SLOT(Display3DOptionsChanged()));
    connect(displayOptions->check3DBlurry, SIGNAL(clicked()), this, SLOT(Display3DOptionsChanged()));
    connect(displayOptions->check3DRotate, SIGNAL(clicked()), this, SLOT(Display3DOptionsChanged()));

    algorithmOptions = new Ui::algorithmOptions();
    optionsClassify = new Ui::optionsClassifyWidget();
    optionsCluster = new Ui::optionsClusterWidget();
    optionsRegress = new Ui::optionsRegressWidget();
    optionsDynamic = new Ui::optionsDynamicWidget();
    optionsMaximize = new Ui::optionsMaximizeWidget();
    optionsReinforcement = new Ui::optionsReinforcementWidget();
    optionsProject = new Ui::optionsProjectWidget();
    optionsCompare = new Ui::optionsCompare();

    algorithmWidget = new QWidget();
    algorithmOptions->setupUi(algorithmWidget);

    algorithmWidget->setWindowFlags(Qt::Tool); // disappears when unfocused on the mac
    //algorithmWidget->setWindowFlags(Qt::WindowStaysOnTopHint);
    displayDialog->setWindowFlags(Qt::Tool); // disappears when unfocused on the mac
    //drawToolbarWidget->setWindowFlags(Qt::Tool);
    drawToolbarWidget->setWindowFlags(Qt::CustomizeWindowHint | Qt::Tool | Qt::WindowTitleHint);
    drawContext1Widget->setWindowFlags(Qt::FramelessWindowHint);
    drawContext2Widget->setWindowFlags(Qt::FramelessWindowHint);
    drawContext3Widget->setWindowFlags(Qt::FramelessWindowHint);
    drawContext4Widget->setWindowFlags(Qt::FramelessWindowHint);
    drawToolbarWidget->setFixedSize(drawToolbarWidget->size());

    classifyWidget = new QWidget(algorithmOptions->tabClass);
    clusterWidget = new QWidget(algorithmOptions->tabClust);
    regressWidget = new QWidget(algorithmOptions->tabRegr);
    dynamicWidget = new QWidget(algorithmOptions->tabDyn);
    maximizeWidget = new QWidget(algorithmOptions->tabMax);
    reinforcementWidget = new QWidget(algorithmOptions->tabReinf);
    projectWidget = new QWidget(algorithmOptions->tabProj);
    optionsClassify->setupUi(classifyWidget);
    optionsCluster->setupUi(clusterWidget);
    optionsRegress->setupUi(regressWidget);
    optionsDynamic->setupUi(dynamicWidget);
    optionsMaximize->setupUi(maximizeWidget);
    optionsReinforcement->setupUi(reinforcementWidget);
    optionsProject->setupUi(projectWidget);
    compareWidget = new QWidget();
    optionsCompare->setupUi(compareWidget);
    optionsDynamic->targetButton->setAcceptDrops(true);

    connect(displayDialog, SIGNAL(rejected()), this, SLOT(HideOptionDisplay()));
    connect(statsDialog, SIGNAL(rejected()), this, SLOT(HideStatsDialog()));
    //connect(manualSelectDialog, SIGNAL(rejected()), this, SLOT(HideManualSelectionDialog()));

    connect(optionsClassify->classifyButton, SIGNAL(clicked()), this, SLOT(Classify()));
    connect(optionsClassify->clearButton, SIGNAL(clicked()), this, SLOT(Clear()));
    connect(optionsClassify->rocButton, SIGNAL(clicked()), this, SLOT(ShowRoc()));
    connect(optionsClassify->compareButton, SIGNAL(clicked()), this, SLOT(CompareAdd()));
    connect(optionsClassify->manualTrainButton, SIGNAL(clicked()), this, SLOT(ManualSelection()));
    connect(optionsClassify->inputDimButton, SIGNAL(clicked()), this, SLOT(InputDimensions()));
    connect(optionsClassify->loadButton, SIGNAL(clicked()), this, SLOT(LoadClassifier()));
    connect(optionsClassify->saveButton, SIGNAL(clicked()), this, SLOT(SaveClassifier()));
    connect(optionsClassify->algoList, SIGNAL(currentIndexChanged(int)), this, SLOT(AlgoChanged()));
    if(!optionsClassify->algoWidget->layout())
    {
        QHBoxLayout *horizontalLayout = new QHBoxLayout(optionsClassify->algoWidget);
        horizontalLayout->setSpacing(0);
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
    }

    connect(optionsRegress->regressionButton, SIGNAL(clicked()), this, SLOT(Regression()));
    connect(optionsRegress->clearButton, SIGNAL(clicked()), this, SLOT(Clear()));
    //connect(optionsRegress->svmTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeActiveOptions()));
    connect(optionsRegress->compareButton, SIGNAL(clicked()), this, SLOT(CompareAdd()));
    connect(optionsRegress->manualTrainButton, SIGNAL(clicked()), this, SLOT(ManualSelection()));
    connect(optionsRegress->inputDimButton, SIGNAL(clicked()), this, SLOT(InputDimensions()));
    connect(optionsRegress->loadButton, SIGNAL(clicked()), this, SLOT(LoadRegressor()));
    connect(optionsRegress->saveButton, SIGNAL(clicked()), this, SLOT(SaveRegressor()));
    connect(optionsRegress->algoList, SIGNAL(currentIndexChanged(int)), this, SLOT(AlgoChanged()));
    if(!optionsRegress->algoWidget->layout())
    {
        QHBoxLayout *horizontalLayout = new QHBoxLayout(optionsRegress->algoWidget);
        horizontalLayout->setSpacing(0);
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
    }

    connect(optionsCluster->clusterButton, SIGNAL(clicked()), this, SLOT(Cluster()));
    connect(optionsCluster->iterationButton, SIGNAL(clicked()), this, SLOT(ClusterIterate()));
    connect(optionsCluster->optimizeButton, SIGNAL(clicked()), this, SLOT(ClusterOptimize()));
    connect(optionsCluster->testButton, SIGNAL(clicked()), this, SLOT(ClusterTest()));
    connect(optionsCluster->clearButton, SIGNAL(clicked()), this, SLOT(Clear()));
    connect(optionsCluster->manualTrainButton, SIGNAL(clicked()), this, SLOT(ManualSelection()));
    connect(optionsCluster->optimizeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(ClusterChanged()));
    connect(optionsCluster->algoList, SIGNAL(currentIndexChanged(int)), this, SLOT(AlgoChanged()));
    if(!optionsCluster->algoWidget->layout())
    {
        QHBoxLayout *horizontalLayout = new QHBoxLayout(optionsCluster->algoWidget);
        horizontalLayout->setSpacing(0);
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
    }

    connect(optionsDynamic->regressionButton, SIGNAL(clicked()), this, SLOT(Dynamize()));
    connect(optionsDynamic->clearButton, SIGNAL(clicked()), this, SLOT(Clear()));
    connect(optionsDynamic->centerCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeActiveOptions()));
    connect(optionsDynamic->resampleCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeActiveOptions()));
    connect(optionsDynamic->resampleSpin, SIGNAL(valueChanged(int)), this, SLOT(ChangeActiveOptions()));
    connect(optionsDynamic->loadButton, SIGNAL(clicked()), this, SLOT(LoadDynamical()));
    connect(optionsDynamic->saveButton, SIGNAL(clicked()), this, SLOT(SaveDynamical()));
    connect(optionsDynamic->dtSpin, SIGNAL(valueChanged(double)), this, SLOT(ChangeActiveOptions()));
    connect(optionsDynamic->obstacleCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(AvoidOptionChanged()));
    connect(optionsDynamic->compareButton, SIGNAL(clicked()), this, SLOT(CompareAdd()));
    connect(optionsDynamic->colorCheck, SIGNAL(clicked()), this, SLOT(ColorMapChanged()));
    connect(optionsDynamic->resampleCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeActiveOptions()));
    connect(optionsDynamic->targetButton, SIGNAL(pressed()), this, SLOT(TargetButton()));
    connect(optionsDynamic->clearTargetButton, SIGNAL(clicked()), this, SLOT(ClearTargets()));
    connect(optionsDynamic->algoList, SIGNAL(currentIndexChanged(int)), this, SLOT(AlgoChanged()));
    if(!optionsDynamic->algoWidget->layout())
    {
        QHBoxLayout *horizontalLayout = new QHBoxLayout(optionsDynamic->algoWidget);
        horizontalLayout->setSpacing(0);
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
    }

    connect(optionsMaximize->maximizeButton, SIGNAL(clicked()), this, SLOT(Maximize()));
    connect(optionsMaximize->pauseButton, SIGNAL(clicked()), this, SLOT(MaximizeContinue()));
    connect(optionsMaximize->clearButton, SIGNAL(clicked()), this, SLOT(Clear()));
    connect(optionsMaximize->targetButton, SIGNAL(pressed()), this, SLOT(TargetButton()));
    connect(optionsMaximize->gaussianButton, SIGNAL(pressed()), this, SLOT(GaussianButton()));
    connect(optionsMaximize->gradientButton, SIGNAL(pressed()), this, SLOT(GradientButton()));
    connect(optionsMaximize->benchmarkButton, SIGNAL(clicked()), this, SLOT(BenchmarkButton()));
    connect(optionsMaximize->compareButton, SIGNAL(clicked()), this, SLOT(CompareAdd()));
    connect(optionsMaximize->algoList, SIGNAL(currentIndexChanged(int)), this, SLOT(AlgoChanged()));
    if(!optionsMaximize->algoWidget->layout())
    {
        QHBoxLayout *horizontalLayout = new QHBoxLayout(optionsMaximize->algoWidget);
        horizontalLayout->setSpacing(0);
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
    }

    connect(optionsReinforcement->maximizeButton, SIGNAL(clicked()), this, SLOT(Reinforce()));
    connect(optionsReinforcement->pauseButton, SIGNAL(clicked()), this, SLOT(ReinforceContinue()));
    connect(optionsReinforcement->clearButton, SIGNAL(clicked()), this, SLOT(Clear()));
    connect(optionsReinforcement->targetButton, SIGNAL(pressed()), this, SLOT(TargetButton()));
    connect(optionsReinforcement->clearTargetButton, SIGNAL(clicked()), this, SLOT(ClearTargets()));
    connect(optionsReinforcement->gaussianButton, SIGNAL(pressed()), this, SLOT(GaussianButton()));
    connect(optionsReinforcement->gradientButton, SIGNAL(pressed()), this, SLOT(GradientButton()));
    connect(optionsReinforcement->benchmarkButton, SIGNAL(clicked()), this, SLOT(BenchmarkButton()));
    connect(optionsReinforcement->algoList, SIGNAL(currentIndexChanged(int)), this, SLOT(AlgoChanged()));
    if(!optionsReinforcement->algoWidget->layout())
    {
        QHBoxLayout *horizontalLayout = new QHBoxLayout(optionsReinforcement->algoWidget);
        horizontalLayout->setSpacing(0);
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
    }

    connect(optionsProject->projectButton, SIGNAL(clicked()), this, SLOT(Project()));
    connect(optionsProject->manifoldButton, SIGNAL(clicked()), this, SLOT(ProjectManifold()));
    connect(optionsProject->revertButton, SIGNAL(clicked()), this, SLOT(ProjectRevert()));
    connect(optionsProject->reprojectButton, SIGNAL(clicked()), this, SLOT(ProjectReproject()));
    connect(optionsProject->manualTrainButton, SIGNAL(clicked()), this, SLOT(ManualSelection()));
    connect(optionsProject->algoList, SIGNAL(currentIndexChanged(int)), this, SLOT(AlgoChanged()));
    if(!optionsProject->algoWidget->layout())
    {
        QHBoxLayout *horizontalLayout = new QHBoxLayout(optionsProject->algoWidget);
        horizontalLayout->setSpacing(0);
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
    }

    connect(optionsCompare->compareButton, SIGNAL(clicked()), this, SLOT(Compare()));
    connect(optionsCompare->screenshotButton, SIGNAL(clicked()), this, SLOT(CompareScreenshot()));
    connect(optionsCompare->clearButton, SIGNAL(clicked()), this, SLOT(CompareClear()));
    connect(optionsCompare->removeButton, SIGNAL(clicked()), this, SLOT(CompareRemove()));
    connect(optionsCompare->inputDimButton, SIGNAL(clicked()), this, SLOT(InputDimensions()));

    connect(optionsRegress->outputDimCombo, SIGNAL(currentIndexChanged(int)), optionsCompare->outputDimCombo, SLOT(setCurrentIndex(int)));
    connect(optionsCompare->outputDimCombo, SIGNAL(currentIndexChanged(int)), optionsRegress->outputDimCombo, SLOT(setCurrentIndex(int)));

    optionsClassify->algoList->clear();
    optionsCluster->algoList->clear();
    optionsRegress->algoList->clear();
    optionsDynamic->algoList->clear();
    optionsMaximize->algoList->clear();
    optionsReinforcement->algoList->clear();
    optionsProject->algoList->clear();

    QHBoxLayout *layout = new QHBoxLayout(optionsCompare->resultWidget);
    compare = new CompareAlgorithms(optionsCompare->resultWidget);

    connect(algorithmOptions->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(AlgoChanged()));

    //canvas = new Canvas(ui.centralWidget);
    canvas = new Canvas(ui.canvasWidget);
    connect(canvas, SIGNAL(Drawing(fvec,int)), this, SLOT(Drawing(fvec,int)));
    connect(canvas, SIGNAL(DrawCrosshair()), this, SLOT(DrawCrosshair()));
    connect(canvas, SIGNAL(Navigation(fvec)), this, SLOT(Navigation(fvec)));
    connect(canvas, SIGNAL(Released()), this, SLOT(DrawingStopped()));
    connect(canvas, SIGNAL(CanvasMoveEvent()), this, SLOT(CanvasMoveEvent()));
    //connect(canvas, SIGNAL(ZoomChanged()), this, SLOT(ZoomChanged()));

    import = new DataImporter();
    connect(import, import->SetDataSignal(), this, SLOT(SetData(std::vector<fvec>, ivec, std::vector<ipair>, bool)));
    connect(import, import->SetTimeseriesSignal(), this, SLOT(SetTimeseries(std::vector<TimeSerie>)));
    connect(import, SIGNAL(SetDimensionNames(QStringList)), this, SLOT(SetDimensionNames(QStringList)));
    connect(import, SIGNAL(SetClassNames(std::map<int,QString>)), this, SLOT(SetClassNames(std::map<int,QString>)));
    connect(import, SIGNAL(SetCategorical(std::map<int,std::vector<std::string> >)), this, SLOT(SetCategorical(std::map<int,std::vector<std::string> >)));

    gridSearch = new GridSearch(canvas);
    generator = new DataGenerator(canvas);
    connect(generator->ui->addButton, SIGNAL(clicked()), this, SLOT(AddData()));

    glw = new GLWidget(canvas);
    vis = new Visualization(canvas);

    dataEdit = new DatasetEditor(canvas);
    connect(dataEdit, SIGNAL(DataEdited()), this, SLOT(DataEdited()));

    drawTimer = new DrawTimer(canvas, &mutex);
    drawTimer->glw = glw;
    drawTimer->classifier = &classifier;
    drawTimer->regressor = &regressor;
    drawTimer->dynamical = &dynamical;
    drawTimer->clusterer = &clusterer;
    drawTimer->maximizer = &maximizer;
    drawTimer->reinforcement = &reinforcement;
    drawTimer->reinforcementProblem = &reinforcementProblem;
    drawTimer->classifierMulti = &classifierMulti;
    connect(drawTimer, SIGNAL(MapReady(QImage)), canvas, SLOT(SetConfidenceMap(QImage)));
    connect(drawTimer, SIGNAL(ModelReady(QImage)), canvas, SLOT(SetModelImage(QImage)));
    connect(drawTimer, SIGNAL(CurveReady()), this, SLOT(SetROCInfo()));
    connect(drawTimer, SIGNAL(AnimationReady(QImage)), canvas, SLOT(SetAnimationImage(QImage)));
}

void MLDemos::initPlugins()
{
    qDebug() << "Importing plugins";
    QDir pluginsDir = QDir(qApp->applicationDirPath());
    QStringList pluginFileNames;
    QDir alternativeDir = pluginsDir;

#if defined(Q_OS_WIN)
    if (pluginsDir.dirName().toLower() == "debug" || pluginsDir.dirName().toLower() == "release") pluginsDir.cdUp();
#elif defined(Q_OS_MAC)
    if (pluginsDir.dirName() == "MacOS") {
        if(!pluginsDir.cd("plugins"))
        {
            qDebug() << "looking for alternative directory";
            pluginsDir.cdUp();
            pluginsDir.cdUp();
            alternativeDir = pluginsDir;
            alternativeDir.cd("plugins");
        }
        pluginsDir.cdUp();
    }
#endif
    bool bFoundPlugins = false;
#if defined(DEBUG)
    qDebug() << "looking for debug plugins";
    bFoundPlugins = pluginsDir.cd("pluginsDebug");
#else
    qDebug() << "looking for release plugins";
    bFoundPlugins = pluginsDir.cd("plugins");
#endif
    if(!bFoundPlugins)
    {
        qDebug() << "plugins not found on: " << pluginsDir.absolutePath();
        qDebug() << "using alternative directory: " << alternativeDir.absolutePath();
        pluginsDir = alternativeDir;
    }
    foreach (QString fileName, pluginsDir.entryList(QDir::Files))
    {
        QPluginLoader *pluginLoader = new QPluginLoader(pluginsDir.absoluteFilePath(fileName));
        QObject *plugin = pluginLoader->instance();
        if (plugin)
        {
            pluginLoaders.push_back(pluginLoader);
            qDebug() << "loading " << fileName;
            // check type of plugin
            CollectionInterface *iCollection = qobject_cast<CollectionInterface *>(plugin);
            if(iCollection)
            {
                std::vector<ClassifierInterface*> classifierList = iCollection->GetClassifiers();
                std::vector<ClustererInterface*> clustererList = iCollection->GetClusterers();
                std::vector<RegressorInterface*> regressorList = iCollection->GetRegressors();
                std::vector<DynamicalInterface*> dynamicalList = iCollection->GetDynamicals();
                std::vector<MaximizeInterface*> maximizerList = iCollection->GetMaximizers();
                std::vector<ReinforcementInterface*> reinforcementList = iCollection->GetReinforcements();
                std::vector<ProjectorInterface*> projectorList = iCollection->GetProjectors();
                FOR(i, classifierList.size()) AddPlugin(classifierList[i], SLOT(ChangeActiveOptions));
                FOR(i, clustererList.size()) AddPlugin(clustererList[i], SLOT(ChangeActiveOptions));
                FOR(i, regressorList.size()) AddPlugin(regressorList[i], SLOT(ChangeActiveOptions));
                FOR(i, dynamicalList.size()) AddPlugin(dynamicalList[i], SLOT(ChangeActiveOptions));
                FOR(i, maximizerList.size()) AddPlugin(maximizerList[i], SLOT(ChangeActiveOptions));
                FOR(i, reinforcementList.size()) AddPlugin(reinforcementList[i], SLOT(ChangeActiveOptions));
                FOR(i, projectorList.size()) AddPlugin(projectorList[i], SLOT(ChangeActiveOptions));
                continue;
            }
            ClassifierInterface *iClassifier = qobject_cast<ClassifierInterface *>(plugin);
            if (iClassifier)
            {
                AddPlugin(iClassifier, SLOT(ChangeActiveOptions()));
                continue;
            }
            ClustererInterface *iClusterer = qobject_cast<ClustererInterface *>(plugin);
            if (iClusterer)
            {
                AddPlugin(iClusterer, SLOT(ChangeActiveOptions()));
                continue;
            }
            RegressorInterface *iRegressor = qobject_cast<RegressorInterface *>(plugin);
            if (iRegressor)
            {
                AddPlugin(iRegressor, SLOT(ChangeActiveOptions()));
                continue;
            }
            DynamicalInterface *iDynamical = qobject_cast<DynamicalInterface *>(plugin);
            if (iDynamical)
            {
                AddPlugin(iDynamical, SLOT(ChangeActiveOptions()));
                continue;
            }
            MaximizeInterface *iMaximize = qobject_cast<MaximizeInterface *>(plugin);
            if (iMaximize)
            {
                AddPlugin(iMaximize, SLOT(ChangeActiveOptions()));
                continue;
            }
            ReinforcementInterface *iReinforcement = qobject_cast<ReinforcementInterface *>(plugin);
            if (iReinforcement)
            {
                AddPlugin(iReinforcement, SLOT(ChangeActiveOptions()));
                continue;
            }
            ProjectorInterface *iProject = qobject_cast<ProjectorInterface *>(plugin);
            if (iProject)
            {
                AddPlugin(iProject, SLOT(ChangeActiveOptions()));
                continue;
            }
            InputOutputInterface *iIO = qobject_cast<InputOutputInterface *>(plugin);
            if (iIO)
            {
                AddPlugin(iIO);
                continue;
            }
            AvoidanceInterface *iAvoid = qobject_cast<AvoidanceInterface *>(plugin);
            if (iAvoid)
            {
                AddPlugin(iAvoid, SLOT(ChangeActiveOptions()));
                continue;
            }
        }
        else
        {
            qDebug() << pluginLoader->errorString();
            delete pluginLoader;
        }
    }
}

// this function is to set the font size of gui elements in the interface for non-osx systems
// this is necessary because the different OS interpret in different ways the default styles
// which tends to mess up the layout of labels and texts
void MLDemos::SetTextFontSize()
{
#if defined(Q_OS_MAC)
    return; // default fontsizes are for mac already ;)
#endif
    QList<QWidget*> children;
    // we will make this tiny so that we don't risk cropping the texts
    QFont font("Lucida Sans Unicode", 7);
    children = algorithmWidget->findChildren<QWidget*>();
    FOR(i, children.size()) if(children[i]) children[i]->setFont(font);
    optionsMaximize->gaussianButton->setFont(QFont("Lucida Sans Unicode", 18));
    optionsMaximize->gradientButton->setFont(QFont("Lucida Sans Unicode", 18));
    optionsMaximize->targetButton->setFont(QFont("Lucida Sans Unicode", 18));
    optionsReinforcement->gaussianButton->setFont(QFont("Lucida Sans Unicode", 18));
    optionsReinforcement->gradientButton->setFont(QFont("Lucida Sans Unicode", 18));
    optionsReinforcement->targetButton->setFont(QFont("Lucida Sans Unicode", 18));
    children = compareWidget->findChildren<QWidget*>();
    FOR(i, children.size()) if(children[i]) children[i]->setFont(font);
    children = displayDialog->findChildren<QWidget*>();
    FOR(i, children.size()) if(children[i]) children[i]->setFont(font);
    children = statsDialog->findChildren<QWidget*>();
    FOR(i, children.size()) if(children[i]) children[i]->setFont(font);
    children = aboutDialog->findChildren<QWidget*>();
    FOR(i, children.size()) if(children[i]) children[i]->setFont(font);
    children = manualSelectDialog->findChildren<QWidget*>();
    FOR(i, children.size()) if(children[i]) children[i]->setFont(font);
    children = inputDimensionsDialog->findChildren<QWidget*>();
    FOR(i, children.size()) if(children[i]) children[i]->setFont(font);
    children = generator->findChildren<QWidget*>();
    FOR(i, children.size()) if(children[i]) children[i]->setFont(font);
}

void MLDemos::ShowContextMenuSpray(const QPoint &point)
{
    HideContextMenus();
    QPoint pt = QPoint(30, 0);
    drawContext1Widget->move(drawToolbar->sprayButton->mapToGlobal(pt));
    drawContext1Widget->show();
    drawContext1Widget->setFocus();
    drawContext1Widget->repaint();
    update();
}

void MLDemos::ShowContextMenuLine(const QPoint &point)
{
    HideContextMenus();
    QPoint pt = QPoint(30, 0);
    drawContext2Widget->move(drawToolbar->lineButton->mapToGlobal(pt));
    drawContext2Widget->show();
    drawContext2Widget->setFocus();
    drawContext2Widget->repaint();
    update();
}
void MLDemos::ShowContextMenuEllipse(const QPoint &point)
{
    HideContextMenus();
    QPoint pt = QPoint(30, 0);
    drawContext2Widget->move(drawToolbar->ellipseButton->mapToGlobal(pt));
    drawContext2Widget->show();
    drawContext2Widget->setFocus();
    drawContext2Widget->repaint();
    update();
}
void MLDemos::ShowContextMenuErase(const QPoint &point)
{
    HideContextMenus();
    QPoint pt = QPoint(30, 0);
    drawContext1Widget->move(drawToolbar->eraseButton->mapToGlobal(pt));
    drawContext1Widget->show();
    drawContext1Widget->setFocus();
    drawContext1Widget->repaint();
    update();
}
void MLDemos::ShowContextMenuObstacle(const QPoint &point)
{
    HideContextMenus();
    QPoint pt = QPoint(30, 0);
    drawContext3Widget->move(drawToolbar->obstacleButton->mapToGlobal(pt));
    drawContext3Widget->show();
    drawContext3Widget->setFocus();
    drawContext3Widget->repaint();
    update();
}
void MLDemos::ShowContextMenuReward(const QPoint &point)
{
    HideContextMenus();
    QPoint pt = QPoint(30, 0);
    drawContext4Widget->move(drawToolbar->paintButton->mapToGlobal(pt));
    drawContext4Widget->show();
    drawContext4Widget->setFocus();
    drawContext4Widget->repaint();
    update();
}

bool IsChildOf(QObject *child, QObject *parent)
{
    if(!parent || !child) return false;
    if(child == parent) return true;
    QList<QObject*> list = parent->children();
    if(list.isEmpty()) return false;
    QList<QObject*>::iterator i;
    for (i = list.begin(); i<list.end(); ++i)
    {
        if(IsChildOf(child, *i)) return true;
    }
    return false;
}

void MLDemos::FocusChanged(QWidget *old, QWidget *now)
{
    if(drawContext1Widget && drawContext1Widget->isVisible())
    {
        if(!IsChildOf(now, drawContext1Widget)) HideContextMenus();
    }
    if(drawContext2Widget && drawContext2Widget->isVisible())
    {
        if(!IsChildOf(now, drawContext2Widget)) HideContextMenus();
    }
    if(drawContext3Widget && drawContext3Widget->isVisible())
    {
        if(!IsChildOf(now, drawContext3Widget)) HideContextMenus();
    }
    if(drawContext4Widget && drawContext4Widget->isVisible())
    {
        if(!IsChildOf(now, drawContext4Widget)) HideContextMenus();
    }
}

void MLDemos::HideContextMenus()
{
    drawContext1Widget->hide();
    drawContext2Widget->hide();
    drawContext3Widget->hide();
    drawContext4Widget->hide();
}

void MLDemos::AddPlugin(InputOutputInterface *iIO)
{
    inputoutputs.push_back(iIO);
    bInputRunning.push_back(false);
    connect(this, SIGNAL(SendResults(std::vector<fvec>)), iIO->object(), iIO->FetchResultsSlot());
    connect(iIO->object(), iIO->SetDataSignal(), this, SLOT(SetData(std::vector<fvec>, ivec, std::vector<ipair>, bool)));
    connect(iIO->object(), iIO->SetTimeseriesSignal(), this, SLOT(SetTimeseries(std::vector<TimeSerie>)));
    connect(iIO->object(), iIO->QueryClassifierSignal(), this, SLOT(QueryClassifier(std::vector<fvec>)));
    connect(iIO->object(), iIO->QueryRegressorSignal(), this, SLOT(QueryRegressor(std::vector<fvec>)));
    connect(iIO->object(), iIO->QueryDynamicalSignal(), this, SLOT(QueryDynamical(std::vector<fvec>)));
    connect(iIO->object(), iIO->QueryClustererSignal(), this, SLOT(QueryClusterer(std::vector<fvec>)));
    connect(iIO->object(), iIO->QueryMaximizerSignal(), this, SLOT(QueryMaximizer(std::vector<fvec>)));
    connect(iIO->object(), iIO->DoneSignal(), this, SLOT(DisactivateIO(QObject *)));
    QString name = iIO->GetName();
    QAction *pluginAction = ui.menuInput_Output->addAction(name);
    pluginAction->setCheckable(true);
    pluginAction->setChecked(false);
    connect(pluginAction,SIGNAL(toggled(bool)), this, SLOT(ActivateIO()));
    QAction *importAction = ui.menuImport->addAction(name);
    importAction->setCheckable(true);
    importAction->setChecked(false);
    connect(importAction,SIGNAL(toggled(bool)), this, SLOT(ActivateImport()));
}

void MLDemos::AddPlugin(ClassifierInterface *iClassifier, const char *method)
{
    if(!iClassifier) return;
    // we add the interface so we can use it to produce classifiers
    classifiers.push_back(iClassifier);
    // we add the classifier parameters to the gui
    optionsClassify->algoList->addItem(iClassifier->GetName());
    QWidget *widget = iClassifier->GetParameterWidget();
    widget->setParent(optionsClassify->algoWidget);
    optionsClassify->algoWidget->layout()->addWidget(widget);
    widget->hide();
    algoWidgets["classifiers"].push_back(widget);
}

void MLDemos::AddPlugin(ClustererInterface *iCluster, const char *method)
{
    if(!iCluster) return;
    clusterers.push_back(iCluster);
    optionsCluster->algoList->addItem(iCluster->GetName());
    QWidget *widget = iCluster->GetParameterWidget();
    widget->setParent(optionsCluster->algoWidget);
    optionsCluster->algoWidget->layout()->addWidget(widget);
    widget->hide();
    algoWidgets["clusterers"].push_back(widget);
}

void MLDemos::AddPlugin(RegressorInterface *iRegress, const char *method)
{
    if(!iRegress) return;
    regressors.push_back(iRegress);
    optionsRegress->algoList->addItem(iRegress->GetName());
    QWidget *widget = iRegress->GetParameterWidget();
    widget->setParent(optionsRegress->algoWidget);
    optionsRegress->algoWidget->layout()->addWidget(widget);
    widget->hide();
    algoWidgets["regressors"].push_back(widget);
}

void MLDemos::AddPlugin(DynamicalInterface *iDynamical, const char *method)
{
    if(!iDynamical) return;
    dynamicals.push_back(iDynamical);
    optionsDynamic->algoList->addItem(iDynamical->GetName());
    QWidget *widget = iDynamical->GetParameterWidget();
    widget->setParent(optionsDynamic->algoWidget);
    optionsDynamic->algoWidget->layout()->addWidget(widget);
    widget->hide();
    algoWidgets["dynamicals"].push_back(widget);
}

void MLDemos::AddPlugin(AvoidanceInterface *iAvoid, const char *method)
{
    if(!iAvoid) return;
    avoiders.push_back(iAvoid);
    optionsDynamic->obstacleCombo->addItem(iAvoid->GetName());
}

void MLDemos::AddPlugin(MaximizeInterface *iMaximizer, const char *method)
{
    if(!iMaximizer) return;
    maximizers.push_back(iMaximizer);
    optionsMaximize->algoList->addItem(iMaximizer->GetName());
    QWidget *widget = iMaximizer->GetParameterWidget();
    widget->setParent(optionsMaximize->algoWidget);
    optionsMaximize->algoWidget->layout()->addWidget(widget);
    widget->hide();
    algoWidgets["maximizers"].push_back(widget);
}

void MLDemos::AddPlugin(ReinforcementInterface *iReinforcement, const char *method)
{
    if(!iReinforcement) return;
    reinforcements.push_back(iReinforcement);
    optionsReinforcement->algoList->addItem(iReinforcement->GetName());
    QWidget *widget = iReinforcement->GetParameterWidget();
    widget->setParent(optionsReinforcement->algoWidget);
    optionsReinforcement->algoWidget->layout()->addWidget(widget);
    widget->hide();
    algoWidgets["reinforcements"].push_back(widget);
}

void MLDemos::AddPlugin(ProjectorInterface *iProject, const char *method)
{
    if(!iProject) return;
    projectors.push_back(iProject);
    optionsProject->algoList->addItem(iProject->GetName());
    QWidget *widget = iProject->GetParameterWidget();
    widget->setParent(optionsProject->algoWidget);
    optionsProject->algoWidget->layout()->addWidget(widget);
    widget->hide();
    algoWidgets["projectors"].push_back(widget);
}

MLDemos::~MLDemos()
{
    Clear();
    FOR(i, inputoutputs.size())
    {
        if(inputoutputs[i] && bInputRunning[i]) inputoutputs[i]->Stop();
    }
    SaveLayoutOptions();
    DEL(glw);
    DEL(vis);
    FOR(i, pluginLoaders.size()) pluginLoaders.at(i)->unload();
    DEL(optionsClassify);
    DEL(optionsRegress);
    DEL(optionsCluster);
    DEL(optionsDynamic);
    DEL(optionsMaximize);
    DEL(optionsReinforcement);
    DEL(optionsProject);
    DEL(optionsCompare);
    DEL(manualSelection);
    DEL(inputDimensions);
    DEL(showStats);
    DEL(displayOptions);
    DEL(drawToolbar);
    DEL(drawToolbarWidget);
    DEL(drawToolbarContext1);
    DEL(drawToolbarContext2);
    DEL(drawToolbarContext3);
    DEL(drawToolbarContext4);
    DEL(drawContext1Widget);
    DEL(drawContext2Widget);
    DEL(drawContext3Widget);
    DEL(drawContext4Widget);
    DEL(algorithmOptions);
    DEL(displayDialog);
    DEL(aboutPanel);
    DEL(aboutDialog);
    DEL(statsDialog);
    DEL(manualSelectDialog);
    DEL(inputDimensionsDialog);
    DEL(algorithmWidget);
    DEL(compareWidget);
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
    if (true)
    {
        mutex.lock();
        DEL(clusterer);
        DEL(regressor);
        DEL(dynamical);
        if(!classifierMulti.size()) DEL(classifier);
        classifier = 0;
        sourceDims.clear();
        FOR(i,classifierMulti.size()) DEL(classifierMulti[i]); classifierMulti.clear();
        DEL(maximizer);
        DEL(reinforcement);
        DEL(projector);
        mutex.unlock();
        qApp->quit();
    } else {
        event->ignore();
    }
}

void MLDemos::resizeEvent( QResizeEvent *event )
{
    if(!canvas) return;
    if(canvas->canvasType == 2)
    {
        QSizePolicy policy = vis->sizePolicy();
        policy.setHorizontalPolicy(QSizePolicy::Preferred);
        policy.setVerticalPolicy(QSizePolicy::Preferred);
        vis->setSizePolicy(policy);
        vis->setMinimumSize(ui.canvasArea->size());
        vis->resize(ui.canvasArea->size());
        //CanvasOptionsChanged();
    }
    else if (canvas->canvasType == 1) // 3D View
    {
        QSizePolicy policy = glw->sizePolicy();
        policy.setHorizontalPolicy(QSizePolicy::Preferred);
        policy.setVerticalPolicy(QSizePolicy::Preferred);
        glw->setSizePolicy(policy);
        glw->setMinimumSize(ui.canvasArea->size());
        glw->resize(ui.canvasArea->size());
    }
    canvas->ResizeEvent();

    CanvasMoveEvent();
}

void MLDemos::ClusterChanged()
{
    if(optionsCluster->optimizeCombo->currentIndex() == 3) // F1
    {
        optionsCluster->trainRatioCombo->setVisible(true);
    }
    else
    {
        optionsCluster->trainRatioCombo->setVisible(false);
    }
}

void MLDemos::AlgoChanged()
{
    if(algorithmOptions->tabClass->isVisible())
    {
        int index = optionsClassify->algoList->currentIndex();
        FOR(i, algoWidgets["classifiers"].size())
        {
            if(i==index) algoWidgets["classifiers"][i]->show();
            else algoWidgets["classifiers"][i]->hide();
        }
        if(index != -1 && index < classifiers.size()) gridSearch->SetClassifier(classifiers[index]);
    }
    if(algorithmOptions->tabClust->isVisible())
    {
        int index = optionsCluster->algoList->currentIndex();
        FOR(i, algoWidgets["clusterers"].size())
        {
            if(i==index) algoWidgets["clusterers"][i]->show();
            else algoWidgets["clusterers"][i]->hide();
        }
        if(index != -1 && index < clusterers.size()) gridSearch->SetClusterer(clusterers[index]);
    }
    if(algorithmOptions->tabDyn->isVisible())
    {
        int index = optionsDynamic->algoList->currentIndex();
        FOR(i, algoWidgets["dynamicals"].size())
        {
            if(i==index) algoWidgets["dynamicals"][i]->show();
            else algoWidgets["dynamicals"][i]->hide();
        }
        if(index != -1 && index < dynamicals.size()) gridSearch->SetDynamical(dynamicals[index]);
    }
    if(algorithmOptions->tabMax->isVisible())
    {
        int index = optionsMaximize->algoList->currentIndex();
        FOR(i, algoWidgets["maximizers"].size())
        {
            if(i==index) algoWidgets["maximizers"][i]->show();
            else algoWidgets["maximizers"][i]->hide();
        }
        if(index != -1 && index < maximizers.size()) gridSearch->SetMaximizer(maximizers[index]);
    }
    if(algorithmOptions->tabProj->isVisible())
    {
        int index = optionsProject->algoList->currentIndex();
        FOR(i, algoWidgets["projectors"].size())
        {
            if(i==index) algoWidgets["projectors"][i]->show();
            else algoWidgets["projectors"][i]->hide();
        }
        if(index != -1 && index < projectors.size()) gridSearch->SetProjector(projectors[index]);
    }
    if(algorithmOptions->tabRegr->isVisible())
    {
        int index = optionsRegress->algoList->currentIndex();
        FOR(i, algoWidgets["regressors"].size())
        {
            if(i==index) algoWidgets["regressors"][i]->show();
            else algoWidgets["regressors"][i]->hide();
        }
        if(index != -1 && index < regressors.size()) gridSearch->SetRegressor(regressors[index]);
    }
    if(algorithmOptions->tabReinf->isVisible())
    {
        int index = optionsReinforcement->algoList->currentIndex();
        FOR(i, algoWidgets["reinforcements"].size())
        {
            if(i==index) algoWidgets["reinforcements"][i]->show();
            else algoWidgets["reinforcements"][i]->hide();
        }
        if(index != -1 && index < reinforcements.size()) gridSearch->SetReinforcement(reinforcements[index]);
    }

    ChangeInfoFile();
    actionAlgorithms->setChecked(algorithmWidget->isVisible());
    if(algorithmOptions->tabMax->isVisible() || algorithmOptions->tabReinf->isVisible())
    {
        drawToolbar->paintButton->setChecked(true);
    }
    if(algorithmOptions->tabDyn->isVisible())
    {
        drawToolbar->trajectoryButton->setChecked(true);
    }
    if(algorithmOptions->tabRegr->isVisible() || algorithmOptions->tabClass->isVisible() || algorithmOptions->tabClust->isVisible() || algorithmOptions->tabProj->isVisible())
    {
        if(!drawToolbar->sprayButton->isChecked() && !drawToolbar->spray3DButton->isChecked())
        {
            drawToolbar->sprayButton->setChecked(true);
        }
    }
}

void MLDemos::CompareAdd()
{
    if(algorithmOptions->tabClass->isVisible())
    {
        int tab = optionsClassify->algoList->currentIndex();
        QString name = classifiers[tab]->GetAlgoString();
        QString parameterData;
        QTextStream stream(&parameterData, QIODevice::WriteOnly);
        stream << "Classification" << ":" << tab << "\n";
        classifiers[tab]->SaveParams(stream);
        optionsCompare->algoList->addItem(name);
        compareOptions.push_back(parameterData);
    }
    if(algorithmOptions->tabRegr->isVisible())
    {
        int tab = optionsRegress->algoList->currentIndex();
        QString name = regressors[tab]->GetAlgoString();
        QString parameterData;
        QTextStream stream(&parameterData, QIODevice::WriteOnly);
        stream << "Regression" << ":" << tab << "\n";
        regressors[tab]->SaveParams(stream);
        optionsCompare->algoList->addItem(name);
        compareOptions.push_back(parameterData);
    }
    if(algorithmOptions->tabDyn->isVisible())
    {
        int tab = optionsDynamic->algoList->currentIndex();
        QString name = dynamicals[tab]->GetAlgoString();
        QString parameterData;
        QTextStream stream(&parameterData, QIODevice::WriteOnly);
        stream << "Dynamical" << ":" << tab << "\n";
        dynamicals[tab]->SaveParams(stream);
        optionsCompare->algoList->addItem(name);
        compareOptions.push_back(parameterData);
    }
    if(algorithmOptions->tabMax->isVisible())
    {
        int tab = optionsMaximize->algoList->currentIndex();
        QString name = maximizers[tab]->GetAlgoString();
        QString parameterData;
        QTextStream stream(&parameterData, QIODevice::WriteOnly);
        stream << "Optimization" << ":" << tab << "\n";
        maximizers[tab]->SaveParams(stream);
        optionsCompare->algoList->addItem(name);
        compareOptions.push_back(parameterData);
    }
    actionCompare->setChecked(true);
    compareWidget->show();
}

void MLDemos::CompareClear()
{
    optionsCompare->algoList->clear();
    compareOptions.clear();
}

void MLDemos::CompareRemove()
{
    int offset = 0;
    FOR(i, optionsCompare->algoList->count())
    {
        if(optionsCompare->algoList->item(i)->isSelected())
        {
            compareOptions.removeAt(i-offset);
            offset++;
        }
    }

    QList<QListWidgetItem *> selected = optionsCompare->algoList->selectedItems();
    FOR(i, selected.size()) delete selected[i];
    if(optionsCompare->algoList->count()) optionsCompare->algoList->item(0)->setSelected(true);
}

void MLDemos::ShowAlgorithmOptions()
{
    if(actionAlgorithms->isChecked()) algorithmWidget->show();
    else algorithmWidget->hide();
}

void MLDemos::ShowOptionCompare()
{
    if(actionCompare->isChecked())
    {
        compareWidget->show();
    }
    else
    {
        compareWidget->hide();
    }
}

void MLDemos::ShowGridSearch()
{
    if(actionGridsearch->isChecked())
    {
        gridSearch->show();
    }
    else
    {
        gridSearch->hide();
    }
}

void MLDemos::ShowSampleDrawing()
{
    if(actionDrawSamples->isChecked())
    {
        drawToolbarWidget->show();
    }
    else
    {
        drawToolbarWidget->hide();
    }
}

void MLDemos::ShowAddData()
{
    if(actionAddData->isChecked())
    {
        generator->show();
    }
    else
    {
        generator->hide();
    }
}

void MLDemos::HideAddData()
{
    generator->hide();
    actionAddData->setChecked(false);
}

void MLDemos::ShowOptionDisplay()
{
    if(actionDisplayOptions->isChecked()) displayDialog->show();
    else displayDialog->hide();
}

void MLDemos::ShowToolbar()
{
    if(ui.actionSmall_Icons->isChecked())
    {
        toolBar->setIconSize(QSize(32,32));
        toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    }
    else
    {
        toolBar->setIconSize(QSize(64,64));
        toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    }
    if(ui.actionShow_Toolbar->isChecked()) toolBar->show();
    else toolBar->hide();
}

void MLDemos::ShowStatsDialog()
{
    if(actionShowStats->isChecked()) statsDialog->show();
    else statsDialog->hide();
}

void MLDemos::ShowAbout()
{
    aboutDialog->show();
}

void MLDemos::ManualSelection()
{
    manualSelectDialog->show();
}

void MLDemos::InputDimensions()
{
    inputDimensionsDialog->show();
}

void MLDemos::HideSampleDrawing()
{
    drawToolbarWidget->hide();
    actionDrawSamples->setChecked(false);
}

void MLDemos::HideOptionDisplay()
{
    displayDialog->hide();
    actionDisplayOptions->setChecked(false);
}

void MLDemos::HideOptionCompare()
{
    compareWidget->show();
    actionCompare->setChecked(false);
}

void MLDemos::HideToolbar()
{
    toolBar->hide();
    ui.actionShow_Toolbar->setChecked(false);
}

void MLDemos::HideStatsDialog()
{
    statsDialog->hide();
    actionShowStats->setChecked(false);
}

void MLDemos::AddData()
{
    ClearData();
    pair<vector<fvec>,ivec> newData = generator->Generate();
    canvas->data->AddSamples(newData.first, newData.second);
    ResetPositiveClass();
    FitToData();
}

void MLDemos::Clear()
{
    drawTimer->Stop();
    drawTimer->Clear();
    QMutexLocker lock(&mutex);
    qApp->processEvents();
    if(!classifierMulti.size()) DEL(classifier);
    classifier = 0;
    sourceDims.clear();
    FOR(i,classifierMulti.size()) DEL(classifierMulti[i]); classifierMulti.clear();
    DEL(regressor);
    DEL(dynamical);
    DEL(clusterer);
    DEL(maximizer);
    DEL(reinforcement);
    DEL(projector);
    sourceDims.clear();
    glw->clearLists();
    canvas->maps.confidence = QPixmap();
    canvas->maps.model = QPixmap();
    canvas->maps.info = QPixmap();
    canvas->liveTrajectory.clear();
    canvas->sampleColors.clear();
    canvas->maps.animation = QPixmap();
    canvas->repaint();
    UpdateInfo();
}

void MLDemos::ResetPositiveClass()
{
    int labMin = INT_MAX, labMax = INT_MIN;
    if(!canvas->data->GetCount())
    {
        labMin = 0;
        labMax = 1;
        optionsClassify->positiveSpin->setValue(1);
    }
    else
    {
        ivec labels = canvas->data->GetLabels();
        FOR(i, labels.size())
        {
            if(labels[i] > labMax) labMax = labels[i];
            if(labels[i] < labMin) labMin = labels[i];
        }
    }
    if(labMin == labMax) labMax = labMin+1;
    int dimCount = max(2,canvas->data->GetDimCount());
    int currentOutputDim = optionsCompare->outputDimCombo->currentIndex();

    optionsCompare->outputDimCombo->clear();
    optionsRegress->outputDimCombo->clear();
    FOR(i, dimCount)
    {
        if(i < canvas->dimNames.size())
        {
            optionsCompare->outputDimCombo->addItem(QString("%1) %2").arg(i+1).arg(canvas->dimNames.at(i)));
            optionsRegress->outputDimCombo->addItem(QString("%1) %2").arg(i+1).arg(canvas->dimNames.at(i)));
        }
        else
        {
            optionsCompare->outputDimCombo->addItem(QString("%1").arg(i+1));
            optionsRegress->outputDimCombo->addItem(QString("%1").arg(i+1));
        }
    }
    if(currentOutputDim < dimCount) optionsCompare->outputDimCombo->setCurrentIndex(currentOutputDim);

    int currentPositive = optionsClassify->positiveSpin->value();
    optionsClassify->positiveSpin->setRange(labMin,labMax);
    if(currentPositive < labMin)
        optionsClassify->positiveSpin->setValue(labMin);
    else if(currentPositive > labMax)
        optionsClassify->positiveSpin->setValue(labMax);
    ui.canvasX1Spin->setRange(1,dimCount);
    ui.canvasX2Spin->setRange(1,dimCount);
    ui.canvasX3Spin->setRange(0,dimCount);
    canvas->SetDim(ui.canvasX1Spin->value()-1,ui.canvasX2Spin->value()-1, ui.canvasX3Spin->value()-1);
    dataEdit->Update();
    ManualSelectionUpdated();
    InputDimensionsUpdated();
}

void MLDemos::ChangeActiveOptions()
{
    DisplayOptionsChanged();
}

void MLDemos::ClearData()
{
    sourceData.clear();
    sourceLabels.clear();
    projectedData.clear();
    if(canvas)
    {
        canvas->sampleColors.clear();
        canvas->data->Clear();
        canvas->maps.model = QPixmap();
    }
    UpdateInfo();
    canvas->repaint();
}

void MLDemos::ClearAll()
{
    sourceData.clear();
    sourceLabels.clear();
    projectedData.clear();
    if(canvas)
    {
        canvas->dimNames.clear();
        canvas->classNames.clear();
        canvas->sampleColors.clear();
        canvas->data->Clear();
        canvas->targets.clear();
        canvas->targetAge.clear();
        canvas->maps.animation = QPixmap();
        canvas->maps.reward = QPixmap();
        canvas->maps.samples = QPixmap();
        canvas->maps.trajectories = QPixmap();
        canvas->maps.grid = QPixmap();
    }
    Clear();
    ResetPositiveClass();
    FitToData();
    UpdateInfo();
}

void MLDemos::DataEdited()
{
    canvas->ResetSamples();
    FitToData();
    ResetPositiveClass();
    UpdateInfo();
    canvas->repaint();
}

void MLDemos::ShowDataEditor()
{
    dataEdit->show();
}

void MLDemos::AvoidOptionChanged()
{
    if(dynamical)
    {
        int avoidIndex = optionsDynamic->obstacleCombo->currentIndex();
        mutex.lock();
        if(dynamical->avoid) delete dynamical->avoid;
        if(!avoidIndex) dynamical->avoid = 0;
        else dynamical->avoid = avoiders[avoidIndex-1]->GetObstacleAvoidance();
        mutex.unlock();
        drawTimer->Stop();
        drawTimer->Clear();
        drawTimer->start(QThread::NormalPriority);
    }
}

void MLDemos::ColorMapChanged()
{
    if(dynamical)
    {
        drawTimer->Stop();
        drawTimer->Clear();
        drawTimer->bColorMap = optionsDynamic->colorCheck->isChecked();
        drawTimer->start(QThread::NormalPriority);
    }
}

void MLDemos::DisplayOptionsChanged()
{
    if(!canvas) return;

    canvas->bDisplayInfo = displayOptions->infoCheck->isChecked();
    canvas->bDisplayLearned = displayOptions->modelCheck->isChecked();
    canvas->bDisplayMap = displayOptions->mapCheck->isChecked();
    canvas->bDisplaySamples = displayOptions->samplesCheck->isChecked();
    canvas->bDisplayTrajectories = displayOptions->samplesCheck->isChecked();
    canvas->bDisplayTimeSeries = displayOptions->samplesCheck->isChecked();
    canvas->bDisplayGrid = displayOptions->gridCheck->isChecked();
    canvas->bDisplayLegend = displayOptions->legendCheck->isChecked();
    {
        int xIndex = ui.canvasX1Spin->value()-1;
        int yIndex = ui.canvasX2Spin->value()-1;
        int zIndex = ui.canvasX3Spin->value()-1;
        canvas->SetDim(xIndex, yIndex, zIndex);
    }
    float zoom = displayOptions->spinZoom->value();
    if(zoom >= 0.f) zoom += 1.f;
    else zoom = 1.f / (fabs(zoom)+1.f);
    if(zoom != canvas->GetZoom())
    {
        drawTimer->Stop();
        drawTimer->Clear();
        canvas->SetZoom(zoom);
        if(mutex.tryLock())
        {
            if(!canvas->canvasType)
            {
                if(classifier)
                {
                    classifiers[tabUsedForTraining]->Draw(canvas, classifier);
                    DrawClassifiedSamples(canvas, classifier, classifierMulti);
                    if(classifier->UsesDrawTimer())
                    {
                        drawTimer->start(QThread::NormalPriority);
                    }
                }
                else if(regressor)
                {
                    regressors[tabUsedForTraining]->Draw(canvas, regressor);
                    //drawTimer->start(QThread::NormalPriority);
                }
                else if(clusterer)
                {
                    clusterers[tabUsedForTraining]->Draw(canvas, clusterer);
                }
                else if(dynamical)
                {
                    dynamicals[tabUsedForTraining]->Draw(canvas, dynamical);
                    if(dynamicals[tabUsedForTraining]->UsesDrawTimer()) drawTimer->start(QThread::NormalPriority);
                }
                else if(maximizer)
                {
                    drawTimer->start(QThread::NormalPriority);
                }
            }
            if(projector)
            {
                projectors[tabUsedForTraining]->Draw(canvas, projector);
            }
            mutex.unlock();
        }
        canvas->repaint();
    }
    //	canvas->bDisplayTrajectories = displayOptions->trajectoriesCheck->isChecked();
    if(optionsDynamic)
    {
        canvas->trajectoryCenterType = optionsDynamic->centerCombo->currentIndex();
        canvas->trajectoryResampleType = optionsDynamic->resampleCombo->currentIndex();
        canvas->trajectoryResampleCount = optionsDynamic->resampleSpin->value();
    }
    CanvasTypeChanged();
    CanvasOptionsChanged();
    canvas->ResetSamples();
    canvas->repaint();
}

void MLDemos::Display3DOptionsChanged()
{
    if(canvas->canvasType == 0)
    {
        if(canvas->bDisplayGrid != displayOptions->gridCheck->isChecked())
        {
            canvas->bDisplayGrid = displayOptions->gridCheck->isChecked();
            canvas->repaint();
        }
    }
    else canvas->bDisplayGrid = displayOptions->gridCheck->isChecked();
    if(!glw) return;
    glw->bDisplaySamples = displayOptions->check3DSamples->isChecked();
    glw->bDisplayLines = displayOptions->check3DWireframe->isChecked();
    glw->bDisplaySurfaces = displayOptions->check3DSurfaces->isChecked();
    glw->bDisplayTransparency = displayOptions->check3DTransparency->isChecked();
    glw->bDisplayBlurry = displayOptions->check3DBlurry->isChecked();
    glw->bRotateCamera = displayOptions->check3DRotate->isChecked();
    glw->update();
}

void MLDemos::ChangeInfoFile()
{
    QString infoFile, mainFile;
    if(algorithmOptions->tabClass->isVisible())
    {
        mainFile = "classification.html";
        int tab = optionsClassify->algoList->currentIndex();
        if(tab < 0 || tab >= (int)classifiers.size() || !classifiers[tab]) return;
        infoFile = classifiers[tab]->GetInfoFile();
    }
    if(algorithmOptions->tabClust->isVisible())
    {
        mainFile = "clustering.html";
        int tab = optionsCluster->algoList->currentIndex();
        if(tab < 0 || tab >= (int)clusterers.size() || !clusterers[tab]) return;
        infoFile = clusterers[tab]->GetInfoFile();
    }
    if(algorithmOptions->tabRegr->isVisible())
    {
        mainFile = "regression.html";
        int tab = optionsRegress->algoList->currentIndex();
        if(tab < 0 || tab >= (int)regressors.size() || !regressors[tab]) return;
        infoFile = regressors[tab]->GetInfoFile();
    }
    if(algorithmOptions->tabDyn->isVisible())
    {
        mainFile = "dynamical.html";
        int tab = optionsDynamic->algoList->currentIndex();
        if(tab < 0 || tab >= (int)dynamicals.size() || !dynamicals[tab]) return;
        infoFile = dynamicals[tab]->GetInfoFile();
    }
    if(algorithmOptions->tabMax->isVisible())
    {
        mainFile = "maximization.html";
        int tab = optionsMaximize->algoList->currentIndex();
        if(tab < 0 || tab >= (int)maximizers.size() || !maximizers[tab]) return;
        infoFile = maximizers[tab]->GetInfoFile();
    }
    if(algorithmOptions->tabProj->isVisible())
    {
        mainFile = "projection.html";
        int tab = optionsProject->algoList->currentIndex();
        if(tab < 0 || tab >= (int)projectors.size() || !projectors[tab]) return;
        infoFile = projectors[tab]->GetInfoFile();
    }
    if(algorithmOptions->tabReinf->isVisible())
    {
        mainFile = "reinforcement.html";
        int tab = optionsReinforcement->algoList->currentIndex();
        if(tab < 0 || tab >= (int)reinforcements.size() || !reinforcements[tab]) return;
        infoFile = reinforcements[tab]->GetInfoFile();
    }
    if(mainFile == "") mainFile = "mldemos.html"; // we want the main information page

    QDir helpDir = QDir(qApp->applicationDirPath());
    QDir alternativeDir = helpDir;
#if defined(Q_OS_WIN)
    if (helpDir.dirName().toLower() == "debug" || helpDir.dirName().toLower() == "release") helpDir.cdUp();
#elif defined(Q_OS_MAC)
    if (helpDir.dirName() == "MacOS") {
        if(!helpDir.cd("help"))
        {
            helpDir.cdUp();
            helpDir.cdUp();
            helpDir.cdUp();
            alternativeDir = helpDir;
        }
        else helpDir.cdUp();
    }
#endif
    if(!helpDir.cd("help"))
    {
        //qDebug() << "using alternative directory: " << alternativeDir.absolutePath();
        helpDir = alternativeDir;
        if(!helpDir.cd("help")) return;
    }
    //qDebug() << "using help directory: " << helpDir.absolutePath();

    showStats->helpAlgoText->clear();
    if(!infoFile.isEmpty())
    {
        QString filePath(helpDir.absolutePath() + "/" + infoFile);
        if(QFile::exists(filePath))
        {
            showStats->helpAlgoText->setSource(QUrl::fromLocalFile(filePath));
        }
        else
        {
            showStats->helpAlgoText->setText("No information available.");
        }
    }

    showStats->helpMainText->clear();
    QString filePath2(helpDir.absolutePath() + "/" + mainFile);
    if(QFile::exists(filePath2))
    {
        showStats->helpMainText->setSource(QUrl::fromLocalFile(filePath2));
    }
    else
    {
        showStats->helpMainText->setText("No information available.");
    }
}

void MLDemos::ManualSelectionUpdated()
{
    if(!canvas) return;
    // we add the samples
    vector<fvec> samples = canvas->data->GetSamples();
    int dim = samples.size() ? samples[0].size() : 0;
    ivec labels = canvas->data->GetLabels();
    manualSelection->sampleList->clear();
    FOR(i, samples.size())
    {
        QString item = QString("%1: (%2)").arg(i+1).arg(labels[i]);
        FOR(d, dim) item += QString(" %1").arg(samples[i][d], 0, 'f', 2);
        manualSelection->sampleList->addItem(item);
    }
    ManualSelectionChanged();
}

void MLDemos::ManualSelectionChanged()
{
    int count = manualSelection->sampleList->count();
    int trainCount = count, testCount = 0;
    QList<QListWidgetItem*> selected = manualSelection->sampleList->selectedItems();
    if(selected.size())
    {
        trainCount = selected.size();
        testCount = count - trainCount;
    }
    manualSelection->TrainLabel->setText(QString("Train: %1").arg(trainCount));
    manualSelection->TestLabel->setText(QString("Test: %1").arg(testCount));
}

void MLDemos::ManualSelectionClear()
{
    manualSelection->sampleList->clearSelection();
    ManualSelectionChanged();
}

void MLDemos::ManualSelectionInvert()
{
    FOR(i, manualSelection->sampleList->count())
    {
        manualSelection->sampleList->item(i)->setSelected(!manualSelection->sampleList->item(i)->isSelected());
    }
    ManualSelectionChanged();
}

void MLDemos::ManualSelectionRandom()
{
    float ratio = (manualSelection->randomCombo->currentIndex()+1.f)/10.f;
    manualSelection->sampleList->clearSelection();
    u32* perm = randPerm(manualSelection->sampleList->count());
    FOR(i, ratio*manualSelection->sampleList->count())
    {
        manualSelection->sampleList->item(perm[i])->setSelected(true);
    }
    KILL(perm);
    ManualSelectionChanged();
}

void MLDemos::ManualSelectionRemove()
{
    if(!canvas || !canvas->data->GetCount()) return;
    QList<QListWidgetItem*> selected = manualSelection->sampleList->selectedItems();
    if(!selected.size()) return;
    ivec removeList(selected.count());
    FOR(i, selected.count())
    {
        removeList[i] = manualSelection->sampleList->row(selected[i]);
    }
    canvas->data->RemoveSamples(removeList);
    if(canvas->sampleColors.size() && removeList.size() < canvas->sampleColors.size())
    {
        int offset = 0;
        FOR(i, removeList.size())
        {
            int index = i - offset;
            if(index < 0 || index >= canvas->sampleColors.size()) continue;
            canvas->sampleColors.erase(canvas->sampleColors.begin()+index);
            offset++;
        }
    }
    ManualSelectionUpdated();
    ManualSelectionChanged();
    canvas->ResetSamples();
    CanvasOptionsChanged();
    canvas->repaint();
}

void MLDemos::InputDimensionsUpdated()
{
    if(!canvas) return;
    int dim = canvas->data->GetDimCount();
    inputDimensions->dimList->clear();
    fvec xMin(dim,FLT_MAX), xMax(dim,-FLT_MAX);
    vector<fvec> samples = canvas->data->GetSamples();
    FOR(i, samples.size())
    {
        FOR(d, dim)
        {
            xMin[d] = min(xMin[d], samples[i][d]);
            xMax[d] = max(xMax[d], samples[i][d]);
        }
    }
    FOR(d, dim)
    {
        QString item = QString("%1").arg(d+1);
        if(d < canvas->dimNames.size()) item += QString(") %2").arg(canvas->dimNames.at(d));
        item += QString(" : [%1 --> %2]").arg(xMin[d], 0, 'f', 3).arg(xMax[d], 0, 'f', 3);
        inputDimensions->dimList->addItem(item);
    }
    ManualSelectionChanged();
    vis->UpdateDims();
}

void MLDemos::InputDimensionsChanged()
{
    int count = inputDimensions->dimList->count();
    QList<QListWidgetItem*> selected = inputDimensions->dimList->selectedItems();
    inputDimensions->TrainLabel->setText(QString("Used: %1").arg(selected.size()));
    inputDimensions->TestLabel->setText(QString("Unused: %1").arg(count-selected.size()));
}

void MLDemos::InputDimensionsClear()
{
    inputDimensions->dimList->clearSelection();
    ManualSelectionChanged();
}

void MLDemos::InputDimensionsInvert()
{
    FOR(i, inputDimensions->dimList->count())
    {
        inputDimensions->dimList->item(i)->setSelected(!inputDimensions->dimList->item(i)->isSelected());
    }
    ManualSelectionChanged();
}

void MLDemos::InputDimensionsRandom()
{
    float ratio = (inputDimensions->randomCombo->currentIndex()+1.f)/10.f;
    inputDimensions->dimList->clearSelection();
    u32* perm = randPerm(inputDimensions->dimList->count());
    FOR(i, ratio*inputDimensions->dimList->count())
    {
        inputDimensions->dimList->item(perm[i])->setSelected(true);
    }
    KILL(perm);
    ManualSelectionChanged();
}

void MLDemos::DrawCrosshair()
{
    int drawType = 0;
    if(drawToolbar->tabWidget->currentIndex() == 0) // drawingTab
    {
        if(drawToolbar->singleButton->isChecked()) drawType = 1;
        if(drawToolbar->sprayButton->isChecked()) drawType = 2;
        if(drawToolbar->spray3DButton->isChecked()) drawType = 9;
        if(drawToolbar->trajectoryButton->isChecked()) drawType = 6;
        if(drawToolbar->obstacleButton->isChecked()) drawType = 7;
        if(drawToolbar->paintButton->isChecked()) drawType = 8;
        if(drawToolbar->eraseButton->isChecked()) drawType = 3;
    }
    else if(drawToolbar->tabWidget->currentIndex() == 1) // editTab
    {
        if(drawToolbar->dragButton->isChecked()) drawType = -1;
        if(drawToolbar->moveButton->isChecked()) drawType = -2;
        if(drawToolbar->moveClassButton->isChecked()) drawType = -3;
        if(drawToolbar->extrudeButton->isChecked()) drawType = -4;
        if(drawToolbar->sprayClassButton->isChecked()) drawType = -5;
    }
    else if(drawToolbar->tabWidget->currentIndex() == 2)
    {
        if(drawToolbar->ellipseButton->isChecked()) drawType = 4;
        if(drawToolbar->lineButton->isChecked()) drawType = 5;
    }

    if(!drawType || drawType == 1 || drawType == 6 || drawType == -1 || drawType == -3)
    {
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

    switch(drawType)
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
    for(float theta=0; theta < 2*PIf + 0.1; theta += 0.1f)
    {
        float X, Y;
        if(drawType == 2 || drawType == 3)
        {
            X = sqrtf(aX)/2 * cosf(theta);
            Y = sqrtf(aY)/2 * sinf(theta);
        }
        else
        {
            X = aX * cosf(theta);
            Y = aY * sinf(theta);
        }

        float RX = + X * cos_angle + Y * sin_angle;
        float RY = - X * sin_angle + Y * cos_angle;

        point = QPointF(RX*Size,RY*Size);
        if(theta==0)
        {
            cursor.moveTo(point);
            continue;
        }
        cursor.lineTo(point);
        oldPoint = point;
    }
    canvas->crosshair = cursor;
    canvas->bNewCrosshair = false;
}

void MLDemos::Drawing( fvec sample, int label)
{
    if(canvas->canvasType) return;
    int drawType = 0; // none
    if(drawToolbar->tabWidget->currentIndex() == 0) // drawingTab
    {
        if(drawToolbar->singleButton->isChecked()) drawType = 1;
        if(drawToolbar->sprayButton->isChecked()) drawType = 2;
        if(drawToolbar->spray3DButton->isChecked()) drawType = 9;
        if(drawToolbar->trajectoryButton->isChecked()) drawType = 6;
        if(drawToolbar->obstacleButton->isChecked()) drawType = 7;
        if(drawToolbar->paintButton->isChecked()) drawType = 8;
        if(drawToolbar->eraseButton->isChecked()) drawType = 3;
    }
    else if(drawToolbar->tabWidget->currentIndex() == 1) // editTab
    {
        if(drawToolbar->dragButton->isChecked()) drawType = -1;
        if(drawToolbar->moveButton->isChecked()) drawType = -2;
        if(drawToolbar->moveClassButton->isChecked()) drawType = -3;
        if(drawToolbar->extrudeButton->isChecked()) drawType = -4;
        if(drawToolbar->sprayClassButton->isChecked()) drawType = -5;
    }
    else if(drawToolbar->tabWidget->currentIndex() == 2)
    {
        if(drawToolbar->ellipseButton->isChecked()) drawType = 4;
        if(drawToolbar->lineButton->isChecked()) drawType = 5;
    }
    if(!drawType) return;

    if(drawType < 0)
    {
        Editing(-drawType, sample, label);
        return;
    }

    int speed = 6;
    bool bEmpty = canvas->data->GetCount() == 0;

    if(label) label = drawToolbar->classSpin->value();

    switch(drawType)
    {
    case 1: // single samples
    {
        // we don't want to draw too often
        if(drawTime.elapsed() < 50/speed) return; // msec elapsed since last drawing
        canvas->data->AddSample(sample, label);
        if(!selectedData.size()) selectedData.push_back(0);
    }
        break;
    case 2: // spray samples
    case 9: // spray 3D samples
    {
        // we don't want to draw too often
        if(drawTime.elapsed() < 200/speed) return; // msec elapsed since last drawing
        int type = drawToolbarContext1->randCombo->currentIndex();
        float s = drawToolbar->radiusSpin->value();
        float size = s*canvas->height();
        int count = drawToolbarContext1->spinCount->value();

        QPointF sampleCoords = canvas->toCanvasCoords(sample);
        // we generate the new data
        float variance = sqrtf(size*size/9.f*0.5f);
        int dim = canvas->data->GetDimCount();
        if(drawType == 9) dim = 3;
        int xIndex = canvas->xIndex;
        int yIndex = canvas->yIndex;
        float radius = 1.f;
        if(dim > 2)
        {
            fvec s1 = canvas->toSampleCoords(sampleCoords.x() - size, sampleCoords.y() - size);
            fvec s2 = canvas->toSampleCoords(sampleCoords.x() + size, sampleCoords.y() + size);
            radius = sqrtf((s1-s2)*(s1-s2))/2;
        }
        fvec newSample(2,0);
        FOR(i, count)
        {

            if(type == 0) // uniform
            {
                newSample[0] = (rand()/(float)RAND_MAX - 0.5f)*size + sampleCoords.x();
                newSample[1] = (rand()/(float)RAND_MAX - 0.5f)*size + sampleCoords.y();
            }
            else // normal
            {
                newSample[0] = RandN((float)sampleCoords.x(), variance);
                newSample[1] = RandN((float)sampleCoords.y(), variance);
            }
            fvec canvasSample = canvas->toSampleCoords(newSample[0],newSample[1]);
            while(canvasSample.size() < dim) canvasSample.push_back(0);
            if(dim > 2)
            {
                FOR(d, dim)
                {
                    if(d == xIndex || d == yIndex) continue;
                    canvasSample[d] = (drand48()-0.5f)*radius;
                }
            }
            //qDebug() << "added sample " << canvasSample.size() << canvas->data->GetDimCount() << ":" << canvasSample[0] << canvasSample[1] << canvasSample[2];
            //canvasSample.push_back(label ? RandN(1.f,0.5f) : RandN(-1.f,0.5f));
            //canvasSample.push_back(label ? RandN(-.5f,0.5f) : RandN(.5f,0.5f));
            canvas->data->AddSample(canvasSample, label);
            if(bEmpty)
            {
                if(canvas->zooms.size() != dim)
                {
                    while(canvas->zooms.size() < dim) canvas->zooms.push_back(1.f);
                    while(canvas->center.size() < dim) canvas->center.push_back(0.f);
                }
            }
            if(!selectedData.size()) selectedData.push_back(0);
        }
    }
        break;
    case 3: // erase
    {
        float s = drawToolbar->radiusSpin->value();
        float size = s*canvas->height();
        QPointF center = canvas->toCanvasCoords(sample);
        bool anythingDeleted = canvas->DeleteData(center, size/2);
        if(anythingDeleted)
        {
            drawTimer->Stop();
            drawTimer->Clear();
            QMutexLocker lock(&mutex);
            if(dynamical && dynamical->avoid) dynamical->avoid->SetObstacles(canvas->data->GetObstacles());
            drawTimer->start(QThread::NormalPriority);
            canvas->ResetSamples();
        }
    }
        break;
    case 4: // ellipse
    {
        if(drawTime.elapsed() < 200/speed) return; // msec elapsed since last drawing
        float aX = drawToolbarContext2->spinSigmaX->value();
        float aY = drawToolbarContext2->spinSigmaY->value();
        float angle = -drawToolbarContext2->spinAngle->value()/180.f*PIf;
        int count = drawToolbarContext1->spinCount->value()+1;
        float sin_angle = sinf(angle);
        float cos_angle = cosf(angle);

        QPointF oldPoint, point;
        float startTheta = rand()/(float)RAND_MAX*2*PIf;
        for(float theta=0; theta < 2*PIf; theta += 2.f*PIf/count)
        {
            float X = aX * cosf(theta+startTheta);
            float Y = aY * sinf(theta+startTheta);

            float RX = + X * cos_angle + Y * sin_angle;
            float RY = - X * sin_angle + Y * cos_angle;

            fvec newSample;
            newSample.resize(2,0);
            newSample[0] = sample[0] + RX;
            newSample[1] = sample[1] + RY;
            if(theta==0)
            {
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
        if(drawTime.elapsed() < 200/speed) return; // msec elapsed since last drawing
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
        FOR(i,count)
        {
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
        if(trajectory.first == -1) // we're starting a trajectory
        {
            trajectory.first = canvas->data->GetCount();
        }
        // we don't want to draw too often
        //if(drawTime.elapsed() < 50/speed) return; // msec elapsed since last drawing
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
        /*
      // if we need to initialize the reward map
      if(!canvas->data->GetReward()->rewards)
      {
       ivec size;
       size.resize(2, 64);
       int length = size[0]*size[1];
       float *values = new float[length];
       FOR(i, length) values[i] = rand()/(float)RAND_MAX - 0.5f;
       canvas->data->GetReward()->SetReward(values, size, canvas->canvasTopLeft(), canvas->canvasBottomRight());
       delete [] values;
      }
      canvas->data->GetReward()->ShiftValueAt(sample, 0.2, label ? 0.33 : -0.33);
      //qDebug() << canvas->data->GetReward()->ValueAt(sample);
      */
    }
        break;
    }
    canvas->repaint();
    drawTime.restart();
    ResetPositiveClass();
    if(bEmpty)
    {
        optionsCompare->outputDimCombo->setCurrentIndex(canvas->data->GetDimCount()-1);
        optionsRegress->outputDimCombo->setCurrentIndex(canvas->data->GetDimCount()-1);
    }
    UpdateInfo();
}

void MLDemos::Editing(int editType, fvec position, int label)
{
    if(!selectedData.size()) // we're beginning our editing
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
            if(closestSample.size())
            {
                int closest = closestSample[0];
                int closestLabel = canvas->data->GetLabel(closest);
                selectedData.clear();
                selectionWeights.clear();
                FOR(i, canvas->data->GetCount())
                {
                    if(canvas->data->GetLabel(i) != closestLabel) continue;
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
            FOR(i, selectedData.size()) selectedData[i] = i;
            // we need to generate a fixed "spacing" so that we can 'scale' the samples
            FOR(i, selectedData.size()) selectionWeights[i] = canvas->data->GetSample(i)[yIndex] + drand48()*2.f - 1.f;
        }
            break;
        case 5: // change sample labels
        {
            int newLabel = drawToolbar->classSpin->value();
            selectedData = canvas->SelectSamples(center, radius);
            FOR(i, selectedData.size())
            {
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
        FOR(i, selectedData.size())
        {
            fvec sample = canvas->data->GetSample(selectedData[i]);
            float weight = 1.f;
            if(selectionWeights.size())
            {
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
        FOR(i, selectedData.size())
        {
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
    if(selectedData.size() || (canvas->sampleColors.size() && canvas->sampleColors.size() != canvas->data->GetCount()))
    {
        if(classifier) DrawClassifiedSamples(canvas, classifier, classifierMulti);
        if(clusterer) clusterers[tabUsedForTraining]->Draw(canvas, clusterer);
        if(regressor) regressors[tabUsedForTraining]->Draw(canvas, regressor);
        if(dynamical) dynamicals[tabUsedForTraining]->Draw(canvas, dynamical);
    }
    selectedData.clear();
    selectionStart = fvec();

    if(trajectory.first != -1)
    {
        // the last point is a duplicate, we take it out
        canvas->data->AddSequence(trajectory);
        canvas->drawnTrajectories = 0;
        trajectory.first = -1;
        canvas->repaint();
    }
    if(bNewObstacle)
    {
        bNewObstacle = false;
        canvas->data->AddObstacle(obstacle);
        canvas->repaint();
        if(dynamical && dynamical->avoid)
        {
            drawTimer->Stop();
            drawTimer->Clear();
            drawTimer->start(QThread::NormalPriority);
        }
    }
}

void MLDemos::DimPlus()
{
    if(!canvas->data->GetCount()) return;
    vector<fvec> samples = canvas->data->GetSamples();
    int dim = samples[0].size()+1;
    FOR(i, samples.size())
    {
        samples[i].push_back(0);
    }
    canvas->data->SetSamples(samples);
    canvas->zooms.push_back(1.f);
    canvas->center.push_back(0.f);
    canvas->ResetSamples();
    ResetPositiveClass();
    Clear();
    if(canvas->canvasType == 0) ui.canvasX2Spin->setValue(dim);
    canvas->repaint();
}

void MLDemos::DimLess()
{
    if(!canvas->data->GetCount()) return;
    vector<fvec> samples = canvas->data->GetSamples();
    int dim = samples[0].size();
    switch(dim)
    {
    case 1:
        break;
    case 2:
    {
        FOR(i, samples.size()) samples[i][1] = 0;
        canvas->data->SetSamples(samples);
        canvas->center[1] = 0;
        canvas->zooms[1] = 1.f;
    }
        break;
    default:
    {
        FOR(i, samples.size()) samples[i].pop_back();
        canvas->data->SetSamples(samples);
        canvas->zooms.pop_back();
        canvas->center.pop_back();
        if(ui.canvasX2Spin->value() == dim) ui.canvasX2Spin->setValue(dim-1);
    }
        break;
    }
    canvas->ResetSamples();
    ResetPositiveClass();
    Clear();
    canvas->repaint();
}

void MLDemos::FitToData()
{
    canvas->FitToData();
    float zoom = canvas->GetZoom();
    if(zoom >= 1) zoom -=1;
    else zoom = 1/(-zoom) - 1;
    if(zoom == displayOptions->spinZoom->value())
    {
        DisplayOptionsChanged();
        return;
    }
    displayOptions->spinZoom->blockSignals(true);
    displayOptions->spinZoom->setValue(zoom);
    displayOptions->spinZoom->blockSignals(false);
    drawTimer->Stop();
    drawTimer->Clear();
    if(!canvas->canvasType)
    {
        QMutexLocker lock(&mutex);
        if(classifier)
        {
            classifiers[tabUsedForTraining]->Draw(canvas, classifier);
            DrawClassifiedSamples(canvas, classifier, classifierMulti);
            if(classifier->UsesDrawTimer()) drawTimer->start(QThread::NormalPriority);
        }
        else if(regressor)
        {
            regressors[tabUsedForTraining]->Draw(canvas, regressor);
            //drawTimer->start(QThread::NormalPriority);
        }
        else if(clusterer)
        {
            clusterers[tabUsedForTraining]->Draw(canvas, clusterer);
            drawTimer->start(QThread::NormalPriority);
        }
        else if(dynamical)
        {
            dynamicals[tabUsedForTraining]->Draw(canvas, dynamical);
            if(dynamicals[tabUsedForTraining]->UsesDrawTimer()) drawTimer->start(QThread::NormalPriority);
        }
        else if(projector)
        {
            projectors[tabUsedForTraining]->Draw(canvas, projector);
        }
    }
    DisplayOptionsChanged();
}

void MLDemos::CanvasMoveEvent()
{
    if(canvas->canvasType) return;
    drawTimer->Stop();
    drawTimer->Clear();
    UpdateLearnedModel();

    QMutexLocker lock(&mutex);
    if(canvas->canvasType != 1)
    {
        if(classifier)
        {
            classifiers[tabUsedForTraining]->Draw(canvas, classifier);
            DrawClassifiedSamples(canvas, classifier, classifierMulti);
            if(classifier->UsesDrawTimer())
            {
                drawTimer->start(QThread::NormalPriority);
            }
        }
        else if(regressor)
        {
            regressors[tabUsedForTraining]->Draw(canvas, regressor);
            //drawTimer->start(QThread::NormalPriority);
        }
        else if(clusterer)
        {
            clusterers[tabUsedForTraining]->Draw(canvas, clusterer);
            drawTimer->start(QThread::NormalPriority);
        }
        else if(dynamical)
        {
            dynamicals[tabUsedForTraining]->Draw(canvas, dynamical);
            if(dynamicals[tabUsedForTraining]->UsesDrawTimer()) drawTimer->start(QThread::NormalPriority);
        }
        else if(projector)
        {
            projectors[tabUsedForTraining]->Draw(canvas, projector);
        }
    }
    canvas->repaint();
}

void MLDemos::ZoomChanged(float d)
{
    displayOptions->spinZoom->setValue(displayOptions->spinZoom->value()+d/4);
}

void MLDemos::CanvasTypeChanged()
{
    bool bProjected = canvas->data->bProjected;
    int type = ui.canvasTypeCombo->currentIndex();

    if(!canvas->data->GetCount())
    {
        if(type > 1 && !canvas->rewardPixmap().isNull()) // we only have rewards
        {
            ui.canvasTypeCombo->setCurrentIndex(0);
            return;
        }
    }

    ui.canvasAxesWidget->hide();
    switch(type)
    {
    case 0: // standard
        ui.canvasAxesWidget->show();
        ui.canvasX3Spin->setEnabled(false);
        ui.canvasX1Label->setText(bProjected ? "e1" : "x1");
        ui.canvasX2Label->setText(bProjected ? "e2" : "x2");
        break;
    case 1: // 3D viewport
    {
        ui.canvasAxesWidget->show();
        ui.canvasX3Spin->setEnabled(true);
        ui.canvasX1Label->setText(bProjected ? "e1" : "x1");
        ui.canvasX2Label->setText(bProjected ? "e2" : "x2");
        ui.canvasX3Label->setText(bProjected ? "e3" : "x3");
        // if we haven't really set the third dimension, we pick the one manually
        if(canvas->data->GetDimCount() > 2 &&
                ui.canvasX3Spin->value() == 0 ||
                ui.canvasX3Spin->value() == ui.canvasX2Spin->value())
            ui.canvasX3Spin->setValue(ui.canvasX2Spin->value() == canvas->data->GetDimCount() ?
                                          canvas->data->GetDimCount()-1 : ui.canvasX2Spin->value()+1);
        if(regressor && canvas->data->GetDimCount() > 2 &&
                ui.canvasX2Spin->value() == optionsRegress->outputDimCombo->currentIndex()+1)
        {
            ui.canvasX2Spin->setValue(ui.canvasX1Spin->value()+1);
            ui.canvasX3Spin->setValue(ui.canvasX2Spin->value()+1);
        }
        if(canvas->data->GetDimCount() <= 2) ui.canvasX3Spin->setValue(0);
    }
        break;
    case 2: // Visualizations
        break;
    }
    if ((!glw || !glw->isVisible()) && (!vis || !vis->isVisible()) && canvas->canvasType == type) return;
    if(type == 1) // 3D viewport
    {
        vis->hide();
        displayOptions->tabWidget->setCurrentIndex(1);
        canvas->Clear();
        canvas->repaint();
        ui.canvasWidget->repaint();
        //        ui.canvasWidget->hide();
        if(!ui.canvasArea->layout())
        {
            // this is an ugly hack that forces the layout behind to be drawn as a cleared image
            // for some reason otherwise, the canvas will only be repainted AFTER it is re-shown
            // and it will flicker with the last image shown beforehand
            QBoxLayout *layout = new QBoxLayout(QBoxLayout::LeftToRight);
            layout->setContentsMargins(1,1,1,1);
            layout->setMargin(1);
            ui.canvasArea->setLayout(layout);
            ui.canvasArea->layout()->addWidget(glw);
            ui.canvasArea->layout()->addWidget(vis);
        }
        QSizePolicy policy = glw->sizePolicy();
        policy.setHorizontalPolicy(QSizePolicy::Preferred);
        policy.setVerticalPolicy(QSizePolicy::Preferred);
        canvas->SetCanvasType(type);
        glw->setSizePolicy(policy);
        glw->setMinimumSize(ui.canvasArea->size());
        glw->resize(ui.canvasArea->size());
        FOR(i, glw->objects.size())
        {
            if(!glw->objectAlive[i]) continue;
            if(glw->objects[i].objectType.contains("Reward"))
            {
                glw->killList.push_back(i);
                //glw->objects.erase(glw->objects.begin() + i);
                //i--;
            }
        }
        glw->show();
        glw->repaint();
    }
    else if(type==2) // visualizations
    {
        glw->hide();
        displayOptions->tabWidget->setCurrentIndex(0);
        canvas->Clear();
        canvas->repaint();
        ui.canvasWidget->repaint();
        //        ui.canvasWidget->hide();
        if(!ui.canvasArea->layout())
        {
            // this is an ugly hack that forces the layout behind to be drawn as a cleared image
            // for some reason otherwise, the canvas will only be repainted AFTER it is re-shown
            // and it will flicker with the last image shown beforehand
            QBoxLayout *layout = new QBoxLayout(QBoxLayout::LeftToRight);
            layout->setContentsMargins(1,1,1,1);
            layout->setMargin(1);
            ui.canvasArea->setLayout(layout);
            ui.canvasArea->layout()->addWidget(glw);
            ui.canvasArea->layout()->addWidget(vis);
        }
        QSizePolicy policy = vis->sizePolicy();
        policy.setHorizontalPolicy(QSizePolicy::Preferred);
        policy.setVerticalPolicy(QSizePolicy::Preferred);
        canvas->SetCanvasType(type);
        vis->setSizePolicy(policy);
        vis->setMinimumSize(ui.canvasArea->size());
        vis->resize(ui.canvasArea->size());
        vis->show();
        vis->Update();
    }
    else
    {
        displayOptions->tabWidget->setCurrentIndex(0);
        canvas->SetCanvasType(type);
        CanvasOptionsChanged();
        canvas->ResetSamples();
        glw->hide();
        vis->hide();
        ui.canvasWidget->show();
        ui.canvasWidget->repaint();
    }
    UpdateLearnedModel();
    canvas->repaint();
}

void MLDemos::CanvasOptionsChanged()
{
    QSizePolicy policy = ui.canvasWidget->sizePolicy();
    int dims = canvas ? (canvas->data->GetCount() ? canvas->data->GetSample(0).size() : 2) : 2;
    int w = ui.canvasArea->width();
    int h = ui.canvasArea->height();
    bool bNeedsZoom = false;
    if(canvas->canvasType == 1)
    {
        if(h/dims < 100)
        {
            h = 100*dims;
            bNeedsZoom = true;
        }
        if(w/dims < 100)
        {
            w = 100*dims;
            bNeedsZoom = true;
        }
    }
    //drawTimer->Stop();
    //drawTimer->Clear();

    if(canvas->canvasType == 0 || !bNeedsZoom)
    {
        policy.setHorizontalPolicy(QSizePolicy::Preferred);
        policy.setVerticalPolicy(QSizePolicy::Preferred);
        ui.canvasWidget->setSizePolicy(policy);
        ui.canvasWidget->setMinimumSize(ui.canvasArea->size());
        ui.canvasWidget->resize(ui.canvasArea->size());
        canvas->resize(ui.canvasWidget->size());
        ui.canvasArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        ui.canvasArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        canvas->SetCanvasType(canvas->canvasType);
        canvas->ResizeEvent();

        if(mutex.tryLock())
        {
            if(clusterer)
            {
                clusterers[tabUsedForTraining]->Draw(canvas, clusterer);
                drawTimer->start(QThread::NormalPriority);
            }
            if(dynamical)
            {
                dynamicals[tabUsedForTraining]->Draw(canvas, dynamical);
                drawTimer->start(QThread::NormalPriority);
            }
            if(projector)
            {
                projectors[tabUsedForTraining]->Draw(canvas, projector);
            }
            mutex.unlock();
        }
        return;
    }
    policy.setHorizontalPolicy(QSizePolicy::Fixed);
    policy.setVerticalPolicy(QSizePolicy::Fixed);
    ui.canvasWidget->setSizePolicy(policy);
    ui.canvasWidget->setMinimumSize(w,h);
    ui.canvasWidget->resize(w,h);
    canvas->resize(ui.canvasWidget->size());
    ui.canvasArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui.canvasArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    canvas->SetCanvasType(canvas->canvasType);

    if(mutex.tryLock())
    {
        if(classifier)
        {
            classifiers[tabUsedForTraining]->Draw(canvas, classifier);
            DrawClassifiedSamples(canvas, classifier, classifierMulti);
        }
        else if(regressor)
        {
            regressors[tabUsedForTraining]->Draw(canvas, regressor);
        }
        else if(dynamical)
        {
            dynamicals[tabUsedForTraining]->Draw(canvas, dynamical);
            drawTimer->start(QThread::NormalPriority);
        }
        else if(maximizer)
        {
        }
        if(clusterer)
        {
            clusterers[tabUsedForTraining]->Draw(canvas, clusterer);
            drawTimer->start(QThread::NormalPriority);
        }
        if(projector)
        {
            projectors[tabUsedForTraining]->Draw(canvas, projector);
        }
        mutex.unlock();
    }
    canvas->ResizeEvent();
}

void MLDemos::Navigation( fvec sample )
{
    if(sample[0]==-1)
    {
        ZoomChanged(sample[1]);
        return;
    }
    if(!mutex.tryLock(20)) return;
    QString information;
    char string[255];
    int count = canvas->data->GetCount();
    int pcount = 0, ncount = 0;
    ivec labels = canvas->data->GetLabels();
    FOR(i, labels.size())
    {
        if(labels[i] == 0) ++pcount;
        else ++ncount;
    }
    sprintf(string, "samples: %d (o:%.3d|x:%.3d)", count, pcount, ncount);
    information += QString(string);
    sprintf(string, " | x%d: %.3f x%d: %.3f", canvas->xIndex+1, sample[canvas->xIndex], canvas->yIndex+1, sample[canvas->yIndex]);
    information += QString(string);
    if(classifier)
    {
        float score;
        if(sourceDims.size())
        {
            fvec newSample(sourceDims.size());
            FOR(d, sourceDims.size()) newSample[d] = sample[sourceDims[d]];
            sample = newSample;
        }

        if(classifier->IsMultiClass())
        {
            fvec res = classifier->TestMulti(sample);
            int max = 0;
            FOR(i, res.size()) if(res[max] < res[i]) max = i;
            score = classifier->inverseMap[max];
        }
        else
        {
            score = classifier->Test(sample);
        }
        drawTimer->bPaused = false;
        sprintf(string, " | value: %.4f", score);
        information += QString(string);
    }
    else if(dynamical)
    {
        // we build the trajectory(by hand)

        int count = 1000;
        std::vector<fvec> trajectory;
        fvec position = sample;
        if(dynamical->avoid) dynamical->avoid->SetObstacles(canvas->data->GetObstacles());
        FOR(i, count)
        {
            trajectory.push_back(position);
            fvec velocity = dynamical->Test(position);
            if(dynamical->avoid)
            {
                fvec newVelocity = dynamical->avoid->Avoid(position, velocity);
                velocity = newVelocity;
            }
            position += velocity*dynamical->dT;
            if(velocity == 0) break;
        }
        canvas->liveTrajectory = trajectory;
        canvas->repaint();
    }
    mutex.unlock();
    ui.statusBar->showMessage(information);
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
    if(algorithmOptions->tabMax->isVisible())
    {
        canvas->targets.clear();
        canvas->targetAge.clear();
        canvas->repaint();
    }
    Qt::DropAction dropAction = drag->exec();
}

void MLDemos::ClearTargets()
{
    if(!canvas->targets.size()) return;
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
    mimeData->setColorData(QVariant(optionsMaximize->varianceSpin->value()));
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

void MLDemos::BenchmarkButton()
{
    int w = canvas->width(), h = canvas->height();
    int type = optionsMaximize->benchmarkCombo->currentIndex();
    QImage image(w, h, QImage::Format_ARGB32);
    image.fill(qRgb(255,255,255));

    int dim = 2;
    float minSpace = 0.f;
    float maxSpace = 1.f;
    float minVal = FLT_MAX;
    float maxVal = -FLT_MAX;
    switch(type)
    {
    case 0: // griewangk
        minSpace = -60.f;
        maxSpace = 60.f;
        minVal = 0;
        maxVal = 2;
        break;
    case 1: // rastragin
        minSpace = -5.12f;
        maxSpace = 5.12f;
        minVal = 0;
        maxVal = 82;
        break;
    case 2: // schwefel
        minSpace = -500.f;
        maxSpace = 500.f;
        minVal = -838;
        maxVal = 838;
        break;
    case 3: // ackley
        minSpace = -2.f;
        maxSpace = 2.f;
        minVal = 0;
        maxVal = 2.3504;
    case 4: // michalewicz
        minSpace = -2;
        maxSpace = 2;
        minVal = -1.03159;
        maxVal = 5.74;
        //		minVal = -1.03159;
        //		maxVal = 55.74;
    }

    bool bSetValues = minVal == FLT_MAX;
    Eigen::VectorXd x(2);
    fVec point;
    float value = 0;
    FOR(i, w)
    {
        x[0] = i/(float)w*(maxSpace - minSpace) + minSpace;
        FOR(j, h)
        {
            x[1] = j/(float)h*(maxSpace - minSpace) + minSpace;

            switch(type)
            {
            case 0:
                value = griewangk(x)(0);
                break;
            case 1:
                value = rastragin(x)(0);
                break;
            case 2:
                value = schwefel(x)(0);
                break;
            case 3:
                value = ackley(x)(0);
                break;
            case 4:
                value = sixhump(x)(0);
                break;
            }
            if(bSetValues)
            {
                if(value < minVal) minVal = value;
                if(value > maxVal) maxVal = value;
            }
            else
            {
                value = (value-minVal)/(maxVal-minVal);
            }

            int color = 255.f*max(0.f,min(1.f,value));
            image.setPixel(i,j,qRgba(255, color, color, 255));
        }
    }
    if(bSetValues) qDebug() << "minmax: " << minVal << " " << maxVal;

    canvas->maps.reward = QPixmap::fromImage(image);
    canvas->repaint();
}

void MLDemos::SaveData()
{
    if(!canvas) return;
    QString filename = QFileDialog::getSaveFileName(this, tr("Save Data"), "", tr("ML Files (*.ml)"));
    if(filename.isEmpty()) return;
    if(!filename.endsWith(".ml")) filename += ".ml";
    Save(filename);
}
void MLDemos::Save(QString filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly))
    {
        ui.statusBar->showMessage("WARNING: Unable to save file");
        return;
    }
    file.close();
    if(!canvas->maps.reward.isNull()) RewardFromMap(canvas->maps.reward.toImage());
    canvas->data->Save(filename.toAscii());
    SaveParams(filename);
    ui.statusBar->showMessage("Data saved successfully");
}

void MLDemos::LoadData()
{
    if(!canvas) return;
    QString filename = QFileDialog::getOpenFileName(this, tr("Load Data"), "", tr("ML Files (*.ml)"));
    if(filename.isEmpty()) return;
    if(!filename.endsWith(".ml")) filename += ".ml";
    Load(filename);
}

void MLDemos::Load(QString filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
    {
        ui.statusBar->showMessage("WARNING: Unable to open file");
        return;
    }
    file.close();
    ClearData();
    canvas->data->Load(filename.toAscii());
    MapFromReward();
    LoadParams(filename);
    //    QImage reward(filename + "-reward.png");
    //    if(!reward.isNull()) canvas->maps.reward = QPixmap::fromImage(reward);
    ui.statusBar->showMessage("Data loaded successfully");
    ResetPositiveClass();
    optionsRegress->outputDimCombo->setCurrentIndex(optionsRegress->outputDimCombo->count()-1);
    UpdateInfo();
    canvas->repaint();
}

void MLDemos::ImportData()
{
    if(!canvas || !import) return;
    QString filename = QFileDialog::getOpenFileName(this, tr("Import Data Data"), "", tr("Dataset Files (*.csv *.data *.txt)"));
    if(filename.isEmpty()) return;
    import->Start();
    import->Parse(filename);
    import->SendData();
}

void MLDemos::ImportData(QString filename)
{
    import->Start();
    import->Parse(filename);
    import->SendData();
    if(import->GetHeaders().size()) canvas->dimNames = import->GetHeaders();
    ResetPositiveClass();
    ui.statusBar->showMessage("Data loaded successfully");
}

void MLDemos::dragEnterEvent(QDragEnterEvent *event)
{
    QList<QUrl> dragUrl;
    if(event->mimeData()->hasUrls())
    {
        QList<QUrl> urls = event->mimeData()->urls();
        QStringList dataType;
        dataType << ".ml" << ".csv" << ".data";
        for(int i=0; i<urls.size(); i++)
        {
            QString filename = urls[i].path();
            for(int j=0; j < dataType.size(); j++)
            {
                if(filename.toLower().endsWith(dataType[j]))
                {
                    dragUrl.push_back(urls[i]);
                    break;
                }
            }
        }
        if(dragUrl.size())
        {
            event->acceptProposedAction();
        }
    }
}

void MLDemos::dropEvent(QDropEvent *event)
{
    if(!event->mimeData()->hasUrls()) return;
    FOR(i, event->mimeData()->urls().length())
    {
        QString filename = event->mimeData()->urls()[i].toLocalFile();
        qDebug() << "accepted drop file:" << filename;
        if(filename.toLower().endsWith(".ml"))
        {
            ClearData();
            canvas->data->Load(filename.toAscii());
            MapFromReward();
            LoadParams(filename);
            ui.statusBar->showMessage("Data loaded successfully");
            ResetPositiveClass();
            UpdateInfo();
            canvas->repaint();
        }
        else if(filename.toLower().endsWith(".csv") || filename.toLower().endsWith(".data"))
        {
            ClearData();
            ImportData(filename);
            ResetPositiveClass();
            UpdateInfo();
            canvas->repaint();
        }
    }
    event->acceptProposedAction();
}

void MLDemos::ExportSVG()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Save Vector Image"), "", tr("Images (*.svg)"));
    if(filename.isEmpty()) return;
    if(!filename.endsWith(".svg")) filename += ".svg";

    DrawSVG svg(canvas, &mutex);
    svg.classifier = classifier;
    svg.classifierMulti = classifierMulti;
    svg.regressor = regressor;
    svg.clusterer = clusterer;
    svg.dynamical = dynamical;
    svg.maximizer = maximizer;
    svg.projector = projector;
    if(classifier) svg.drawClass = classifiers[tabUsedForTraining];
    if(regressor) svg.drawRegr = regressors[tabUsedForTraining];
    if(dynamical) svg.drawDyn = dynamicals[tabUsedForTraining];
    if(clusterer) svg.drawClust = clusterers[tabUsedForTraining];
    if(projector) svg.drawProj = projectors[tabUsedForTraining];
    svg.Write(filename);
    ui.statusBar->showMessage("Vector Image saved successfully");
}

void MLDemos::Screenshot()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Save Screenshot"), "", tr("Images (*.png *.jpg)"));
    if(filename.isEmpty()) return;
    if(!filename.endsWith(".jpg") && !filename.endsWith(".png")) filename += ".png";
    if(canvas->canvasType == 1)
    {
        QImage img = glw->grabFrameBuffer();
        if(!img.save(filename)) ui.statusBar->showMessage("WARNING: Unable to save image");
        else ui.statusBar->showMessage("Image saved successfully");
    }
    else
    {
        if(!canvas->SaveScreenshot(filename)) ui.statusBar->showMessage("WARNING: Unable to save image");
        else ui.statusBar->showMessage("Image saved successfully");
    }
}

void MLDemos::CompareScreenshot()
{
    if(!canvas || !compare) return;
    QClipboard *clipboard = QApplication::clipboard();
    QPixmap screenshot = compare->Display().copy();
    clipboard->setImage(screenshot.toImage());
    //clipboard->setPixmap(screenshot);
    clipboard->setText(compare->ToString());
}

void MLDemos::ToClipboard()
{
    QClipboard *clipboard = QApplication::clipboard();
    if(canvas->canvasType == 1)
    {
        QImage img = glw->grabFrameBuffer();
        clipboard->setImage(img);
        clipboard->setPixmap(QPixmap::fromImage(img));
    }
    else
    {
        QPixmap screenshot = canvas->GetScreenshot();
        if(screenshot.isNull())
        {
            ui.statusBar->showMessage("WARNING: Nothing to copy to clipboard");
            return;
        }
        clipboard->setImage(screenshot.toImage());
        clipboard->setPixmap(screenshot);
    }
    ui.statusBar->showMessage("Image copied successfully to clipboard");
}

/************************************/
/*                                  */
/*      Input Output functions      */
/*                                  */
/************************************/

void MLDemos::ActivateIO()
{
    QList<QAction *> pluginActions = ui.menuInput_Output->actions();
    FOR(i, inputoutputs.size())
    {
        if(i<pluginActions.size() && inputoutputs[i] && pluginActions[i])
        {
            if(pluginActions[i]->isChecked())
            {
                bInputRunning[i] = true;
                inputoutputs[i]->Start();
            }
            else if(bInputRunning[i])
            {
                bInputRunning[i] = false;
                inputoutputs[i]->Stop();
            }
        }
    }
}

void MLDemos::ActivateImport()
{
    QList<QAction *> pluginActions = ui.menuInput_Output->actions();
    FOR(i, inputoutputs.size())
    {
        if(i<pluginActions.size() && inputoutputs[i] && pluginActions[i])
        {
            if(pluginActions[i]->isChecked())
            {
                bInputRunning[i] = true;
                inputoutputs[i]->Start();
            }
            else if(bInputRunning[i])
            {
                bInputRunning[i] = false;
                inputoutputs[i]->Stop();
            }
        }
    }
}

void MLDemos::DisactivateIO(QObject *io)
{
    if(!io) return;
    // first we find the right plugin
    int pluginIndex = -1;
    FOR(i, inputoutputs.size())
    {
        if(inputoutputs[i]->object() == io)
        {
            pluginIndex = i;
            break;
        }
    }
    if(pluginIndex == -1)
    {
        statusBar()->showMessage("Unable to unload plugin: ");
        return; // something weird is going on!
    }
    QList<QAction *> pluginActions = ui.menuInput_Output->actions();
    if(pluginIndex < pluginActions.size() && pluginActions[pluginIndex])
    {
        pluginActions[pluginIndex]->setChecked(false);
        if(bInputRunning[pluginIndex]) inputoutputs[pluginIndex]->Stop();
        bInputRunning[pluginIndex] = false;
    }
    pluginActions = ui.menuImport->actions();
    if(pluginIndex < pluginActions.size() && pluginActions[pluginIndex])
    {
        pluginActions[pluginIndex]->setChecked(false);
        if(bInputRunning[pluginIndex]) inputoutputs[pluginIndex]->Stop();
        bInputRunning[pluginIndex] = false;
    }
}

void MLDemos::SetData(std::vector<fvec> samples, ivec labels, std::vector<ipair> trajectories, bool bProjected)
{
    sourceData.clear();
    sourceLabels.clear();
    projectedData.clear();
    if(!canvas) return;
    canvas->dimNames.clear();
    canvas->sampleColors.clear();
    canvas->data->Clear();
    canvas->data->AddSamples(samples, labels);
    canvas->data->bProjected = bProjected;
    if(bProjected) ui.status->setText("Projected Data (PCA, etc.)");
    else ui.status->setText("Raw Data");
    if(trajectories.size())
    {
        canvas->data->AddSequences(trajectories);
    }
    FitToData();
    ResetPositiveClass();
    ManualSelectionUpdated();
    optionsRegress->outputDimCombo->setCurrentIndex(optionsRegress->outputDimCombo->count()-1);
    CanvasOptionsChanged();
    drawToolbar->singleButton->setChecked(true);
    drawToolbar->singleButton->setChecked(false);
    canvas->ResetSamples();
    canvas->repaint();
}

void MLDemos::SetDimensionNames(QStringList headers)
{
    //qDebug() << "setting dimension names" << headers;
    canvas->dimNames = headers;
    ResetPositiveClass();
    CanvasOptionsChanged();
    dataEdit->Update();
    canvas->ResetSamples();
    canvas->repaint();
}

void MLDemos::SetClassNames(std::map<int,QString> classNames)
{
    canvas->classNames = classNames;
    ResetPositiveClass();
    CanvasOptionsChanged();
    dataEdit->Update();
    canvas->ResetSamples();
    canvas->repaint();
}

void MLDemos::SetCategorical(std::map<int,std::vector<std::string> > categorical)
{
    canvas->data->categorical = categorical;
    ResetPositiveClass();
    CanvasOptionsChanged();
    dataEdit->Update();
    canvas->ResetSamples();
    canvas->repaint();
}

void MLDemos::SetTimeseries(std::vector<TimeSerie> timeseries)
{
    //	qDebug() << "importing " << timeseries.size() << " timeseries";
    sourceData.clear();
    sourceLabels.clear();
    projectedData.clear();
    if(!canvas) return;
    canvas->dimNames.clear();
    canvas->sampleColors.clear();
    canvas->data->Clear();
    canvas->data->AddTimeSeries(timeseries);
    FitToData();
    ResetPositiveClass();
    ManualSelectionUpdated();
    canvas->ResetSamples();
    canvas->repaint();
    qDebug() << "added " << canvas->data->GetTimeSeries().size() << " timeseries";
    //	qDebug() << " dim: " << dim << " count: " << count << " frames: " << frames;

    /*
 vector<TimeSerie> series = canvas->data->GetTimeSeries();
 FOR(i, series.size())
 {
  FOR(j, series[i].size())
  {
   qDebug() << i << " " << j << ": " << series[i][j][0];
   FOR(d, series[i][j].size())
   {
//				qDebug() << i << " " << j << " " << d << ": " << series[i][j][d];
   }
  }
 }
 */
}

void MLDemos::QueryClassifier(std::vector<fvec> samples)
{
    std::vector<fvec> results;
    QMutexLocker lock(&mutex);
    fvec result;
    result.resize(1);
    if(classifier && samples.size())
    {
        results.resize(samples.size());
        FOR(i, samples.size())
        {
            fvec sample = samples[i];
            if(sourceDims.size())
            {
                fvec newSample(sourceDims.size());
                FOR(d, sourceDims.size()) newSample[d] = sample[sourceDims[d]];
                sample = newSample;
            }
            result[0] = classifier->Test(sample);
            results[i] = result;
        }
    }
    emit SendResults(results);
}

void MLDemos::QueryRegressor(std::vector<fvec> samples)
{
    std::vector<fvec> results;
    QMutexLocker lock(&mutex);
    if(regressor && samples.size())
    {
        results.resize(samples.size());
        FOR(i, samples.size())
        {
            results[i] = regressor->Test(samples[i]);
        }
    }
    emit SendResults(results);
}

void MLDemos::QueryDynamical(std::vector<fvec> samples)
{
    std::vector<fvec> results;
    QMutexLocker lock(&mutex);
    if(dynamical && samples.size())
    {
        results.resize(samples.size());
        FOR(i, samples.size())
        {
            results[i] = dynamical->Test(samples[i]);
        }
    }
    emit SendResults(results);
}

void MLDemos::QueryClusterer(std::vector<fvec> samples)
{
    std::vector<fvec> results;
    QMutexLocker lock(&mutex);
    if(clusterer && samples.size())
    {
        results.resize(samples.size());
        FOR(i, samples.size())
        {
            results[i] = clusterer->Test(samples[i]);
        }
    }
    emit SendResults(results);
}

void MLDemos::QueryMaximizer(std::vector<fvec> samples)
{
    std::vector<fvec> results;
    QMutexLocker lock(&mutex);
    if(maximizer && samples.size())
    {
        results.resize(samples.size());
        FOR(i, samples.size())
        {
            results[i] = maximizer->Test(samples[i]);
        }
    }
    emit SendResults(results);
}

void MLDemos::QueryProjector(std::vector<fvec> samples)
{
    std::vector<fvec> results;
    QMutexLocker lock(&mutex);
    if(projector && samples.size())
    {
        results.resize(samples.size());
        FOR(i, samples.size())
        {
            results[i] = projector->Project(samples[i]);
        }
    }
    emit SendResults(results);
}
