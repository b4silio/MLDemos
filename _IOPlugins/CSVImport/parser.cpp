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
#include <QDebug>
#include <basicMath.h>
#include "parser.h"

/* CSVRow stuff */

std::istream& operator>>(std::istream& str,CSVRow& data)
{
    data.readNextRow(str);
    return str;
}

std::string const& CSVRow::operator[](std::size_t index) const
{
    return m_data[index];
}
std::string CSVRow::at(size_t column) const
{
    return m_data.at(column);
}
std::string CSVRow::getFirstCell() const
{
	if(!m_data.size()) return string();
	return m_data.at(0);
}
std::string CSVRow::getLastCell() const
{
	if(!m_data.size()) return string();
	return m_data.back();
}
std::vector<std::string> CSVRow::getParsedLine() const
{
    return m_data;
}
std::size_t CSVRow::size() const
{
    return m_data.size();
}

void CSVRow::readNextRow(std::istream& str)
{
    // read line from file
    std::string line;
    std::getline(str,line);

    // convert to stream
    std::stringstream lineStream(line);
    std::string cell;

    // update row (array) content
    m_data.clear();
    while(std::getline(lineStream,cell,','))
    {
        m_data.push_back(cell);
    }
}

unsigned int getType(string input)
{
    if (is<unsigned int>(input))    return UNSIGNED_INT_TYPE;
    if (is<int>(input))             return INT_TYPE;
    if (is<float>(input))           return FLOAT_TYPE;
    if (is<char>(input))            return CHAR_TYPE;
    if (is<string>(input))          return STRING_TYPE;
                                    return UNKNOWN_TYPE;
}

/* CSVIterator stuff */

CSVIterator::CSVIterator(std::istream& str)  :m_str(str.good()?&str:NULL)
{
    ++(*this);
}

CSVIterator::CSVIterator():m_str(NULL)
{
}

bool CSVIterator::eof()
{
    return (m_str == NULL);
}

// Pre Increment
CSVIterator& CSVIterator::operator++()
{
    if (m_str)
    {
        (*m_str) >> m_row;
        m_str = m_str->good()?m_str:NULL;
    }
    return *this;
}
// Post increment
CSVIterator CSVIterator::operator++(int)
{
    CSVIterator tmp(*this);
    ++(*this);
    return tmp;
}

CSVRow const& CSVIterator::operator*()   const
{
    return m_row;
}

CSVRow const* CSVIterator::operator->()  const
{
    return &m_row;
}

bool CSVIterator::operator!=(CSVIterator const& rhs)
{
    return !((*this) == rhs);
}

bool CSVIterator::operator==(CSVIterator const& rhs)
{
    return ((this == &rhs) || ((this->m_str == NULL) && (rhs.m_str == NULL)));
}

/* CSVParser stuff */
CSVParser::CSVParser()
{
    outputLabelColumn = 2;
}

void CSVParser::clear()
{
    outputLabelColumn = 0;
    classLabels.clear();
    data.clear();
    dataTypes.clear();
}

void CSVParser::parse(const char* fileName)
{
    // init
    file.open(fileName);
    if(!file.is_open()) return;

    // Parse CSV input file
    for(CSVIterator parser(file); !parser.eof(); ++parser)
    {
        if(!parser->size()) continue;
		vector<string> parsed = parser->getParsedLine();
		if(!parsed.size()) continue;
		// Fill dataset
                data.push_back(parsed);
    }

    cout << "Parsing done, read " << data.size() << " entries" << endl;
    cout << "Found " << data.at(0).size()-1 << " input labels" << endl;

    // look for data types
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
        dataTypes.push_back(getType(data.at(testRow).at(i)));
    }

    // Read output (class) labels
    getOutputLabelTypes(true);
    file.close();
}

void CSVParser::setOutputColumn(unsigned int column)
{
    outputLabelColumn = column;
   //getOutputLabelTypes(true); // need to update output label types
}

map<string,unsigned int> CSVParser::getOutputLabelTypes(bool reparse)
{
    if (!reparse) return classLabels;
    unsigned int id = 0;
    pair<map<string,unsigned int>::iterator,bool> ret;
    // Use by default the last column as output class
    if ((data.size() && outputLabelColumn == -1) || outputLabelColumn >= data.size()) outputLabelColumn = data.at(0).size()-1;
    for(vector<vector<string> >::iterator it = data.begin(); it<data.end(); it++)
    {
        ret = classLabels.insert( pair<string,unsigned int>(it->at(outputLabelColumn),id) );
        if (ret.second == true) id++; // new class found
    }
    return classLabels;
}

vector<size_t> CSVParser::getMissingValIndex()
{
    vector<size_t> missingValIndex;
    size_t nbCols = data.at(0).size();
    for (size_t i = 0; i < data.size(); i++)
        for (size_t j = 0; j < nbCols; j++)
            if (data[i][j] == "?") missingValIndex.push_back(i);
    return missingValIndex;
}

bool CSVParser::hasData()
{
    return data.size();
}

void CSVParser::cleanData(unsigned int acceptedTypes)
{
    vector<string>::iterator it_str;
    vector<unsigned int>::iterator it_uint = dataTypes.begin();
    for(size_t i = 0; i < dataTypes.size(); i++)
        if (!(dataTypes[i]&acceptedTypes) &&  // data type does not correspond to a requested one
           (i != outputLabelColumn))       // output labels are stored separately, ignore
        {
            cout << "Removing colum " << i << " of type " << dataTypes[i] <<  " ... ";
            for(size_t j = 0; j < data.size(); j++)
            {
                /* @note it seems that if we have --i instead of (i-1), the compiler produces bad code (SIGSEGV) */
                it_str = data.at(j).begin() + (i-1);
                data.at(j).erase(it_str); // delete the column
            }
            cout << "and matching type reference ...  " ;
            it_uint = dataTypes.begin() + (i-1);
            dataTypes.erase(it_uint); // delete the input to stay consistant
            i--; if (i < outputLabelColumn) outputLabelColumn--;
        }
}

pair<vector<fvec>,ivec> CSVParser::getData(ivec excludeIndex, int maxSamples)
{
    if(!data.size()) return pair<vector<fvec>,ivec>();
    vector<fvec> samples(data.size());
    ivec labels(data.size());
    int count = data.size();
    int dim = data[0].size();
    outputLabelColumn = min(dim-1, outputLabelColumn);
    vector< map<string,int> > labelMaps(dim);
    ivec labelCounters(dim,0);
    pair<map<string,int>::iterator,bool> ret;
    FOR(i, data.size())
    {
        // check if it's always a number
        fvec& sample = samples[i];
        sample.resize(dim-1);
        FOR(j, dim)
        {
            QString s(data[i][j].c_str());
            bool ok;
            float val = s.toFloat(&ok);
            if(j==outputLabelColumn || !ok)
            {
                if(labelMaps[j].count(data[i][j]) > 0) val = labelMaps[j][data[i][j]];
                else
                {
                    val = (float)labelCounters[j];
                    ret = labelMaps[j].insert(pair<string,int>(data[i][j], val));
                    if(ret.second) labelCounters[j]++;
                }
            }
            if(j!=outputLabelColumn)
            {
                int index = j < outputLabelColumn ? j : j-1;
                sample[index] = val;
            }
        }
    }
    bool numerical = true;
    FOR(i, data.size())
    {
        bool ok;
        float val = QString(data[i][outputLabelColumn].c_str()).toFloat(&ok);
        if(!ok)
        {
            numerical = false;
            break;
        }
    }
    FOR(i, data.size())
    {
        bool ok;
        float val = QString(data[i][outputLabelColumn].c_str()).toFloat(&ok);
        if(numerical)
        {
            labels[i] = val;
        }
        else
        {
            labels[i] = labelMaps[outputLabelColumn][data[i][outputLabelColumn]];
        }
    }

    if(maxSamples != -1 && maxSamples < samples.size())
    {
        vector<fvec> newSamples(maxSamples);
        ivec newLabels(maxSamples);
        u32 *perm = randPerm(maxSamples);
        FOR(i, maxSamples)
        {
            newSamples[i] = samples[perm[i]];
            newLabels[i] = labels[perm[i]];
        }
        samples = newSamples;
        labels = newLabels;
        delete [] perm;
    }
    if(!excludeIndex.size()) return pair<vector<fvec>,ivec>(samples,labels);
    vector<fvec> newSamples(data.size());
    int newDim = dim - excludeIndex.size();
    FOR(i, excludeIndex.size())
    {
        // if it's the output we can ignore it but we need to reincrement the number of dimensions
        if(excludeIndex[i] == outputLabelColumn)
        {
            newDim++;
            break;
        }
    }
    vector<bool> bExclude(dim, false);
    FOR(i, excludeIndex.size())
    {
        bExclude[excludeIndex[i]] = true;
    }
    FOR(i, data.size())
    {
        newSamples[i].resize(newDim);
        int nD = 0;
        FOR(d, dim)
        {
            if(bExclude[d] || d == outputLabelColumn) continue;
            if (d < outputLabelColumn) newSamples[i][nD] = samples[i][d];
            else if(d > outputLabelColumn) newSamples[i][nD] = samples[i][d-1];
            nD++;
        }
    }
    return pair<vector<fvec>,ivec>(newSamples,labels);
}
