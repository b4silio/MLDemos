#include "jacgrid.h"
#include <stdio.h>
#include <cmath>

#include "cell_table.h"

#define addr(i, j, k) (((k)*XDIMYDIM) + ((j)*XDIM) + (i))
#define saddr(x, y, n) (((x)*YDIM*13) + ((y)*13) + (n))


static void contour_calc_cell_verts(unsigned int, unsigned int, unsigned int, unsigned int);
static void accessible_calc_cell_verts(unsigned int, unsigned int, unsigned int, unsigned int);
static void molecular_calc_cell_verts(unsigned int, unsigned int, unsigned int, unsigned int);

static void calc_index_and_temps(const float *data, unsigned int x1, unsigned int y1, unsigned int z1,
                                 unsigned int *index);


// these are static variables to avoid putting them on the stack
// repeatedly for each element of the grid
static float DATA1, DATA2, DATA3, DATA4, DATA5, DATA6, DATA7, DATA8;
static unsigned int XDIM, YDIM, ZDIM, XDIMYDIM;
static float threshold;
static float xtrans, ytrans, ztrans;
static unsigned int crossings[13];
static surfaceT *surface;
static const surfaceT *scratch_surface;
static const gridT *grid;
static unsigned int *this_slice, *last_slice;
static const JACAtomsBase *atoms;
static float PRAD;

static const unsigned int REENTRANT_SURFACE = 1;
static const unsigned int CONTACT_SURFACE = 2;

/*****************************************************************************
 *
 * Heavily, heavily modified from iso.c, part of the isovis distribution,
 * developed at the National Center for Supercomputing Applications at the
 * University of Illinois at Urbana-Champaign.
 *
 * This program implements the marching cubes surface tiler described by
 * Lorensen & Cline in the Siggraph 87 Conference Proceedings.
 *
 * Written by Mike Krogh, NCSA, Feb.  2, 1990
 *
 ****************************************************************************/

bool jacMakeSurface(surfaceT &surf, unsigned int itype,
                    const gridT &grd, float thold,
                    const JACAtomsBase &atms,
                    const surfaceT *old_surface)
{
    //set globals
    threshold = thold;
    surface = &surf;
    grid = &grd;
    scratch_surface = old_surface;
    xtrans = grid->org[0];
    ytrans = grid->org[1];
    ztrans = grid->org[2];

    for (int i = 0; i<3; ++i)
        surf.gridUnit[i] = grd.unit[i];

    PRAD = JACGetProbeRadius();
    atoms = &atms;

    register unsigned int x, y, z;
    register unsigned int xdim1, ydim1, zdim1;
    unsigned int index;
    unsigned int npolys;
    unsigned int *swap;
    register unsigned int num_o_polys;
    register unsigned int poly;
    register unsigned int poly3;
    register unsigned int *polys;
    void (*calc_cell_verts)(unsigned int, unsigned int, unsigned int, unsigned int);

    /* pointer to apropriate vert calc routine */
    switch (itype)
    {
    case JACSurfaceTypes::SURF_MOLECULAR:
        calc_cell_verts = molecular_calc_cell_verts;
        break;
    case JACSurfaceTypes::SURF_ACCESSIBLE:
        calc_cell_verts = accessible_calc_cell_verts;
        break;
    case JACSurfaceTypes::SURF_CONTOUR:
        calc_cell_verts = contour_calc_cell_verts;
        break;
    default:
        surface->Resize(0, 0);
        return 0;
        break;
    }

    XDIM = grid->npts[0];
    YDIM = grid->npts[1];
    ZDIM = grid->npts[2];
    XDIMYDIM = XDIM*YDIM;

    zdim1 = ZDIM-1;
    ydim1 = YDIM-1;
    xdim1 = XDIM-1;

    npolys = 0; /* keep count of total polygons */

    this_slice = (unsigned int*)malloc(13*sizeof(unsigned int)*grid->npts[1]*grid->npts[0]);
    last_slice = (unsigned int*)malloc(13*sizeof(unsigned int)*grid->npts[1]*grid->npts[0]);
    if (!this_slice || !last_slice)
    {
        free(this_slice);
        free(last_slice);
        surface->Resize(0, 0);
        return 0;
    }

    for (z = 0; z<zdim1; z++)
    {
        /* process each cell in the volume */
        /* swap this & last slice */
        swap = this_slice;
        this_slice = last_slice;
        last_slice = swap;

        // recalc max sizes here if necessary, to avoid doing this in the inner loop
        if (surface->nverts + 15*xdim1*ydim1 > surface->maxverts
            || surface->nconn + 30*xdim1*ydim1 > surface->maxconn)
            surface->Resize(surface->nverts + 15*xdim1*ydim1,
                            surface->nconn + 30*xdim1*ydim1);

        for (y = 0; y<ydim1; y++)
        {
            for (x = 0; x<xdim1; x++)
            {
                calc_index_and_temps(grid->data, x, y, z, &index);
                if (index)
                {
                    calc_cell_verts(index, x, y, z);

                    num_o_polys = cell_table[index].npolys;
                    polys = &cell_table[index].polys[0];

                    for (poly = 0; poly<num_o_polys; poly++)
                    {
                        poly3 = poly*3;
                        surface->triangles[surface->nconn++] = crossings[polys[poly3]];
                        surface->triangles[surface->nconn++] = crossings[polys[poly3+1]];
                        surface->triangles[surface->nconn++] = crossings[polys[poly3+2]];
                    }
                    npolys += num_o_polys;
                }
            }
        }
    }

    //printf("%d new polygons generated from %d vertices.\n",npolys, surface->nverts);
    free(this_slice);
    free(last_slice);

    surface->Resize(surface->nverts, surface->nconn);
    return 1;
}

static void calc_index_and_temps(const float *data,
                                 unsigned int x1, unsigned int y1, unsigned int z1,
                                 unsigned int *index)
/* This subroutine calculates the index and creates some global */
/* temporary variables (for speed). */
{

    register const float *tmp;

    *index = 0;

    tmp = data + (z1*XDIMYDIM) + (y1*XDIM) + x1;

    *index += (threshold <= (DATA1 = *(tmp)));
    *index += (threshold <= (DATA2 = *(tmp + 1))) * 2;

    tmp += XDIM;
    *index += (threshold <= (DATA3 = *(tmp + 1))) * 4;
    *index += (threshold <= (DATA4 = *(tmp))) * 8;

    tmp = tmp - XDIM + XDIMYDIM;
    *index += (threshold <= (DATA5 = *(tmp))) * 16;
    *index += (threshold <= (DATA6 = *(tmp + 1))) * 32;

    tmp += XDIM;
    *index += (threshold <= (DATA7 = *(tmp + 1))) * 64;
    *index += (threshold <= (DATA8 = *(tmp))) * 128;

}

/*-
   there are several nearly identical routines to calculate the vertices of
   a cell.  Although odd this is, in fact, the Right Thing, because 1) it
   eliminates the necessity of a lot of if-checking to see what kind of
   surface we are building, and 2) the interpolation routines are different
   for different kinds of surfaces.

   For a plain contour, we don't need to worry about owners and such for
   smoothing or any other reason, so we don't deal with them there.

   -*/
static void contour_calc_cell_verts(unsigned int index, unsigned int x1, unsigned int y1, unsigned int z1)
{
    register unsigned int i;
    register unsigned int x2, y2, z2;
    unsigned int nedges;
    unsigned int crnt_edge;
    float *vrtx;

#define linterp(a1, a2, a, b1, b2, unit) \
    (unit*((float)(((a-a1) * (float)(b2-b1) / (a2-a1)) + (float)b1)))

    x2 = x1+1;
    y2 = y1+1;
    z2 = z1+1;

    nedges = cell_table[index].nedges;
    for (i = 0; i<nedges; i++)
    {
        crnt_edge = cell_table[index].edges[i];
        vrtx = &surface->vertices[surface->nverts*3];
        switch (crnt_edge)
        {
        case 1:
            if (z1==0)
            {
                *vrtx++ = linterp(DATA1, DATA2, threshold, x1, x2, grid->unit[0])+xtrans;
                *vrtx++ = grid->unit[1]*(float)y1+ytrans;
                *vrtx++ = grid->unit[2]*(float)z1+ztrans;
                this_slice[saddr(x1, y1, 1)] =
                    crossings[1] = surface->nverts;
                surface->nverts++;
            }
            else
                this_slice[saddr(x1, y1, 1)] =
                    crossings[1] = last_slice[saddr(x1, y1, 5)];
            break;

        case 2:
            if (z1==0)
            {
                *vrtx++ = grid->unit[0]*(float)x2+xtrans;
                *vrtx++ = linterp(DATA2, DATA3, threshold, y1, y2, grid->unit[1])+ytrans;
                *vrtx++ = grid->unit[2]*(float)z1+ztrans;
                this_slice[saddr(x1, y1, 2)] =
                    crossings[2] = surface->nverts;
                surface->nverts++;
            }
            else
                this_slice[saddr(x1, y1, 2)] =
                    crossings[2] = last_slice[saddr(x1, y1, 6)];
            break;

        case 3:
            if (z1==0)
            {
                *vrtx++ = linterp(DATA4, DATA3, threshold, x1, x2, grid->unit[0])+xtrans;
                *vrtx++ = grid->unit[1]*(float)y2+ytrans;
                *vrtx++ = grid->unit[2]*(float)z1+ztrans;
                this_slice[saddr(x1, y1, 3)] =
                    crossings[3] = surface->nverts;
                surface->nverts++;
            }
            else
                this_slice[saddr(x1, y1, 3)] =
                    crossings[3] = last_slice[saddr(x1, y1, 7)];
            break;

        case 4:
            if (z1==0)
            {
                *vrtx++ = grid->unit[0]*(float)x1+xtrans;
                *vrtx++ = linterp(DATA1, DATA4, threshold, y1, y2, grid->unit[1])+ytrans;
                *vrtx++ = grid->unit[2]*(float)z1+ztrans;
                this_slice[saddr(x1, y1, 4)] =
                    crossings[4] = surface->nverts;
                surface->nverts++;
            }
            else
                this_slice[saddr(x1, y1, 4)] =
                    crossings[4] = last_slice[saddr(x1, y1, 8)];
            break;

        case 5:
            if (y1==0)
            {
                *vrtx++ = linterp(DATA5, DATA6, threshold, x1, x2, grid->unit[0])+xtrans;
                *vrtx++ = grid->unit[1]*(float)y1+ytrans;
                *vrtx++ = grid->unit[2]*(float)z2+ztrans;
                this_slice[saddr(x1, y1, 5)] =
                    crossings[5] = surface->nverts;
                surface->nverts++;
            }
            else
                this_slice[saddr(x1, y1, 5)] =
                    crossings[5] = this_slice[saddr(x1, y1-1, 7)];
            break;

        case 6:
            *vrtx++ = grid->unit[0]*(float)x2+xtrans;
            *vrtx++ = linterp(DATA6, DATA7, threshold, y1, y2, grid->unit[1])+ytrans;
            *vrtx++ = grid->unit[2]*(float)z2+ztrans;
            this_slice[saddr(x1, y1, 6)] =
                crossings[6] = surface->nverts;
            surface->nverts++;
            break;

        case 7:
            *vrtx++ = linterp(DATA8, DATA7, threshold, x1, x2, grid->unit[0])+xtrans;
            *vrtx++ = grid->unit[1]*(float)y2+ytrans;
            *vrtx++ = grid->unit[2]*(float)z2+ztrans;
            this_slice[saddr(x1, y1, 7)] =
                crossings[7] = surface->nverts;
            surface->nverts++;
            break;

        case 8:
            if (x1==0)
            {
                *vrtx++ = grid->unit[0]*(float)x1+xtrans;
                *vrtx++ = linterp(DATA5, DATA8, threshold, y1, y2, grid->unit[1])+ytrans;
                *vrtx++ = grid->unit[2]*(float)z2+ztrans;
                this_slice[saddr(x1, y1, 8)] =
                    crossings[8] = surface->nverts;
                surface->nverts++;
            }
            else
                this_slice[saddr(x1, y1, 8)] =
                    crossings[8] = this_slice[saddr(x1-1, y1, 6)];
            break;

        case 9:
            if (x1==0)
            {
                *vrtx++ = grid->unit[0]*(float)x1+xtrans;
                *vrtx++ = grid->unit[1]*(float)y1+ytrans;
                *vrtx++ = linterp(DATA1, DATA5, threshold, z1, z2, grid->unit[2])+ztrans;
                this_slice[saddr(x1, y1, 9)] =
                    crossings[9] = surface->nverts;
                surface->nverts++;
            }
            else
                this_slice[saddr(x1, y1, 9)] =
                    crossings[9] = this_slice[saddr(x1-1, y1, 10)];
            break;

        case 10:
            if (y1==0)
            {
                *vrtx++ = grid->unit[0]*(float)x2+xtrans;
                *vrtx++ = grid->unit[1]*(float)y1+ytrans;
                *vrtx++ = linterp(DATA2, DATA6, threshold, z1, z2, grid->unit[2])+ztrans;
                this_slice[saddr(x1, y1, 10)] =
                    crossings[10] = surface->nverts;
                surface->nverts++;
            }
            else
                this_slice[saddr(x1, y1, 10)] =
                    crossings[10] = this_slice[saddr(x1, y1-1, 12)];
            break;

        case 11:
            if (x1==0)
            {
                *vrtx++ = grid->unit[0]*(float)x1+xtrans;
                *vrtx++ = grid->unit[1]*(float)y2+ytrans;
                *vrtx++ = linterp(DATA4, DATA8, threshold, z1, z2, grid->unit[2])+ztrans;
                this_slice[saddr(x1, y1, 11)] =
                    crossings[11] = surface->nverts;
                surface->nverts++;
            }
            else
                this_slice[saddr(x1, y1, 11)] =
                    crossings[11] = this_slice[saddr(x1-1, y1, 12)];
            break;

        case 12:
            *vrtx++ = grid->unit[0]*(float)x2+xtrans;
            *vrtx++ = grid->unit[1]*(float)y2+ytrans;
            *vrtx++ = linterp(DATA3, DATA7, threshold, z1, z2, grid->unit[2])+ztrans;
            this_slice[saddr(x1, y1, 12)] =
                crossings[12] = surface->nverts;
            surface->nverts++;
            break;

        } /* end switch */
    }
}
#undef linterp

static inline void do_smooth(float rad, const float *center, float *vertex)
{
    float v[3];
    float sum;
    float scale;

    v[0] = vertex[0]-center[0];
    v[1] = vertex[1]-center[1];
    v[2] = vertex[2]-center[2];
    sum = v[0]*v[0]+v[1]*v[1]+v[2]*v[2];
    if (sum>=.00001)
    {
        scale = rad/(float)sqrt((double)sum);
        vertex[0] = v[0]*scale + center[0];
        vertex[1] = v[1]*scale + center[1];
        vertex[2] = v[2]*scale + center[2];
    }
}

static void accessible_calc_cell_verts(unsigned int index, unsigned int x1, unsigned int y1, unsigned int z1)
{
    register unsigned int i;
    register unsigned int x2, y2, z2;
    register unsigned int thisaddr = 0;
    unsigned int nedges, owner;
    unsigned int crnt_edge;
    unsigned int lookedup;
    float *vrtx, *vrtx_sav;

    x2 = x1+1;
    y2 = y1+1;
    z2 = z1+1;

    nedges = cell_table[index].nedges;
    for (i = 0; i<nedges; i++)
    {
        crnt_edge = cell_table[index].edges[i];
        vrtx_sav = vrtx = &surface->vertices[surface->nverts*3];
        lookedup = true;
        switch (crnt_edge)
        {
        case 1:
            if (z1==0)
            {
                lookedup = false;
                if (index&1)
                    thisaddr = addr(x1, y1, z1);
                else
                    thisaddr = addr(x2, y1, z1);

                *vrtx++ = grid->unit[0]*(float)(x1+0.5)+xtrans;
                *vrtx++ = grid->unit[1]*(float)y1+ytrans;
                *vrtx++ = grid->unit[2]*(float)z1+ztrans;
                this_slice[saddr(x1, y1, 1)] =
                    crossings[1] = surface->nverts;
            }
            else
                this_slice[saddr(x1, y1, 1)] =
                    crossings[1] = last_slice[saddr(x1, y1, 5)];
            break;

        case 2:
            if (z1==0)
            {
                lookedup = false;
                if (index&2)
                    thisaddr = addr(x2, y1, z1);
                else
                    thisaddr = addr(x2, y2, z1);

                *vrtx++ = grid->unit[0]*(float)x2+xtrans;
                *vrtx++ = grid->unit[1]*(float)(y1+0.5)+ytrans;
                *vrtx++ = grid->unit[2]*(float)z1+ztrans;
                this_slice[saddr(x1, y1, 2)] =
                    crossings[2] = surface->nverts;
            }
            else
                this_slice[saddr(x1, y1, 2)] =
                    crossings[2] = last_slice[saddr(x1, y1, 6)];
            break;

        case 3:
            if (z1==0)
            {
                lookedup = false;
                if (index&4)
                    thisaddr = addr(x2, y2, z1);
                else
                    thisaddr = addr(x1, y2, z1);
                *vrtx++ = grid->unit[0]*(float)(x1+0.5)+xtrans;
                *vrtx++ = grid->unit[1]*(float)y2+ytrans;
                *vrtx++ = grid->unit[2]*(float)z1+ztrans;
                this_slice[saddr(x1, y1, 3)] =
                    crossings[3] = surface->nverts;
            }
            else
                this_slice[saddr(x1, y1, 3)] =
                    crossings[3] = last_slice[saddr(x1, y1, 7)];
            break;

        case 4:
            if (z1==0)
            {
                lookedup = false;
                if (index&8)
                    thisaddr = addr(x1, y2, z1);
                else
                    thisaddr = addr(x1, y1, z1);

                *vrtx++ = grid->unit[0]*(float)x1+xtrans;
                *vrtx++ = grid->unit[1]*(float)(y1+0.5)+ytrans;
                *vrtx++ = grid->unit[2]*(float)z1+ztrans;
                this_slice[saddr(x1, y1, 4)] =
                    crossings[4] = surface->nverts;
            }
            else
                this_slice[saddr(x1, y1, 4)] =
                    crossings[4] = last_slice[saddr(x1, y1, 8)];
            break;

        case 5:
            if (y1==0)
            {
                lookedup = false;
                if (index&16)
                    thisaddr = addr(x1, y1, z2);
                else
                    thisaddr = addr(x2, y1, z2);
                *vrtx++ = grid->unit[0]*(float)(x1+0.5)+xtrans;
                *vrtx++ = grid->unit[1]*(float)y1+ytrans;
                *vrtx++ = grid->unit[2]*(float)z2+ztrans;
                this_slice[saddr(x1, y1, 5)] =
                    crossings[5] = surface->nverts;
            }
            else
                this_slice[saddr(x1, y1, 5)] =
                    crossings[5] = this_slice[saddr(x1, y1-1, 7)];
            break;

        case 6:
            lookedup = false;
            if (index&32)
                thisaddr = addr(x2, y1, z2);
            else
                thisaddr = addr(x2, y2, z2);
            *vrtx++ = grid->unit[0]*(float)x2+xtrans;
            *vrtx++ = grid->unit[1]*(float)(y1+0.5)+ytrans;
            *vrtx++ = grid->unit[2]*(float)z2+ztrans;
            this_slice[saddr(x1, y1, 6)] =
                crossings[6] = surface->nverts;
            break;
        case 7:
            lookedup = false;
            if (index&64)
                thisaddr = addr(x2, y2, z2);
            else
                thisaddr = addr(x1, y2, z2);
            *vrtx++ = grid->unit[0]*(float)(x1+0.5)+xtrans;
            *vrtx++ = grid->unit[1]*(float)y2+ytrans;
            *vrtx++ = grid->unit[2]*(float)z2+ztrans;
            this_slice[saddr(x1, y1, 7)] =
                crossings[7] = surface->nverts;
            break;

        case 8:
            if (x1==0)
            {
                lookedup = false;
                if (index&128)
                    thisaddr = addr(x1, y2, z2);
                else
                    thisaddr = addr(x1, y1, z2);
                *vrtx++ = grid->unit[0]*(float)x1+xtrans;
                *vrtx++ = grid->unit[1]*(float)(y1+0.5)+ytrans;
                *vrtx++ = grid->unit[2]*(float)z2+ztrans;
                this_slice[saddr(x1, y1, 8)] =
                    crossings[8] = surface->nverts;
            }
            else
                this_slice[saddr(x1, y1, 8)] =
                    crossings[8] = this_slice[saddr(x1-1, y1, 6)];
            break;

        case 9:
            if (x1==0)
            {
                lookedup = false;
                if (index&1)
                    thisaddr = addr(x1, y1, z1);
                else
                    thisaddr = addr(x1, y1, z2);
                *vrtx++ = grid->unit[0]*(float)x1+xtrans;
                *vrtx++ = grid->unit[1]*(float)y1+ytrans;
                *vrtx++ = grid->unit[2]*(float)(z1+0.5)+ztrans;
                this_slice[saddr(x1, y1, 9)] =
                    crossings[9] = surface->nverts;
            }
            else
                this_slice[saddr(x1, y1, 9)] =
                    crossings[9] = this_slice[saddr(x1-1, y1, 10)];
            break;

        case 10:
            if (y1==0)
            {
                lookedup = false;
                if (index&2)
                    thisaddr = addr(x2, y1, z1);
                else
                    thisaddr = addr(x2, y1, z2);
                *vrtx++ = grid->unit[0]*(float)x2+xtrans;
                *vrtx++ = grid->unit[1]*(float)y1+ytrans;
                *vrtx++ = grid->unit[2]*(float)(z1+0.5)+ztrans;
                this_slice[saddr(x1, y1, 10)] =
                    crossings[10] = surface->nverts;
            }
            else
                this_slice[saddr(x1, y1, 10)] =
                    crossings[10] = this_slice[saddr(x1, y1-1, 12)];
            break;

        case 11:
            if (x1==0)
            {
                lookedup = false;
                if (index&8)
                    thisaddr = addr(x1, y2, z1);
                else
                    thisaddr = addr(x1, y2, z2);
                *vrtx++ = grid->unit[0]*(float)x1+xtrans;
                *vrtx++ = grid->unit[1]*(float)y2+ytrans;
                *vrtx++ = grid->unit[2]*(float)(z1+0.5)+ztrans;
                this_slice[saddr(x1, y1, 11)] =
                    crossings[11] = surface->nverts;
            }
            else
                this_slice[saddr(x1, y1, 11)] =
                    crossings[11] = this_slice[saddr(x1-1, y1, 12)];
            break;

        case 12:
            lookedup = false;
            if (index&4)
                thisaddr = addr(x2, y2, z1);
            else
                thisaddr = addr(x2, y2, z2);
            *vrtx++ = grid->unit[0]*(float)x2+xtrans;
            *vrtx++ = grid->unit[1]*(float)y2+ytrans;
            *vrtx++ = grid->unit[2]*(float)(z1+0.5)+ztrans;
            this_slice[saddr(x1, y1, 12)] =
                crossings[12] = surface->nverts;
            break;

        } /* end switch */

        if (!lookedup)
        {
            owner = surface->owner[surface->nverts] = grid->owner[thisaddr];
            surface->nverts++;
            do_smooth(atoms->GetRadius(owner)+PRAD,
                      atoms->GetCoord(owner), vrtx_sav);
        }
    } /* end for */
}

static void molecular_calc_cell_verts(unsigned int index, unsigned int x1, unsigned int y1, unsigned int z1)
{
    register unsigned int i;
    register unsigned int x2, y2, z2;
    unsigned int nedges, thisowner, thatowner;
    unsigned int crnt_edge, lookedup;
    register unsigned int thisaddr = 0, thataddr = 0;
    float *vrtx, *vrtx_sav;

    x2 = x1+1;
    y2 = y1+1;
    z2 = z1+1;

    nedges = cell_table[index].nedges;
    for (i = 0; i<nedges; i++)
    {
        crnt_edge = cell_table[index].edges[i];
        vrtx_sav = vrtx = &surface->vertices[surface->nverts*3];
        lookedup = true;
        switch (crnt_edge)
        {
        case 1:
            if (z1==0)
            {
                lookedup = false;
                if (index&1)
                {
                    thisaddr = addr(x1, y1, z1);
                    thataddr = addr(x2, y1, z1);
                }
                else
                {
                    thisaddr = addr(x2, y1, z1);
                    thataddr = addr(x1, y1, z1);
                }
                *vrtx++ = grid->unit[0]*(float)(x1+0.5)+xtrans;
                *vrtx++ = grid->unit[1]*(float)y1+ytrans;
                *vrtx++ = grid->unit[2]*(float)z1+ztrans;
                this_slice[saddr(x1, y1, 1)] =
                    crossings[1] = surface->nverts;
            }
            else
                this_slice[saddr(x1, y1, 1)] =
                    crossings[1] = last_slice[saddr(x1, y1, 5)];
            break;

        case 2:
            if (z1==0)
            {
                lookedup = false;
                if (index&2)
                {
                    thisaddr = addr(x2, y1, z1);
                    thataddr = addr(x2, y2, z1);
                }
                else
                {
                    thisaddr = addr(x2, y2, z1);
                    thataddr = addr(x2, y1, z1);
                }
                *vrtx++ = grid->unit[0]*(float)x2+xtrans;
                *vrtx++ = grid->unit[1]*(float)(y1+0.5)+ytrans;
                *vrtx++ = grid->unit[2]*(float)z1+ztrans;
                this_slice[saddr(x1, y1, 2)] =
                    crossings[2] = surface->nverts;
            }
            else
                this_slice[saddr(x1, y1, 2)] =
                    crossings[2] = last_slice[saddr(x1, y1, 6)];
            break;

        case 3:
            if (z1==0)
            {
                lookedup = false;
                if (index&4)
                {
                    thisaddr = addr(x2, y2, z1);
                    thataddr = addr(x1, y2, z1);
                }
                else
                {
                    thisaddr = addr(x1, y2, z1);
                    thataddr = addr(x2, y2, z1);
                }
                *vrtx++ = grid->unit[0]*(float)(x1+0.5)+xtrans;
                *vrtx++ = grid->unit[1]*(float)y2+ytrans;
                *vrtx++ = grid->unit[2]*(float)z1+ztrans;
                this_slice[saddr(x1, y1, 3)] =
                    crossings[3] = surface->nverts;
            }
            else
                this_slice[saddr(x1, y1, 3)] =
                    crossings[3] = last_slice[saddr(x1, y1, 7)];
            break;

        case 4:
            if (z1==0)
            {
                lookedup = false;
                if (index&8)
                {
                    thisaddr = addr(x1, y2, z1);
                    thataddr = addr(x1, y1, z1);
                }
                else
                {
                    thisaddr = addr(x1, y1, z1);
                    thataddr = addr(x1, y2, z1);
                }
                *vrtx++ = grid->unit[0]*(float)x1+xtrans;
                *vrtx++ = grid->unit[1]*(float)(y1+0.5)+ytrans;
                *vrtx++ = grid->unit[2]*(float)z1+ztrans;
                this_slice[saddr(x1, y1, 4)] =
                    crossings[4] = surface->nverts;
            }
            else
                this_slice[saddr(x1, y1, 4)] =
                    crossings[4] = last_slice[saddr(x1, y1, 8)];
            break;

        case 5:
            if (y1==0)
            {
                lookedup = false;
                if (index&16)
                {
                    thisaddr = addr(x1, y1, z2);
                    thataddr = addr(x2, y1, z2);
                }
                else
                {
                    thisaddr = addr(x2, y1, z2);
                    thataddr = addr(x1, y1, z2);
                }
                *vrtx++ = grid->unit[0]*(float)(x1+0.5)+xtrans;
                *vrtx++ = grid->unit[1]*(float)y1+ytrans;
                *vrtx++ = grid->unit[2]*(float)z2+ztrans;
                this_slice[saddr(x1, y1, 5)] =
                    crossings[5] = surface->nverts;
            }
            else
                this_slice[saddr(x1, y1, 5)] =
                    crossings[5] = this_slice[saddr(x1, y1-1, 7)];
            break;

        case 6:
            lookedup = false;
            if (index&32)
            {
                thisaddr = addr(x2, y1, z2);
                thataddr = addr(x2, y2, z2);
            }
            else
            {
                thisaddr = addr(x2, y2, z2);
                thataddr = addr(x2, y1, z2);
            }
            *vrtx++ = grid->unit[0]*(float)x2+xtrans;
            *vrtx++ = grid->unit[1]*(float)(y1+0.5)+ytrans;
            *vrtx++ = grid->unit[2]*(float)z2+ztrans;
            this_slice[saddr(x1, y1, 6)] =
                crossings[6] = surface->nverts;
            break;

        case 7:
            lookedup = false;
            if (index&64)
            {
                thisaddr = addr(x2, y2, z2);
                thataddr = addr(x1, y2, z2);
            }
            else
            {
                thisaddr = addr(x1, y2, z2);
                thataddr = addr(x2, y2, z2);
            }
            *vrtx++ = grid->unit[0]*(float)(x1+0.5)+xtrans;
            *vrtx++ = grid->unit[1]*(float)y2+ytrans;
            *vrtx++ = grid->unit[2]*(float)z2+ztrans;
            this_slice[saddr(x1, y1, 7)] =
                crossings[7] = surface->nverts;
            break;

        case 8:
            if (x1==0)
            {
                lookedup = false;
                if (index&128)
                {
                    thisaddr = addr(x1, y2, z2);
                    thataddr = addr(x1, y1, z2);
                }
                else
                {
                    thisaddr = addr(x1, y1, z2);
                    thataddr = addr(x1, y2, z2);
                }
                *vrtx++ = grid->unit[0]*(float)x1+xtrans;
                *vrtx++ = grid->unit[1]*(float)(y1+0.5)+ytrans;
                *vrtx++ = grid->unit[2]*(float)z2+ztrans;
                this_slice[saddr(x1, y1, 8)] =
                    crossings[8] = surface->nverts;
            }
            else
                this_slice[saddr(x1, y1, 8)] =
                    crossings[8] = this_slice[saddr(x1-1, y1, 6)];
            break;

        case 9:
            if (x1==0)
            {
                lookedup = false;
                if (index&1)
                {
                    thisaddr = addr(x1, y1, z1);
                    thataddr = addr(x1, y1, z2);
                }
                else
                {
                    thisaddr = addr(x1, y1, z2);
                    thataddr = addr(x1, y1, z1);
                }
                *vrtx++ = grid->unit[0]*(float)x1+xtrans;
                *vrtx++ = grid->unit[1]*(float)y1+ytrans;
                *vrtx++ = grid->unit[2]*(float)(z1+0.5)+ztrans;
                this_slice[saddr(x1, y1, 9)] =
                    crossings[9] = surface->nverts;
            }
            else
                this_slice[saddr(x1, y1, 9)] =
                    crossings[9] = this_slice[saddr(x1-1, y1, 10)];
            break;

        case 10:
            if (y1==0)
            {
                lookedup = false;
                if (index&2)
                {
                    thisaddr = addr(x2, y1, z1);
                    thataddr = addr(x2, y1, z2);
                }
                else
                {
                    thisaddr = addr(x2, y1, z2);
                    thataddr = addr(x2, y1, z1);
                }
                *vrtx++ = grid->unit[0]*(float)x2+xtrans;
                *vrtx++ = grid->unit[1]*(float)y1+ytrans;
                *vrtx++ = grid->unit[2]*(float)(z1+0.5)+ztrans;
                this_slice[saddr(x1, y1, 10)] =
                    crossings[10] = surface->nverts;
            }
            else
                this_slice[saddr(x1, y1, 10)] =
                    crossings[10] = this_slice[saddr(x1, y1-1, 12)];
            break;

        case 11:
            if (x1==0)
            {
                lookedup = false;
                if (index&8)
                {
                    thisaddr = addr(x1, y2, z1);
                    thataddr = addr(x1, y2, z2);
                }
                else
                {
                    thisaddr = addr(x1, y2, z2);
                    thataddr = addr(x1, y2, z1);
                }
                *vrtx++ = grid->unit[0]*(float)x1+xtrans;
                *vrtx++ = grid->unit[1]*(float)y2+ytrans;
                *vrtx++ = grid->unit[2]*(float)(z1+0.5)+ztrans;
                this_slice[saddr(x1, y1, 11)] =
                    crossings[11] = surface->nverts;
            }
            else
                this_slice[saddr(x1, y1, 11)] =
                    crossings[11] = this_slice[saddr(x1-1, y1, 12)];
            break;

        case 12:
            lookedup = false;
            if (index&4)
            {
                thisaddr = addr(x2, y2, z1);
                thataddr = addr(x2, y2, z2);
            }
            else
            {
                thisaddr = addr(x2, y2, z2);
                thataddr = addr(x2, y2, z1);
            }
            *vrtx++ = grid->unit[0]*(float)x2+xtrans;
            *vrtx++ = grid->unit[1]*(float)y2+ytrans;
            *vrtx++ = grid->unit[2]*(float)(z1+0.5)+ztrans;
            this_slice[saddr(x1, y1, 12)] =
                crossings[12] = surface->nverts;
            break;
        } /* end switch */

        if (!lookedup)
        {
            thisowner = surface->owner[surface->nverts] =
                            grid->owner[thisaddr];
            thatowner = grid->owner[thataddr];
            surface->nverts++;

            if (grid->type[thisaddr]==CONTACT_SURFACE)
                do_smooth(atoms->GetRadius(thisowner),
                          atoms->GetCoord(thisowner), vrtx_sav);
            else
                do_smooth(PRAD, &scratch_surface->vertices[thatowner*3], vrtx_sav);
        }
    } /* end for */
}

#undef linterp
