#ifndef GLUTILS_H
#define GLUTILS_H

/**
 *  From NinjaRat
 *  http://musingsofninjarat.wordpress.com/spheres-through-triangle-tessellation/
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <vector>

#ifdef WIN32
#include <GL/glew.h>
#endif

#define rat_octahedron_solid_base	1
#define rat_icosahedron_solid_base	2

#define _A 0.525731112119133606f
#define _B 0.850650808352039932f

#ifndef MATH_PI
#	ifndef M_PI
#		define MATH_PI 3.14159f
#	else
#		define MATH_PI M_PI
#	endif
#endif

extern unsigned int octa_indices[8][3];
extern float octa_verts[6][3];
extern unsigned int icosa_indices[20][3];
extern float icosa_verts[12][3];
extern inline void normalize_vert(float *a);
extern void draw_recursive_tri(float *a,float *b,float *c,unsigned int div,float r);
extern void DrawTessellatedSphere(float radius=1.f, unsigned int detail=3,int solid_base=1);
extern void DrawStandardSphere(double r=1.f, int lats=32, int longs=32);
extern void DrawSphereIsolines(double r=1.f, int segments=64);
extern GLuint DrawGaussian(float *mean, float *eigVal, float *eigVec, float prior=1.f, bool wireframe=true,
                                        float colorRed=0.5f, float colorGreen=0.5f, float colorBlue=0.5f);
extern GLuint DrawMeshGrid(float *values, float *mins, float *maxes, int xSteps, int ySteps, int valueDim);


#endif // GLUTILS_H
