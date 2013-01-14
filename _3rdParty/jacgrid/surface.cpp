#include "jacgrid.h"
#include "jacgrid_private.h"
#include "linalg.h"
#include <cmath>
#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

#include <vector>
#include <map>
#include <set>


static const float UNMARKED_POINT = -9999.0;
static const unsigned int REENTRANT_SURFACE = 1;
static const unsigned int CONTACT_SURFACE = 2;

static unsigned int SURF_GRID_DIMENSION = 65;
static float SURF_PROBE_RADIUS = 1.4f;


void surfaceT::Clear()
{
    vertices.clear();
    normals.clear();
    triangles.clear();
    owner.clear();
    nverts = 0;
    nconn = 0;
    maxverts = 0;
    maxconn = 0;
    gridUnit[0] = gridUnit[1] = gridUnit[2] = 0.0f;
    needs_resort = 0;
}

surfaceT::surfaceT()
{
    Clear();
}

surfaceT::surfaceT(unsigned int verts, unsigned int polys)
{
    Clear();
    Resize(verts, polys);
}

bool surfaceT::Resize(unsigned int verts, unsigned int polys)
{
    maxverts = verts;
    maxconn = polys;

    owner.resize(verts);
    triangles.resize(polys);

    normals.resize(verts*3);
    vertices.resize(verts*3);

    if (nverts>verts)
        nverts = verts;
    if (nconn>polys)
        nconn = polys;

    return true;
}

void surfaceT::BuildNeighborList(std::vector< std::set<unsigned int> > &nbrList) const
{
    nbrList.clear();
    nbrList.resize(nverts);

    for (unsigned int i = 0; i<nconn; i += 3)
    {
        unsigned int t1 = triangles[i];
        unsigned int t2 = triangles[i+1];
        unsigned int t3 = triangles[i+2];

        nbrList[t1].insert(t2);
        nbrList[t1].insert(t3);

        nbrList[t2].insert(t1);
        nbrList[t2].insert(t3);

        nbrList[t3].insert(t1);
        nbrList[t3].insert(t2);
    }
}

void surfaceT::BuildVertexToTriangleList(std::vector< std::set<unsigned int> > &v2tList) const
{
    v2tList.clear();
    v2tList.resize(nverts);

    for (unsigned int i = 0; i<nconn; i += 3)
    {
        v2tList[triangles[i+0]].insert(i/3);
        v2tList[triangles[i+1]].insert(i/3);
        v2tList[triangles[i+2]].insert(i/3);
    }
}

#define DELETE_FLAG -99999.0000 /* flag that indicates which vertices are */
                                /* to be deleted in reduce */

static void ReduceToLine(surfaceT &surf,
                         std::vector<std::set<unsigned int> > &v2t,
                         float *a, float *b, unsigned int an, unsigned int bn, unsigned int /* cn */)
/* replace a/b with single vertex at midpoint and update all references */
{
    if (an!=bn)
    {
        a[0] = (a[0]+b[0])/2.0f;
        a[1] = (a[1]+b[1])/2.0f;
        a[2] = (a[2]+b[2])/2.0f;
        b[0] = b[1] = b[2] = DELETE_FLAG;

        // for each triangle which mentions atom b, update that reference to atom a
        for (std::set<unsigned int>::iterator i = v2t[bn].begin();
             i!=v2t[bn].end(); ++i)
        {
            unsigned int *tri = &surf.triangles[*i*3];
            for (unsigned int j = 0; j<3; ++j)
                if (tri[j]==bn)
                    tri[j] = an;
            v2t[an].insert(*i);
        }
        //v2t[bn].clear();
    }
}

static void CutTriangle(surfaceT &surf,
                        std::vector<std::set<unsigned int> > &v2t,
                        float *a, float *b, float *c,
                        unsigned int an, unsigned int bn, unsigned int cn)
/* replace a, b & c ith single vertex at centroid and update all references */
{
    if ((an!=bn)&&(an!=cn))
    {
        a[0] = (a[0]+b[0]+c[0])/3.0f;
        a[1] = (a[1]+b[1]+c[1])/3.0f;
        a[2] = (a[2]+b[2]+c[2])/3.0f;
        b[0] = b[1] = b[2] = c[0] = c[1] = c[2] = DELETE_FLAG;
    }

    if (an!=bn)
    {
        for (std::set<unsigned int>::iterator i = v2t[bn].begin();
             i!=v2t[bn].end(); ++i)
        {
            unsigned int *tri = &surf.triangles[*i*3];
            for (unsigned int j = 0; j<3; ++j)
                if (tri[j]==bn)
                    tri[j] = an;
            v2t[an].insert(*i);
        }
        //v2t[bn].clear();
    }

    if (an!=cn)
    {
        for (std::set<unsigned int>::iterator i = v2t[cn].begin();
             i!=v2t[cn].end(); ++i)
        {
            unsigned int *tri = &surf.triangles[*i*3];
            for (unsigned int j = 0; j<3; ++j)
                if (tri[j]==cn)
                    tri[j] = an;
            v2t[an].insert(*i);
        }
        //v2t[cn].clear();
    }
}

void surfaceT::Reduce(float mindist, unsigned int *pruned)
/*-

   this routine reduces the number of vertices in a surface by eliminating
   triangles that are too small, and replacing skinny triangles by a single
   line segment.  This of course has the effect of increasing the rendering
   speed, as well as improving the appearance of surfaces.

   If triangle ABC is to be eliminated, vertex A is replaced by the
   centroid of ABC and all references to B and C changed to point to A.

   Triangle ABC is to be reduced to a line because AB is too short, A is
   replaced by the midpoint of AB and then all references to B are changed
   to point to A instead.

   surface:  the surface to be reduced
   mindist:  what's the smallest side to allow?

   pruned, an optional unsigned int array which should be at least
   nverts elements, which, on output, inidicates if each of the
   original vertices was deleted.  This can be used to keep update external
   property arrays after the surface has been pruned.

   -*/
{
    register unsigned int i;
    float *v0, *v1, *v2;
    float d0, d1, d2;
    unsigned int vn0, vn1, vn2, key;
    unsigned int *in_list, *out_list;

    if (!nverts)
        return;

    std::vector<std::set<unsigned int> > v2t;
    BuildVertexToTriangleList(v2t);

    /* loop through again, checking for small sides */
    mindist = mindist*mindist;
    for (i = 0; i<nconn; i += 3)
    {
        vn0 = triangles[i];
        vn1 = triangles[i+1];
        vn2 = triangles[i+2];
        v0 = &vertices[(vn0)*3+0];
        v1 = &vertices[(vn1)*3+0];
        v2 = &vertices[(vn2)*3+0];
        key = 0;
        if ((d0 = distsq(v0, v1))<mindist)
            key += 1;
        if ((d1 = distsq(v1, v2))<mindist)
            key += 2;
        if ((d2 = distsq(v2, v0))<mindist)
            key += 4;

        switch (key)
        {
        case 0:
            /* no action */
            break;
        case 1:
            ReduceToLine(*this, v2t, v0, v1, vn0, vn1, vn2);
            break;
        case 2:
            ReduceToLine(*this, v2t, v1, v2, vn1, vn2, vn0);
            break;
        case 3:
            if (d0<d1)
                ReduceToLine(*this, v2t, v0, v1, vn0, vn1, vn2);
            else
                ReduceToLine(*this, v2t, v1, v2, vn1, vn2, vn0);
            break;
        case 4:
            ReduceToLine(*this, v2t, v2, v0, vn2, vn0, vn1);
            break;
        case 5:
            if (d0<d2)
                ReduceToLine(*this, v2t, v0, v1, vn0, vn1, vn2);
            else
                ReduceToLine(*this, v2t, v2, v0, vn2, vn0, vn1);
            break;
        case 6:
            if (d1<d2)
                ReduceToLine(*this, v2t, v1, v2, vn1, vn2, vn0);
            else
                ReduceToLine(*this, v2t, v2, v0, vn2, vn0, vn1);
            break;
        case 7:
            CutTriangle(*this, v2t, v0, v1, v2, vn0, vn1, vn2);
            break;
        }

        /* set flag to delete polygon */
        if (key)
        {
            triangles[i] =
                triangles[i+1] =
                    triangles[i+2] = UINT_MAX;
        }
    }

    /* final pass: delete excess triangles from polygon list */
    int deleted = 0;
    for (in_list = out_list = &triangles[0], i = 0;
         i<nconn; i++, in_list++)
    {
        if (*in_list!=UINT_MAX)
        {
            *out_list = *in_list;
            out_list++;
        }
        else
            deleted++;
    }
    nconn -= deleted;

    /* clean up holes in vertex list, reclaim memory */
    std::vector<int> offset;
    offset.resize(nverts);
    unsigned int *new_owner = &owner[0];
    unsigned int *old_owner = &owner[0];
    float *cold = &vertices[0];
    float *cnew = &vertices[0];
    deleted = 0;
    for (i = 0; i<nverts; i++)
    {
        if (*cold==DELETE_FLAG)
        {
            if (pruned)
                pruned[i] = true;
            deleted++;
            cold += 3;
            old_owner++;
        }
        else
        {
            if (pruned)
                pruned[i] = false;
            *new_owner++ = *old_owner++;
            *cnew++ = *cold++;
            *cnew++ = *cold++;
            *cnew++ = *cold++;
        }
        offset[i] = -deleted;
    }
    nverts -= deleted;

    /* remap triangle list */
    unsigned int *inew;
    for (i = 0, inew = &triangles[0]; i<nconn; i++, inew++)
    {
        *inew += offset[*inew];
    }
    Resize(nverts, nconn);
}





unsigned int JACGetGridDimension()
{
    return SURF_GRID_DIMENSION;
}

void JACSetGridDimension(unsigned int d)
{
    SURF_GRID_DIMENSION = d;
}

float JACGetProbeRadius()
{
    return SURF_PROBE_RADIUS;
}
void JACSetProbeRadius(float r)
{
    SURF_PROBE_RADIUS = r;
}

inline float MIN(float a, float b)
{
    if (a<b)
        return (a);
    else
        return (b);
}
inline float MAX(float a, float b)
{
    if (a>b)
        return (a);
    else
        return (b);
}

void JACSetGridParams(gridT &grid, unsigned int itype,
                      bool uniform, const JACAtomsBase &atoms)

/*- set up the gridding parameters for the atoms -*/
{
    bool gotatom;
    unsigned icnt, i, j;
    float comin[3] = {0.0, 0.0, 0.0};
    float comax[3] = {0.0, 0.0, 0.0};
    float colen[3] = {0.0, 0.0, 0.0}, radmax = 0, slop;
    float min_g_size;

    gotatom = false;
    icnt = 0;

    switch (itype)
    {
    case JACSurfaceTypes::SURF_MOLECULAR:
    case JACSurfaceTypes::SURF_ACCESSIBLE:
        slop = 2.0f*JACGetProbeRadius();
        break;
    case JACSurfaceTypes::SURF_CONTOUR: /* FALLSTHRU */
    default:
        slop = 0.0f;
        break;
    }

    for (unsigned int iatom = 0; iatom<atoms.GetCount(); iatom++)
    {
        if (atoms.IsSelected(iatom))
        {
            const float *crd = atoms.GetCoord(iatom);
            float rad = atoms.GetRadius(iatom);
            icnt++;
            if (!gotatom)
            {
                /* grab from the first selected atom as default */
                radmax = rad;
                for (j = 0; j<3; j++)
                {
                    comin[j] = crd[j];
                    comax[j] = crd[j];
                }
                gotatom = true;
            }
            for (j = 0; j<3; j++)
            {
                if (crd[j] < comin[j]) comin[j] = crd[j];
                if (crd[j] > comax[j]) comax[j] = crd[j];
            }
            if (rad > radmax)
                radmax = rad;
        } /* selected */
    } /* all atoms */

    //printf("Gridding %d of %d atoms:\n",icnt,atoms.GetCount());

    for (i = 0; i<3; i++)
    {
        grid.center[i] = (comax[i]-comin[i])/2.0f+comin[i];
        comax[i] += (slop+radmax);
        comin[i] -= (slop+radmax);
        colen[i] = fabsf(comax[i]-comin[i]);
    }

    if (uniform)
    {
        //printf("Making grid uniform\n");
        grid.size[0] = colen[0];
        if (grid.size[0]<colen[1])
            grid.size[0] = colen[1];
        if (grid.size[0]<colen[2])
            grid.size[0] = colen[2];
        min_g_size = (grid.npts[0]-1)/3.0f;
        if (grid.size[0]<min_g_size)
            grid.size[0] = min_g_size;
        for (i = 0; i<3; i++)
            grid.size[i] = grid.size[0];
    }

    for (i = 0; i<3; i++)
    {
        if (!uniform)
        {
            min_g_size = (grid.npts[i]-1)/3.0f;
            grid.size[i] = colen[i];
            if (grid.size[i]<min_g_size)
                grid.size[i] = min_g_size;
        }
        grid.org[i] = grid.center[i]-grid.size[i]/2.0f;
        grid.unit[i] = grid.size[i]/(float)(grid.npts[i]-1);
    }

    //printf("Grid unit sizes: %.3f %.3f %.3f\n"
    //"      Dimension: %d %d %d\n"
    //"         Length: %.3f %.3f %.3f\n"
    //"         Center: %f %f %f\n"
    //"         Origin: %f %f %f\n",
    //grid.unit[0],grid.unit[1],grid.unit[2],
    //grid.npts[0],grid.npts[1],grid.npts[2],
    //grid.size[0],grid.size[1],grid.size[2],
    //grid.center[0],grid.center[1],grid.center[2],
    //grid.org[0],grid.org[1],grid.org[2]);
}

// flag atoms on grid as contact or reentrant grid points
#define addr(i, j, k) (((k)*DIVISIONS2) + ((j)*DIVISIONS) + (i))

static int JACFlagGrid(gridT &grid, const JACAtomsBase &atoms,
                       unsigned int buried_value = CONTACT_SURFACE)

{
    int i, j, k, thisaddr;
    float gpt[3];
    const float *vrtx;
    unsigned int iatom;
    int imax[3], imin[3];
    float exprad, expradsq, fmax[3], fmin[3], dsq, rad, radsq;
    unsigned int DIVISIONS, DIVISIONS2;

    float prad = JACGetProbeRadius();

    DIVISIONS = grid.npts[0];
    DIVISIONS2 = grid.npts[0]*grid.npts[1];

    for (iatom = 0; iatom<atoms.GetCount(); iatom++)
    {
        vrtx = atoms.GetCoord(iatom);
        rad = atoms.GetRadius(iatom);

        /* skip atoms with zero radius and unselected atoms */
        if (rad!=0.0f && atoms.IsSelected(iatom))
        {
            radsq = rad*rad;
            exprad = rad+prad; /* VdW + probe radius */
            expradsq = exprad*exprad;

            /* find the appropriate indices (imax,imin) into the grid for this
               atom and the corresponding coordinates (fmax,fmin) */
            for (i = 0; i<3; i++)
            {
                fmax[i] = vrtx[i]+exprad;
                fmin[i] = vrtx[i]-exprad;
                imin[i] = (int)floor((fmin[i]-grid.org[i])/grid.unit[i]); /* round down */
                imax[i] = (int)MIN(floor((fmax[i]-grid.org[i])/grid.unit[i])+2,
                                   (float)(grid.npts[i]-1));
            }
            /* at this point imax and imin range from 0 to grid.npts-1 */

            /* test each grid point around the atom */
            for (k = imin[2], gpt[2] = grid.org[2]+k*grid.unit[2];
                 k<imax[2];
                 k++, gpt[2] += grid.unit[2] )
            {
                for (j = imin[1], gpt[1] = grid.org[1]+j*grid.unit[1];
                     j<imax[1];
                     j++, gpt[1] += grid.unit[1] )
                {
                    for (i = imin[0], gpt[0] = grid.org[0]+i*grid.unit[0];
                         i<imax[0];
                         i++, gpt[0] += grid.unit[0] )
                    {
                        dsq = distsq(gpt, vrtx);
                        if (dsq<=expradsq)
                        {
                            /* the point is buried by expaned atom */
                            thisaddr = addr(i, j, k);
                            if ((grid.owner[thisaddr]==UINT_MAX)
                                || (dsq<=grid.data[thisaddr]))
                            {
                                /* not buried before    OR the new atom is closer */
                                grid.data[thisaddr] = dsq;
                                grid.owner[thisaddr] = iatom;
                                /* set type */
                                if (dsq<=radsq)
                                    grid.type[thisaddr] = buried_value;
                                else
                                    grid.type[thisaddr] = REENTRANT_SURFACE;
                            }
                        }
                    }
                }
            }
        }
    }
    //printf("Done flagging buried atoms.\n");
    return 0;
}

typedef struct
{
    int delta[3];
    float *data;
} gmaskT;
static gmaskT gmask;

static int build_grid_mask(gridT &grid)
{
    float f[3];
    int i, j, k, thisaddr;
    float gpt[3], testrad;
    int dim;

    dim = 1;
    float prad = JACGetProbeRadius();
    for (i = 0; i<3; i++)
    {
        f[i] = prad/grid.unit[i];
        gmask.delta[i] = (int)(f[i] + 0.5) + 1;
        dim *= ((gmask.delta[i]*2) + 1);
    }
    if (!(gmask.data = (float*)malloc(dim*sizeof(float))))
        return (0);

    testrad = prad*prad;
    thisaddr = 0;
    for (k = -gmask.delta[2]; k<=gmask.delta[2]; k++)
    {
        gpt[2] = (float)k*grid.unit[2];
        for (j = -gmask.delta[1]; j<=gmask.delta[1]; j++)
        {
            gpt[1] = (float)j*grid.unit[1];
            for (i = -gmask.delta[0]; i<=gmask.delta[0]; i++)
            {
                gpt[0] = (float)i*grid.unit[0];
                if ((gpt[0]*gpt[0] + gpt[1]*gpt[1] + gpt[2]*gpt[2])>testrad)
                    gmask.data[thisaddr] = 1.0;
                else
                    gmask.data[thisaddr] = 0.0;
                thisaddr++;
            }
        }
    }
    return 1;
}


static inline int apply_gmask(gridT &grid, int owner,
                              int i, int j, int k,
                              float *vrtx)
{
    register int x, y, z, i1, j1, k1;
    register float *maskptr;
    register int thisaddr;
    int DIVISIONS, DIVISIONS2, dim[3];
    float d, *data;

    DIVISIONS = grid.npts[0];
    DIVISIONS2 = grid.npts[0]*grid.npts[1];

    dim[0] = 2*gmask.delta[0]+1;
    dim[1] = 2*gmask.delta[1]+1;
    dim[2] = 2*gmask.delta[2]+1;
    i -= gmask.delta[0];
    j -= gmask.delta[1];
    k -= gmask.delta[2];
    maskptr = gmask.data;

    float px = grid.org[0]+i*grid.unit[0];
    float py = grid.org[1]+j*grid.unit[1];
    float pz = grid.org[2]+k*grid.unit[2];

    float gu0 = grid.unit[0];
    float gu1 = grid.unit[1];
    float gu2 = grid.unit[2];
    float dz, dy;
    unsigned int addry, addrz;

    for (z = 0, k1 = k; z<dim[2]; z++, k1++, pz += gu2)
    {
        addrz = k1*DIVISIONS2;
        dz = (vrtx[2]-pz)*(vrtx[2]-pz);
        for (y = 0, j1 = j; y<dim[1]; y++, j1++, py += gu1)
        {
            addry = addrz+j1*DIVISIONS;
            dy = dz+(vrtx[1]-py)*(vrtx[1]-py);
            for (x = 0, i1 = i; x<dim[0]; x++, i1++, px += gu0)
            {
                thisaddr = addry+i1;
                if (*maskptr==0.0)
                {
                    if ((i1>=0) && (i1<DIVISIONS)
                        && (j1>=0) && (j1<DIVISIONS)
                        && (k1>=0) && (k1<DIVISIONS))
                    {
                        d = dy+((vrtx[0]-px)*(vrtx[0]-px));
                        data = &grid.data[thisaddr];
                        if (*data==UNMARKED_POINT || *data>0.0)
                        {
                            grid.owner[thisaddr] = owner;
                            *data = -d;
                        }
                        else if (*data<0.0 && d<-*data)
                        {
                            grid.owner[thisaddr] = owner;
                            *data = -d;
                        }
                    }
                }
                maskptr++;
            }
        }
    }
    return (1);
}

//FIXME: this could stand to be much faster, since it's a loop over each
//       vertex in the surface

//NOTE: as long as the grid spacing is less than the probe radius, you
//don't have to do *every* vertex, one vertex per triangle should be
//sufficient.  Could also play games with the scratch isosurface generation
//to only look at every other gridpoint, perhaps, or copy the scratch grid
//to one twice as coarse before creating the scratch surface.

// at the end of the day, however, anything done to apply_gmask to speed
// it up would be a win

static int JACUnFlagGrid(gridT &grid, surfaceT &surface)
{
    register int i, j, k;
    unsigned int l;
    float *vrtx;

    if (!build_grid_mask(grid))
        return 0;

    for (l = 0; l<surface.nverts; l++)
    {
        vrtx = &surface.vertices[l*3];
        i = (int)floor((vrtx[0]-grid.org[0])/grid.unit[0]+0.5);
        j = (int)floor((vrtx[1]-grid.org[1])/grid.unit[1]+0.5);
        k = (int)floor((vrtx[2]-grid.org[2])/grid.unit[2]+0.5);
        apply_gmask(grid, l, i, j, k, vrtx);
    }

    free(gmask.data);
    return 1;
}


bool JACMakeAccessibleSurface(surfaceT &surf, const JACAtomsBase &atoms)
{

    unsigned int dim = JACGetGridDimension();

    gridT grid(UNMARKED_POINT, dim, dim, dim, true);
    if (!grid.IsValid())
        return false;

    JACSetGridParams(grid, JACSurfaceTypes::SURF_ACCESSIBLE, true, atoms);
    JACFlagGrid(grid, atoms);
    if (!jacMakeSurface(surf, JACSurfaceTypes::SURF_ACCESSIBLE, grid, 0.0, atoms))
        return false;

    //FIXME: smooth_surface, reduce_surface, triangle_normalize

    return true;
}


bool JACMakeMolecularSurface(surfaceT &surf, const JACAtomsBase &atoms)
{
    unsigned int dim = JACGetGridDimension();

    gridT grid(UNMARKED_POINT, dim, dim, dim, true);
    if (!grid.IsValid())
        return false;

    JACSetGridParams(grid, JACSurfaceTypes::SURF_ACCESSIBLE, true, atoms);
    JACFlagGrid(grid, atoms);

    surfaceT scratch_surface;
    scratch_surface.Resize(10000, 10000);
    jacMakeSurface(scratch_surface, JACSurfaceTypes::SURF_ACCESSIBLE, grid, 0.0, atoms);

    JACUnFlagGrid(grid, scratch_surface);
    jacMakeSurface(surf, JACSurfaceTypes::SURF_MOLECULAR,
                   grid, 0.0, atoms, &scratch_surface);

    //FIXME: smooth_surface, reduce_surface, triangle_normalize
    return true;
}

static void calc_normal(float *p1, float *p2, float *p3, float *n)
/* This subroutine calculates a normal from three vertices */
{

    float u[3], v[3];
    float sum, mag;


    u[0] = p3[0] - p2[0];
    u[1] = p3[1] - p2[1];
    u[2] = p3[2] - p2[2];

    v[0] = p1[0] - p2[0];
    v[1] = p1[1] - p2[1];
    v[2] = p1[2] - p2[2];

    n[0] = u[1]*v[2] - u[2]*v[1];
    n[1] = u[2]*v[0] - u[0]*v[2];
    n[2] = u[0]*v[1] - u[1]*v[0];

    sum = n[0]*n[0] + n[1]*n[1] + n[2]*n[2];
    mag = (float)sqrt((double)sum);

    if (mag == 0.0)
        return;

    n[0] = n[0] / mag;
    n[1] = n[1] / mag;
    n[2] = n[2] / mag;

}

void JACSurfaceNormalize(surfaceT &surface)
{
    register unsigned int i;
    float *p1, *p2, *p3;
    float n[3];

    /* clear the normals */
    for (i = 0, p1 = &surface.normals[0]; i<surface.nverts; i++)
    {
        *p1++ = 0.0;
        *p1++ = 0.0;
        *p1++ = 0.0;
    }

    for (i = 0; i<surface.nconn; i += 3)
    {
        /* get normal */
        calc_normal(&surface.vertices[surface.triangles[i]*3],
                    &surface.vertices[surface.triangles[(i+1)]*3],
                    &surface.vertices[surface.triangles[(i+2)]*3], n);

        p1 = &surface.normals[surface.triangles[i]*3];
        p2 = &surface.normals[surface.triangles[(i+1)]*3];
        p3 = &surface.normals[surface.triangles[(i+2)]*3];

        p1[0] += n[0];
        p1[1] += n[1];
        p1[2] += n[2];

        p2[0] += n[0];
        p2[1] += n[1];
        p2[2] += n[2];

        p3[0] += n[0];
        p3[1] += n[1];
        p3[2] += n[2];

    }

    /* renormalize */
    for (i = 0; i<surface.nverts; i++)
        vnorm(&surface.normals[i*3]);
}

float JACCalculateVolume(unsigned int itype,
                         const JACAtomsBase &atoms)
{
    float pradtmp = 0;

    unsigned int dim = JACGetGridDimension();
    gridT grid(UNMARKED_POINT, dim, dim, dim, true);
    if (!grid.IsValid())
        return -1.0;

    JACSetGridParams(grid, itype, true, atoms);
    switch (itype)
    {
    case JACSurfaceTypes::SURF_ACCESSIBLE:
        JACFlagGrid(grid, atoms);
        break;
    case JACSurfaceTypes::SURF_VDW:
        pradtmp = JACGetProbeRadius();
        JACSetProbeRadius(0.0f);
        JACFlagGrid(grid, atoms);
        JACSetProbeRadius(pradtmp);
        break;
    case JACSurfaceTypes::SURF_MOLECULAR:
        JACFlagGrid(grid, atoms);
        surfaceT scratch_surface;
        scratch_surface.Resize(10000, 10000);
        jacMakeSurface(scratch_surface, JACSurfaceTypes::SURF_ACCESSIBLE, grid, 0.0, atoms);

        JACUnFlagGrid(grid, scratch_surface);
        break;
    }

    /* count grid points above 0.0.  These are buried. */
    unsigned int cnt = 0;
    unsigned int gsize = grid.npts[0]*grid.npts[1]*grid.npts[2];
    const float *data = grid.data;
    for (unsigned int i = 0; i<gsize; i++)
        if (*data++>0.0)
            cnt++;

    float retval = grid.unit[0]*grid.unit[1]*grid.unit[2]*cnt;
    return retval;
}


float JACCalculateAccessibleVolume(const JACAtomsBase &atoms)
{
    return JACCalculateVolume(JACSurfaceTypes::SURF_ACCESSIBLE, atoms);
}

float JACCalculateMolecularVolume(const JACAtomsBase &atoms)
{
    return JACCalculateVolume(JACSurfaceTypes::SURF_MOLECULAR, atoms);
}

float JACCalculateVdWVolume(const JACAtomsBase &atoms)
{
    return JACCalculateVolume(JACSurfaceTypes::SURF_VDW, atoms);
}


float JACSurfaceArea(const surfaceT &surface, unsigned int *select, unsigned int sel_mask)
{
    unsigned int tri_cnt = 0;
    float area = 0.0f;
    for (unsigned int i = 0; i<surface.nconn; i += 3)
    {
        unsigned int v1 = surface.triangles[i];
        unsigned int v2 = surface.triangles[i+1];
        unsigned int v3 = surface.triangles[i+2];
        if (!select
            || (select[v1] & sel_mask
                && select[v2] & sel_mask
                && select[v3] & sel_mask))
        {
            tri_cnt++;
            area += tarea(&surface.vertices[v1*3],
                          &surface.vertices[v2*3],
                          &surface.vertices[v3*3]);
        }
    }
    return area;
}

static inline float tvolume(const float *a, const float *b, const float *c, const float *d)
/*
 * return 6 times the volume of the tetrahedron ABCD
 * The 6 times is an optimization.  Besure to divide your final results
 * from using this routine by 6.
 */
{
    float l[3], m[3], n[3];

    l[0] = a[0]-d[0];
    l[1] = a[1]-d[1];
    l[2] = a[2]-d[2];

    m[0] = b[0]-d[0];
    m[1] = b[1]-d[1];
    m[2] = b[2]-d[2];

    n[0] = c[0]-d[0];
    n[1] = c[1]-d[1];
    n[2] = c[2]-d[2];

    return (+l[0]*(m[1]*n[2]-m[2]*n[1])
            - m[0]*(l[1]*n[2]-l[2]*n[1])
            + n[0]*(l[1]*m[2]-l[2]*m[1]));
}

float JACSurfaceVolume(const surfaceT &surface, unsigned int *select, unsigned int sel_mask)
{
    unsigned int cnt = 0;
    float center[3];
    unsigned int i;

    center[0] = center[1] = center[2] = 0.0;
    for (i = 0; i<surface.nverts; i++)
    {
        if (!select || (select[i] & sel_mask))
        {
            cnt++;
            center[0] += surface.vertices[i*3];
            center[1] += surface.vertices[i*3];
            center[2] += surface.vertices[i*3];
        }
    }
    center[0] /= cnt;
    center[1] /= cnt;
    center[2] /= cnt;

    float volume = 0.0;
    cnt = 0;
    for (i = 0; i<surface.nconn; i += 3)
    {
        unsigned int v1 = surface.triangles[i];
        unsigned int v2 = surface.triangles[i+1];
        unsigned int v3 = surface.triangles[i+2];
        if (!select
            || (select[v1] & sel_mask
                && select[v2] & sel_mask
                && select[v3] & sel_mask))
        {
            cnt++;
            volume += tvolume(&surface.vertices[v1*3],
                              &surface.vertices[v2*3],
                              &surface.vertices[v3*3], center);
        }
    }

    return volume/6.0f;
}

void JACSurfaceSwapFace(surfaceT &surface,
                        unsigned int *select,
                        unsigned int sel_mask)
{
    unsigned int i;
    unsigned int swap;
    unsigned *c1, *c2, *c3;
    float *f;

    if (!surface.nverts)
        return;

    std::vector<bool> invert;
    invert.resize(surface.nverts);

    for (i = 0; i<surface.nconn; i += 3)
    {
        c1 = &surface.triangles[i];
        c2 = &surface.triangles[i+1];
        c3 = &surface.triangles[i+2];
        if (!select
            || (select[*c1] & sel_mask
                && select[*c2] & sel_mask
                && select[*c3] & sel_mask) )
        {
            swap = *c1;
            *c1 = *c2;
            *c2 = swap;
            invert[*c1] = true;
            invert[*c2] = true;
            invert[*c3] = true;
        }
    }

    for (i = 0; i<surface.nverts; i++)
    {
        if (invert[i])
        {
            f = &surface.normals[i*3];
            *f = -(*f);
            f++;
            *f = -(*f);
            f++;
            *f = -(*f);
            f++;
        }
    }
}

void JACInvertTriangles(surfaceT &surface,
                        unsigned int *select,
                        unsigned int sel_mask)
{
    unsigned int i;
    unsigned int swap;
    unsigned *c1, *c2, *c3;

    if (!surface.nverts)
        return;

    for (i = 0; i<surface.nconn; i += 3)
    {
        c1 = &surface.triangles[i];
        c2 = &surface.triangles[i+1];
        c3 = &surface.triangles[i+2];
        if (!select
            || (select[*c1] & sel_mask
                && select[*c2] & sel_mask
                && select[*c3] & sel_mask) )
        {
            swap = *c1;
            *c1 = *c2;
            *c2 = swap;
        }
    }
}


void JACOverlapOrDifferenceGrid(gridT &grid1,
                                const JACAtomsBase &atoms1,
                                const JACAtomsBase &atoms2,
                                bool difference)
{
    unsigned int dim = JACGetGridDimension();
    grid1.Clear();
    grid1.Resize(UNMARKED_POINT, dim, dim, dim, true);
    gridT grid2(UNMARKED_POINT, dim, dim, dim, true);

    if (!grid1.IsValid() || !grid2.IsValid())
        return;

    JACSetGridParams(grid1, JACSurfaceTypes::SURF_CONTOUR, true,
                     JACAtomsMerge(atoms1, atoms2));
    JACSetGridParams(grid2, JACSurfaceTypes::SURF_CONTOUR, true,
                     JACAtomsMerge(atoms1, atoms2));

    float pradtmp = JACGetProbeRadius();
    JACSetProbeRadius(0.0f);
    JACFlagGrid(grid1, atoms1, 1);
    JACFlagGrid(grid2, atoms2, 1);
    JACSetProbeRadius(pradtmp);

    unsigned int dim3 = grid1.GetSize();
    unsigned int score = difference ? 1 : 2;
    for (unsigned int i = 0; i<dim3; ++i)
        grid1.data[i] = (grid1.type[i] + grid2.type[i] == score);
}



void JACOverlapGrid(gridT &grid,
                    const JACAtomsBase &atoms1,
                    const JACAtomsBase &atoms2)
{
    JACOverlapOrDifferenceGrid(grid, atoms1, atoms2, true);
}

void JACDifferenceGrid(gridT &grid,
                       const JACAtomsBase &atoms1,
                       const JACAtomsBase &atoms2)
{
    JACOverlapOrDifferenceGrid(grid, atoms1, atoms2, false);
}

void JACOverlapSurface(surfaceT &surface,
                       const JACAtomsBase &atoms1,
                       const JACAtomsBase &atoms2)
{
    gridT grid;
    JACOverlapGrid(grid, atoms1, atoms2);
    jacMakeSurface(surface, JACSurfaceTypes::SURF_CONTOUR, grid, 0.0f, atoms1);
}

void JACDifferenceSurface(surfaceT &surface,
                          const JACAtomsBase &atoms1,
                          const JACAtomsBase &atoms2)
{
    gridT grid;
    JACDifferenceGrid(grid, atoms1, atoms2);
    jacMakeSurface(surface, JACSurfaceTypes::SURF_CONTOUR, grid, 0.0f, atoms1);
}

bool JACMakeSurface(surfaceT &surf, unsigned int itype,
                    const gridT &grd, float threshold)
{
    JACAtomsXYZR atoms(0, 0);
    return jacMakeSurface(surf, itype, grd, threshold, atoms, 0);
}

void JACSmoothSurface(surfaceT &surf, unsigned int beg, unsigned int end)
{
    std::vector< std::set<unsigned int> > nbrList;
    std::vector<float> newverts;
    newverts.resize(surf.nverts*3);

    if (end==UINT_MAX)
        end = surf.nverts;

    // copy all vertices
    newverts = surf.vertices;

    surf.BuildNeighborList(nbrList);
    for (unsigned int i = beg; i<end; ++i)
    {
        float *nv = &newverts[i*3];
        if (nbrList[i].size())
        {
            nv[0] *= 0.5;
            nv[1] *= 0.5;
            nv[2] *= 0.5;
            float frac = 0.5f/float(nbrList[i].size());
            for (std::set<unsigned int>::iterator j =
                     nbrList[i].begin(); j!=nbrList[i].end(); ++j)
            {
                float *ov = &surf.vertices[*j*3];
                nv[0] += frac*ov[0];
                nv[1] += frac*ov[1];
                nv[2] += frac*ov[2];
            }
        }
    }

    //copy smoothed vertices
    surf.vertices = newverts;
}

class ProbeT
{
public:
    ProbeT(unsigned int level);
    ~ProbeT();

    unsigned int nprobe; /* number of points in probe */
    float *pco;          /* coordinates of unit probe at origin */
    float       *area;         /* surface area of probe point */

private:
    void MakeIcosahedron(float v[12][3]);
    void SubTriangle(float *a, float *b, float *c, unsigned int level);
};

void ProbeT::MakeIcosahedron(float v[12][3])
{
    float a, b;
    unsigned int i, j, k;

    a = 0.8506508f;
    b = 0.5257311f;

    k = 0;
    for (i = 0; i < 2; i++)
    {
        a = -a;
        for (j = 0; j < 2; j++)
        {
            b = -b;

            v[k][0] = 0.0;
            v[k][1] = a;
            v[k++][2] = b;

            v[k][0] = b;
            v[k][1] = 0.0;
            v[k++][2] = a;

            v[k][0] = a;
            v[k][1] = b;
            v[k++][2] = 0.0;
        }
    }
}


void ProbeT::SubTriangle(float *a, float *b, float *c, unsigned int level)
/*-
 * Recursively subdivides triangle ABC levl times, storing the result in
 * the probe array, PROBE->pco.
 *
 * STRATEGY: if not the bottom level, find the midpoints of each of the
 * sides of this sub_triangle and recurse, calling each of the four sub
 * -sub_triangles in turn.  If it's the bottom level, find the center, add
 * that point to the output probe object, and find its area.
   -*/
{
    float d[3], e[3], f[3];

    if (level > 0)
    {
        /* find new midpoints */
        vmidp(d, a, b);
        vmidp(e, b, c);
        vmidp(f, a, c);

        SubTriangle(a, d, f, level - 1);
        SubTriangle(d, b, e, level - 1);
        SubTriangle(d, e, f, level - 1);
        SubTriangle(f, e, c, level - 1);
    }
    else
    {
        /* normalize the vectors */
        vnorm(a);
        vnorm(b);
        vnorm(c);
        /* find center */
        vcent(d, a, b, c);
        vnorm(d);
        vset(&pco[nprobe*3], d);
        /* find area */
        vdif(d, a, b);
        vdif(e, c, b);
        cross(f, e, d);
        area[nprobe] = amag(f) / 2.0f;
        nprobe++;
    }
}


ProbeT::ProbeT(unsigned int level)
/*-
   builds a probe sphere for surface area calculation, by recursively
   sub-dividing an icosahedrom level times.
   -*/
{
    float v[12][3];
    unsigned int i, j, k;
    float scale, total;

    unsigned int size[] =
    {
        12, 80, 320, 1280
    };


    area = (float*)malloc(sizeof(float)*size[level]);
    pco = (float*)malloc(sizeof(float)*size[level]*3);
    if (!area || !pco)
    {
        if (area) free(area);
        if (pco) free(pco);
    }

    MakeIcosahedron(v);
    nprobe = 0;
    for (i = 0; i < 10; i++)
    {
        for (j = i + 1; j < 11; j++)
        {
            if (dist(&v[i][0], &v[j][0]) < 1.1)
            {
                for (k = j + 1; k < 12; k++)
                {
                    if ((dist(&v[i][0], &v[k][0]) < 1.1)
                        && (dist(&v[j][0], &v[k][0]) < 1.1))
                        SubTriangle(&v[i][0], &v[j][0], &v[k][0], level);
                }
            }
        }
    }

    scale = 0.0;
    for (i = 0; i < nprobe; i++)
        scale += area[i];
    scale = float(M_PI*4.0f) / scale;
    total = 0.0;
    for (i = 0; i < nprobe; i++)
    {
        area[i] *= scale;
        total += area[i];
    }
}

ProbeT::~ProbeT()
{
    if (area) free(area);
    if (pco) free(pco);
}



float JACSurfaceArea(const JACAtomsBase &atoms,
                     const JACAtomsBase &context,
                     float *surface_area,
                     float &fractional,
                     VertexSender *vs,
                     unsigned int pres)
/*-
   the main surface area routine.  implements the method of shrake & rupley
   to calculate the surface ares of the selected atoms

   strategy: first grid the atoms.  assign each atom to a grid, so that for
   each grid point there is a list of atoms that are within that grid
   point.  then for each atom, search all atoms in the neighboring grid
   points for contact.  if contact is made, search all probe points on the
   atom.

   Surface is calculated and generated for atoms in the "atoms" set.  No
   surface is generated for atoms in the "context" set, but they do
   occlude volume.  Finally, atoms with radii of zero are completely
   ignored, even if it is in one of the selections.

   -*/
{
    std::map<unsigned int, std::vector<unsigned int> > nbor_map;

    bool accessible;
    float pp[3], tsa = 0.0f, gtsa = 0.0f, rmax = 0.0f;
    int i1, j1, k1, icoord[3];
    int i, j, k, npts[3];
    unsigned int iatom;
    std::vector<unsigned int> neighbor;
    float prad = JACGetProbeRadius();


    if (pres>3) pres = 3;
    ProbeT probe(pres);
    if (!atoms.GetCount())
        return 0.0;

    int DIMENSION = 65;
    gridT grid;
    grid.Resize(0.0, DIMENSION, DIMENSION, DIMENSION);

    JACAtomsMerge jam(atoms, context);
    JACSetGridParams(grid, JACSurfaceTypes::SURF_ACCESSIBLE, true, jam);

    std::vector<float> exprad;
    std::vector<float> expradsq;
    exprad.resize(atoms.GetCount());
    expradsq.resize(atoms.GetCount());

    /* set up neighbor lists */
    for (iatom = 0; iatom < atoms.GetCount(); iatom++)
    {
        /* only use selected  atoms with a radius > 0 */
        float radius = atoms.GetRadius(iatom);
        if (radius > 0.0 && (atoms.IsSelected(iatom) || context.IsSelected(iatom)))
        {
            if (radius > rmax)
                rmax = radius;

            const float *coord = atoms.GetCoord(iatom);
            exprad[iatom] = radius + prad;
            expradsq[iatom] = exprad[iatom] * exprad[iatom];

            i = (unsigned int)((coord[0] - grid.org[0]) / grid.unit[0]);
            j = (unsigned int)((coord[1] - grid.org[1]) / grid.unit[1]);
            k = (unsigned int)((coord[2] - grid.org[2]) / grid.unit[2]);

            nbor_map[k*4225 + j*65 + i].push_back(iatom);
        }
    }


    /*
     * now, for each atom search neighboring grid cubes for contacts, and
     * evaluate if found
     */
    for (i = 0; i<3; i++)
        npts[i] = (unsigned int)((rmax + prad) / grid.unit[i]) + 2;

    for (iatom = 0; iatom < atoms.GetCount(); iatom++)
    {
        if (atoms.GetRadius(iatom) && atoms.IsSelected(iatom))
        {
            /* generate surface for this atom */
            const float *coord = atoms.GetCoord(iatom);
            neighbor.clear();
            icoord[0] = (unsigned int)(0.5 + (coord[0] - grid.org[0]) / grid.unit[0]);
            icoord[1] = (unsigned int)(0.5 + (coord[1] - grid.org[1]) / grid.unit[1]);
            icoord[2] = (unsigned int)(0.5 + (coord[2] - grid.org[2]) / grid.unit[2]);

            /* set up a neighbor list for this atom by going through the cubes */
            /* surrounding this one. */
            for (k1 = icoord[2] - npts[2]; k1 <= icoord[2] + npts[2]; k1++)
                if ((k1 >= 0) && (k1 < DIMENSION))
                    for (j1 = icoord[1] - npts[1]; j1 <= icoord[1] + npts[1]; j1++)
                        if ((j1 >= 0) && (j1 < DIMENSION))
                            for (i1 = icoord[0]-npts[0]; i1 <= icoord[0] + npts[0]; i1++)
                                if ((i1 >= 0) && (i1 < DIMENSION))
                                {
                                    std::vector<unsigned int> &v = nbor_map[k1*4225 + j1*65 + i1];
                                    for (std::vector<unsigned int>::iterator vi = v.begin();
                                         vi!=v.end(); ++vi)
                                    {
                                        unsigned int nbor_atom = *vi;
                                        if (nbor_atom != iatom)
                                        {
                                            float testdist;
                                            testdist = exprad[iatom] + exprad[nbor_atom];
                                            testdist *= testdist;
                                            if (testdist > distsq(atoms.GetCoord(nbor_atom), coord))
                                                neighbor.push_back(nbor_atom);
                                        }
                                    }
                                }

            /* test each point on a scaled probe */
            surface_area[iatom] = 0.0;
            for (unsigned int j = 0; j < probe.nprobe; j++)
            {
                /* scale/translate the probe */
                for (k = 0; k < 3; k++)
                    pp[k] = (coord[k]) + exprad[iatom]*probe.pco[j*3+k];

                accessible = true;
                for (unsigned int k = 0; k < neighbor.size(); k++)
                {
                    if (distsq(pp, atoms.GetCoord(neighbor[k])) < expradsq[neighbor[k]])
                    {
                        accessible = false;
                        break; /* skip rest of neighbors */
                    }
                }

                if (accessible)
                {
                    surface_area[iatom] += probe.area[j] * expradsq[iatom];
                    if (vs)
                        vs->SendVertex(pp);
                }
                gtsa += probe.area[j] * expradsq[iatom];
            }
            tsa += surface_area[iatom];
        }
    }

    fractional = tsa/gtsa;
    return tsa;
}



