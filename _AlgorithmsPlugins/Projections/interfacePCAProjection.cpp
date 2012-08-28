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
    /*
    if(canvas->canvasType) return;
    vector<fvec> samples = projector->source;
    vector<fvec> projected = projector->projected;
    if(!projected.size() || !samples.size()) return;
    if(projected[0].size() > samples[0].size()) return;
    ivec labels = canvas->data->GetLabels();
    qDebug() << "projected: " << projected[0].size() << "samples: " << samples[0].size();

    int pDim = projected[0].size(), sDim = samples[0].size();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(Qt::black,0.5f));
    FOR(i, samples.size())
    {
        //projected[i] = projector->Project(samples[i]);
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
    */
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
