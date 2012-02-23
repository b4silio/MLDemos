#include "interfaceKPCAProjection.h"
#include "projectorKPCA.h"
#include <QDebug>

using namespace std;

KPCAProjection::KPCAProjection()
    : widget(new QWidget())
{
    params = new Ui::paramsKPCA();
    params->setupUi(widget);
    connect(params->kernelTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeOptions()));
}

void KPCAProjection::ChangeOptions()
{
    switch(params->kernelTypeCombo->currentIndex())
    {
    case 0: // linear
        params->kernelDegSpin->setEnabled(false);
        params->kernelDegSpin->setVisible(false);
        break;
    case 1: // poly
        params->kernelDegSpin->setEnabled(true);
        params->kernelDegSpin->setVisible(true);
        params->kernelWidthSpin->setEnabled(false);
        params->kernelWidthSpin->setVisible(false);
        break;
    case 2: // RBF
        params->kernelDegSpin->setEnabled(false);
        params->kernelDegSpin->setVisible(false);
        params->kernelWidthSpin->setEnabled(true);
        params->kernelWidthSpin->setVisible(true);
        break;
    }
}

// virtual functions to manage the algorithm creation
Projector *KPCAProjection::GetProjector()
{
    return new ProjectorKPCA();
}

void KPCAProjection::DrawInfo(Canvas *canvas, QPainter &painter, Projector *projector)
{
    if(!canvas || !projector) return;
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

void KPCAProjection::DrawModel(Canvas *canvas, QPainter &painter, Projector *projector)
{
    if(!canvas || !projector) return;
    vector<fvec> samples = projector->source;
    vector<fvec> projected = projector->projected;
    ivec labels = canvas->data->GetLabels();
    if(!samples.size()) return;
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setOpacity(0.2);
    painter.setPen(QPen(Qt::black,0.5f));
    FOR(i, samples.size())
    {
        //projected[i] = projector->Project(samples[i]);
        QPointF p1 = canvas->toCanvasCoords(samples[i]);
        QPointF p2 = canvas->toCanvasCoords(projected[i]);
        painter.drawLine(p1, p2);
    }
    painter.setOpacity(0.2);
    FOR(i, samples.size())
    {
        painter.setBrush(SampleColor[labels[i]%SampleColorCnt]);
        painter.setPen(Qt::black);
        QPointF p1 = canvas->toCanvasCoords(samples[i]);
        painter.drawEllipse(p1, 5, 5);
    }
}

// virtual functions to manage the GUI and I/O
QString KPCAProjection::GetAlgoString()
{
    return QString("KPCA");
}

void KPCAProjection::SetParams(Projector *projector)
{
    if(!projector) return;
    ProjectorKPCA *kpca = dynamic_cast<ProjectorKPCA*>(projector);
    if(!kpca) return;
    kpca->SetParams(params->kernelTypeCombo->currentIndex(), params->kernelDegSpin->value(), params->kernelWidthSpin->value());
}

void KPCAProjection::SaveOptions(QSettings &settings)
{
    settings.setValue("kernelTypeCombo", params->kernelTypeCombo->currentIndex());
    settings.setValue("kernelDegSpin", params->kernelDegSpin->value());
    settings.setValue("kernelWidthSpin", params->kernelWidthSpin->value());
}

bool KPCAProjection::LoadOptions(QSettings &settings)
{
    if(settings.contains("kernelTypeCombo")) params->kernelTypeCombo->setCurrentIndex(settings.value("kernelTypeCombo").toInt());
    if(settings.contains("kernelDegSpin")) params->kernelDegSpin->setValue(settings.value("kernelDegSpin").toInt());
    if(settings.contains("kernelWidthSpin")) params->kernelWidthSpin->setValue(settings.value("kernelWidthSpin").toFloat());
    ChangeOptions();
    return true;
}

void KPCAProjection::SaveParams(QTextStream &file)
{
    file << "clusterOptions" << ":" << "kernelTypeCombo" << " " << params->kernelTypeCombo->currentIndex() << "\n";
    file << "clusterOptions" << ":" << "kernelDegSpin" << " " << params->kernelDegSpin->value() << "\n";
    file << "clusterOptions" << ":" << "kernelWidthSpin" << " " << params->kernelWidthSpin->value() << "\n";
}

bool KPCAProjection::LoadParams(QString name, float value)
{
    if(name.endsWith("kernelTypeCombo")) params->kernelTypeCombo->setCurrentIndex((int)value);
    if(name.endsWith("kernelDegSpin")) params->kernelDegSpin->setValue(value);
    if(name.endsWith("kernelWidthSpin")) params->kernelWidthSpin->setValue(value);
    ChangeOptions();
    return true;
}
