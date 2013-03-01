#ifndef PROJECTORSAMMON_H
#define PROJECTORSAMMON_H

#include <public.h>
#include <mymaths.h>
#include <projector.h>
#include <dlib/statistics.h>
//#include "dlibTypes.h"


class ProjectorSammon : public Projector
{
public:
    long num_dims;

    ProjectorSammon();

    void Train(std::vector< fvec > samples, ivec labels);
    fvec Project(const fvec &sample);
    const char *GetInfoString(){return "Sammon Projection";}
};

#endif // PROJECTORSAMMON_H
