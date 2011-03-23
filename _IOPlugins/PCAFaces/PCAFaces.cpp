/*********************************************************************
MLDemos: A User-Friendly visualization toolkit for machine learning
Copyright (C) 2010  Basilio Noris
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
#include "PCAFaces.h"
#include "cameraGrabber.h"
#include "sampleManager.h"
#include "eigenFaces.h"
#include <QDebug>
using namespace std;

PCAFaces::PCAFaces()
: projector(0), guiDialog(0)
{
}

PCAFaces::~PCAFaces()
{
	guiDialog->hide();
	DEL(projector);
}

void PCAFaces::Start()
{
	if(!projector)
	{
		gui = new Ui::PCAFacesDialog();
		gui->setupUi(guiDialog = new QDialog());
		projector = new Projector(gui);
		connect(gui->closeButton, SIGNAL(clicked()), this, SLOT(Closing()));
		connect(projector, SIGNAL(Update()), this, SLOT(Updating()));
		connect(gui->spinE1, SIGNAL(valueChanged(int)), this, SLOT(Updating()));
		connect(gui->spinE2, SIGNAL(valueChanged(int)), this, SLOT(Updating()));
	}
	guiDialog->show();
}

void PCAFaces::Stop()
{
	guiDialog->hide();
}

void PCAFaces::Closing()
{
	emit(Done(this));
}

void PCAFaces::Updating()
{
	if(!projector) return;
	pair<vector<fvec>,ivec> data = projector->GetData();
	if(data.first.size() < 2) return;
	emit(SetData(data.first, data.second, vector<ipair>()));
}

void PCAFaces::FetchResults(std::vector<fvec> results)
{

}

Q_EXPORT_PLUGIN2(IO_PCAFaces, PCAFaces)
