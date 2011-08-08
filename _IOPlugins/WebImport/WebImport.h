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


#ifndef WEBIMPORT_H_INCLUDED
#define WEBIMPORT_H_INCLUDED

#include <map>
#include <iterator>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

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
