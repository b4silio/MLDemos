#include <public.h>
#include <mymaths.h>
#include "expose.h"
#include "ui_expose.h"
#include <QClipboard>

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
