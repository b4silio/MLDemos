#ifndef _DATA_IMPORTER_H_
#define _DATA_IMPORTER_H_

#include "parser.h"
#include <interfaces.h>
#include <QFileDialog>
#include <QTableView>
#include <QDebug>

namespace Ui {
    class DataImporterDialog;
}
class DataImporter : public QObject, public InputOutputInterface
{
	Q_OBJECT
	Q_INTERFACES(InputOutputInterface)

public:
	const char* QueryClassifierSignal() {return SIGNAL(QueryClassifier(std::vector<fvec>));}
	const char* QueryRegressorSignal() {return SIGNAL(QueryRegressor(std::vector<fvec>));}
	const char* QueryDynamicalSignal() {return SIGNAL(QueryDynamical(std::vector<fvec>));}
	const char* QueryClustererSignal() {return SIGNAL(QueryClusterer(std::vector<fvec>));}
	const char* QueryMaximizerSignal() {return SIGNAL(QueryMaximizer(std::vector<fvec>));}
    const char* SetDataSignal() {return SIGNAL(SetData(std::vector<fvec>, ivec, std::vector<ipair>, bool));}
	const char* SetTimeseriesSignal() {return SIGNAL(SetTimeseries(std::vector<TimeSerie>));}
	const char* FetchResultsSlot() {return SLOT(FetchResults(std::vector<fvec>));}
	const char* DoneSignal() {return SIGNAL(Done(QObject *));}
    QObject *object(){return this;}
    QString GetName(){return "DataImporter";}

	void Start();
	void Stop();
    QStringList GetHeaders(){return headers;}

    DataImporter();
    ~DataImporter();

private:
    Ui::DataImporterDialog *gui;
	QDialog *guiDialog;
    CSVParser *inputParser;
    QStringList headers;

    bool saveFile(const QString &filename, QIODevice *data);

signals:
	void Done(QObject *);
    void SetData(std::vector<fvec> samples, ivec labels, std::vector<ipair> trajectories, bool bProjected);
	void SetTimeseries(std::vector<TimeSerie> series);
    void SetDimensionNames(QStringList headers);
	void QueryClassifier(std::vector<fvec> samples);
	void QueryRegressor(std::vector<fvec> samples);
	void QueryDynamical(std::vector<fvec> samples);
	void QueryClusterer(std::vector<fvec> samples);
	void QueryMaximizer(std::vector<fvec> samples);
public slots:
	void FetchResults(std::vector<fvec> results);
	void Closing();
    void Parse(QString filename);
	void LoadFile();
    void SendData();
private slots:
    void classIgnoreChanged();
    void headerChanged();
    void classColumnChanged(int value);
};

#endif // _DATA_IMPORTER_H_
