#include "interfaceLDAProjection.h"
#include "projectorLDA.h"
#include <QDebug>

using namespace std;

LDAProjection::LDAProjection()
    : widget(new QWidget())
{
    params = new Ui::paramsLDA();
    params->setupUi(widget);
}

LDAProjection::~LDAProjection()
{
    delete params;
}

// virtual functions to manage the algorithm creation
Projector *LDAProjection::GetProjector()
{
    return new ProjectorLDA();
}

fvec LDAProjection::GetParams()
{
    return fvec();
}

void LDAProjection::SetParams(Projector *projector, fvec parameters){}

void LDAProjection::GetParameterList(std::vector<QString> &parameterNames,
                             std::vector<QString> &parameterTypes,
                             std::vector< std::vector<QString> > &parameterValues){}

void LDAProjection::DrawInfo(Canvas *canvas, QPainter &painter, Projector *projector)
{
    if(!canvas || !projector) return;
    if(canvas->canvasType) return;
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
}

void LDAProjection::DrawModel(Canvas *canvas, QPainter &painter, Projector *projector)
{
    if(!canvas || !projector) return;
    if(canvas->canvasType) return;
    vector<fvec> samples = projector->source;
    vector<fvec> projected = projector->projected;
    ivec labels = canvas->data->GetLabels();
    if(!samples.size()) return;
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(Qt::black,0.5f));
    FOR(i, samples.size())
    {
        projected[i] = projector->Project(samples[i]);
        QPointF p1 = canvas->toCanvasCoords(samples[i]);
        QPointF p2 = canvas->toCanvasCoords(projected[i]);
        painter.drawLine(p1, p2);
    }
    painter.setOpacity(0.3);
    FOR(i, samples.size())
    {
        painter.setBrush(SampleColor[labels[i]%SampleColorCnt]);
        painter.setPen(Qt::black);
        QPointF p1 = canvas->toCanvasCoords(samples[i]);
        painter.drawEllipse(p1, 5, 5);
    }
    painter.setPen(Qt::black);
    FOR(i, projected.size())
    {
        painter.setBrush(SampleColor[labels[i]%SampleColorCnt]);
        QPointF p1 = canvas->toCanvasCoords(projected[i]);
        painter.drawEllipse(p1, 5, 5);
    }
}

// virtual functions to manage the GUI and I/O
QString LDAProjection::GetAlgoString()
{
    return QString("LDA");
}

void LDAProjection::SetParams(Projector *projector)
{
    if(!projector) return;
    ((ProjectorLDA*) projector)->ldaType = params->typeCombo->currentIndex();
}

void LDAProjection::SaveOptions(QSettings &settings)
{
    settings.setValue("typeCombo", params->typeCombo->currentIndex());
}

bool LDAProjection::LoadOptions(QSettings &settings)
{
    if(settings.contains("typeCombo")) params->typeCombo->setCurrentIndex(settings.value("typeCombo").toInt());
    return true;
}

void LDAProjection::SaveParams(QTextStream &file)
{
    file << "projectOptions" << ":" << "typeCombo" << " " << params->typeCombo->currentIndex() << "\n";
}

bool LDAProjection::LoadParams(QString name, float value)
{
    if(name.endsWith("typeCombo")) params->typeCombo->setCurrentIndex((int)value);
    return true;
}
