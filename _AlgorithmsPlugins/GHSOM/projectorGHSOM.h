#ifndef CLUSTERERGHSOM_H
#define CLUSTERERGHSOM_H

#include <public.h>
#include <mymaths.h>
#include <projector.h>

class ProjectorGHSOM : public Projector
{
public:
    long num_dims;

    ProjectorGHSOM();

    void Train(std::vector< fvec > samples, ivec labels);
    fvec Project(const fvec &sample);
    const char *GetInfoString(){return "GHSOM";}
    void SetParams(float tau1, float tau2, int xSize, int ySize, int expandCycles, int normalizationType, float learningRate, float neighborhoodRadius);
};

#endif // CLUSTERERGHSOM_H
