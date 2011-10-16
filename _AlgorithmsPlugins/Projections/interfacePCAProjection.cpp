#include "interfacePCAProjection.h"
#include "projectorPCA.h"

PCAProjection::PCAProjection()
    : widget(new QWidget())
{
    params = new Ui::paramsPCA();
    params->setupUi(widget);
}


// virtual functions to manage the algorithm creation
Projector *PCAProjection::GetProjector()
{
    return new ProjectorPCA();
}

void PCAProjection::DrawInfo(Canvas *canvas, QPainter &painter, Projector *projector)
{
    if(!canvas || !projector) return;
    ProjectorPCA *pca = (ProjectorPCA*)projector;
    QPixmap pixmap(params->eigenGraph->width(), params->eigenGraph->height());
    QBitmap bitmap(pixmap.width(), pixmap.height());
    bitmap.clear();
    pixmap.setMask(bitmap);
    pixmap.fill(Qt::transparent);
    QPainter eigenPainter(&pixmap);
    pca->DrawEigenvals(eigenPainter);
    params->eigenGraph->setPixmap(pixmap);

    params->eigenList->clear();
    fvec values = pca->GetEigenValues();
    float accumulator = 0;
    float maxEigVal = 0;
    FOR(i, values.size()) if(values[i] == values[i]) maxEigVal += values[i];

    FOR(i, values.size())
    {
        float eigval = values[i];
        if(eigval == eigval)
        {
            accumulator += eigval / maxEigVal;
        }
        params->eigenList->addItem(QString("e%1 %2 %3%%").arg(i).arg(eigval, 0, 'f', 2).arg(accumulator*100, 0, 'f', 1));
    }
}

void PCAProjection::DrawModel(Canvas *canvas, QPainter &painter, Projector *projector)
{
    if(!canvas || !projector) return;
}

// virtual functions to manage the GUI and I/O
QString PCAProjection::GetAlgoString()
{
    return QString("PCA");
}

void PCAProjection::SetParams(Projector *projector)
{
    if(!projector) return;
    if(params->useRangeCheck->isChecked())
    {
        projector->startIndex = params->startRangeSpin->value()-1;
        projector->stopIndex = params->stopRangeSpin->value()-1;
    }
    else
    {
        projector->startIndex = 0;
        projector->stopIndex = -1;
    }
}

void PCAProjection::SaveOptions(QSettings &settings)
{
    //settings.setValue("kernelCluster", params->kernelClusterSpin->value());
}

bool PCAProjection::LoadOptions(QSettings &settings)
{
    //if(settings.contains("kernelCluster")) params->kernelClusterSpin->setValue(settings.value("kernelCluster").toFloat());
    return true;
}

void PCAProjection::SaveParams(QTextStream &file)
{
    //file << "clusterOptions" << ":" << "kernelCluster" << " " << params->kernelClusterSpin->value() << "\n";
}

bool PCAProjection::LoadParams(QString name, float value)
{
    //if(name.endsWith("kernelCluster")) params->kernelClusterSpin->setValue((int)value);
    return true;
}
