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
    //connect(gui->spinE1, SIGNAL(valueChanged(int)), this, SLOT(Updating()));
    //connect(gui->spinE2, SIGN.AL(valueChanged(int)), this, SLOT(Updating()));
    connect(gui->loadFile, SIGNAL(clicked()), this, SLOT(LoadFile())); // file loader
    connect(gui->browserWebView, SIGNAL(linkClicked(QUrl)), this, SLOT(LinkHandler(QUrl)));
    connect(gui->backButton, SIGNAL(clicked()),gui->browserWebView,SLOT(back()));
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
        }
    }
}

bool WebImport::saveFile(const QString &filename, QIODevice *data)
{
//    QDir dir;
//    if (!dir.exists("datasets")) dir.mkdir("datasets");
//    dir.cd("datasets");
    //QDir::setCurrent(dir.dirName());
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
    Parse(filename);
}

void WebImport::Parse(QString filename)
{
    if(filename.isEmpty()) return;
    inputParser->parse(filename.toStdString().c_str());
    pair<vector<fvec>,ivec> data = inputParser->getData(NUMERIC_TYPES);
    qDebug() << "Dataset extracted";
    if(data.first.size() < 2) return;
    gui->tableWidget->setRowCount(data.first.size());
    gui->tableWidget->setColumnCount(data.first.at(0).size()+1);
    for(size_t r = 0; r < data.first.size(); r++)
    {
        for(size_t c = 0; c < data.first.at(r).size(); c++)
        {
            QTableWidgetItem *newItem = new  QTableWidgetItem(QString::number(data.first.at(r).at(c)));
            gui->tableWidget->setItem(r, c, newItem);
        }
        QTableWidgetItem *newItem = new  QTableWidgetItem(QString::number(data.second.at(r)));
        gui->tableWidget->setItem(r, data.first.at(r).size(), newItem);
    }
    emit(SetData(data.first, data.second, vector<ipair>()));
}

void WebImport::FetchResults(std::vector<fvec> results)
{

}
