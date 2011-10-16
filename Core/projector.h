#ifndef PROJECTOR_H
#define PROJECTOR_H

#include <vector>
#include "mymaths.h"

class Projector
{
protected:
    std::vector<fvec> projected;

public:
    u32 dim;
    u32 startIndex, stopIndex;

    Projector() : dim(2), startIndex(0), stopIndex(-1) {}
    ~Projector(){}

    virtual void Train(std::vector< fvec > samples, int startIndex=0, int stopIndex=-1){this->startIndex=startIndex; this->stopIndex=stopIndex;}
    virtual fvec Project(const fvec &sample){ return sample; }
    virtual fvec Project(const fVec &sample){ return Project((fvec)sample); }
    virtual char *GetInfoString(){return NULL;}
    virtual std::vector<fvec> GetProjected(){ return projected; }
};

#endif // PROJECTOR_H
