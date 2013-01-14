#ifndef JAC_JACGRID_H
#define JAC_JACGRID_H

#include <stdlib.h>
#include <string.h>
#include <limits.h>


#include <string>
#include <vector>

#include "atom.h"
#include "grid.h"
#include "surface.h"
#include "plane.h"

unsigned int JACGetGridDimension();
void         JACSetGridDimension(unsigned int);
float        JACGetProbeRadius();
void         JACSetProbeRadius(float r);

////////////////////////////////////////////////////
// surfacing and volumes
////////////////////////////////////////////////////
namespace JACSurfaceTypes
{
    const unsigned int SURF_ACCESSIBLE = 0;
    const unsigned int SURF_MOLECULAR = 1;
    const unsigned int SURF_VDW = 2;
    const unsigned int SURF_CONTOUR = 3;
    const unsigned int SURF_TYPES_MAX = 3;
}

bool JACMakeAccessibleSurface(surfaceT &surf, const JACAtomsBase &atoms);

bool JACMakeMolecularSurface(surfaceT &surf, const JACAtomsBase &atoms);

bool JACMakeSurface(surfaceT &surf, unsigned int itype,
                    const gridT &grd, float threshold);

void JACSetGridParams(gridT &grid, unsigned int itype,
                      bool uniform, const JACAtomsBase &atoms);

void JACSurfaceNormalize(surfaceT &surface);

float JACCalculateVolume(unsigned int itype, const JACAtomsBase &atoms);

float JACCalculateAccessibleVolume(const JACAtomsBase &atoms);

float JACCalculateMolecularVolume(const JACAtomsBase &atoms);

float JACCalculateVdWVolume(const JACAtomsBase &atoms);

float JACSurfaceArea(const surfaceT &surface,
                     unsigned int *selection = 0,
                     unsigned int sel_mask = 1);

float JACSurfaceVolume(const surfaceT &surface,
                       unsigned int *selection = 0,
                       unsigned int sel_mask = 1);

void JACSurfaceSwapFace(surfaceT &surface,
                        unsigned int *select = 0,
                        unsigned int sel_mask = 1);

void JACInvertTriangles(surfaceT &surface,
                        unsigned int *select = 0,
                        unsigned int sel_mask = 1);

void JACOverlapGrid(gridT &grid,
                    const JACAtomsBase &atoms1,
                    const JACAtomsBase &atoms2);

void JACDifferenceGrid(gridT &grid,
                       const JACAtomsBase &atoms1,
                       const JACAtomsBase &atoms2);

void JACOverlapSurface(surfaceT &surface,
                       const JACAtomsBase &atoms1,
                       const JACAtomsBase &atoms2);

void JACDifferenceSurface(surfaceT &surface,
                          const JACAtomsBase &atoms1,
                          const JACAtomsBase &atoms2);

void JACSmoothSurface(surfaceT &surface,
                      unsigned int begin = 0,
                      unsigned int end = UINT_MAX);

class VertexSender
{
public:
    virtual ~VertexSender()
    {
    }
    virtual void BeginLine()
    {
    }
    virtual void EndLine()
    {
    }
    virtual void SendVertex(const float *v) = 0;
};


float JACSurfaceArea(const JACAtomsBase &atoms,
                     const JACAtomsBase &context,
                     float *surface_area,
                     float &fractional,
                     VertexSender *vs = 0,
                     unsigned int pres = 1);
#endif // ifndef JAC_JACGRID_H
