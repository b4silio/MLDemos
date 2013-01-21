#include "visualization.h"
#include "ui_visualization.h"

#include <algorithm>
#include <canvas.h>
#include <QPainter>
#include <QBitmap>
#include <QDebug>
using namespace std;

Visualization::Visualization(Canvas *canvas, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Visualization), spacer(0),
    canvas(canvas), data(canvas->data)
{
    ui->setupUi(this);
    connect(ui->typeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(OptionsChanged()));
    connect(ui->grayscaleCheck, SIGNAL(clicked()), this, SLOT(OptionsChanged()));
    connect(ui->updateButton, SIGNAL(clicked()), this, SLOT(Update()));
    connect(ui->flavorCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(Update()));
    connect(ui->inputCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(Update()));
    connect(ui->x1Combo, SIGNAL(currentIndexChanged(int)), this, SLOT(Update()));
    connect(ui->x2Combo, SIGNAL(currentIndexChanged(int)), this, SLOT(Update()));
    connect(ui->x3Combo, SIGNAL(currentIndexChanged(int)), this, SLOT(Update()));
    connect(ui->grayscaleCheck, SIGNAL(clicked()), this, SLOT(Update()));
    ui->display->installEventFilter(this);
    spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    OptionsChanged();
}

Visualization::~Visualization()
{
    delete ui;
}

bool Visualization::eventFilter(QObject *obj, QEvent *event)
{
    if(obj == ui->display && event->type() == QEvent::MouseButtonDblClick)
    {
        if(ui->optionsWidget->isVisible()) ui->optionsWidget->hide();
        else ui->optionsWidget->show();
        adjustSize();
        Update();
        return true;
    }
    if(obj == ui->display && event->type() == QEvent::Paint)
    {
        QPainter painter;
        painter.begin(ui->display);
        if(displayPixmap.isNull())
        {
            painter.fillRect(0,0,ui->display->width(), ui->display->height(), Qt::white);
        }
        else
        {
            painter.drawPixmap(0,0, ui->display->width(), ui->display->height(), displayPixmap, 0,0,displayPixmap.width(), displayPixmap.height());
        }
        return true;
    }
    return false;
}

void Visualization::resizeEvent(QResizeEvent *event)
{
    Update();
}

void Visualization::OptionsChanged()
{
    int index = ui->typeCombo->currentIndex();
    ui->flavorCombo->hide();
    ui->zoomSlider->hide();
    ui->axesWidget->hide();
    ui->inputCombo->hide();
    switch(index)
    {
    case 0: // samples: scatterplots
        ui->zoomSlider->show();
        break;
    case 1: // samples: parallel coordinates
        break;
    case 2: // samples: radial graphs
        break;
    case 3: // samples: andrews plots
        break;
    case 4: // samples: bubble plots
        ui->axesWidget->show();
        break;
    case 5: // distribution: Individual plots
    {
        ui->inputCombo->show();
        ui->flavorCombo->blockSignals(true);
        ui->flavorCombo->clear();
        ui->flavorCombo->addItem("Histograms");
        ui->flavorCombo->addItem("Box Plots");
        ui->flavorCombo->addItem("HDR Box Plots");
        ui->flavorCombo->addItem("Box Percentile Plots");
        ui->flavorCombo->addItem("Violin Plots");
        ui->flavorCombo->addItem("Raws");
        ui->flavorCombo->addItem("Star Plots");
        ui->flavorCombo->setCurrentIndex(1);
        ui->flavorCombo->show();
        ui->flavorCombo->blockSignals(false);
    }
        break;
    case 6: // distribution: splatter plots
        ui->axesWidget->show();
        ui->flavorCombo->blockSignals(true);
        ui->flavorCombo->clear();
        ui->flavorCombo->addItem("Jet Colormap");
        ui->flavorCombo->addItem("Autumn");
        ui->flavorCombo->addItem("Heat");
        ui->flavorCombo->setCurrentIndex(0);
        ui->flavorCombo->show();
        ui->flavorCombo->blockSignals(false);
        break;
    case 7: // distribution: Correlation
    {
        ui->axesWidget->hide();
        ui->flavorCombo->blockSignals(true);
        ui->flavorCombo->clear();
        ui->flavorCombo->addItem("Scatterplot Circles");
        ui->flavorCombo->addItem("Scatterplot Ellipses");
        ui->flavorCombo->addItem("Scatterplot Graphs");
        ui->flavorCombo->setCurrentIndex(0);
        ui->flavorCombo->show();
        ui->flavorCombo->blockSignals(false);
    }
        break;
    }
    Update();
}

void Visualization::Update()
{
    int index = ui->typeCombo->currentIndex();
    displayPixmap = QPixmap();
    ui->display->setMinimumSize(0,0);
    ui->display->setMaximumSize(65536,65536);
    ui->scrollAreaWidgetContents->layout()->removeWidget(spacer);
    ui->scrollAreaWidgetContents->adjustSize();

    switch(index)
    {
    case 0: // samples: scatterplots
        GenerateScatterPlot();
        break;
    case 1: // samples: parallel coordinates
        GenerateParallelCoords();
        break;
    case 2: // samples: radial graphs
        GenerateRadialGraph();
        break;
    case 3: // samples: andrews plots
        GenerateAndrewsPlot();
        break;
    case 4: // samples: bubble plots
        GenerateBubblePlot();
        break;
    case 5: // distribution: Individual plots
        ui->scrollAreaWidgetContents->layout()->addWidget(spacer);
        //ui->scrollAreaWidgetContents->layout()->;
        ui->scrollAreaWidgetContents->adjustSize();
        GenerateIndividualPlot();
        break;
    case 6: // distribution: splatter plots
        GenerateSplatterPlot();
        break;
    case 7: // distribution: Correlation
        GenerateCorrelationPlot();
        break;
    }
    ui->display->repaint();
}

void Visualization::UpdateDims()
{
    int dims = data->GetDimCount();
    int x1 = ui->x1Combo->currentIndex();
    int x2 = ui->x2Combo->currentIndex();
    int x3 = ui->x3Combo->currentIndex();
    ui->x1Combo->blockSignals(true);
    ui->x2Combo->blockSignals(true);
    ui->x3Combo->blockSignals(true);
    ui->x1Combo->clear();
    ui->x2Combo->clear();
    ui->x3Combo->clear();
    QStringList dimNames = canvas->dimNames;
    if(dimNames.size() == 0)
    {
        FOR(i, dims)
        {
            dimNames << QString("Dimension %1").arg(i+1);
        }
    }
    dimNames << QString("none");
    FOR(i, dimNames.size())
    {
        ui->x1Combo->addItem(dimNames[i]);
        ui->x2Combo->addItem(dimNames[i]);
        ui->x3Combo->addItem(dimNames[i]);
    }
    if(x1 >= 0 && x1 < dims) ui->x1Combo->setCurrentIndex(x1);
    else ui->x1Combo->setCurrentIndex(min(dims+1,0));
    if(x1 >= 0 && x2 < dims) ui->x2Combo->setCurrentIndex(x2);
    else ui->x2Combo->setCurrentIndex(min(dims+1,1));
    if(x1 >= 0 && x3 < dims) ui->x3Combo->setCurrentIndex(x3);
    else ui->x3Combo->setCurrentIndex(min(dims+1,2));
    ui->x1Combo->blockSignals(false);
    ui->x2Combo->blockSignals(false);
    ui->x3Combo->blockSignals(false);
}

void Visualization::GenerateAndrewsPlot()
{
    std::vector<fvec> samples = data->GetSamples();
    ivec labels = data->GetLabels();
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
    QPixmap pixmap = QPixmap(ui->scrollArea->width()-2, ui->scrollArea->height()-2);
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
            QColor color;
            if(ui->grayscaleCheck->isChecked())
            {
                int gray = (labels[i]%SampleColorCnt)*255/SampleColorCnt;
                color = QColor(gray,gray,gray);
            }
            else color = SampleColor[labels[i]%SampleColorCnt];
            if(labels[i]==0) color = Qt::black;
            painter.setPen(QPen(color,0.5));
            if(j) painter.drawLine(point, old);
            old = point;
        }
    }
    displayPixmap = pixmap;
}


void Visualization::GenerateRadialGraph()
{
    std::vector<fvec> samples = data->GetSamples();
    ivec labels = data->GetLabels();
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
    QPixmap pixmap = QPixmap(ui->scrollArea->width()-2, ui->scrollArea->height()-2);
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
        QColor color;
        if(ui->grayscaleCheck->isChecked())
        {
            int gray = (labels[i]%SampleColorCnt)*255/SampleColorCnt;
            color = QColor(gray,gray,gray);
        }
        else color = SampleColor[labels[i]%SampleColorCnt];
        painter.setPen(color);
    }
    displayPixmap = pixmap;
}

void Visualization::GenerateParallelCoords()
{
    std::vector<fvec> samples = data->GetSamples();
    ivec labels = data->GetLabels();
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
    QPixmap pixmap = QPixmap(ui->scrollArea->width(), ui->scrollArea->height());
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
            QColor color;
            if(ui->grayscaleCheck->isChecked())
            {
                int gray = (labels[i]%SampleColorCnt)*255/SampleColorCnt;
                color = QColor(gray,gray,gray);
            }
            else color = SampleColor[labels[i]%SampleColorCnt];
            if(labels[i]==0) color = Qt::black;
            painter.setPen(color);
            if(d) painter.drawLine(point, old);
            old = point;
        }
    }
    displayPixmap = pixmap;
}

void Visualization::GenerateScatterPlot(bool bCheckOnly)
{
    std::vector<fvec> samples = data->GetSamples();
    ivec labels = data->GetLabels();
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
                if(ui->grayscaleCheck->isChecked())
                {
                    int gray = (labels[i]%SampleColorCnt)*255/SampleColorCnt;
                    QColor color = QColor(gray,gray,gray);
                    painter.setBrush(color);
                    painter.setPen(Qt::black);
                    painter.drawEllipse(point, radius, radius);
                }
                else Canvas::drawSample(painter, point, radius, labels[i]);
            }
            painter.setBrush(Qt::NoBrush);
            painter.setPen(Qt::black);
            painter.setRenderHint(QPainter::Antialiasing, false);
            painter.drawRect(pad/2,pad/2,w+pad, h+pad);
            painter.drawText(pad/2+1, map.height()-pad/2-1, QString("e%1 x e%2").arg(index1+1).arg(index0+1));
            maps.push_back(map);
        }
    }
    QPixmap pixmap;

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
    displayPixmap = pixmap;
    ui->display->setMinimumSize(displayPixmap.size());
    ui->display->setMaximumSize(displayPixmap.size());
}

void Visualization::GenerateBubblePlot()
{
    std::vector<fvec> samples = data->GetSamples();
    ivec labels = data->GetLabels();
    if(!samples.size()) return;
    int dim = samples[0].size();
    if(!dim) return;
    int xIndex = ui->x1Combo->currentIndex();
    int yIndex = ui->x2Combo->currentIndex();
    int zIndex = ui->x3Combo->currentIndex();
    if(xIndex >= dim) xIndex = 0;
    if(yIndex >= dim) yIndex = min(dim-1,0);
    if(zIndex >= dim) zIndex = -1;

    fvec mins(dim, FLT_MAX), maxes(dim, -FLT_MIN);
    FOR(d, dim)
    {
        FOR(i, samples.size())
        {
            mins[d] = min(mins[d], samples[i][d]);
            maxes[d] = max(maxes[d], samples[i][d]);
        }
    }
    fvec diffs = maxes-mins;

    int pad = 20;
    int mapW = (ui->scrollArea->width()-12) - pad*2, mapH = (ui->scrollArea->height()-12) - pad*2;

    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QPixmap pixmap = QPixmap(ui->scrollArea->width()-2, ui->scrollArea->height()-2);
    pixmap.fill(Qt::white);

    QPainter painter(&pixmap);
    painter.drawLine(pad, mapH+pad, mapW+pad, mapH+pad);
    painter.drawLine(pad, pad, pad, mapH+pad);

    if(zIndex == -1)
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
        if(zIndex != -1)
        {
            radius = (samples[i][zIndex]-mins[zIndex])/(diffs[zIndex]);
            radius = radius*60 + 3;
        }
        else radius = drand48()*40 + 3;

        QColor color;
        if(ui->grayscaleCheck->isChecked())
        {
            int gray = (labels[i]%SampleColorCnt)*255/SampleColorCnt;
            color = QColor(gray,gray,gray);
        }
        else color = SampleColor[labels[i]%SampleColorCnt];
        if(labels[i] == 0) color = Qt::lightGray;
        painter.setBrush(color);
        painter.setPen(Qt::black);
        painter.setOpacity(0.5f);
        painter.drawEllipse(QRectF(point.x()-radius/2.,point.y()-radius/2.,radius,radius));
    }

    displayPixmap = pixmap;
}

void Visualization::GenerateIndividualPlot()
{
    std::vector<fvec> samples = data->GetSamples();
    ivec labels = data->GetLabels();
    if(!samples.size()) return;
    int classCount = data->GetClassCount(labels);
    int dim = samples[0].size();

    int inputType = ui->inputCombo->currentIndex();
    int flavorType = ui->flavorCombo->currentIndex();

    fvec mins(dim,FLT_MAX), maxes(dim,-FLT_MAX);

    // we parse the data to gather density information on each class and dimension
    map<int,vector< pair<fvec,fvec> > > classGraphData;
    vector< map<int,fvec> > dimClassData;
    FOR(d, dim)
    {
        map<int,fvec> classData;
        FOR(i, samples.size())
        {
            float v = samples[i][d];
            classData[labels[i]].push_back(v);
            if(mins[d]>v) mins[d] = v;
            if(maxes[d]<v) maxes[d] = v;
        }
        if(mins[d] == maxes[d])
        {
            mins[d] = mins[d]/2;
            maxes[d] = mins[d]*3/2;
        }
        FORIT(classData, int, fvec)
        {
            if(!classGraphData.count(it->first))
            {
                classGraphData[it->first].resize(dim);
            }
            fvec box = BoxPlot(it->second);
            fvec density = Density(it->second, box[4], box[0], (flavorType==3?15:31)); // 3: violin plot
            classGraphData[it->first][d] = make_pair(box, density);
        }
        dimClassData.push_back(classData);
    }

    // we go through all the data to find the minima and maxima for each class
    //fvec mins(dim,FLT_MAX), maxes(dim,-FLT_MAX);
    map<int,fvec> dimMins, dimMaxes;
    FOR(i, samples.size())
    {
        int c = labels[i];
        if(!dimMins.count(c)) dimMins[c] = fvec(dim,FLT_MAX);
        if(!dimMaxes.count(c)) dimMaxes[c] = fvec(dim,-FLT_MAX);
        FOR(d, dim)
        {
            float v = samples[i][d];
            //if(mins[d] > v) mins[d] = v;
            //if(maxes[d] < v) maxes[d] = v;
            if(dimMins[c][d] > v) dimMins[c][d] = v;
            if(dimMaxes[c][d] < v) dimMaxes[c][d] = v;
        }
    }

    // we choose the proper size for each individual graph
    int pad = 20;
    int w,h;
    if(flavorType < 6)
    {
        w = 80;
        h = 220;
    }
    else // star plots
    {
        w = 100;
        h = 120;
    }

    int mapW = ui->scrollArea->width()-12;
    int mapH = ui->scrollArea->height()-12;

    // depending on how many classes/dimensions we have, we adjust the grid count
    int gridX, gridY;
    int count, perItemCount;
    int totalCount = dim*classCount;
    switch(inputType)
    {
    case 0: // dimension
        count = dim;
        perItemCount = classCount;
        break;
    case 1: // class
        count = classCount;
        perItemCount = dim;
        break;
    }
    if(flavorType == 6)
    {
        totalCount = count;
        perItemCount = 1;
    }
    gridX = max(1,min(count,(mapW/(w*perItemCount+pad*2))));
    gridY = max(1,count/gridX + (count%gridX ? 1 : 0) );
    if(mapW<(w*perItemCount+pad)*gridX) mapW = (w*perItemCount+pad)*gridX;
    mapH = gridY*h;

    // and we set the size of the pixmap
    QPixmap pixmap(mapW, mapH);
    QBitmap bitmap(mapW, mapH);
    pixmap.setMask(bitmap);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(Qt::black, 2));

    painter.setPen(Qt::black);
    FOR(i, count)
    {
        QPixmap smallPix = GetGraphPixmap(flavorType, inputType, dim, classCount,
                                          i, w, h,
                                          classGraphData, dimClassData,
                                          mins, dimMins,
                                          maxes, dimMaxes);
        painter.setBrush(SampleColor[i%SampleColorCnt]);
        QPoint tl((i%gridX)*(w*perItemCount+pad), (i/gridX)*h);
        painter.drawRect(tl.x(), tl.y(), w*perItemCount+pad, h);
        painter.drawPixmap(tl, smallPix);
    }

    displayPixmap = pixmap;
    ui->display->setMinimumSize(displayPixmap.size());
    ui->display->setMaximumSize(displayPixmap.size());
    ui->display->adjustSize();
    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
}

void Visualization::GenerateSplatterPlot()
{
    std::vector<fvec> samples = data->GetSamples();
    ivec labels = data->GetLabels();
    if(!samples.size()) return;
    int dim = samples[0].size();
    if(!dim) return;
    int xIndex = ui->x1Combo->currentIndex();
    int yIndex = ui->x2Combo->currentIndex();
    int zIndex = ui->x3Combo->currentIndex();
    if(xIndex >= dim) xIndex = 0;
    if(yIndex >= dim) yIndex = min(dim-1,0);
    if(zIndex >= dim) zIndex = -1;

    fvec mins(dim, FLT_MAX), maxes(dim, -FLT_MIN);
    FOR(d, dim)
    {
        FOR(i, samples.size())
        {
            mins[d] = min(mins[d], samples[i][d]);
            maxes[d] = max(maxes[d], samples[i][d]);
        }
    }
    fvec diffs = maxes-mins;

    int pad = 20;
    int mapW = (ui->scrollArea->width()-12) - pad*2, mapH = (ui->scrollArea->height()-12) - pad*2;

    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QPixmap pixmap = QPixmap(ui->scrollArea->width(), ui->scrollArea->height());
    pixmap.fill(Qt::white);

    QPainter painter(&pixmap);
    painter.drawLine(pad, mapH+pad, mapW+pad, mapH+pad);
    painter.drawLine(pad, pad, pad, mapH+pad);

    painter.setRenderHint(QPainter::Antialiasing);
    FOR(i, samples.size())
    {
        float x = (samples[i][xIndex]-mins[xIndex])/(diffs[xIndex]);
        float y = (samples[i][yIndex]-mins[yIndex])/(diffs[yIndex]);
        QPointF point(x*mapW + pad,y*mapH + pad);

        float radius = 10;
        if(zIndex != -1)
        {
            radius = (samples[i][zIndex]-mins[zIndex])/(diffs[zIndex]);
            radius = radius*60 + 3;
        }
        else
        {
            radius = drand48()*40 + 3;
        }

        QColor color;
        if(ui->grayscaleCheck->isChecked())
        {
            int gray = (labels[i]%SampleColorCnt)*255/SampleColorCnt;
            color = QColor(gray,gray,gray);
        }
        else color = SampleColor[labels[i]%SampleColorCnt];
        if(labels[i] == 0) color = Qt::lightGray;
        painter.setBrush(color);
        painter.setPen(Qt::black);
        painter.setOpacity(0.5f);
        painter.drawEllipse(QRectF(point.x()-radius/2.,point.y()-radius/2.,radius,radius));
    }

    displayPixmap = pixmap;
}

void Visualization::GenerateCorrelationPlot()
{

}

QPixmap Visualization::GetGraphPixmap(int type, int inputType, int dim, int classCount,
                                      int index, int w, int h,
                                      std::map<int,std::vector< std::pair<fvec,fvec> > > classGraphData,
                                      std::vector< std::map<int,fvec> > dimClassData,
                                      fvec mins, std::map<int,fvec> dimMins,
                                      fvec maxes, std::map<int,fvec> dimMaxes)
{
    if(type == 6) return GetStarPixmap(inputType, dim, classCount, index, w, h, mins, dimMins, maxes, dimMaxes);

    int perItemCount;
    int d;
    int cCnt;
    switch(inputType)
    {
    case 0:
        perItemCount = classCount;
        d = index;
        cCnt = 0;
        break;
    case 1:
        perItemCount = dim;
        d = 0;
        cCnt = index;
        break;
    }
    int pad = 20;
    h = h-pad*1.5;
    int res = h;

    QPixmap pixmap(w*perItemCount + pad*2, h+pad*1.5);
    /*
    pixmap.fill(Qt::white);
    */
    QBitmap bitmap(w*perItemCount + pad*2, h+pad*1.5);
    pixmap.setMask(bitmap);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);

    float minv = mins[d], maxv = maxes[d];
    if(inputType == 1)
    {
        FOR(i, mins.size())
        {
            if(minv > mins[i]) minv = mins[i];
            if(maxv < maxes[i]) maxv = maxes[i];
        }
    }

    float edge = minv;
    float delta = maxv - minv;

    // we need to write down the scales
    // first we need to find the closest decimal
    int decimals = -log(delta)/log(10.) + (delta > 1 ? 0 : 1);
    // now we know how "big" our data is: 10^(decimals), we can compute the closest boundaries
    float powDecimals = pow(10., (double)(decimals+1));
    float minsText = decimals ? int(minv*powDecimals)/powDecimals : minv;
    float maxesText = decimals ? int(maxv*powDecimals)/powDecimals : maxv;
    int minsPoint = (minsText-edge)/delta*res;
    int maxesPoint = (maxesText-edge)/delta*res;

    char format[255];
    sprintf(format,"%%.%df", max(0,decimals+1));
    QPoint firstTl(0,pad/2);

    QFont font = painter.font();
    font.setPointSize(9);
    painter.setFont(font);
    painter.setPen(QPen(Qt::black,1));
    char text[255];
    int axisSteps = 10;
    int axisH = h / (axisSteps+1);
    FOR(i, axisSteps+1)
    {
        float value = (maxesText - minsText)*i/(axisSteps) + minsText;
        int y = h - ((maxesPoint-minsPoint)*i/(axisSteps) + minsPoint);
        sprintf(text, format, value);
        painter.drawText(firstTl.x(), firstTl.y() + y-axisH/2, pad*1.5-6, axisH, Qt::AlignVCenter | Qt::AlignRight, QString(text));
        painter.drawLine(firstTl + QPointF(pad*1.5-3, y), firstTl + QPointF(pad*1.5, y));
    }
    painter.drawLine(firstTl.x() + pad*1.5, firstTl.y() + h - maxesPoint, firstTl.x() + pad*1.5, firstTl.y() + h - minsPoint);

    // now we gather the data we need
    vector<fvec> boxplots, densities, datas;
    int classId = 0;
    switch(inputType)
    {
    case 0: // dimension
    {
        int cCnt = 0;
        for(map<int,vector<pair<fvec,fvec> > >::iterator it = classGraphData.begin(); it != classGraphData.end(); it++, cCnt++)
        {
            fvec boxplot = it->second[d].first;
            fvec density = it->second[d].second;
            boxplots.push_back(boxplot);
            densities.push_back(density);
            datas.push_back(dimClassData[d][it->first]);
        }
    }
        break;
    case 1: // classCount
    {
        int c=0;
        for(map<int,vector<pair<fvec,fvec> > >::iterator it = classGraphData.begin(); it != classGraphData.end(); it++, c++)
        {
            if(c == cCnt)
            {
                classId = it->first;
                FOR(d, dim)
                {
                    fvec boxplot = it->second[d].first;
                    fvec density = it->second[d].second;
                    boxplots.push_back(boxplot);
                    densities.push_back(density);
                    datas.push_back(dimClassData[d][classId]);
                }
                break;
            }
        }
    }
        break;
    }

    // then we write the graph description/name
    int graphCorner = pad*1.5;
    int graphWidth = perItemCount*w;
    font = painter.font();
    font.setPointSize(12);
    painter.setFont(font);
    switch(inputType)
    {
    case 0:
        painter.drawText(graphCorner, h+pad/2, graphWidth, pad, Qt::AlignHCenter|Qt::AlignCenter, ui->x1Combo->itemText(d));
        break;
    case 1:
    {
        QString className = QString("Class %1").arg(classId);
        if(canvas->classNames.count(classId)) className = canvas->classNames[classId];
        painter.drawText(graphCorner, h+pad/2, graphWidth, pad, Qt::AlignHCenter|Qt::AlignCenter, className);
    }
        break;
    }

    // then we draw the actual graphs
    FOR(b, boxplots.size())
    {
        QPoint tl(pad + b*w,pad/2);
        fvec boxplot = boxplots[b];
        fvec density = densities[b];

        int topPoint = h - (boxplot[0]-edge)/delta*res;
        int highPoint = h - (boxplot[1]-edge)/delta*res;
        int medPoint = h - (boxplot[2]-edge)/delta*res;
        int lowPoint = h - (boxplot[3]-edge)/delta*res;
        int bottomPoint = h - (boxplot[4]-edge)/delta*res;

        float maxDensity = -FLT_MAX;
        int maxDensityIndex = 0;
        FOR(i, density.size())
        {
            if(maxDensity < density[i])
            {
                maxDensity = density[i];
                maxDensityIndex = i;
            }
        }
        QColor color;
        if(ui->grayscaleCheck->isChecked())
        {
            switch(inputType)
            {
            case 0: // dimension
                color = QColor(cCnt*255/classCount,cCnt*255/classCount,cCnt*255/classCount);
                break;
            case 1: // class
                color = QColor(d*255/dim,d*255/dim,d*255/dim);
                break;
            }
        }
        else color = SampleColor[b%SampleColorCnt];

        switch(type)
        {
        case 0: // Histograms
        {
            int meanPoint = (boxplot[5]-edge)/delta*res;
            int sigmaPoint = boxplot[6]/delta*res;

            painter.setPen(QPen(Qt::black,2));
            painter.setBrush(color);
            painter.drawRect(tl.x() + 20, tl.y() + h-meanPoint, 40, meanPoint);
            painter.drawLine(tl.x() + 25, tl.y() + h-meanPoint+sigmaPoint, tl.x() + 55, tl.y() + h-meanPoint+sigmaPoint);
            painter.drawLine(tl.x() + 25, tl.y() + h-meanPoint-sigmaPoint, tl.x() + 55, tl.y() + h-meanPoint-sigmaPoint);
            painter.setPen(QPen(Qt::black,1));
            painter.drawLine(tl.x() + 40, tl.y() + h-meanPoint+sigmaPoint, tl.x() + 40, tl.y() + h-meanPoint-sigmaPoint);
        }
            break;
        case 1: // Box Plots
        {
            painter.setBrush(color);
            painter.setPen(QPen(Qt::black,2));
            painter.drawLine(tl + QPointF(15, bottomPoint),tl + QPointF(65, bottomPoint));
            painter.drawLine(tl + QPointF(15, topPoint),tl + QPointF(65, topPoint));
            painter.drawLine(tl + QPointF(40, bottomPoint),tl + QPointF(40, topPoint));
            painter.drawRect(tl.x() + 20, tl.y() + lowPoint, 40, highPoint - lowPoint);
            painter.setPen(QPen(Qt::black,1));
            painter.drawLine(tl + QPointF(20, medPoint),tl + QPointF(60, medPoint));
        }
            break;
        case 2: // HDR Box Plots
        {
            // we use the watershed algorithm to isolate separate regions
            fvec vals = density;
            sort(vals.begin(), vals.end());
            fvec water = density;
            fvec highWater, midWater, lowWater;
            FOR(i,vals.size())
            {
                float v = vals[vals.size()-i-1];
                if(!highWater.size() && v < maxDensity) highWater = water;
                //if(!highWater.size() && v < 0.75*maxDensity) highWater = water;
                if(!midWater.size() && v < 0.5*maxDensity) midWater = water;
                if(!lowWater.size() && v < 0.25*maxDensity) lowWater = water;
                FOR(j, water.size())
                {
                    if(water[j] >= v) water[j] = -1;
                }
            }
            // we need to create the segments
            fvec highSegments, midSegments, lowSegments;
            bool bHighOpen = false;
            bool bMidOpen = false;
            bool bLowOpen = false;
            FOR(i, water.size())
            {
                if(highWater.size() && !bHighOpen && highWater[i] == -1)
                {
                    bHighOpen = true;
                    highSegments.push_back(i);
                }
                else if(highWater.size() && bHighOpen && highWater[i] != -1)
                {
                    bHighOpen = false;
                    highSegments.push_back(i);
                }
                if(midWater.size() && !bMidOpen && midWater[i] == -1)
                {
                    bMidOpen = true;
                    midSegments.push_back(i);
                }
                else if(midWater.size() && bMidOpen && midWater[i] != -1)
                {
                    bMidOpen = false;
                    midSegments.push_back(i);
                }
                if(lowWater.size() && !bLowOpen && lowWater[i] == -1)
                {
                    bLowOpen = true;
                    lowSegments.push_back(i);
                }
                else if(lowWater.size() && bLowOpen && lowWater[i] != -1)
                {
                    bLowOpen = false;
                    lowSegments.push_back(i);
                }
            }
            if(bHighOpen) highSegments.push_back(water.size());
            if(bMidOpen) midSegments.push_back(water.size());
            if(bLowOpen) lowSegments.push_back(water.size());

            QColor lightGray = color;
            QColor gray = QColor(color.red()*0.7, color.green()*0.7, color.blue()*0.7);
            QColor darkGray = QColor(color.red()*0.4, color.green()*0.4, color.blue()*0.4);
            painter.setPen(QPen(Qt::black,2));
            painter.setBrush(lightGray);
            painter.drawRect(tl.x() + 20, tl.y() + bottomPoint, 40, topPoint-bottomPoint);
            painter.setBrush(gray);
            painter.setPen(Qt::black);

            /*
            FOR(i, lowSegments.size()/2)
            {
                int yBottom = lowSegments[i*2]*(topPoint-bottomPoint)/(density.size()+1) + bottomPoint;
                int yTop = lowSegments[i*2+1]*(topPoint-bottomPoint)/(density.size()+1) + bottomPoint;
                painter.drawRect(tl.x() + 20, tl.y() + yBottom, 40, yTop-yBottom);
            }
            */
            //painter.setBrush(QColor(120,120,120));
            FOR(i, midSegments.size()/2)
            {
                int yBottom = midSegments[i*2]*(topPoint-bottomPoint)/(density.size()+1) + bottomPoint;
                int yTop = midSegments[i*2+1]*(topPoint-bottomPoint)/(density.size()+1) + bottomPoint;
                painter.drawRect(tl.x() + 22, tl.y() + yBottom, 35, yTop-yBottom);
            }
            painter.setBrush(darkGray);
            FOR(i, highSegments.size()/2)
            {
                int yBottom = highSegments[i*2]*(topPoint-bottomPoint)/(density.size()+1) + bottomPoint;
                int yTop = highSegments[i*2+1]*(topPoint-bottomPoint)/(density.size()+1) + bottomPoint;
                painter.drawRect(tl.x() + 22, tl.y() + yBottom, 35, yTop-yBottom);
            }
        }
            break;
        case 3: // Box Percentile Plots
        {
            int highPoint = -1;
            int midPoint = -1;
            int lowPoint = -1;
            float highX=0, midX=0, lowX=0;

            painter.setRenderHint(QPainter::Antialiasing);
            painter.setPen(color);
            painter.setBrush(color);
            float oldX = 0, oldY = 0;
            float v = 0;
            int quartile = 0;
            FOR(i, density.size())
            {
                float y = ((i+1)/(float)(density.size()+1))*(topPoint-bottomPoint) + bottomPoint;
                if(quartile < 2) v += density[i];
                else v -= density[i];
                float x = v*w/2;
                if(!i)
                {
                    QPolygonF poly;
                    poly << tl + QPointF(40, bottomPoint)
                         << tl + QPointF(40+x, y)
                         << tl + QPointF(40-x, y);
                    painter.drawPolygon(poly);
                }
                else
                {
                    if(fabs(oldY - y) < 1) painter.drawLine(tl + QPointF(40-x,y), tl + QPointF(40+x,y));
                    else
                    {
                        QPolygonF poly;
                        poly << tl + QPointF(40-x, y)
                             << tl + QPointF(40+x, y)
                             << tl + QPointF(40+oldX, oldY)
                             << tl + QPointF(40-oldX, oldY);
                        painter.drawPolygon(poly);
                    }
                }
                if(quartile == 0 && v > 0.25) quartile = 1;
                if(quartile == 1 && v > 0.5) quartile = 2;
                if(quartile == 2 && (v < 0.25 || v > 0.75)) quartile = 3;
                oldX = x;
                oldY = y;
            }
            painter.setPen(color);
            QPolygonF poly;
            poly << tl + QPointF(40+oldX, oldY)
                 << tl + QPointF(40-oldX, oldY)
                 << tl + QPointF(40, topPoint);
            painter.setBrush(color);
            painter.drawPolygon(poly);

            painter.setPen(Qt::black);
            oldX = 0; oldY = 0; v = 0; quartile = 0;
            FOR(i, density.size())
            {
                float y = ((i+1)/(float)(density.size()+1))*(topPoint-bottomPoint) + bottomPoint;
                if(quartile < 2) v += density[i];
                else v -= density[i];
                float x = v*w/2;
                if(!i)
                {
                    painter.drawLine(tl + QPointF(40, bottomPoint), tl + QPointF(40-x, y));
                    painter.drawLine(tl + QPointF(40, bottomPoint), tl + QPointF(40+x, y));
                }
                else
                {
                    painter.drawLine(tl + QPointF(40-oldX, oldY), tl + QPointF(40-x, y));
                    painter.drawLine(tl + QPointF(40+oldX, oldY), tl + QPointF(40+x, y));
                }
                if(quartile == 0 && v > 0.25)
                {
                    quartile = 1;
                    highPoint = y;
                    highX = x;
                }
                if(quartile == 1 && v > 0.5)
                {
                    quartile = 2;
                    midPoint = y;
                    midX = x;
                }
                if(quartile == 2 && (v < 0.25 || v > 0.75))
                {
                    quartile = 3;
                    lowPoint = y;
                    lowX = x;
                }
                oldX = x;
                oldY = y;
            }

            painter.setPen(Qt::black);
            painter.drawLine(tl + QPointF(40-oldX, oldY), tl + QPointF(40, topPoint));
            painter.drawLine(tl + QPointF(40+oldX, oldY), tl + QPointF(40, topPoint));
            if(lowPoint != -1) painter.drawLine(tl + QPointF(40-lowX, lowPoint), tl + QPointF(40+lowX, lowPoint));
            if(midPoint != -1) painter.drawLine(tl + QPointF(40-midX, midPoint), tl + QPointF(40+midX, midPoint));
            if(highPoint != -1) painter.drawLine(tl + QPointF(40-highX, highPoint), tl + QPointF(40+highX, highPoint));
        }
            break;
        case 4: // Violin Plots
        {
            float oldX = 0, oldY = 0, v = 0;
            int quartile = 0;

            painter.setRenderHint(QPainter::Antialiasing);
            painter.setPen(color);
            painter.setBrush(color);
            oldX = 0; oldY = 0; v = 0; quartile = 0;
            maxDensity = -FLT_MAX;
            FOR(i, density.size()) if(maxDensity < density[i]) maxDensity = density[i];
            FOR(i, density.size())
            {
                float y = ((i+1)/(float)(density.size()+1))*(topPoint-bottomPoint) + bottomPoint;
                float x = (density[i]/maxDensity)*(w/4);
                if(!i)
                {
                    QPolygonF poly;
                    poly << tl + QPointF(40, bottomPoint)
                         << tl + QPointF(40+x, y)
                         << tl + QPointF(40-x, y);
                    painter.drawPolygon(poly);
                }
                else
                {
                    if(fabs(oldY - y) < 1) painter.drawLine(tl + QPointF(40-x,y), tl + QPointF(40+x,y));
                    else
                    {
                        QPolygonF poly;
                        poly << tl + QPointF(40-x, y)
                             << tl + QPointF(40+x, y)
                             << tl + QPointF(40+oldX, oldY)
                             << tl + QPointF(40-oldX, oldY);
                        painter.drawPolygon(poly);
                    }
                }
                oldX = x;
                oldY = y;
            }
            painter.setPen(color);
            QPolygonF poly;
            poly << tl + QPointF(40+oldX, oldY)
                 << tl + QPointF(40-oldX, oldY)
                 << tl + QPointF(40, topPoint);
            painter.setBrush(color);
            painter.drawPolygon(poly);

            painter.setPen(Qt::black);
            painter.setBrush(Qt::NoBrush);
            oldX = 0; oldY = 0; v = 0; quartile = 0;
            FOR(i, density.size())
            {
                float y = ((i+1)/(float)(density.size()+1))*(topPoint-bottomPoint) + bottomPoint;
                if(quartile < 2) v += density[i];
                else v -= density[i];
                float x = (density[i]/maxDensity)*w/4;
                if(!i)
                {
                    painter.drawLine(tl + QPointF(40, bottomPoint), tl + QPointF(40-x, y));
                    painter.drawLine(tl + QPointF(40, bottomPoint), tl + QPointF(40+x, y));
                }
                else
                {
                    painter.drawLine(tl + QPointF(40-oldX, oldY), tl + QPointF(40-x, y));
                    painter.drawLine(tl + QPointF(40+oldX, oldY), tl + QPointF(40+x, y));
                }
                oldX = x;
                oldY = y;
            }
            painter.drawLine(tl + QPointF(40-oldX, oldY), tl + QPointF(40, topPoint));
            painter.drawLine(tl + QPointF(40+oldX, oldY), tl + QPointF(40, topPoint));

            painter.setBrush(Qt::gray);
            painter.drawLine(tl + QPointF(40, bottomPoint),tl + QPointF(40, topPoint));
            painter.drawRect(tl.x() + 38, tl.y() + lowPoint, 4, (highPoint-lowPoint));
            painter.drawLine(tl + QPointF(38, medPoint),tl + QPointF(42, medPoint));
        }
            break;
        case 5: // Raws
        {
            fvec data = datas[b];
            sort(data.begin(), data.end());

            painter.setPen(Qt::black);
            painter.setBrush(color);

            maxDensity = -FLT_MAX;
            FOR(i, density.size()) if(maxDensity < density[i]) maxDensity = density[i];
            int steps = density.size();
            ivec counts(steps, 0);
            FOR(i, data.size())
            {
                int yIndex = (data[i]-edge)/delta*steps;
                counts[yIndex]++;
            }
            int maxCounts = 0;
            FOR(i, counts.size()) if(maxCounts < counts[i]) maxCounts = counts[i];
            counts = ivec(steps, 0);

            const float radius = 3;
            float flipFlop = 1.f;
            FOR(i, data.size())
            {
                float y = (data[i]-edge)/delta*res;
                int yIndex = (data[i]-edge)/delta*steps;
                float x = 40 + (counts[yIndex]/float(maxCounts))*w*0.25*flipFlop;
                painter.drawEllipse(QPointF(tl.x() + radius/2 + x, tl.y() +  h - y - radius/2), radius, radius);
                counts[yIndex]++;
                flipFlop *= -1.f;
            }
        }
            break;
        }
    }
    return pixmap;
}

QPixmap Visualization::GetStarPixmap(int inputType, int dim, int classCount, int index, int w, int h,
                                     fvec mins, std::map<int,fvec> dimMins,
                                     fvec maxes, std::map<int,fvec> dimMaxes)
{
    int d;
    int cCnt;
    switch(inputType)
    {
    case 0:
        d = index;
        cCnt = 0;
        break;
    case 1:
        d = 0;
        cCnt = index;
        break;
    }
    int pad = 20;
    w += pad;
    h = h-pad*1.5;

    int classId = cCnt;
    int c=0;
    FORIT(dimMaxes, int, fvec)
    {
        if(c==cCnt)
        {
            classId = it->first;
            break;
        }
        c++;
    }

    QPixmap pixmap(w, h+pad*1.5);
    QBitmap bitmap(w, h+pad*1.5);
    pixmap.setMask(bitmap);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(Qt::black, 2));

    // then we write the graph description/name
    QFont font = painter.font();
    font.setPointSize(12);
    painter.setFont(font);
    switch(inputType)
    {
    case 0:
        painter.drawText(0, h+pad/2, w, pad, Qt::AlignHCenter|Qt::AlignCenter, ui->x1Combo->itemText(d));
        break;
    case 1:
    {
        QString className = QString("Class %1").arg(classId);
        if(canvas->classNames.count(classId)) className = canvas->classNames[classId];
        painter.drawText(0, h+pad/2, w, pad, Qt::AlignHCenter|Qt::AlignCenter, className);
    }
        break;
    }

    fvec cmins(classCount,FLT_MAX), cmaxes(classCount,-FLT_MAX);
    fvec dMins, dMaxes;
    switch(inputType)
    {
    case 0:
    {
        int cnt=0;
        FORIT(dimMins, int, fvec)
        {
            dMins.push_back(dimMins[it->first][d]);
            dMaxes.push_back(dimMaxes[it->first][d]);
            if(cmins[cnt] > dimMins[it->first][d]) cmins[cnt] = dimMins[it->first][d];
            if(cmaxes[cnt] < dimMaxes[it->first][d]) cmaxes[cnt] = dimMaxes[it->first][d];
            cnt++;
        }
    }
        break;
    case 1:
        dMins = dimMins[classId];
        dMaxes = dimMaxes[classId];
        break;
    }
    float minminv=FLT_MAX, maxmaxv=-FLT_MAX;
    FOR(i, mins.size())
    {
        if(minminv > mins[i]) minminv = mins[i];
        if(maxmaxv < maxes[i]) maxmaxv = maxes[i];
    }

    int maxRadius = min(w,h)*0.4;
    QPointF center(w/2, h/2 + pad/2);
    painter.setBrush(Qt::NoBrush);
    FOR(d, dMins.size())
    {
        float minv, maxv;
        switch(inputType)
        {
        case 0:
            minv = (dMins[d]-minminv)/(maxmaxv-minminv);
            maxv = (dMaxes[d]-minminv)/(maxmaxv-minminv);
            //minv = (dMins[d]-cmins[d])/(cmaxes[d]-cmins[d]);
            //maxv = (dMaxes[d]-cmins[d])/(cmaxes[d]-cmins[d]);
            break;
        case 1:
            minv = (dMins[d]-mins[d])/(maxes[d]-mins[d]);
            maxv = (dMaxes[d]-mins[d])/(maxes[d]-mins[d]);
            break;
        }

        // we now draw the arcs in 16th of degree
        float angleStart = (360*16)*d/dMins.size();
        float angleLen = (360*16)/dMins.size();
        float maxR = maxRadius * maxv;
        float minR = maxRadius * minv;
        QColor color = SampleColor[d%SampleColorCnt];
        float ratio = 0.1;
        QColor lighterColor(color.red()*ratio + 255*(1-ratio),
                            color.green()*ratio + 255*(1-ratio),
                            color.blue()*ratio + 255*(1-ratio));
        painter.setBrush(color);
        painter.drawPie(QRectF(center-QPointF(maxR,maxR), center+QPoint(maxR,maxR)),angleStart, angleLen);
        painter.setBrush(lighterColor);
        painter.drawPie(QRectF(center-QPointF(minR,minR), center+QPoint(minR,minR)),angleStart, angleLen);
    }

    return pixmap;
}

fvec Visualization::BoxPlot(fvec data)
{
    fvec boxplot(7);
    if(!data.size()) return boxplot;
    int pad = -16;
    int nanCount = 0;
    FOR(i, data.size()) if(data[i] != data[i]) nanCount++;

    float mean = 0;
    float sigma = 0;
    FOR(i, data.size()) if(data[i]==data[i]) mean += data[i] / (data.size()-nanCount);
    FOR(i, data.size()) if(data[i]==data[i]) sigma += powf(data[i]-mean,2);
    sigma = sqrtf(sigma/(data.size()-nanCount));

    float top, bottom, median, quartLow, quartHi;
    vector<float> outliers;
    vector<float> sorted;

    if(data.size() > 1)
    {
        if(sigma==0)
        {
            sorted = data;
        }
        else
        {
            // we look for outliers using the 3*sigma rule
            FOR(i, data.size())
            {
                if(data[i]!=data[i]) continue;
                if (data[i] - mean < 3*sigma)
                    sorted.push_back(data[i]);
                else outliers.push_back(data[i]);
            }
        }
        if(!sorted.size()) return boxplot;
        sort(sorted.begin(), sorted.end());
        int count = sorted.size();
        int half = count/2;
        bottom = sorted[0];
        top = sorted[sorted.size()-1];

        median = count%2 ? sorted[half] : (sorted[half] + sorted[half - 1])/2;
        if(count < 4)
        {
            quartLow = bottom;
            quartHi = top;
        }
        else
        {
            quartLow = half%2 ? sorted[half/2] : (sorted[half/2] + sorted[half/2 - 1])/2;
            quartHi = half%2 ? sorted[half*3/2] : (sorted[half*3/2] + sorted[half*3/2 - 1])/2;
        }
    }
    else
    {
        mean = top = bottom = median = quartLow = quartHi = data[0];
        sigma = 0;
    }
    boxplot[0] = top;
    boxplot[1] = quartHi;
    boxplot[2] = median;
    boxplot[3] = quartLow;
    boxplot[4] = bottom;
    boxplot[5] = mean;
    boxplot[6] = sigma;
    return boxplot;
}

fvec Visualization::Density(fvec data, float minv, float maxv, int bins)
{
    fvec density(bins,0);
    if(!data.size()) return density;
    const int count = data.size();
    FOR(i, data.size())
    {
        float v = data[i];
        int index = (v-minv)/(maxv-minv)*bins;
        index = min(bins-1,max(0,index));
        density[index] += 1.f/count;
    }
    return density;
}
