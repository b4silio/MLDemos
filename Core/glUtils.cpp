#include <gl.h>
#include "glUtils.h"

unsigned int octa_indices[8][3]=
{
    {0,1,2},{0,2,3},
    {0,3,4},{0,4,1},
    {5,2,1},{5,3,2},
    {5,4,3},{5,1,4}
};

float octa_verts[6][3]=
{
    {0,0,-1},{1,0,0},
    {0,-1,0},{-1,0,0},
    {0,1,0},{0,0,1}
};

unsigned int icosa_indices[20][3]=
{
    {0,4,1},{0,9,4},{9,5,4},{4,5,8},{4,8,1},
    {8,10,1},{8,3,10},{5,3,8},{5,2,3},{2,7,3},
    {7,10,3},{7,6,10},{7,11,6},{11,0,6},{0,1,6},
    {6,1,10},{9,0,11},{9,11,2},{9,2,5},{7,2,11}
};

float icosa_verts[12][3]=
{
    {_A,0.0,-_B},{-_A,0.0,-_B},{_A,0.0,_B},{-_A,0.0,_B},
    {0.0,-_B,-_A},{0.0,-_B,_A},{0.0,_B,-_A},{0.0,_B,_A},
    {-_B,-_A,0.0},{_B,-_A,0.0},{-_B,_A,0.0},{_B,_A,0.0}
};

void normalize_vert(float *a)
{
    float d=sqrtf(a[0]*a[0]+a[1]*a[1]+a[2]*a[2]);
    a[0]/=d; a[1]/=d; a[2]/=d;
}

void draw_recursive_tri(float *a,float *b,float *c,unsigned int div,float r)
{
    if (div==0)
    {
        glNormal3fv(a);
        glVertex3f(a[0]*r,a[1]*r,a[2]*r);

        glNormal3fv(b);
        glVertex3f(b[0]*r,b[1]*r,b[2]*r);

        glNormal3fv(c);
        glVertex3f(c[0]*r,c[1]*r,c[2]*r);
    }
    else
    {
        register unsigned int i;
        float ab[3],ac[3],bc[3];
        for (i=0; i<3; i++)
        {
            ab[i]=(a[i]+b[i])/2.0f;
            ac[i]=(a[i]+c[i])/2.0f;
            bc[i]=(b[i]+c[i])/2.0f;
        }

        normalize_vert(ab);
        normalize_vert(ac);
        normalize_vert(bc);

        draw_recursive_tri(a,ab,ac,div-1,r);
        draw_recursive_tri(b,bc,ab,div-1,r);
        draw_recursive_tri(c,ac,bc,div-1,r);
        draw_recursive_tri(ab,bc,ac,div-1,r);
    }
}

void DrawTessellatedSphere(float radius, unsigned int detail,int solid_base)
{
    register unsigned int i;
    switch (solid_base)
    {
    case rat_octahedron_solid_base:
        glBegin(GL_TRIANGLES);
        for (i=0; i<8; i++)
            draw_recursive_tri
            (
                octa_verts[octa_indices[i][0]],
                octa_verts[octa_indices[i][1]],
                octa_verts[octa_indices[i][2]],
                detail,radius
            );
        glEnd();
        break;
    case rat_icosahedron_solid_base:
        glBegin(GL_TRIANGLES);
        for (i=0; i<20; i++)
            draw_recursive_tri
            (
                icosa_verts[icosa_indices[i][0]],
                icosa_verts[icosa_indices[i][1]],
                icosa_verts[icosa_indices[i][2]],
                detail,radius
            );
        glEnd();
        break;
    default:
        break;
    };
}

void DrawStandardSphere(double r, int lats, int longs)
{
    int i, j;
    for(i = 0; i <= lats; i++) {
        double lat0 = M_PI * (-0.5 + (double) (i - 1) / lats);
        double z0  = r*sin(lat0);
        double zr0 =  r*cos(lat0);

        double lat1 = M_PI * (-0.5 + (double) i / lats);
        double z1 = r*sin(lat1);
        double zr1 = r*cos(lat1);

        glBegin(GL_QUAD_STRIP);
        for(j = 0; j <= longs; j++) {
            double lng = 2 * M_PI * (double) (j - 1) / longs;
            double x = cos(lng);
            double y = sin(lng);

            glNormal3f(x * zr0, y * zr0, z0);
            glVertex3f(x * zr0, y * zr0, z0);
            glNormal3f(x * zr1, y * zr1, z1);
            glVertex3f(x * zr1, y * zr1, z1);
        }
        glEnd();
    }
}

void DrawSphereIsolines(double r, int segments)
{
    glBegin(GL_LINE_LOOP);
    for (int plane=0; plane < 3; plane++)
    {
        for (float theta=0; theta <= M_PI*2.f; theta += (M_PI*2.f)/segments)
        {
            float x = cosf(theta)*r;
            float y = sinf(theta)*r;
            switch(plane)
            {
            case 0: // x-y
                glVertex3f(x, y, 0);
                break;
            case 1: // x-z
                glVertex3f(x, 0, y);
                break;
            case 2: // y-z
                glVertex3f(0, x, y);
                break;
            }
        }
    }
    glEnd();
}
