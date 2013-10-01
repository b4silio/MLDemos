#include "projectorNormalize.h"
#include <mymaths.h>
#include <Eigen/Core>
#include <Eigen/Eigen>
#include <Eigen/LU>
#include <QDebug>

#include <map>

using namespace std;
using namespace Eigen;

ProjectorNormalize::ProjectorNormalize()
    : type(0), rangeMin(0.f), rangeMax(1.f), rangeDiff(1.f), dimension(-1)
{}

void ProjectorNormalize::Train(std::vector< fvec > samples, ivec labels)
{
    projected.clear();
    source.clear();
    if(!samples.size()) return;
    source = samples;
    projected = samples;
    dim = samples[0].size();
    if(dimension >= dim) dimension = dim-1;
    dataMin = fvec(dim,FLT_MAX);
    dataMax = fvec(dim,-FLT_MAX);
    dataMean = fvec(dim);
    dataSigma = fvec(dim);
    FOR(i, samples.size()) {
        FOR(d, dim) {
            dataMin[d] = min(samples[i][d], dataMin[d]);
            dataMax[d] = max(samples[i][d], dataMax[d]);
            dataMean[d] += samples[i][d];
        }
    }
    dataMean /= samples.size();
    FOR(i, samples.size())
    {
        FOR(d, dim) dataSigma[d] += (samples[i][d] - dataMean[d])*(samples[i][d] - dataMean[d]);
    }
    dataSigma /= samples.size();
    dataDiff = dataMax-dataMin;
    FOR(i, samples.size()) projected[i] = Project(samples[i]);
}

fvec ProjectorNormalize::Project(const fvec &sample)
{
    fvec newSample = sample;
    switch(type){
    case 0: // range
    {
        if(dimension >= 0 && dimension < dim) {
            newSample[dimension] = ((sample[dimension]-dataMin[dimension]) / dataDiff[dimension])*rangeDiff + rangeMin;
        } else {
            FOR(d, dim) newSample[d] = ((sample[d]-dataMin[d]) / dataDiff[d])*rangeDiff + rangeMin;
        }
    }
        break;
    case 1: // variance
    {
        if(dimension >= 0 && dimension < dim) {
            newSample[dimension] = ((sample[dimension]-dataMean[dimension]) / sqrtf(dataSigma[dimension]))*rangeMax + rangeMin;
        } else {
            FOR(d, dim) newSample[d] = ((sample[d]-dataMean[d]) / sqrtf(dataSigma[d]))*rangeMax + rangeMin;
        }
    }
        break;
    case 2: // center
    {
        if(dimension >= 0 && dimension < dim) {
            newSample[dimension] = (sample[dimension]-dataMean[dimension]) + rangeMin;
        } else {
            newSample = (sample-dataMean) + rangeMin;
        }
    }
        break;
    }
    return newSample;
}

void ProjectorNormalize::SetParams(int type, float rangeMin, float rangeMax, int dimension)
{
    this->type = type;
    this->rangeMin = rangeMin;
    this->rangeMax = rangeMax;
    rangeDiff = rangeMax-rangeMin;
    this->dimension = dimension;
}

