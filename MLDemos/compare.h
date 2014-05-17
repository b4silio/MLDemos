#ifndef _COMPARE_H_
#define _COMPARE_H_

#include <public.h>
#include <mymaths.h>
#include <drawUtils.h>
#include <canvas.h>
#include <QLabel>
#include "basewidget.h"
#include "ui_compare.h"
#include "ui_optsCompare.h"

class CompareAlgorithms : public QObject
{
	Q_OBJECT

	std::map< QString, std::vector< fvec > > results;
	std::map< QString, QStringList > algorithms;
	Ui::CompareDisplay *compareDisplay;
	QLabel *display;
	QPixmap pixmap;
    Canvas *canvas;

public:
    BaseWidget *compareWidget;
    BaseWidget *paramsWidget;
    Ui::optionsCompare *params;
    QList<QString> compareOptions;
    std::vector<fvec> datasetA;
    std::vector<fvec> datasetB;
    ivec labelsA;
    ivec labelsB;

public:
    CompareAlgorithms(Canvas *canvas);
	~CompareAlgorithms();
	void AddResults(fvec results, QString name, QString algorithm);
	void Show();
	void Clear();
    void SetActiveResult(int index);
    QString ToString();
    void Add(QString parameterData, QString displayString);

    bool eventFilter(QObject *obj, QEvent *event);

public slots:
	void Update();
    void CompareScreenshot();
    void CompareClear();
    void CompareRemove();
    void CompareSave();
    void CompareLoad();
    void CompareSetTrain();
    void CompareSetTest();
    void CompareResetTrain();
    void CompareResetTest();
};

#endif // _COMPARE_H_
