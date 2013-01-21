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
#include <mymaths.h>

#include <QVector>
#include <QMatrix4x4>
#include <QVector3D>
#include <QtOpenGL>
#include <QGLFunctions>

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE 0x809D
#define GL_POINT_SPRITE 0x8861
#define GL_COORD_REPLACE 0x8862
#define GL_PROGRAM_POINT_SIZE_EXT 0x8642
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


struct GLObject
{
    QVector<QVector3D> vertices;
    QVector<QVector3D> normals;
    QVector<QVector4D> colors;
    QVector<QVector4D> barycentric;
    QMatrix4x4 model;
    QString objectType;
    QString style;
};

struct GLLight
{
    GLfloat ambientLight[4];
    GLfloat diffuseLight[4];
    GLfloat specularLight[4];
    GLfloat position[4];
    GLLight();
    GLLight(float x, float y, float z);
    void SetPosition(float x, float y, float z);
    void SetAmbient(float r, float g, float b, float a=1.f);
    void SetDiffuse(float r, float g, float b, float a=1.f);
    void SetSpecular(float r, float g, float b, float a=1.f);
};

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
extern std::pair<QVector<QVector3D>, QMatrix4x4> DrawGaussian(float radius, float *mean, float *eigVal, float *eigVec);
extern std::pair<QVector<QVector3D>, QMatrix4x4> DrawGaussianLines(float radius, float *mean, float *eigVal, float *eigVec);
extern GLObject GenerateMeshGrid(fvec &gridPoints, int xSteps, fvec mins, fvec maxes, int xInd=0, int yInd=1, int zInd=2);
extern GLObject GenerateMeshGrid(float *gridPoints, int xSteps, int ySteps, fvec mins, fvec maxes, int xInd=0, int yInd=1, int zInd=2);

#endif // GLUTILS_H
