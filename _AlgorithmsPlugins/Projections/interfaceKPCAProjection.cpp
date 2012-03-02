#include "interfaceKPCAProjection.h"
#include "projectorKPCA.h"
#include <QDebug>
#include <QImage>

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
    case 0: // poly
        params->kernelDegSpin->setEnabled(true);
        params->kernelDegSpin->setVisible(true);
        params->kernelWidthSpin->setEnabled(false);
        params->kernelWidthSpin->setVisible(false);
        break;
    case 1: // RBF
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
    return new ProjectorKPCA(params->dimCountSpin->value());
}

void KPCAProjection::DrawInfo(Canvas *canvas, QPainter &painter, Projector *projector)
{
    if(!canvas || !projector) return;
    /*
    ProjectorKPCA *kpca = dynamic_cast<ProjectorKPCA*>(projector);
    if(!kpca) return;
    painter.setRenderHint(QPainter::Antialiasing);

    int w = canvas->width();
    int h = canvas->height();
    FOR(i, w/4)
    {
        FOR(j, h/4)
        {
            fvec sample = canvas->toSampleCoords(i*4, j*4);
            float value = kpca->Project(sample)[0];
            qDebug() << value;
            value = min(255.f,value*255);
            value = max(0.f,value);
            painter.setPen(QPen(QColor(value,value,value), 2));
            painter.drawLine(QPointF(i*4,j*4), QPointF(i*4,j*4));
        }
    }
    */

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
    ProjectorKPCA *kpca = dynamic_cast<ProjectorKPCA*>(projector);
    if(!kpca) return;
    painter.setRenderHint(QPainter::Antialiasing, false);
    PCA *pca = kpca->pca;

    int W = canvas->width();
    int H = canvas->height();

    int xIndex = canvas->xIndex;
    int yIndex = canvas->yIndex;
    float xmin=FLT_MAX,xmax=-FLT_MAX;
    float ymin=FLT_MAX,ymax=-FLT_MAX;
    vector<fvec> samples = projector->source;
    fvec mean;
    FOR(i, samples.size())
    {
        xmin=min(samples[i][xIndex], xmin);
        xmax=max(samples[i][xIndex], xmax);
        ymin=min(samples[i][yIndex], ymin);
        ymax=max(samples[i][yIndex], ymax);
        if(!i) mean = samples[0];
        else mean += samples[i];
    }
    mean /= samples.size();

    int w = 64;
    int h = 64;
    QImage image(w,h,QImage::Format_RGB32);
    float *values = new float[w*h];
    float vmin = FLT_MAX, vmax = -FLT_MAX;
    int dim = mean.size();
    VectorXd point(dim);
    FOR(d,dim) point(d) = mean[d];
    FOR(i, w)
    {
        FOR(j, h)
        {
            point(xIndex) = i/(float)w*(xmax-xmin) + xmin;
            point(yIndex) = j/(float)h*(ymax-ymin) + ymin;
            VectorXd res = pca->project(point);
            float value = res(0);
            vmin = min(value, vmin);
            vmax = max(value, vmax);
            values[j*w + i] = value;
        }
    }
    FOR(i, w)
    {
        FOR(j, h)
        {
            int value = (int)((values[i,j]-vmin)/(vmax-vmin)*255.f);
            image.setPixel(i,j, qRgb((int)value,value,value));
        }
    }
    QPixmap contours = QPixmap::fromImage(image).scaled(W,H, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    painter.drawPixmap(0,0,contours);

    /*
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
    */
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
    // we add 1 to the kernel type because we have taken out the linear kernel
    kpca->SetParams(params->kernelTypeCombo->currentIndex()+1, params->kernelDegSpin->value(), params->kernelWidthSpin->value());
}

void KPCAProjection::SaveOptions(QSettings &settings)
{
    settings.setValue("kernelTypeCombo", params->kernelTypeCombo->currentIndex());
    settings.setValue("kernelDegSpin", params->kernelDegSpin->value());
    settings.setValue("kernelWidthSpin", params->kernelWidthSpin->value());
    settings.setValue("dimCountSpin", params->dimCountSpin->value());
}

bool KPCAProjection::LoadOptions(QSettings &settings)
{
    if(settings.contains("kernelTypeCombo")) params->kernelTypeCombo->setCurrentIndex(settings.value("kernelTypeCombo").toInt());
    if(settings.contains("kernelDegSpin")) params->kernelDegSpin->setValue(settings.value("kernelDegSpin").toInt());
    if(settings.contains("kernelWidthSpin")) params->kernelWidthSpin->setValue(settings.value("kernelWidthSpin").toFloat());
    if(settings.contains("dimCountSpin")) params->dimCountSpin->setValue(settings.value("dimCountSpin").toInt());
    ChangeOptions();
    return true;
}

void KPCAProjection::SaveParams(QTextStream &file)
{
    file << "clusterOptions" << ":" << "kernelTypeCombo" << " " << params->kernelTypeCombo->currentIndex() << "\n";
    file << "clusterOptions" << ":" << "kernelDegSpin" << " " << params->kernelDegSpin->value() << "\n";
    file << "clusterOptions" << ":" << "kernelWidthSpin" << " " << params->kernelWidthSpin->value() << "\n";
    file << "clusterOptions" << ":" << "dimCountSpin" << " " << params->dimCountSpin->value() << "\n";
}

bool KPCAProjection::LoadParams(QString name, float value)
{
    if(name.endsWith("kernelTypeCombo")) params->kernelTypeCombo->setCurrentIndex((int)value);
    if(name.endsWith("kernelDegSpin")) params->kernelDegSpin->setValue(value);
    if(name.endsWith("kernelWidthSpin")) params->kernelWidthSpin->setValue(value);
    if(name.endsWith("dimCountSpin")) params->dimCountSpin->setValue((int)value);
    ChangeOptions();
    return true;
}
