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
#include <QtGui/QMainWindow>
#include <QTime>
#include <QResizeEvent>
#include <QMutex>
#include <QMutexLocker>
#include "ui_mldemos.h"
#include "ui_viewOptions.h"
#include "ui_aboutDialog.h"
#include "ui_algorithmOptions.h"
#include "ui_optsClassify.h"
#include "ui_optsCluster.h"
#include "ui_optsRegress.h"
#include "ui_optsMaximize.h"
#include "ui_optsDynamic.h"
#include "ui_optsCompare.h"
#include "ui_statisticsDialog.h"
#include "ui_drawingTools.h"
#include "ui_drawingToolsContext1.h"
#include "ui_drawingToolsContext2.h"
#include "ui_drawingToolsContext3.h"
#include "ui_drawingToolsContext4.h"

#include "canvas.h"
#include "classifier.h"
#include "regressor.h"
#include "dynamical.h"
#include "clusterer.h"
#include "interfaces.h"
#include "compare.h"
#include "widget.h"
#include "drawTimer.h"

class MLDemos : public QMainWindow
{
	Q_OBJECT

private:
	QAction *actionClassifiers, *actionRegression, *actionDynamical, *actionMaximizers,
	*actionClustering, *actionDrawSamples, *actionCompare,
	*actionDisplayOptions, *actionShowStats,
	*actionClearData, *actionClearModel, *actionScreenshot,
	*actionNew, *actionSave, *actionLoad;

	QDialog *displayDialog, *about, *statsDialog;

	QWidget *algorithmWidget, *regressWidget, *dynamicWidget, *classifyWidget, *clusterWidget, *maximizeWidget, *compareWidget;

	QNamedWindow *rocWidget, *crossvalidWidget, *infoWidget;

	Ui::MLDemosClass ui;
	Ui::viewOptionDialog *displayOptions;
	Ui::aboutDialog *aboutPanel;
	Ui::statisticsDialog *showStats;
	Ui::algorithmOptions *algorithmOptions;
	Ui::optionsClassifyWidget *optionsClassify;
	Ui::optionsClusterWidget *optionsCluster;
	Ui::optionsRegressWidget *optionsRegress;
	Ui::optionsMaximizeWidget *optionsMaximize;
	Ui::optionsDynamicWidget *optionsDynamic;
	Ui::optionsCompare *optionsCompare;
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
	ipair trajectory;
	Obstacle obstacle;
	bool bNewObstacle;


	void closeEvent(QCloseEvent *event);
	bool Train(Classifier *classifier, int positive, float trainRatio=1);
	void Draw(Classifier *classifier);
	void Train(Regressor *regressor, float trainRatio=1);
	void Draw(Regressor *regressor);
	fvec Train(Dynamical *dynamical);
	void Draw(Dynamical *dynamical);
	fvec Test(Dynamical *dynamical, std::vector< std::vector<fvec> > trajectories, ivec labels);
	void Train(Clusterer *clusterer);
	void Draw(Clusterer *clusterer);
	void Train(Maximizer *maximizer);
	void Draw(Maximizer *maximizer);
	void Test(Maximizer *maximizer);

	QList<ClassifierInterface *> classifiers;
	QList<ClustererInterface *> clusterers;
	QList<RegressorInterface *> regressors;
	QList<DynamicalInterface *> dynamicals;
	QList<AvoidanceInterface *> avoiders;
	QList<MaximizeInterface*> maximizers;
	QList<InputOutputInterface *> inputoutputs;
	QList<bool> bInputRunning;
	QList<QString> compareOptions;
	QLabel *compareDisplay;
	CompareAlgorithms *compare;
	void AddPlugin(ClassifierInterface *iClassifier, const char *method);
	void AddPlugin(ClustererInterface *iCluster, const char *method);
	void AddPlugin(RegressorInterface *iRegress, const char *method);
	void AddPlugin(DynamicalInterface *iDynamical, const char *method);
	void AddPlugin(AvoidanceInterface *iAvoid, const char *method);
	void AddPlugin(MaximizeInterface *iMaximize, const char *method);
	void AddPlugin(InputOutputInterface *iIO);

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

	void UpdateInfo();
	void SetCrossValidationInfo();
	bool bIsRocNew;
	bool bIsCrossNew;
public:
	MLDemos(QString filename="", QWidget *parent = 0, Qt::WFlags flags = 0);
	~MLDemos();

	int tabUsedForTraining;
	Classifier *classifier;
	Regressor *regressor;
	Dynamical *dynamical;
	Clusterer *clusterer;
	Maximizer *maximizer;
	QMutex mutex;
	void resizeEvent( QResizeEvent *event );
	void dragEnterEvent(QDragEnterEvent *event);
	void dropEvent(QDropEvent *event);

signals:
	void SendResults(std::vector<fvec> results);
public slots:
	void SetData(std::vector<fvec> samples, ivec labels, std::vector<ipair> trajectories);
	void SetTimeseries(std::vector<TimeSerie> timeseries);
	void QueryClassifier(std::vector<fvec> samples);
	void QueryRegressor(std::vector<fvec> samples);
	void QueryDynamical(std::vector<fvec> samples);
	void QueryClusterer(std::vector<fvec> samples);
	void QueryMaximizer(std::vector<fvec> samples);

private slots:
	void ShowAbout();
	void ShowOptionClass();
	void ShowOptionRegress();
	void ShowOptionDynamical();
	void ShowOptionCluster();
	void ShowOptionMaximize();
	void ShowOptionCompare();
	void ShowSampleDrawing();
	void ShowOptionDisplay();
	void ShowStatsDialog();
	void ShowToolbar();
	void HideOptionClass();
	void HideOptionRegress();
	void HideOptionDynamical();
	void HideOptionCluster();
	void HideOptionMaximize();
	void HideSampleDrawing();
	void HideOptionDisplay();
	void HideStatsDialog();
	void HideToolbar();
	void AvoidOptionChanged();
	void DisplayOptionChanged();
	void ColorMapChanged();
	void ActivateIO();
	void DisactivateIO(QObject *);

	void Classify();
	void ClassifyCross();
	void Regression();
	void RegressionCross();
	void Maximize();
	void MaximizeContinue();
	void Dynamize();
	void Cluster();
	void ClusterIterate();
	void Avoidance();
	void Compare();
	void CompareScreenshot();
	void Clear();
	void ClearData();
	void SetROCInfo();

	void SaveData();
	void LoadData();
	void ExportOutput();
	void ExportAnimation();
	void ExportSVG();
	void Screenshot();
	void ToClipboard();

	void DrawCrosshair();
	void DrawSingle();
	void DrawSpray();
	void DrawLine();
	void DrawTrajectory();
	void DrawEllipse();
	void DrawErase();
	void DrawObstacle();
	void DrawPaint();
	void Drawing(fvec sample, int label);
	void DrawingStopped();

	void FitToData();
	void ZoomChanged(float d);
	void CanvasMoveEvent();
	void Navigation(fvec sample);
	void ResetPositiveClass();
	void ChangeActiveOptions();
	void ShowRoc();
	void ShowCross();
	void MouseOnRoc(QMouseEvent *event);
	void StatsChanged();
	void AlgoChanged();
	void ChangeInfoFile();
	void TargetButton();
	void GaussianButton();
	void GradientButton();
	void BenchmarkButton();
	void CompareAdd();
	void CompareClear();
	void CompareRemove();


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
