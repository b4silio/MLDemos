#ifndef TABLEVIEW_H
#define TABLEVIEW_H

#include <QAbstractTableModel>

class TableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    TableModel(QObject *parent=0);
    TableModel(QList< QList<float> > array, QObject *parent=0);

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
	QVariant data(const QModelIndex &index, int role) const{return QVariant(0);};
	QVariant headerData(int section, Qt::Orientation orientation, int role) const{return QVariant(0);};
	Qt::ItemFlags flags(const QModelIndex &index) const {return Qt::NoItemFlags;};;
    bool setData(const QModelIndex &index, const QVariant &value, int role=Qt::EditRole);
    bool insertRows(int position, int rows, const QModelIndex &index=QModelIndex());
    bool removeRows(int position, int rows, const QModelIndex &index=QModelIndex());
	QList< QList<float> > getList(){return arrayOfFloats;};

private:
    QList< QList<float> > arrayOfFloats;
};


#endif // TABLEVIEW_H
