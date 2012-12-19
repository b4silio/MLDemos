#include <QtGui>
#include <QtOpenGL>
#include <QDebug>

#include <math.h>
#include <mymaths.h>
#include "glwidget.h"
#include <MathLib/MathLib.h>

using namespace std;

GLWidget::GLWidget(Canvas *canvas, QWidget *parent)
    : QGLWidget(parent), canvas(canvas), zoomFactor(0.049f)
{
    width = 800;
    height = 600;
    xRot = yRot = zRot = 0;
    xPos = yPos = zPos = 0.f;

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(advanceGears()));
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

    glEnable( GL_POINT_SMOOTH );

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
            float rad = 1.f*(zoomFactor/0.04f);
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

        // here we draw the canvas points
        if(canvas->data->GetCount())
        {
            if(canvas->bDisplaySamples || (canvas->sampleColors.size() == canvas->data->GetCount() && canvas->bDisplayLearned))
            {
                glPushAttrib(GL_ALL_ATTRIB_BITS);
                glDisable(GL_LIGHTING);
                glDepthMask(GL_FALSE);

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
            float z=0;
            // get gl matrices
            GLdouble modelMatrix[16];
            glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
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
            fflush(stdout);
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
}

void GLWidget::resizeGL(int width, int height)
{
    this->width = width;
    this->height = height;

    int side = qMin(width, height);
    glViewport(0,0,width, height);
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

void GLWidget::advanceGears()
{
    updateGL();
}

GLuint GLWidget::makeGear(const GLfloat *reflectance, GLdouble innerRadius,
                          GLdouble outerRadius, GLdouble thickness,
                          GLdouble toothSize, GLint toothCount)
{
    const double Pi = 3.14159265358979323846;

    GLuint list = glGenLists(1);
    glNewList(list, GL_COMPILE);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, reflectance);

    GLdouble r0 = innerRadius;
    GLdouble r1 = outerRadius - toothSize / 2.0;
    GLdouble r2 = outerRadius + toothSize / 2.0;
    GLdouble delta = (2.0 * Pi / toothCount) / 4.0;
    GLdouble z = thickness / 2.0;
    int i, j;

    glShadeModel(GL_FLAT);

    for (i = 0; i < 2; ++i) {
        GLdouble sign = (i == 0) ? +1.0 : -1.0;

        glNormal3d(0.0, 0.0, sign);

        glBegin(GL_QUAD_STRIP);
        for (j = 0; j <= toothCount; ++j) {
            GLdouble angle = 2.0 * Pi * j / toothCount;
            glVertex3d(r0 * cos(angle), r0 * sin(angle), sign * z);
            glVertex3d(r1 * cos(angle), r1 * sin(angle), sign * z);
            glVertex3d(r0 * cos(angle), r0 * sin(angle), sign * z);
            glVertex3d(r1 * cos(angle + 3 * delta), r1 * sin(angle + 3 * delta),
                       sign * z);
        }
        glEnd();

        glBegin(GL_QUADS);
        for (j = 0; j < toothCount; ++j) {
            GLdouble angle = 2.0 * Pi * j / toothCount;
            glVertex3d(r1 * cos(angle), r1 * sin(angle), sign * z);
            glVertex3d(r2 * cos(angle + delta), r2 * sin(angle + delta),
                       sign * z);
            glVertex3d(r2 * cos(angle + 2 * delta), r2 * sin(angle + 2 * delta),
                       sign * z);
            glVertex3d(r1 * cos(angle + 3 * delta), r1 * sin(angle + 3 * delta),
                       sign * z);
        }
        glEnd();
    }

    glBegin(GL_QUAD_STRIP);
    for (i = 0; i < toothCount; ++i) {
        for (j = 0; j < 2; ++j) {
            GLdouble angle = 2.0 * Pi * (i + (j / 2.0)) / toothCount;
            GLdouble s1 = r1;
            GLdouble s2 = r2;
            if (j == 1)
                qSwap(s1, s2);

            glNormal3d(cos(angle), sin(angle), 0.0);
            glVertex3d(s1 * cos(angle), s1 * sin(angle), +z);
            glVertex3d(s1 * cos(angle), s1 * sin(angle), -z);

            glNormal3d(s2 * sin(angle + delta) - s1 * sin(angle),
                       s1 * cos(angle) - s2 * cos(angle + delta), 0.0);
            glVertex3d(s2 * cos(angle + delta), s2 * sin(angle + delta), +z);
            glVertex3d(s2 * cos(angle + delta), s2 * sin(angle + delta), -z);
        }
    }
    glVertex3d(r1, 0.0, +z);
    glVertex3d(r1, 0.0, -z);
    glEnd();

    glShadeModel(GL_SMOOTH);

    glBegin(GL_QUAD_STRIP);
    for (i = 0; i <= toothCount; ++i) {
        GLdouble angle = i * 2.0 * Pi / toothCount;
        glNormal3d(-cos(angle), -sin(angle), 0.0);
        glVertex3d(r0 * cos(angle), r0 * sin(angle), +z);
        glVertex3d(r0 * cos(angle), r0 * sin(angle), -z);
    }
    glEnd();

    glEndList();

    return list;
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
