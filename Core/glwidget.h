#ifndef GLWIDGET_H
#define GLWIDGET_H

#include "glUtils.h"
#include <canvas.h>
#include <vector>
#include <QtOpenGL>
#include <QtOpenGLWidgets/QOpenGLWidget>
#include <QMatrix4x4>

class QMatrix;
class QMatrix4x4;
class QOpenGLShaderProgram;
class QOpenGLShader;

class GLWidget : public QOpenGLWidget
{
    Q_OBJECT

#ifdef WIN32
static QOpenGLFunctions glf;
#endif

public:
    GLWidget(Canvas *canvas, QWidget *parent = 0);
    ~GLWidget();
    void InitializeGL();
    void AddObject(GLObject &o);
    void SetObject(int index, GLObject &o);
    void clearLists();
    void killObjects();
    void generateObjects();
    void FixSurfaces(GLObject &o);
    void DrawObject(const GLObject &o) const ;
    void DrawSamples(const GLObject &o) const ;
    void DrawParticles(const GLObject &o) const;
    void DrawLines(const GLObject &o) const;
    void DrawSurfaces_old(const GLObject &o) const;
    void DrawSurfaces(const GLObject &o) const;
    void LoadShader(QOpenGLShaderProgram **program_, QString vshader, QString fshader);

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
    void GenTextures();
    void LoadShaders();
    void InitLights();
    void initializeGL();
    void initializeGLAlt();
    void paintGL();
    void resizeGL(int width, int height);
    void zoom(int delta);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void resizeEvent(QResizeEvent *);
    void timerEvent(QTimerEvent *);

private:
    void normalizeAngle(int &angle);
    void RenderFBO(QOpenGLFramebufferObject *fbo, QOpenGLShaderProgram *program);
    void RenderShadowMap(QOpenGLFramebufferObject *fbo, GLLight light, std::vector<GLObject> objects);

    QMatrix4x4 perspectiveMatrix;
    QMatrix4x4 modelViewMatrix;
    QMatrix4x4 modelViewProjectionMatrix;
    QMatrix3x3 normalMatrix;
    QVector4D viewport;
    int xRot, yRot, zRot;
    float xPos, yPos, zPos;

    float zoomFactor;
    int width, height;

    QPoint lastPos;

    std::map<QString, QOpenGLShaderProgram*> shaders;

public:
    QMutex *mutex;
    Canvas *canvas;
    std::vector<GLuint> drawSampleLists;
    std::vector<GLuint> drawLists;
    std::map<GLuint, fvec> drawSampleListCenters;
    std::vector<GLObject> objects;
    std::vector<bool> objectAlive;
    ivec killList;
    std::vector<GLLight> lights;
    bool bDisplaySamples, bDisplayLines, bDisplaySurfaces, bDisplayTransparency, bDisplayBlurry;
    bool bRotateCamera;
    bool bHiDPI;

    static const GLint texWidth = 128;
    static const GLint texHeight = 128;
    static const constexpr float texHalfWidth = 64.0f;
    static const constexpr float texHalfHeight = 64.0f;
    static const constexpr float texRadius = texWidth*0.9;
    static const int textureCount = 2; // 0: samples, 1: wide circle
    static GLuint *textureNames;
    static std::vector< std::vector<unsigned char> > textureData;

    QOpenGLFramebufferObject *render_fbo;
    QOpenGLFramebufferObject *texture_fbo;
    QOpenGLFramebufferObject *light_fbo;
};

#endif
