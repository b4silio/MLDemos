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
    QAction *actionAlgorithms, *actionDrawSamples, *actionCompare,
    *actionDisplayOptions, *actionShowStats, *actionAddData,
    *actionClearData, *actionClearModel, *actionClearAll, *actionScreenshot,
    *actionNew, *actionSave, *actionLoad, *actionGridsearch;

    ivec selectedData;
    fvec selectionWeights;
    fvec selectionStart;
    QMutex mutex;

    Ui::MLDemosClass ui;
    Ui::statisticsDialog *showStats;
    Ui::ManualSelection* manualSelection;
    Ui::InputDimensions* inputDimensions;
    QDialog *displayDialog, *aboutDialog, *statsDialog, *manualSelectDialog, *inputDimensionsDialog;

private:

    QNamedWindow *rocWidget;

	Ui::viewOptionDialog *displayOptions;
	Ui::aboutDialog *aboutPanel;
	Ui::DrawingToolbar *drawToolbar;
    Ui::DrawingToolbarContext1 *drawToolbarContext1;
	Ui::DrawingToolbarContext2 *drawToolbarContext2;
	Ui::DrawingToolbarContext3 *drawToolbarContext3;
	Ui::DrawingToolbarContext4 *drawToolbarContext4;
    QWidget *drawToolbarWidget;
	QWidget *drawContext1Widget, *drawContext2Widget, *drawContext3Widget, *drawContext4Widget;
	QToolBar *toolBar;

	DrawTimer *drawTimer;
	QTime drawTime;
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
    MLDemos(QString filename="", QWidget *parent = 0, Qt::WindowFlags flags = 0);
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
	void ShowAlgorithmOptions();

    void RestAlgorithmOptionsButton();

	void ShowOptionCompare();
	void ShowSampleDrawing();
	void ShowOptionDisplay();
	void ShowStatsDialog();
	void ShowToolbar();
    void ShowAddData();
    void ShowGridSearch();
    void ResetGridSearchButton();
    void HideSampleDrawing();
    void HideOptionDisplay();
    void HideOptionCompare();
    void HideStatsDialog();
	void HideToolbar();
    void HideAddData();
    void DisplayOptionsChanged();
    void Display3DOptionsChanged();

    void AddData();
    void Clear();
    void ClearData();
    void ClearAll();
    void ShowDataEditor();
    void SetROCInfo();

	void SaveData();
	void LoadData();
    void ImportData();
    void RewardFromMap(QImage rewardMap);
    void MapFromReward();
    void ExportOutput();
	void ExportSVG();
	void Screenshot();
	void ToClipboard();

	void DrawCrosshair();
	void Drawing(fvec sample, int label);
    void Editing(int editType, fvec position, int label);
	void DrawingStopped();
    void DimPlus();
    void DimLess();

    void ManualSelection();
    void InputDimensions();
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

	void ShowContextMenuSpray(const QPoint &point);
	void ShowContextMenuLine(const QPoint &point);
	void ShowContextMenuEllipse(const QPoint &point);
	void ShowContextMenuErase(const QPoint &point);
	void ShowContextMenuObstacle(const QPoint &point);
	void ShowContextMenuReward(const QPoint &point);
	void FocusChanged(QWidget *old, QWidget *now);
	void HideContextMenus();
};

#endif // MLDEMOS_H
