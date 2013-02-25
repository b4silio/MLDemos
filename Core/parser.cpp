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

    // we need to parse blocks with "", which might contain our separator
    size_t found = line.find_first_of("\"");
    if ( found != std::string::npos )
    {
        bool bInside = true;
        size_t start = found;
        size_t stop = 0;
        found = line.find_first_of("\"", found+1);
        while( found != std::string::npos ) {
            if ( bInside ) {
                stop = found;
                size_t sep = line.find_first_of(separator[0], start);
                while(sep < stop && sep != std::string::npos)
                {
                    line[sep] = '_'; // we replace the offending character
                    sep = line.find_first_of(separator[0], sep+1);
                }
            } else {
                start = found;
            }
            bInside = !bInside;
            found = line.find_first_of("\"", found+1);
        }
    }

    // convert to stream
    std::stringstream lineStream(line);
    std::string cell;

    // update row (array) content
    m_data.clear();
    while(std::getline(lineStream,cell,separator[0]))
    {
        std::string test = cell;
        std::remove(test.begin(), test.end(), ' ');
        if(test.empty()) m_data.push_back("?");
        else m_data.push_back(cell);
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

CSVIterator::CSVIterator(std::istream& str, std::string separator)
    : m_str(str.good()?&str:NULL), separator(separator), m_row(separator)
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
    bFirstRowAsHeader = false;
    outputLabelColumn = 2;
}

void CSVParser::clear()
{
    outputLabelColumn = 0;
    classLabels.clear();
    data.clear();
    dataTypes.clear();
}

void CSVParser::parse(const char* fileName, int separatorType)
{
    // init
    uint8_t offset = getBOMsize(fileName);
    file.open(fileName,std::ios_base::binary);
    if(!file.is_open()) return;

    std::string separators[] = {",", ";", "\t", " "};
    int separatorCount = 4;
    int bestSeparator = 0;
    int dim=0;

    if(!separatorType)
    {
        // we test the separators to find which one is best
        for(int i=0; i<separatorCount; i++)
        {
            file.seekg(offset,ios::beg);
            // Parse CSV input file
            CSVIterator parser(file, separators[i]);
            ++parser; // we skip the first line as it might be a header line
            if(parser.eof() || !parser->size()) continue;
            vector<string> parsed = parser->getParsedLine();
            //qDebug() << "separator: " << separators[i].c_str() << ":" << parsed.size();
            if(parsed.size() > dim)
            {
                dim = parsed.size();
                bestSeparator = i;
            }
        }
    }
    else bestSeparator = separatorType-1;

    file.seekg(offset,ios::beg);

    data.clear();
    for(CSVIterator parser(file, separators[bestSeparator]);!parser.eof(); ++parser)
    {
        if(!parser->size()) continue;
        vector<string> parsed = parser->getParsedLine();
        if(!parsed.size()) continue;

        // check for remaining line carrys, *nix only
        string::size_type pos = 0;
#if !(defined WIN32 || defined _WIN32)
        while ( ( pos = parsed.back().find ("\r",pos) ) != string::npos )
            parsed.back().erase ( pos, 1 );
#endif
        // remove null character noise when coming from UTF-X
        // we assume that the data has only ASCII characters
        if (offset)
            for(size_t i = 0; i < parsed.size(); i++)
            {
                pos = 0;
                while ( ( pos = parsed.at(i).find ('\0',pos) ) != string::npos )
                    parsed.at(i).erase ( pos, 1 );

            }

        // Fill dataset
        data.push_back(parsed);
    }

    cout << "Parsing done, read " << data.size() << " entries" << endl;
    cout << "Found " << data.at(0).size() << " input labels / columns" << endl;

    // look for data types
    for(size_t i = 0; i < data.at(1).size(); i++)
    {
        // Look for a non-empty cell
        // start with 2nd row as first might be input labels
        size_t testRow = 1;
        while(data.at(testRow++).at(i) == "?" && testRow != data.size());


        if (testRow == data.size())
        {
            // if the whole column is missing data... delete it
            cout << "Warning: Found empty column, deleting..." << endl;
            for (size_t j = 0; j < data.size(); j++)
                data.at(j).erase(data.at(j).begin()+i);

        } else // save input type
            dataTypes.push_back(getType(data.at(testRow).at(i)));
    }

    cout << data.at(0).size() << " input labels / columns remaining after cleanup" << endl;

//    cout << "Contents: " << endl;
//    for (size_t j=0; j<data.size(); j++)
//    {
//        cout << "|";
//        for (size_t i=0; i<data.at(1).size(); i++)
//            cout << data.at(j).at(i) << "|";
//        cout << endl;
//    }

    // Read output (class) labels
    getOutputLabelTypes(true);
    file.close();
}

void CSVParser::setOutputColumn(int column)
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
    int count = data.size();
    if(bFirstRowAsHeader) count--;
    int headerSkip = bFirstRowAsHeader?1:0;
    if(count <= 0) return pair<vector<fvec>,ivec>();
    vector<fvec> samples(count);
    ivec labels(count);
    int dim = data[0].size();
    if(outputLabelColumn != -1) outputLabelColumn = min(dim-1, outputLabelColumn);
    classNames.clear();
    categorical.clear();
    vector< map<string,int> > labelMaps(dim);
    ivec labelCounters(dim,0);
    pair<map<string,int>::iterator,bool> ret;
    FOR(i, data.size())
    {
        if(!i && bFirstRowAsHeader) continue;
        // check if it's always a number
        fvec& sample = samples[i-headerSkip];
        sample.resize((outputLabelColumn==-1) ? dim : dim-1);
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
                if(outputLabelColumn==-1) sample[j] = val;
                else
                {
                    int index = j < outputLabelColumn ? j : j-1;
                    sample[index] = val;
                }
            }
        }
    }
    FOR(j, dim)
    {
        if(j == outputLabelColumn) continue;
        bool bNumerical = true;
        FORIT(labelMaps[j], string, int)
        {
            bool ok;
            QString(it->first.c_str()).toFloat(&ok);
            if(!ok && it->first != "?")
            {
                bNumerical = false;
                break;
            }
        }
        if(bNumerical) continue;
        int itemCount = 0;
        FORIT(labelMaps[j], string, int)
        {
            itemCount = max(itemCount, it->second);
        }
        itemCount++;
        vector<string> cat(itemCount);
        FORIT(labelMaps[j], string, int)
        {
            int index = it->second;
            cat[index] = it->first;
        }
        categorical[ j>outputLabelColumn ? j-1 : j ] = cat;
    }

    if(labelMaps[outputLabelColumn].size() && outputLabelColumn != -1)
    {
        bool bNumerical = true;
        FORIT(labelMaps[outputLabelColumn], string, int)
        {
            bool ok;
            QString(it->first.c_str()).toFloat(&ok);
            if(!ok && it->first != "?")
            {
                bNumerical = false;
                break;
            }
        }
        if(!bNumerical)
        {
            FORIT(labelMaps[outputLabelColumn], string, int)
            {
                classNames[it->second] = QString(it->first.c_str());
                //else classNames[it->second] = QString("Class %1").arg(QString(it->first.c_str()).toFloat());
            }
        }
        else classNames.clear();
    }
    if(outputLabelColumn == -1)
    {
        FOR(i, data.size())
        {
            if(!i && bFirstRowAsHeader) continue;
            labels[i-headerSkip] = 0;
        }
    }
    else
    {
        /*
        qDebug() << "label indices";
        for(map<string,int>::iterator it = labelMaps[outputLabelColumn].begin(); it != labelMaps[outputLabelColumn].end(); it++)
        {
            qDebug() << (it->first).c_str() << " " << it->second;
        }
        */
        bool numerical = true;
        FOR(i, data.size())
        {
            if(!i && bFirstRowAsHeader) continue;
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
            if(!i && bFirstRowAsHeader) continue;
            bool ok;
            float val = QString(data[i][outputLabelColumn].c_str()).toFloat(&ok);
            if(numerical)
            {
                labels[i-headerSkip] = val;
            }
            else
            {
                int label = labelMaps[outputLabelColumn][data[i][outputLabelColumn]];
                labels[i-headerSkip] = label;
            }
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
        count = samples.size();
        delete [] perm;
    }
    if(!excludeIndex.size()) return pair<vector<fvec>,ivec>(samples,labels);
    vector<fvec> newSamples(count);
    int newDim = dim - excludeIndex.size();
    if(outputLabelColumn != -1) newDim--;
    //qDebug() << "Indices to be excluded: " << excludeIndex.size() << "(newDim: " << newDim << ")";
    FOR(i, excludeIndex.size())
    {
        //qDebug() << i << ":" << excludeIndex[i];
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
    FOR(i, samples.size())
    {
        newSamples[i].resize(newDim);
        int nD = 0;
        FOR(d, dim)
        {
            if(bExclude[d] || d == outputLabelColumn) continue;
            if(outputLabelColumn == -1) newSamples[i][nD] = samples[i][d];
            else if (d < outputLabelColumn) newSamples[i][nD] = samples[i][d];
            else if(d > outputLabelColumn) newSamples[i][nD] = samples[i][d-1];
            nD++;
        }
    }
    qDebug() << "Imported samples: " << newSamples.size() << " labels: " << labels.size();
    return pair<vector<fvec>,ivec>(newSamples,labels);
}


uint8_t CSVParser::getBOMsize(const char* fileName)
{
    FILE *f = fopen(fileName,"rb");
    unsigned char bom[16];
    fread(bom,1,16,f);

    if        (bom[0] == 0x00  && bom[1]  == 0x00 &&
               bom[2] == 0xFE  && bom[3]  == 0xFF) {
        cout << "Detected UTF-32 (BE) encoding" << endl;
        return 4;
    } else if (bom[0]  == 0xFF && bom[1]  == 0xFE &&
               bom[2]  == 0x00 && bom[3]  == 0x00) {
        cout << "Detected UTF-32 (LE) encoding" << endl;
        return 4;
    } else if (bom[0]  == 0xFE && bom[1]  == 0xFF) {
        cout << "Detected UTF-16 (BE) encoding" << endl;
        return 2;
    } else if (bom[0]  == 0xFF && bom[1]  == 0xFE) {
        cout << "Detected UTF-16 (LE) encoding" << endl;
        return 2;
    } else if (bom[0]  == 0xEF && bom[1]  == 0xBB &&
               bom[2]  == 0xBF)                  {
        cout << "Detected UTF-8 encoding" << endl;
        return 3;
    } else if (bom[0]  == 0x2B && bom[1]  == 0x2F &&
               bom[2]  == 0x76 && bom[3]  == 0x38 &&

               bom[4]  == 0x2B && bom[5]  == 0x2F &&
               bom[6]  == 0x76 && bom[7]  == 0x39 &&

               bom[8]  == 0x2B && bom[9]  == 0x2F &&
               bom[10] == 0x76 && bom[11] == 0x2B &&

               bom[12] == 0x2B && bom[13] == 0x2F &&
               bom[14] == 0x76 && bom[15] == 0x2F) {
        cout << "Detected UTF-7 encoding" << endl;
        return 16;
    } else if (bom[0]  == 0xF7 && bom[1]  == 0x64 &&
               bom[2]  == 0x4C)                  {
        cout << "Detected UTF-1 encoding" << endl;
        return 3;
    } else if (bom[0]  == 0xDD && bom[1]  == 0x73 &&
               bom[2]  == 0x66 && bom[3]  == 0x73) {
        cout << "Detected UTF-EBCDIC encoding" << endl;
        return 4;
    } else if (bom[0]  == 0x0E && bom[1]  == 0xFE &&
               bom[2]  == 0xFF)                  {
        cout << "Detected SCSU encoding" << endl;
        return 3;
    } else if (bom[0]  == 0xFB && bom[1]  == 0xEE &&
               bom[2]  == 0x28)                  {
        cout << "Detected BOCU-1 encoding" << endl;
        return 3;
    } else if (bom[0]  == 0x84 && bom[1]  == 0x31 &&
               bom[2]  == 0x95 && bom[3]  == 0x33) {
        cout << "Detected GB-18030 encoding" << endl;
        return 4;
    } else {
        cout << "No BOM detected, possibly pure ASCII or UTF-8" << endl;
        return 0;
    }
}
