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

    QTimer *timer = new QTimer(this);
    timer->start(20);
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

    // We initialize the shaders
    QGLShaderProgram *program=0;
    LoadShader(&program,":/MLDemos/shaders/drawSamples.vsh",":/MLDemos/shaders/drawSamples.fsh");
    program->bindAttributeLocation("vertex", 0);
    program->bindAttributeLocation("color", 1);
    shaders["Samples"] = program;
    program = 0;

    //glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClearColor(1.f, 1.f, 1.f, 1.0f);
}

void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPushMatrix();
    glDisable(GL_LIGHTING);
    glRotated(xRot / 16.0, 1.0, 0.0, 0.0);
    glRotated(yRot / 16.0, 0.0, 1.0, 0.0);
    glRotated(zRot / 16.0, 0.0, 0.0, 1.0);
    glTranslatef(xPos, yPos, zPos);

    /*
    program->setUniformValue("matrix", modelMatrix);
    program->enableAttributeArray(0);
    program->enableAttributeArray(1);
    program->setAttributeArray(0, vertices.constData());
    program->setAttributeArray(1, texCoords.constData());
    */
    if(canvas)
    {
        int xIndex = canvas->xIndex;
        int yIndex = canvas->yIndex;
        int zIndex = canvas->zIndex;
        int dim = canvas->data->GetDimCount();

        // Here we draw the axes lines
        if(canvas->bDisplayGrid)
        {
            glPushAttrib(GL_ALL_ATTRIB_BITS);
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

        modelMatrix = perspectiveMatrix;
        modelMatrix.rotate(xRot / 16.0, 1.0, 0.0, 0.0);
        modelMatrix.rotate(yRot / 16.0, 0.0, 1.0, 0.0);
        modelMatrix.rotate(zRot / 16.0, 0.0, 0.0, 1.0);
        modelMatrix.translate(xPos, yPos, zPos);

        QVector<QVector3D> vertices;
        QVector<GLfloat> colors;
        srand(1);
        FOR(i, canvas->data->GetCount())
        {
            fvec sample = canvas->data->GetSample(i);
            vertices.append(QVector3D(xIndex >= 0 ? sample[xIndex] : 0,
                                      yIndex >= 0 ? sample[yIndex] : 0,
                                      zIndex >= 0 ? sample[zIndex] : 0));
            colors.append(canvas->data->GetLabel(i));
        }

        // here we draw the canvas points
        if(canvas->data->GetCount())
        {
            QGLShaderProgram *program = shaders["Samples"];
            program->bind();
            program->setUniformValue("matrix", modelMatrix);
            program->setUniformValue("viewport", viewport);
            program->enableAttributeArray(0);
            program->enableAttributeArray(1);
            program->setAttributeArray(0, vertices.constData());
            program->setAttributeArray(1, colors.constData(),1);

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
            glDrawArrays(GL_POINTS,0,vertices.size());

            glPopAttrib();

            program->release();

            if(canvas->bDisplaySamples || (canvas->sampleColors.size() == canvas->data->GetCount() && canvas->bDisplayLearned))
            {
                glPushAttrib(GL_ALL_ATTRIB_BITS);
                glDisable(GL_LIGHTING);
                glDisable(GL_TEXTURE_2D);
                glDepthMask(GL_TRUE);

                /*
                FOR(i, canvas->data->GetCount())
                {
                    fvec sample = canvas->data->GetSample(i);
                    int label = canvas->data->GetLabel(i);
                    QColor c = SampleColor[label%SampleColorCnt];
                    if(i < canvas->sampleColors.size() && canvas->bDisplayLearned) c = canvas->sampleColors[i];
                    float sX=0,sY=0,sZ=0;
                    if(xIndex >= 0) sX = sample[xIndex];
                    if(yIndex >= 0) sY = sample[yIndex];
                    if(zIndex >= 0) sZ = sample[zIndex];
                    float side = 0.008f*(zoomFactor/ZoomZero);

                    glPushMatrix();

                    glTranslatef(sX, sY, sZ);

                    //glEnable(GL_LINE_SMOOTH);
                    //glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
                    glEnable (GL_BLEND);                // Enable Blending
                    // Set The Blend Mode
                    glBlendFunc (GL_SRC_ALPHA ,GL_ONE_MINUS_SRC_ALPHA);
                    glEnable (GL_CULL_FACE);
                    glPolygonMode (GL_BACK, GL_LINE);   // Draw Backfacing Polygons As Wireframes
                    glLineWidth (4);                    // Set The Line Width
                    glCullFace (GL_FRONT);              // Don't Draw Any Front-Facing Polygons
                    glDepthFunc (GL_LEQUAL);            // Change The Depth Mode
                    glColor3f(0,0,0);                   // Set The Outline Color
                    DrawTessellatedSphere(side, 1);
                    glDepthFunc (GL_LESS);              // Reset The Depth-Testing Mode
                    glCullFace (GL_BACK);               // Reset The Face To Be Culled
                    glPolygonMode (GL_BACK, GL_FILL);       // Reset Back-Facing Polygon Drawing Mode
                    glDisable (GL_BLEND);

                    glColor3f(c.redF(), c.greenF(), c.blueF());
                    DrawTessellatedSphere(side, 1);

                    glPopMatrix();
                }
                /**/

                /*
                glDisable(GL_LIGHTING);
                //glDepthMask(GL_FALSE);

                glEnable(GL_TEXTURE_2D);
                glEnable(GL_POINT_SPRITE);
                glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
                glBindTexture(GL_TEXTURE_2D, textureNames[0]);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                glPointSize(12.0); // 12 pixel dot!
                glBegin(GL_POINTS); // Render points.
                FOR(i, canvas->data->GetCount())
                {
                    fvec sample = canvas->data->GetSample(i);
                    int label = canvas->data->GetLabel(i);
                    QColor c = SampleColor[label%SampleColorCnt];
                    if(i < canvas->sampleColors.size() && canvas->bDisplayLearned) c = canvas->sampleColors[i];
                    glSample(sample, c, xIndex, yIndex, zIndex);
                }
                glDepthMask(GL_TRUE);
                glEnd();
                /**/
                glPopAttrib();
            }
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
