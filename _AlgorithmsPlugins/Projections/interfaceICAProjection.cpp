#include "interfaceICAProjection.h"
#include "projectorICA.h"

using namespace std;

ICAProjection::ICAProjection()
    : widget(new QWidget())
{
    params = new Ui::paramsICA();
    params->setupUi(widget);
}

ICAProjection::~ICAProjection()
{
    delete params;
}

// virtual functions to manage the algorithm creation
Projector *ICAProjection::GetProjector()
{
    return new ProjectorICA();
}

fvec ICAProjection::GetParams()
{
    return fvec();
}

void ICAProjection::SetParams(Projector *projector, fvec parameters){}

void ICAProjection::GetParameterList(std::vector<QString> &parameterNames,
                             std::vector<QString> &parameterTypes,
                             std::vector< std::vector<QString> > &parameterValues){}

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
    //file << "projectOptions" << ":" << "kernelCluster" << " " << params->kernelClusterSpin->value() << "\n";
}

bool ICAProjection::LoadParams(QString name, float value)
{
    //if(name.endsWith("kernelCluster")) params->kernelClusterSpin->setValue((int)value);
    return true;
}
