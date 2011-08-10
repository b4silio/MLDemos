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
    connect(gui->spinE1, SIGNAL(valueChanged(int)), this, SLOT(Updating()));
    connect(gui->spinE2, SIGNAL(valueChanged(int)), this, SLOT(Updating()));
    guiDialog->show();
}

void WebImport::Stop()
{
    guiDialog->hide();
}

void WebImport::Closing()
{
    emit(Done(this));
}

void WebImport::Updating()
{
    pair<vector<fvec>,ivec> data;// = projector->GetData();
    if(data.first.size() < 2) return;
    emit(SetData(data.first, data.second, vector<ipair>()));
}

void WebImport::FetchResults(std::vector<fvec> results)
{

}

