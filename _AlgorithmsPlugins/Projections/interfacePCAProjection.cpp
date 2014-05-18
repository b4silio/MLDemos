#include "interfacePCAProjection.h"
#include "projectorPCA.h"
#include <QDebug>

PCAProjection::PCAProjection()
    : widget(new QWidget()), eigenWidget(0)
{
    params = new Ui::paramsPCA();
    params->setupUi(widget);

    connect(params->showEigenvectorButton, SIGNAL(clicked()), this, SLOT(ShowEigenVectors()));
    connect(params->useRangeCheck, SIGNAL(clicked()), this, SLOT(ChangeOptions()));
    ChangeOptions();
}

PCAProjection::~PCAProjection()
{
    delete params;
    if(eigenWidget) delete eigenWidget;
}

void PCAProjection::ChangeOptions()
{
    bool bRange = params->useRangeCheck->isChecked();
    params->startRangeSpin->setVisible(bRange);
    params->stopRangeSpin->setVisible(bRange);
    params->labelStart->setVisible(bRange);
    params->labelStop->setVisible(bRange);
}

// virtual functions to manage the algorithm creation
Projector *PCAProjection::GetProjector()
{
    return new ProjectorPCA();
}

fvec PCAProjection::GetParams()
{
    return fvec();
}

void PCAProjection::GetParameterList(std::vector<QString> &parameterNames,
                             std::vector<QString> &parameterTypes,
                             std::vector< std::vector<QString> > &parameterValues){}

void PCAProjection::ShowEigenVectors()
{
    if(!eigenWidget) return;
    if(eigenWidget->isVisible()) eigenWidget->hide();
    else eigenWidget->show();
}

void PCAProjection::DrawInfo(Canvas *canvas, QPainter &painter, Projector *projector)
{
    if(!canvas || !projector) return;
    ProjectorPCA *pca = dynamic_cast<ProjectorPCA*>(projector);
    if(!pca) return;
    QPixmap pixmap(params->eigenGraph->width(), params->eigenGraph->height());
    //QBitmap bitmap(pixmap.width(), pixmap.height());
    //bitmap.clear();
    //pixmap.setMask(bitmap);
    pixmap.fill(Qt::transparent);
    QPainter eigenPainter(&pixmap);
    pca->DrawEigenvals(eigenPainter);
    params->eigenGraph->setPixmap(pixmap);

    params->eigenList->clear();
    fvec values = pca->GetEigenValues();
    float accumulator = 0;
    float maxEigVal = 0;
    FOR(i, values.size()) if(values[i] == values[i] && values[i] >= 0) maxEigVal += values[i];

    FOR(i, values.size())
    {
        float eigval = values[i];
        if(eigval == eigval && eigval >= 0)
        {
            accumulator += eigval / maxEigVal;
        }
        else eigval = 0;
        params->eigenList->addItem(QString("%1: %2 %3%%").arg(i+1).arg(eigval, 0, 'f', 2).arg(eigval/maxEigVal*100, 0, 'f', 1));
    }
    vector<fvec> eigenVec = pca->GetEigenVectors();
    int dim = eigenVec.size();
    int eigenCount = eigenVec.size() ? eigenVec[0].size() : 2;
    if(!eigenWidget)
    {
        eigenWidget = new QWidget();
        QBoxLayout *layout = new QBoxLayout(QBoxLayout::LeftToRight, eigenWidget);
        eigenWidget->setWindowTitle("PCA EigenVectors");
        eigenTable = new QTableWidget(eigenCount, dim);
        layout->addWidget(eigenTable);
    }
    else
    {
        eigenTable->setColumnCount(dim);
        eigenTable->setRowCount(eigenCount);
    }
    FOR(i, dim)
    {
        if(i >= eigenVec.size()) break;
        FOR(j, eigenCount)
        {
            if(j >= eigenVec[i].size()) break;
            QTableWidgetItem *item = new QTableWidgetItem(QString("%1").arg(eigenVec[i][j], 0, 'f', 4));
            eigenTable->setItem(j,i, item);
        }
    }
    QStringList labels;
    FOR(i, dim)
    {
        labels << QString("e%1: %2").arg(i+1).arg(values[i], 0, 'f', 3);
    }
    eigenTable->setHorizontalHeaderLabels(labels);
}

void PCAProjection::DrawModel(Canvas *canvas, QPainter &painter, Projector *projector)
{
    if(!canvas || !projector) return;
    ProjectorPCA *pca = dynamic_cast<ProjectorPCA*>(projector);
    if(!pca) return;
    if(canvas->data->bProjected) return; // We are displaying a Manifold to 1D
    vector<fvec> eigs = pca->GetEigenVectors();
    if(!eigs.size()) return;

    fvec topleft = canvas->fromCanvas(0,0);
    fvec bottomRight = canvas->fromCanvas(canvas->width(), canvas->height());
    fvec diff = (topleft - bottomRight);
    float canvasSize = sqrtf(diff*diff);

    fvec stop = eigs[0];
    int dim = stop.size();
    stop /= sqrtf(stop*stop);
    stop *= canvasSize/2;

    fvec start = stop*-1;
    // we look for the average
    fvec mean(dim,0);
    FOR(i, pca->source.size()) mean += pca->source[i];
    mean /= pca->source.size();
    stop += mean;
    start += mean;
    QPointF p1 = canvas->toCanvasCoords(start);
    QPointF p2 = canvas->toCanvasCoords(stop);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(Qt::black, 2));
    painter.drawLine(p1, p2);
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
        int startIndex = params->startRangeSpin->value()-1;
        int stopIndex = params->stopRangeSpin->value()-1;
        projector->startIndex = min(startIndex, stopIndex);
        projector->stopIndex = max(startIndex, stopIndex);
    }
    else
    {
        projector->startIndex = 0;
        projector->stopIndex = -1;
    }
}

void PCAProjection::SetParams(Projector *projector, fvec parameters){}

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
    //file << "projectOptions" << ":" << "kernelCluster" << " " << params->kernelClusterSpin->value() << "\n";
}

bool PCAProjection::LoadParams(QString name, float value)
{
    //if(name.endsWith("kernelCluster")) params->kernelClusterSpin->setValue((int)value);
    return true;
}
