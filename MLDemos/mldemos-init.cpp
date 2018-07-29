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

    connect(actionNew, SIGNAL(triggered()), this, SLOT(ClearData()));
    connect(actionSave, SIGNAL(triggered()), this, SLOT(SaveData()));
    connect(actionLoad, SIGNAL(triggered()), this, SLOT(LoadData()));
    connect(actionAlgorithms, SIGNAL(triggered()), this, SLOT(ShowAlgorithmOptions()));
    connect(actionAlgorithms,SIGNAL(triggered()),this, SLOT(HideAlgorithmOptions()));
    connect(actionCompare, SIGNAL(triggered()), this, SLOT(ShowOptionCompare()));
    connect(actionDrawSamples, SIGNAL(triggered()), this, SLOT(ShowSampleDrawing()));
    connect(actionDisplayOptions, SIGNAL(triggered()), this, SLOT(ShowOptionDisplay()));
    connect(actionAddData, SIGNAL(triggered()), this, SLOT(ShowAddData()));
    connect(actionGridsearch, SIGNAL(triggered()), this, SLOT(ShowGridSearch()));
    connect(actionClearData, SIGNAL(triggered()), this, SLOT(ClearData()));
    connect(actionClearAll, SIGNAL(triggered()), this, SLOT(ClearAll()));
    connect(actionClearModel, SIGNAL(triggered()), this, SLOT(Clear()));
    connect(actionScreenshot, SIGNAL(triggered()), this, SLOT(Screenshot()));
    connect(actionShowStats, SIGNAL(triggered()), this, SLOT(ShowStatsDialog()));

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
    connect(drawToolbar->radiusSpin, SIGNAL(valueChanged(double)), this, SLOT(CrosshairChanged()));
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
    connect(drawToolbar->sprayButton, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(ShowContextMenuSpray(const QPoint &)));
    connect(drawToolbar->ellipseButton, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(ShowContextMenuEllipse(const QPoint &)));
    connect(drawToolbar->lineButton, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(ShowContextMenuLine(const QPoint &)));
    connect(drawToolbar->eraseButton, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(ShowContextMenuErase(const QPoint &)));
    connect(drawToolbar->obstacleButton, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(ShowContextMenuObstacle(const QPoint &)));
    connect(drawToolbar->paintButton, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(ShowContextMenuReward(const QPoint &)));

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

    displayDialog->setWindowFlags(Qt::Tool); // disappears when unfocused on the mac
    //drawToolbarWidget->setWindowFlags(Qt::Tool);
    drawToolbarWidget->setWindowFlags(Qt::CustomizeWindowHint | Qt::Tool | Qt::WindowTitleHint);
    drawContext1Widget->setWindowFlags(Qt::FramelessWindowHint);
    drawContext2Widget->setWindowFlags(Qt::FramelessWindowHint);
    drawContext3Widget->setWindowFlags(Qt::FramelessWindowHint);
    drawContext4Widget->setWindowFlags(Qt::FramelessWindowHint);
    drawToolbarWidget->setFixedSize(drawToolbarWidget->size());

    connect(displayDialog, SIGNAL(rejected()), this, SLOT(HideOptionDisplay()));
    connect(statsDialog, SIGNAL(rejected()), this, SLOT(HideStatsDialog()));
    //connect(manualSelectDialog, SIGNAL(rejected()), this, SLOT(HideManualSelectionDialog()));

    canvas = new Canvas(ui.canvasWidget);
    compare = new CompareAlgorithms(canvas);
    import = new DataImporter();
    gridSearch = new GridSearch(canvas);
    generator = new DataGenerator(canvas);
    glw = new GLWidget(canvas);
    vis = new Visualization(canvas);
    dataEdit = new DatasetEditor(canvas);
    drawTimer = new DrawTimer(canvas, &mutex);
    drawTimer->glw = glw;
    algo = new AlgorithmManager(this, canvas, glw, &mutex, drawTimer, compare, gridSearch);
    algo->inputDimensions = inputDimensions;
    algo->manualSelection = manualSelection;
    plugin = new PluginManager(this, algo);

    connect(generator, SIGNAL(finished(int)), this, SLOT(HideAddData()));

    connect(canvas, SIGNAL(Drawing(fvec,int)), this, SLOT(Drawing(fvec,int)));
    connect(canvas, SIGNAL(DrawCrosshair()), this, SLOT(DrawCrosshair()));
    connect(canvas, SIGNAL(Navigation(fvec)), this, SLOT(Navigation(fvec)));
    connect(canvas, SIGNAL(Released()), this, SLOT(DrawingStopped()));
    connect(canvas, SIGNAL(CanvasMoveEvent()), this, SLOT(CanvasMoveEvent()));
    //connect(canvas, SIGNAL(ZoomChanged()), this, SLOT(ZoomChanged()));

    connect(compare->params->compareButton, SIGNAL(clicked()), algo, SLOT(Compare()));
    connect(compare->params->inputDimButton, SIGNAL(clicked()), this, SLOT(ShowInputDimensions()));
    connect(algo->optionsRegress->outputDimCombo, SIGNAL(currentIndexChanged(int)), compare->params->outputDimCombo, SLOT(setCurrentIndex(int)));
    connect(compare->params->outputDimCombo, SIGNAL(currentIndexChanged(int)), algo->optionsRegress->outputDimCombo, SLOT(setCurrentIndex(int)));

    connect(import, import->SetDataSignal(), this, SLOT(SetData(std::vector<fvec>, ivec, std::vector<ipair>, bool)));
    connect(import, import->SetTimeseriesSignal(), this, SLOT(SetTimeseries(std::vector<TimeSerie>)));
    connect(import, SIGNAL(SetDimensionNames(QStringList)), this, SLOT(SetDimensionNames(QStringList)));
    connect(import, SIGNAL(SetClassNames(std::map<int,QString>)), this, SLOT(SetClassNames(std::map<int,QString>)));
    connect(import, SIGNAL(SetCategorical(std::map<int,std::vector<std::string> >)), this, SLOT(SetCategorical(std::map<int,std::vector<std::string> >)));

    connect(generator->ui->addButton, SIGNAL(clicked()), this, SLOT(AddDataFromGenerator()));
    connect(dataEdit, SIGNAL(DataEdited()), this, SLOT(DataEdited()));

    connect(drawTimer, SIGNAL(MapReady(QImage)), canvas, SLOT(SetConfidenceMap(QImage)));
    connect(drawTimer, SIGNAL(ModelReady(QImage)), canvas, SLOT(SetModelImage(QImage)));
    connect(drawTimer, SIGNAL(CurveReady()), this, SLOT(SetROCInfo()));
    connect(drawTimer, SIGNAL(AnimationReady(QImage)), canvas, SLOT(SetAnimationImage(QImage)));

    connect(algo, SIGNAL(CanvasOptionsChanged()), this, SLOT(CanvasOptionsChanged()));
    connect(algo, SIGNAL(CanvasTypeChanged()), this, SLOT(CanvasTypeChanged()));
    connect(algo, SIGNAL(DisplayOptionsChanged()), this, SLOT(DisplayOptionsChanged()));
    connect(algo, SIGNAL(ResetPositiveClass()), this, SLOT(ResetPositiveClass()));
    connect(algo, SIGNAL(UpdateInfo()), this, SLOT(UpdateInfo()));
    connect(algo, SIGNAL(Trained()), this, SLOT(Trained()));
    connect(ui.restrictDimCheck, SIGNAL(clicked(bool)), algo, SLOT(RestrictDimChanged()));
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
    children = algo->algorithmWidget->findChildren<QWidget*>();
    FOR (i, children.size()) if (children[i]) children[i]->setFont(font);
    algo->optionsMaximize->gaussianButton->setFont(QFont("Lucida Sans Unicode", 18));
    algo->optionsMaximize->gradientButton->setFont(QFont("Lucida Sans Unicode", 18));
    algo->optionsMaximize->targetButton->setFont(QFont("Lucida Sans Unicode", 18));
    algo->optionsReinforcement->gaussianButton->setFont(QFont("Lucida Sans Unicode", 18));
    algo->optionsReinforcement->gradientButton->setFont(QFont("Lucida Sans Unicode", 18));
    algo->optionsReinforcement->targetButton->setFont(QFont("Lucida Sans Unicode", 18));
    children = compare->paramsWidget->findChildren<QWidget*>();
    FOR (i, children.size()) if (children[i]) children[i]->setFont(font);
    children = displayDialog->findChildren<QWidget*>();
    FOR (i, children.size()) if (children[i]) children[i]->setFont(font);
    children = statsDialog->findChildren<QWidget*>();
    FOR (i, children.size()) if (children[i]) children[i]->setFont(font);
    children = aboutDialog->findChildren<QWidget*>();
    FOR (i, children.size()) if (children[i]) children[i]->setFont(font);
    children = manualSelectDialog->findChildren<QWidget*>();
    FOR (i, children.size()) if (children[i]) children[i]->setFont(font);
    children = inputDimensionsDialog->findChildren<QWidget*>();
    FOR (i, children.size()) if (children[i]) children[i]->setFont(font);
    children = generator->findChildren<QWidget*>();
    FOR (i, children.size()) if (children[i]) children[i]->setFont(font);
}
