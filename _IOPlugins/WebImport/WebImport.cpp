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
    //connect(gui->spinE2, SIGNAL(valueChanged(int)), this, SLOT(Updating()));
	connect(gui->loadFile, SIGNAL(clicked()), this, SLOT(LoadFile()));
	connect(gui->browserWebView, SIGNAL(linkClicked(QUrl)), this, SLOT(Download(QUrl)));
    guiDialog->show();
	gui->browserWebView->show();
	qDebug() << "WebImport initialized";
}

void WebImport::Stop()
{
    guiDialog->hide();
}

void WebImport::Closing()
{
    emit(Done(this));
}

void WebImport::Download(QUrl url)
{
	if(url.toString().endsWith(".data"))
	{
		qDebug() << "Importing!! " << url.toString();
	}
	else
	{
		qDebug() << "Loading page: " << url.toString();
		gui->browserWebView->load(url);
	}
}

void WebImport::LoadFile()
{
	QString filename = QFileDialog::getOpenFileName(NULL, tr("Load Data"), QDir::currentPath(), tr("dataset files (*.data *.csv);;All files (*.*)"));
	Update(filename);
}

void WebImport::Update(QString filename)
{
    if(filename.isEmpty()) return;
    inputParser->parse(filename.toStdString().c_str());
    pair<vector<fvec>,ivec> data = inputParser->getData(NUMERIC_TYPES);
    std::cout << "Dataset extracted" << std::endl;
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
