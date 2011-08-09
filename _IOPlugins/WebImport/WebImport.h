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
// http://stackoverflow.com/users/14065/martin
// http://stackoverflow.com/questions/1120140/csv-parser-in-c
// http://www.gamedev.net/topic/603211-get-type-of-string-in-c/
// http://www.gamedev.net/user/78572-rip-off/


#ifndef WEBIMPORT_H_INCLUDED
#define WEBIMPORT_H_INCLUDED

#define UNSIGNED_INT_TYPE    '1'
#define INT_TYPE             '2'
#define FLOAT_TYPE           '3'
#define DOUBLE_TYPE          '4'
#define CHAR_TYPE            '5'
#define STRING_TYPE          '6'
#define UNKNOWN_TYPE         '7'

#include <map>
#include <iterator>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

using namespace std;

#include <interfaces.h>
#include <QTimerEvent>
#include "ui_WebImport.h"
#include "public.h"
#include "widget.h"
#include <QMutex>
#include <QMutexLocker>

class WebImport : public QObject, public InputOutputInterface
{
	Q_OBJECT
	Q_INTERFACES(InputOutputInterface)
public:
	const char* QueryClassifierSignal() {return SIGNAL(QueryClassifier(std::vector<fvec>));}
	const char* QueryRegressorSignal() {return SIGNAL(QueryRegressor(std::vector<fvec>));}
	const char* QueryDynamicalSignal() {return SIGNAL(QueryDynamical(std::vector<fvec>));}
	const char* QueryClustererSignal() {return SIGNAL(QueryClusterer(std::vector<fvec>));}
	const char* QueryMaximizerSignal() {return SIGNAL(QueryMaximizer(std::vector<fvec>));}
	const char* SetDataSignal() {return SIGNAL(SetData(std::vector<fvec>, ivec, std::vector<ipair>));}
	const char* FetchResultsSlot() {return SLOT(FetchResults(std::vector<fvec>));}
	const char* DoneSignal() {return SIGNAL(Done(QObject *));}
	QObject *object(){return this;};
	QString GetName(){return "WebImport";};

	void Start();
	void Stop();

	Ui::WebImportDialog *gui;
	QDialog *guiDialog;
	//Projector *projector;
	WebImport();
	~WebImport();
signals:
	void Done(QObject *);
	void SetData(std::vector<fvec> samples, ivec labels, std::vector<ipair> trajectories);
	void QueryClassifier(std::vector<fvec> samples);
	void QueryRegressor(std::vector<fvec> samples);
	void QueryDynamical(std::vector<fvec> samples);
	void QueryClusterer(std::vector<fvec> samples);
	void QueryMaximizer(std::vector<fvec> samples);
public slots:
	void FetchResults(std::vector<fvec> results);
	void Closing();
	void Updating();
};

template<typename T>
bool is(const std::string &s)
{
    std::istringstream stream(s);
    T instance;
    return ((stream >> instance) && (stream >> std::ws) && stream.eof());
}

char getType(string input)
{
    if (is<unsigned int>(input))    return UNSIGNED_INT_TYPE;
    if (is<int>(input))             return INT_TYPE;
    if (is<float>(input))           return FLOAT_TYPE;
    if (is<char>(input))            return CHAR_TYPE;
    if (is<string>(input))          return STRING_TYPE;
                                    return UNKNOWN_TYPE;
}

vector<size_t> getMissingValIndex(vector<vector<string> > dataset)
{
    vector<size_t> missingValIndex;
    size_t nbCols = dataset.at(0).size();
    for (size_t i = 0; i < dataset.size(); i++)
        for (size_t j = 0; j < nbCols; j++)
            if (dataset[i][j] == "?") missingValIndex.push_back(i);
    return missingValIndex;
}

class CSVRow
{
public:
    std::string const& operator[](std::size_t index) const
    {
        return m_data[index];
    }
    std::string at(size_t column) const
    {
        return m_data.at(column);
    }
    std::string first() const
    {
        return m_data.at(0);
    }
    std::string last() const
    {
        return m_data.at(m_data.size()-1);
    }
    std::vector<std::string> getParsedLine() const
    {
        return m_data;
    }
    std::size_t size() const
    {
        return m_data.size();
    }
    void readNextRow(std::istream& str)
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
private:
    std::vector<std::string>    m_data;
};

std::istream& operator>>(std::istream& str,CSVRow& data)
{
    data.readNextRow(str);
    return str;
}

class CSVIterator
{
public:
    typedef std::input_iterator_tag     iterator_category;
    typedef CSVRow                      value_type;
    typedef std::size_t                 difference_type;
    typedef CSVRow*                     pointer;
    typedef CSVRow&                     reference;

    CSVIterator(std::istream& str)  :m_str(str.good()?&str:NULL)
    {
        ++(*this);
    }
    CSVIterator()                   :m_str(NULL) {}

    // Pre Increment
    CSVIterator& operator++()
    {
        if (m_str)
        {
            (*m_str) >> m_row;
            m_str = m_str->good()?m_str:NULL;
        }
        return *this;
    }
    // Post increment
    CSVIterator operator++(int)
    {
        CSVIterator tmp(*this);
        ++(*this);
        return tmp;
    }
    CSVRow const& operator*()   const
    {
        return m_row;
    }
    CSVRow const* operator->()  const
    {
        return &m_row;
    }

//        std::string at(size_t column)           {return m_row.at(column);}
//        std::string first()                     {return m_row.at(0);}
//        std::string last()                      {return m_row.at(m_row.size()-1);}

    bool operator==(CSVIterator const& rhs)
    {
        return ((this == &rhs) || ((this->m_str == NULL) && (rhs.m_str == NULL)));
    }
    bool operator!=(CSVIterator const& rhs)
    {
        return !((*this) == rhs);
    }

private:
    std::istream*       m_str;
    CSVRow              m_row;
};

#endif // WEBIMPORT_H_INCLUDED
