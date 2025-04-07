#ifndef MLDEMOS_H
#define MLDEMOS_H

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
#include <QMainWindow>
#include <QtWidgets>
#include <QTime>
#include <QElapsedTimer>
#include <QResizeEvent>
#include <QMutex>
#include <QMutexLocker>
#include "ui_mldemos.h"
#include "ui_viewOptions.h"
#include "ui_aboutDialog.h"
#include "ui_statisticsDialog.h"
#include "ui_drawingTools.h"
#include "ui_drawingToolsContext1.h"
#include "ui_drawingToolsContext2.h"
#include "ui_drawingToolsContext3.h"
#include "ui_drawingToolsContext4.h"
#include "ui_manualSelection.h"
#include "ui_inputDimensions.h"
#include "ui_displayoptions.h"

#include "canvas.h"
#include "classifier.h"
#include "regressor.h"
#include "dynamical.h"
#include "clusterer.h"
#include "maximize.h"
#include "reinforcement.h"
#include "reinforcementProblem.h"
#include "interfaces.h"
#include "compare.h"
#include "widget.h"
#include "drawTimer.h"
#include "dataImporter.h"
#include "datagenerator.h"
#include "dataseteditor.h"
#include "gridsearch.h"
#include "glwidget.h"
#include "visualization.h"
#include "algorithmmanager.h"
#include "pluginmanager.h"

class MLDemos : public QMainWindow
{
	Q_OBJECT

public:
    ivec selectedData;
    fvec selectionWeights;
    fvec selectionStart;
    QMutex mutex;

    Ui::MLDemosClass ui;
    Ui::statisticsDialog *showStats;
    Ui::ManualSelection* manualSelection;
    Ui::InputDimensions* inputDimensions;
    Ui::DrawingToolbar *drawToolbar;
    QDialog *displayDialog, *aboutDialog, *statsDialog, *manualSelectDialog, *inputDimensionsDialog;

private:

    QNamedWindow *rocWidget;

    Ui::viewOptionDialog *viewOptions;
    Ui::aboutDialog *aboutPanel;
    Ui::DisplayOptions* displayOptions;
    Ui::DrawingToolbarContext1 *drawToolbarContext1;
	Ui::DrawingToolbarContext2 *drawToolbarContext2;
	Ui::DrawingToolbarContext3 *drawToolbarContext3;
	Ui::DrawingToolbarContext4 *drawToolbarContext4;
    QWidget *displayOptionWidget;
    QWidget *drawToolbarWidget;
	QWidget *drawContext1Widget, *drawContext2Widget, *drawContext3Widget, *drawContext4Widget;

	DrawTimer *drawTimer;
    QElapsedTimer drawTime;
	Canvas *canvas;
    GridSearch *gridSearch;
    GLWidget *glw;
    Visualization *vis;
    AlgorithmManager *algo;
    PluginManager *plugin;
    DataImporter *import;
    DataGenerator *generator;
    DatasetEditor *dataEdit;
    ReinforcementProblem reinforcementProblem;
	ipair trajectory;
	Obstacle obstacle;
	bool bNewObstacle;
    QString lastTrainingInfo;

	CompareAlgorithms *compare;

	void initDialogs();
	void initToolBars();
	void initPlugins();
	void SaveLayoutOptions();
    void LoadLayoutOptions();
    void LoadAlgorithmsOptions();
    void SetTextFontSize();
	void SaveParams(QString filename);
	void LoadParams(QString filename);
	void Load(QString filename);
	void Save(QString filename);
    void ImportData(QString filename);

    std::vector<bool> GetManualSelection();
    ivec GetInputDimensions();
	void SetCrossValidationInfo();
	bool bIsRocNew;
	bool bIsCrossNew;
public:
    MLDemos(QString filename="", QWidget *parent = 0, Qt::WindowFlags flags = Qt::Widget);
	~MLDemos();

	void resizeEvent( QResizeEvent *event );
	void dragEnterEvent(QDragEnterEvent *event);
	void dropEvent(QDropEvent *event);
    void closeEvent(QCloseEvent *event);

public slots:
    void SetData(std::vector<fvec> samples, ivec labels, std::vector<ipair> trajectories, bool bProjected);
	void SetTimeseries(std::vector<TimeSerie> timeseries);
    void SetDimensionNames(QStringList headers);
    void SetClassNames(std::map<int,QString> classNames);
    void SetCategorical(std::map<int,std::vector<std::string> > categorical);
    void DataEdited();
    void UpdateInfo();
    void CanvasTypeChanged();
    void CanvasOptionsChanged();
    void Trained();

	void ShowAbout();
	void ShowOptionCompare();
	void ShowOptionDisplay();
	void ShowStatsDialog();
    void ToggleDataGenerator();
    void ShowGridSearch();
    void ResetGridSearchButton();
    void HideOptionDisplay();
    void HideOptionCompare();
    void HideGridSearch();
    void HideStatsDialog();
    void HideDataGenerator();
    void ShowEditingTools();
    void HideEditingTools();
    void DisplayOptionsChanged();
    void Display3DOptionsChanged();
    void DrawToolsChanged();
    void ResetMinimumWidth();
    void ClearPluginSelectionText();
    void AddPluginSelectionText(QString text);

    void AddDataFromGenerator();
    void Clear();
    void ClearData();
    void ClearAll();
    void ShowDataEditor();
    void SetROCInfo();

	void SaveData();
	void LoadData();
    void ImportData();
    void ReloadPlugins();
    void RewardFromMap(QImage rewardMap);
    void MapFromReward();
    void ExportOutput();
	void ExportSVG();
	void ScreenshotToFile();
	void ScreenshotToClipboard();

	void DrawCrosshair();
    void CrosshairChanged();
    void Drawing(fvec sample, int label);
    void Editing(int editType, fvec position, int label);
	void DrawingStopped();
    void DimPlus();
    void DimLess();

    void ShowManualSelection();
    void ShowInputDimensions();
    void FitToData();
	void ZoomChanged(float d);
	void CanvasMoveEvent();
	void Navigation(fvec sample);
	void ResetPositiveClass();
	void ChangeActiveOptions();
    void ShowRoc();
//	void ShowCross();
	void MouseOnRoc(QMouseEvent *event);
	void StatsChanged();
	void AlgoChanged();
	void ChangeInfoFile();
    void ManualSelectionUpdated();
    void ManualSelectionChanged();
    void ManualSelectionClear();
    void ManualSelectionInvert();
    void ManualSelectionRemove();
    void ManualSelectionRandom();
    void InputDimensionsUpdated();
    void InputDimensionsChanged();
    void InputDimensionsClear();
    void InputDimensionsInvert();
    void InputDimensionsRandom();
    void TargetButton();
    void ClearTargets();
	void GaussianButton();
	void GradientButton();
	void BenchmarkButton();
};

#endif // MLDEMOS_H
