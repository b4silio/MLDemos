#include "testDatasetManager.h"
#include <public.h>
#include <datasetManager.h>
#include <mymaths.h>

using namespace std;

void TestDatasetManager::initDatasetManager()
{
    DatasetManager manager;
    QCOMPARE(manager.GetCount(), 0);
}

void TestDatasetManager::addData_data()
{
    QTest::addColumn<int>("count");
    QTest::addColumn<int>("dim");

    QTest::newRow("1x 1D") << 1 << 1;
    QTest::newRow("10000x 1D") << 10000 << 1;
    QTest::newRow("1x 10D") << 1 << 10;
    QTest::newRow("1x 10000D") << 1 << 10000;
    QTest::newRow("10x 10D") << 10 << 10;
    QTest::newRow("1000x 1000D") << 1000 << 1000;
//    QTest::newRow("100000x 2D") << 100000 << 2;
}

void TestDatasetManager::addData()
{
    QFETCH(int, count);
    QFETCH(int, dim);
    DatasetManager manager;
    fvec sample(dim);
    FOR(i, count) {
        FOR(d, dim) sample[d] = drand48()*2 - 1.f;
        manager.AddSample(sample);
    }
    QCOMPARE(manager.GetCount(), count);
    QCOMPARE(manager.GetDimCount(), dim);
}

void TestDatasetManager::addBulkData_data()
{
    QTest::addColumn<int>("count");
    QTest::addColumn<int>("dim");

    QTest::newRow("1x 1D") << 1 << 1;
    QTest::newRow("10000x 1D") << 10000 << 1;
    QTest::newRow("1x 10D") << 1 << 10;
    QTest::newRow("1x 10000D") << 1 << 10000;
    QTest::newRow("10x 10D") << 10 << 10;
    QTest::newRow("1000x 1000D") << 1000 << 1000;
    QTest::newRow("100000x 1000D") << 100000 << 1000;
}

void TestDatasetManager::addBulkData()
{
    QFETCH(int, count);
    QFETCH(int, dim);
    DatasetManager manager;
    fvec sample(dim);
    vector<fvec> samples;
    FOR(i, count) {
        FOR(d, dim) sample[d] = drand48()*2 - 1.f;
        samples.push_back(sample);
    }
    manager.AddSamples(samples);
    QCOMPARE(manager.GetCount(), count);
    QCOMPARE(manager.GetDimCount(), dim);
}

void TestDatasetManager::addDataset_data()
{
    QTest::addColumn<int>("count");
    QTest::addColumn<int>("dim");

    QTest::newRow("1x 1D") << 1 << 1;
    QTest::newRow("10000x 1D") << 10000 << 1;
    QTest::newRow("1x 10D") << 1 << 10;
    QTest::newRow("1x 10000D") << 1 << 10000;
    QTest::newRow("10x 10D") << 10 << 10;
    QTest::newRow("1000x 1000D") << 1000 << 1000;
    QTest::newRow("100000x 1000D") << 100000 << 1000;
}

void TestDatasetManager::addDataset()
{
    QFETCH(int, count);
    QFETCH(int, dim);
    DatasetManager manager1, manager2;
    fvec sample(dim);
    vector<fvec> samples;
    FOR(i, count) {
        FOR(d, dim) sample[d] = drand48()*2 - 1.f;
        samples.push_back(sample);
    }
    manager1.AddSamples(samples);
    manager2.AddSamples(manager1);
    manager2.AddSamples(manager1);

    QCOMPARE(manager2.GetCount(), 2*count);
    QCOMPARE(manager2.GetDimCount(), dim);
}

QTEST_MAIN(TestDatasetManager)
