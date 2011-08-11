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

#include "tableview.h"

TableModel::TableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

TableModel::TableModel(QList< QList<float> > array, QObject *parent)
    : QAbstractTableModel(parent)
{
    arrayOfFloats = array;
}

int TableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return arrayOfFloats.size();
}

int TableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if (arrayOfFloats.size())
        return arrayOfFloats.at(0).size();
    else
        return 0;

}

bool TableModel::setData(const QModelIndex &index, const QVariant &value, int role){return false;};
bool TableModel::insertRows(int position, int rows, const QModelIndex &index){return false;};
bool TableModel::removeRows(int position, int rows, const QModelIndex &index){return false;};
