/*********************************************************************
MLDemos: A User-Friendly visualization toolkit for machine learning
Copyright (C) 2011 Chrstophe Paccolat
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

#include "CSVImport.h"
#include "pcaprojection.h"

Q_EXPORT_PLUGIN2(IO_CSVImport, CSVImport)

CSVImport::CSVImport()
    : guiDialog(0), gui(0), inputParser(0)
{
}

CSVImport::~CSVImport()
{
    if(gui && guiDialog) guiDialog->hide();
    DEL(inputParser);
}

void CSVImport::Start()
{
    if(!gui)
    {
        gui = new Ui::CSVImportDialog();
        gui->setupUi(guiDialog = new QDialog());
        connect(gui->closeButton, SIGNAL(clicked()), this, SLOT(Closing()));
        connect(guiDialog, SIGNAL(finished(int)), this, SLOT(Closing()));
        connect(gui->spinBox, SIGNAL(valueChanged(int)), this, SLOT(spinBoxChanged(int)));
        connect(gui->loadFile, SIGNAL(clicked()), this, SLOT(LoadFile())); // file loader
        connect(gui->dumpButton, SIGNAL(clicked()),this,SLOT(on_dumpButton_clicked()));
//        connect(gui->pcaButton, SIGNAL(clicked()),this,SLOT(on_pcaButton_clicked()));
        guiDialog->show();
    }
    else guiDialog->show();
    if(!inputParser) inputParser = new CSVParser();
}

void CSVImport::Stop()
{
    guiDialog->hide();
}

void CSVImport::Closing()
{
    emit(Done(this));
}

bool CSVImport::saveFile(const QString &filename, QIODevice *data)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Could not open " << qPrintable(filename) << " for writing: " << qPrintable(file.errorString());
        return false;
    }

    file.write(data->readAll());
    file.close();

    return true;
}

void CSVImport::LoadFile()
{
	QString filename = QFileDialog::getOpenFileName(NULL, tr("Load Data"), QDir::currentPath(), tr("dataset files (*.data *.csv);;All files (*.*)"));
    if(filename.isEmpty()) return;
    Parse(filename);
    gui->tabWidget->setCurrentIndex(0);
}

void CSVImport::Parse(QString filename)
{
    if(filename.isEmpty()) return;
    inputParser->clear();
    inputParser->parse(filename.toStdString().c_str());
    vector<vector<string> > rawData = inputParser->getRawData();
    qDebug() << "Dataset extracted";
//    if(data.first.size() < 2) return;
    if(rawData.size() < 2) return;
    bool bUseHeader = gui->headerCheck->isChecked();

    gui->tableWidget->clear();
    gui->tableWidget->setRowCount(rawData.size());
    gui->tableWidget->setColumnCount(rawData[0].size());
    if(bUseHeader)
    {
        QStringList headerLabels;
        FOR(i, rawData[0].size())
        {
            headerLabels << rawData[0][i].c_str();
        }
        gui->tableWidget->setHorizontalHeaderLabels(headerLabels);
    }
    for(size_t r = 0; r < rawData.size(); r++)
    {
        if(!r && bUseHeader) continue;
        for(size_t c = 0; c < rawData[r].size(); c++)
        {
            QTableWidgetItem *newItem = new  QTableWidgetItem(QString(rawData[r][c].c_str()));
            gui->tableWidget->setItem(r-bUseHeader, c, newItem);
        }
    }
    gui->spinBox->setRange(1,rawData[0].size());
}

void CSVImport::FetchResults(std::vector<fvec> results)
{

}

void CSVImport::spinBoxChanged(int value)
{
    inputParser->setOutputColumn(value-1);
}

void CSVImport::on_dumpButton_clicked()
{
    ivec excludeIndices;
    vector<bool> bExcluded(gui->tableWidget->columnCount(), false);
    QModelIndexList indexes = gui->tableWidget->selectionModel()->selection().indexes();
    FOR(i, indexes.count())
    {
        QModelIndex index = indexes.at(i);
        bExcluded[index.column()] = true;
    }
    FOR(i, bExcluded.size())
    {
        if(bExcluded[i]) excludeIndices.push_back(i);
    }
    inputParser->setFirstRowAsHeader(gui->headerCheck->isChecked());
	pair<vector<fvec>,ivec> data = inputParser->getData(excludeIndices, 1000);
    emit(SetData(data.first, data.second, vector<ipair>(), false));
}

/*
void CSVImport::on_pcaButton_clicked()
{
    ivec excludeIndices;
    vector<bool> bExcluded(gui->tableWidget->columnCount(), false);
    QModelIndexList indexes = gui->tableWidget->selectionModel()->selection().indexes();
    FOR(i, indexes.count())
    {
        QModelIndex index = indexes.at(i);
        bExcluded[index.column()] = true;
    }
    FOR(i, bExcluded.size())
    {
        if(bExcluded[i]) excludeIndices.push_back(i);
    }

	pair<vector<fvec>,ivec> data = inputParser->getData(excludeIndices, inputParser->getCount());
    ProjectionPCA pca;
	if(!data.first.size()) return;
    int pcaCount = min(data.first[0].size(),data.first.size() -1);
	pca.Train(data.first, pcaCount);
    data.first = pca.samples;
	if(data.first.size() > 2000)
	{
		data.first.resize(2000);
		data.second.resize(2000);
	}
	DEL(eigLabel);
    eigLabel = pca.EigenValues();
    eigLabel->show();
    emit(SetData(data.first, data.second, vector<ipair>(), true));
}
*/
