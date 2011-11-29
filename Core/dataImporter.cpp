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

#include "dataImporter.h"
#include "ui_dataImport.h"

DataImporter::DataImporter()
    : guiDialog(0), gui(0), inputParser(0)
{
}

DataImporter::~DataImporter()
{
    if(gui && guiDialog) guiDialog->hide();
    DEL(inputParser);
}

void DataImporter::Start()
{
    if(!gui)
    {
        gui = new Ui::DataImporterDialog();
        gui->setupUi(guiDialog = new QDialog());
        guiDialog->setWindowTitle("CVS Import");
        connect(gui->closeButton, SIGNAL(clicked()), this, SLOT(Closing()));
        connect(guiDialog, SIGNAL(finished(int)), this, SLOT(Closing()));
        connect(gui->classColumnSpin, SIGNAL(valueChanged(int)), this, SLOT(classColumnChanged(int)));
        connect(gui->headerCheck, SIGNAL(clicked()), this, SLOT(headerChanged()));
        connect(gui->loadFile, SIGNAL(clicked()), this, SLOT(LoadFile())); // file loader
        connect(gui->dumpButton, SIGNAL(clicked()),this,SLOT(SendData()));
        connect(gui->classIgnoreCheck, SIGNAL(clicked()), this, SLOT(classIgnoreChanged()));
        guiDialog->show();
    }
    else guiDialog->show();
    if(!inputParser) inputParser = new CSVParser();
}

void DataImporter::Stop()
{
    guiDialog->hide();
}

void DataImporter::Closing()
{
    guiDialog->hide();
    emit(Done(this));
}

bool DataImporter::saveFile(const QString &filename, QIODevice *data)
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

void DataImporter::LoadFile()
{
	QString filename = QFileDialog::getOpenFileName(NULL, tr("Load Data"), QDir::currentPath(), tr("dataset files (*.data *.csv);;All files (*.*)"));
    if(filename.isEmpty()) return;
    Parse(filename);
}

void DataImporter::Parse(QString filename)
{
    if(filename.isEmpty()) return;
    headers.clear();
    inputParser->clear();
    inputParser->parse(filename.toStdString().c_str());
    vector<vector<string> > rawData = inputParser->getRawData();
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
            headers << rawData[0][i].c_str();
            headerLabels <<  QString("%1:").arg(i+1) + rawData[0][i].c_str();
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
    gui->classColumnSpin->setRange(1,rawData[0].size());
}

void DataImporter::FetchResults(std::vector<fvec> results)
{

}

void DataImporter::classIgnoreChanged()
{
    classColumnChanged(0);
}

void DataImporter::headerChanged()
{
    headers.clear();
    vector<vector<string> > rawData = inputParser->getRawData();
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
            headers << rawData[0][i].c_str();
            headerLabels <<  QString("%1:").arg(i+1) + rawData[0][i].c_str();
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
    gui->classColumnSpin->setRange(1,rawData[0].size());
}

void DataImporter::classColumnChanged(int value)
{
    if(gui->classIgnoreCheck->isChecked())
    {
        inputParser->setOutputColumn(-1);
    }
    else  inputParser->setOutputColumn(value-1);
}

void DataImporter::SendData()
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
    emit(SetDimensionNames(headers));
}
