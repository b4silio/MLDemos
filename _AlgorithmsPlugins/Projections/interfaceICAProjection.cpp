#include "interfaceICAProjection.h"
#include "projectorICA.h"

ICAProjection::ICAProjection()
    : widget(new QWidget())
{
    params = new Ui::paramsICA();
    params->setupUi(widget);
}

// virtual functions to manage the algorithm creation
Projector *ICAProjection::GetProjector()
{
    return new ProjectorICA();
}

void ICAProjection::DrawInfo(Canvas *canvas, QPainter &painter, Projector *projector)
{
    if(!canvas || !projector) return;
    ProjectorICA *ica = dynamic_cast<ProjectorICA*>(projector);
    if(!ica) return;
    int dim = projector->dim;
    double *trans = ica->GetTransf();
    params->mixingTable->clear();
    params->mixingTable->setRowCount(dim);
    params->mixingTable->setColumnCount(dim);
    FOR(i, dim) params->mixingTable->setColumnWidth(i, max(30, (params->mixingTable->width()-16)/dim));
    FOR(i, dim)
    {
        FOR(j, dim)
        {
            QTableWidgetItem *newItem = new  QTableWidgetItem(QString("%1").arg(trans[i*dim + j], 0, 'f', 3));
            params->mixingTable->setItem(i, j, newItem);
        }
    }
}

void ICAProjection::DrawModel(Canvas *canvas, QPainter &painter, Projector *projector)
{
    if(!canvas || !projector) return;
    if(canvas->canvasType) return;
}

// virtual functions to manage the GUI and I/O
QString ICAProjection::GetAlgoString()
{
    return QString("ICA");
}

void ICAProjection::SetParams(Projector *projector)
{
    if(!projector) return;
    ((ProjectorICA*) projector)->method = params->methodCombo->currentIndex();
}

void ICAProjection::SaveOptions(QSettings &settings)
{
    //settings.setValue("kernelCluster", params->kernelClusterSpin->value());
}

bool ICAProjection::LoadOptions(QSettings &settings)
{
    //if(settings.contains("kernelCluster")) params->kernelClusterSpin->setValue(settings.value("kernelCluster").toFloat());
    return true;
}

void ICAProjection::SaveParams(QTextStream &file)
{
    //file << "clusterOptions" << ":" << "kernelCluster" << " " << params->kernelClusterSpin->value() << "\n";
}

bool ICAProjection::LoadParams(QString name, float value)
{
    //if(name.endsWith("kernelCluster")) params->kernelClusterSpin->setValue((int)value);
    return true;
}
