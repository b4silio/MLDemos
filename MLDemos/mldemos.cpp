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
#include "basicMath.h"
#include "fgmm/gaussian.h"


MLDemos::MLDemos(QString filename, QWidget *parent, Qt::WFlags flags)
    : QMainWindow(parent, flags),
      canvas(0),
      classifier(0),
      regressor(0),
      dynamical(0),
      clusterer(0),
      bIsRocNew(true),
      bIsCrossNew(true),
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
    connect(ui.actionNew, SIGNAL(triggered()), this, SLOT(ClearData()));
    connect(ui.actionSave, SIGNAL(triggered()), this, SLOT(SaveData()));
    connect(ui.actionLoad, SIGNAL(triggered()), this, SLOT(LoadData()));
    connect(ui.actionExportOutput, SIGNAL(triggered()), this, SLOT(ExportOutput()));
    connect(ui.actionExportAnimation, SIGNAL(triggered()), this, SLOT(ExportAnimation()));

    initDialogs();
    initToolBars();
    initPlugins();
	LoadLayoutOptions();
    DisplayOptionChanged();
    UpdateInfo();
    FitToData();

	ui.canvasWidget->resize(width(), height());
	canvas->resize(ui.canvasWidget->width(), ui.canvasWidget->height());
	canvas->ResizeEvent();
	canvas->repaint();

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

    actionClassifiers = new QAction(QIcon(":/MLDemos/icons/classify.png"), tr("&Classification"), this);
    actionClassifiers->setShortcut(QKeySequence(tr("C")));
    actionClassifiers->setStatusTip(tr("Classify the data"));
    actionClassifiers->setCheckable(true);

    actionRegression = new QAction(QIcon(":/MLDemos/icons/regress.png"), tr("&Regression"), this);
    actionRegression->setShortcut(QKeySequence(tr("R")));
    actionRegression->setStatusTip(tr("Estimate Regression"));
    actionRegression->setCheckable(true);

    actionDynamical = new QAction(QIcon(":/MLDemos/icons/dynamical.png"), tr("&Dynamical"), this);
    actionDynamical->setShortcut(QKeySequence(tr("D")));
    actionDynamical->setStatusTip(tr("Estimate Dynamical System"));
    actionDynamical->setCheckable(true);

    actionClustering = new QAction(QIcon(":/MLDemos/icons/cluster.png"), tr("C&lustering"), this);
    actionClustering->setShortcut(QKeySequence(tr("L")));
    actionClustering->setStatusTip(tr("Cluster the data"));
    actionClustering->setCheckable(true);

    actionDrawSamples = new QAction(QIcon(":/MLDemos/icons/draw.png"), tr("&Drawing"), this);
    actionDrawSamples->setShortcut(QKeySequence(tr("W")));
    actionDrawSamples->setStatusTip(tr("Show Sample Drawing Options"));
    actionDrawSamples->setCheckable(true);

    actionClearModel = new QAction(QIcon(":/MLDemos/icons/clearmodel.png"), tr("Clear Model"), this);
    actionClearModel->setShortcut(QKeySequence(tr("Shift+X")));
    actionClearModel->setStatusTip(tr("Clear current model"));

    actionClearData = new QAction(QIcon(":/MLDemos/icons/cleardata.png"), tr("Clear Data"), this);
    actionClearData->setShortcut(QKeySequence(tr("X")));
    actionClearData->setStatusTip(tr("Clear all data"));

    actionScreenshot = new QAction(QIcon(":/MLDemos/icons/screenshot.png"), tr("Save Screenshot"), this);
    actionScreenshot->setShortcut(QKeySequence(tr("Alt+S")));
    actionScreenshot->setStatusTip(tr("Save the current image to disk"));

    actionDisplayOptions = new QAction(QIcon(":/MLDemos/icons/display.png"), tr("Display &Options"), this);
    actionDisplayOptions->setShortcut(QKeySequence(tr("O")));
    actionDisplayOptions->setStatusTip(tr("Show Display Options"));
    actionDisplayOptions->setCheckable(true);

    actionShowStats = new QAction(QIcon(":/MLDemos/icons/stats.png"), tr("Data Info/Statistics"), this);
    actionShowStats->setShortcut(QKeySequence(tr("I")));
    actionShowStats->setStatusTip(tr("Display Data Information and Statistics"));
    actionShowStats->setCheckable(true);

    connect(actionClassifiers, SIGNAL(triggered()), this, SLOT(ShowOptionClass()));
    connect(actionRegression, SIGNAL(triggered()), this, SLOT(ShowOptionRegress()));
    connect(actionDynamical, SIGNAL(triggered()), this, SLOT(ShowOptionDynamical()));
    connect(actionClustering, SIGNAL(triggered()), this, SLOT(ShowOptionCluster()));
    connect(actionDrawSamples, SIGNAL(triggered()), this, SLOT(ShowSampleDrawing()));
    connect(actionDisplayOptions, SIGNAL(triggered()), this, SLOT(ShowOptionDisplay()));
    connect(actionClearData, SIGNAL(triggered()), this, SLOT(ClearData()));
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

    QToolBar *toolBar = addToolBar("Tools");
    toolBar->setObjectName("MainToolBar");
	//toolBar->setMovable(false);
	//toolBar->setFloatable(false);
    toolBar->setIconSize(QSize(64,64));
    toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    toolBar->addAction(actionNew);
    toolBar->addAction(actionLoad);
    toolBar->addAction(actionSave);
    toolBar->addSeparator();
    toolBar->addAction(actionClustering);
    toolBar->addAction(actionClassifiers);
    toolBar->addAction(actionRegression);
    toolBar->addAction(actionDynamical);
    toolBar->addSeparator();
    toolBar->addAction(actionClearModel);
    toolBar->addAction(actionClearData);
    toolBar->addSeparator();
    toolBar->addAction(actionDrawSamples);
    toolBar->addSeparator();
    toolBar->addAction(actionScreenshot);
    toolBar->addAction(actionDisplayOptions);
    toolBar->addAction(actionShowStats);
}

void MLDemos::initDialogs()
{
    drawToolbar = new Ui::DrawingToolbar();
    drawToolbarContext1 = new Ui::DrawingToolbarContext1();
    drawToolbarContext2 = new Ui::DrawingToolbarContext2();
    drawToolbarContext3 = new Ui::DrawingToolbarContext3();

    drawToolbar->setupUi(drawToolbarWidget = new QWidget());
    drawToolbarContext1->setupUi(drawContext1Widget = new QWidget());
    drawToolbarContext2->setupUi(drawContext2Widget = new QWidget());
    drawToolbarContext3->setupUi(drawContext3Widget = new QWidget());

    connect(qApp, SIGNAL(focusChanged(QWidget *,QWidget *)),this,SLOT(FocusChanged(QWidget *,QWidget *)));

    drawToolbar->sprayButton->setContextMenuPolicy(Qt::CustomContextMenu);
    drawToolbar->ellipseButton->setContextMenuPolicy(Qt::CustomContextMenu);
    drawToolbar->lineButton->setContextMenuPolicy(Qt::CustomContextMenu);
    drawToolbar->eraseButton->setContextMenuPolicy(Qt::CustomContextMenu);
    drawToolbar->obstacleButton->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(drawToolbar->sprayButton, SIGNAL(customContextMenuRequested( const QPoint& )), this, SLOT(ShowContextMenuSpray(const QPoint &)));
    connect(drawToolbar->ellipseButton, SIGNAL(customContextMenuRequested( const QPoint& )), this, SLOT(ShowContextMenuEllipse(const QPoint &)));
    connect(drawToolbar->lineButton, SIGNAL(customContextMenuRequested( const QPoint& )), this, SLOT(ShowContextMenuLine(const QPoint &)));
    connect(drawToolbar->eraseButton, SIGNAL(customContextMenuRequested( const QPoint& )), this, SLOT(ShowContextMenuErase(const QPoint &)));
    connect(drawToolbar->obstacleButton, SIGNAL(customContextMenuRequested( const QPoint& )), this, SLOT(ShowContextMenuObstacle(const QPoint &)));

    displayOptions = new Ui::viewOptionDialog();
    aboutPanel = new Ui::aboutDialog();
    showStats = new Ui::statisticsDialog();

    displayOptions->setupUi(displayDialog = new QDialog());
    aboutPanel->setupUi(about = new QDialog());
    showStats->setupUi(statsDialog = new QDialog());
    rocWidget = new QNamedWindow("ROC Curve", false, showStats->rocWidget);
    crossvalidWidget = new QNamedWindow("Cross Validation", false, showStats->crossvalidWidget);
    infoWidget = new QNamedWindow("Info", false, showStats->informationWidget);

    connect(showStats->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(StatsChanged()));
    connect(rocWidget, SIGNAL(ResizeEvent(QResizeEvent *)), this, SLOT(StatsChanged()));
    connect(crossvalidWidget, SIGNAL(ResizeEvent(QResizeEvent *)), this, SLOT(StatsChanged()));
    connect(infoWidget, SIGNAL(ResizeEvent(QResizeEvent *)), this, SLOT(StatsChanged()));

    connect(drawToolbar->singleButton, SIGNAL(clicked()), this, SLOT(DrawSingle()));
    connect(drawToolbar->sprayButton, SIGNAL(clicked()), this, SLOT(DrawSpray()));
    connect(drawToolbar->lineButton, SIGNAL(clicked()), this, SLOT(DrawLine()));
    connect(drawToolbar->ellipseButton, SIGNAL(clicked()), this, SLOT(DrawEllipse()));
    connect(drawToolbar->eraseButton, SIGNAL(clicked()), this, SLOT(DrawErase()));
    connect(drawToolbar->trajectoryButton, SIGNAL(clicked()), this, SLOT(DrawTrajectory()));
    connect(drawToolbar->obstacleButton, SIGNAL(clicked()), this, SLOT(DrawObstacle()));

    connect(displayOptions->clipboardButton, SIGNAL(clicked()), this, SLOT(ToClipboard()));
    connect(displayOptions->mapCheck, SIGNAL(clicked()), this, SLOT(DisplayOptionChanged()));
    connect(displayOptions->modelCheck, SIGNAL(clicked()), this, SLOT(DisplayOptionChanged()));
    connect(displayOptions->infoCheck, SIGNAL(clicked()), this, SLOT(DisplayOptionChanged()));
    connect(displayOptions->samplesCheck, SIGNAL(clicked()), this, SLOT(DisplayOptionChanged()));
    connect(displayOptions->gridCheck, SIGNAL(clicked()), this, SLOT(DisplayOptionChanged()));
    connect(displayOptions->spinZoom, SIGNAL(valueChanged(double)), this, SLOT(DisplayOptionChanged()));
    connect(displayOptions->zoomFitButton, SIGNAL(clicked()), this, SLOT(FitToData()));
    //	connect(displayOptions->trajectoriesCheck, SIGNAL(clicked()), this, SLOT(DisplayOptionChanged()));
    //	connect(displayOptions->singleclassCheck, SIGNAL(clicked()), this, SLOT(DisplayOptionChanged()));


    algorithmOptions = new Ui::algorithmOptions();
    optionsClassify = new Ui::optionsClassifyWidget();
    optionsCluster = new Ui::optionsClusterWidget();
    optionsRegress = new Ui::optionsRegressWidget();
    optionsDynamic = new Ui::optionsDynamicWidget();

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
    drawToolbarWidget->setFixedSize(drawToolbarWidget->size());

    classifyWidget = new QWidget(algorithmOptions->tabClass);
    clusterWidget = new QWidget(algorithmOptions->tabClust);
    regressWidget = new QWidget(algorithmOptions->tabRegr);
    dynamicWidget = new QWidget(algorithmOptions->tabDyn);
    optionsClassify->setupUi(classifyWidget);
    optionsCluster->setupUi(clusterWidget);
    optionsRegress->setupUi(regressWidget);
    optionsDynamic->setupUi(dynamicWidget);

    connect(displayDialog, SIGNAL(rejected()), this, SLOT(HideOptionDisplay()));
    connect(statsDialog, SIGNAL(rejected()), this, SLOT(HideStatsDialog()));

    connect(optionsClassify->classifyButton, SIGNAL(clicked()), this, SLOT(Classify()));
    connect(optionsClassify->clearButton, SIGNAL(clicked()), this, SLOT(Clear()));
    connect(optionsClassify->rocButton, SIGNAL(clicked()), this, SLOT(ShowRoc()));
    connect(optionsClassify->crossValidButton, SIGNAL(clicked()), this, SLOT(ClassifyCross()));

    connect(optionsRegress->regressionButton, SIGNAL(clicked()), this, SLOT(Regression()));
    connect(optionsRegress->crossValidButton, SIGNAL(clicked()), this, SLOT(RegressionCross()));
    connect(optionsRegress->clearButton, SIGNAL(clicked()), this, SLOT(Clear()));
    //connect(optionsRegress->svmTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeActiveOptions()));

    connect(optionsCluster->clusterButton, SIGNAL(clicked()), this, SLOT(Cluster()));
    connect(optionsCluster->iterationButton, SIGNAL(clicked()), this, SLOT(ClusterIterate()));
    connect(optionsCluster->clearButton, SIGNAL(clicked()), this, SLOT(Clear()));

    connect(optionsDynamic->regressionButton, SIGNAL(clicked()), this, SLOT(Dynamize()));
    connect(optionsDynamic->clearButton, SIGNAL(clicked()), this, SLOT(Clear()));
    connect(optionsDynamic->centerCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeActiveOptions()));
    connect(optionsDynamic->resampleCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeActiveOptions()));
    connect(optionsDynamic->resampleSpin, SIGNAL(valueChanged(int)), this, SLOT(ChangeActiveOptions()));
	connect(optionsDynamic->dtSpin, SIGNAL(valueChanged(double)), this, SLOT(ChangeActiveOptions()));
	connect(optionsDynamic->obstacleCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(AvoidOptionChanged()));

    optionsClassify->tabWidget->clear();
    optionsCluster->tabWidget->clear();
    optionsRegress->tabWidget->clear();
    optionsDynamic->tabWidget->clear();
    optionsClassify->tabWidget->setUsesScrollButtons(true);
    optionsCluster->tabWidget->setUsesScrollButtons(true);
    optionsRegress->tabWidget->setUsesScrollButtons(true);
    optionsDynamic->tabWidget->setUsesScrollButtons(true);

	//canvas = new Canvas(ui.centralWidget);
	canvas = new Canvas(ui.canvasWidget);
    connect(canvas, SIGNAL(Drawing(fvec,int)), this, SLOT(Drawing(fvec,int)));
    connect(canvas, SIGNAL(DrawCrosshair()), this, SLOT(DrawCrosshair()));
    connect(canvas, SIGNAL(Navigation(fvec)), this, SLOT(Navigation(fvec)));
    connect(canvas, SIGNAL(Released()), this, SLOT(DrawingStopped()));
    connect(canvas, SIGNAL(CanvasMoveEvent()), this, SLOT(CanvasMoveEvent()));
    //connect(canvas, SIGNAL(ZoomChanged()), this, SLOT(ZoomChanged()));
    drawTimer = new DrawTimer(canvas, &mutex);
    drawTimer->classifier = &classifier;
    drawTimer->regressor = &regressor;
    drawTimer->dynamical = &dynamical;
    drawTimer->clusterer = &clusterer;
    connect(drawTimer, SIGNAL(MapReady(QImage)), canvas, SLOT(SetConfidenceMap(QImage)));
    connect(drawTimer, SIGNAL(ModelReady(QImage)), canvas, SLOT(SetModelImage(QImage)));
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
		if(pluginsDir.cd("plugins"))
        {
            alternativeDir = pluginsDir;
			pluginsDir.cdUp();
		}
		pluginsDir.cdUp();
		pluginsDir.cdUp();
		pluginsDir.cdUp();
	}
#endif
    bool bFoundPlugins = false;
#if defined(DEBUG)
    bFoundPlugins = pluginsDir.cd("pluginsDebug");
#else
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
        QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
        QObject *plugin = loader.instance();
        if (plugin)
        {
            qDebug() << fileName;
            // check type of plugin
            CollectionInterface *iCollection = qobject_cast<CollectionInterface *>(plugin);
            if(iCollection)
            {
                std::vector<ClassifierInterface*> classifierList = iCollection->GetClassifiers();
                std::vector<ClustererInterface*> clustererList = iCollection->GetClusterers();
                std::vector<RegressorInterface*> regressorList = iCollection->GetRegressors();
                std::vector<DynamicalInterface*> dynamicalList = iCollection->GetDynamicals();
                FOR(i, classifierList.size()) AddPlugin(classifierList[i], SLOT(ChangeActiveOptions));
                FOR(i, clustererList.size()) AddPlugin(clustererList[i], SLOT(ChangeActiveOptions));
                FOR(i, regressorList.size()) AddPlugin(regressorList[i], SLOT(ChangeActiveOptions));
                FOR(i, dynamicalList.size()) AddPlugin(dynamicalList[i], SLOT(ChangeActiveOptions));
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
    }
}

void MLDemos::ShowContextMenuSpray(const QPoint &point)
{
    QPoint pt = QPoint(30, 0);
    drawContext1Widget->move(drawToolbar->sprayButton->mapToGlobal(pt));
    drawContext1Widget->show();
    drawContext1Widget->setFocus();
}
void MLDemos::ShowContextMenuLine(const QPoint &point)
{
    QPoint pt = QPoint(30, 0);
    drawContext2Widget->move(drawToolbar->lineButton->mapToGlobal(pt));
    drawContext2Widget->show();
    drawContext2Widget->setFocus();
}
void MLDemos::ShowContextMenuEllipse(const QPoint &point)
{
    QPoint pt = QPoint(30, 0);
    drawContext2Widget->move(drawToolbar->ellipseButton->mapToGlobal(pt));
    drawContext2Widget->show();
    drawContext2Widget->setFocus();
}
void MLDemos::ShowContextMenuErase(const QPoint &point)
{
    QPoint pt = QPoint(30, 0);
    drawContext1Widget->move(drawToolbar->eraseButton->mapToGlobal(pt));
    drawContext1Widget->show();
    drawContext1Widget->setFocus();
}
void MLDemos::ShowContextMenuObstacle(const QPoint &point)
{
    QPoint pt = QPoint(30, 0);
    drawContext3Widget->move(drawToolbar->obstacleButton->mapToGlobal(pt));
    drawContext3Widget->show();
    drawContext3Widget->setFocus();
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
    if(drawContext1Widget->isVisible())
    {
        if(!IsChildOf(now, drawContext1Widget)) HideContextMenus();
    }
    if(drawContext2Widget->isVisible())
    {
        if(!IsChildOf(now, drawContext2Widget)) HideContextMenus();
    }
    if(drawContext3Widget->isVisible())
    {
        if(!IsChildOf(now, drawContext3Widget)) HideContextMenus();
    }
}

void MLDemos::HideContextMenus()
{
    drawContext1Widget->hide();
    drawContext2Widget->hide();
    drawContext3Widget->hide();
}

void MLDemos::AddPlugin(InputOutputInterface *iIO)
{
    inputoutputs.push_back(iIO);
    bInputRunning.push_back(false);
    connect(this, SIGNAL(SendResults(std::vector<fvec>)), iIO->object(), iIO->FetchResultsSlot());
    connect(iIO->object(), iIO->SetDataSignal(), this, SLOT(SetData(std::vector<fvec>, ivec, std::vector<ipair>)));
    connect(iIO->object(), iIO->QueryClassifierSignal(), this, SLOT(QueryClassifier(std::vector<fvec>)));
    connect(iIO->object(), iIO->QueryRegressorSignal(), this, SLOT(QueryRegressor(std::vector<fvec>)));
    connect(iIO->object(), iIO->QueryDynamicalSignal(), this, SLOT(QueryDynamical(std::vector<fvec>)));
    connect(iIO->object(), iIO->QueryClustererSignal(), this, SLOT(QueryClusterer(std::vector<fvec>)));
    connect(iIO->object(), iIO->DoneSignal(), this, SLOT(DisactivateIO(QObject *)));
    QString name = iIO->GetName();
    QAction *pluginAction = ui.menuInput_Output->addAction(name);
    pluginAction->setCheckable(true);
    pluginAction->setChecked(false);
    connect(pluginAction,SIGNAL(toggled(bool)), this, SLOT(ActivateIO()));
}

void MLDemos::AddPlugin(ClassifierInterface *iClassifier, const char *method)
{
    if(!iClassifier) return;
    // we add the interface so we can use it to produce classifiers
    classifiers.push_back(iClassifier);
    // we add the classifier parameters to the gui
    optionsClassify->tabWidget->addTab(iClassifier->GetParameterWidget(), iClassifier->GetName());
}

void MLDemos::AddPlugin(ClustererInterface *iCluster, const char *method)
{
    if(!iCluster) return;
    clusterers.push_back(iCluster);
    optionsCluster->tabWidget->addTab(iCluster->GetParameterWidget(), iCluster->GetName());
}

void MLDemos::AddPlugin(RegressorInterface *iRegress, const char *method)
{
    if(!iRegress) return;
    regressors.push_back(iRegress);
    optionsRegress->tabWidget->addTab(iRegress->GetParameterWidget(), iRegress->GetName());
}

void MLDemos::AddPlugin(DynamicalInterface *iDynamical, const char *method)
{
    if(!iDynamical) return;
    dynamicals.push_back(iDynamical);
    optionsDynamic->tabWidget->addTab(iDynamical->GetParameterWidget(), iDynamical->GetName());
}

void MLDemos::AddPlugin(AvoidanceInterface *iAvoid, const char *method)
{
    if(!iAvoid) return;
    avoiders.push_back(iAvoid);
    optionsDynamic->obstacleCombo->addItem(iAvoid->GetName());
}

MLDemos::~MLDemos()
{
    Clear();
    FOR(i, inputoutputs.size())
    {
        if(inputoutputs[i] && bInputRunning[i]) inputoutputs[i]->Stop();
    }
    SaveLayoutOptions();
    delete optionsClassify;
    delete optionsRegress;
    delete optionsCluster;
    delete optionsDynamic;
    delete drawToolbar;
    delete drawToolbarContext1;
    delete drawToolbarContext2;
    delete displayOptions;

    canvas->hide();
    delete canvas;
}

void MLDemos::closeEvent(QCloseEvent *event)
{
    if (true)
    {
        mutex.lock();
        DEL(regressor);
        DEL(classifier);
        mutex.unlock();
        qApp->quit();
    } else {
        event->ignore();
    }
}

void MLDemos::resizeEvent( QResizeEvent *event )
{
#ifdef MACX // ugly hack to avoid resizing problems on the mac
    //	qDebug() << "resizing: " << ui.centralWidget->geometry() << endl;
	// if(height() < 600) resize(width(),600);
#endif // MACX
    if(canvas)
    {
		canvas->resize(ui.canvasWidget->width(), ui.canvasWidget->height());
		canvas->ResizeEvent();
    }
	CanvasMoveEvent();
}

void MLDemos::ShowOptionClass()
{
    if(actionClassifiers->isChecked())
    {
        algorithmOptions->tabWidget->setCurrentWidget(algorithmOptions->tabClass);
        algorithmWidget->show();
    }
    else algorithmWidget->hide();
    actionClustering->setChecked(false);
    actionRegression->setChecked(false);
    actionDynamical->setChecked(false);
}

void MLDemos::ShowOptionRegress()
{
    if(actionRegression->isChecked())
    {
        algorithmOptions->tabWidget->setCurrentWidget(algorithmOptions->tabRegr);
        algorithmWidget->show();
    }
    else algorithmWidget->hide();
    actionClustering->setChecked(false);
    actionClassifiers->setChecked(false);
    actionDynamical->setChecked(false);
}

void MLDemos::ShowOptionDynamical()
{
    if(actionDynamical->isChecked())
    {
        algorithmOptions->tabWidget->setCurrentWidget(algorithmOptions->tabDyn);
        algorithmWidget->show();
    }
    else algorithmWidget->hide();
    actionClustering->setChecked(false);
    actionClassifiers->setChecked(false);
    actionRegression->setChecked(false);
}

void MLDemos::ShowOptionCluster()
{
    if(actionClustering->isChecked())
    {
        algorithmOptions->tabWidget->setCurrentWidget(algorithmOptions->tabClust);
        algorithmWidget->show();
    }
    else algorithmWidget->hide();
    actionClassifiers->setChecked(false);
    actionRegression->setChecked(false);
    actionDynamical->setChecked(false);
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

void MLDemos::ShowOptionDisplay()
{
    if(actionDisplayOptions->isChecked()) displayDialog->show();
    else displayDialog->hide();
}

void MLDemos::ShowStatsDialog()
{
    if(actionShowStats->isChecked()) statsDialog->show();
    else statsDialog->hide();
}

void MLDemos::ShowAbout()
{
    about->show();
}

void MLDemos::HideOptionClass()
{
    if(algorithmOptions->tabClass->isVisible()) algorithmWidget->hide();
    actionClassifiers->setChecked(false);
}

void MLDemos::HideOptionRegress()
{
    if(algorithmOptions->tabRegr->isVisible()) algorithmWidget->hide();
    actionRegression->setChecked(false);
}

void MLDemos::HideOptionDynamical()
{
    if(algorithmOptions->tabDyn->isVisible()) algorithmWidget->hide();
    actionDynamical->setChecked(false);
}

void MLDemos::HideOptionCluster()
{
    if(algorithmOptions->tabClust->isVisible()) algorithmWidget->hide();
    actionClustering->setChecked(false);
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

void MLDemos::HideStatsDialog()
{
    statsDialog->hide();
    actionShowStats->setChecked(false);
}


void MLDemos::Clear()
{
    drawTimer->Stop();
    drawTimer->Clear();
    QMutexLocker lock(&mutex);
    DEL(classifier);
    DEL(regressor);
    DEL(dynamical);
    DEL(clusterer);
    canvas->confidencePixmap = QPixmap();
    canvas->modelPixmap = QPixmap();
    canvas->infoPixmap = QPixmap();
    canvas->liveTrajectory.clear();
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
    optionsClassify->positiveSpin->setRange(labMin,labMax);
    if(optionsClassify->positiveSpin->value() < labMin)
        optionsClassify->positiveSpin->setValue(labMin);
    else if(optionsClassify->positiveSpin->value() > labMax)
        optionsClassify->positiveSpin->setValue(labMax);
}

void MLDemos::ChangeActiveOptions()
{
    DisplayOptionChanged();
}

void MLDemos::ClearData()
{
    if(canvas)
    {
        canvas->data->Clear();
    }
    Clear();
    ResetPositiveClass();
    UpdateInfo();
}

void MLDemos::DrawSingle()
{
    if(drawToolbar->singleButton->isChecked())
    {
        drawToolbar->sprayButton->setChecked(false);
        drawToolbar->eraseButton->setChecked(false);
        drawToolbar->ellipseButton->setChecked(false);
        drawToolbar->lineButton->setChecked(false);
        drawToolbar->trajectoryButton->setChecked(false);
        drawToolbar->obstacleButton->setChecked(false);
    }
}

void MLDemos::DrawSpray()
{
    if(drawToolbar->sprayButton->isChecked())
    {
        drawToolbar->singleButton->setChecked(false);
        drawToolbar->eraseButton->setChecked(false);
        drawToolbar->ellipseButton->setChecked(false);
        drawToolbar->lineButton->setChecked(false);
        drawToolbar->trajectoryButton->setChecked(false);
        drawToolbar->obstacleButton->setChecked(false);
    }
}

void MLDemos::DrawErase()
{
    if(drawToolbar->eraseButton->isChecked())
    {
        drawToolbar->singleButton->setChecked(false);
        drawToolbar->sprayButton->setChecked(false);
        drawToolbar->ellipseButton->setChecked(false);
        drawToolbar->lineButton->setChecked(false);
        drawToolbar->trajectoryButton->setChecked(false);
        drawToolbar->obstacleButton->setChecked(false);
    }
}

void MLDemos::DrawLine()
{
    if(drawToolbar->lineButton->isChecked())
    {
        drawToolbar->singleButton->setChecked(false);
        drawToolbar->sprayButton->setChecked(false);
        drawToolbar->ellipseButton->setChecked(false);
        drawToolbar->eraseButton->setChecked(false);
        drawToolbar->trajectoryButton->setChecked(false);
        drawToolbar->obstacleButton->setChecked(false);
    }
}

void MLDemos::DrawEllipse()
{
    if(drawToolbar->ellipseButton->isChecked())
    {
        drawToolbar->singleButton->setChecked(false);
        drawToolbar->sprayButton->setChecked(false);
        drawToolbar->eraseButton->setChecked(false);
        drawToolbar->lineButton->setChecked(false);
        drawToolbar->trajectoryButton->setChecked(false);
        drawToolbar->obstacleButton->setChecked(false);
    }
}

void MLDemos::DrawTrajectory()
{
    if(drawToolbar->trajectoryButton->isChecked())
    {
        drawToolbar->singleButton->setChecked(false);
        drawToolbar->sprayButton->setChecked(false);
        drawToolbar->eraseButton->setChecked(false);
        drawToolbar->lineButton->setChecked(false);
        drawToolbar->ellipseButton->setChecked(false);
        drawToolbar->obstacleButton->setChecked(false);
    }
}

void MLDemos::DrawObstacle()
{
    if(drawToolbar->obstacleButton->isChecked())
    {
        drawToolbar->eraseButton->setChecked(false);
        drawToolbar->singleButton->setChecked(false);
        drawToolbar->sprayButton->setChecked(false);
        drawToolbar->ellipseButton->setChecked(false);
        drawToolbar->lineButton->setChecked(false);
        drawToolbar->trajectoryButton->setChecked(false);
    }
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

void MLDemos::DisplayOptionChanged()
{
    if(!canvas) return;
    canvas->bDisplayInfo = displayOptions->infoCheck->isChecked();
    canvas->bDisplayLearned = displayOptions->modelCheck->isChecked();
    canvas->bDisplayMap = displayOptions->mapCheck->isChecked();
    canvas->bDisplaySamples = displayOptions->samplesCheck->isChecked();
    canvas->bDisplayTrajectories = displayOptions->samplesCheck->isChecked();
    canvas->bDisplayGrid = displayOptions->gridCheck->isChecked();
    float zoom = displayOptions->spinZoom->value();
    if(zoom >= 0.f) zoom += 1.f;
    else zoom = 1.f / (fabs(zoom)+1.f);
    if(zoom != canvas->GetZoom())
    {
        drawTimer->Stop();
        drawTimer->Clear();
        canvas->SetZoom(zoom);
        QMutexLocker lock(&mutex);
        if(classifier)
        {
            classifiers[tabUsedForTraining]->Draw(canvas, classifier);
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
        canvas->repaint();
    }
    //	canvas->bDisplayTrajectories = displayOptions->trajectoriesCheck->isChecked();
    if(optionsDynamic)
    {
        canvas->trajectoryCenterType = optionsDynamic->centerCombo->currentIndex();
        canvas->trajectoryResampleType = optionsDynamic->resampleCombo->currentIndex();
        canvas->trajectoryResampleCount = optionsDynamic->resampleSpin->value();
    }
    canvas->ResetSamples();
    canvas->repaint();
}

void MLDemos::DrawCrosshair()
{
    int drawType = 0;
    if(drawToolbar->singleButton->isChecked()) drawType = 1;
    if(drawToolbar->sprayButton->isChecked()) drawType = 2;
    if(drawToolbar->eraseButton->isChecked()) drawType = 3;
    if(drawToolbar->ellipseButton->isChecked()) drawType = 4;
    if(drawToolbar->lineButton->isChecked()) drawType = 5;
    if(drawToolbar->trajectoryButton->isChecked()) drawType = 6;
    if(drawToolbar->obstacleButton->isChecked()) drawType = 7;

    if(!drawType || drawType == 1 || drawType == 6)
    {
        canvas->crosshair = QPainterPath();
        canvas->bNewCrosshair = false;
        return;
    }
    int type = drawToolbarContext1->randCombo->currentIndex();
    float aX = drawToolbarContext2->spinSigmaX->value();
    float aY = drawToolbarContext2->spinSigmaY->value();
    float angle = -drawToolbarContext2->spinAngle->value()/180.f*PIf;
    float s = drawToolbarContext1->spinSize->value();
    int size = (int)(s*canvas->height());
    int sizeX = (int)(aX*canvas->height());
    int Size = canvas->height();

    QPainterPath cursor;

    float sin_angle = sinf(angle);
    float cos_angle = cosf(angle);

    if(drawType == 5) // line
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
    if(drawType == 2) // spray
    {
        cursor.addEllipse(QPoint(0,0),size/2,size/2);
        canvas->crosshair = cursor;
        canvas->bNewCrosshair = false;
        return;
    }

    if(drawType == 3) // erase
    {
        cursor.addEllipse(QPoint(0,0),size/2,size/2);
        canvas->crosshair = cursor;
        canvas->bNewCrosshair = false;
        return;
    }

    if(drawType == 7) // obstacles
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
    int drawType = 0; // none
    if(drawToolbar->singleButton->isChecked()) drawType = 1;
    if(drawToolbar->sprayButton->isChecked()) drawType = 2;
    if(drawToolbar->eraseButton->isChecked()) drawType = 3;
    if(drawToolbar->ellipseButton->isChecked()) drawType = 4;
    if(drawToolbar->lineButton->isChecked()) drawType = 5;
    if(drawToolbar->trajectoryButton->isChecked()) drawType = 6;
    if(drawToolbar->obstacleButton->isChecked()) drawType = 7;
    if(!drawType) return;

    int speed = 6;

    if(label) label = drawToolbar->classSpin->value();

    if(drawType == 1) // single samples
    {
        // we don't want to draw too often
        if(drawTime.elapsed() < 50/speed) return; // msec elapsed since last drawing
        canvas->data->AddSample(sample, label);
        canvas->repaint();
    }
    else if(drawType == 2) // spray samples
    {
        // we don't want to draw too often
        if(drawTime.elapsed() < 200/speed) return; // msec elapsed since last drawing
        int type = drawToolbarContext1->randCombo->currentIndex();
        float s = drawToolbarContext1->spinSize->value();
        float size = s*canvas->height();
        int count = drawToolbarContext1->spinCount->value();

        QPointF sampleCoords = canvas->toCanvasCoords(sample);
        if(type == 0) // uniform
        {
            fvec newSample;
            newSample.resize(2,0);
            FOR(i, count)
            {
                newSample[0] = (rand()/(float)RAND_MAX - 0.5f)*size + sampleCoords.x();
                newSample[1] = (rand()/(float)RAND_MAX - 0.5f)*size + sampleCoords.y();
                canvas->data->AddSample(canvas->toSampleCoords(newSample[0],newSample[1]), label);
            }
        }
        else // normal
        {
            // we generate the new data
            gaussian gauss;
            gaussian_init(&gauss,2);
            gauss.mean[0] = sampleCoords.x();
            gauss.mean[1] = sampleCoords.y();
            gauss.covar->_[0] = size*size/9.f*0.5f;
            gauss.covar->_[1] = 0;
            gauss.covar->_[2] = size*size/9.f*0.5f;
            smat_cholesky(gauss.covar, gauss.covar_cholesky);
            fvec newSample;
            newSample.resize(2,0);
            FOR(i, count)
            {
                gaussian_draw(&gauss, &newSample[0]);
                canvas->data->AddSample(canvas->toSampleCoords(newSample[0],newSample[1]), label);
            }
        }
        canvas->repaint();
    }
    else if(drawType == 3) // erase
    {
        float s = drawToolbarContext1->spinSize->value();
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
        canvas->repaint();
    }
    else if(drawType == 4) // ellipse
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
        canvas->repaint();
    }
    else if(drawType == 5) // line
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
        canvas->repaint();
    }
    else if(drawType == 6) // trajectory
    {
        if(trajectory.first == -1) // we're starting a trajectory
        {
            trajectory.first = canvas->data->GetCount();
        }
        // we don't want to draw too often
        //if(drawTime.elapsed() < 50/speed) return; // msec elapsed since last drawing
        canvas->data->AddSample(sample, label, _TRAJ);
        trajectory.second = canvas->data->GetCount()-1;
        canvas->repaint();
    }
    else if(drawType == 7) // obstacle
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
    drawTime.restart();
    ResetPositiveClass();
    UpdateInfo();
}

void MLDemos::DrawingStopped()
{
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

void MLDemos::FitToData()
{
    canvas->FitToData();
    float zoom = canvas->GetZoom();
    if(zoom >= 1) zoom -=1;
    else zoom = 1/(-zoom) - 1;
    if(zoom == displayOptions->spinZoom->value()) return; // nothing to be done!
    displayOptions->spinZoom->blockSignals(true);
    displayOptions->spinZoom->setValue(zoom);
    displayOptions->spinZoom->blockSignals(false);
    drawTimer->Stop();
    drawTimer->Clear();
    QMutexLocker lock(&mutex);
    if(classifier)
    {
        classifiers[tabUsedForTraining]->Draw(canvas, classifier);
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
    canvas->repaint();
}

void MLDemos::CanvasMoveEvent()
{
    drawTimer->Stop();
    drawTimer->Clear();
    QMutexLocker lock(&mutex);
    if(classifier)
    {
        classifiers[tabUsedForTraining]->Draw(canvas, classifier);
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
    canvas->repaint();
}

void MLDemos::ZoomChanged(float d)
{
    displayOptions->spinZoom->setValue(displayOptions->spinZoom->value()+d/4);
}

void MLDemos::Navigation( fvec sample )
{
    if(sample[0]==-1)
    {
        ZoomChanged(sample[1]);
        return;
    }
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
    sprintf(string, " | x: %.3f y: %.3f", sample[0], sample[1]);
    information += QString(string);
    QMutexLocker lock(&mutex);
    if(classifier)
    {
        float score = classifier->Test(sample);
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
    ui.statusBar->showMessage(information);
}

void MLDemos::SaveData()
{
    if(!canvas) return;
    QString filename = QFileDialog::getSaveFileName(this, tr("Save Data"), "", tr("ML Files (*.ml)"));
    if(filename.isEmpty()) return;
    if(!filename.endsWith(".ml")) filename += ".ml";
    Save(filename);
}
void MLDemos :: Save(QString filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly))
    {
        ui.statusBar->showMessage("WARNING: Unable to save file");
        return;
    }
    file.close();
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
    LoadParams(filename);
    ui.statusBar->showMessage("Data loaded successfully");
    ResetPositiveClass();
    UpdateInfo();
    canvas->repaint();
}

void MLDemos::dragEnterEvent(QDragEnterEvent *event)
{
    QList<QUrl> dragUrl;
    if(event->mimeData()->hasUrls())
    {
        QList<QUrl> urls = event->mimeData()->urls();
        QStringList dataType;
        dataType << ".ml";
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
        if(filename.toLower().endsWith(".ml"))
        {
            ClearData();
            canvas->data->Load(filename.toAscii());
            LoadParams(filename);
            ui.statusBar->showMessage("Data loaded successfully");
            ResetPositiveClass();
            UpdateInfo();
            canvas->repaint();
        }
    }
    event->acceptProposedAction();
}

void MLDemos::Screenshot()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Save Screenshot"), "", tr("Images (*.png *.jpg)"));
    if(filename.isEmpty()) return;
    if(!filename.endsWith(".jpg") && !filename.endsWith(".png")) filename += ".png";
    if(!canvas->SaveScreenshot(filename)) ui.statusBar->showMessage("WARNING: Unable to save image");
    else ui.statusBar->showMessage("Image saved successfully");
}

void MLDemos::ToClipboard()
{
    QPixmap screenshot = canvas->GetScreenshot();
    if(screenshot.isNull())
    {
        ui.statusBar->showMessage("WARNING: Nothing to copy to clipboard");
        return;
    }

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setImage(screenshot.toImage());
    clipboard->setPixmap(screenshot);
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
}

void MLDemos::SetData(std::vector<fvec> samples, ivec labels, std::vector<ipair> trajectories)
{
    canvas->data->Clear();
    canvas->data->AddSamples(samples, labels);
    if(trajectories.size())
    {
        canvas->data->AddSequences(trajectories);
    }
    canvas->ResetSamples();
    canvas->repaint();
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
            result[0] = classifier->Test(samples[i]);
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

