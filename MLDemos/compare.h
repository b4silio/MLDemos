#ifndef _COMPARE_H_
#define _COMPARE_H_

#include <public.h>
#include <mymaths.h>
#include <drawUtils.h>
#include <QLabel>
#include "ui_compare.h"

class CompareAlgorithms : public QObject
{
	Q_OBJECT

	std::map< QString, std::vector< fvec > > results;
	std::map< QString, QStringList > algorithms;
	Ui::CompareDisplay *compareDisplay;
	QLabel *display;
	QPixmap pixmap;
public:
    QWidget *compareWidget;

public:
    CompareAlgorithms(QWidget *parent=0);
	~CompareAlgorithms();
	void AddResults(fvec results, QString name, QString algorithm);
	void Show();
	void Clear();
    void SetActiveResult(int index);
    QPixmap &Display(){return pixmap;}
    QString ToString();

public slots:
	void Update();
};

#endif // _COMPARE_H_
