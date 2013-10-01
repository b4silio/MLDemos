#ifndef PROJECTORNORMALIZE_H
#define PROJECTORNORMALIZE_H

#include <public.h>
#include <mymaths.h>
#include <projector.h>

class ProjectorNormalize : public Projector
{
    float rangeMin, rangeMax, rangeDiff;
    fvec dataMin, dataMax, dataDiff, dataMean, dataSigma;
    int dimension;
public:
    int type;
    ProjectorNormalize();

    void Train(std::vector< fvec > samples, ivec labels);
    fvec Project(const fvec &sample);
    const char *GetInfoString(){return "Normalization";}
    void SetParams(int type, float rangeMin, float rangeMax, int dimension);
};

#endif // PROJECTORNORMALIZE_H
