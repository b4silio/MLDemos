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
        guiDialog->setWindowTitle("CVS Import");
        connect(gui->closeButton, SIGNAL(clicked()), this, SLOT(Closing()));
        connect(guiDialog, SIGNAL(finished(int)), this, SLOT(Closing()));
        connect(gui->classColumnSpin, SIGNAL(valueChanged(int)), this, SLOT(classColumnChanged(int)));
        connect(gui->headerCheck, SIGNAL(clicked()), this, SLOT(headerChanged()));
        connect(gui->loadFile, SIGNAL(clicked()), this, SLOT(LoadFile())); // file loader
        connect(gui->dumpButton, SIGNAL(clicked()),this,SLOT(on_dumpButton_clicked()));
        connect(gui->classIgnoreCheck, SIGNAL(clicked()), this, SLOT(classIgnoreChanged()));
        connect(gui->importLimitSpin, SIGNAL(valueChanged(int)), this, SLOT(on_importLimitSpin_valueChanged(int)));
        connect(gui->importLimitCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(on_importLimitCombo_currentIndexChanged(int)));
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
    gui->importLimitSpin->setMaximum(inputParser->getCount());
    gui->importLimitSpin->setValue(inputParser->getCount());
    gui->importLimitCombo->setEnabled(true);
    gui->importLimitCombo->addItem(QString("-->"));
    gui->importLimitCombo->addItem(QString("10%"),QVariant(0.10));
    gui->importLimitCombo->addItem(QString("25%"),QVariant(0.25));
    gui->importLimitCombo->addItem(QString("50%"),QVariant(0.50));
    gui->importLimitCombo->addItem(QString("75%"),QVariant(0.75));
    gui->importLimitCombo->addItem(QString("100%"),QVariant(1.00));
    gui->importLimitCombo->setCurrentIndex(5);
}

void CSVImport::Parse(QString filename)
{
    if(filename.isEmpty()) return;
    inputParser->clear();
    inputParser->parse(filename.toStdString().c_str());
    vector<vector<string> > rawData = inputParser->getRawData();
    qDebug() << "Dataset extracted";
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

void CSVImport::FetchResults(std::vector<fvec> results)
{

}

void CSVImport::classIgnoreChanged()
{
    classColumnChanged(0);
}

void CSVImport::headerChanged()
{
    vector<vector<string> > rawData = inputParser->getRawData();
    qDebug() << "Dataset extracted";
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
            headerLabels <<  QString("%1:").arg(i) + rawData[0][i].c_str();
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

void CSVImport::classColumnChanged(int value)
{
    if(gui->classIgnoreCheck->isChecked())
    {
        inputParser->setOutputColumn(-1);
    }
    else  inputParser->setOutputColumn(value-1);
}

void CSVImport::on_dumpButton_clicked()
{
    ivec excludeIndices;
    int maxSamples = gui->importLimitSpin->value();
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
    pair<vector<fvec>,ivec> data = inputParser->getData(excludeIndices, maxSamples);
    emit(SetData(data.first, data.second, vector<ipair>(), false));
}

void CSVImport::on_importLimitSpin_valueChanged(int arg1)
{
    int spinnerValue = gui->importLimitSpin->value();
    int nbSamples = inputParser->getCount();
    if (nbSamples <= 0) return;
    int percentage = floor(100*spinnerValue/nbSamples);
    // TODO: send that info to status in the plugin
    if(spinnerValue >= 5000)
    {
        QMessageBox limitWarning;
        limitWarning.setText("Running some algorithms on large datasets may take quite some time...");
        limitWarning.exec();
    }
}

void CSVImport::on_importLimitCombo_currentIndexChanged(int index)
{
    if(index == 0)
        gui->importLimitSpin->setEnabled(true);
    else
    {
        gui->importLimitSpin->setEnabled(false);
        int nbSamples = inputParser->getCount();
        float percentage = gui->importLimitCombo->itemData(index).toFloat();
        gui->importLimitSpin->setValue(floor(nbSamples*percentage));
    }

}
