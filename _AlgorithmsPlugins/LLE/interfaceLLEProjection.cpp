#include "interfaceLLEProjection.h"
#include "projectorLLE.h"
#include <QDebug>
#include <QImage>
#include <QClipboard>
#include <qcontour.h>
#include <algorithm>

using namespace std;
using namespace Eigen;
void DrawLLEEigenvals(QPainter &painter, fvec eigs);

LLEProjection::LLEProjection()
    : widget(new QWidget()) //, pcaPointer(0), contourWidget(new QWidget()), xIndex(0), yIndex(1)
{
    params = new Ui::paramsLLE();
    params->setupUi(widget);

//    contours = new Ui::ContourWidget();
//    contours->setupUi(contourWidget);
//    contourWidget->layout()->setSizeConstraint( QLayout::SetFixedSize );
//    contourWidget->setWindowTitle("Kernel Eigenvector Projections");

//    connect(params->kernelTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeOptions()));
//    connect(params->contourButton, SIGNAL(clicked()), this, SLOT(ShowContours()));
//    connect(contours->dimSpin, SIGNAL(valueChanged(int)), this, SLOT(DrawContours(int)));
//    connect(contours->displayCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(ShowContours()));
//    connect(contours->clipboardButton, SIGNAL(clicked()), this, SLOT(SaveScreenshot()));
//    connect(contours->spinX1, SIGNAL(valueChanged(int)), this, SLOT(ContoursChanged()));
//    connect(contours->spinX2, SIGNAL(valueChanged(int)), this, SLOT(ContoursChanged()));
//    connect(contours->spinZoom, SIGNAL(valueChanged(int)), this, SLOT(ContoursChanged()));

//    contours->plotLabel->installEventFilter(this);
}

bool LLEProjection::eventFilter(QObject *obj, QEvent *event)
{
//    if(obj == contours->plotLabel)
//    {
//        if(event->type() == QEvent::MouseButtonRelease)
//        {
//            int displayType = contours->displayCombo->currentIndex();
//            if(displayType != 0) return QObject::eventFilter(obj, event);
//            int index = contours->dimSpin->value();
//            contours->plotLabel->setPixmap(contourPixmaps[index]);
//        }
//        else if(event->type() == QEvent::MouseMove || event->type() == QEvent::MouseButtonPress)
//        {
//            QMouseEvent *e = static_cast<QMouseEvent *>(event);
//            bool bLowerThan = e->buttons() == Qt::LeftButton;
//            int displayType = contours->displayCombo->currentIndex();
//            int index = contours->dimSpin->value();
//            QImage image = contourPixmaps[index].toImage();
//            if(displayType != 0 || e->x() < 0 || e->y() < 0 || e->x() >= image.width() || e->y() >= image.height()) return QObject::eventFilter(obj, event);
//            int limit = qRed(image.pixel(e->x(), e->y()));
//            // we get the neighboring pixels
//            if(e->y() < image.height()-2)
//            {
//                int l2 = qRed(image.pixel(e->x(), e->y()+1));
//                int l3 = qRed(image.pixel(e->x(), e->y()+2));
//                if(abs(l2 - limit) > 5 || abs(l3 - limit) > 5) return QObject::eventFilter(obj, event);
//            }
//            if(limit <= 0 || limit >= 255) return QObject::eventFilter(obj, event);
//            FOR(x, image.width())
//            {
//                FOR(y, image.height())
//                {
//                    QRgb p = image.pixel(x,y);
//                    int red = qRed(p);
//                    if(red >= 255) continue;
//                    int gray = qRed(p);
//                    if(bLowerThan && gray > limit) image.setPixel(x,y,qRgb(0,0,255));
//                    else if(!bLowerThan && gray < limit) image.setPixel(x,y,qRgb(0,255,0));
//                }
//            }
//            contours->plotLabel->setPixmap(QPixmap::fromImage(image));
//            contours->plotLabel->repaint();
//            // we need to get the color of the map
//            return true;
//        }
//    }
    return QObject::eventFilter(obj, event);
}


LLEProjection::~LLEProjection()
{
    delete params;
//    DEL(contours);
//    DEL(contourWidget);
}

// virtual functions to manage the algorithm creation
Projector *LLEProjection::GetProjector()
{
    return new ProjectorLLE(params->dimCountSpin->value());
}

void LLEProjection::SetParams(Projector *projector)
{
    if(!projector) return;
    ProjectorLLE *lle = dynamic_cast<ProjectorLLE*>(projector);
    if(!lle) return;

    lle->SetParams(params->knnSpin->value());
}

fvec LLEProjection::GetParams()
{
    int knn = params->knnSpin->value();

    fvec par(1);
    par[0] = knn;
    return par;
}

void LLEProjection::SetParams(Projector *projector, fvec parameters)
{
    if(!projector) return;
    int knn = parameters.size() > 0 ? parameters[0] : 0;

    ProjectorLLE *lle = dynamic_cast<ProjectorLLE*>(projector);
    if(!lle) return;

    lle->SetParams(knn);
}

void LLEProjection::GetParameterList(std::vector<QString> &parameterNames,
                                std::vector<QString> &parameterTypes,
                                std::vector< std::vector<QString> > &parameterValues)
{
    parameterNames.push_back("K-NN");
    parameterTypes.push_back("Integer");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("1");
    parameterValues.back().push_back("200");
}

//void LLEProjection::SaveScreenshot()
//{
//    const QPixmap *screenshot = contours->plotLabel->pixmap();
//    if(screenshot->isNull()) return;

//    QClipboard *clipboard = QApplication::clipboard();
//    clipboard->setImage(screenshot->toImage());
//    clipboard->setPixmap(*screenshot);
//}

//void LLEProjection::ContoursChanged()
//{
//    contourPixmaps.clear();
//    ShowContours();
//}

//void LLEProjection::ShowContours()
//{
//    PCA *pca = dynamic_cast<PCA*> (pcaPointer);
//    if(!pca) return;
//    DrawContours(contours->dimSpin->value());
//    contourWidget->show();
//}

//void LLEProjection::ChangeOptions()
//{
//    switch(params->kernelTypeCombo->currentIndex())
//    {
//    case 0: // poly
//        params->kernelDegSpin->setEnabled(true);
//        params->kernelDegSpin->setVisible(true);
//        params->kernelWidthSpin->setEnabled(true);
//        params->kernelWidthSpin->setVisible(true);
//        params->kernelDegSpin->setDecimals(0);
//        params->kernelDegSpin->setRange(1,999);
//        params->kernelDegSpin->setSingleStep(1);
//        params->kernelWidthSpin->setRange(-999,999);
//        params->param1Label->setText("Degree");
//        params->param2Label->setText("Offset");
//        break;
//    case 1: // RBF
//        params->kernelDegSpin->setEnabled(false);
//        params->kernelDegSpin->setVisible(false);
//        params->param1Label->setText("");
//        params->param2Label->setText("Width");
//        params->kernelWidthSpin->setRange(0.001,999);
//        params->kernelWidthSpin->setEnabled(true);
//        params->kernelWidthSpin->setVisible(true);
//        break;
//    case 2: // TANH
//        params->kernelDegSpin->setEnabled(true);
//        params->kernelDegSpin->setVisible(true);
//        params->kernelWidthSpin->setEnabled(true);
//        params->kernelWidthSpin->setVisible(true);
//        params->kernelDegSpin->setDecimals(3);
//        params->kernelDegSpin->setRange(0.01,100);
//        params->kernelDegSpin->setSingleStep(0.1);
//        params->kernelWidthSpin->setRange(-999,999);
//        params->param1Label->setText("Scale");
//        params->param2Label->setText("Offset");
//        break;
//    }
//}

void LLEProjection::DrawInfo(Canvas *canvas, QPainter &painter, Projector *projector)
{
    if(!canvas || !projector) return;
}

//void LLEProjection::GetContoursPixmap(int index)
//{
//    PCA *pca = dynamic_cast<PCA*>(pcaPointer);
//    if(!pca) return;
//    if(contourPixmaps.count(index)) return; // nothing to be done here, moving on!

//    // we compute the density map
//    int w = 65;
//    int h = 65;
//    int hmo = h-1; // we will drop one line at the edges to avoid weird border conditions
//    int wmo = w-1;
//    QImage image(wmo,hmo,QImage::Format_RGB32);
//    int dim = pca->sourcePoints.rows();
//    float zoom = 1;
//    // use [xy]Index given in the constuctor if spinX1 [ov]
//    if (contours->spinX1->isVisible()) {
//        xIndex = contours->spinX1->value()-1;
//        yIndex = contours->spinX2->value()-1;
//        zoom = (contours->spinZoom->value()-1)*0.25 + 1;
//    }
//    //qDebug() << "LLEProjection::GetContoursPixmap - xIndex:" << xIndex << ", yIndex:" << yIndex << ", zoom:" << zoom << ".";
//    double multiplier = 1000.; // this is used to avoid numerical instabilities when computing the contour lines
//    VectorXd point(dim);
//    FOR(d,dim) point(d) = 0.0;
//    double xdiff = xmax - xmin;
//    double ydiff = ymax - ymin;
//    //qDebug() << "LLEProjection::GetContoursPixmap - xmin:" << xmin << ", ymin:" << ymin << ", xman:" << xmax << ", ymax:" << ymax << ".";
//    double *values = new double[w*h];
//    double vmin = DBL_MAX, vmax = -DBL_MAX;

//    double zxmin = xmin - xdiff*0.5*(zoom-1);
//    double zxmax = xmax + xdiff*0.5*(zoom-1);
//    double zxdiff = zxmax - zxmin;
//    double zymin = ymin - ydiff*0.5*(zoom-1);
//    double zymax = ymax + ydiff*0.5*(zoom-1);
//    double zydiff = zymax - zymin;
//    FOR(i, w)
//    {
//        FOR(j, h)
//        {
//            if ( xIndex < dim ) point( xIndex ) = i * zxdiff / (double)w + zxmin;
//            if ( yIndex < dim ) point( yIndex ) = j * zydiff / (double)h + zymin;
//            double value = pcaPointer->test( point, index-1, multiplier ); // indices start from 1 in params.dimCountSpin
//            vmin = min(value, vmin);
//            vmax = max(value, vmax);
//            values[j*w + i] = value;
//        }
//    }
//    double vdiff=vmax-vmin;
//    //qDebug() << "LLEProjection::GetContoursPixmap - vmin:" << vmin << ", vmax:" << vmax << " - vdiff: " << vdiff << ".";
//    if(vdiff == 0) vdiff = 1.f;
//    FOR(i, wmo)
//    {
//        FOR(j, hmo)
//        {
//            int value = (int)((values[j*w + i]-vmin)/vdiff*255);
//            image.setPixel(i,j, qRgb((int)value,value,value));
//        }
//    }
//    QPixmap contourPixmap = QPixmap::fromImage(image).scaled(512,512, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

//    int W = contourPixmap.width();
//    int H = contourPixmap.height();
//    QPainter painter(&contourPixmap);
//    painter.setRenderHint(QPainter::Antialiasing);

//    // we plot the samples
//    painter.setPen(Qt::black);
//    painter.setBrush(Qt::white);
//    painter.setOpacity(1);
//    FOR(i, contourSamples.size())
//    {
//        fvec &sample = contourSamples[i];
//        float x = (sample[xIndex]-zxmin)/(zxmax-zxmin);
//        float y = (sample[yIndex]-zymin)/(zymax-zymin);
//        x = (x + 1.f/w)*W;
//        y = (y + 1.f/w)*H;
//        Canvas::drawSample(painter, QPointF(x,y), 10, contourSampleLabels[i]);
//    }

//    // we plot the contour lines
//    if(contourSamples.size())
//    {
//        QContour contour(values, w, h);
//        contour.bDrawColorbar = true;
//        //contour.SetLimits(zvmin, zvmax);
//        contour.Paint(painter, 20, zoom);
//    }

//    contourPixmaps[index] = contourPixmap;
//    delete [] values;
//}

//void LLEProjection::DrawContours(int index)
//{
//    PCA *pca = dynamic_cast<PCA*>(pcaPointer);
//    if(!pca) return;
//    int displayType = contours->displayCombo->currentIndex();
//    if ( ( displayType < 0 ) or ( displayType > 1 ) ) {
//        qDebug() << "LLEProjection::DrawContours - displayType out of range :" << displayType;
//        displayType = 0;
//    }

//    switch(displayType)
//    {
//    case 0: // single
//    {
//        // ensure that we have the right pixmap
//        GetContoursPixmap(index);
//        contours->plotLabel->setPixmap(contourPixmaps[index]);
//    }
//        break;
//    case 1: // take all the values and draw them
//    {
//        int maximum = contours->dimSpin->maximum();
//        for(int i=1; i<=contours->dimSpin->maximum(); i++)
//        {
//            GetContoursPixmap(i);
//        }
//        int gridX = std::ceil(sqrtf(maximum));
//        //int gridY = std::ceil(maximum / (float)gridX);
//        int gridY = gridX;

//        int w = contourPixmaps[1].width();
//        int h = contourPixmaps[1].height();
//        QPixmap bigPixmap(gridX*w, gridX*h);
//        //QBitmap bitmap(bigPixmap.width(), bigPixmap.height());
//        //bitmap.clear();
//        //bigPixmap.setMask(bitmap);
//        bigPixmap.fill(Qt::transparent);
//        QPainter painter(&bigPixmap);
//        for(int i=1; i<=contours->dimSpin->maximum(); i++)
//        {
//            int x = ((i-1)%gridX)*w;
//            int y = ((i-1)/gridX)*h;
//            QRect rect(x,y,w,h);
//            painter.drawPixmap(rect, contourPixmaps[i], QRect(0,0,w,h));
//        }
//        contours->plotLabel->setPixmap(bigPixmap.scaled(QSize(w,h), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
//    }
//        break;
//    }
//    if(contourWidget->isVisible()) contours->plotLabel->repaint();
//}


void LLEProjection::DrawModel(Canvas *canvas, QPainter &painter, Projector *projector)
{
    if(!canvas || !projector) return;
    //if(canvas->canvasType) return;
    if(!canvas->data->bProjected) // We are displaying a Manifold to 1D
    {
        painter.setRenderHint(QPainter::Antialiasing);

        // we need to sort the list of points
        vector< pair<float, int> > points(projector->projected.size());
        FOR(i, projector->projected.size())
        {
            points[i] = make_pair(projector->projected[i][0], i);
        }
        sort(points.begin(), points.end());
        float minVal = points.front().first;
        float maxVal = points.back().first;

        // now we go through the points and compute the back projection
        int steps = min((int)points.size(), 64);
        int index = 0;
        vector<QPointF> pointList;
        FOR(i, steps)
        {
            float val = (i+1)/(float)steps*(maxVal-minVal) + minVal;
            int nextIndex = (i+1)/(float)steps*points.size();
            fvec mean(canvas->data->GetDimCount());
            float meanVal = 0;
            int count = 0;
            while(index < points.size() && index < nextIndex)
            //while(index < points.size() && points[index].first < val)
            {
                meanVal += points[index].first;
                mean += canvas->data->GetSample(points[index].second);
                count++;
                index++;
            }
            mean /= count;
            meanVal /= count;
            // we look for the closest point to the value in projected space
            int closest = 0;
            float closestDist = FLT_MAX;
            FOR(p, points.size())
            {
                float dist = (meanVal-points[p].first)*(meanVal-points[p].first);
                if(dist < closestDist)
                {
                    closestDist = dist;
                    closest = p;
                }
            }
            QPointF point = canvas->toCanvasCoords(mean);
            //QPointF point = canvas->toCanvasCoords(canvas->data->GetSample(points[closest].second));
            if(!count) pointList.push_back(pointList.back());
            else pointList.push_back(point);
        }
        // and we draw it
        FOR(i, pointList.size()-1)
        {
            painter.setPen(QPen(Qt::black, 2));
            painter.drawLine(pointList[i], pointList[i+1]);
        }
        FOR(i, pointList.size())
        {
            painter.setPen(QPen(Qt::black, 3));
            painter.setBrush(QBrush(QColor(255*i/pointList.size(), 255*i/pointList.size(), 255*i/pointList.size())));
            painter.drawEllipse(pointList[i], 7, 7);
        }
    }

    if(!projector) return;
    ProjectorLLE *lle = dynamic_cast<ProjectorLLE*>(projector);
    fvec eigenval;
    VectorXd eigs = lle->eigenvalues;
    FOR(i, eigs.rows())
    {
        eigenval.push_back(eigs(i));
    }
    sort(eigenval.begin(), eigenval.end());
    eigenval.resize(params->dimCountSpin->value());
    //FOR(i, eigenval.size()) qDebug() << "eigs" << i << eigenval[i];

    QPixmap pixmap(params->eigenGraph->size());
    pixmap.fill(Qt::transparent);
    QPainter eigenPainter(&pixmap);
    DrawLLEEigenvals(eigenPainter, eigenval);
    params->eigenGraph->setPixmap(pixmap);

    params->eigenList->clear();
    FOR(i, eigenval.size()) params->eigenList->addItem(QString("%1: %2").arg(i+1).arg(eigenval[i], 0, 'd', 10));

//    contourPixmaps.clear();
//    if(!canvas || !projector) return;
//    ProjectorLLE *lle = dynamic_cast<ProjectorLLE*>(projector);
//    if(!lle) return;
//    contourPca = *lle->pca;
//    pcaPointer = &contourPca;
//    vector<fvec> samples = projector->source;
//    contourSamples = samples;
//    contourSampleLabels = canvas->data->GetLabels();
//    if(contourSampleLabels.size() != contourSamples.size()) contourSampleLabels = ivec(contourSamples.size(), 0);

//    // changed FLT_MAX -> DBL_MAX [ov]
//    xmin=ymin=DBL_MAX;
//    xmax=ymax=-DBL_MAX;
//    int dim = samples.size() ? samples[0].size() : 2;
//    contours->spinX1->setRange(1, dim);
//    contours->spinX2->setRange(1, dim);
//    FOR(i, samples.size())
//    {
//        contourSamples[i] -= lle->mean;
//        xmin=min((double)samples[i][xIndex]-lle->mean[xIndex], xmin);
//        xmax=max((double)samples[i][xIndex]-lle->mean[xIndex], xmax);
//        ymin=min((double)samples[i][yIndex]-lle->mean[yIndex], ymin);
//        ymax=max((double)samples[i][yIndex]-lle->mean[yIndex], ymax);
//    }

//    double xdiff = (xmax - xmin);
//    double ydiff = (ymax - ymin);

//    double diff = max(xdiff, ydiff);
//    double xmid = (xmax-xmin)/2 + xmin;
//    double ymid = (ymax-ymin)/2 + ymin;
//    xmin = xmid - diff;
//    xmax = xmid + diff;
//    ymin = ymid - diff;
//    ymax = ymid + diff;
//    /*
//    if(xdiff <= ydiff)
//    {
//        xdiff = ydiff;
//        double xmid = ((xmax - xmin)/2.0) + xmin;
//        xmin = xmid - xdiff/2.0;
//        xmax = xmid + xdiff/2.0;
//    }
//    else if(ydiff < xdiff)
//    {
//        ydiff = xdiff;
//        double ymid = ((ymax - ymin)/2.0) + ymin;
//        ymin = ymid - (ydiff/2.0);
//        ymax = ymid + (ydiff/2.0);
//    }
//    if(xdiff == 0) xdiff = .5f;
//    if(ydiff == 0) ydiff = .5f;
//    */

//    /*
//    xmin -= xdiff;
//    xmax += xdiff;
//    ymin -= ydiff;
//    ymax += ydiff;
//    */
//    if(samples.size() < 3)
//    {
//        xmin -= diff;
//        xmax += diff;
//        ymin -= diff;
//        ymax += diff;
//    }

//    // we get the eigenvectors
//    fvec eigenvalues;
//    VectorXd eigs = lle->pca->eigenvalues;
//    FOR(i, eigs.rows())
//    {
//        eigenvalues.push_back(eigs(i));
//    }
//    sort(eigenvalues.begin(), eigenvalues.end(), std::greater<float>());
//    eigenvalues.resize(params->dimCountSpin->value());
//    //FOR(i, eigenvalues.size()) qDebug() << "eigs" << i << eigenvalues[i];

//    float accumulator = 0;
//    float maxEigVal = 0;
//    FOR(i, eigenvalues.size()) if(eigenvalues[i] == eigenvalues[i] && eigenvalues[i] >= 0) maxEigVal += eigenvalues[i];

//    params->eigenList->clear();
//    FOR(i, eigenvalues.size())
//    {
//        float eigval = eigenvalues[i];
//        if(eigval == eigval && eigval >= 0)
//        {
//            accumulator += eigval / maxEigVal;
//        }
//        else eigval = 0;
//        params->eigenList->addItem(QString("%1: %2 %3%%").arg(i+1).arg(eigval, 0, 'f', 2).arg(eigval/maxEigVal*100, 0, 'f', 1));
//    }


//    QPixmap pixmap(params->eigenGraph->size());
//    pixmap.fill(Qt::transparent);
//    QPainter eigenPainter(&pixmap);
//    DrawEigenvals(eigenPainter, eigenvalues, true);
//    params->eigenGraph->setPixmap(pixmap);

//    contours->dimSpin->setRange(1, lle->targetDims);
//    DrawContours(contours->dimSpin->value());

//    if(canvas->canvasType) return;
//    if(!canvas->data->bProjected) // We are displaying a Manifold to 1D
//    {
//        painter.setRenderHint(QPainter::Antialiasing);

//        // we need to sort the list of points
//        vector< pair<float, int> > points(projector->projected.size());
//        FOR(i, projector->projected.size())
//        {
//            points[i] = make_pair(projector->projected[i][0], i);
//        }
//        sort(points.begin(), points.end());
//        float minVal = points.front().first;
//        float maxVal = points.back().first;

//        // now we go through the points and compute the back projection
//        int steps = min((int)points.size(), 64);
//        int index = 0;
//        vector<QPointF> pointList;
//        FOR(i, steps)
//        {
//            float val = (i+1)/(float)steps*(maxVal-minVal) + minVal;
//            int nextIndex = (i+1)/(float)steps*points.size();
//            fvec mean(canvas->data->GetDimCount());
//            float meanVal = 0;
//            int count = 0;
//            while(index < points.size() && index < nextIndex)
//            //while(index < points.size() && points[index].first < val)
//            {
//                meanVal += points[index].first;
//                mean += canvas->data->GetSample(points[index].second);
//                count++;
//                index++;
//            }
//            mean /= count;
//            meanVal /= count;
//            // we look for the closest point to the value in projected space
//            int closest = 0;
//            float closestDist = FLT_MAX;
//            FOR(p, points.size())
//            {
//                float dist = (meanVal-points[p].first)*(meanVal-points[p].first);
//                if(dist < closestDist)
//                {
//                    closestDist = dist;
//                    closest = p;
//                }
//            }
//            QPointF point = canvas->toCanvasCoords(mean);
//            //QPointF point = canvas->toCanvasCoords(canvas->data->GetSample(points[closest].second));
//            if(!count) pointList.push_back(pointList.back());
//            else pointList.push_back(point);
//        }
//        // and we draw it
//        FOR(i, pointList.size()-1)
//        {
//            painter.setPen(QPen(Qt::black, 2));
//            painter.drawLine(pointList[i], pointList[i+1]);
//        }
//        FOR(i, pointList.size())
//        {
//            painter.setPen(QPen(Qt::black, 3));
//            painter.setBrush(QBrush(QColor(255*i/pointList.size(), 255*i/pointList.size(), 255*i/pointList.size())));
//            painter.drawEllipse(pointList[i], 7, 7);
//        }
//    }

}

void DrawLLEEigenvals(QPainter &painter, fvec eigs)
{
    int w=painter.window().width();
    int h=painter.window().height();
    int pad = 5;

    int dim = eigs.size();
    float maxEigVal = 1.f;
    if (dim) maxEigVal = eigs[dim-1];

    painter.setPen(Qt::gray);
    painter.drawLine(QPointF(pad, h-2*pad), QPointF(w-pad, h-2*pad));
    painter.drawLine(QPointF(pad, pad), QPointF(pad, h-2*pad));
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);
    int rectW = (w-2*pad) / dim - 2;
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
    painter.drawText(0,0,w,2.5*pad,Qt::AlignCenter, "eigenvalues");
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

// virtual functions to manage the GUI and I/O
QString LLEProjection::GetAlgoString()
{
    return QString("LLE");
}

void LLEProjection::SaveOptions(QSettings &settings)
{
    settings.setValue("knnSpin", params->knnSpin->value());
    settings.setValue("dimCountSpin", params->dimCountSpin->value());
}

bool LLEProjection::LoadOptions(QSettings &settings)
{
    if(settings.contains("knnSpin")) params->knnSpin->setValue(settings.value("knnSpin").toInt());
    if(settings.contains("dimCountSpin")) params->dimCountSpin->setValue(settings.value("dimCountSpin").toInt());
//    ChangeOptions();
    return true;
}

void LLEProjection::SaveParams(QTextStream &file)
{
    file << "projectOptions" << ":" << "knnSpin" << " " << params->knnSpin->value() << "\n";
    file << "projectOptions" << ":" << "dimCountSpin" << " " << params->dimCountSpin->value() << "\n";
}

bool LLEProjection::LoadParams(QString name, float value)
{
    if(name.endsWith("knnSpin")) params->knnSpin->setValue((int)value);
    if(name.endsWith("dimCountSpin")) params->dimCountSpin->setValue((int)value);
    //ChangeOptions();
    return true;
}
