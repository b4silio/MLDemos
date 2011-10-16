#include "projectorICA.h"
#include <JnS/JnS.h>
#include <JnS/Matutil.h>

using namespace std;

ProjectorICA::ProjectorICA(int method)
    : Transf(0), method(method)
{
}

ProjectorICA::~ProjectorICA()
{
    KILL(Transf);
}

void ProjectorICA::Train(std::vector<fvec> samples, int startIndex, int stopIndex)
{
    this->startIndex=startIndex;
    this->stopIndex=stopIndex;

    if(!samples.size()) return;
    dim = samples[0].size();
    meanAll.resize(dim,0);
    FOR(i, samples.size())
    {
        meanAll += samples[i];
    }
    meanAll /= samples.size();

    const int nbsensors = dim;
    const int nbsamples = samples.size();

    if(!Transf)
    {
        Transf = new double[nbsensors*nbsensors];
    }
    double *Data, *Mixing;
    Data = new double[nbsensors*nbsamples];
    Mixing = new double[nbsensors*nbsensors];

    FOR(i, samples.size())
    {
        FOR(d, nbsensors)
        {
            Data[i*nbsensors + d] = samples[i][d] - meanAll[d];
            //		Data[i*nbsensors + d] = rand()/(float)RAND_MAX/5.;
        }
    }

    Identity(Mixing, nbsensors);
    Mixing[0] = 2.0 ;
    switch(method)
    {
    case 0:
        Jade(Transf, Data, nbsensors, nbsamples ) ;
        break;
    case 1:
        Shibbs(Transf, Data, nbsensors, nbsamples ) ;
        break;
    }

    FOR(i,nbsensors*nbsensors) Transf[i] /= 10;

    projected = vector<fvec>(samples.size());
    FOR(i, samples.size())
    {
        projected[i].resize(dim);
        FOR(d, dim)
        {
            projected[i][d] = Data[i*nbsensors + d];
        }
    }

    delete [] Data;
    delete [] Mixing;
}

fvec ProjectorICA::Project(const fvec &sample)
{
    int dim = sample.size();
    if(!dim) return sample;
    double *X;
    if((X = (double *) calloc(dim*1, sizeof(double))) == NULL) OutOfMemory();
    Transform(X, Transf, dim, 1);
}
