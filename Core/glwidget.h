#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <canvas.h>
#include <vector>
#include <QGLWidget>

class GLWidget : public QGLWidget
{
    Q_OBJECT

public:
    GLWidget(Canvas *canvas, QWidget *parent = 0);
    ~GLWidget();
    void clearLists();

public slots:
    void setXRotation(int angle);
    void setYRotation(int angle);
    void setZRotation(int angle);
    void setXPosition(float pos);
    void setYPosition(float pos);
    void setZPosition(float pos);

signals:
    void xRotationChanged(int angle);
    void yRotationChanged(int angle);
    void zRotationChanged(int angle);
    void xPositionChanged(float pos);
    void yPositionChanged(float pos);
    void zPositionChanged(float pos);

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);
    void zoom(int delta);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void resizeEvent(QResizeEvent *);

private slots:
    void advanceGears();

private:
    GLuint makeGear(const GLfloat *reflectance, GLdouble innerRadius,
                    GLdouble outerRadius, GLdouble thickness,
                    GLdouble toothSize, GLint toothCount);
    void normalizeAngle(int *angle);

    int xRot, yRot, zRot;
    float xPos, yPos, zPos;

    Canvas *canvas;
    float zoomFactor;
    int width, height;

    QPoint lastPos;

public:
    std::vector<GLuint> drawSampleLists;
    std::vector<GLuint> drawLists;
    std::map<GLuint, fvec> drawSampleListCenters;

    static const GLint texWidth = 128;
    static const GLint texHeight = 128;
    static const float texHalfWidth = 64.0f;
    static const float texHalfHeight = 64.0f;
    static const float texRadius = texWidth*0.9;
    static const int textureCount = 2; // 0: samples, 1: wide circle
    static GLuint *textureNames;
    static unsigned char **textureData;

    static inline void glSample(fvec sample, QColor c, int xIndex, int yIndex, int zIndex)
    {
        glColor3f(c.redF(), c.greenF(), c.blueF());
        float sX=0,sY=0,sZ=0;
        if(xIndex >= 0) sX = sample[xIndex];
        if(yIndex >= 0) sY = sample[yIndex];
        if(zIndex >= 0) sZ = sample[zIndex];
        glVertex3f(sX,sY,sZ);
    }

    static inline void glLine(fvec p1, fvec p2, int xIndex=0, int yIndex=1, int zIndex=2)
    {
        glVertex3f(p1[xIndex], p1[yIndex], zIndex >= 0 ? p1[zIndex] : 0.f);
        glVertex3f(p2[xIndex], p2[yIndex], zIndex >= 0 ? p2[zIndex] : 0.f);
    }
};

#endif
