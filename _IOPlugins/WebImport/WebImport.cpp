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

WebImport::WebImport()
: guiDialog(0)
{
}

WebImport::~WebImport()
{
	if(gui && guiDialog) guiDialog->hide();
	//DEL(projector);
}

void WebImport::Start()
{
	//if(!projector)
	//{
		gui = new Ui::WebImportDialog();
		gui->setupUi(guiDialog = new QDialog());
		//projector = new Projector(gui);
		connect(gui->closeButton, SIGNAL(clicked()), this, SLOT(Closing()));
		//connect(projector, SIGNAL(Update()), this, SLOT(Updating()));
		connect(gui->spinE1, SIGNAL(valueChanged(int)), this, SLOT(Updating()));
		connect(gui->spinE2, SIGNAL(valueChanged(int)), this, SLOT(Updating()));
	//}
	guiDialog->show();
}

void WebImport::Stop()
{
	//if(projector)
	//{
	//	guiDialog->hide();
	//}
}

void WebImport::Closing()
{
	emit(Done(this));
}

void WebImport::Updating()
{
	//if(!projector) return;
	pair<vector<fvec>,ivec> data;// = projector->GetData();
	if(data.first.size() < 2) return;
	emit(SetData(data.first, data.second, vector<ipair>()));
}

void WebImport::FetchResults(std::vector<fvec> results)
{

}

int parseInput(char* fileName)
{
    // init
    ifstream file(fileName);
    map<string,unsigned int> classLabels;
    vector<vector<string> > data;
    pair<map<string,unsigned int>::iterator,bool> ret;
    unsigned int id = 0;

    // Parse CSV input file
    for(CSVIterator parser(file);parser != CSVIterator();++parser)
    {
        // Read output (class) labels
        ret = classLabels.insert( pair<string,unsigned int>(parser->last(),id) );
        if (ret.second == true) id++; // new class found

        // Fill dataset
        data.push_back(parser->getParsedLine());
    }
    cout << "Parsing done, read " << data.size() << " entries" << endl;
    cout << "Found " << id << " class labels" << endl;

    // look for data types
    map<unsigned int, char> inputTypes;
    for(size_t i = 0; i < data.at(1).size(); i++)
    {
        // Look for a non-empty cell
        // start with 2nd row as first might be input labels
        size_t testRow = 1;
        while(data.at(testRow).at(i) == "?") testRow++;

        // The whole column is missing data...
        if (testRow == data.size())
        {
            cout << "WebImport: Warning: Found empty column" << endl;
            // TODO delete it
        }

        // save input types
        inputTypes.insert( pair<unsigned int, char>(i,getType(data.at(testRow).at(i))));
    }
}

Q_EXPORT_PLUGIN2(IO_WebImport, WebImport)
