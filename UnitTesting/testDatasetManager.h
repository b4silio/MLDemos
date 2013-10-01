#ifndef TESTDATASETMANAGER_H
#define TESTDATASETMANAGER_H

#include <QtTest/QTest>

class TestDatasetManager : public QObject
{
    Q_OBJECT
private slots:
    void initDatasetManager();
    void addData();
    void addData_data();
    void addBulkData();
    void addBulkData_data();
    void addDataset();
    void addDataset_data();
};

#endif // TESTDATASETMANAGER_H
