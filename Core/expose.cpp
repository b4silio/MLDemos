#include <public.h>
#include <mymaths.h>
#include <basicMath.h>
#include "expose.h"
#include "ui_expose.h"
#include <QClipboard>
#include <QBitmap>
#include <QDebug>
#include <QProgressDialog>

using namespace std;

Expose::Expose(Canvas *canvas, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Expose),
    canvas(canvas)
{
    ui->setupUi(this);
    connect(ui->typeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(Repaint()));
    connect(ui->clipboardButton, SIGNAL(clicked()), this, SLOT(Clipboard()));
    this->setWindowTitle("Multivariate Visualisation");
}

Expose::~Expose()
{
    delete ui;
}

void Expose::DrawData(QPixmap& pixmap, std::vector<fvec> samples, ivec labels, std::vector<dsmFlags> flags, int type, bool bProjected, QStringList names, std::pair<fvec,fvec> bounds)
{
    if(!samples.size() || !labels.size()) return;
    vector<QColor> sampleColors(labels.size());
    FOR(i, labels.size())
    {
        QColor color = SampleColor[labels[i]%SampleColorCnt];
        sampleColors[i] = color;
    }
    DrawData(pixmap, samples, sampleColors, flags, type, bProjected, false, names, bounds);
}

void Expose::DrawData(QPixmap& pixmap, std::vector<fvec> samples, std::vector<QColor> sampleColors, std::vector<dsmFlags> flags, int type, bool bProjected, bool bLearned, QStringList names, std::pair<fvec,fvec> bounds)
{
    if(!samples.size()) return;
    int w = pixmap.width(), h = pixmap.height();

    int dim = samples[0].size();
    int gridX = dim;
    int gridY = dim;

    fvec mins = bounds.first;
    fvec maxes = bounds.second;
    if(!bounds.first.size())
    {
        mins.resize(dim, FLT_MAX);
        maxes.resize(dim, -FLT_MIN);
        FOR(d, dim)
        {
            FOR(i, samples.size())
            {
                mins[d] = min(mins[d], samples[i][d]);
                maxes[d] = max(maxes[d], samples[i][d]);
            }
        }
        bounds.first = mins;
        bounds.second = maxes;
    }
    fvec diffs(dim, 0);
    FOR(d, dim)
	{
		diffs[d] = maxes[d] - mins[d];
	}

    int pad = 20;
    int mapW = w - pad*2, mapH = h - pad*2;
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    switch(type)
    {
    case 0: // scatterplots
    {
        mapW = w/gridX - pad*2;
        mapH = h/gridX - pad*2;
        int radiusBase = max(5.f, 5 * sqrtf(mapW / 200.f));
#ifndef WIN32
        QProgressDialog progress("Generating Scatterplots...", "Abort", 0, dim*dim, NULL);
        progress.setWindowModality(Qt::WindowModal);
        int progressCnt = 0;
#endif
        QList<QPixmap> maps;
        FOR(index0, dim)
        {
            FOR(index1, dim)
			{
                QPixmap map(mapW + 2*pad,mapH + 2*pad);
                int smallW = map.width() - 2*pad, smallH = map.height() - 2*pad;
                if(!bLearned) map.fill(Qt::white);
                else
                {
                    QBitmap bitmap(map.size());
                    bitmap.clear();
                    map.setMask(bitmap);
                    map.fill(Qt::transparent);
                }
                QPainter painter(&map);
                painter.setRenderHint(QPainter::Antialiasing);

				if(diffs[index0] != 0.f && diffs[index1] != 0.f)
				{
					FOR(i, samples.size())
					{
                        if(flags[i] == _TRAJ || flags[i] == _OBST) continue;
						float x = samples[i][index0];
						float y = samples[i][index1];
						x = (x-mins[index0])/diffs[index0];
						y = (y-mins[index1])/diffs[index1];
						QPointF point(y*smallW + pad, x*smallH + pad);
                        float radius = radiusBase;
                        if(bLearned)
                        {
                            radius = radiusBase + radiusBase/2;
                            if(i < sampleColors.size()) painter.setPen(QPen(sampleColors[i], radiusBase/2));
                            painter.setBrush(Qt::NoBrush);
                            painter.drawEllipse(QRectF(point.x()-radius/2.,point.y()-radius/2.,radius,radius));
                            radius += radiusBase/2-1;
                            painter.setPen(QPen(Qt::black, 0.5));
                            painter.drawEllipse(QRectF(point.x()-radius/2.,point.y()-radius/2.,radius,radius));
                        }
                        else
                        {
                            if(i < sampleColors.size()) painter.setBrush(sampleColors[i]);
                            painter.setPen(Qt::black);
                            painter.drawEllipse(QRectF(point.x()-radius/2.,point.y()-radius/2.,radius,radius));
                        }
					}
                }
                painter.setBrush(Qt::NoBrush);
                painter.setPen(Qt::black);
                painter.setRenderHint(QPainter::Antialiasing, false);
                painter.drawRect(pad/2,pad/2,smallW+pad, smallH+pad);
                QString text =  QString("x%1  x%2").arg(index1+1).arg(index0+1);
                if(index0 < names.size() && index1 < names.size()) text = names.at(index1) + " " + names.at(index0);
                if(bProjected) text = QString("e%1  e%2").arg(index1+1).arg(index0+1);
                painter.drawText(pad/2+1, map.height()-pad/2-1,text);
                maps.push_back(map);
#ifndef WIN32
                progress.setValue(progressCnt++);
                progress.update();
                if (progress.wasCanceled()) break;
            }
            if (progress.wasCanceled()) break;
        }
        progress.setValue(dim*dim);
#else
            }
        }
#endif

        FOR(i, maps.size())
        {
            int xIndex = i%gridX;
            int yIndex = i/gridX;
            painter.drawPixmap(QPoint(xIndex*w/gridX, yIndex*h/gridY), maps[i]);
        }
    }
        break;
    case 1: // parallel coordinates
    {
        painter.setRenderHint(QPainter::Antialiasing, false);
        FOR(d, dim)
        {
            float x = d*mapW/(float)(dim-1) + pad;
            painter.setPen(Qt::black);
            painter.drawLine(x, pad, x, mapH+pad);
            QString text =  QString("x%1").arg(d+1);
            if(d < names.size()) text = names.at(d);
            if(bProjected) text = QString("e%1").arg(d+1);
            painter.drawText(x-10, mapH+2*pad-4, text);
        }

        painter.setRenderHint(QPainter::Antialiasing);
        FOR(i, samples.size())
        {
            QPointF old;
			FOR(d, dim)
            {
				if(diffs[d] == 0.f) continue;
				float x = d*mapW/(float)(dim-1) + pad;
                float y = samples[i][d];
                y = (y-mins[d])/(maxes[d] - mins[d]);
                QPointF point(x,pad + y*mapH);
                float radius = 7;
                QColor color = Qt::black;
                if(i < sampleColors.size()) color = sampleColors[i];
                painter.setBrush(color);
                painter.setPen(Qt::black);
                painter.drawEllipse(QRectF(point.x()-radius/2.,point.y()-radius/2.,radius,radius));
                if(color == Qt::white) color = Qt::black;
                painter.setPen(color);
                if(d) painter.drawLine(point, old);
                old = point;
            }
        }
    }
        break;
    case 2: // radial graphs
    {
        float radius = min(mapW, mapH)/3.f;
        QPointF center(mapW*0.5f, mapH*0.5f);
        QPointF old;
        painter.setPen(Qt::black);
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(center, radius, radius);
        FOR(d, dim)
        {
			float theta = d/(float)(dim)*2*M_PI;
            QPointF point = QPointF(cos(theta), sin(theta))*radius;
            painter.setBrush(Qt::white);
            painter.drawEllipse(center+point, 4, 4);
            QString text =  QString("x%1").arg(d+1);
            if(d < names.size()) text = names.at(d);
            if(bProjected) text = QString("e%1").arg(d+1);
            painter.drawText(center + point*1.1, text);
            old = point;
        }

        painter.setPen(Qt::black);
        FOR(i, samples.size())
        {
            QPointF samplePoint;
            float dimSum = 0;
            FOR(d, dim)
            {
				if(diffs[d] == 0.f) continue;
				float theta = d/(float)(dim)*2*M_PI;
                QPointF point = QPointF(cos(theta), sin(theta))*radius;
                float value = (samples[i][d]-mins[d])/(maxes[d]-mins[d]);
                samplePoint += point*value;
                dimSum += value;
            }
            samplePoint /= dimSum;
            float drawRadius = 7;
            QPointF point = center + samplePoint;
            QColor color = Qt::black;
            if(i < sampleColors.size()) color = sampleColors[i];
            painter.setBrush(color);
            painter.drawEllipse(QRectF(point.x()-drawRadius/2.,point.y()-drawRadius/2.,drawRadius,drawRadius));
        }
    }
        break;
    case 3: // andrews plots
    {
        float radius = min(mapW, mapH)/3.f;
        QPointF center(mapW*0.5f, mapH*0.5f);
        painter.setPen(Qt::black);

        // f(t) = x0/sqrt(2) + x1*sin(t) + x2*cos(t) + x3*sin(2t) + x4*cos(2t) + x5*sin(3t) + x6*cos(3t) + x7*sin(4t)
        vector<fvec> values(samples.size());
        const int steps = 200;
        float minv=FLT_MAX, maxv=-FLT_MAX;
        FOR(i, samples.size())
        {
            values[i].resize(steps);
            FOR(j, steps)
            {
                float t = j/(float)steps*(M_PI*2) - M_PI;
                float value = 0;
                FOR(d, dim)
                {
					if(diffs[d] == 0.f) continue;
					float v = (samples[i][d]-mins[d])/(maxes[d]-mins[d]);
                    if(!d) value += v*sqrtf(2.f);
                    else
                    {

                        value += v * (d%2 ? sin(t*((d+1)/2)) : cos(t*((d+1)/2)));
                    }
                }
                values[i][j] = value;
                minv = min(minv, value);
                maxv = max(maxv, value);
            }
        }

        painter.setRenderHint(QPainter::Antialiasing, false);
        painter.setPen(Qt::black);
        painter.drawLine(pad, mapH-10+pad, mapW+pad, mapH-10+pad);
        QFont font = painter.font();
        font.setPointSize(9);
        painter.setFont(font);
        QStringList ticks;
        ticks << "-pi" << "-pi/2" << "0" << "pi/2" << "pi";
        FOR(i, 5)
        {
            int x = i*mapW/(4);
            painter.setRenderHint(QPainter::Antialiasing, false);
            painter.drawLine(pad + x, mapH-10 - 4 + pad, pad + x, mapH-10 + 4 + pad);
            painter.setRenderHint(QPainter::Antialiasing);
            painter.drawText(pad + x-25,mapH-10 + 4 + pad, 50, 20, Qt::AlignCenter, ticks[i]);
        }

        painter.setRenderHint(QPainter::Antialiasing);
        FOR(i, values.size())
        {
            QColor color = Qt::black;
            if(i < sampleColors.size()) color = sampleColors[i];
            if(color == Qt::white) color = Qt::black;
            painter.setPen(QPen(color,0.5));

            QPointF old;
            FOR(j, values[i].size())
            {
                float value = (values[i][j]-minv)/(maxv-minv);
                QPointF point = QPointF(j*pixmap.width()/steps, value*mapH + pad);
                if(j) painter.drawLine(point, old);
                old = point;
            }
        }
    }
        break;
    }
}

void Expose::DrawTrajectories(QPixmap& pixmap, vector< vector<fvec> > trajectories, ivec labels, int type, int drawMode, std::pair<fvec,fvec> bounds)
{
    if(!trajectories.size() || !labels.size()) return;
    vector<QColor> sampleColors(labels.size());
    FOR(i, labels.size())
    {
        QColor color = SampleColor[labels[i]%SampleColorCnt];
        sampleColors[i] = color;
    }
    DrawTrajectories(pixmap, trajectories, sampleColors, type, drawMode, bounds);
}

void Expose::DrawTrajectories(QPixmap& pixmap, vector< vector<fvec> > trajectories, std::vector<QColor> sampleColors, int type, int drawMode, std::pair<fvec,fvec> bounds)
{
    if(!trajectories.size()) return;
    int w = pixmap.width(), h = pixmap.height();

    int dim = trajectories[0][0].size()/2; // trajectories have velocity embedded so we only want half the dimensions
    if(!sampleColors.size()) dim = trajectories[0][0].size(); // unless we are actually drawing the trajectories without velocity
    int gridX = dim;
    int gridY = dim;

    fvec mins = bounds.first;
    fvec maxes = bounds.second;
    if(!bounds.first.size())
    {
        mins.resize(dim, FLT_MAX);
        maxes.resize(dim, -FLT_MIN);
        FOR(d, dim)
        {
            FOR(i, trajectories.size())
            {
                FOR(j, trajectories[i].size())
                {
                    mins[d] = min(mins[d], trajectories[i][j][d]);
                    maxes[d] = max(maxes[d], trajectories[i][j][d]);
                }
            }
        }
        bounds.first = mins;
        bounds.second = maxes;
    }
    fvec diffs(dim, 0);
    FOR(d, dim)
    {
        diffs[d] = maxes[d] - mins[d];
    }

    int pad = 20;
    int mapW = w - pad*2, mapH = h - pad*2;
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    if(type==0)
    {
        mapW = w/gridX - pad*2;
        mapH = h/gridX - pad*2;
        int radiusBase = max(4.f, 4 * sqrtf(mapW / 200.f));

        QList<QPixmap> maps;
        FOR(index0, dim)
        {
            FOR(index1, dim)
            {
                QPixmap map(mapW + 2*pad,mapH + 2*pad);
                int smallW = map.width() - 2*pad, smallH = map.height() - 2*pad;

                QBitmap bitmap(map.size());
                bitmap.clear();
                map.setMask(bitmap);
                map.fill(Qt::transparent);

                QPainter painter(&map);
                painter.setRenderHint(QPainter::Antialiasing);

                if(diffs[index0] != 0.f && diffs[index1] != 0.f)
                {
                    int sampleColorCounter = 0;
                    FOR(i, trajectories.size())
                    {
                        QPointF oldPoint, firstPoint, point;
                        int count = trajectories[i].size();
                        if(drawMode == 0 && i < sampleColors.size()) painter.setBrush(sampleColors[sampleColorCounter]);
                        else painter.setBrush(Qt::black);
                        FOR(j, count)
                        {
                            fvec pt = trajectories[i][j];
                            float x = pt[index0];
                            float y = pt[index1];
                            x = (x-mins[index0])/diffs[index0];
                            y = (y-mins[index1])/diffs[index1];
                            point = QPointF(y*smallW + pad, x*smallH + pad);
                            if(drawMode == 0) painter.setPen(QPen(Qt::black, 0.5));
                            else if(drawMode == 1) painter.setPen(QPen(Qt::green, 1));
                            if(j)
                            {
                                painter.drawLine(point, oldPoint);
                                if(j<count-1 && sampleColors.size())
                                {
                                    painter.drawEllipse(point, max(1,radiusBase/4), max(1,radiusBase/4));
                                }
                            }
                            else firstPoint = point;
                            oldPoint = point;
                            sampleColorCounter++;
                        }
                        if(drawMode == 0)
                        {
                            painter.setBrush(Qt::NoBrush);
                            painter.setPen(Qt::green);
                            painter.drawEllipse(firstPoint, radiusBase, radiusBase);
                            painter.setPen(Qt::red);
                            painter.drawEllipse(point, radiusBase/2, radiusBase/2);
                        }
                    }
                }
                /*
                painter.setBrush(Qt::NoBrush);
                painter.setPen(Qt::black);
                painter.setRenderHint(QPainter::Antialiasing, false);
                painter.drawRect(pad/2,pad/2,smallW+pad, smallH+pad);
                QString text =  QString("x%1  x%2").arg(index1+1).arg(index0+1);
                if(index0 < names.size() && index1 < names.size()) text = names.at(index1) + " " + names.at(index0);
                if(bProjected) text = QString("e%1  e%2").arg(index1+1).arg(index0+1);
                painter.drawText(pad/2+1, map.height()-pad/2-1,text);
                */
                maps.push_back(map);
            }
        }

        FOR(i, maps.size())
        {
            int xIndex = i%gridX;
            int yIndex = i/gridX;
            painter.drawPixmap(QPoint(xIndex*w/gridX, yIndex*h/gridY), maps[i]);
        }
    }
}

void Expose::DrawVariableData(QPixmap& pixmap, std::vector<fvec> samples, ivec labels, int type, fvec params, bool bProjected, QStringList names)
{
    if(!samples.size() || !labels.size()) return;
    vector<QColor> sampleColors(labels.size());
    FOR(i, labels.size())
    {
        QColor color = SampleColor[labels[i]%SampleColorCnt];
        sampleColors[i] = color;
    }
    DrawVariableData(pixmap, samples, sampleColors, type, params, bProjected, false, names);
}

void Expose::DrawVariableData(QPixmap& pixmap, std::vector<fvec> samples, std::vector<QColor> sampleColors, int type, fvec params, bool bProjected, bool bLearned, QStringList names)
{
    if(!samples.size()) return;
    int w = pixmap.width(), h = pixmap.height();

    int dim = samples[0].size();
    int gridX = dim;
    int gridY = dim;

    fvec mins(dim, FLT_MAX), maxes(dim, -FLT_MIN), diffs(dim, 0);
    FOR(d, dim)
    {
        FOR(i, samples.size())
        {
            mins[d] = min(mins[d], samples[i][d]);
            maxes[d] = max(maxes[d], samples[i][d]);
        }
    }
    FOR(d, dim)
    {
        diffs[d] = maxes[d] - mins[d];
    }

    int pad = 20;
    int mapW = w - pad*2, mapH = h - pad*2;
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setPen(Qt::black);
    switch(type)
    {
    case 0: // bubble graph
    {
        painter.setRenderHint(QPainter::Antialiasing, false);
        painter.drawLine(pad, mapH+pad, mapW+pad, mapH+pad);
        painter.drawLine(pad, pad, pad, mapH+pad);

        int xIndex = params[0];
        int yIndex = params[1];
        int sIndex = params[2];
        if(sIndex == -1)
        {
            srand48(0);
            srand(0);
        }
        painter.setRenderHint(QPainter::Antialiasing);
        FOR(i, samples.size())
        {
            float x = (samples[i][xIndex]-mins[xIndex])/(diffs[xIndex]);
            float y = (samples[i][yIndex]-mins[yIndex])/(diffs[yIndex]);
            QPointF point(x*mapW + pad,y*mapH + pad);

            float radius = 10;
            if(sIndex != -1)
            {
                radius = (samples[i][sIndex]-mins[sIndex])/(diffs[sIndex]);
                radius = radius*60 + 3;
            }
            else
            {
                radius = drand48()*40 + 3;
            }

            QColor color = Qt::black;
            if(i < sampleColors.size()) color = sampleColors[i];
            painter.setBrush(color);
            painter.setPen(Qt::black);
            painter.setOpacity(0.5f);
            painter.drawEllipse(QRectF(point.x()-radius/2.,point.y()-radius/2.,radius,radius));
        }
    }
    break;
    }
}
void Expose::GenerateAndrewsPlots()
{
    std::vector<fvec> samples = canvas->data->GetSamples();
    ivec labels = canvas->data->GetLabels();
    if(!samples.size()) return;
    int dim = samples[0].size();

    fvec mins(dim, FLT_MAX), maxes(dim, -FLT_MIN);
    FOR(d, dim)
    {
        FOR(i, samples.size())
        {
            mins[d] = min(mins[d], samples[i][d]);
            maxes[d] = max(maxes[d], samples[i][d]);
        }
    }

    int pad = 20;
    int mapW = (ui->scrollArea->width()-12) - pad*2, mapH = (ui->scrollArea->height()-12) - pad*2;

    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    pixmap = QPixmap(ui->scrollArea->width(), ui->scrollArea->height());
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    float radius = min(mapW, mapH)/3.f;
    QPointF center(mapW*0.5f, mapH*0.5f);
    QPointF old;
    painter.setPen(Qt::black);

    // f(t) = x0/sqrt(2) + x1*sin(t) + x2*cos(t) + x3*sin(2t) + x4*cos(2t) + x5*sin(3t) + x6*cos(3t) + x7*sin(4t)
    vector<fvec> values(samples.size());
    const int steps = 200;
    float minv=FLT_MAX, maxv=-FLT_MAX;
    FOR(i, samples.size())
    {
        values[i].resize(steps);
        FOR(j, steps)
        {
            float t = j/(float)steps*(M_PI*2) - M_PI;
            float value = 0;
            FOR(d, dim)
            {
                float v = (samples[i][d]-mins[d])/(maxes[d]-mins[d]);
                if(!d) value += v*sqrtf(2.f);
                else
                {

                    value += v * (d%2 ? sin(t*((d+1)/2)) : cos(t*((d+1)/2)));
                }
            }
            values[i][j] = value;
            minv = min(minv, value);
            maxv = max(maxv, value);
        }
    }

    FOR(i, values.size())
    {
        FOR(j, values[i].size())
        {
            float value = (values[i][j]-minv)/(maxv-minv);
            QPointF point = QPointF(j*pixmap.width()/steps, value*mapH + pad);
            QColor color = SampleColor[labels[i]%SampleColorCnt];
			painter.setPen(QPen(color,0.5));
			if(j) painter.drawLine(point, old);
            old = point;
        }
    }

    ui->display->setPixmap(pixmap);
    ui->display->repaint();
}


void Expose::GenerateRadialGraph()
{
    std::vector<fvec> samples = canvas->data->GetSamples();
    ivec labels = canvas->data->GetLabels();
    if(!samples.size()) return;
    int dim = samples[0].size();

    fvec mins(dim, FLT_MAX), maxes(dim, -FLT_MIN);
    FOR(d, dim)
    {
        FOR(i, samples.size())
        {
            mins[d] = min(mins[d], samples[i][d]);
            maxes[d] = max(maxes[d], samples[i][d]);
        }
    }

    int pad = 20;
    int mapW = (ui->scrollArea->width()-12) - pad*2, mapH = (ui->scrollArea->height()-12) - pad*2;

    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    pixmap = QPixmap(ui->scrollArea->width(), ui->scrollArea->height());
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    float radius = min(mapW, mapH)/3.f;
    QPointF center(mapW*0.5f, mapH*0.5f);
    QPointF old;
    painter.setPen(Qt::black);
    FOR(d, dim)
    {
        float theta = d/(float)(dim)*2*M_PI;
        QPointF point = QPointF(cos(theta), sin(theta))*radius;
        if(d) painter.drawLine(center + point, center + old);
        painter.drawText(center + point*1.1, QString("e%1").arg(d+1));
        old = point;
    }
    painter.drawLine(center + QPointF(1.f, 0.f)*radius, center + old);

    painter.setRenderHint(QPainter::Antialiasing);
    FOR(i, samples.size())
    {
        QPointF samplePoint;
        float dimSum = 0;
        FOR(d, dim)
        {
            float theta = d/(float)(dim)*2*M_PI;
            QPointF point = QPointF(cos(theta), sin(theta))*radius;
            float value = (samples[i][d]-mins[d])/(maxes[d]-mins[d]);
            samplePoint += point*value;
            dimSum += value;
        }
        samplePoint /= dimSum;
        float drawRadius = 7;
        Canvas::drawSample(painter, center + samplePoint, drawRadius, labels[i]);
        QColor color = SampleColor[labels[i]%SampleColorCnt];
        painter.setPen(color);
    }

    ui->display->setPixmap(pixmap);
    ui->display->repaint();
}

void Expose::GenerateParallelCoords()
{
    std::vector<fvec> samples = canvas->data->GetSamples();
    ivec labels = canvas->data->GetLabels();
    if(!samples.size()) return;
    int dim = samples[0].size();

    fvec mins(dim, FLT_MAX), maxes(dim, -FLT_MIN);
    FOR(d, dim)
    {
        FOR(i, samples.size())
        {
            mins[d] = min(mins[d], samples[i][d]);
            maxes[d] = max(maxes[d], samples[i][d]);
        }
    }

    int pad = 20;
    int mapW = (ui->scrollArea->width()-12) - pad*2, mapH = (ui->scrollArea->height()-12) - pad*2;

    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    pixmap = QPixmap(ui->scrollArea->width(), ui->scrollArea->height());
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    FOR(d, dim)
    {
        float x = d*mapW/(float)(dim-1) + pad;
        painter.setPen(Qt::black);
        painter.drawLine(x, pad, x, mapH+pad);
        painter.drawText(x-10, mapH+2*pad-4, QString("e%1").arg(d+1));
    }

    painter.setRenderHint(QPainter::Antialiasing);
    FOR(i, samples.size())
    {
        QPointF old;
        FOR(d, dim)
        {
            float x = d*mapW/(float)(dim-1) + pad;
            float y = samples[i][d];
            y = (y-mins[d])/(maxes[d] - mins[d]);
            QPointF point(x,pad + y*mapH);
            float radius = 7;
            Canvas::drawSample(painter, point, radius, labels[i]);
            QColor color = SampleColor[labels[i]%SampleColorCnt];
            painter.setPen(color);
            if(d) painter.drawLine(point, old);
            old = point;
        }
    }

    ui->display->setPixmap(pixmap);
    ui->display->repaint();
}

void Expose::GenerateScatterPlot(bool bCheckOnly)
{
    std::vector<fvec> samples = canvas->data->GetSamples();
    ivec labels = canvas->data->GetLabels();
    if(!samples.size()) return;
    int dim = samples[0].size();
    bool bEvenCount = dim%2 == 1;
    int gridX = dim;
    int gridY = dim;

    fvec mins(dim, FLT_MAX), maxes(dim, -FLT_MIN);
    FOR(d, dim)
    {
        FOR(i, samples.size())
        {
            mins[d] = min(mins[d], samples[i][d]);
            maxes[d] = max(maxes[d], samples[i][d]);
        }
    }

    int pad = 20;
    int mapW = (ui->scrollArea->width()-12)/gridX - pad*2;
    int mapH = (ui->scrollArea->height()-12)/gridX - pad*2;

    bool bScroll = false;
    if(mapW < 100 || mapH < 100)
    {
        bScroll = true;
        mapW = max(mapW, 100);
        mapH = max(mapH, 100);
    }
    if(bScroll && bCheckOnly) return;

    QList<QPixmap> maps;
    FOR(index0, dim)
    {
        FOR(index1, dim)
        {
            QPixmap map(mapW + 2*pad,mapH + 2*pad);
            int w = map.width() - 2*pad, h = map.height() - 2*pad;
            map.fill(Qt::white);
            QPainter painter(&map);
            painter.setRenderHint(QPainter::Antialiasing);

            FOR(i, samples.size())
            {
                float x = samples[i][index0];
                float y = samples[i][index1];
                x = (x-mins[index0])/(maxes[index0] - mins[index0]);
                y = (y-mins[index1])/(maxes[index1] - mins[index1]);
                QPointF point(y*w + pad, x*h + pad);
                float radius = 5;
                Canvas::drawSample(painter, point, radius, labels[i]);
            }
            painter.setBrush(Qt::NoBrush);
            painter.setPen(Qt::black);
            painter.setRenderHint(QPainter::Antialiasing, false);
            painter.drawRect(pad/2,pad/2,w+pad, h+pad);
            painter.drawText(pad/2+1, map.height()-pad/2-1, QString("e%1 x e%2").arg(index1+1).arg(index0+1));
            maps.push_back(map);
        }
    }

    if(bScroll)
    {
        pixmap = QPixmap((mapW+2*pad)*gridX, (mapH+2*pad)*gridY);
        ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    }
    else
    {
        pixmap = QPixmap(ui->scrollArea->width(), ui->scrollArea->height());
        ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    FOR(i, maps.size())
    {
        int xIndex = i%gridX;
        int yIndex = i/gridX;
        painter.drawPixmap(QPoint(xIndex*pixmap.width()/gridX, yIndex*pixmap.height()/gridY), maps[i]);
    }
    ui->display->setPixmap(pixmap);
    ui->display->repaint();
}

void Expose::resizeEvent( QResizeEvent *event )
{
    if(ui->typeCombo->currentIndex() == 0 && ui->scrollArea->horizontalScrollBarPolicy() == Qt::ScrollBarAlwaysOn)
    {
        GenerateScatterPlot(true);
    }
    else Repaint();
    repaint();
}

void Expose::Repaint()
{
    switch(ui->typeCombo->currentIndex())
    {
    case 0:
        GenerateScatterPlot();
        break;
    case 1:
        GenerateParallelCoords();
        break;
    case 2:
        GenerateRadialGraph();
        break;
    case 3:
        GenerateAndrewsPlots();
        break;
    }
    repaint();
}

void Expose::Clipboard()
{
    QImage image = ui->display->pixmap()->toImage();
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setImage(image);
}

void Expose::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    if(!canvas) return;
    if(pixmap.isNull()) Repaint();
}
