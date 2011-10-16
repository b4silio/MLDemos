#ifndef PROJECTORICA_H
#define PROJECTORICA_H

#include <public.h>
#include <mymaths.h>
#include <projector.h>

class ProjectorICA : public Projector
{
private:
    fvec meanAll;
    double* Transf;
public:
    int method;
    ProjectorICA(int method=0);
    ~ProjectorICA();

    void Train(std::vector< fvec > samples, int startIndex=0, int stopIndex=-1);
    fvec Project(const fvec &sample);
    char *GetInfoString(){return "Independent Component Analysis";}
    double *GetTransf(){return Transf;}
};

#endif // PROJECTORICA_H
