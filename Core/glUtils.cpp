#include "glUtils.h"
#include <public.h>

QGLFunctions *gl = 0;

GLLight::GLLight()
{
    position[0] = 100.f;
    position[1] = 100.f;
    position[2] = 100.f;
    position[3] = 1.f;
    ambientLight[0] = 0.1f;
    ambientLight[1] = 0.1f;
    ambientLight[2] = 0.1f;
    ambientLight[3] = 1.f;
    diffuseLight[0] = .7f;
    diffuseLight[1] = .7f;
    diffuseLight[2] = .7f;
    diffuseLight[3] = 1.f;
    specularLight[0] = 0;
    specularLight[1] = 0;
    specularLight[2] = 0;
    specularLight[3] = 1.f;
}

GLLight::GLLight(float x, float y, float z)
{
    position[0] = x;
    position[1] = y;
    position[2] = z;
    position[3] = 1.f;
    ambientLight[0] = 0.1f;
    ambientLight[1] = 0.1f;
    ambientLight[2] = 0.1f;
    ambientLight[3] = 1.f;
    diffuseLight[0] = .7f;
    diffuseLight[1] = .7f;
    diffuseLight[2] = .7f;
    diffuseLight[3] = 1.f;
    specularLight[0] = 0;
    specularLight[1] = 0;
    specularLight[2] = 0;
    specularLight[3] = 1.f;
}

void GLLight::SetPosition(float x, float y, float z)
{
    position[0] = x;
    position[1] = y;
    position[2] = z;
    position[3] = 1.f;
}

void GLLight::SetAmbient(float r, float g, float b, float a)
{
    ambientLight[0] = r;
    ambientLight[1] = g;
    ambientLight[2] = b;
    ambientLight[3] = a;
}

void GLLight::SetDiffuse(float r, float g, float b, float a)
{
    diffuseLight[0] = r;
    diffuseLight[1] = g;
    diffuseLight[2] = b;
    diffuseLight[3] = a;
}

void GLLight::SetSpecular(float r, float g, float b, float a)
{
    specularLight[0] = r;
    specularLight[1] = g;
    specularLight[2] = b;
    specularLight[3] = a;
}

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
    for (int plane=0; plane < 3; plane++)
    {
        glBegin(GL_LINE_LOOP);
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
        glEnd();
    }
}

std::pair<QVector<QVector3D>, QMatrix4x4> DrawGaussian(float radius, float *mean, float *eigVal, float *eigVec)
{
    // we copy the eigenvectors to a 4x4 rotation matrix
    float rotation[4*4];
    rotation[0]  = eigVec[0]; rotation[0 + 1] = eigVec[1]; rotation[0 + 2] = eigVec[2]; rotation[0 + 3] = 0;
    rotation[4]  = eigVec[3]; rotation[4 + 1] = eigVec[4]; rotation[4 + 2] = eigVec[5]; rotation[4 + 3] = 0;
    rotation[8]  = eigVec[6]; rotation[8 + 1] = eigVec[7]; rotation[8 + 2] = eigVec[8]; rotation[8 + 3] = 0;
    rotation[12] = 0;  rotation[12 + 1] = 0 ; rotation[12 + 2] = 0 ; rotation[12 + 3] = 1;

    QMatrix4x4 rot;
    for(int i=0; i<4; i++)
    {
        for(int j=0; j<4; j++)
        {
            rot(j,i) = rotation[i*4 + j];
        }
    }

    QVector<QVector3D> vertices;
    QMatrix4x4 model;
    model.translate(mean[0], mean[1], mean[2]);
    model *= rot;
    model.scale(eigVal[0],eigVal[1],eigVal[2]);
    int lats = 32, longs = 32; // amount of segments to be drawn
    int i, j;
    for(i = 0; i <= lats; i++) {
        double lat0 = M_PI * (-0.5 + (double) (i - 1) / lats);
        double z0  = radius*sin(lat0);
        double zr0 =  radius*cos(lat0);

        double lat1 = M_PI * (-0.5 + (double) i / lats);
        double z1 = radius*sin(lat1);
        double zr1 = radius*cos(lat1);

        QVector3D A, B, oldA, oldB;
        for(j = 0; j <= longs; j++) {
            double lng = 2 * M_PI * (double) (j - 1) / longs;
            double x = cos(lng);
            double y = sin(lng);

            A = QVector3D(x*zr0, y*zr0, z0);
            B = QVector3D(x*zr1, y*zr1, z1);
            if(i>0 && j>0)
            {
                vertices.append(oldA);
                vertices.append(oldB);
                vertices.append(B);
                vertices.append(A);
            }
            oldA = A;
            oldB = B;
        }
    }
    return std::make_pair(vertices, model);
}

std::pair<QVector<QVector3D>, QMatrix4x4> DrawGaussianLines(float radius, float *mean, float *eigVal, float *eigVec)
{
    // we copy the eigenvectors to a 4x4 rotation matrix
    float rotation[4*4];
    rotation[0]  = eigVec[0]; rotation[0 + 1] = eigVec[1]; rotation[0 + 2] = eigVec[2]; rotation[0 + 3] = 0;
    rotation[4]  = eigVec[3]; rotation[4 + 1] = eigVec[4]; rotation[4 + 2] = eigVec[5]; rotation[4 + 3] = 0;
    rotation[8]  = eigVec[6]; rotation[8 + 1] = eigVec[7]; rotation[8 + 2] = eigVec[8]; rotation[8 + 3] = 0;
    rotation[12] = 0;  rotation[12 + 1] = 0 ; rotation[12 + 2] = 0 ; rotation[12 + 3] = 1;

    QMatrix4x4 rot;
    for(int i=0; i<4; i++)
    {
        for(int j=0; j<4; j++)
        {
            rot(j,i) = rotation[i*4 + j];
        }
    }

    QVector<QVector3D> vertices;
    QMatrix4x4 model;
    model.translate(mean[0], mean[1], mean[2]);
    model *= rot;
    model.scale(eigVal[0],eigVal[1],eigVal[2]);

    int segments = 64;
    for (int plane=0; plane < 3; plane++)
    {
        float oldX, oldY;
        float firstX, firstY;
        for (float theta=0; theta <= M_PI*2.f; theta += (M_PI*2.f)/segments)
        {
            float x = cosf(theta)*radius;
            float y = sinf(theta)*radius;
            if(theta > 0)
            {
                switch(plane)
                {
                case 0: // x-y
                    vertices.append(QVector3D(oldX,oldY,0));
                    vertices.append(QVector3D(x,y,0));
                    break;
                case 1: // x-z
                    vertices.append(QVector3D(oldX,0,oldY));
                    vertices.append(QVector3D(x,0,y));
                    break;
                case 2: // y-z
                    vertices.append(QVector3D(0,oldX,oldY));
                    vertices.append(QVector3D(0,x,y));
                    break;
                }
            }
            else {
                firstX = x;
                firstY = y;
            }
            oldX = x;
            oldY = y;
        }
        switch(plane)
        {
        case 0: // x-y
            vertices.append(QVector3D(oldX,oldY,0));
            vertices.append(QVector3D(firstX,firstY,0));
            break;
        case 1: // x-z
            vertices.append(QVector3D(oldX,0,oldY));
            vertices.append(QVector3D(firstX,0,firstY));
            break;
        case 2: // y-z
            vertices.append(QVector3D(0,oldX,oldY));
            vertices.append(QVector3D(0,firstX,firstY));
            break;
        }
    }
    return std::make_pair(vertices, model);
}

GLuint DrawGaussian(float *mean, float *eigVal, float *eigVec, float prior,
                    bool wireframe, float colorRed, float colorGreen, float colorBlue)
{
    // we copy the eigenvectors to a 4x4 rotation matrix
    float rotation[4*4];
    rotation[0]  = eigVec[0]; rotation[0 + 1] = eigVec[1]; rotation[0 + 2] = eigVec[2]; rotation[0 + 3] = 0;
    rotation[4]  = eigVec[3]; rotation[4 + 1] = eigVec[4]; rotation[4 + 2] = eigVec[5]; rotation[4 + 3] = 0;
    rotation[8]  = eigVec[6]; rotation[8 + 1] = eigVec[7]; rotation[8 + 2] = eigVec[8]; rotation[8 + 3] = 0;
    rotation[12] = 0;  rotation[12 + 1] = 0 ; rotation[12 + 2] = 0 ; rotation[12 + 3] = 1;

    GLuint list = glGenLists(1);
    glNewList(list, GL_COMPILE);

    glPushAttrib(GL_ALL_ATTRIB_BITS);

    glDisable( GL_TEXTURE_2D );
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#ifndef WIN32
    glBlendEquation(GL_FUNC_ADD);
#endif
    /*
	QGLFunctions gl(QGLContext::currentContext());
	gl.glBlendEquation(GL_FUNC_ADD);
	*/
    if(!wireframe)
    {
        glEnable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glEnable(GL_ALPHA_TEST);
        glShadeModel(GL_SMOOTH);

        int steps = 30;
        float speed = 3.f;
        for(int d=0; d<steps; d++)
        {
            float mcolor[] = { colorRed, colorGreen, colorBlue, (std::min(prior+0.3f, 1.f))*(1.f - d/(float)steps)*expf(-(d/(float)steps)*speed)};
            glPushMatrix();
            glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mcolor);
            glTranslatef(mean[0], mean[1], mean[2]);
            glMultMatrixf(rotation);
            glScalef(eigVal[0], eigVal[1], eigVal[2]);
            //DrawSphere(0.01 + d * 2.8f / steps);
            DrawStandardSphere(0.01 + d * 2.8f / steps);
            glPopMatrix();
        }
    }
    else
    {
        glDisable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glEnable( GL_LINE_SMOOTH );
        glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );

        glPushMatrix();
        glTranslatef(mean[0], mean[1], mean[2]);
        glMultMatrixf(rotation);
        glScalef(eigVal[0], eigVal[1], eigVal[2]);
        glColor3d(0,0,0);
        glLineWidth(2.f);
        glDisable(GL_LINE_STIPPLE); // dashed/ dotted lines
        DrawSphereIsolines(1);
        glLineWidth(0.5f);
        glEnable(GL_LINE_STIPPLE); // dashed/ dotted lines
        glLineStipple (1, 0xAAAA); // dash pattern AAAA: dots
        DrawSphereIsolines(2);
        glPopMatrix();
    }
    glPopAttrib();
    glEndList();
    return list;
}

GLuint DrawMeshGrid(float *values, float *mins, float *maxes, int xSteps, int ySteps, int valueDim)
{
    GLuint list = glGenLists(1);
    glNewList(list, GL_COMPILE);

    glPushAttrib(GL_ALL_ATTRIB_BITS);

    glDisable( GL_TEXTURE_2D );
    glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#ifndef WIN32
    glBlendEquation(GL_FUNC_ADD);
#endif
	 /*
	 QGLFunctions gl(QGLContext::currentContext());
	 gl.glBlendEquation(GL_FUNC_ADD);
	 */

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glEnable( GL_LINE_SMOOTH );
    glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );

    //glLineWidth(1.f);
    //glDisable(GL_LINE_STIPPLE); // dashed/ dotted lines
    glLineWidth(0.5f);
    glEnable(GL_LINE_STIPPLE); // dashed/ dotted lines
    glLineStipple (2, 0xAAAA); // dash pattern AAAA: dots


    int xInd = 0, yInd = 1, zInd = valueDim;
    if(valueDim == yInd) yInd = 2;
    else if(valueDim == xInd) xInd = 2;

    glColor3f(0.f,0.f,0.f);
    float v[3];
    for(int y=0; y<ySteps; y++)
    {
        v[yInd] = y/(float)ySteps*(maxes[yInd]-mins[yInd]) + mins[yInd];
        glBegin(GL_LINE_STRIP);
        for(int x=0; x<xSteps; x++)
        {
            v[xInd] = x/(float)xSteps*(maxes[xInd]-mins[xInd]) + mins[xInd];
            v[zInd] = values[x+y*xSteps];
            glVertex3f(v[0], v[1], v[2]);
        }
        glEnd();
    }
    for(int x=0; x<xSteps; x++)
    {
        v[xInd] = x/(float)xSteps*(maxes[xInd]-mins[xInd]) + mins[xInd];
        glBegin(GL_LINE_STRIP);
        for(int y=0; y<ySteps; y++)
        {
            v[yInd] = y/(float)ySteps*(maxes[yInd]-mins[yInd]) + mins[yInd];
            v[zInd] = values[x+y*xSteps];
            glVertex3f(v[0], v[1], v[2]);
        }
        glEnd();
    }

    glPopAttrib();
    glEndList();
    return list;
}


GLObject GenerateMeshGrid(fvec &gridPoints, int xSteps, fvec mins, fvec maxes, int xInd, int yInd, int zInd)
{
    int ySteps = gridPoints.size() / xSteps;
    return GenerateMeshGrid(&gridPoints[0], xSteps, ySteps, mins, maxes, xInd, yInd, zInd);
}

GLObject GenerateMeshGrid(float *gridPoints, int xSteps, int ySteps, fvec mins, fvec maxes, int xInd, int yInd, int zInd)
{
    GLObject o;
    QVector3D v1,v2,v3,v4,normal;
    std::vector<QVector3D> normalMap((ySteps-1)*(xSteps-1));
    float pt1[3], pt2[3], pt3[3], pt4[3];
    FOR(y, ySteps-1)
    {
        pt1[yInd] = y/(float)ySteps*(maxes[yInd]-mins[yInd]) + mins[yInd];
        pt2[yInd] = y/(float)ySteps*(maxes[yInd]-mins[yInd]) + mins[yInd];
        pt3[yInd] = (y+1)/(float)ySteps*(maxes[yInd]-mins[yInd]) + mins[yInd];
        pt4[yInd] = (y+1)/(float)ySteps*(maxes[yInd]-mins[yInd]) + mins[yInd];
        FOR(x, xSteps-1)
        {
            pt1[xInd] = x/(float)xSteps*(maxes[xInd]-mins[xInd]) + mins[xInd];
            pt2[xInd] = (x+1)/(float)xSteps*(maxes[xInd]-mins[xInd]) + mins[xInd];
            pt3[xInd] = (x+1)/(float)xSteps*(maxes[xInd]-mins[xInd]) + mins[xInd];
            pt4[xInd] = x/(float)xSteps*(maxes[xInd]-mins[xInd]) + mins[xInd];

            pt1[zInd] = gridPoints[x    +    y*xSteps];
            pt2[zInd] = gridPoints[(x+1)+    y*xSteps];
            pt3[zInd] = gridPoints[(x+1)+(y+1)*xSteps];
            pt4[zInd] = gridPoints[x    +(y+1)*xSteps];
            v1 = QVector3D(pt1[0],pt1[1],pt1[2]);
            v2 = QVector3D(pt2[0],pt2[1],pt2[2]);
            v3 = QVector3D(pt3[0],pt3[1],pt3[2]);
            v4 = QVector3D(pt4[0],pt4[1],pt4[2]);
            QVector3D a = (v2 - v1).normalized();
            QVector3D b = (v4 - v1).normalized();
            normal = QVector3D::crossProduct(b,a);
            normalMap[y*(xSteps-1) + x] = normal.normalized();
            o.vertices.append(v1);
            o.vertices.append(v2);
            o.vertices.append(v3);
            o.vertices.append(v4);
        }
    }
    o.normals.resize(o.vertices.size());

    // we compute the normals themselves
    FOR(y, ySteps-1)
    {
        FOR(x,xSteps-1)
        {
            int i1 = (x + y*(xSteps-1))*4;
            int i2 = ((x-1) + y*(xSteps-1))*4 + 1;
            int i3 = ((x-1) + (y-1)*(xSteps-1))*4 + 2;
            int i4 = (x + (y-1)*(xSteps-1))*4 + 3;
            QVector3D n = normalMap[x + y*(xSteps-1)];
            normal = QVector3D(0,0,0);
            o.normals[i1] += n;
            if(x)
            {
                o.normals[i2] += n;
                if(y) o.normals[i3] += n;
            }
            if(y) o.normals[i4] += n;
        }
    }
    FOR(i, o.normals.size()) o.normals[i].normalize();

    o.objectType = "Surfaces,quads";
    return o;
}
