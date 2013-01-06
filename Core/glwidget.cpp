#ifdef WIN32
#include <GL/glew.h>
#endif
#include <QtGui>
#include <QtOpenGL>
#include <QDebug>

#include <math.h>
#include <mymaths.h>
#include "glwidget.h"
#include <MathLib/MathLib.h>
#include "glUtils.h"

#define ZoomZero 0.049f
using namespace std;

GLWidget::GLWidget(Canvas *canvas, QWidget *parent)
    : QGLWidget(parent), canvas(canvas), zoomFactor(ZoomZero)
{
    width = 800;
    height = 600;
    xRot = yRot = zRot = 0;
    xPos = yPos = zPos = 0.f;

    startTimer(20);
}

void GLWidget::timerEvent(QTimerEvent *)
{
    repaint();
}

GLWidget::~GLWidget()
{
    makeCurrent();
    clearLists();
}

void GLWidget::clearLists()
{
    FOR(i, drawSampleLists.size())
    {
        glDeleteLists(drawSampleLists[i], 1);
    }
    FOR(i, drawLists.size())
    {
        glDeleteLists(drawLists[i], 1);
    }
    drawSampleLists.clear();
    drawLists.clear();
    drawSampleListCenters.clear();
    objects.clear();
}

void GLWidget::initializeGL()
{
    /*
    // this enables multisampling (antialiasing)
    glEnable(GL_MULTISAMPLE);
    GLint bufs;
    GLint samples;
    glGetIntegerv(GL_SAMPLE_BUFFERS, &bufs);
    glGetIntegerv(GL_SAMPLES, &samples);
    qDebug("Have %d buffers and %d samples", bufs, samples);
    */

    textureData = new unsigned char*[textureCount];
    // first we generate the samples sprite (a circle with a black c)
    FOR(i, textureCount)
    {
        textureData[i] = new unsigned char[texWidth*texHeight*4];
        unsigned char *pData = textureData[i];
        FOR(y, texHeight)
        {
            FOR(x, texWidth)
            {
                int offs = (x + y*texWidth) * 4;
                float xoffs = ((float)x - texHalfWidth) / texHalfWidth;
                float yoffs = ((float)y - texHalfWidth) / texHalfHeight;
                float r = std::sqrt(xoffs*xoffs + yoffs*yoffs);
                switch (i)
                {
                case 0: // samples
                {
                    if(r < 0.65)
                    {
                        pData[offs + 0] = 255; //r
                        pData[offs + 1] = 255; //g
                        pData[offs + 2] = 255; //b
                        pData[offs + 3] = 255; // *
                    }
                    else if (r < 0.9)
                    {
                        pData[offs + 0] = 0; //r
                        pData[offs + 1] = 0; //g
                        pData[offs + 2] = 0; //b
                        pData[offs + 3] = 255; // *
                    }
                    else
                    {
                        pData[offs + 0] = 255; //r
                        pData[offs + 1] = 255; //g
                        pData[offs + 2] = 255; //b
                        pData[offs + 3] = 0; // *
                    }
                }
                    break;
                case 1: // wide circle
                {
                    if(r > 0.6 && r < 0.9)
                    {
                        pData[offs + 0] = 255; //r
                        pData[offs + 1] = 255; //g
                        pData[offs + 2] = 255; //b
                        pData[offs + 3] = 255; // *
                    }
                    else
                    {
                        pData[offs + 0] = 255; //r
                        pData[offs + 1] = 255; //g
                        pData[offs + 2] = 255; //b
                        pData[offs + 3] = 0; // *
                    }
                }
                    break;
                }
                //printf("alpha: %f\n", pData[x + y*texWidth + 3]);
            }
        }
    }

    glGenTextures(2, textureNames); // 0: samples, 1: wide circles
    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    FOR(i, textureCount)
    {
        glBindTexture(GL_TEXTURE_2D, textureNames[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData[i]);
    }
    glEnable(GL_POINT_SPRITE);
    glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable( GL_POINT_SMOOTH );

    // Somewhere in the initialization part of your programâ¦
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // Create light components
    GLfloat ambientLight[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat diffuseLight[] = { 0.8f, 0.8f, 0.8, 1.0f };
    GLfloat specularLight[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    //GLfloat position[] = { -1.5f, 1.0f, -4.0f, 1.0f };
    GLfloat position[] = { 10.0f, 10.0f, 10.0f, 1.0f };

    // Assign created components to GL_LIGHT0
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
    glLightfv(GL_LIGHT0, GL_POSITION, position);
    glEnable(GL_NORMALIZE); // so that when we scale stuff, the lighting doesnt go bonkers

    // We initialize the lights
    GLLight light;
    light.SetAmbient(0.1,0.1,0.1);
    light.SetDiffuse(0.5,0.5,0.5);
    light.SetSpecular(0.5,0.5,0.5);
    light.SetPosition(50.0f, 0.0f, 0.0f);
    lights.push_back(light);
    light.SetAmbient(0,0,0);
    light.SetSpecular(0,0,0);
    light.SetPosition(-50.0f, 50.0f, -50.0f);
    lights.push_back(light);
    light.SetAmbient(0,0,0);
    light.SetSpecular(0,0,0);
    light.SetPosition(50.0f, 50.0f, 50.0f);
    lights.push_back(light);


    // We initialize the shaders
    QGLShaderProgram *program=0;
    LoadShader(&program,":/MLDemos/shaders/drawSamples.vsh",":/MLDemos/shaders/drawSamples.fsh");
    program->bindAttributeLocation("vertex", 0);
    program->bindAttributeLocation("color", 1);
    shaders["Samples"] = program;
    program = 0;
    LoadShader(&program,":/MLDemos/shaders/smoothTransparent.vsh",":/MLDemos/shaders/smoothTransparent.fsh");
    program->bindAttributeLocation("vertex", 0);
    program->bindAttributeLocation("vertexNormal", 1);
    program->bindAttributeLocation("barycentric", 2);
    shaders["SmoothTransparent"] = program;

    //glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClearColor(1.f, 1.f, 1.f, 1.0f);
}


void GLWidget::generateObjects()
{
    if(!canvas) return;
    int xIndex = canvas->xIndex;
    int yIndex = canvas->yIndex;
    int zIndex = canvas->zIndex;
    int dim = canvas->data->GetDimCount();

    // we generate the canvas samples
    GLObject o;
    o.objectType = "Samples";
    FOR(i, canvas->data->GetCount())
    {
        fvec sample = canvas->data->GetSample(i);
        o.vertices.append(QVector3D(xIndex >= 0 && xIndex < dim ? sample[xIndex] : 0,
                                    yIndex >= 0 && yIndex < dim ? sample[yIndex] : 0,
                                    zIndex >= 0 && zIndex < dim ? sample[zIndex] : 0));
        o.colors.append(canvas->data->GetLabel(i));
    }
    FOR(i, objects.size())
    {
        if(objects[i].objectType == "Samples")
        {
            objects[i] = o;
            return;
        }
    }
    objects.push_back(o);
}

void GLWidget::DrawObject(GLObject &o)
{
    if(o.objectType.contains("Samples")) DrawSamples(o);
    else if(o.objectType.contains("Lines")) DrawLines(o);
    else if(o.objectType.contains("Surfaces")) DrawSurfaces(o);
}

void GLWidget::DrawSamples(GLObject &o)
{
    QGLShaderProgram *program = shaders["Samples"];
    program->bind();

    program->setUniformValue("matrix", modelViewProjectionMatrix);
    program->setUniformValue("viewport", viewport);
    program->enableAttributeArray(0);
    program->enableAttributeArray(1);
    program->setAttributeArray(0, o.vertices.constData());
    program->setAttributeArray(1, o.colors.constData(),1);

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glEnable(GL_BLEND);
    glEnable(GL_ALPHA_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glPointSize(12);

    // we actually draw stuff!
    glDrawArrays(GL_POINTS,0,o.vertices.size());

    glPopAttrib();

    program->release();
}

void GLWidget::DrawLines(GLObject &o)
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glShadeModel (GL_FLAT);
    glDisable(GL_POINT_SPRITE);

    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_BLEND);
    glEnable(GL_ALPHA_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    QString style = o.style.toLower();
    if(style.contains("dotted")) glEnable(GL_LINE_STIPPLE);
    else glDisable(GL_LINE_STIPPLE);

    glLineWidth(1.f);
    glLineStipple (1, 0xFFFF);
    glColor3f(0,0,0);
    QStringList params = style.split(",");
    FOR(i, params.size())
    {
        if(params[i].contains("width"))
        {
            QStringList p = params[i].split(":");
            float val = p[1].toFloat();
            glLineWidth(val);
        }
        if(params[i].contains("color"))
        {
            QStringList p = params[i].split(":");
            float r = p[1].toFloat();
            float g = p[2].toFloat();
            float b = p[3].toFloat();
            glColor3f(r,g,b);
        }
        if(params[i].contains("pattern"))
        {
            QStringList p = params[i].split(":");
            int factor = p[1].toInt();
            int pattern = p[2].toInt();
            glLineStipple (factor, pattern);
        }
    }
    if(style.contains("linestrip")) glBegin(GL_LINE_STRIP);
    else glBegin(GL_LINES);
    FOR(i, o.vertices.size())
    {
        glVertex3f(o.vertices.at(i).x(),o.vertices.at(i).y(),o.vertices.at(i).z());
    }
    glEnd();
    glPopAttrib();
}


void RecomputeBarycentric(GLObject &o)
{
    o.barycentric.resize(o.vertices.size());
    if(o.objectType.contains("quadstrip")){}
    else if(o.objectType.contains("quads"))
    {
        FOR(i, o.vertices.size()/4)
        {
            int index = i*4;
            o.barycentric[index  ] = QVector4D(1,1,1,1);
            o.barycentric[index+1] = QVector4D(0,1,1,1);
            o.barycentric[index+2] = QVector4D(0,0,1,1);
            o.barycentric[index+3] = QVector4D(1,0,1,1);
        }
    }
    else
    {
        FOR(i, o.vertices.size()/3)
        {
            int index = i*3;
            o.barycentric[index  ] = QVector4D(1,0,0,1);
            o.barycentric[index+1] = QVector4D(0,1,0,1);
            o.barycentric[index+2] = QVector4D(0,0,1,1);
        }
    }
}

void RecomputeNormals(GLObject &o)
{
    // we need to go through all the faces
    o.normals.resize(o.vertices.size());
    if(o.objectType.contains("quadstrip")) {}
    else if(o.objectType.contains("quads"))
    {
        qDebug() << "quads" << o.vertices.size();
        QVector3D a,b;
        QVector<QVector3D> faceNormals(o.vertices.size()/4);
        qDebug() << "generating face normals";
        FOR(i, o.vertices.size()/4) // for each quad
        {
            int index = i*4;
            bool bInvert = false;
            if(o.vertices[index] != o.vertices[index+1])
            {
                a = o.vertices[index+1] - o.vertices[index];
                if(o.vertices[index] != o.vertices[index+3])
                    b = o.vertices[index+3] - o.vertices[index];
                else
                {
                    b = o.vertices[index+2] - o.vertices[index];
                    bInvert = true;
                }
            }
            else
            {
                a = o.vertices[index+2] - o.vertices[index];
                b = o.vertices[index+3] - o.vertices[index];
                bInvert = true;
            }
            faceNormals[i] = bInvert ? QVector3D::crossProduct(a,b) : QVector3D::crossProduct(b,a);
            faceNormals[i].normalize();
        }
        qDebug() << "gathering twins information";
        // now that whe have these we need to know which vertices overlap
        vector<int> twins(o.vertices.size());
        FOR(i, twins.size()) twins[i] = i;
        FOR(i, twins.size())
        {
            if(twins[i] != i) // we have already computed its twin
            {
                o.normals[i] = o.normals[twins[i]];
                continue;
            }
            o.normals[i] = faceNormals[i/4];
            for(int j=i+1;j<twins.size();j++)
            {
                if(o.vertices[i] == o.vertices[j])
                {
                    twins[j] = i;
                    o.normals[i] += faceNormals[j/4];
                }
            }
            o.normals[i].normalize();
        }
        qDebug() << "done.";
    }
    else
    {
        QVector<QVector3D> faceNormals(o.vertices.size()/3);
        FOR(i, o.vertices.size()/3) // for each triangle
        {
            int index = i*3;
            QVector3D a = o.vertices[index+1] - o.vertices[index];
            QVector3D b = o.vertices[index+2] - o.vertices[index];
            faceNormals[i] = QVector3D::crossProduct(b,a);
            faceNormals[i].normalize();
        }
        // now that whe have these we need to know which vertices overlap
        vector<int> twins(o.vertices.size());
        FOR(i, twins.size()) twins[i] = i;
        FOR(i, twins.size())
        {
            o.normals[i] = faceNormals[i/3];
            if(twins[i] != i) // we have already passed by here
            {
                o.normals[i] = o.normals[twins[i]];
                continue;
            }
            for(int j=i+1;j<twins.size();j++)
            {
                if(o.vertices[i] == o.vertices[j])
                {
                    twins[j] = i;
                    o.normals[i] += faceNormals[j/3];
                }
            }
            o.normals[i].normalize();
        }
    }
}

void GLWidget::DrawSurfaces(GLObject &o)
{
    QString style = o.style.toLower();
    QStringList params = style.split(",");
    if(o.normals.size() != o.vertices.size()) // we need to recompute the normals
    {
        qDebug() << "recomputing normals";
        RecomputeNormals(o);
        qDebug() << "Done.";
    }
    if(o.barycentric.size() != o.vertices.size())
    {
        qDebug() << "recomputing barycentrics";
        RecomputeBarycentric(o);
        qDebug() << "Done.";
    }

    float alpha = 1.f;
    if(style.contains("transparent")) alpha = 0.5f;
    QVector4D material(0.5,0.5,0.5,alpha);
    FOR(i, params.size())
    {
        if(params[i].contains("color"))
        {
            QStringList p = params[i].split(":");
            float r = p[1].toFloat();
            float g = p[2].toFloat();
            float b = p[3].toFloat();
            if(p.size() > 4) alpha = p[4].toFloat();
            material = QVector4D(r,g,b,alpha);
        }
    }

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glEnable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    if(style.contains("smooth")) glShadeModel(GL_SMOOTH);
    else glShadeModel (GL_FLAT);

    if(style.contains("doublesided")) glCullFace(GL_NONE);
    else glCullFace(GL_BACK);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glEnable(GL_ALPHA_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_COLOR_MATERIAL);
    //glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE | GL_SPECULAR);

    // setup lights
    FOR(i, lights.size())
    {
        glEnable(GL_LIGHT0+i);
        glLightfv(GL_LIGHT0+i, GL_AMBIENT, lights[i].ambientLight);
        glLightfv(GL_LIGHT0+i, GL_DIFFUSE, lights[i].diffuseLight);
        glLightfv(GL_LIGHT0+i, GL_SPECULAR, lights[i].specularLight);
        glLightfv(GL_LIGHT0+i, GL_POSITION, lights[i].position);
    }

    if(style.contains("smooth"))
    {
        QVector4D surfaceStyle(1,0,0,0);
        if(style.contains("wireframe")) surfaceStyle.setY(1);
        if(style.contains("isolines")) surfaceStyle.setY(2);
        if(style.contains("blurry")) surfaceStyle.setZ(1);
        QMatrix4x4 mvMatrix = modelViewMatrix * o.model;
        QMatrix4x4 mvpMatrix = perspectiveMatrix * mvMatrix;
        QMatrix3x3 normMatrix = mvMatrix.normalMatrix();
        QVector4D l0pos = modelViewMatrix * QVector4D(lights[0].position[0],
                                                  lights[0].position[1],
                                                  lights[0].position[2],
                                                  lights[0].position[3]);
        QVector4D l1pos = modelViewMatrix * QVector4D(lights[1].position[0],
                                                  lights[1].position[1],
                                                  lights[1].position[2],
                                                  lights[1].position[3]);
        QVector4D l2pos = modelViewMatrix * QVector4D(lights[2].position[0],
                                                  lights[2].position[1],
                                                  lights[2].position[2],
                                                  lights[2].position[3]);
        QGLShaderProgram *program = shaders["SmoothTransparent"];
        program->bind();
        program->setUniformValue("mvMatrix", mvMatrix);
        program->setUniformValue("mvpMatrix", mvpMatrix);
        program->setUniformValue("normalMatrix", normMatrix);
        program->setUniformValue("viewport", viewport);
        program->setUniformValue("materialColor", material);
        program->setUniformValue("surfaceStyle", surfaceStyle);
        program->setUniformValueArray("lights[0].position", &l0pos,1);
        program->setUniformValueArray("lights[0].ambient", lights[0].ambientLight,1,4);
        program->setUniformValueArray("lights[0].diffuse", lights[0].diffuseLight,1,4);
        program->setUniformValueArray("lights[0].specular", lights[0].specularLight,1,4);
        program->setUniformValueArray("lights[1].position", &l1pos,1);
        program->setUniformValueArray("lights[1].ambient", lights[1].ambientLight,1,4);
        program->setUniformValueArray("lights[1].diffuse", lights[1].diffuseLight,1,4);
        program->setUniformValueArray("lights[1].specular", lights[1].specularLight,1,4);
        program->setUniformValueArray("lights[2].position", &l2pos,1);
        program->setUniformValueArray("lights[2].ambient", lights[2].ambientLight,1,4);
        program->setUniformValueArray("lights[2].diffuse", lights[2].diffuseLight,1,4);
        program->setUniformValueArray("lights[2].specular", lights[2].specularLight,1,4);
        program->enableAttributeArray(0);
        program->enableAttributeArray(1);
        program->enableAttributeArray(2);
        program->setAttributeArray(0, o.vertices.constData());
        program->setAttributeArray(1, o.normals.constData());
        program->setAttributeArray(2, o.barycentric.constData());

        GLenum mode = GL_TRIANGLES;
        if(o.objectType.contains("quadstrip")) mode = GL_QUAD_STRIP;
        else if(o.objectType.contains("quads")) mode = GL_QUADS;
        glDrawArrays(mode, 0, o.vertices.size());
        program->release();
    }
    else
    {
        GLfloat whiteSpecularMaterial[] = {1.0, 1.0, 1.0};
        GLfloat shininess[] = {128}; //set the shininess of the
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, whiteSpecularMaterial);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shininess);

        glColor4f(material.x(), material.y(), material.z(), material.w());
        float model[16];
        FOR(i, 16) model[i] = o.model.constData()[i];
        glPushMatrix();
        glMultMatrixf(model);
        if(o.objectType.contains("quadstrip")) glBegin(GL_QUAD_STRIP);
        else if(o.objectType.contains("quads")) glBegin(GL_QUADS);
        else glBegin(GL_TRIANGLES);
        FOR(i, o.vertices.size())
        {
            glNormal3f(o.normals.at(i).x(),o.normals.at(i).y(),o.normals.at(i).z());
            //glNormal3f(o.vertices.at(i).x(),o.vertices.at(i).y(),o.vertices.at(i).z());
            glVertex3f(o.vertices.at(i).x(),o.vertices.at(i).y(),o.vertices.at(i).z());
        }
        glEnd();
        glPopMatrix();
    }
    glPopAttrib();
}

void DrawAxes(float zoomFactor)
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glShadeModel (GL_FLAT);
    glDisable(GL_POINT_SPRITE);

    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDisable(GL_LINE_STIPPLE);
    glLineWidth(1.f);
    glLineStipple (1, 0xFFFF);
    glColor3f(0,0,0);
    float rad = 1.f*(zoomFactor/ZoomZero);
    glBegin(GL_LINES);
    glVertex3f(-rad, 0, 0);
    glVertex3f(+rad, 0, 0);
    glVertex3f(0, -rad, 0);
    glVertex3f(0, +rad, 0);
    glVertex3f(0, 0, -rad);
    glVertex3f(0, 0, +rad);
    glEnd();

    glEnable(GL_LINE_STIPPLE);
    glLineWidth(0.5f);
    glLineStipple (3, 0xAAAA);
    int steps = 10;
    FOR(i, steps-1)
    {
        glBegin(GL_LINES);
        glVertex3f(-rad, rad*(i+1)/steps, 0);
        glVertex3f(+rad, rad*(i+1)/steps, 0);
        glVertex3f(rad*(i+1)/steps, -rad, 0);
        glVertex3f(rad*(i+1)/steps, +rad, 0);
        glVertex3f(-rad, 0, rad*(i+1)/steps);
        glVertex3f(+rad, 0, rad*(i+1)/steps);
        glVertex3f(0, -rad, rad*(i+1)/steps);
        glVertex3f(0, +rad, rad*(i+1)/steps);
        glVertex3f(0, rad*(i+1)/steps, -rad);
        glVertex3f(0, rad*(i+1)/steps, +rad);
        glVertex3f(rad*(i+1)/steps, 0, -rad);
        glVertex3f(rad*(i+1)/steps, 0, +rad);

        glVertex3f(-rad, -rad*(i+1)/steps, 0);
        glVertex3f(+rad, -rad*(i+1)/steps, 0);
        glVertex3f(-rad*(i+1)/steps, -rad, 0);
        glVertex3f(-rad*(i+1)/steps, +rad, 0);
        glVertex3f(-rad, 0, -rad*(i+1)/steps);
        glVertex3f(+rad, 0, -rad*(i+1)/steps);
        glVertex3f(0, -rad, -rad*(i+1)/steps);
        glVertex3f(0, +rad, -rad*(i+1)/steps);
        glVertex3f(0, -rad*(i+1)/steps, -rad);
        glVertex3f(0, -rad*(i+1)/steps, +rad);
        glVertex3f(-rad*(i+1)/steps, 0, -rad);
        glVertex3f(-rad*(i+1)/steps, 0, +rad);
        glEnd();
    }
    glPopAttrib();
}

float angle = 0;
void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(!canvas) return;
    generateObjects();

    glPushMatrix();

    // we do the camera rotation once for 'standard' opengl geometry (lines etc)
    glRotated(xRot / 16.0, 1.0, 0.0, 0.0);
    glRotated(yRot / 16.0, 0.0, 1.0, 0.0);
    glRotated(zRot / 16.0, 0.0, 0.0, 1.0);
    glTranslatef(xPos, yPos, zPos);

    // and once for shader-based geometry
    modelViewMatrix.setToIdentity();
    modelViewMatrix.rotate(xRot / 16.0, 1.0, 0.0, 0.0);
    modelViewMatrix.rotate(yRot / 16.0, 0.0, 1.0, 0.0);
    modelViewMatrix.rotate(zRot / 16.0, 0.0, 0.0, 1.0);
    modelViewMatrix.translate(xPos, yPos, zPos);

    lights[0].position[0] = 2*sin(angle*0.1f);
    lights[0].position[1] = 0;
    lights[0].position[2] = 2*cos(angle*0.1f);
    lights[1].position[0] = 2*sin(angle*0.13f);
    lights[1].position[1] = -2*cos(angle*0.13f);
    lights[1].position[2] = 2*sin(angle*0.13f);
    angle += 0.1f;

    modelViewProjectionMatrix = perspectiveMatrix*modelViewMatrix;
    normalMatrix = modelViewMatrix.normalMatrix();

    int xIndex = canvas->xIndex;
    int yIndex = canvas->yIndex;
    int zIndex = canvas->zIndex;
    int dim = canvas->data->GetDimCount();

    // Here we draw the axes lines
    if(canvas->bDisplayGrid)
    {
        DrawAxes(zoomFactor);
    }
    // we draw the lights
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glShadeModel (GL_FLAT);
    glDisable(GL_POINT_SPRITE);
    FOR(i, lights.size())
    {
        glPushMatrix();
        glTranslatef(lights[i].position[0],lights[i].position[1],lights[i].position[2]);
        glColor3f(0,0,1.0);
        DrawStandardSphere(0.05f);
        glPopMatrix();
    }
    glPopAttrib();

    FOR(i, objects.size())
    {
        if(objects[i].objectType == "Samples")
        {
            // here we draw the canvas samples
            if(canvas->bDisplaySamples || (canvas->sampleColors.size() == canvas->data->GetCount() && canvas->bDisplayLearned))
            {
                DrawObject(objects[i]);
            }
        }
    }
    FOR(i, objects.size())
    {
        if(objects[i].objectType == "Samples") continue;
        DrawObject(objects[i]);
    }

    if(canvas->bDisplayInfo || canvas->bDisplayLearned)
    {
        // we begin with the sample sprites
        glBindTexture(GL_TEXTURE_2D, GLWidget::textureNames[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        vector< pair<float, int> > list;

        // get gl matrices
        GLdouble modelMatrix[16];
        GLdouble projectionMatrix[16];
        glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
        glGetDoublev(GL_PROJECTION_MATRIX, projectionMatrix);
        MathLib::Matrix4 model;
        FOR(j, 4)
        {
            FOR(i, 4)
            {
                model(i,j) = modelMatrix[j*4 + i];
            }
        }

        FOR(i, drawSampleLists.size())
        {
            if(drawSampleListCenters.count(drawSampleLists[i]))
            {
                fvec mean = drawSampleListCenters[drawSampleLists[i]];
                MathLib::Vector3 v(mean[0],mean[1],mean[2]);
                MathLib::Vector3 vCam;
                model.Transform(v, vCam);
                list.push_back(make_pair(vCam(2), i));
            }
            else list.push_back(make_pair(-FLT_MAX, i));
        }
        std::sort(list.begin(), list.end());

        FOR(i, list.size())
        {
            glCallList(drawSampleLists[list[i].second]);
        }

        FOR(i, drawLists.size())
        {
            glCallList(drawLists[i]);
        }
    }
    glPopMatrix();
}

void GLWidget::zoom(int delta)
{
    if(delta > 0)
    {
        zoomFactor *= 0.9;
    }
    else
    {
        zoomFactor *= 1.1;
    }
    resizeGL(width, height);
    repaint();
}

void GLWidget::resizeGL(int width, int height)
{
    this->width = width;
    this->height = height;

    int side = qMin(width, height);
    glViewport(0,0,width, height);
    viewport = QVector4D(0,0,width,height);
    //    glViewport((width - side) / 2, (height - side) / 2, side, side);
    float ratio = width/(float)height;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    double left = -ratio, right = ratio, bottom = -1.0, top = 1.0;
    double zNear = 4.0;
    double zFar = 60.0;
    glFrustum(left*zoomFactor, right*zoomFactor,
              bottom*zoomFactor, top*zoomFactor, zNear, zFar);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslated(0.0, 0.0, -40.0);

    perspectiveMatrix.setToIdentity();
    perspectiveMatrix.frustum(left*zoomFactor, right*zoomFactor,
                              bottom*zoomFactor, top*zoomFactor, zNear, zFar);
    perspectiveMatrix.translate(0,0,-40);
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    lastPos = event->pos();
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();

    if (event->modifiers() & Qt::ShiftModifier)
    {
        if(event->buttons() & Qt::LeftButton)
        {
            setXPosition(xPos + -dy/64.f*sin(yRot));
            setZPosition(zPos + -dx/64.f*(-cos(yRot)));
        }
        else if(event->buttons() & Qt::RightButton)
        {
            setYPosition(yPos - dy/64.f);
        }
    }
    else
    {
        if (event->buttons() & Qt::LeftButton) {
            setXRotation(xRot + 8 * dy);
            setYRotation(yRot + 8 * dx);
        } else if (event->buttons() & Qt::RightButton) {
            //setXRotation(xRot + 8 * dy);
            //setZRotation(zRot + 8 * dx);
        }
    }
    lastPos = event->pos();
}

void GLWidget::wheelEvent(QWheelEvent *event)
{
    zoom(event->delta());
}

void GLWidget::resizeEvent(QResizeEvent *event)
{
    resizeGL(event->size().width(), event->size().height());
}

void GLWidget::normalizeAngle(int *angle)
{
    while (*angle < 0)
        *angle += 360 * 16;
    while (*angle > 360 * 16)
        *angle -= 360 * 16;
}

GLuint *GLWidget::textureNames = new GLuint[textureCount];
unsigned char **GLWidget::textureData = 0;

void GLWidget::LoadShader(QGLShaderProgram **program_, QString vshader, QString fshader)
{
    QGLShaderProgram *program = *program_;
    if(program)
    {
        program->release();
        QList<QGLShader*> shaders = program->shaders();
        FOR(i, shaders.size())
        {
            delete shaders.at(i);
        }
        program->removeAllShaders();
    }
    else program = new QGLShaderProgram;
    QGLShader *VertexShader = NULL, *FragmentShader = NULL;

    // load and compile vertex shader
    QFileInfo vsh(vshader);
    if(vsh.exists())
    {
        VertexShader = new QGLShader(QGLShader::Vertex);
        if(VertexShader->compileSourceFile(vshader))
            program->addShader(VertexShader);
        else qWarning() << "Vertex Shader Error" << VertexShader->log();
    }
    else qWarning() << "Vertex Shader source file " << vshader << " not found.";

    // load and compile fragment shader
    QFileInfo fsh(fshader);
    if(fsh.exists())
    {
        FragmentShader = new QGLShader(QGLShader::Fragment);
        if(FragmentShader->compileSourceFile(fshader))
            program->addShader(FragmentShader);
        else qWarning() << "Fragment Shader Error" << FragmentShader->log();
    }
    else qWarning() << "Fragment Shader source file " << fshader << " not found.";

    if(!program->link())
    {
        qWarning() << "Shader Program Linker Error" << program->log();
    }
    else program->bind();
    program->release();
    *program_ = program;
}


void GLWidget::setXPosition(float pos)
{
    if (pos != xPos) {
        xPos = pos;
        emit xPositionChanged(pos);
        updateGL();
    }
}

void GLWidget::setYPosition(float pos)
{
    if (pos != yPos) {
        yPos = pos;
        emit yPositionChanged(pos);
        updateGL();
    }
}

void GLWidget::setZPosition(float pos)
{
    if (pos != zPos) {
        zPos = pos;
        emit zPositionChanged(pos);
        updateGL();
    }
}

void GLWidget::setXRotation(int angle)
{
    normalizeAngle(&angle);
    if (angle != xRot) {
        xRot = angle;
        emit xRotationChanged(angle);
        updateGL();
    }
}

void GLWidget::setYRotation(int angle)
{
    normalizeAngle(&angle);
    if (angle != yRot) {
        yRot = angle;
        emit yRotationChanged(angle);
        updateGL();
    }
}

void GLWidget::setZRotation(int angle)
{
    normalizeAngle(&angle);
    if (angle != zRot) {
        zRot = angle;
        emit zRotationChanged(angle);
        updateGL();
    }
}

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
