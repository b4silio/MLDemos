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

#include "WebImport.h"
#include "pcaprojection.h"

Q_EXPORT_PLUGIN2(IO_WebImport, WebImport)

WebImport::WebImport()
: guiDialog(0)
{
}

WebImport::~WebImport()
{
    if(gui && guiDialog) guiDialog->hide();
    DEL(inputParser);
}

void WebImport::Start()
{
    gui = new Ui::WebImportDialog();
    gui->setupUi(guiDialog = new QDialog());
    inputParser = new CSVParser();
    connect(gui->closeButton, SIGNAL(clicked()), this, SLOT(Closing()));
	connect(guiDialog, SIGNAL(finished(int)), this, SLOT(Closing()));
    connect(gui->spinBox, SIGNAL(valueChanged(int)), this, SLOT(spinBoxChanged(int)));
    //connect(gui->spinE1, SIGNAL(valueChanged(int)), this, SLOT(Updating()));
    //connect(gui->spinE2, SIGN.AL(valueChanged(int)), this, SLOT(Updating()));
    connect(gui->loadFile, SIGNAL(clicked()), this, SLOT(LoadFile())); // file loader
    connect(gui->browserWebView, SIGNAL(linkClicked(QUrl)), this, SLOT(LinkHandler(QUrl)));
    connect(gui->backButton, SIGNAL(clicked()),gui->browserWebView,SLOT(back()));
    connect(gui->dumpButton, SIGNAL(clicked()),this,SLOT(on_dumpButton_clicked()));
    connect(gui->pcaButton, SIGNAL(clicked()),this,SLOT(on_pcaButton_clicked()));
    connect(&manager, SIGNAL(finished(QNetworkReply*)),SLOT(downloadHandler(QNetworkReply*)));
    guiDialog->show();
	gui->browserWebView->show();
    gui->browserWebView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
}

void WebImport::Stop()
{
    guiDialog->hide();
}

void WebImport::Closing()
{
    emit(Done(this));
}

void WebImport::LinkHandler(const QUrl & url)
{
	if(url.toString().endsWith(".data"))
    {
        qDebug() << "Importing: " << url.toString();
        //gui->browserWebView->triggerPageAction(QWebPage::DownloadLinkToDisk);

        QNetworkRequest request(url);
        reply = manager.get(request);
	}
	else
    {
		qDebug() << "Loading page: " << url.toString();
        gui->browserWebView->load(url);
	}
}

void WebImport::downloadHandler(QNetworkReply *reply)
{
    qDebug() << "Got network manager reply...";
    QUrl url = reply->url();
    if (reply->error()) {
        qDebug() << "Download of " << url.toEncoded().constData() << " failed: " << qPrintable(reply->errorString());
    } else {
        QString path = url.path();
        QString filename = QFileInfo(path).fileName();
        if (saveFile(filename, reply))
        {
           qDebug() << "Download of " << url.toEncoded().constData() << " succeeded (saved to " <<  qPrintable(filename) << ")";
           Parse(filename);
           gui->tabWidget->setCurrentIndex(0);
        }
    }
}

bool WebImport::saveFile(const QString &filename, QIODevice *data)
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

void WebImport::LoadFile()
{
	QString filename = QFileDialog::getOpenFileName(NULL, tr("Load Data"), QDir::currentPath(), tr("dataset files (*.data *.csv);;All files (*.*)"));
    if(filename.isEmpty()) return;
    Parse(filename);
    gui->tabWidget->setCurrentIndex(0);
}

void WebImport::Parse(QString filename)
{
    if(filename.isEmpty()) return;
    inputParser->clear();
    inputParser->parse(filename.toStdString().c_str());
    vector<vector<string> > rawData = inputParser->getRawData();
    qDebug() << "Dataset extracted";
//    if(data.first.size() < 2) return;
    if(rawData.size() < 2) return;
    gui->tableWidget->clear();
    gui->tableWidget->setRowCount(rawData.size());
    gui->tableWidget->setColumnCount(rawData[0].size());
    for(size_t r = 0; r < rawData.size(); r++)
    {
        for(size_t c = 0; c < rawData[r].size(); c++)
        {
            QTableWidgetItem *newItem = new  QTableWidgetItem(QString(rawData[r][c].c_str()));
            gui->tableWidget->setItem(r, c, newItem);
        }
    }
    gui->spinBox->setRange(1,rawData[0].size());
    gui->pcaCountSpin->setRange(2, max(rawData[0].size()-1, rawData.size()-2));
}

void WebImport::FetchResults(std::vector<fvec> results)
{

}

void WebImport::spinBoxChanged(int value)
{
    inputParser->setOutputColumn(value-1);
}

void WebImport::on_dumpButton_clicked()
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

    pair<vector<fvec>,ivec> data = inputParser->getData(excludeIndices);
    emit(SetData(data.first, data.second, vector<ipair>()));
}

void WebImport::on_pcaButton_clicked()
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

    int pcaCount = gui->pcaCountSpin->value();
    pair<vector<fvec>,ivec> data = inputParser->getData(excludeIndices);
    PCAProjection pca;
    pca.Train(data.first, pcaCount);
    data.first = pca.samples;
    emit(SetData(data.first, data.second, vector<ipair>()));
}
