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

using namespace std;

int main()
{
    // init
    ifstream file("iris.data");
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
    map<unsigned int, char> inputLabels;
    for(int i = 0;i < data.at(0).size();i++)
    {

    }
}
