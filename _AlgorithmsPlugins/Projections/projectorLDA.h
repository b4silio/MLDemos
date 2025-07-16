#ifndef PROJECTORLDA_H
#define PROJECTORLDA_H

#include <public.h>
#include <mymaths.h>
#include <projector.h>

class ProjectorLDA : public Projector
{
    fvec w, mean;
public:
    int ldaType;
    ProjectorLDA();

    void Train(std::vector< fvec > samples, ivec labels);
    fvec Project(const fvec &sample);
    const char *GetInfoString(){return "Linear Discriminant Analysis";}
};

#endif // PROJECTORLDA_H
