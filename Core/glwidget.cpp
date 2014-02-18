#include <QtGui>
#include <QtOpenGL>
#include <QtOpenGL/QGLFunctions>
#include <QtOpenGL/QGLShaderProgram>
#include <QDebug>

#include <math.h>
#include <mymaths.h>
#include "glwidget.h"
#include <MathLib/MathLib.h>
#include "glUtils.h"

#define ZoomZero 0.0125f

using namespace std;

bool bDisplayShadows = false;
QMatrix4x4 lightMvMatrix;
QMatrix4x4 lightPMatrix;
QMatrix4x4 lightMvpMatrix;
QGLFramebufferObject *lightBlur_fbo;
QLabel *lightLabel = 0;

void checkGL()
{
    GLenum errors = glGetError();
    switch(errors)
    {
    case GL_INVALID_ENUM:
        qDebug() << "Function called with inappropriate enum.";
        break;
    case GL_INVALID_VALUE:
        qDebug() << "Function called with out of range numeric value.";
    case GL_INVALID_OPERATION:
        qDebug() << "Operation performed out of context, or not allowed in the current state";
        break;
    case GL_INVALID_FRAMEBUFFER_OPERATION:
        qDebug() << "Framebuffer object is not complete yet";
        break;
    case GL_OUT_OF_MEMORY:
        qDebug() << "Out of Memory";
        break;
    }
}

#ifdef WIN32
#define GLActiveTexture glf.glActiveTexture
QGLFunctions GLWidget::glf;
#else
    #define GLActiveTexture glActiveTexture
#endif

GLuint *GLWidget::textureNames = 0;

unsigned char **GLWidget::textureData = 0;

GLWidget::GLWidget(Canvas *canvas, QWidget *parent)
    : QGLWidget(QGLFormat(QGL::SampleBuffers|QGL::AlphaChannel),parent)
{
    this->canvas = canvas;
    this->zoomFactor = ZoomZero;
    bHiDPI = (qApp->devicePixelRatio() > 1);
    mutex = new QMutex();
    bDisplaySamples=bDisplayLines=bDisplaySurfaces=bDisplayTransparency=bDisplayBlurry=true;
    bRotateCamera=false;
    makeCurrent();
#ifdef WIN32
    glf.initializeGLFunctions(this->context());
#endif
    width = 800;
    height = 600;

    if (QGLFramebufferObject::hasOpenGLFramebufferBlit()) {
        QGLFramebufferObjectFormat format;
        format.setSamples(8);
        format.setAttachment(QGLFramebufferObject::CombinedDepthStencil);
        render_fbo = new QGLFramebufferObject(width, height, format);
        texture_fbo = new QGLFramebufferObject(width, height, format);
        light_fbo = new QGLFramebufferObject(width,height, format);
        lightBlur_fbo = new QGLFramebufferObject(width,height, format);
    } else {
        render_fbo = new QGLFramebufferObject(width*2, height*2);
        texture_fbo = render_fbo;
        light_fbo = new QGLFramebufferObject(width,height);
        lightBlur_fbo = light_fbo;
    }
    xRot = yRot = zRot = 0;
    xPos = yPos = zPos = 0.f;

    if(parent) resize(parent->width(), parent->height());
    startTimer(20);
}

void GLWidget::timerEvent(QTimerEvent *)
{
    if(bRotateCamera)
    {
        setYRotation(yRot + 3.f);
    }
    repaint();
}

GLWidget::~GLWidget()
{
    makeCurrent();
    mutex->lock();
    if(textureNames) glDeleteTextures(2, textureNames);
    //KILL(textureNames);
    objects.clear();
    objectAlive.clear();
    if(textureData){
        FOR(i, textureCount) delete [] textureData[i];
        delete [] textureData;
    }
    textureData = 0;
    mutex->unlock();
    FORIT(shaders, QString, QGLShaderProgram*)
    {
        QGLShaderProgram *program = it->second;
        if(!program) continue;
        QList<QGLShader*> shaderList = program->shaders();
        program->removeAllShaders();
        FOR(i, shaderList.size())
        {
            delete shaderList.at(i);
        }
        delete program;
    }
    shaders.clear();
    DEL(render_fbo);
    DEL(light_fbo);
    if (QGLFramebufferObject::hasOpenGLFramebufferBlit()) {
        DEL(lightBlur_fbo);
        DEL(texture_fbo);
    }

    DEL(mutex);
}

void GLWidget::AddObject(GLObject &o)
{
    objects.push_back(o);
    objectAlive.push_back(true);
}

void GLWidget::SetObject(int index, GLObject &o)
{
    if (index < 0 || index > objects.size()) return;
    mutex->lock();
    objects[index] = o;
    objectAlive[index] = true;
    mutex->unlock();
}

void GLWidget::clearLists()
{
    mutex->lock();
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
    killList.resize(objects.size());
    FOR(i, objects.size()) killList[i] = i;
    mutex->unlock();
}

void GLWidget::killObjects()
{
    if(!killList.size()) return;
    objectAlive.resize(objects.size(), true);
    std::sort(killList.begin(), killList.end(), std::greater<int>());
    FOR(i, killList.size())
    {
#ifdef WIN32
        qDebug() << "killing object " << killList[i] << "->" << objects[killList[i]].objectType;
        //objects.erase(objects.begin() + killList[i]);
        //objectAlive.erase(objectAlive.begin() + killList[i]);
        objectAlive[killList[i]] = false;
        //GLObject &o = objects[killList[i]];
        //o.vertices.clear();
        //o.barycentric.clear();
        //o.colors.clear();
        //o.normals.clear();
#else
        objects.erase(objects.begin() + killList[i]);
        objectAlive.erase(objectAlive.begin() + killList[i]);
#endif
    }
    killList.clear();
    FOR(i, objects.size())
    {
        qDebug() << i << (objectAlive[i] ? "alive" : "dead") << "->" << objects[i].objectType << "->" << objects[i].vertices.size();
    }
}

void GLWidget::initializeGL()
{
    //if(!textureNames) textureNames = new GLuint[textureCount];
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
                    if(r < 0.7)
                    {
                        pData[offs + 0] = 255; //r
                        pData[offs + 1] = 255; //g
                        pData[offs + 2] = 255; //b
                        pData[offs + 3] = 255; // *
                    }
                    else if (r < 0.95)
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
                    if(r > 0.6 && r < 0.8)
                    {
                        pData[offs + 0] = 255; //r
                        pData[offs + 1] = 255; //g
                        pData[offs + 2] = 255; //b
                        pData[offs + 3] = 255; // *
                    }
                    else if(r > 0.5 && r < 0.95) // the outlines
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
                case 2: // fuzzy particle
                {
                    pData[offs + 0] = 255*(1.f-r);
                    pData[offs + 1] = 255*(1.f-r);
                    pData[offs + 2] = 255*(1.f-r);
                    pData[offs + 3] = 255;
                }
                    break;
                }
            }
        }
    }
    if(textureNames)
    {
        glDeleteTextures(textureCount, textureNames);
        DEL(textureNames);
    }
    textureNames = new GLuint[textureCount];
    glGenTextures(textureCount, textureNames); // 0: samples, 1: wide circles
    glEnable(GL_TEXTURE_2D);
    GLActiveTexture(GL_TEXTURE0);

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
    light.SetDiffuse(0.8,0.8,0.8);
    light.SetSpecular(1.0,1.0,1.0);
    light.SetPosition(50.0f, 0.0f, 0.0f);
    lights.push_back(light);
    light.SetAmbient(0,0,0);
    light.SetSpecular(0,0,0);
    light.SetPosition(-50.0f, 50.0f, -50.0f);
    lights.push_back(light);
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
    program->bindAttributeLocation("vertexColor", 2);
    program->bindAttributeLocation("barycentric", 3);
    shaders["SmoothTransparent"] = program;
    program = 0;
    LoadShader(&program, ":/MLDemos/shaders/renderFBO.vsh", ":/MLDemos/shaders/renderFBO.fsh");
    program->bindAttributeLocation("vertex",0);
    shaders["RenderFBO"] = program;
    program = 0;
    LoadShader(&program, ":/MLDemos/shaders/blurFBO.vsh", ":/MLDemos/shaders/blurFBO.fsh");
    program->bindAttributeLocation("vertex",0);
    shaders["BlurFBO"] = program;
    program = 0;
    LoadShader(&program,":/MLDemos/shaders/depthSamples.vsh",":/MLDemos/shaders/depthSamples.fsh");
    program->bindAttributeLocation("vertex", 0);
    shaders["DepthSamples"] = program;
    program = 0;
    LoadShader(&program,":/MLDemos/shaders/drawSamplesShadow.vsh",":/MLDemos/shaders/drawSamplesShadow.fsh");
    program->bindAttributeLocation("vertex", 0);
    program->bindAttributeLocation("color", 1);
    shaders["SamplesShadow"] = program;

    glEnable(GL_MULTISAMPLE);
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
    GLObject oSamples;
    oSamples.objectType = "Samples,Canvas";
    int dataCount = canvas->data->GetCount();
    FOR(i, dataCount)
    {
        fvec sample = canvas->data->GetSample(i);
        if(canvas->data->GetFlag(i) == _TRAJ) continue; // we don't want to draw trajectories
        oSamples.vertices.append(QVector3D(xIndex >= 0 && xIndex < dim ? sample[xIndex] : 0,
                                           yIndex >= 0 && yIndex < dim ? sample[yIndex] : 0,
                                           zIndex >= 0 && zIndex < dim ? sample[zIndex] : 0));
        QColor color = SampleColor[canvas->data->GetLabel(i)%SampleColorCnt];
        oSamples.colors.append(QVector4D(color.redF(), color.greenF(), color.blueF(),1));
    }
    // aaand the trajectories
    ivec trajLabels(canvas->data->GetSequences().size());
    FOR(i, canvas->data->GetSequences().size()) trajLabels[i] = canvas->data->GetLabel(canvas->data->GetSequences()[i].first);
    vector< vector<fvec> > trajectories = canvas->data->GetTrajectories(canvas->trajectoryResampleType,
                                                                        canvas->trajectoryResampleCount,
                                                                        canvas->trajectoryCenterType, 0.1, true);
    vector<GLObject> oTrajs;
    FOR(i, trajectories.size())
    {
        GLObject o;
        FOR(j, trajectories[i].size())
        {
            fvec sample = trajectories[i][j];
            o.vertices.append(QVector3D(xIndex >= 0 && xIndex < dim ? sample[xIndex] : 0,
                                        yIndex >= 0 && yIndex < dim ? sample[yIndex] : 0,
                                        zIndex >= 0 && zIndex < dim ? sample[zIndex] : 0));
            QColor color = SampleColor[trajLabels[i]%SampleColorCnt];
            o.colors.append(QVector4D(color.redF(), color.greenF(), color.blueF(),1));
        }
        o.objectType = "Samples,Canvas,trajectories";
        o.style ="pointsize:5,width:0.5,dotted";
        oTrajs.push_back(o);
    }

    // and we replace the current objects with them
    bool bReplacedSamples=false;
    FOR(i, objects.size())
    {
        if(objects[i].objectType.contains("trajectories"))
        {
            objects.erase(objects.begin()+i);
            objectAlive.erase(objectAlive.begin() + i);
            i--;
        }
        else if(dataCount && !bReplacedSamples && objects[i].objectType.contains("Samples,Canvas"))
        {
            objects[i] = oSamples;
            objectAlive[i] = true;
            bReplacedSamples = true;
        }
    }
    if(!bReplacedSamples && dataCount)
    {
        objects.push_back(oSamples);
        objectAlive.push_back(true);
    }
    if(oTrajs.size())
    {
        objects.insert(objects.end(),oTrajs.begin(),oTrajs.end());
        objectAlive.resize(objects.size(), true);
    }

    if(!canvas->maps.reward.isNull())
    {
        int rewardIndex = -1;
        FOR(i, objects.size())
        {
            if(objects[i].objectType.contains("Reward"))
            {
                rewardIndex = i;
                break;
            }
        }
        if(rewardIndex == -1)
        {
            int w = canvas->maps.reward.width();
            int h = canvas->maps.reward.height();
            float ratio = h/(float)w;
            int xSteps = 100;
            int ySteps = h * xSteps / w;
            //int ySteps = h * xSteps / w;
            QImage rewardImage = canvas->maps.reward.scaled(xSteps,ySteps,Qt::KeepAspectRatioByExpanding).toImage();
            QRgb *pixels = (QRgb*) rewardImage.bits();
            float *values = new float[xSteps*ySteps];
            float maxData = 0;
            FOR(i, xSteps*ySteps)
            {
                values[i] = 1.f - qBlue(pixels[i])/255.f; // all data is in a 0-1 range
                maxData = max(maxData, values[i]);
            }
            if(maxData > 0)
            {
                FOR(i, xSteps*ySteps) values[i] = values[i]/maxData*0.5f; // we ensure that the data is normalized
            }
            fvec mins(3,-1.f);
            fvec maxes(3,1.f);
            fflush(stdout);
            GLObject o = GenerateMeshGrid(values, xSteps, ySteps, mins, maxes, 0, 2, 1);
            o.objectType = "Surfaces,Reward,quads";
            o.style = "smooth,color:1:0.7:0.7:1,specularity:0.2,shininess:8";
            objects.push_back(o);
            objectAlive.push_back(true);
            KILL(values);
            printf("reward mesh generated: %d vertices (%d normals)\n", o.vertices.size(), o.normals.size());fflush(stdout);
        }
    }
}

void GLWidget::DrawObject(const GLObject &o) const
{
    if(!o.vertices.size()) return;
    if(bDisplaySamples && o.objectType.contains("Samples")) DrawSamples(o);
    if(bDisplayLines && o.objectType.contains("Lines") || o.objectType.contains("trajectories")) DrawLines(o);
    else if(bDisplaySurfaces && o.objectType.contains("Surfaces")) DrawSurfaces(o);
    else if(bDisplayLines && o.objectType.contains("Particles")) DrawParticles(o);
}

void GLWidget::DrawParticles(const GLObject &o) const
{
    QString style = o.style.toLower();
    float pointSize = 12.f;
    if(style.contains("pointsize"))
    {
        QStringList params = style.split(",");
        FOR(i, params.size())
        {
            if(params[i].contains("pointsize"))
            {
                QStringList p = params[i].split(":");
                pointSize = p.at(1).toFloat();
                break;
            }
        }
    }

    QGLShaderProgram *program = shaders.at("Samples");
    program->bind();
    program->enableAttributeArray(0);
    program->enableAttributeArray(1);
    program->setAttributeArray(0, o.vertices.constData());
    program->setAttributeArray(1, o.colors.constData());
    program->setUniformValue("matrix", modelViewProjectionMatrix);

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glDisable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glEnable(GL_BLEND);
    glEnable(GL_ALPHA_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, GLWidget::textureNames[2]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    program->setUniformValue("color_texture", 0);
    glEnable(GL_PROGRAM_POINT_SIZE_EXT);
    glPointSize(pointSize);

    // we actually draw stuff!
    glDrawArrays(GL_POINTS,0,o.vertices.size());
    glPopAttrib();

    program->release();
}

void GLWidget::DrawSamples(const GLObject &o) const
{
    QString style = o.style.toLower();
    float pointSize = 12.f;
    if(style.contains("pointsize"))
    {
        QStringList params = style.split(",");
        FOR(i, params.size())
        {
            if(params[i].contains("pointsize"))
            {
                QStringList p = params[i].split(":");
                pointSize = p.at(1).toFloat();
                break;
            }
        }
    }
    if(bHiDPI) pointSize *= 2;

    QGLShaderProgram *program = bDisplayShadows ? shaders.at("SamplesShadow") : shaders.at("Samples");
    program->bind();
    program->enableAttributeArray(0);
    program->enableAttributeArray(1);
    program->setAttributeArray(0, o.vertices.data());
    program->setAttributeArray(1, o.colors.data());
    //program->setAttributeArray(0, o.vertices.constData());
    //program->setAttributeArray(1, o.colors.constData());
    program->setUniformValue("matrix", modelViewProjectionMatrix);

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glDisable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glEnable(GL_BLEND);
    glEnable(GL_ALPHA_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_POINT_SPRITE);
    if(o.style.contains("rings")) glBindTexture(GL_TEXTURE_2D, GLWidget::textureNames[1]);
    else glBindTexture(GL_TEXTURE_2D, GLWidget::textureNames[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    program->setUniformValue("color_texture", 0);

    if(bDisplayShadows)
    {
        glEnable(GL_LIGHTING);
        program->setUniformValue("lightMvpMatrix", lightMvpMatrix);
        program->setUniformValue("lightMvMatrix", lightMvMatrix);
        GLActiveTexture(GL_TEXTURE1);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, light_fbo->texture());
        program->setUniformValue("shadow_texture", 1);
        program->setUniformValue("pointSize", pointSize);
        GLActiveTexture(GL_TEXTURE0);
    }

    glEnable(GL_PROGRAM_POINT_SIZE_EXT);
    glPointSize(pointSize);

    // we actually draw stuff!
    glDrawArrays(GL_POINTS,0,o.vertices.size());

    glPopAttrib();

    program->release();
}

void GLWidget::DrawLines(const GLObject &o) const
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
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
    glLineStipple (3, 0xFFFF);
    glColor3f(0,0,0);

    int fading = 0;
    QStringList params = style.split(",");
    FOR(i, params.size())
    {
        if(params[i].contains("fading"))
        {
            QStringList p = params[i].split(":");
            fading = p[1].toInt();
        }
        if(params[i].contains("width"))
        {
            QStringList p = params[i].split(":");
            float val = p[1].toFloat();
            glLineWidth(val);
        }
        if(params[i].contains("linecolor"))
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

    glPushMatrix();

#include <qglobal.h>
    // qreal might be a float, need to account for that here.
    // No choice but to use the logic in Qt/qglobal.h
#if defined(QT_COORD_TYPE)
    // typedef QT_COORD_TYPE qreal;
    // (which we'll just assume is a double)
    glMultMatrixd(o.model.constData());
#elif defined(QT_NO_FPU) || defined(QT_ARCH_ARM) || defined(QT_ARCH_WINDOWSCE) || defined(QT_ARCH_SYMBIAN)
    // typedef float qreal;
    glMultMatrixf(o.model.constData());
#else
    // typedef double qreal;
    glMultMatrixf(o.model.constData());
#endif

    if(o.objectType.contains("linestrip") || o.objectType.contains("trajectories")) glBegin(GL_LINE_STRIP);
    else glBegin(GL_LINES);
    FOR(i, o.vertices.size())
    {
        glVertex3f(o.vertices.at(i).x(),o.vertices.at(i).y(),o.vertices.at(i).z());
        float alpha = 1.f;
        if(fading) alpha = 1.f - ((i/2)%fading)/float(fading);
        if(!o.objectType.contains("trajectories") && i < o.colors.size()) glColor4f(o.colors[i].x(), o.colors[i].y(), o.colors[i].z(), alpha);
        else glColor4f(0,0,0,alpha);
    }
    glEnd();
    glPopMatrix();
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
        QVector3D a,b;
        QVector<QVector3D> faceNormals(o.vertices.size()/4);
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
        if(o.vertices.size() < 65536) // if we have too many points it would be a tad too slow
        {
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
        }
        else // flatshaded crappy version
        {
            FOR(i, o.vertices.size())
            {
                int faceIndex = i/4;
                o.normals[i] = faceNormals[faceIndex];
            }
        }
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
        if(o.vertices.size() < 65536) // if we have too many points it would be a tad too slow
        {
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
        else // flatshaded crappy version
        {
            FOR(i, o.vertices.size())
            {
                int faceIndex = i/4;
                o.normals[i] = faceNormals[faceIndex];
            }
        }
    }
}

void GLWidget::FixSurfaces(GLObject &o)
{
    if(!o.objectType.contains("Surfaces")) return;
    if(o.normals.size() != o.vertices.size()) // we need to recompute the normals
    {
        qDebug() << "recomputing normals";
        RecomputeNormals(o);
        qDebug() << "Done.";
    }
    if(o.colors.size() != o.vertices.size())
    {
        qDebug() << "filling in colors";
        o.colors.resize(o.vertices.size());
        FOR(i, o.vertices.size())
        {
            o.colors[i] = QVector4D(1,1,1,1);
        }
    }

    if(o.barycentric.size() != o.vertices.size())
    {
        qDebug() << "recomputing barycentrics";
        RecomputeBarycentric(o);
        qDebug() << "Done.";
    }
}

void GLWidget::DrawSurfaces(const GLObject &o) const
{
    QString style = o.style.toLower();
    QStringList params = style.split(",");

    float specularity = 0.7f;
    float shininess = 8.f;
    float alpha = style.contains("transparent") ? 0.5f : 1.f;
    QVector4D material(1,1,1,alpha);
    float offset = 0.f;
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
        else if(params[i].contains("specularity"))
        {
            QStringList p = params[i].split(":");
            specularity = p[1].toFloat();
        }
        else if(params[i].contains("shininess"))
        {
            QStringList p = params[i].split(":");
            shininess = p[1].toFloat();
        }
        else if(params[i].contains("offset"))
        {
            QStringList p = params[i].split(":");
            offset = p[1].toFloat();
        }
    }

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glEnable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    if(style.contains("smooth")) glShadeModel(GL_SMOOTH);
    else glShadeModel (GL_FLAT);

    if(offset)
    {
        glPolygonOffset(offset, 1.0);
        glEnable(GL_POLYGON_OFFSET_FILL);
    }
    //    glCullFace(GL_BACK);
    //    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);

    glEnable(GL_BLEND);
    //glBlendEquation(GL_FUNC_ADD);
    glEnable(GL_ALPHA_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE | GL_SPECULAR);

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
        if(style.contains("isolines"))
        {
            surfaceStyle.setY(2 + 1); // by default we draw isolines on the vertical axis
            QStringList params = style.split(",");
            FOR(i, params.size())
            {
                if(params[i].contains("isolines:"))
                {
                    QStringList p = params[i].split(":");
                    surfaceStyle.setY(2 + p[1].toFloat());
                }
            }
        }
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
        QGLShaderProgram *program = shaders.at("SmoothTransparent");
        program->bind();
        program->setUniformValue("mvMatrix", mvMatrix);
        program->setUniformValue("mvpMatrix", mvpMatrix);
        program->setUniformValue("normalMatrix", normMatrix);
        program->setUniformValue("viewport", viewport);
        program->setUniformValue("materialColor", material);
        program->setUniformValue("surfaceStyle", surfaceStyle);
        program->setUniformValue("specularity", specularity);
        program->setUniformValue("shininess", shininess);
        program->setUniformValue("transparency", bDisplayTransparency);
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
        program->enableAttributeArray(3);
        program->setAttributeArray(0, o.vertices.constData());
        program->setAttributeArray(1, o.normals.constData());
        program->setAttributeArray(2, o.colors.constData());
        program->setAttributeArray(3, o.barycentric.constData());

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

void DrawLights(std::vector<GLLight> &lights)
{
    // we draw the lights
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    FOR(i, lights.size())
    {
        glPushMatrix();
        glTranslatef(lights[i].position[0],lights[i].position[1],lights[i].position[2]);
        glColor3f(0,0,1.0);
        DrawStandardSphere(0.05f);
        glPopMatrix();
    }
    glPopAttrib();
}

float angle = 0;
void GLWidget::paintGL()
{
    if(!canvas) return;
    mutex->lock();
    killObjects();
    generateObjects();
    FOR(i, objects.size()) FixSurfaces(objects[i]);
    bool bEmpty = !canvas->bDisplayGrid;
    if(bEmpty)
    {
        FOR(i, objects.size())
        {
            if(objects[i].vertices.size())
            {
                bEmpty = false;
                break;
            }
        }
    }
    modelViewMatrix.setToIdentity();
    modelViewMatrix.rotate(xRot / 16.0, 1.0, 0.0, 0.0);
    modelViewMatrix.rotate(yRot / 16.0, 0.0, 1.0, 0.0);
    modelViewMatrix.rotate(zRot / 16.0, 0.0, 0.0, 1.0);
    modelViewMatrix.translate(xPos, yPos, zPos);

    bool bHasReward = false;
    if(bEmpty)
    {
        glClearColor(1,1,1,1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        mutex->unlock();
        return;
    }
    FOR(i, objects.size())
    {
        if(objects[i].objectType.contains("Reward"))
        {
            bHasReward = true;
            break;
        }
    }
    if(bHasReward)
    {
        lights[0].position[0] = -5;
        lights[0].position[1] = 0;
        lights[0].position[2] = -5;
        lights[1].position[0] = 5;
        lights[1].position[1] = 5;
        lights[1].position[2] = -5;
    }
    else
    {
        lights[0].position[0] = 2*sin(angle*0.1f);
        lights[0].position[1] = 0;
        lights[0].position[2] = 2*cos(angle*0.1f);
        lights[1].position[0] = 2*sin(angle*0.13f);
        lights[1].position[1] = -2*cos(angle*0.13f);
        lights[1].position[2] = 2*sin(angle*0.13f);
        angle += 0.1f;
    }

    modelViewProjectionMatrix = perspectiveMatrix*modelViewMatrix;
    normalMatrix = modelViewMatrix.normalMatrix();

    // currently broken in windows (crashes for some reason every now and then)
    //RenderShadowMap(light_fbo, lights[0], objects);
    bool bDisplayShadowMap = false;
    if (bDisplayShadowMap)
    {
        QPixmap pixmap = QPixmap::fromImage(light_fbo->toImage());
        if(!lightLabel)
        {
            lightLabel = new QLabel();
            lightLabel->setScaledContents(true);
        }
        lightLabel->setPixmap(pixmap);
        lightLabel->show();
        lightLabel->repaint();
    }

    render_fbo->bind();
    checkGL();
    glEnable(GL_MULTISAMPLE);
    glClearColor(1.f, 1.f, 1.f, 1.0f);
    //	glClearColor(.5f, .5f, .5f, 1.0f);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glPushMatrix();

    // we do the camera rotation once for 'standard' opengl geometry (lines etc)
    glRotated(xRot / 16.0, 1.0, 0.0, 0.0);
    glRotated(yRot / 16.0, 0.0, 1.0, 0.0);
    glRotated(zRot / 16.0, 0.0, 0.0, 1.0);
    glTranslatef(xPos, yPos, zPos);

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    checkGL();

    bool bBlurry = false;
    if(bDisplayBlurry && bDisplayTransparency)
    {
        FOR(i, objects.size())
        {
            if(!objectAlive[i]) continue;
            if(objects[i].objectType.contains("Surface") && objects[i].style.contains("blurry"))
            {
                bBlurry = true;
                break;
            }
        }
        if(bBlurry)
        {
            // first we begin by drawing all our blurry surfaces
            glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
            //glDisable(GL_DEPTH_TEST);
            glEnable(GL_STENCIL_TEST);
            glStencilFunc(GL_ALWAYS, 1, 1);
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
            checkGL();
            int blurAmount = 3;
            FOR(i, objects.size())
            {
                if(!objectAlive[i]) continue;
                if(objects[i].objectType.contains("Surface") && objects[i].style.contains("blurry"))
                {
                    QStringList params = objects[i].style.split(",");
                    FOR(j, params.size())
                    {
                        if(params[j].contains("blurry:"))
                        {
                            QStringList p = params[j].split(":");
                            blurAmount = p[1].toFloat();
                            break;
                        }
                    }
                    DrawObject(objects[i]);
                }
            }
            glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
            glStencilFunc(GL_EQUAL, 1, 1);
            glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP);
            checkGL();

            // then we draw all the stuff that is BEHIND the surfaces
            glDepthFunc(GL_GREATER);

            // Here we draw the axes lines
            if(canvas->bDisplayGrid) DrawAxes(zoomFactor);
            // DrawLights(lights);
            FOR(i, objects.size())
            {
                if(!objectAlive[i]) continue;
                if(objects[i].objectType.contains("Canvas"))
                {
                    if(objects[i].objectType.contains("trajectories"))
                    {
                        if(canvas->bDisplayTrajectories) DrawObject(objects[i]);
                    }
                    else if(canvas->bDisplaySamples ||
                            (canvas->sampleColors.size() == canvas->data->GetCount() && canvas->bDisplayLearned))
                    {
                        // we check if we need to recolor the object
                        if(canvas->bDisplayLearned && canvas->sampleColors.size() == canvas->data->GetCount())
                        {
                            GLObject o = objects[i];
                            FOR(j, o.vertices.size())
                            {
                                QColor c = canvas->sampleColors[j];
                                o.colors[j] = QVector4D(c.redF(), c.greenF(), c.blueF(), 1);
                            }
                            DrawObject(o);
                        }
                        else DrawObject(objects[i]);
                    }
                }
            }
            FOR(i, objects.size())
            {
                if(!objectAlive[i]) continue;
                if(objects[i].objectType.contains("Canvas")) continue;
                if(objects[i].objectType.contains("Surface") && objects[i].style.contains("blurry")) continue;
                DrawObject(objects[i]);
            }
            glDepthFunc(GL_LEQUAL);

            QGLShaderProgram *program = shaders["BlurFBO"];
            program->bind();
            program->setUniformValue("bVertical", 1);
            program->setUniformValue("amount", blurAmount);
            QRect rect(0, 0, render_fbo->width(), render_fbo->height());
            if(texture_fbo)
            {
                if(texture_fbo != render_fbo) QGLFramebufferObject::blitFramebuffer(texture_fbo, rect, render_fbo, rect);
                RenderFBO(texture_fbo, program);
                if(texture_fbo != render_fbo) QGLFramebufferObject::blitFramebuffer(texture_fbo, rect, render_fbo, rect);
                program->setUniformValue("bVertical", 0);
                RenderFBO(texture_fbo, program);
            }
            program->release();

            glDisable(GL_STENCIL_TEST);
            FOR(i, objects.size())
            {
                if(!objectAlive[i]) continue;
                if(objects[i].objectType.contains("Surfaces") && objects[i].style.contains("blurry"))
                    DrawObject(objects[i]);
            }
        }
    }

    // Here we draw the axes lines
    if(canvas->bDisplayGrid) DrawAxes(zoomFactor);
    // DrawLights(lights);

    FOR(i, objects.size())
    {
        if(!objectAlive[i]) continue;
        if(objects[i].objectType.contains("Canvas"))
        {
            if(objects[i].objectType.contains("trajectories"))
            {
                if(bDisplayLines) DrawObject(objects[i]);
            }
            else if(bDisplaySamples)
            {
                // we check if we need to recolor the object
                if(canvas->bDisplayLearned && canvas->sampleColors.size() == canvas->data->GetCount())
                {
                    GLObject o = objects[i];
                    FOR(j, o.vertices.size())
                    {
                        QColor c = canvas->sampleColors[j];
                        o.colors[j] = QVector4D(c.redF(), c.greenF(), c.blueF(),1);
                    }
                    DrawObject(o);
                }
                else DrawObject(objects[i]);
            }
        }
    }
    FOR(i, objects.size())
    {
        if(!objectAlive[i]) continue;
        if(objects[i].objectType.contains("Canvas")) continue;
        if(bDisplayBlurry && bDisplayTransparency &&
                objects[i].objectType.contains("Surface") &&
                objects[i].style.contains("blurry")) continue;
        DrawObject(objects[i]);
    }

    glPopAttrib();
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    if(bDisplaySamples)
    {
        // we begin with the sample sprites
        glEnable(GL_TEXTURE_2D);
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
    glPopAttrib();
    glPopMatrix();
    render_fbo->release();

    if (render_fbo && texture_fbo && render_fbo != texture_fbo) {
        QRect rect(0, 0, render_fbo->width(), render_fbo->height());
        QGLFramebufferObject::blitFramebuffer(texture_fbo, rect, render_fbo, rect);
    }

    QGLShaderProgram *program = shaders["RenderFBO"];
    program->bind();
    RenderFBO(texture_fbo, program);
    program->release();
    mutex->unlock();
}

void GLWidget::RenderShadowMap(QGLFramebufferObject *fbo, GLLight light, std::vector<GLObject> objects)
{
    if(!fbo || !objects.size()) return;

    //glMatrixMode(GL_PROJECTION);
    //glPushMatrix();
    //glMatrixMode(GL_MODELVIEW);
    //glPushMatrix();
    //glViewport(0,0,512,512);

    fbo->bind();
    glEnable(GL_MULTISAMPLE);
    glClearColor(1.f, 1.f, 1.f, 1.0f);
    //    glClearColor(.5f, .5f, .5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);


    double zNear = 1.0;
    double zFar = 60.0;

    QVector3D lightUp(0,1,0);
    QVector3D lightPos(light.position[0], light.position[1], light.position[2]);
    lightMvMatrix.setToIdentity();
    //    mvMatrix.lookAt(QVector3D(0,0,-40), QVector3D(0,0,0), lightUp);
    lightMvMatrix.lookAt(lightPos, QVector3D(0,0,0), lightUp);
    lightPMatrix.setToIdentity();
    lightPMatrix.perspective(90.,1.,zNear,zFar);

    lightMvpMatrix = lightPMatrix * lightMvMatrix;
    QMatrix3x3 nMatrix = lightMvMatrix.normalMatrix();

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    FOR(i, objects.size())
    {

        GLObject& o = objects[i];
        if(!o.vertices.size()) continue;
        QString style = o.style.toLower();
        if(style.contains("transparent")) continue;
        float pointSize = 12.f;
        if(style.contains("pointsize"))
        {
            QStringList params = style.split(",");
            FOR(i, params.size())
            {
                if(params[i].contains("pointsize"))
                {
                    QStringList p = params[i].split(":");
                    pointSize = p.at(1).toFloat();
                    break;
                }
            }
        }

        QGLShaderProgram *program = shaders["DepthSamples"];
        program->bind();
        program->enableAttributeArray(0);
        program->setAttributeArray(0, o.vertices.constData());
        program->setUniformValue("mvpMatrix", lightMvpMatrix);
        program->setUniformValue("mvmatrix", lightMvMatrix);

        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glDisable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        glEnable(GL_ALPHA_TEST);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glEnable(GL_TEXTURE_2D);
        glEnable(GL_POINT_SPRITE);
        if(o.style.contains("rings")) glBindTexture(GL_TEXTURE_2D, GLWidget::textureNames[1]);
        else glBindTexture(GL_TEXTURE_2D, GLWidget::textureNames[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        program->setUniformValue("color_texture", 0);
        glEnable(GL_PROGRAM_POINT_SIZE_EXT);
        glPointSize(pointSize);

        // we actually draw stuff!
        glDrawArrays(GL_POINTS,0,o.vertices.size());
        glPopAttrib();
        program->release();
    }
    glPopAttrib();

    // we blur the shadowmap
    QGLShaderProgram *program = shaders["BlurFBO"];
    program->bind();
    program->setUniformValue("bVertical", 1);
    program->setUniformValue("amount", 7);
    QRect rect(0, 0, light_fbo->width(), light_fbo->height());
    if (light_fbo != lightBlur_fbo) QGLFramebufferObject::blitFramebuffer(lightBlur_fbo, rect, light_fbo, rect);
    RenderFBO(lightBlur_fbo, program);
    if (light_fbo != lightBlur_fbo) QGLFramebufferObject::blitFramebuffer(lightBlur_fbo, rect, light_fbo, rect);
    program->setUniformValue("bVertical", 0);
    RenderFBO(lightBlur_fbo, program);
    program->release();
    fbo->release();

    //glMatrixMode(GL_PROJECTION);
    //glPopMatrix();
    //glMatrixMode(GL_MODELVIEW);
    //glPopMatrix();
    //glViewport(0,0,width,height);
}


void GLWidget::RenderFBO(QGLFramebufferObject *fbo, QGLShaderProgram *program)
{
    if(!fbo || !program) return;
    // save GL state
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    // set ortho view
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, width, 0.0, height, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);

    program->setUniformValue("texture", 0);
    GLfloat fbo_vertices[] = {
        -1, -1,
         1, -1,
        -1,  1,
         1,  1};
    program->enableAttributeArray(0);
    program->setAttributeArray(0,fbo_vertices,2);

    // we bind the texture
    glBindTexture(GL_TEXTURE_2D, fbo->texture());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_MULTISAMPLE);

    glDrawArrays(GL_TRIANGLE_STRIP,0,4);

    // restore GL state
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glPopAttrib();
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
    mutex->lock();
    this->width = width;
    this->height = height;

    glViewport(0,0,width, height);
    viewport = QVector4D(0,0,width,height);
    float ratio = width/(float)height;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    double left = -ratio, right = ratio, bottom = -1.0, top = 1.0;
    double zNear = 1.0;
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
    if(width != render_fbo->width() && height != render_fbo->height())
    {
        if(render_fbo->isBound()) render_fbo->release();
        delete render_fbo;
        delete light_fbo;
        if (QGLFramebufferObject::hasOpenGLFramebufferBlit()) {
            delete lightBlur_fbo;
            delete texture_fbo;
            QGLFramebufferObjectFormat format;
            format.setSamples(64);
            format.setAttachment(QGLFramebufferObject::CombinedDepthStencil);
            render_fbo = new QGLFramebufferObject(width, height, format);
            texture_fbo = new QGLFramebufferObject(width, height);
            light_fbo = new QGLFramebufferObject(width,height, format);
            lightBlur_fbo = new QGLFramebufferObject(width,height);
        } else {
            render_fbo = new QGLFramebufferObject(width*2, height*2);
            texture_fbo = render_fbo;
            light_fbo = new QGLFramebufferObject(width,height);
            lightBlur_fbo = lightBlur_fbo;
        }
    }
    mutex->unlock();
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

void GLWidget::normalizeAngle(int &angle)
{
    while (angle < 0) angle += 360 * 16;
    while (angle > 360 * 16) angle -= 360 * 16;
}

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
    normalizeAngle(angle);
    if (angle != xRot) {
        xRot = angle;
        emit xRotationChanged(angle);
        updateGL();
    }
}

void GLWidget::setYRotation(int angle)
{
    normalizeAngle(angle);
    if (angle != yRot) {
        yRot = angle;
        emit yRotationChanged(angle);
        updateGL();
    }
}

void GLWidget::setZRotation(int angle)
{
    normalizeAngle(angle);
    if (angle != zRot) {
        zRot = angle;
        emit zRotationChanged(angle);
        updateGL();
    }
}
