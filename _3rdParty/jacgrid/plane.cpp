#include "jacgrid.h"

JACPlane::JACPlane()
    : xdim(0),
      ydim(0),
      data(0)
{
}

JACPlane::JACPlane(unsigned int _xdim, unsigned int _ydim)
    : xdim(_xdim),
      ydim(_ydim)
{
    data = (float*)malloc(sizeof(float)*xdim*ydim);
}

JACPlane::~JACPlane()
{
    if (data)
        free(data);
    data = 0;
}

void JACPlane::SetParams(float _unit,
                         float *_org,
                         float *_xprime, float *_yprime, float *_zprime)
{
    unit = _unit;
    for (int i = 0; i<3; ++i)
    {
        org[i] = _org[i];
        xprime[i] = _xprime[i];
        yprime[i] = _yprime[i];
        zprime[i] = _zprime[i];
    }
}

void JACPlane::Transform(float *v, float i, float j) const
{
    v[0] = (i*xprime[0] + j*yprime[0])*unit + org[0];
    v[1] = (i*xprime[1] + j*yprime[1])*unit + org[1];
    v[2] = (i*xprime[2] + j*yprime[2])*unit + org[2];
}

void JACPlane::LoadFromGrid(const gridT &grid)
{
    register unsigned int i, l, m;
    float r[3];
    register float *tmp;
    float DATA1, DATA2, DATA3, DATA4, DATA5, DATA6, DATA7, DATA8;
    unsigned int valid, thisaddr, dim2;
    float *current;
    float x, y, z, x1, y1, z1, t5, t10, t15, t20;

    /* clear the grid */
    dim2 = xdim*ydim;
    for (l = 0, current = data; l<dim2; l++)
        *current++ = 0.0f;

    /* fill it with interpolated points */
    dim2 = grid.npts[0]*grid.npts[1];
    for (m = 0; m<ydim; m++)
    {
        for (l = 0; l<xdim; l++)
        {
            thisaddr = xdim*m+l;
            valid = true;
            for (i = 0; i<3; i++)
            {
                r[i] = ((org[i] - grid.org[i])
                        +unit*((float)l*xprime[i] + (float)m*yprime[i])) /  grid.unit[i];
                if ((r[i]<0) || (r[i]>(grid.npts[i]-1)))
                {
                    data[thisaddr] = 0.0f;
                    valid = false;
                    break;
                }
            }
            /* setup */
            if (!valid)
                continue;
            tmp = grid.data + ((unsigned int)r[2]*dim2)
                  +((unsigned int)r[1]*grid.npts[0]) + (unsigned int)r[0];
            DATA1 = *(tmp);
            DATA2 = *(tmp + 1);
            tmp += grid.npts[0];
            DATA3 = *(tmp + 1);
            DATA4 = *(tmp);
            tmp = tmp - grid.npts[0] + dim2;
            DATA5 = *(tmp);
            DATA6 = *(tmp + 1);
            tmp += grid.npts[0];
            DATA7 = *(tmp + 1);
            DATA8 = *(tmp);

            /* trilinear interpolation */
            x = r[0]-(unsigned int)r[0];
            y = r[1]-(unsigned int)r[1];
            z = r[2]-(unsigned int)r[2];
            x1 = 1.0f - x;
            y1 = 1.0f - y;
            z1 = 1.0f - z;
            t5 = y1*z1;
            t10 = y*z1;
            t15 = y1*z;
            t20 = y*z;
            data[thisaddr] =
                DATA1*x1*t5 + DATA2*x*t5 + DATA3*x*t10 + DATA4*x1*t10
                +DATA5*x1*t15 + DATA6*x*t15 + DATA7*x*t20 + DATA8*x1*t20;
        } /* m */
    } /* l */
}


typedef struct
{
    unsigned int nedges;
    unsigned int edges[4];
} CellTableT;

static CellTableT cell_table[16] =
{
    {0, { 0, 0 }},
    {2, { 1, 4 }},
    {2, { 1, 2 }},
    {2, { 2, 4 }},
    {2, { 2, 3 }},
    {4, { 1, 3, 2, 4 }},
    {2, { 1, 3 }},
    {2, { 3, 4 }},
    {2, { 3, 4 }},
    {2, { 1, 3 }},
    {4, { 1, 3, 2, 4 }},
    {2, { 2, 3 }},
    {2, { 2, 4 }},
    {2, { 1, 2 }},
    {2, { 4, 1 }},
    {0, { 0, 0 }}
};

#define linterp(start, end, thresh) ((thresh - start)/(end-start))

void JACPlane::Contour(float threshold, VertexSender &vs)
/*-
 * routine implements a version of marching squares to contour a 2D data
 * field.  data is a pointer to the 2D array of data of dimension
 * (xdim,ydim) threshold is the level at which to contour. vs.SendVertex is
 * called once for each vertex in the contour, and is passed a vertex.
 * each pair of these vertices is a line segment in the contour.
   -*/
{
    unsigned int index = 0;
    register float *tmp;
    float DATA1, DATA2, DATA3, DATA4;
    float f, v[3];
    register unsigned int x1, y1, x2, y2;
    register unsigned int xdim1, ydim1;
    unsigned int edge_loop, edge;

    ydim1 = ydim-1;
    xdim1 = xdim-1;
    for (y1 = 0, y2 = 1; y1<ydim1; y1++, y2++)
    {
        for (x1 = 0, x2 = 1; x1<xdim1; x1++, x2++)
        {
            index = 0;
            tmp = data + (y1*xdim) + x1;
            index += (threshold <= (DATA1 = *(tmp)));
            index += (threshold <= (DATA2 = *(tmp+1)))*2;
            tmp += xdim;
            index += (threshold <= (DATA3 = *(tmp + 1))) * 4;
            index += (threshold <= (DATA4 = *(tmp))) * 8;
            if (index)
            {
                for (edge_loop = 0; edge_loop<cell_table[index].nedges; edge_loop++)
                {
                    edge = cell_table[index].edges[edge_loop];
                    switch (edge)
                    {
                    case 1:
                        f = x1 + linterp(DATA1, DATA2, threshold);
                        Transform(v, f, (float)y1);
                        break;
                    case 2:
                        f = y1 + linterp(DATA2, DATA3, threshold);
                        Transform(v, (float)x2, f);
                        break;
                    case 3:
                        f = x1 + linterp(DATA4, DATA3, threshold);
                        Transform(v, f, (float)y2);
                        break;
                    case 4:
                        f = y1 + linterp(DATA1, DATA4, threshold);
                        Transform(v, (float)x1, f);
                        break;
                    default:
                        break;
                    } /* switch */
                    vs.SendVertex(v);
                } /* each edge */
            } /* index != 0 */
        } /* x1 */
    } /* y1 */
}

#define gaddr(i, j, k) (((k)*GDIVISIONS2) + ((j)*GDIVISIONS) + (i))

void JACPlane::ContourGrid(const gridT &grid, char axis,
                           float threshold, VertexSender &vs)
{
    unsigned int GDIVISIONS = grid.npts[0];
    unsigned int GDIVISIONS2 = grid.npts[0]*grid.npts[1];
    float xunit[3] = {1.0, 0.0, 0.0};
    float yunit[3] = {0.0, 1.0, 0.0};
    float zunit[3] = {0.0, 0.0, 1.0};
    unsigned int i, j, k, index;

    if (data)
        free(data);
    data = 0;

    switch (axis)
    {
    case 'Z':
    case 'z':
    {
        xdim = grid.npts[0];
        ydim = grid.npts[1];
        data = (float*)malloc(sizeof(float)*xdim*ydim);
        for (i = 0; i<3; i++)
        {
            unit = 1.0;
            org[i] = 0.0;
            xprime[i] = xunit[i];
            yprime[i] = yunit[i];
            zprime[i] = zunit[i];
        }
        for (k = 0; k<grid.npts[2]; k++)
        {
            org[2] = (float)k;
            for (j = 0; j<grid.npts[1]; j++)
            {
                for (i = 0; i<grid.npts[0]; i++)
                {
                    index = j*grid.npts[0] + i;
                    data[index] = grid.data[gaddr(i, j, k)];
                }
            }
            Contour(threshold, vs);
        }
    }
    break;
    case 'Y':
    case 'y':
    {
        xdim = grid.npts[0];
        ydim = grid.npts[2];
        data = (float*)malloc(sizeof(float)*xdim*ydim);

        for (i = 0; i<3; i++)
        {
            unit = 1.0;
            org[i] = 0.0;
            xprime[i] = xunit[i];
            yprime[i] = zunit[i];
            zprime[i] = yunit[i];
        }
        for (j = 0; j<grid.npts[1]; j++)
        {
            org[1] = (float)j;
            for (k = 0; k<grid.npts[2]; k++)
            {
                for (i = 0; i<grid.npts[0]; i++)
                {
                    index = k*grid.npts[0] + i;
                    data[index] = grid.data[gaddr(i, j, k)];
                }
            }
            Contour(threshold, vs);
        }
    }
    break;
    case 'X':
    case 'x':
    {
        xdim = grid.npts[1];
        ydim = grid.npts[2];
        data = (float*)malloc(sizeof(float)*xdim*ydim);
        for (i = 0; i<3; i++)
        {
            unit = 1.0;
            org[i] = 0.0;
            xprime[i] = yunit[i];
            yprime[i] = zunit[i];
            zprime[i] = xunit[i];
        }
        for (i = 0; i<grid.npts[0]; i++)
        {
            org[0] = (float)i;
            for (k = 0; k<grid.npts[2]; k++)
            {
                for (j = 0; j<grid.npts[1]; j++)
                {
                    index = k*grid.npts[1] + j;
                    data[index] = grid.data[gaddr(i, j, k)];
                }
            }
            Contour(threshold, vs);
        }
    }
    break;
    default:
        return;
    }

    if (data)
        free(data);
    data = 0;
}

