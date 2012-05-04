#include "interfaceKPCAProjection.h"
#include "projectorKPCA.h"
#include <QDebug>
#include <QImage>
#include <QClipboard>
#include <qcontour.h>
#include <algorithm>

using namespace std;

KPCAProjection::KPCAProjection()
    : widget(new QWidget()), contourLabel(0), pcaPointer(0), contourWidget(new QWidget()),
      xIndex(0), yIndex(1)
{
    params = new Ui::paramsKPCA();
    params->setupUi(widget);
    contours = new Ui::ContourWidget();
    contours->setupUi(contourWidget);
    contourWidget->layout()->setSizeConstraint( QLayout::SetFixedSize );
    contourWidget->setWindowTitle("Kernel Eigenvector Projections");

    connect(params->kernelTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeOptions()));
    connect(params->contourButton, SIGNAL(clicked()), this, SLOT(ShowContours()));
    connect(contours->dimSpin, SIGNAL(valueChanged(int)), this, SLOT(DrawContours(int)));
    connect(contours->displayCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(ShowContours()));
    connect(contours->clipboardButton, SIGNAL(clicked()), this, SLOT(SaveScreenshot()));
    connect(contours->spinX1, SIGNAL(valueChanged(int)), this, SLOT(ContoursChanged()));
    connect(contours->spinX2, SIGNAL(valueChanged(int)), this, SLOT(ContoursChanged()));
    connect(contours->spinZoom, SIGNAL(valueChanged(int)), this, SLOT(ContoursChanged()));
}

void KPCAProjection::SaveScreenshot()
{
    const QPixmap *screenshot = contours->plotLabel->pixmap();
    if(screenshot->isNull()) return;

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setImage(screenshot->toImage());
    clipboard->setPixmap(*screenshot);
}

void KPCAProjection::ContoursChanged()
{
    contourPixmaps.clear();
    ShowContours();
}

void KPCAProjection::ShowContours()
{
    PCA *pca = dynamic_cast<PCA*> (pcaPointer);
    if(!pca) return;
    DrawContours(contours->dimSpin->value());
    contourWidget->show();
}

void KPCAProjection::ChangeOptions()
{
    switch(params->kernelTypeCombo->currentIndex())
    {
    case 0: // poly
        params->kernelDegSpin->setEnabled(true);
        params->kernelDegSpin->setVisible(true);
        params->kernelWidthSpin->setEnabled(true);
        params->kernelWidthSpin->setVisible(true);
        params->kernelDegSpin->setDecimals(0);
        params->kernelDegSpin->setRange(1,999);
        params->kernelDegSpin->setSingleStep(1);
        params->kernelWidthSpin->setRange(-999,999);
        params->param1Label->setText("Degree");
        params->param2Label->setText("Offset");
        break;
    case 1: // RBF
        params->kernelDegSpin->setEnabled(false);
        params->kernelDegSpin->setVisible(false);
        params->param1Label->setText("");
        params->param2Label->setText("Width");
        params->kernelWidthSpin->setRange(0.001,999);
        params->kernelWidthSpin->setEnabled(true);
        params->kernelWidthSpin->setVisible(true);
        break;
    case 2: // TANH
        params->kernelDegSpin->setEnabled(true);
        params->kernelDegSpin->setVisible(true);
        params->kernelWidthSpin->setEnabled(true);
        params->kernelWidthSpin->setVisible(true);
        params->kernelDegSpin->setDecimals(3);
        params->kernelDegSpin->setRange(0.01,100);
        params->kernelDegSpin->setSingleStep(0.1);
        params->kernelWidthSpin->setRange(-999,999);
        params->param1Label->setText("Scale");
        params->param2Label->setText("Offset");
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
}

void KPCAProjection::GetContoursPixmap(int index)
{
    PCA *pca = dynamic_cast<PCA*>(pcaPointer);
    if(!pca) return;
    if(contourPixmaps.count(index)) return; // nothing to be done here, moving on!

    // we compute the density map
    int w = 65;
    int h = 65;
    int hmo = h-1; // we will drop one line at the edges to avoid weird border conditions
    int wmo = w-1;
    QImage image(wmo,hmo,QImage::Format_RGB32);
    int dim = pca->sourcePoints.rows();
    int zoom = 1;
    // use [xy]Index given in the constuctor if spinX1 [ov]
    if (contours->spinX1->isVisible()) {
        xIndex = contours->spinX1->value()-1;
        yIndex = contours->spinX2->value()-1;
        zoom = contours->spinZoom->value();
    }
    //qDebug() << "KPCAProjection::GetContoursPixmap - xIndex:" << xIndex << ", yIndex:" << yIndex << ", zoom:" << zoom << ".";
    double multiplier = 1000.; // this is used to avoid numerical instabilities when computing the contour lines
    VectorXd point(dim);
    FOR(d,dim) point(d) = 0.0;
    double xdiff = xmax - xmin;
    double ydiff = ymax - ymin;
    //qDebug() << "KPCAProjection::GetContoursPixmap - xmin:" << xmin << ", ymin:" << ymin << ", xman:" << xmax << ", ymax:" << ymax << ".";
    double *values = new double[w*h];
    double vmin = DBL_MAX, vmax = -DBL_MAX;

    double zxmin = xmin - xdiff*0.5*(zoom-1);
    double zxmax = xmax + xdiff*0.5*(zoom-1);
    double zxdiff = zxmax - zxmin;
    double zymin = ymin - ydiff*0.5*(zoom-1);
    double zymax = ymax + ydiff*0.5*(zoom-1);
    double zydiff = zymax - zymin;
    FOR(i, w)
    {
        FOR(j, h)
        {
            if ( xIndex < dim ) point( xIndex ) = ( (double)i / ( (double)w * zxdiff ) ) + zxmin;
            if ( yIndex < dim ) point( yIndex ) = ( (double)j / ( (double)h * zydiff ) ) + zymin;
            double value = pcaPointer->test( point, index-1, multiplier ); // indices start from 1 in params.dimCountSpin
            vmin = min(value, vmin);
            vmax = max(value, vmax);
            values[j*w + i] = value;
        }
    }
    //double zvmin=vmin, zvmax=vmax;
    if(zoom != 1)
    {
        /*
        zvmin = DBL_MAX;
        zvmax = -DBL_MAX;
        FOR(i, w)
        {
            FOR(j, h)
            {
                if(xIndex<dim) point(xIndex) = i/(double)w*(xmax-xmin) + xmin;
                if(yIndex<dim) point(yIndex) = j/(double)h*(ymax-ymin) + ymin;
                double value = pcaPointer->test(point, index-1, multiplier); // indices start from 1 in params.dimCountSpin
                zvmin = min(value, zvmin);
                zvmax = max(value, zvmax);
            }
        }
        */
    }
    double vdiff=vmax-vmin;
    //qDebug() << "KPCAProjection::GetContoursPixmap - vmin:" << vmin << ", vmax:" << vmax << " - vdiff: " << vdiff << ".";
    if(vdiff == 0) vdiff = 1.f;
    FOR(i, wmo)
    {
        FOR(j, hmo)
        {
            int value = (int)((values[j*w + i]-vmin)/vdiff*255);
            image.setPixel(i,j, qRgb((int)value,value,value));
        }
    }
    QPixmap contourPixmap = QPixmap::fromImage(image).scaled(512,512, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    int W = contourPixmap.width();
    int H = contourPixmap.height();
    QPainter painter(&contourPixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // we plot the samples
    painter.setPen(Qt::black);
    painter.setBrush(Qt::white);
    painter.setOpacity(1);
    FOR(i, contourSamples.size())
    {
        fvec &sample = contourSamples[i];
        //qDebug() << "drawing sample at " << sample[0] << sample[1];
        float x = (sample[xIndex]-zxmin)/(zxmax-zxmin)*w;
        float y = (sample[yIndex]-zymin)/(zymax-zymin)*h;
        x = (x+1)*W/w;
        y = (y+1)*H/h;
        Canvas::drawSample(painter, QPointF(x,y), 10, contourSampleLabels[i]);
    }

    // we plot the contour lines
    if(contourSamples.size())
    {
        QContour contour(values, w, h);
        contour.bDrawColorbar = true;
        //contour.SetLimits(zvmin, zvmax);
        contour.Paint(painter, 20, zoom);
    }

    contourPixmaps[index] = contourPixmap;
    delete [] values;
}

void KPCAProjection::DrawContours(int index)
{
    PCA *pca = dynamic_cast<PCA*>(pcaPointer);
    if(!pca) return;
    int displayType = contours->displayCombo->currentIndex();
    if ( ( displayType < 0 ) or ( displayType > 1 ) ) {
        qDebug() << "KPCAProjection::DrawContours - displayType out of range :" << displayType;
        displayType = 0;
    }

    switch(displayType)
    {
    case 0: // single
    {
        // ensure that we have the right pixmap
        GetContoursPixmap(index);
        contours->plotLabel->setPixmap(contourPixmaps[index]);
    }
        break;
    case 1: // take all the values and draw them
    {
        int maximum = contours->dimSpin->maximum();
        for(int i=1; i<=contours->dimSpin->maximum(); i++)
        {
            GetContoursPixmap(i);
        }
        int gridX = std::ceil(sqrtf(maximum));
        //int gridY = std::ceil(maximum / (float)gridX);
        int gridY = gridX;

        int w = contourPixmaps[1].width();
        int h = contourPixmaps[1].height();
        QPixmap bigPixmap(gridX*w, gridX*h);
        QBitmap bitmap(bigPixmap.width(), bigPixmap.height());
        bitmap.clear();
        bigPixmap.setMask(bitmap);
        bigPixmap.fill(Qt::transparent);
        QPainter painter(&bigPixmap);
        for(int i=1; i<=contours->dimSpin->maximum(); i++)
        {
            int x = ((i-1)%gridX)*w;
            int y = ((i-1)/gridX)*h;
            QRect rect(x,y,w,h);
            painter.drawPixmap(rect, contourPixmaps[i], QRect(0,0,w,h));
        }
        contours->plotLabel->setPixmap(bigPixmap.scaled(QSize(w,h), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    }
        break;
    }
    if(contourWidget->isVisible()) contours->plotLabel->repaint();
}

void DrawEigenvals(QPainter &painter, fvec eigs, bool bSkipFirstEigenvector)
{
    int w=painter.window().width();
    int h=painter.window().height();
    int pad = 5;

    int dim = eigs.size();
    float maxEigVal = 1.f;
    if(dim > 2) maxEigVal = bSkipFirstEigenvector ? eigs[1] : eigs[0];
    else if(dim) maxEigVal = eigs[0];

    /*
    FOR(i, dim)
    {
        if(!i && bSkipFirstEigenvector) continue;
        if(eigs[i] == eigs[i]) maxEigVal += eigs[i];
    }

    maxEigVal = max(1.f,maxEigVal);
    float maxAccumulator = 0;
    FOR(i, dim) if(eigs[i] == eigs[i]) maxAccumulator += eigs[i];

    float accumulator = 0;
    */

    painter.setPen(Qt::gray);
    painter.drawLine(QPointF(pad, h-2*pad), QPointF(w-pad, h-2*pad));
    painter.drawLine(QPointF(pad, pad), QPointF(pad, h-2*pad));
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);
    int rectW = (w-2*pad) / (dim-1) - 2;
    FOR(i, dim)
    {
        float eigval = eigs[i];

        int x = dim==1 ? w/2 : i * (w-2*pad) / dim + pad;
        int y = (int)(eigval/maxEigVal * (h-2*pad));
        y = min(y, h-2*pad);
        painter.drawRect(x,h-2*pad,rectW,-y);
    }
    painter.setBrush(Qt::NoBrush);
    //painter.drawLine(point, QPoint(w-2*pad, h-2*pad));
    QFont font = painter.font();
    font.setPointSize(8);
    painter.setFont(font);
    painter.setPen(Qt::black);
    painter.drawText(0,0,w,2*pad,Qt::AlignCenter, "eigenvalues");
    int step = 1;
    while((dim/step > 8)) step++;
    for(int i=0; i<dim; i+=step)
    {
        int x = dim==1? w/2 : (i+0.5f) * (w-2*pad) / (dim) + pad+(!i?1:0);
        if(i==dim-1) x -= 4;
        //        int x = dim==1? w/2 : i*(w-2*pad)/(dim-1);
        painter.drawText(x - 4, h-1, QString("l%1").arg(i+1));
    }
}
void KPCAProjection::DrawModel(Canvas *canvas, QPainter &painter, Projector *projector)
{
    contourPixmaps.clear();
    if(!canvas || !projector) return;
    ProjectorKPCA *kpca = dynamic_cast<ProjectorKPCA*>(projector);
    if(!kpca) return;
    contourPca = *kpca->pca;
    pcaPointer = &contourPca;
    vector<fvec> samples = projector->source;
    contourSamples = samples;
    contourSampleLabels = canvas->data->GetLabels();
    if(contourSampleLabels.size() != contourSamples.size()) contourSampleLabels = ivec(contourSamples.size(), 0);

    // use [xy]Index given in the constuctor [ov]
    //xIndex = contours->spinX1->value();
    //yIndex = contours->spinX2->value();

    // changed FLT_MAX -> DBL_MAX [ov]
    xmin=ymin=DBL_MAX;
    xmax=ymax=-DBL_MAX;
    int dim = samples.size() ? samples[0].size() : 2;
    contours->spinX1->setRange(1, dim);
    contours->spinX2->setRange(1, dim);
    //if(canvas->xIndex < dim) contours->spinX1->setValue(xIndex+1);
    //if(canvas->yIndex < dim) contours->spinX2->setValue(yIndex+1);
    qDebug() << "xIndex:"<< xIndex << ", yIndex:" << yIndex;
    FOR(i, samples.size())
    {
        contourSamples[i] -= kpca->mean;
        xmin=min((double)samples[i][xIndex]-kpca->mean[xIndex], xmin);
        xmax=max((double)samples[i][xIndex]-kpca->mean[xIndex], xmax);
        ymin=min((double)samples[i][yIndex]-kpca->mean[yIndex], ymin);
        ymax=max((double)samples[i][yIndex]-kpca->mean[yIndex], ymax);
        /*
        xmin=min((double)samples[i][xIndex], xmin);
        xmax=max((double)samples[i][xIndex], xmax);
        ymin=min((double)samples[i][yIndex], ymin);
        ymax=max((double)samples[i][yIndex], ymax);
        */
    }
    //qDebug() << "KPCAProjection::DrawModel - xmin:" << xmin << ", ymin:" << ymin << ", xman:" << xmax << ", ymax:" << ymax << ".";

    double xdiff = (xmax - xmin);
    double ydiff = (ymax - ymin);
    if(xdiff <= ydiff)
    {
        xdiff = ydiff;
        double xmid = ((xmax - xmin)/2.0) + xmin;
        xmin = xmid - xdiff/2.0;
        xmax = xmid + xdiff/2.0;
    }
    else if(ydiff < xdiff)
    {
        ydiff = xdiff;
        double ymid = ((ymax - ymin)/2.0) + ymin;
        ymin = ymid - (ydiff/2.0);
        ymax = ymid + (ydiff/2.0);
    }
    if(xdiff == 0) xdiff = .5f;
    if(ydiff == 0) ydiff = .5f;
    xmin -= xdiff;
    xmax += xdiff;
    ymin -= ydiff;
    ymax += ydiff;
    if(samples.size() < 3)
    {
        xmin -= 3*xdiff;
        xmax += 3*xdiff;
        ymin -= 3*ydiff;
        ymax += 3*ydiff;
        //cout << pcaPointer->eigenVectors;
    }

    // we get the eigenvectors
    fvec eigenvalues;
    VectorXd eigs = kpca->pca->eigenvalues;
    FOR(i, eigs.rows())
    {
        eigenvalues.push_back(eigs(i));
    }
    sort(eigenvalues.begin(), eigenvalues.end(), std::greater<float>());
    eigenvalues.resize(params->dimCountSpin->value());
    FOR(i, eigenvalues.size()) qDebug() << "eigs" << i << eigenvalues[i];


    float accumulator = 0;
    float maxEigVal = 0;
    FOR(i, eigenvalues.size()) if(eigenvalues[i] == eigenvalues[i] && eigenvalues[i] >= 0) maxEigVal += eigenvalues[i];

    params->eigenList->clear();
    FOR(i, eigenvalues.size())
    {
        float eigval = eigenvalues[i];
        if(eigval == eigval && eigval >= 0)
        {
            accumulator += eigval / maxEigVal;
        }
        else eigval = 0;
        params->eigenList->addItem(QString("%1: %2 %3%%").arg(i+1).arg(eigval, 0, 'f', 2).arg(eigval/maxEigVal*100, 0, 'f', 1));
    }

    QPixmap pixmap(params->eigenGraph->size());
    QBitmap bitmap(params->eigenGraph->size());
    pixmap.setMask(bitmap);
    pixmap.fill(Qt::transparent);
    QPainter eigenPainter(&pixmap);
    DrawEigenvals(eigenPainter, eigenvalues, true);
    params->eigenGraph->setPixmap(pixmap);

    contours->dimSpin->setRange(1, kpca->targetDims);
    DrawContours(contours->dimSpin->value());
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
    if(settings.contains("kernelDegSpin")) params->kernelDegSpin->setValue(settings.value("kernelDegSpin").toFloat());
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
