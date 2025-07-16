#ifndef JAC_LINALG_H
#define JAC_LINALG_H
#include <cmath>

inline float distsq(const float *a, const float *b)
/*- the square of the distance -*/
{
    return (a[0]-b[0])*(a[0]-b[0])
           +(a[1]-b[1])*(a[1]-b[1])
           +(a[2]-b[2])*(a[2]-b[2]);
}

inline float dist(const float *a, const float *b)
/*- return the Eucledian distance between a & b -*/
{
    return (float)sqrt(distsq(a, b));
}

inline float dot(const float *x, const float *y)
/*- return dot product of x & y -*/
{
    return (x[0]*y[0] + x[1]*y[1] + x[2]*y[2]);
}

inline float amag(const float *x)
/*- return magnitude of x -*/
{
    float amag = dot(x, x);
    return (float)sqrt(amag);
}

inline void vnorm(float *x)
/*- normalize x -*/
{
    float fact;

    fact = amag(x);
    if (fact<1e-12)
        return;
    x[0] = x[0]/fact;
    x[1] = x[1]/fact;
    x[2] = x[2]/fact;
}

inline void vmidp(float *a, const float *b, const float *c)
/*- return a = midpoint of b & c -*/
{
    a[0] = (b[0]+c[0])/2.0f;
    a[1] = (b[1]+c[1])/2.0f;
    a[2] = (b[2]+c[2])/2.0f;
}

inline void vcent(float *a, const float *b, const float *c, const float *d)
/*- return a = center (average) of b, c, and d -*/
{
    a[0] = (b[0]+c[0]+d[0])/3.0f;
    a[1] = (b[1]+c[1]+d[1])/3.0f;
    a[2] = (b[2]+c[2]+d[2])/3.0f;
}

inline void cross(float *z, const float *x, const float *y)
/*- return Z = X cross Y -*/
{
    z[0] = (x[1]*y[2])-(y[1]*x[2]);
    z[1] = (x[2]*y[0])-(y[2]*x[0]);
    z[2] = (x[0]*y[1])-(y[0]*x[1]);
}

inline void vset(float *a, const float *b)
/*- set a = b -*/
{
    a[0] = b[0];
    a[1] = b[1];
    a[2] = b[2];
}

inline void vsum(float *a, const float *b, const float *c)
/*- return A = B + C */
{
    a[0] = b[0]+c[0];
    a[1] = b[1]+c[1];
    a[2] = b[2]+c[2];
}

inline void vdif(float *a, const float *b, const float *c)
/*- return A = B - C */
{
    a[0] = b[0]-c[0];
    a[1] = b[1]-c[1];
    a[2] = b[2]-c[2];
}

inline float tarea(const float *a, const float *b, const float *c)
/*- returns the area of triangle ABC -*/
{
    float d[3], e[3], f[3];

    /* find area */
    vdif(d, a, b);
    vdif(e, c, b);
    cross(f, e, d);
    return (float)(amag(f) / 2.0f);
}
#endif // ifndef JAC_LINALG_H
