#include <GHSOM/globals.h>
#include <QGLWidget>
#include "interfaceGHSOM.h"
#include "projectorGHSOM.h"
#include <QDebug>

using namespace std;

GHSOMProjector::GHSOMProjector()
    : widget(new QWidget())
{
    params = new Ui::paramsGHSOM();
    params->setupUi(widget);
    connect(params->growingCheck, SIGNAL(clicked()), this, SLOT(ChangeOptions()));
    connect(params->tau1Spin, SIGNAL(valueChanged(double)), this, SLOT(ChangeOptions()));
    ChangeOptions();
}

// virtual functions to manage the algorithm creation
Projector *GHSOMProjector::GetProjector()
{
    return new ProjectorGHSOM();
}

void GHSOMProjector::ChangeOptions()
{
    int bGrowing = params->growingCheck->isChecked();
    int bFullTau = params->tau1Spin->value() != 1.f;
    params->label1->setVisible(bGrowing);
    params->label2->setVisible(bGrowing&&bFullTau);
    params->label3->setVisible(bGrowing&&bFullTau);
    params->tau1Spin->setVisible(bGrowing);
    params->tau2Spin->setVisible(bGrowing&&bFullTau);
    params->expandSpin->setVisible(bGrowing&&bFullTau);
}


void GHSOMProjector::SetParams(Projector *projector)
{
    if(!projector) return;
    ProjectorGHSOM *ghsom = dynamic_cast<ProjectorGHSOM*>(projector);
    if(!ghsom) return;
    float tau1 = params->tau1Spin->value();
    float tau2 = params->tau2Spin->value();
    float learningRate = params->learnRateSpin->value();
    float neighborhoodRadius = params->nrSpin->value();
    int xSize = params->xSizeSpin->value();
    int ySize = params->ySizeSpin->value();
    int expandCycles = params->expandSpin->value();
    int normalizationType = params->normalizationCombo->currentIndex();
    bool bGrowing = params->growingCheck->isChecked();
    if(!bGrowing)
    {
        tau1 = 1.0;
        tau2 = 1.0;
        expandCycles = 100;
    }
    ghsom->SetParams(tau1, tau2, xSize, ySize,
                     expandCycles, normalizationType,
                     learningRate, neighborhoodRadius);
}

fvec GHSOMProjector::GetParams()
{
    float tau1 = params->tau1Spin->value();
    float tau2 = params->tau2Spin->value();
    float learningRate = params->learnRateSpin->value();
    float neighborhoodRadius = params->nrSpin->value();
    int xSize = params->xSizeSpin->value();
    int ySize = params->ySizeSpin->value();
    int expandCycles = params->expandSpin->value();
    int normalizationType = params->normalizationCombo->currentIndex();
    bool bGrowing = params->growingCheck->isChecked();

    int i=0;
    fvec par(9);
    par[i++] = tau1;
    par[i++] = tau2;
    par[i++] = learningRate;
    par[i++] = neighborhoodRadius;
    par[i++] = xSize;
    par[i++] = ySize;
    par[i++] = expandCycles;
    par[i++] = normalizationType;
    par[i++] = bGrowing;
    return par;
}

void GHSOMProjector::SetParams(Projector *projector, fvec parameters)
{
    if(!projector) return;
    ProjectorGHSOM *ghsom = dynamic_cast<ProjectorGHSOM*>(projector);
    if(!ghsom) return;
    int i=0;
    float tau1 = parameters.size() > i ? parameters[i] : 0; i++;
    float tau2 = parameters.size() > i ? parameters[i] : 0; i++;
    float learningRate = parameters.size() > i ? parameters[i] : 0; i++;
    float neighborhoodRadius = parameters.size() > i ? parameters[i] : 0; i++;
    int xSize = parameters.size() > i ? parameters[i] : 0; i++;
    int ySize = parameters.size() > i ? parameters[i] : 0; i++;
    int expandCycles = parameters.size() > i ? parameters[i] : 0; i++;
    int normalizationType = parameters.size() > i ? parameters[i] : 0; i++;
    bool bGrowing = parameters.size() > i ? parameters[i] : 0; i++;

    if(!bGrowing)
    {
        tau1 = 1.0;
        tau2 = 1.0;
        expandCycles = 100;
    }

    ghsom->SetParams(tau1, tau2, xSize, ySize,
                     expandCycles, normalizationType,
                     learningRate, neighborhoodRadius);
}

void GHSOMProjector::GetParameterList(std::vector<QString> &parameterNames,
                                std::vector<QString> &parameterTypes,
                                std::vector< std::vector<QString> > &parameterValues)
{
    parameterNames.push_back("Growth Rate 1");
    parameterNames.push_back("Growth Rate 2");
    parameterNames.push_back("Learning Rate");
    parameterNames.push_back("Neighborhood Radius");
    parameterNames.push_back("X Grid Size");
    parameterNames.push_back("Y Grid Size");
    parameterNames.push_back("Expand Cycles");
    parameterNames.push_back("Normalization Type");
    parameterNames.push_back("Growing");
    parameterTypes.push_back("Real");
    parameterTypes.push_back("Real");
    parameterTypes.push_back("Real");
    parameterTypes.push_back("Real");
    parameterTypes.push_back("Integer");
    parameterTypes.push_back("Integer");
    parameterTypes.push_back("Integer");
    parameterTypes.push_back("List");
    parameterTypes.push_back("List");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("0.00000001f");
    parameterValues.back().push_back("1.f");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("0.00000001f");
    parameterValues.back().push_back("1.f");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("0.00000001f");
    parameterValues.back().push_back("1.f");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("0.00000001f");
    parameterValues.back().push_back("9999999");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("1");
    parameterValues.back().push_back("999999");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("1");
    parameterValues.back().push_back("999999");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("1");
    parameterValues.back().push_back("99999999");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("None");
    parameterValues.back().push_back("Length");
    parameterValues.back().push_back("Interval");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("False");
    parameterValues.back().push_back("True");
}

void GHSOMProjector::DrawInfo(Canvas *canvas, QPainter &painter, Projector *projector)
{
    if(!canvas || !projector) return;
    if(canvas->canvasType) return;
    /*
    vector<fvec> samples = projector->source;
    vector<fvec> projected = projector->projected;
    ivec labels = canvas->data->GetLabels();
    if(!samples.size()) return;
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(Qt::black,0.5f));
    QPointF start(FLT_MAX, FLT_MAX), stop(-FLT_MAX, -FLT_MAX);
    FOR(i, samples.size())
    {
        projected[i] = projector->Project(samples[i]);
        QPointF p1 = canvas->toCanvasCoords(samples[i]);
        QPointF p2 = canvas->toCanvasCoords(projected[i]);
        if(start.x() > p2.x()) start = p2;
        if(stop.x() < p2.x()) stop = p2;
    }
    QPointF diff = stop - start;
    painter.setPen(QPen(Qt::black, 2));
    painter.drawLine(start - diff*0.25f, stop + diff*0.25f);
    painter.drawLine(stop - diff*0.25f, start + diff*0.25f);
    */
}

void GHSOMProjector::DrawGL(Canvas *canvas, GLWidget *glw, Projector *projector)
{
    if(!canvas || !glw || !projector) return;
    int dim = canvas->data->GetDimCount();
    int xIndex = canvas->xIndex;
    int yIndex = canvas->yIndex;
    int zIndex = canvas->zIndex;
    if(canvas->zIndex >= dim) zIndex = -1;

    GLuint list = glGenLists(1);
    glNewList(list, GL_COMPILE);

    int layerCount = 0;
    for(int i=Globals::layers->size()-1; i>0; i--)
    {
		GVector<NeuronLayer> *layer = Globals::layers->elementAt(i);
        layerCount += layer->size();
    }
    layerCount--;

    for(int i=Globals::layers->size()-1; i>0; i--) // we skip the first as it is just the average of all weights
    {
		GVector<NeuronLayer> *layer = Globals::layers->elementAt(i);
        FOR(j, layer->size())
        {
            Neuron ***map = layer->elementAt(j)->getMap();
            int w = layer->elementAt(j)->getX();
            int h = layer->elementAt(j)->getY();
            vector<fvec> points(w*h);
            vector< pair<fvec,fvec> > dataLines;
            FOR(y, h)
            {
                FOR(x, w)
                {
                    Neuron *neuron = map[x][y];
                    fvec mean(dim, 0);
                    if(neuron->weightsize >= dim)
                    {
                        FOR(d, dim) mean[d] = neuron->weights[d];
                    }
                    points[x + y*w] = mean;

                    if(neuron->getRepresentingDataItems()->size())
                    {
                        FOR(i, neuron->getRepresentingDataItems()->size())
                        {
                            DataItem *it = neuron->getRepresentingDataItems()->elementAt(i);
                            if(it->getLength() >= dim)
                            {
                                fvec point(dim);
                                FOR(d, dim) point[d] = it->getDataVector()[d];
                                dataLines.push_back(make_pair(point, points[x + y*w]));
                            }
                        }
                    }
                }
            }

            glPushAttrib(GL_ALL_ATTRIB_BITS);

            glDisable( GL_TEXTURE_2D );
            glEnable( GL_LINE_SMOOTH );
            glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            glColor4f(0,0,0,1.f);
            glEnable(GL_LINE_STIPPLE); // enable dashed/ dotted lines
            glLineWidth(0.5f); // line width
            glLineStipple (1, 0xAAAA); // dash pattern AAAA: dots
            glBegin(GL_LINES);
            FOR(i, dataLines.size())
            {
                fvec &p1 = dataLines[i].first;
                fvec &p2 = dataLines[i].second;
                GLWidget::glLine(p1, p2, xIndex, yIndex, zIndex);
            }
            glEnd();

            glDisable(GL_LINE_STIPPLE);
            glLineWidth(2.f); // line width
            glBegin(GL_LINES);
            FOR(y, h)
            {
                FOR(x, w)
                {
                    if(x < w-1) GLWidget::glLine(points[x+y*w], points[(x+1)+y*w], xIndex, yIndex, zIndex);
                    if(y < h-1) GLWidget::glLine(points[x+y*w], points[x+(y+1)*w], xIndex, yIndex, zIndex);
                }
            }
            glEnd();

            if(layer->elementAt(j)->getSuperNeuron())
            {
                glEnable(GL_LINE_STIPPLE); // enable dashed/ dotted lines
                glLineWidth(0.5f); // line width
                glLineStipple (1, 0xAAAA); // dash pattern AAAA: dots
                glBegin(GL_LINES);
                fvec anchor(dim);
                Neuron *neuron = layer->elementAt(j)->getSuperNeuron();
                if(neuron->weightsize>=dim)
                {
                    FOR(d, dim) anchor[d] = neuron->weights[d];
                    FOR(i, points.size()) GLWidget::glLine(anchor, points[i], xIndex, yIndex, zIndex);
                }
                glEnd();
            }
            glPopAttrib();

            glPushAttrib(GL_ALL_ATTRIB_BITS);
            glDisable( GL_LIGHTING);
            glEnable(GL_COLOR_MATERIAL);
            glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);

            glEnable(GL_TEXTURE_2D);
            glEnable(GL_POINT_SPRITE);
            glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
            glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            glPointSize(16.0);
            glBegin(GL_POINTS);
            FOR(i, points.size())
            {
                fvec point = points[i];
                QColor c = SampleColor[layerCount%SampleColorCnt];
                GLWidget::glSample(point, c, xIndex, yIndex, zIndex);
            }
            glEnd();

            layerCount--;
            glPopAttrib();
        }
    }
    glEndList();
    glw->drawSampleLists.push_back(list);
}

void DrawNeuronLayer(Canvas *canvas, QPainter &painter, NeuronLayer* layer, int label)
{
    if(!layer) return;
    Neuron ***map = layer->getMap();
    int w = layer->getX(), h = layer->getY();
    if(!w || !h) return;
    int dim = canvas->data->GetDimCount();

    vector< pair<QPointF, QPointF> > dataLines;
    vector<QPointF> points(w*h);
    FOR(y, h)
    {
        FOR(x, w)
        {
            Neuron *neuron = map[x][y];
            fvec mean(dim, 0);
            if(neuron->weightsize >= dim)
            {
                FOR(d, dim) mean[d] = neuron->weights[d];
            }
            points[x + y*w] = canvas->toCanvasCoords(mean);

            if(neuron->getRepresentingDataItems()->size())
            {
                FOR(i, neuron->getRepresentingDataItems()->size())
                {
                    DataItem *it = neuron->getRepresentingDataItems()->elementAt(i);
                    if(it->getLength() >= dim)
                    {
                        fvec point(dim);
                        FOR(d, dim) point[d] = it->getDataVector()[d];
                        dataLines.push_back(make_pair(canvas->toCanvasCoords(point), points[x + y*w]));
                    }
                }
            }
        }
    }

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(Qt::black, 0.5));
    FOR(i, dataLines.size())
    {
        painter.drawLine(dataLines[i].first, dataLines[i].second);
    }

    painter.setPen(QPen(Qt::black, 2));
    FOR(y, h)
    {
        FOR(x, w)
        {
            if(x < w-1) painter.drawLine(points[x+y*w], points[(x+1)+y*w]);
            if(y < h-1) painter.drawLine(points[x+y*w], points[x+(y+1)*w]);
        }
    }

    if(layer->getSuperNeuron())
    {
        fvec anchor(dim);
        Neuron *neuron = layer->getSuperNeuron();
        if(neuron->weightsize>=dim)
        {
            FOR(d, dim) anchor[d] = neuron->weights[d];
            painter.setPen(QPen(Qt::black, 0.5));
            QPointF p1 = canvas->toCanvasCoords(anchor);
            FOR(i, points.size()) painter.drawLine(p1, points[i]);
        }
    }

    painter.setPen(QPen(Qt::black, 3));
    painter.setBrush(SampleColor[label%SampleColorCnt]);
    FOR(i, points.size())
    {
        painter.drawEllipse(points[i], 8, 8);
    }
}

void GHSOMProjector::DrawModel(Canvas *canvas, QPainter &painter, Projector *projector)
{
    if(!canvas || !projector) return;
    if(canvas->canvasType) return;
    int dim = canvas->data->GetDimCount();

    NeuronLayer *layer = Globals::hfm->getLayer1Map();

    int layerCount = 0;
    for(int i=Globals::layers->size()-1; i>0; i--)
    {
		GVector<NeuronLayer> *layer = Globals::layers->elementAt(i);
        layerCount += layer->size();
    }
    for(int i=Globals::layers->size()-1; i>0; i--) // we skip the first as it is just the average of all weights
    {
		GVector<NeuronLayer> *layer = Globals::layers->elementAt(i);
        FOR(j, layer->size()) DrawNeuronLayer(canvas, painter, layer->elementAt(j), --layerCount);
    }

    printf("-----------------------\n");
    printf("Neurons layer structure\n");
    printf("-----------------------\n");
    printf("\nlayer count: %d\n", Globals::layers->size());
    FOR(i, Globals::layers->size())
    {
        printf("Layer[%d]", i);
		GVector<NeuronLayer> *layer = Globals::layers->elementAt(i);
        if(layer)
        {
            printf(": %d element(s)\n", layer->size());
            FOR(j, layer->size())
            {
                NeuronLayer *le = layer->elementAt(j);
                if(!le) continue;
                printf("\t(%d x %d) units\t0x%x\n", le->getX(), le->getY(), le->getSuperNeuron());
                Neuron ***map = le->getMap();
                FOR(y, le->getY())
                {
                    FOR(x, le->getX())
                    {
                        printf("\t[%d]x[%d]: ", x, y);
                        FOR(i, map[x][y]->weightsize) printf("%.2f ",map[x][y]->weights[i]);
                        printf("\n");
                    }
                }
            }
        }
        else printf("\n");
    }
    fflush(stdout);

    if(!canvas->data->bProjected) // We are displaying a Manifold to 1D
    {
        painter.setRenderHint(QPainter::Antialiasing);
    }
}

// virtual functions to manage the GUI and I/O
QString GHSOMProjector::GetAlgoString()
{
    return QString("GHSOM");
}

void GHSOMProjector::SaveOptions(QSettings &settings)
{
    settings.setValue("tau1Spin", params->tau1Spin->value());
    settings.setValue("tau2Spin", params->tau2Spin->value());
    settings.setValue("learnRateSpin", params->learnRateSpin->value());
    settings.setValue("nrSpin", params->nrSpin->value());
    settings.setValue("xSizeSpin", params->xSizeSpin->value());
    settings.setValue("ySizeSpin", params->ySizeSpin->value());
    settings.setValue("expandSpin", params->expandSpin->value());
    settings.setValue("normalizationCombo", params->normalizationCombo->currentIndex());
}

bool GHSOMProjector::LoadOptions(QSettings &settings)
{
    if(settings.contains("tau1Spin")) params->tau1Spin->setValue(settings.value("tau1Spin").toFloat());
    if(settings.contains("tau2Spin")) params->tau2Spin->setValue(settings.value("tau2Spin").toFloat());
    if(settings.contains("learnRateSpin")) params->learnRateSpin->setValue(settings.value("learnRateSpin").toFloat());
    if(settings.contains("nrSpin")) params->nrSpin->setValue(settings.value("nrSpin").toFloat());
    if(settings.contains("xSizeSpin")) params->xSizeSpin->setValue(settings.value("xSizeSpin").toInt());
    if(settings.contains("ySizeSpin")) params->ySizeSpin->setValue(settings.value("ySizeSpin").toInt());
    if(settings.contains("expandSpin")) params->expandSpin->setValue(settings.value("expandSpin").toInt());
    if(settings.contains("normalizationCombo")) params->normalizationCombo->setCurrentIndex(settings.value("normalizationCombo").toInt());
    return true;
}

void GHSOMProjector::SaveParams(QTextStream &file)
{
    file << "projectOptions" << ":" << "tau1Spin" << " " << params->tau1Spin->value() << "\n";
    file << "projectOptions" << ":" << "tau2Spin" << " " << params->tau2Spin->value() << "\n";
    file << "projectOptions" << ":" << "learnRateSpin" << " " << params->learnRateSpin->value() << "\n";
    file << "projectOptions" << ":" << "nrSpin" << " " << params->nrSpin->value() << "\n";
    file << "projectOptions" << ":" << "xSizeSpin" << " " << params->xSizeSpin->value() << "\n";
    file << "projectOptions" << ":" << "ySizeSpin" << " " << params->ySizeSpin->value() << "\n";
    file << "projectOptions" << ":" << "expandSpin" << " " << params->expandSpin->value() << "\n";
    file << "projectOptions" << ":" << "normalizationCombo" << " " << params->normalizationCombo->currentIndex() << "\n";
}

bool GHSOMProjector::LoadParams(QString name, float value)
{
    if(name.endsWith("tau1Spin")) params->tau1Spin->setValue((float)value);
    if(name.endsWith("tau2Spin")) params->tau2Spin->setValue((float)value);
    if(name.endsWith("learnRateSpin")) params->learnRateSpin->setValue((float)value);
    if(name.endsWith("nrSpin")) params->nrSpin->setValue((float)value);
    if(name.endsWith("xSizeSpin")) params->xSizeSpin->setValue((int)value);
    if(name.endsWith("ySizeSpin")) params->ySizeSpin->setValue((int)value);
    if(name.endsWith("expandSpin")) params->expandSpin->setValue((int)value);
    if(name.endsWith("normalizationCombo")) params->normalizationCombo->setCurrentIndex((int)value);
    return true;
}

Q_EXPORT_PLUGIN2(mld_GHSOM, GHSOMProjector)
