#include "grid.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>

bool gridT::Resize(float InitValue,
                   unsigned int dimx, unsigned int dimy, unsigned int dimz,
                   bool with_surface_fields)
{
    register unsigned int i;
    float *dataptr;
    unsigned int *ownerptr;
    unsigned int *typeptr;

    _valid = false;
    _size = dimx*dimy*dimz;
    if (!dimx || !dimy || !dimz)
        return false;

    /* initialize the arrays */
    npts[0] = dimx;
    npts[1] = dimy;
    npts[2] = dimz;

    if (data) free(data);
    if (type) free(type);
    if (owner) free(owner);

    data = 0;
    type = 0;
    owner = 0;

    data = (float*)malloc(sizeof(float)*_size);
    if (!data)
        return false;

    if (with_surface_fields)
    {
        type = (unsigned int*)malloc(sizeof(unsigned int)*_size);
        owner = (unsigned int*)malloc(sizeof(unsigned int)*_size);
        if (!type || !owner)
        {
            free(data);
            free(type);
            free(owner);
            return false;
        }

        for (i = 0, dataptr = data, typeptr = type, ownerptr = owner;
             i<_size; i++)
        {
            *dataptr++ = InitValue;
            *typeptr++ = 0;
            *ownerptr++ = UINT_MAX; /* 0 is a valid owner! */
        }
    }
    else
    {
        for (i = 0, dataptr = data; i<_size; i++)
            *dataptr++ = InitValue;
    }

    _valid = true;
    return true;
}

gridT::gridT()
{
    data = 0;
    owner = 0;
    type = 0;
    Clear();
}

gridT::gridT(float InitVal,
             unsigned int dimx, unsigned int dimy, unsigned int dimz,
             bool with_surface_fields)
{
    data = 0;
    owner = 0;
    type = 0;
    Clear();
    Resize(InitVal, dimx, dimy, dimz, with_surface_fields);
}

float &gridT::operator[](unsigned int index)
{
    return data[index];
}

gridT::~gridT()
{
    free(data);
    free(type);
    free(owner);
}

void gridT::Clear()
{
    _valid = false;
    npts[0] = npts[1] = npts[2] = 0;
    for (unsigned int i = 0; i<3; ++i)
    {
        unit[i] = 0.0;
        size[i] = 0.0;
        org[i] = 0.0;
        center[i] = 0.0;
    }

    if (data)
        memset(data, 0, sizeof(float)*_size);
    if (type)
        memset(type, 0, sizeof(unsigned int)*_size);
    if (owner)
        memset(owner, 0, sizeof(unsigned int)*_size);

    _valid = false;
}






