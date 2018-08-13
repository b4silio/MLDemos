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
    connect(ui.canvasTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(CanvasTypeChanged()));
    connect(ui.canvasX1Spin, SIGNAL(valueChanged(int)), this, SLOT(DisplayOptionsChanged()));
    connect(ui.canvasX2Spin, SIGNAL(valueChanged(int)), this, SLOT(DisplayOptionsChanged()));
    connect(ui.canvasX3Spin, SIGNAL(valueChanged(int)), this, SLOT(DisplayOptionsChanged()));

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

    connect(drawToolbar->singleButton, SIGNAL(clicked()), this, SLOT(DrawToolsChanged()));
    connect(drawToolbar->sprayButton, SIGNAL(clicked()), this, SLOT(DrawToolsChanged()));
    connect(drawToolbar->spray3DButton, SIGNAL(clicked()), this, SLOT(DrawToolsChanged()));
    connect(drawToolbar->trajectoryButton, SIGNAL(clicked()), this, SLOT(DrawToolsChanged()));
    connect(drawToolbar->paintButton, SIGNAL(clicked()), this, SLOT(DrawToolsChanged()));
    connect(drawToolbar->obstacleButton, SIGNAL(clicked()), this, SLOT(DrawToolsChanged()));
    connect(drawToolbar->sprayClassButton, SIGNAL(clicked()), this, SLOT(DrawToolsChanged()));
    connect(drawToolbar->eraseButton, SIGNAL(clicked()), this, SLOT(DrawToolsChanged()));
    connect(drawToolbar->extrudeButton, SIGNAL(clicked()), this, SLOT(DrawToolsChanged()));
    connect(drawToolbar->dimPlusButton, SIGNAL(clicked()), this, SLOT(DrawToolsChanged()));
    connect(drawToolbar->dimLessButton, SIGNAL(clicked()), this, SLOT(DrawToolsChanged()));
    connect(drawToolbar->lineButton, SIGNAL(clicked()), this, SLOT(DrawToolsChanged()));
    connect(drawToolbar->ellipseButton, SIGNAL(clicked()), this, SLOT(DrawToolsChanged()));
    connect(drawToolbar->dragButton, SIGNAL(clicked()), this, SLOT(DrawToolsChanged()));
    connect(drawToolbar->moveButton, SIGNAL(clicked()), this, SLOT(DrawToolsChanged()));
    connect(drawToolbar->moveClassButton, SIGNAL(clicked()), this, SLOT(DrawToolsChanged()));
    connect(drawToolbar->datasetGeneratorButton, SIGNAL(clicked()), this, SLOT(ToggleDataGenerator()));
    connect(drawToolbar->compareButton, SIGNAL(clicked()), this, SLOT(ShowOptionCompare()));
    connect(drawToolbar->gridsearchButton, SIGNAL(clicked()), this, SLOT(ShowGridSearch()));

    drawToolbar->sprayButton->setContextMenuPolicy(Qt::CustomContextMenu);
    drawToolbar->ellipseButton->setContextMenuPolicy(Qt::CustomContextMenu);
    drawToolbar->lineButton->setContextMenuPolicy(Qt::CustomContextMenu);
    drawToolbar->eraseButton->setContextMenuPolicy(Qt::CustomContextMenu);
    drawToolbar->obstacleButton->setContextMenuPolicy(Qt::CustomContextMenu);
    drawToolbar->paintButton->setContextMenuPolicy(Qt::CustomContextMenu);
    drawToolbar->optionWidget->layout()->addWidget(drawContext1Widget);
    drawToolbar->optionWidget->layout()->addWidget(drawContext2Widget);
    drawToolbar->optionWidget->layout()->addWidget(drawContext3Widget);
    drawToolbar->optionWidget->layout()->addWidget(drawContext4Widget);
    drawContext1Widget->hide();
    drawContext2Widget->hide();
    drawContext3Widget->hide();
    drawContext4Widget->hide();

    viewOptions = new Ui::viewOptionDialog();
    displayOptions = new Ui::DisplayOptions();
    aboutPanel = new Ui::aboutDialog();
    showStats = new Ui::statisticsDialog();
    manualSelection = new Ui::ManualSelection();
    inputDimensions = new Ui::InputDimensions();

    viewOptions->setupUi(displayDialog = new QDialog());
    displayOptions->setupUi(displayOptionWidget = new QWidget());
    ui.displayOptionWidget->layout()->addWidget(displayOptionWidget);
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

    connect(viewOptions->spinZoom, SIGNAL(valueChanged(double)), this, SLOT(DisplayOptionsChanged()));
    connect(viewOptions->check3DSamples, SIGNAL(clicked()), this, SLOT(Display3DOptionsChanged()));
    connect(viewOptions->check3DWireframe, SIGNAL(clicked()), this, SLOT(Display3DOptionsChanged()));
    connect(viewOptions->check3DSurfaces, SIGNAL(clicked()), this, SLOT(Display3DOptionsChanged()));
    connect(viewOptions->check3DTransparency, SIGNAL(clicked()), this, SLOT(Display3DOptionsChanged()));
    connect(viewOptions->check3DBlurry, SIGNAL(clicked()), this, SLOT(Display3DOptionsChanged()));
    connect(viewOptions->check3DRotate, SIGNAL(clicked()), this, SLOT(Display3DOptionsChanged()));
    connect(displayOptions->showSamples, SIGNAL(clicked()), this, SLOT(DisplayOptionsChanged()));
    connect(displayOptions->showOutput, SIGNAL(clicked()), this, SLOT(DisplayOptionsChanged()));
    connect(displayOptions->showModel, SIGNAL(clicked()), this, SLOT(DisplayOptionsChanged()));
    connect(displayOptions->showBackground, SIGNAL(clicked()), this, SLOT(DisplayOptionsChanged()));
    connect(displayOptions->showGrid, SIGNAL(clicked()), this, SLOT(DisplayOptionsChanged()));
    connect(displayOptions->showLegend, SIGNAL(clicked()), this, SLOT(DisplayOptionsChanged()));
    connect(displayOptions->saveToClipboard, SIGNAL(clicked()), this, SLOT(ScreenshotToClipboard()));
    connect(displayOptions->saveToFile, SIGNAL(clicked()), this, SLOT(ScreenshotToFile()));
    connect(displayOptions->fitToViewport, SIGNAL(clicked()), this, SLOT(FitToData()));
    connect(displayOptions->clearAll, SIGNAL(clicked()), this, SLOT(ClearAll()));
    connect(displayOptions->clearData, SIGNAL(clicked()), this, SLOT(ClearData()));
    connect(displayOptions->showStats, SIGNAL(clicked()), this, SLOT(ShowStatsDialog()));
    connect(displayOptions->showOptions, SIGNAL(clicked()), this, SLOT(ShowOptionDisplay()));

    displayDialog->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    if(ui.leftPaneWidget->layout()==nullptr) ui.leftPaneWidget->setLayout(new QHBoxLayout());
    ui.leftPaneWidget->layout()->addWidget(drawToolbarWidget);
    drawToolbarWidget->show();
    drawContext1Widget->setWindowFlags(Qt::FramelessWindowHint);
    drawContext2Widget->setWindowFlags(Qt::FramelessWindowHint);
    drawContext3Widget->setWindowFlags(Qt::FramelessWindowHint);
    drawContext4Widget->setWindowFlags(Qt::FramelessWindowHint);

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
    ui.canvasWidget->layout()->addWidget(glw);
    ui.canvasWidget->layout()->addWidget(vis);
    glw->hide();
    vis->hide();
    ResetMinimumWidth();

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
    connect(compare, SIGNAL(Hiding()), this, SLOT(HideOptionCompare()));
    connect(gridSearch, SIGNAL(Hiding()), this, SLOT(HideGridSearch()));

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
