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
    : guiDialog(0), gui(0), inputParser(0), filename("")
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
        connect(gui->importLimitSpin, SIGNAL(valueChanged(int)), this, SLOT(on_importLimitSpin_valueChanged(int)));
        connect(gui->importLimitCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(on_importLimitCombo_currentIndexChanged(int)));
        connect(gui->separatorCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(separatorChanged()));
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
    filename = QFileDialog::getOpenFileName(NULL, tr("Load Data"), QDir::currentPath(), tr("dataset files (*.data *.csv);;All files (*.*)"));
    if(filename.isEmpty()) return;
    Parse(filename);
}

void DataImporter::separatorChanged()
{
    if(filename.isEmpty()) return;
    Parse(filename);
    headerChanged();
}

void DataImporter::Parse(QString filename)
{
    if(filename.isEmpty()) return;
    this->filename = filename;
    headers.clear();
    inputParser->clear();
    int separatorType = gui->separatorCombo->currentIndex();
    inputParser->parse(filename.toStdString().c_str(), separatorType);
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
    gui->classColumnSpin->setValue(rawData[0].size());
    classColumnChanged(rawData[0].size());

    // Populate the import limit combobox only once
    if(gui->importLimitCombo->count()==0)
    {
        gui->importLimitCombo->addItem(QString("-->"));
        gui->importLimitCombo->addItem(QString("10%"),QVariant(0.10));
        gui->importLimitCombo->addItem(QString("25%"),QVariant(0.25));
        gui->importLimitCombo->addItem(QString("50%"),QVariant(0.50));
        gui->importLimitCombo->addItem(QString("75%"),QVariant(0.75));
        gui->importLimitCombo->addItem(QString("100%"),QVariant(1.00));
    }

    // By default stay below IMPORT_WARNING_THRESHOLD
    int nbSamples = inputParser->getCount();
    gui->importLimitSpin->setMaximum(nbSamples);
    if (nbSamples > IMPORT_WARNING_THRESHOLD)
    {
        gui->importLimitSpin->setValue(IMPORT_WARNING_THRESHOLD);
        gui->importLimitCombo->setCurrentIndex(0);
    } else {
        gui->importLimitSpin->setValue(nbSamples);
        gui->importLimitCombo->setCurrentIndex(5);
    }

    gui->importLimitCombo->setEnabled(true);
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
    int nbSamples = gui->importLimitSpin->value();
    // @note we have a bootle neck in getData... limit default nbSamples to ~2000
    pair<vector<fvec>,ivec> data = inputParser->getData(excludeIndices, nbSamples);
    classNames = inputParser->getClassNames();
    emit(SetData(data.first, data.second, vector<ipair>(), false));
    emit(SetDimensionNames(headers));
    emit(SetClassNames(classNames));

    // Data in dataset Manager has been updated, update visual reminder
    gui->dumpButton->setStyleSheet("color: rgb(0, 0, 0)");
}

void DataImporter::on_importLimitCombo_currentIndexChanged(int index)
{
    if(index == 0) // manual limit selection
    {
        gui->importLimitSpin->setEnabled(true);
        gui->importLimitSpin->setFocus();
    }
    else // use predefined limits
    {
        gui->importLimitSpin->setEnabled(false);
        int nbSamples = inputParser->getCount();
        float percentage = gui->importLimitCombo->itemData(index).toFloat();
        int importedSamplesCount = floor(nbSamples*percentage);
        gui->importLimitSpin->setValue(importedSamplesCount);
        if(importedSamplesCount > IMPORT_WARNING_THRESHOLD)
        {
            QMessageBox limitWarning;
            limitWarning.setText("Running some algorithms on large datasets may take quite some time...");
            limitWarning.exec();
        }

        // show visually to the user that the dataManager has not been updated yet...
        gui->dumpButton->setStyleSheet("color: rgb(255, 0, 0)");
        gui->dumpButton->setFocus();
    }
}

void DataImporter::on_importLimitSpin_valueChanged(int arg1)
{
    // show visually to the user that the dataManager has not been updated yet...
    gui->dumpButton->setStyleSheet("color: rgb(255, 0, 0)");
}

