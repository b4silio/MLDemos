#include "projectorGHSOM.h"
#include <mymaths.h>
#include <QDebug>
#include <GHSOM/globals.h>
#include <map>

using namespace std;

ProjectorGHSOM::ProjectorGHSOM()
{
    Globals::EXPAND_CYCLES = 10; // after how many iterations through the data we check if we want to expand
    Globals::MAX_CYCLES = 0; // maximum number of iterations through data (0: ignored)
    Globals::TAU_1 = 0.01; // percentage of remaining error that needs to be explained by each map. 1.0 == standard SOM
    Globals::TAU_2 = 1.0; // granularity of the last layer of maps 1.0 == non-hierarchical SOM (1 map only)
    Globals::INITIAL_LEARNRATE = 0.7; // how strongly the winner and neighboring units are adapted initially, decreases over time
    Globals::NR = 0.0009; // Neighborhood radius
    Globals::randomSeed = 1; // change ad libitum
    Globals::normInputVectors=NORM_LENGTH; // NONE, LENGTH, INTERVAL. Normalize data by their length or clamp in a 0-1 interval
    Globals::INITIAL_X_SIZE = 2; // initial size in the x direction
    Globals::INITIAL_Y_SIZE = 2; // initial size in the y direction
    Globals::LABELS_NUM = 0; // max number of labels per unit. 0: no labels
}

void ProjectorGHSOM::SetParams(float tau1, float tau2, int xSize, int ySize, int expandCycles, int normalizationType, float learningRate, float neighborhoodRadius)
{
    Globals::TAU_1 = tau1;
    Globals::TAU_2 = tau2;
    Globals::INITIAL_X_SIZE = xSize;
    Globals::INITIAL_Y_SIZE = ySize;
    Globals::EXPAND_CYCLES = expandCycles;
    Globals::normInputVectors = normalizationType;
    Globals::INITIAL_LEARNRATE = learningRate;
    Globals::NR = neighborhoodRadius;
}

void ProjectorGHSOM::Train(std::vector< fvec > samples, ivec labels)
{
    if(!samples.size()) return;
    source = samples;
    projected = samples;
    dim = samples[0].size();

    Data_Vector *res = new Data_Vector();
    if(Globals::layers) Globals::layers->removeAllElements();

    Globals::vectorlength = dim;
    Globals::numofallvecs = samples.size();
    //Globals::currentDataLoader = new DataLoader();

    FOR(i, samples.size())
    {
        float *data = new float[dim];
        FOR(d, dim) data[d] = samples[i][d];

        if (Globals::normInputVectors==NORM_LENGTH)
        {
            data = Globals::normVec(data);
        }
        char *id = new char[150];
        sprintf(id,"sample%d", i+1);
        DataItem *di = new DataItem(id,data,dim);
        res->addElement(di);
        delete [] data;
        delete [] id;
    }
    if (Globals::normInputVectors==NORM_INTERVAL) res = Globals::normIntervalVector(res);
    float *mean = Globals::meanVector(res, dim);
    qDebug() << "mean" << mean[0] << mean[1];

    Globals::dataItems = res;

    char **vecDescr = new char *[dim];
    FOR(d, dim)
    {
        vecDescr[d] = new char[150];
        sprintf(vecDescr[d], "dim%1", d+1);
    }

    Globals::vectorDescription = vecDescr;

    Globals::initHFM();

    Globals::trainHFM();

//    Globals::savePath = "/Users/basilio/Desktop";
//    Globals::saveHFMAs(HTML);

    //    if (Globals::SAVE_AS_HTML) Globals::saveHFMAs(HTML);
    //    if(Globals::SAVE_AS_SOMLIB) Globals::saveHFMAs(SOMLIB);

}

fvec ProjectorGHSOM::Project(const fvec &sample)
{
    return sample;
}
