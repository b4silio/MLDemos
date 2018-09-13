/*********************************************************************
MLDemos: A User-Friendly visualization toolkit for machine learning
Copyright (C) 2010  Basilio Noris
Contact: mldemos@b4silio.com

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public License,
version 3 as published by the Free Software Foundation.

This library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free
Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*********************************************************************/
#include "interfaceDBSCAN.h"
#include "drawUtils.h"
#include <QPixmap>
#include <QBitmap>
#include <QDebug>
#include <QPainter>
#include "opencvincludes.h"

using namespace std;

ClustDBSCAN::ClustDBSCAN()
    : widget(new QWidget()), zoomWidget(new QWidget())
{
    // we initialize the hyperparameter widget
        params = new Ui::ParametersDBSCAN();
        params->setupUi(widget);

    // and the graph windows
        graphzoom = new Ui::graphZoom();
        graphzoom->setupUi(zoomWidget);
        zoomWidget->setWindowTitle("OPTICS reachability-distance plot");
        params->zoomButton->setVisible(false);
        params->typeCombo->setVisible(false);
        params->typeLabel->setVisible(false);
        params->dendoLabel->setVisible(false);
        params->depthSpin->setVisible(false);
        //params->metricCombo->setVisible(false);
        //params->metricLabel->setVisible(false);

      // connecting slots for opening the windows and changing the UI
        connect(params->zoomButton, SIGNAL(clicked()), this, SLOT(showZoom()));
        connect(params->typeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(typeChanged(int)));
}

ClustDBSCAN::~ClustDBSCAN()
{
    delete params;
    delete graphzoom;
    delete zoomWidget;
}

// Paint the graph on the new windows and show it. User may also click on the button to refresh the plot.
void ClustDBSCAN::showZoom()
{
        QPixmap pixmap(graphzoom->graph->size());
        //QBitmap bitmap(graphzoom->graph->size());
        //bitmap.clear();
        //pixmap.setMask(bitmap);
        pixmap.fill(Qt::transparent);
        QPainter dPainter(&pixmap);
        DrawDendogram(dPainter,true);
        graphzoom->graph->setPixmap(pixmap);
        zoomWidget->show();
}

// DBSCAN and OPTICS do not have the same parameters and DBSCAN doesn't need the plotting zone.
void ClustDBSCAN::typeChanged(int ntype)
{
    if (ntype==0) // DBSCAN
    {
       params->depthSpin->setVisible(false);
       params->dendoGraph->setVisible(false);
       params->dendoLabel->setVisible(false);
       params->zoomButton->setVisible(false);
    }
    else
    {
        params->depthSpin->setVisible(true);
        params->dendoGraph->setVisible(true);
        params->dendoLabel->setVisible(true);
    }
}

void ClustDBSCAN::SetParams(Clusterer *clusterer)
{
    SetParams(clusterer, GetParams());
}

fvec ClustDBSCAN::GetParams()
{
    double minNeighbours = params->minptsSpin->value()-1;
    double eps = params->epsSpin->value();
    int metric = params->metricCombo->currentIndex();
    int type = params->typeCombo->currentIndex();
    double depth = params->depthSpin->value();

    int i=0;
    fvec par(5);
    par[i++] = minNeighbours;
    par[i++] = eps;
    par[i++] = metric;
    par[i++] = type;
    par[i++] = depth;
    return par;
}

void ClustDBSCAN::SetParams(Clusterer *clusterer, fvec parameters)
{
    if(!clusterer) return;
    ClustererDBSCAN * dbscan = dynamic_cast<ClustererDBSCAN *>(clusterer);
    if(!dbscan) return;

    int i=0;
    float minpts = (int)parameters.size() > i ? parameters[i] : 0; i++;
    float eps = (int)parameters.size() > i ? parameters[i] : 0; i++;
    int metric = (int)parameters.size() > i ? parameters[i] : 0; i++;
    int type = (int)parameters.size() > i ? parameters[i] : 0; i++;
    float depth = (int)parameters.size() > i ? parameters[i] : 0; i++;

    dbscan->SetParams(minpts, eps, metric,depth,type);
}

void ClustDBSCAN::GetParameterList(std::vector<QString> &parameterNames,
                             std::vector<QString> &parameterTypes,
                             std::vector< std::vector<QString> > &parameterValues)
{
    parameterNames.clear();
    parameterTypes.clear();
    parameterValues.clear();
    parameterNames.push_back("Min Points");
    parameterNames.push_back("Epsilon");
    parameterNames.push_back("Metric Type");
    parameterNames.push_back("Algorithm");
    parameterNames.push_back("Depth.");
    parameterTypes.push_back("Integer");
    parameterTypes.push_back("Real");
    parameterTypes.push_back("List");
    parameterTypes.push_back("List");
    parameterTypes.push_back("Real");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("1");
    parameterValues.back().push_back("99999");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("0.00000000001f");
    parameterValues.back().push_back("99999999.f");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("Cosine");
    parameterValues.back().push_back("Euclidean");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("DBSCAN");
    parameterValues.back().push_back("OPTICS");
    parameterValues.back().push_back("OPTICS WP");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("0.00000000001f");
    parameterValues.back().push_back("99999999.f");
}

Clusterer *ClustDBSCAN::GetClusterer()
{
    // we instanciate the algorithm object
    ClustererDBSCAN *clusterer = new ClustererDBSCAN();
    // we set its parameters
    SetParams(clusterer);
    // we return it to the main program
	return clusterer;
}

void ClustDBSCAN::DrawInfo(Canvas *canvas, QPainter &painter, Clusterer *clusterer)
{
    if(!canvas || !clusterer) return;
    painter.setRenderHint(QPainter::Antialiasing);

    ClustererDBSCAN * dbscan = dynamic_cast<ClustererDBSCAN*>(clusterer);
    if(!dbscan) return;

    int w = painter.viewport().width();
    int h = painter.viewport().height();

    float xRadius = -1.f;
    float yRadius = -1.f;
    int xIndex = canvas->xIndex;
    int yIndex = canvas->yIndex;
    float eps = params->epsSpin->value();
    int metric = params->metricCombo->currentIndex();

    if(dbscan->_type==0) // DBSCAN: we draw the core point with a black border
    {
        if(dbscan->pts.empty()) return;

        Point sample = dbscan->pts[0];
        fvec pt;
        pt.resize(sample.size(),0);
        FOR(j, sample.size()) pt[j]=sample[j];
        QPointF point = canvas->toCanvasCoords(pt);
        fvec ptDiff = pt;
        ptDiff[0] += eps;
        QPointF d = (canvas->toCanvasCoords(ptDiff) - point);
        xRadius = sqrt(d.x()*d.x() + d.y()*d.y());

        ptDiff = pt;
        ptDiff[1] += eps;
        d = (canvas->toCanvasCoords(ptDiff) - point);
        yRadius = sqrt(d.x()*d.x() + d.y()*d.y());

        QPolygonF astroid;
        int steps = 64;
        FOR(i, steps+1) {
            float theta = i/float(steps)*M_PI*2;
            float x = xRadius*.25*(3*cos(theta) + cos(3*theta));
            float y = yRadius*.25*(3*sin(theta) - sin(3*theta));
            astroid << QPointF(x,y);
        }

        std::map<int,QPixmap> pixmapsMap;

        // for every point in the current dataset
        FOR(i, dbscan->_pointId_to_clusterId.size())
        {
            if (!dbscan->_core[i] && !dbscan->_noise[i]) continue;
            int cid = dbscan->_pointId_to_clusterId[i];
            if(!pixmapsMap.count(cid)) {
                pixmapsMap[cid] = QPixmap(w,h);
                pixmapsMap[cid].fill(Qt::transparent);
            }

            // we get the sample
            Point sample = dbscan->pts[i];
            // transform it again in fvec from Point...
            fvec pt;
            pt.resize(sample.size(),0);
            FOR(j, sample.size()) {
                pt[j]=sample[j];
            }

            //... and we get the point in canvas coordinates (2D pixel by pixel) corresponding to the sample (N-dimensional in R)
            QPointF point = canvas->toCanvasCoords(pt);

            if (dbscan->_core[i]) {
                // add it to the set
                QPainter pixPainter(&pixmapsMap[cid]);
                pixPainter.setPen(Qt::NoPen);
                pixPainter.setBrush(Qt::white);
                QRectF r(point.x()-xRadius, point.y()-yRadius,xRadius*2,yRadius*2);
                if(metric == 0) {
                    pixPainter.drawEllipse(r);
                } else if (metric == 1) {
                    QPolygonF poly;
                    poly << point - QPointF(-xRadius,0);
                    poly << point - QPointF(0,-yRadius);
                    poly << point - QPointF(+xRadius,0);
                    poly << point - QPointF(0,+yRadius);
                    pixPainter.drawPolygon(poly);
                } else if (metric == 2) {
                    pixPainter.drawRect(r);
                } else if (metric == 3) {
                    pixPainter.drawPolygon(astroid.translated(point));
                }
             } else {
                // dont add it
            }
        }
        painter.setPen(QPen(Qt::black, 1, Qt::DotLine));

        FORIT(pixmapsMap, int, QPixmap) {
            cv::Mat image = QImageToCvMat(it->second.toImage());
            cv::Mat gray, edges;
            cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
            cv::Canny(gray, edges, 50, 100, 3);
            std::vector<std::vector<cv::Point> > contours;
            cv::findContours(gray, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
            FOR(i, contours.size()) {
                std::vector<cv::Point>& contour = contours.at(i);
                QPolygonF poly;
                FOR(j, contour.size()) {
                    poly << QPointF(contour[j].x, contour[j].y);
                }
                painter.drawPolygon(poly, Qt::WindingFill);
            }
        }
    } else {// OPTICS: we draw the position in the ordered list
        FOR(i, dbscan->_optics_list.size()) {
            // we get the sample
            Point sample = dbscan->pts[dbscan->_optics_list[i]];

            //transform the sample in fvec and we get the point in canvas coordinates (2D pixel by pixel) corresponding to the sample (N-dimensional in R)

            fvec pt;
            pt.resize(sample.size(),0);
            FOR(j, sample.size())
            {
                pt[j]=sample[j];
            }

            QPointF point = canvas->toCanvasCoords(pt);

            // and we add a text indicating the position in the list
            QFont font = painter.font();
            font.setPointSize(8);
            painter.setFont(font);
            painter.setPen(Qt::gray); //not easy to choose the color without knowing what will be painted below
            painter.drawText(point,QString("   %1").arg(i));
        }
    }
}

void ClustDBSCAN::DrawDendogram(QPainter &painter, bool legend) //draw the reachability plot
{
    //what is the size of our painting area?
    int w=painter.window().width();
    int h=painter.window().height();
    int pad = 0; // no padding between vertical bars

    vector<double> val; // build the list we will actually plot

    FOR(j,optics_list.size())
    {
        if (reachability[optics_list[j]] == -1)
        {
            val.push_back(params->epsSpin->value()); // if undefined, plot it at the maximum size
        }
        else
        {
            val.push_back(reachability[optics_list[j]]);
        }
    }

    int dim = val.size();
    float maxVal = params->epsSpin->value() * 1.1; // make the graph 10% bigger that the highest value
    painter.setBrush(Qt::white);
    painter.setPen(Qt::NoPen);
    painter.drawRect(0,0,w,h); // plotting area
    painter.setPen(Qt::gray);
    painter.drawLine(QPointF(pad, h-2*pad), QPointF(w-pad, h-2*pad));// axis
    painter.drawLine(QPointF(pad, pad), QPointF(pad, h-2*pad));// axis
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);
    int rectW = (w-2*pad) / (dim-1); // width of the bar
    FOR(i, dim)
    {
        // compute the position and size of the bar
        int x = dim==1 ? w/2 : i * (w-2*pad) / dim + pad;
        int y = (int)(val[i]/maxVal * (h-2*pad));
        y = min(y, h-2*pad);
        rectW = dim==1 ? rectW : (i+1) * (w-2*pad) / dim + pad -x;

       // color it accordingly to its cluster or black for noise
        int cid = pointId_to_clusterId[optics_list[i]];
        if(cid == 0)
        {
            painter.setBrush(Qt::black);
        }
        else
        {
            float r = SampleColor[cid].red();
            float g = SampleColor[cid].green();
            float b = SampleColor[cid].blue();
            painter.setBrush(QColor(r,g,b));
        }
        //let's draw it!
        painter.drawRect(x,h-2*pad,rectW,-y);
    }
    // draw the depth threshold (even if it makes less sense for OPTICS WP)
    painter.setPen(Qt::red);
    painter.setBrush(Qt::NoBrush);
    int y = (int)(params->depthSpin->value()/maxVal * (h-2*pad));
    painter.drawLine(QPointF(0, h-2*pad-y), QPointF(w-2*pad, h-2*pad-y));
    // draw the title
    QFont font = painter.font();
    font.setPointSize(8);
    painter.setFont(font);
    painter.setPen(Qt::gray);
    painter.drawText(0,0,w,2*pad,Qt::AlignCenter, "Reachability-distance plot");
    if(legend) {
        for(int i=0; i<9; i+=1){
            //use a small trick to put the ticks on the scale
            painter.drawText(0, h-(i*h) / 8, QString("_ %1").arg(i*maxVal/8.0));
        }
    }
}

void ClustDBSCAN::DrawModel(Canvas *canvas, QPainter &painter, Clusterer *clusterer)
{
    ClustererDBSCAN * dbscan = dynamic_cast<ClustererDBSCAN*>(clusterer);
    if(!dbscan) return;

    // we start by making the painter paint things nicely
    painter.setRenderHint(QPainter::Antialiasing);

    // for every point in the current dataset
    FOR(i, dbscan->_pointId_to_clusterId.size()) {
        // we get the sample
        Point sample = dbscan->pts[i];

        // and we get the cluster id
        int cid = dbscan->_pointId_to_clusterId[i];
        // we get the point in canvas coordinates (2D pixel by pixel) corresponding to the sample (N-dimensional in R)

        fvec pt;
        pt.resize(sample.size(),0);
        FOR(j, sample.size()) {
            pt[j]=sample[j];
        }
        QPointF point = canvas->toCanvasCoords(pt);

        if(cid > 0) {
            // we combine together the contribution of each cluster to color the sample properly
            painter.setBrush(SampleColor[cid%SampleColorCnt]);
        } else {
            // we combine together the contribution of each cluster to color the sample properly
            painter.setBrush(Qt::gray);
        }
        // set the color of the brush and a black edge
        painter.setPen(Qt::black);
        // and draw the sample itself
        painter.drawEllipse(point,5,5);
    }

    // we copy the data for zoom display
    optics_list = dbscan->_optics_list;
    reachability = dbscan->_reachability;
    pointId_to_clusterId = dbscan->_pointId_to_clusterId;

    if(params->typeCombo->currentIndex()>0) {
        // and plot the small graph
        QPixmap pixmap(params->dendoGraph->size());
        pixmap.fill(Qt::transparent);
        QPainter dPainter(&pixmap);
        DrawDendogram(dPainter,false);
        params->dendoGraph->setPixmap(pixmap);
        params->zoomButton->setVisible(true); // now we are allowed to zoom
    }
}

void ClustDBSCAN::SaveOptions(QSettings &settings)
{
    // we save to the system registry each parameter value
    settings.setValue("MinPts", params->minptsSpin->value());
    settings.setValue("Eps", params->epsSpin->value());
    settings.setValue("Metric", params->metricCombo->currentIndex());
    settings.setValue("Type", params->typeCombo->currentIndex());
    settings.setValue("Depth", params->depthSpin->value());
 }

bool ClustDBSCAN::LoadOptions(QSettings &settings)
{
    // we load the parameters from the registry so that when we launch the program we keep all values
    if(settings.contains("MinPts")) params->minptsSpin->setValue(settings.value("MinPts").toFloat());
    if(settings.contains("Eps")) params->epsSpin->setValue(settings.value("Eps").toFloat());
    if(settings.contains("Metric")) params->metricCombo->setCurrentIndex(settings.value("Metric").toInt());
    if(settings.contains("Type")) params->typeCombo->setCurrentIndex(settings.value("Type").toInt());
    if(settings.contains("Depth")) params->depthSpin->setValue(settings.value("Depth").toFloat());
    if(params->typeCombo->currentIndex()==0) // prepare also the interface by hidding unnecessary stuff
    {
        params->depthSpin->setVisible(false);
        params->dendoGraph->setVisible(false);
        params->dendoLabel->setVisible(false);
        params->zoomButton->setVisible(false);
    }
    return true;
}

void ClustDBSCAN::SaveParams(QTextStream &file)
{
    // same as above but for files/string saving
    file << "clusterOptions" << ":" << "MinPts" << " " << params->minptsSpin->value() << "\n";
    file << "clusterOptions" << ":" << "Eps" << " " << params->epsSpin->value() << "\n";
    file << "clusterOptions" << ":" << "Metric" << " " << params->metricCombo->currentIndex() << "\n";
    file << "clusterOptions" << ":" << "Depth" << " " << params->depthSpin->value() << "\n";
    file << "clusterOptions" << ":" << "Type" << " " << params->typeCombo->currentIndex() << "\n";
}

bool ClustDBSCAN::LoadParams(QString name, float value)
{
    // same as above but for files/string saving
    if(name.endsWith("MinPts")) params->minptsSpin->setValue(value);
    if(name.endsWith("Eps")) params->epsSpin->setValue(value);
    if(name.endsWith("Metric")) params->metricCombo->setCurrentIndex((int)value);
    if(name.endsWith("Depth")) params->depthSpin->setValue(value);
    if(name.endsWith("Type")) params->typeCombo->setCurrentIndex((int)value);
    if(params->typeCombo->currentIndex()==0) // prepare also the interface by hidding unnecessary stuff
    {
        params->depthSpin->setVisible(false);
        params->dendoGraph->setVisible(false);
        params->dendoLabel->setVisible(false);
        params->zoomButton->setVisible(false);
    }
    return true;
}
