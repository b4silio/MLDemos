#include "interfaceSammonProjection.h"
#include "projectorSammon.h"
#include <QDebug>

using namespace std;

SammonProjection::SammonProjection()
    : widget(new QWidget())
{
    params = new Ui::paramsSammon();
    params->setupUi(widget);
}

SammonProjection::~SammonProjection()
{
    delete params;
}


// virtual functions to manage the algorithm creation
Projector *SammonProjection::GetProjector()
{
    return new ProjectorSammon();
}

fvec SammonProjection::GetParams()
{
    return fvec();
}

void SammonProjection::SetParams(Projector *projector, fvec parameters){}

void SammonProjection::GetParameterList(std::vector<QString> &parameterNames,
                             std::vector<QString> &parameterTypes,
                             std::vector< std::vector<QString> > &parameterValues){}

void SammonProjection::DrawInfo(Canvas *canvas, QPainter &painter, Projector *projector)
{
    if(!canvas || !projector) return;
    if(canvas->canvasType) return;
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

void SammonProjection::DrawModel(Canvas *canvas, QPainter &painter, Projector *projector)
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
QString SammonProjection::GetAlgoString()
{
    return QString("Sammon");
}

void SammonProjection::SetParams(Projector *projector)
{
    if(!projector) return;
    ProjectorSammon *sammon = dynamic_cast<ProjectorSammon*>(projector);
    if(!sammon) return;
    sammon->num_dims = params->dimCountSpin->value();
}

void SammonProjection::SaveOptions(QSettings &settings)
{
    settings.setValue("dimCount", params->dimCountSpin->value());
}

bool SammonProjection::LoadOptions(QSettings &settings)
{
    if(settings.contains("dimCount")) params->dimCountSpin->setValue(settings.value("dimCount").toInt());
    return true;
}

void SammonProjection::SaveParams(QTextStream &file)
{
    file << "projectOptions" << ":" << "dimCount" << " " << params->dimCountSpin->value() << "\n";
}

bool SammonProjection::LoadParams(QString name, float value)
{
    if(name.endsWith("dimCount")) params->dimCountSpin->setValue((int)value);
    return true;
}
