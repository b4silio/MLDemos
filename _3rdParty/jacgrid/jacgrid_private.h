#ifndef JAC_JACGRID_PRIVATE_H
#define JAC_JACGRID_PRIVATE_H

#include "jacgrid.h"

bool jacMakeSurface(surfaceT &surf, unsigned int itype,
                    const gridT &grd, float thold,
                    const JACAtomsBase &atms,
                    const surfaceT *old_surface = 0);
#endif
