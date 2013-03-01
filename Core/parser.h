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

#ifndef PARSER_H
#define PARSER_H

#define UNSIGNED_INT_TYPE   1
#define INT_TYPE            2
#define FLOAT_TYPE          4
#define DOUBLE_TYPE         8
#define CHAR_TYPE           16
#define STRING_TYPE         32
#define UNKNOWN_TYPE        64
#define ALL_TYPES           127
#define NUMERIC_TYPES       (UNSIGNED_INT_TYPE | INT_TYPE | FLOAT_TYPE | DOUBLE_TYPE)

#define MISSING_VALUE        "?"

#include <map>
#include <iterator>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <types.h>
#include <QString>
#include <stdint.h>

using namespace std;

template<typename T>
bool is(const std::string &s)
{
    std::istringstream stream(s);
    T instance;
    return ((stream >> instance) && (stream >> std::ws) && stream.eof());
}

class CSVRow
{
public:
    CSVRow(std::string separator = ","):separator(separator){}
    std::string const& operator[](std::size_t index) const;
    std::string at(size_t column) const;
    std::string getFirstCell() const;
    std::string getLastCell() const;
    std::vector<std::string> getParsedLine() const;
    std::size_t size() const;

    void readNextRow(std::istream& str);

private:
    std::vector<std::string>    m_data;
    string separator;
};

class CSVIterator
{
public:
    typedef std::input_iterator_tag     iterator_category;
    typedef CSVRow                      value_type;
    typedef std::size_t                 difference_type;
    typedef CSVRow*                     pointer;
    typedef CSVRow&                     reference;

    CSVIterator(std::istream& str, std::string separator=",");
    CSVIterator();

    bool eof();

    // Pre Increment
    CSVIterator& operator++();

    // Post increment
    CSVIterator operator++(int);

    CSVRow const& operator*()   const;
    CSVRow const* operator->()  const;
    bool operator!=(CSVIterator const& rhs);
    bool operator==(CSVIterator const& rhs);

private:
    std::string separator;
    std::istream*       m_str;
    CSVRow              m_row;
};

class CSVParser
{
public:
    CSVParser();
    void clear();
    void parse(const char* fileName, int separatorType=0);
    vector<size_t> getMissingValIndex();
    void cleanData(unsigned int acceptedTypes);
    pair<vector<fvec>,ivec> getData(ivec excludeIndex = ivec(), int maxSamples=-1);
    map<string,unsigned int> getOutputLabelTypes(bool reparse);
    void setOutputColumn(int column);
    void setFirstRowAsHeader(bool value){bFirstRowAsHeader = value;}
    bool hasData();
    vector<unsigned int> getDataType(){return dataTypes;}
    int getCount(){return data.size();}
    vector< vector<string> > getRawData(){return data;}
    static pair<vector<fvec>, ivec> numericFromRawData(vector< vector<string> > rawData);
    map<int,QString> getClassNames(){return classNames;}
    map<int, vector<string> > getCategorical(){return categorical;}

private:
    bool bFirstRowAsHeader;
    int outputLabelColumn;
    ifstream file;
    map<string,unsigned int> classLabels;
    map<int, QString> classNames;
    vector<vector<string> > data;
    vector<unsigned int> dataTypes;
    map<int, vector<string> > categorical;
    uint8_t getBOMsize(const char* fileName);
};

#endif // PARSER_H
