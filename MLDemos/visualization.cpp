#include "visualization.h"
#include "ui_visualization.h"

#include <algorithm>
#include <canvas.h>
#include <QPainter>
#include <QBitmap>
#include <QDebug>

using namespace std;

/*
// color set from
// http://www.dickblick.com/products/prismacolor-nupastel-color-sets/#photos
QColor DimColor[] = {QColor(237,233,230),QColor(42,46,47),QColor(93,84,75),
                     QColor(154,92,77),QColor(82,67,148),QColor(46,80,118),
                     QColor(74,114,184),QColor(68,139,123),QColor(148,182,122),
                     QColor(254,197,68),QColor(231,92,49),QColor(215,85,83)};
int DimColorCnt = 12;
*/
// color set from
// http://cdn.dickblick.com/items/200/34/20034-4809-3-3ww-l.jpg
QColor DimColor[] = {QColor("#ebecee"),QColor("#6e7275"),QColor("#9ba0a4"),
                      QColor("#202221"),QColor("#4c4845"),QColor("#765c4b"),
                      QColor("#62443c"),QColor("#964a32"),QColor("#89553d"),
                      QColor("#ddaf4d"),QColor("#ac732e"),QColor("#b28474"),
                      QColor("#54376d"),QColor("#883058"),QColor("#453983"),
                      QColor("#1d2226"),QColor("#20466b"),QColor("#1552af"),
                      QColor("#2768bc"),QColor("#89b8d4"),QColor("#008099"),
                      QColor("#9ed2c5"),QColor("#3e785f"),QColor("#00897f"),
                      QColor("#349b7e"),QColor("#85bda2"),QColor("#5e634d"),
                      QColor("#9fb670"),QColor("#86ab7f"),QColor("#3e4442"),
                      QColor("#dfde82"),QColor("#f4f1e0"),QColor("#fdbc22"),
                      QColor("#f89d2a"),QColor("#fb8c16"),QColor("#f3e0b8"),
                      QColor("#ffb6a5"),QColor("#fd8539"),QColor("#df4f2d"),
                      QColor("#b63f45"),QColor("#de447a"),QColor("#9a342f"),
                      QColor("#914060"),QColor("#723e40")};
int DimColorCnt = 44;

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
    connect(ui->inputCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(OptionsChanged()));
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
    delete spacer;
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

bool Visualization::SaveScreenshot(QString filename)
{
    return displayPixmap.save(filename);
}

QPixmap Visualization::GetDisplayPixmap()
{
    return displayPixmap;
}

void Visualization::OptionsChanged()
{
    int index = ui->typeCombo->currentIndex();
    ui->flavorCombo->hide();
    ui->zoomSlider->hide();
    ui->axesWidget->hide();
    ui->x1Combo->setEnabled(true);
    ui->x2Combo->setEnabled(true);
    ui->x3Combo->setEnabled(true);
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
        ui->x1Combo->setEnabled(true);
        ui->x2Combo->setEnabled(true);
        ui->x3Combo->setEnabled(true);
        break;
    case 5: // distribution: Individual plots
    {
        if(ui->inputCombo->itemText(1) != "By Class")
        {
            int i = ui->inputCombo->currentIndex();
            ui->inputCombo->blockSignals(true);
            ui->inputCombo->clear();
            ui->inputCombo->addItem("By Dimension");
            ui->inputCombo->addItem("By Class");
            ui->inputCombo->setCurrentIndex(i);
            ui->inputCombo->blockSignals(false);
        }
        ui->inputCombo->show();
        int i = ui->flavorCombo->currentIndex();
        ui->flavorCombo->blockSignals(true);
        ui->flavorCombo->clear();
        ui->flavorCombo->addItem("Histograms");
        ui->flavorCombo->addItem("Box Plots");
        ui->flavorCombo->addItem("HDR Box Plots");
        ui->flavorCombo->addItem("Box Percentile Plots");
        ui->flavorCombo->addItem("Violin Plots");
        ui->flavorCombo->addItem("Raws");
        ui->flavorCombo->addItem("Star Plots");
        ui->flavorCombo->addItem("Radial Density Plots");
        ui->flavorCombo->setCurrentIndex(i>=0&&i<ui->flavorCombo->count() ? i : 1);
        ui->flavorCombo->show();
        ui->flavorCombo->blockSignals(false);
    }
        break;
    case 6: // distribution: Correlation
    {
        int i = ui->flavorCombo->currentIndex();
        ui->flavorCombo->blockSignals(true);
        ui->flavorCombo->clear();
        ui->flavorCombo->addItem("Scatterplot Circles");
        ui->flavorCombo->addItem("Scatterplot Ellipses");
        ui->flavorCombo->addItem("Scatterplot Graphs");
        ui->flavorCombo->setCurrentIndex(i>=0&&i<ui->flavorCombo->count() ? i : 0);
        ui->flavorCombo->show();
        ui->flavorCombo->blockSignals(false);
    }
        break;
        /*
    case 7: // distribution: splatter plots
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
        */
    case 7: // distribution: sample distances
        break;
    case 8: // distribution: density
        if(ui->inputCombo->itemText(1) != "Combined")
        {
            int i = ui->inputCombo->currentIndex();
            ui->inputCombo->blockSignals(true);
            ui->inputCombo->clear();
            ui->inputCombo->addItem("By Dimension");
            ui->inputCombo->addItem("Combined");
            ui->inputCombo->setCurrentIndex(i);
            ui->inputCombo->blockSignals(false);
        }
        ui->inputCombo->show();
        if(!ui->inputCombo->currentIndex()) ui->axesWidget->show();
        ui->x1Combo->setEnabled(true);
        ui->x2Combo->setEnabled(false);
        ui->x3Combo->setEnabled(false);
        int i = ui->flavorCombo->currentIndex();
        ui->flavorCombo->blockSignals(true);
        ui->flavorCombo->clear();
        ui->flavorCombo->addItem("Overlapping Kernel");
        ui->flavorCombo->addItem("Cumulative Kernel");
        ui->flavorCombo->addItem("StreamGraph");
        ui->flavorCombo->setCurrentIndex(i>=0&&i<ui->flavorCombo->count() ? i : 0);
        ui->flavorCombo->show();
        ui->flavorCombo->blockSignals(false);
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
        ui->scrollAreaWidgetContents->adjustSize();
        GenerateIndividualPlot();
        break;
    case 6: // distribution: Correlation
        ui->scrollAreaWidgetContents->layout()->addWidget(spacer);
        ui->scrollAreaWidgetContents->adjustSize();
        GenerateCorrelationPlot();
        break;
        /*
    case 7: // distribution: splatter plots
        GenerateSplatterPlot();
        break;
        */
    case 7:
        GenerateSampleDistancePlot();
        break;
    case 8: // distribution: density
        GenerateDensityPlot();
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
    bool bClassInput = ui->inputCombo->isVisible() && ui->inputCombo->currentIndex();
    QStringList dimNames;
    if(!bClassInput)
    {
        dimNames = canvas->dimNames;
        if(dimNames.size() == 0)
        {
            FOR(i, dims)
            {
                dimNames << QString("Dimension %1").arg(i+1);
            }
        }
    }
    else
    {
        int classCount = data->GetClassCount(data->GetLabels());
        FOR(c, classCount) dimNames << canvas->GetClassString(c);
    }
    dimNames << QString("none");
    FOR(i, dimNames.size())
    {
        if(i<dimNames.size()-1) ui->x1Combo->addItem(dimNames[i]);
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
    QPixmap pixmap = QPixmap(ui->scrollArea->width()-2, ui->scrollArea->height()-2);
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
    int bProjected = data->bProjected;
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
            QString labelText = bProjected ? QString("e%1 x e%2").arg(index1+1).arg(index0+1) : QString("x%1 x x%2").arg(index1+1).arg(index0+1);
            painter.drawText(pad/2+1, map.height()-pad/2-1, labelText);
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
            fvec &data = it->second;
            float mean=0, sigma=0;
            FOR(i, data.size()) mean += data[i];
            mean /= data.size();
            FOR(i, data.size()) sigma += (data[i]-mean)*(data[i]-mean);
            sigma /= data.size();
            sigma = sqrt(sigma);
            fvec box = BoxPlot(data);
            fvec density = KernelDensity(data, sigma, box[4], box[0], flavorType == 7 ? 47 : 31);
            //fvec density = Density(data, box[4], box[0], (flavorType==3?15:31)); // 3: violin plot
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
    else if(flavorType == 6) // star plots
    {
        w = 200;
        h = 220;
    }
    else // radial plots
    {
        w = 300;
        h = 320;
    }

    int mapW = ui->scrollArea->width()-24;
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
    if(flavorType >= 6)
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
    //QBitmap bitmap(mapW, mapH);
    //bitmap.clear();
    //pixmap.setMask(bitmap);
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
        QPoint tl((i%gridX)*(w*perItemCount+pad), (i/gridX)*h);
        if(flavorType <= 6)
        {
            switch(inputType)
            {
            case 0:
                painter.setBrush(DimColor[i%DimColorCnt]);
                break;
            case 1:
                painter.setBrush(SampleColor[i%SampleColorCnt]);
                break;
            }
            painter.drawRect(tl.x(), tl.y(), w*perItemCount+pad, h);
        }
        painter.drawPixmap(tl, smallPix);
    }

    displayPixmap = pixmap;
    ui->display->setMinimumSize(displayPixmap.size());
    ui->display->setMaximumSize(displayPixmap.size());
    ui->display->adjustSize();
    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
}

void adaptFontSize(QPainter * painter, int flags, QRectF rect, QString text){
    QFont font = painter->font();
    QRect fontBoundRect;
    fontBoundRect = painter->fontMetrics().boundingRect(rect.toRect(),flags, text);
    while(rect.width() < fontBoundRect.width() ||  rect.height() < fontBoundRect.height()){
        font.setPointSizeF(font.pointSizeF()*0.95);
        painter->setFont(font);
        fontBoundRect = painter->fontMetrics().boundingRect(rect.toRect(),flags, text);
    }
}

void Visualization::GenerateCorrelationPlot()
{
    std::vector<fvec> samples = data->GetSamples();
    ivec labels = data->GetLabels();
    if(!samples.size()) return;
    int dim = samples[0].size();
    int gridX = dim;
    int gridY = dim;
    int flavorType = ui->flavorCombo->currentIndex();

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
    int w = max(24,min(100, (ui->scrollArea->width()-12-2*pad)/gridX));
    int h = w;
    //int mapW = (ui->scrollArea->width()-12)/gridX - pad*2;
    //int mapH = (ui->scrollArea->height()-12)/gridX - pad*2;
    int mapW = gridX * w + pad*2;
    int mapH = gridY * h + pad*2;

    // we compute the correlations
    // sum((x - muX)*(y-muY)) / sqrt(sigmaX*sigmaY);
    // or
    // (n*sum(x*y) - sum(x)*sum(y))/(sqrt(n*sum(x*x)-(sum(x)^2)*sqrt(n*sum(y*y)-(sum(y)^2));

    fvec sums(dim,0), sqsum(dim,0);
    FOR(i, samples.size())
    {
        FOR(d, dim)
        {
            sums[d] += samples[i][d];
            sqsum[d] += samples[i][d]*samples[i][d];
        }
    }

    double n = samples.size();
    dvec corr(dim*dim, 0);
    FOR(d, dim) corr[dim*d + d] = 1.f;
    FOR(d1, dim)
    {
        FOR(d2, d1)
        {
            if(d1==d2) continue;
            double a=0;
            FOR(i, samples.size())
            {
                a += samples[i][d1]*samples[i][d2];
            }
            double rho = n*a - sums[d1]*sums[d2];
            rho /= sqrt(n*sqsum[d1] - sums[d1]*sums[d1])*sqrt(n*sqsum[d2] - sums[d2]*sums[d2]);
            corr[dim*d1 + d2] = rho;
            corr[dim*d2 + d1] = rho;
        }
    }

    QPixmap pixmap(mapW, mapH);
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);

    // we draw the underlying grid
    QColor gridLines = Qt::black;
    QColor gridBackground = Qt::white;
    switch(flavorType)
    {
    case 0:
        gridBackground = QColor(128,128,128);
        gridLines = Qt::white;
        break;
    case 1:
        break;
    case 2:
        break;
    }
    if(flavorType != 1)
    {
        painter.setBrush(gridBackground);
        painter.setPen(QPen(Qt::black,2));
        painter.drawRect(pad, pad, w*dim, h*dim);
        painter.setPen(QPen(gridLines, 1));
        painter.setOpacity(0.5f);
        painter.setBrush(Qt::NoBrush);
        FOR(d, dim+1)
        {
            painter.drawLine(pad + d*w, pad, pad + d*w, mapH-pad);
            painter.drawLine(pad, pad + d*h, mapW-pad, pad + d*h);
        }
    }

    // we write the dimension names on the diagonal
    QFont font = painter.font();
    font.setPointSize(10);
    painter.setFont(font);
    QTextOption o(Qt::AlignCenter);
    o.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    painter.setPen(QPen(gridLines, 1));
    painter.setOpacity(0.5f);

    int optimalFontSize = font.pointSize();
    // we look for the longest dimension name
    QRectF r(0,0,w,h);
    QString longestName;
    QStringList names;
    FOR(d, dim)
    {
        QString dimName;
        if(d < canvas->dimNames.size()) dimName = canvas->dimNames[d];
        else
        {
            if(w > 50) dimName = QString("Dimension %1").arg(d+1);
            else dimName = QString("Dim %1").arg(d+1);
        }
        if(longestName.length() < dimName.length()) longestName = dimName;
        names << dimName;
    }
    int flags = Qt::TextDontClip|Qt::TextWrapAnywhere;
    adaptFontSize(&painter, flags, r, longestName);
    FOR(d, dim)
    {
        painter.drawText(QRectF(pad+d*w, pad+d*h, w, h), names.at(d), o);
    }

    painter.setRenderHint(QPainter::Antialiasing);

    optimalFontSize = font.pointSize();
    if(flavorType == 2)
    {
        flags = Qt::TextDontClip|Qt::TextWordWrap;
        QString t = QString("0.000");
        font.setPointSize(50);
        painter.setFont(font);
        adaptFontSize(&painter, flags, r, t);
        font = painter.font();
        optimalFontSize = font.pointSize();
    }
    // and now we draw the correlations themselves
    painter.setOpacity(1.f);
    painter.setPen(QPen(Qt::black, 1));
    FOR(d1, dim)
    {
        FOR(d2, dim)
        {
            if(d1 == d2) continue;
            double rho = corr[dim*d1 + d2];
            rho = rho > 0 ? min(1., rho) : max(-1., rho);
            QPointF center(pad + d1*w + w/2, pad + d2*h + h/2);
            switch(flavorType)
            {
            case 0: // circles
            {
                QColor color = rho > 0 ? QColor((1-rho)*255, (1-rho)*255, 255) : QColor(255, (1+rho)*255, (1+rho)*255);
                float radius = w*0.49*fabs(rho);
                painter.setBrush(color);
                painter.drawEllipse(center, radius, radius);
            }
                break;
            case 1: // ellipses
            {
                QColor color = rho > 0 ? QColor((1-rho)*255, (1-rho)*255, 255) : QColor(255, (1+rho)*255, (1+rho)*255);
                float radius = w*0.49*(1-fabs(rho));
                painter.setBrush(color);
                painter.translate(center);
                painter.rotate(rho > 0 ? -45 : 45);
                painter.drawEllipse(QPointF(0,0), w*0.49, radius);
                painter.rotate(rho > 0 ? 45 : -45);
                painter.translate(-center);
            }
                break;
            case 2: // graphs
            {
                QRectF r(center.x()-w/2,center.y()-h/2,w,h);
                if (d1 > d2) // we write down the value
                {
                    painter.setPen(Qt::black);
                    QString text = QString("%1").arg(rho, 0, 'f', 3);
                    font.setPointSize(max(5.,optimalFontSize*fabs(rho)));
                    painter.setFont(font);
                    painter.drawText(r, text, o);
                    font.setPointSize(10);
                    /*
                    double t = rho / sqrt((1. - rho*rho)/(n-2));
                    // now we should compute the p value (area under the curve for the student t distribution)
                    painter.setFont(font);
                    if (t < 0.001) painter.drawText(r.x(), r.y(), r.width(), r.height(), Qt::AlignRight|Qt::AlignTop, "***");
                    else if(t<0.01)painter.drawText(r.x(), r.y(), r.width(), r.height(), Qt::AlignRight|Qt::AlignTop, "**");
                    else if(t<0.05)painter.drawText(r.x(), r.y(), r.width(), r.height(), Qt::AlignRight|Qt::AlignTop, "*");
                    else painter.drawText(r.x(), r.y(), r.width(), r.height(), Qt::AlignRight|Qt::AlignTop, QString("%1").arg(t,0,'f',3));
                    */
                }
                else // we draw the correlation
                {
                    painter.setBrush(Qt::NoBrush);
                    painter.setPen(Qt::black);
                    painter.setOpacity(0.5);
                    int rad = 3;
                    FOR(i, samples.size())
                    {
                        float x = (samples[i][d1]-mins[d1])/(maxes[d1]-mins[d1])*(w-2*(rad+1)) + center.x() - w/2 + (rad+1);
                        float y = (1.f-(samples[i][d2]-mins[d2])/(maxes[d2]-mins[d2]))*(h-2*(rad+1)) + center.y() - h/2 + (rad+1);
                        painter.drawEllipse(QPointF(x,y), rad, rad);
                    }
                    painter.setOpacity(1);
                    painter.setPen(QPen(Qt::red,2));
                    painter.drawLine(center-QPointF(w*0.3, -h*0.3*rho), center+QPointF(w*0.3, -h*0.3*rho));
                }
            }
                break;
            }
        }
    }

    displayPixmap = pixmap;
    ui->display->setMinimumSize(displayPixmap.size());
    ui->display->setMaximumSize(displayPixmap.size());
}

void Visualization::GenerateSampleDistancePlot()
{
    std::vector<fvec> samples = data->GetSamples();
    ivec labels = data->GetLabels();
    if(!samples.size()) return;
    int dim = samples[0].size();
    if(!dim) return;
    int count = samples.size();

    // we need to make a big fat matrix with all the sample distances
    float *D = new float[count*count];
    float maxD = 0;
    FOR(i, count)
    {
        D[i*count + i] = 0;
        FOR(j, i)
        {
            float distance = 0;
            FOR(d, dim)
            {
                float dist = samples[i][d]-samples[j][d];
                distance += dist*dist;
            }
            distance = sqrtf(distance);
            maxD = max(maxD, distance);
            D[i*count + j] = distance;
            D[j*count + i] = distance;
        }
    }

    int pad = 30;
    int mapW = (ui->scrollArea->width()-12) - pad*2, mapH = (ui->scrollArea->height()-12) - pad*2;

    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QPixmap pixmap = QPixmap(ui->scrollArea->width()-2, ui->scrollArea->height()-2);
    pixmap.fill(Qt::transparent);

    QImage image(count,count, QImage::Format_RGB32);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    QFont font = painter.font();
    font.setPointSize(9);
    painter.setFont(font);

    FOR(i, count)
    {
        FOR(j, count)
        {
            float dist = D[i*count + j];
            image.setPixel(i,j,qRgb(dist/maxD*255,dist/maxD*255,dist/maxD*255));
        }
    }
    painter.drawPixmap(pad,pad,QPixmap::fromImage(image).scaled(QSize(mapW, mapH), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

    delete [] D;
    displayPixmap = pixmap;
}

void Visualization::GenerateDensityPlot()
{
    std::vector<fvec> samples = data->GetSamples();
    ivec labels = data->GetLabels();
    if(!samples.size()) return;
    int dim = samples[0].size();
    if(!dim) return;
    int flavorType = ui->flavorCombo->currentIndex();
    int inputType = ui->inputCombo->currentIndex();
    int classCount = data->GetClassCount(labels);
    int count = inputType ? dim : classCount;
    int D = min(dim, ui->x1Combo->currentIndex());

    fvec mins(dim, FLT_MAX), maxes(dim, -FLT_MIN);
    FOR(d, dim)
    {
        FOR(i, samples.size())
        {
            mins[d] = min(mins[d], samples[i][d]);
            maxes[d] = max(maxes[d], samples[i][d]);
        }
    }

    int pad = 30;
    int mapW = (ui->scrollArea->width()-12) - pad*2, mapH = (ui->scrollArea->height()-12) - pad*2;

    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QPixmap pixmap = QPixmap(ui->scrollArea->width()-2, ui->scrollArea->height()-2);
    pixmap.fill(Qt::white);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    map<int,int> classMap;
    int cnt=0;
    FOR(i, labels.size()) if(!classMap.count(labels[i])) classMap[labels[i]] = cnt++;

    vector<fvec> densities(count);
    int densityBins = inputType ? 256 : 128;
    fvec maxDensities(count, -FLT_MAX);
    fvec sumDensities(densityBins,0);
    if(inputType)
    {
        FOR(d, count)
        {
            float mean=0, sigma=0;
            fvec data(samples.size());
            FOR(i, samples.size())
            {
                data[i] = samples[i][d];
                mean += data[i];
            }
            mean /= data.size();
            FOR(i, data.size()) sigma += (data[i]-mean)*(data[i]-mean);
            sigma /= data.size();
            sigma = sqrtf(sigma);
            densities[d] = KernelDensity(data, sigma, mins[d], maxes[d], densityBins);
            FOR(i, densityBins)
            {
                if(maxDensities[d] < densities[d][i]) maxDensities[d] = densities[d][i];
                sumDensities[i] += densities[d][i];
            }
        }
    }
    else
    {
        int c=0;
        FORIT(classMap, int, int)
        {
            float mean=0, sigma=0;
            int meanCount=0;
            int label = it->first;
            fvec data;
            FOR(i, samples.size())
            {
                if(labels[i] != label) continue;
                data.push_back(samples[i][D]);
                mean += samples[i][D];
                meanCount++;
            }
            mean /= meanCount;
            FOR(i, data.size()) sigma += (data[i]-mean)*(data[i]-mean);
            sigma /= data.size();
            sigma = sqrtf(sigma);
            densities[c] = KernelDensity(data, sigma, mins[D], maxes[D], densityBins);
            FOR(i, densityBins)
            {
                if(maxDensities[c] < densities[c][i]) maxDensities[c] = densities[c][i];
                sumDensities[i] += densities[c][i];
            }
            c++;
        }
    }
    float maxDensity = -FLT_MAX;
    FOR(i, densityBins) if(maxDensity < sumDensities[i]) maxDensity = sumDensities[i];

    fvec baseline(densityBins,0);
    fvec streamCenter(densityBins, 0);
    fvec perm(count);

    // streamgraph
    if(flavorType == 2)
    {
        // first we can sort the dimensions so that the ones with the least variability
        // lay in the center of the stream graph. To do so we simply compute the variance
        // on the density
        fvec means(count,0);
        FOR(i, densityBins)
        {
            FOR(d, count) means[d] += densities[d][i];
        }
        means /= densityBins;
        fvec sigmas(count,0);
        FOR(i, densityBins)
        {
            FOR(d, count) sigmas[d] += (densities[d][i]-means[d])*(densities[d][i]-means[d]);
        }
        sigmas /= densityBins;
        FOR(d, count) sigmas[d] = sqrt(sigmas[d]);

        // now we can sort them
        vector< pair<float,int> > sigmaInd;
        FOR(d, count) sigmaInd.push_back(make_pair(sigmas[d],d));
        std::sort(sigmaInd.begin(), sigmaInd.end()); // sort smallest to largest

        // we want to fill in the permutation as a flip-flop
        // (small ones in the middle, large ones out)
        // TODO actually do it!
        //FOR(d, dim) perm[d] = sigmaInd[d].second;
        int top=(count-1)/2, bottom=(count-1)/2+1;
        FOR(d, count)
        {
            if(d%2==0 && top>=0) perm[top--] = sigmaInd[d].second;
            else if(bottom<count) perm[bottom++] = sigmaInd[d].second;
        }

        // and redistribute the densities
        vector<fvec> newDensities(count);
        FOR(d, count) newDensities[d] = densities[perm[d]];
        densities = newDensities;

        // Set shape of baseline values.
        FOR(i, densityBins)
        {
            // the 'center' is a rolling point. It is initialized as the previous
            // iteration's center value
            streamCenter[i] = i == 0 ? 0 : streamCenter[i-1];

            // find the total size of all layers at this point
            float totalSize = sumDensities[i];

            // account for the change of every layer to offset the center point
            FOR(d, count)
            {
                float increase;
                float moveUp;
                if (i == 0) {
                    increase = densities[d][i];
                    moveUp = 0.5f;
                } else {
                    float belowSize = 0.5f * densities[d][i];
                    for (int k = d + 1; k < count; k++) {
                        belowSize += densities[k][i];
                    }
                    increase = densities[d][i] - densities[d][i-1];
                    moveUp = totalSize == 0 ? 0 : (belowSize / totalSize);
                }
                streamCenter[i] += (moveUp - 0.5) * increase;
            }

            // set baseline to the bottom edge according to the center line
            //baseline[i] = streamCenter[i] - 0.5f * totalSize;
        }
    }

    baseline = fvec(densityBins,0);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    QPointF origin(pad, mapH+pad);

    // we recenter the graph so that it will pass through the center at its highest density
    if(flavorType==2)
    {
        int maxInd = 0;
        float maxVal = -FLT_MAX;
        FOR(i, densityBins)
        {
            if(maxVal < sumDensities[i])
            {
                maxVal = sumDensities[i];
                maxInd = i;
            }
        }
        float center = streamCenter[maxInd];
        origin.setY(pad + mapH/2 + center/maxDensity*mapH);
        maxDensity *= 1.1;
    }

    // we draw the area under the curve(s)
    vector < vector<QPointF> > pathTops, pathBottoms;
    FOR(d, count)
    {
        vector<QPointF> pathTop, pathBottom;
        QPointF old;
        float oldCum;
        FOR(i, densityBins)
        {
            float x = i / (float)(densityBins-1) * mapW;
            float y;
            switch(flavorType)
            {
            case 0:
                painter.setOpacity(0.5);
                y = densities[d][i] / maxDensities[d] * mapH;
                break;
            case 1:
                y = baseline[i] + (densities[d][i] / maxDensity) * mapH;
                break;
            case 2:
                y = (baseline[i] + densities[d][i]+streamCenter[i]-0.5*sumDensities[i])/maxDensity*mapH;
                break;
            }

            QPointF p(x,-y);
            pathTop.push_back(p);
            if(i)
            {
                if(flavorType == 2)
                {
                    int y1 = - (baseline[i-1] + streamCenter[i-1]-0.5*sumDensities[i-1])/maxDensity*mapH;
                    int y2 = - (baseline[i] + streamCenter[i]-0.5*sumDensities[i])/maxDensity*mapH;
                    if(i==1) pathBottom.push_back(QPointF(old.x(), y1));
                    pathBottom.push_back(QPointF(p.x(), y2));
                }
                else
                {
                    if(i==1) pathBottom.push_back(QPointF(old.x(), -oldCum));
                    pathBottom.push_back(QPointF(p.x(),-baseline[i]));
                }
                if(flavorType == 0)
                {
                    painter.setPen(Qt::black);
                    painter.setOpacity(1);
                    painter.drawLine(origin + old, origin + p);
                    painter.setPen(Qt::NoPen);
                    painter.setOpacity(0.5);
                }
            }
            old = p;
            oldCum = baseline[i];
            if(flavorType==1) baseline[i] = y;
        }
        if(flavorType)
        {
            FOR(i, densityBins)
            {
                baseline[i] += densities[d][i];
            }
        }
        pathTops.push_back(pathTop);
        pathBottoms.push_back(pathBottom);
    }

    if(flavorType==2 && inputType)
    {
        float maxVal = -FLT_MAX;
        float minVal = FLT_MAX;
        FOR(i, pathTops.size())
        {
            FOR(j, pathTops[i].size())
            {
                if(minVal > pathTops[i][j].y()) minVal = pathTops[i][j].y();
                if(maxVal < pathBottoms[i][j].y()) maxVal = pathBottoms[i][j].y();
            }
        }
        FOR(i, pathTops.size())
        {
            FOR(j, pathTops[i].size())
            {
                pathTops[i][j].setY((pathTops[i][j].y()-minVal)/(maxVal-minVal)*mapH);
                pathBottoms[i][j].setY((pathBottoms[i][j].y()-minVal)/(maxVal-minVal)*mapH);
            }
        }
        origin.setY(pad);
    }

    FOR(d, count)
    {
        if(flavorType == 0 && maxDensities[d] == 0) continue;
        QColor color = ui->grayscaleCheck->isChecked() ?
                    QColor(255*d/count,255*d/count,255*d/count) :
                    (d ? (inputType ? DimColor[d%DimColorCnt] : SampleColor[d%SampleColorCnt]) : QColor(220,220,220));
        if(flavorType == 2)
        {
            int dp = perm[d];
            color = ui->grayscaleCheck->isChecked() ?
                        QColor(255*dp/count,255*dp/count,255*dp/count) :
                        (dp ? (inputType ? DimColor[dp%DimColorCnt] : SampleColor[dp%SampleColorCnt]) : QColor(220,220,220));
        }
        painter.setBrush(color);

        // we generate the path itself
        QPainterPath surface(origin + pathTops[d].front());
        FOR(i, pathTops[d].size()-1) surface.lineTo(origin + pathTops[d][i+1]);
        FOR(i, pathBottoms[d].size()) surface.lineTo(origin + pathBottoms[d][pathBottoms[d].size()-1-i]);
        if(flavorType==0) painter.setOpacity(0.6);
        painter.drawPath(surface);
        if(flavorType==0)
        {
            QPainterPath path(origin + pathTops[d].front());
            FOR(i, pathTops[d].size()-1) path.lineTo(origin + pathTops[d][i+1]);
            // and then the graph lines
            painter.setOpacity(1);
            painter.setPen(QPen(Qt::black, 1));
            painter.setBrush(Qt::NoBrush);
            painter.drawPath(path);
        }
    }
    if(flavorType > 0)
    {
        FOR(d, count)
        {
            QPainterPath path(origin + pathTops[d].front());
            FOR(i, pathTops[d].size()-1) path.lineTo(origin + pathTops[d][i+1]);
            // and then the graph lines
            painter.setOpacity(1);
            painter.setPen(QPen(Qt::black, 1));
            painter.setBrush(Qt::NoBrush);
            painter.drawPath(path);
            if(flavorType==2 && !d)
            {
                QPainterPath bottom(origin + pathBottoms[d].front());
                FOR(i, pathBottoms[d].size()-1) bottom.lineTo(origin + pathBottoms[d][i+1]);
                painter.drawPath(bottom);
            }
        }
    }

    // we draw the axes
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setPen(QPen(Qt::black, 2));
    painter.drawLine(pad, mapH+pad, mapW+pad-1, mapH+pad);
    painter.drawLine(pad, pad, pad, mapH+pad);

    QFont font = painter.font();
    font.setPointSize(16);
    painter.setFont(font);
    painter.drawText(pad, mapH+pad, mapW, pad, Qt::AlignCenter, "Normalized Coordinate Range");
    painter.save();
    painter.rotate(-90);
    switch(flavorType)
    {
    case 0:
        painter.drawText(-mapH-pad, 0, mapH, pad, Qt::AlignCenter, "Normalized Density");
        break;
    case 1:
        painter.drawText(-mapH-pad, 0, mapH, pad, Qt::AlignCenter, "Cumulative Density");
        break;
    case 2:
        painter.drawText(-mapH-pad, 0, mapH, pad, Qt::AlignCenter, "Density");
        break;
    }
    painter.restore();
    font.setPointSize(12);
    painter.setFont(font);
    painter.drawText(pad/2, mapH+pad+2, pad, pad, Qt::AlignHCenter|Qt::AlignTop, QString("0"));
    painter.drawText(pad+mapW-pad/2, mapH+pad+2, pad, pad, Qt::AlignHCenter|Qt::AlignTop, QString("1"));

    // we draw the legend
    int maxLength = 0;
    int maxCount = 0;
    FOR(i, count)
    {
        QString s = inputType ? (canvas->dimNames.size() > i ? canvas->dimNames.at(i) : QString("Dimension %1").arg(i+1)) : canvas->GetClassString(i);
        int y = pad + 5 + i*20;
        if(y > pad + mapH - 10) break;
        int length = painter.fontMetrics().width(s);
        maxLength = max(maxLength, length);
        maxCount++;
    }
    if(maxCount < count) maxCount--;
    int legendW = 10 + 17 + maxLength;
    int legendH = min(10 + count*20, mapH - 10);
    painter.setBrush(Qt::white);
    painter.setOpacity(0.5);
    painter.setPen(Qt::NoPen);
    painter.drawRect(pad + mapW - legendW, pad, legendW, legendH);
    painter.setBrush(Qt::NoBrush);
    painter.setOpacity(1);
    painter.setPen(Qt::black);
    painter.drawRect(pad + mapW - legendW, pad, legendW, legendH);
    painter.setRenderHints(QPainter::Antialiasing);
    FOR(i, count)
    {
        QString s = inputType ? (canvas->dimNames.size() > i ? canvas->dimNames.at(i) : QString("Dimension %1").arg(i+1)) : canvas->GetClassString(i);
        int y = pad + 5 + i*20;
        if(y > pad + mapH - 10) break;
        if(i == maxCount) s = "  ...";
        painter.drawText(pad + mapW - legendW + 22, y, legendW-20, 20, Qt::AlignLeft|Qt::AlignVCenter, s);
        QColor color = inputType ? DimColor[i%DimColorCnt] : SampleColor[i%SampleColorCnt];
        painter.setBrush(color);
        painter.drawEllipse(QPoint(pad + mapW - legendW + 5 + 7, y + 10), 6, 6);
    }

    displayPixmap = pixmap;
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

QPixmap Visualization::GetGraphPixmap(int type, int inputType, int dim, int classCount,
                                      int index, int w, int h,
                                      std::map<int,std::vector< std::pair<fvec,fvec> > > classGraphData,
                                      std::vector< std::map<int,fvec> > dimClassData,
                                      fvec mins, std::map<int,fvec> dimMins,
                                      fvec maxes, std::map<int,fvec> dimMaxes)
{
    if(type == 6) return GetStarPixmap(inputType, dim, classCount, index, w, h, mins, dimMins, maxes, dimMaxes);
    else if(type == 7) return GetRadialPixmap(classGraphData, inputType, dim, classCount, index, w, h, mins, maxes);

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
    //QBitmap bitmap(w*perItemCount + pad*2, h+pad*1.5);
    //bitmap.clear();
    //pixmap.setMask(bitmap);
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
    bool bCategorical = data->IsCategorical(d);
    int axisSteps = bCategorical ? data->categorical[d].size()-1 : 10;
    int axisH = h / (axisSteps+1);
    FOR(i, axisSteps+1)
    {
        float value = (maxesText - minsText)*i/(axisSteps) + minsText;
        int y = h - ((maxesPoint-minsPoint)*i/(axisSteps) + minsPoint);
        if(bCategorical) strcpy(text, data->GetCategorical(d, i).c_str());
        else sprintf(text, format, value);

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
        else
        {
            color = inputType ? DimColor[b%DimColorCnt] : SampleColor[b%SampleColorCnt];
        }

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
            float sumDensity = 0;
            maxDensity = -FLT_MAX;
            FOR(i, density.size())
            {
                if(maxDensity < density[i]) maxDensity = density[i];
                sumDensity += density[i];
            }

            painter.setRenderHint(QPainter::Antialiasing);
            painter.setPen(color);
            painter.setBrush(color);
            float oldX = 0, oldY = 0;
            float v = 0;
            int quartile = 0;
            FOR(i, density.size())
            {
                float y = ((i+1)/(float)(density.size()+1))*(topPoint-bottomPoint) + bottomPoint;
                if(quartile < 2) v += density[i]/sumDensity;
                else v -= density[i]/sumDensity;
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
                if(quartile < 2) v += density[i]/sumDensity;
                else v -= density[i]/sumDensity;
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
    //QBitmap bitmap(w, h+pad*1.5);
    //bitmap.clear();
    //pixmap.setMask(bitmap);
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
        if(minv == maxv)
        {
            minv = 0;
            maxv = 0.1;
        }

        // we now draw the arcs in 16th of degree
        float angleStart = (360*16)*d/dMins.size();
        float angleLen = (360*16)/dMins.size();
        float maxR = maxRadius * maxv;
        float minR = maxRadius * minv;
        QColor color = inputType ? DimColor[d%DimColorCnt] : SampleColor[d%SampleColorCnt];
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

QPixmap Visualization::GetRadialPixmap(std::map<int,std::vector< std::pair<fvec,fvec> > > classGraphData, int inputType, int dim, int classCount, int index, int w, int h, fvec mins, fvec maxes)
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
    for(map<int,vector<pair<fvec,fvec> > >::iterator it = classGraphData.begin(); it != classGraphData.end(); it++)
    {
        if(c==cCnt)
        {
            classId = it->first;
            break;
        }
        c++;
    }

    QPixmap pixmap(w, h+pad*1.5);
    //QBitmap bitmap(w, h+pad*1.5);
    //bitmap.clear();
    //pixmap.setMask(bitmap);
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

    vector<fvec> densities;
    switch(inputType)
    {
    case 0: // dimension
    {
        int cCnt = 0;
        for(map<int,vector<pair<fvec,fvec> > >::iterator it = classGraphData.begin(); it != classGraphData.end(); it++, cCnt++)
        {
            fvec density = it->second[d].second;
            densities.push_back(density);
        }
    }
        break;
    case 1: // classCount
    {
        FOR(d, dim)
        {
            fvec density = classGraphData[classId][d].second;
            densities.push_back(density);
        }
    }
        break;
    }
    int bins = densities[0].size();

    fvec sums(bins,0);
    FOR(i, densities.size())
    {
        FOR(j, densities[i].size())
        {
            sums[j] += densities[i][j];
        }
    }
    float maxSum = 0;
    float centerRadius = 3.f;
    FOR(i, sums.size())
    {
        maxSum = max(maxSum, sums[i]);
    }
    maxSum += centerRadius;

    // we recompute the center
    int maxRadius = min(w,h)*0.5;
    float angleLen = 2*M_PI/(bins);
    QPointF center(0,0);
    //QPointF center(w/2, h/2 + pad/2);
    float minx=FLT_MAX, miny=FLT_MAX, maxx=-FLT_MAX, maxy=-FLT_MAX;
    FOR(i, bins)
    {
        float angleStart = i*angleLen;
        float radius = (sums[i]+centerRadius)/maxSum*maxRadius;
        float x = radius*cosf(angleStart);
        float y = radius*sinf(angleStart);
        minx = min(minx, x);
        maxx = max(maxx, x);
        miny = min(miny, y);
        maxy = max(maxy, y);
    }
    center = QPointF(w/2, h/2 + pad/2) - QPointF((maxx+minx)/2, (maxy+miny)/2);
    float diffx = maxx-minx;
    float diffy = maxy-miny;
    float diff = max(diffx,diffy);
    float radDif = min(w,h) * 0.9 / diff;
    maxRadius *= radDif;
    //qDebug() << "diff" << (diff/min(w,h)) << "->" << radDif;

    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(Qt::black, 0.5));
    fvec pads(bins,centerRadius);
    FOR(i, densities.size())
    {
        QColor color = inputType ? DimColor[i%DimColorCnt] : SampleColor[i%SampleColorCnt];
        //if(i==0) color = Qt::lightGray;
        float oldRadius = 0;
        float oldSmallRadius = 0;
        painter.setBrush(color);
        FOR(j, bins+1)
        {
            float angleStart = (j%bins)*angleLen;
            float smallRadius = pads[j%bins]/maxSum*maxRadius;
            float radius = (pads[j%bins] + densities[i][j%bins])/maxSum*maxRadius;
            if(j)
            {
                QPolygonF p;
                p << center+QPointF(radius*cosf(angleStart),radius*sinf(angleStart));
                p << center+QPointF(oldRadius*cosf(angleStart-angleLen),oldRadius*sinf(angleStart-angleLen));
                p << center+QPointF(oldSmallRadius*cosf(angleStart-angleLen),oldSmallRadius*sinf(angleStart-angleLen));
                p << center+QPointF(smallRadius*cosf(angleStart),smallRadius*sinf(angleStart));
                painter.drawPolygon(p);
            }
            oldRadius = radius;
            oldSmallRadius = smallRadius;
        }
        pads += densities[i];
    }

    pads = fvec(bins,centerRadius);
    painter.setPen(QPen(Qt::black, 1));
    FOR(i, densities.size())
    {
        float oldRadius = 0;
        float oldSmallRadius = 0;
        FOR(j, bins+1)
        {
            float angleStart = (j%bins)*angleLen;
            float smallRadius = pads[j%bins]/maxSum*maxRadius;
            if(i==0)
            {
                if(j)
                {
                    painter.drawLine(center+QPointF(smallRadius*cosf(angleStart),smallRadius*sinf(angleStart)),
                                     center+QPointF(oldSmallRadius*cosf(angleStart-angleLen),oldSmallRadius*sinf(angleStart-angleLen)));
                }
            }
            float radius = (pads[j%bins] + densities[i][j%bins])/maxSum*maxRadius;
            if(j)
            {
                painter.drawLine(center+QPointF(radius*cosf(angleStart),radius*sinf(angleStart)),
                                 center+QPointF(oldRadius*cosf(angleStart-angleLen),oldRadius*sinf(angleStart-angleLen)));
            }
            oldRadius = radius;
            oldSmallRadius = smallRadius;
        }
        pads += densities[i];
    }

    font.setPointSize(9);
    painter.setFont(font);
    switch(inputType)
    {
    case 0: // dimension
    {
        float minv = mins[d];
        float maxv = maxes[d];
        float rad = centerRadius/maxSum*maxRadius;
        QString text;
        text = QString("%1").arg(minv, 0, 'f', 2);
        painter.drawText(center.x(), center.y()-10, rad-4, 10, Qt::AlignRight|Qt::AlignVCenter, text);
        text = QString("%1").arg((maxv-minv)*1./4. + minv, 0, 'f', 2);
        painter.drawText(center.x()-40, center.y()-rad+4, 80, 10, Qt::AlignHCenter|Qt::AlignTop, text);
        text = QString("%1").arg((maxv-minv)*2./4. + minv, 0, 'f', 2);
        painter.drawText(center.x()-rad+4, center.y()-5, rad-4, 10, Qt::AlignLeft|Qt::AlignVCenter, text);
        text = QString("%1").arg((maxv-minv)*3./4. + minv, 0, 'f', 2);
        painter.drawText(center.x()-40, center.y()+rad-14, 80, 10, Qt::AlignHCenter|Qt::AlignBottom, text);
        text = QString("%1").arg(maxv, 0, 'f', 2);
        painter.drawText(center.x(), center.y(), rad-4, 10, Qt::AlignRight|Qt::AlignVCenter, text);
    }
        break;
    case 1: // class
        break;
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

fvec Visualization::KernelDensity(fvec data, float sigma, float minv, float maxv, int bins)
{
    fvec density(bins,0);
    if(!data.size() || sigma==0 || sigma != sigma) return density;
    const int count = data.size();
    double scale = 1 / sqrt(2*M_PI);
    double h = sigma*pow(4./3.,0.2)*pow(count,-0.2);
    FOR(i, bins)
    {
        double iV = double(i)/bins*(maxv-minv) + minv;
        double x = 0;
        FOR(j, count)
        {
            double a = (data[j]-iV) / h;
            double k = exp(-0.5*a*a)*scale;
            x += k;
        }
        density[i] = x / (count*h);
    }
    return density;
}
