#include "marginalwidget.h"
#include <QPainter>
#include <QDebug>

using namespace std;

void FindHumanReadableLimits(const float minv, const float maxv, const float minGridSteps,
                             float &gridMin, float &gridMax, float &gridStep)
{
    float diff = maxv-minv;
    float digits = (log(diff)/log(10));
    digits = digits > 0 ? (int)digits : (int)digits - 1;
    gridStep = powf(10.f,digits);
    while(int(diff / gridStep) < minGridSteps) gridStep /= 4.f;
    gridMin = (int)(minv / gridStep) * gridStep;
    gridMax = (int)(maxv / gridStep + 0.5f) * gridStep;
}

QWidget *MarginalWidget::widget=0;
Ui::MarginalWidget *MarginalWidget::ui=0;
bool MarginalWidget::bClassMarginal = false;
std::vector< std::vector<fvec> > MarginalWidget::classMarginals = std::vector< std::vector<fvec> >();
std::vector< std::vector< std::vector<fvec> > > MarginalWidget::classMarginalGmm = std::vector< std::vector< std::vector<fvec> > >();
std::vector<fvec> MarginalWidget::marginals = std::vector<fvec>();
std::vector< std::vector<fvec> > MarginalWidget::marginalGmm = std::vector< std::vector<fvec> >();
std::vector<fvec> MarginalWidget::limits = std::vector<fvec>();

MarginalWidget::MarginalWidget()
{
    if ( !ui ) {
        ui = new Ui::MarginalWidget();
        ui->setupUi(widget = new QWidget());
        ui->display->setScaledContents(true);
        connect(ui->dimCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(MarginalChanged()));
        widget->installEventFilter(this);
    }
}

MarginalWidget::~MarginalWidget()
{
    DEL(ui);
    DEL(widget);
}

bool MarginalWidget::eventFilter(QObject *obj, QEvent *event)
{
    if ( obj == widget && event->type() == QEvent::Resize ) {
        MarginalChanged();
        return true;
    } else {
        return QObject::eventFilter(obj, event);
    }
}

void MarginalWidget::SetClassMarginals (std::vector< std::vector<fvec> > classMarginals,
                                        std::vector< std::vector< std::vector<fvec> > > classMarginalGmm,
                                        std::vector<fvec> limits)
{
    this->classMarginals = classMarginals;
    this->classMarginalGmm = classMarginalGmm;
    this->limits = limits;
    bClassMarginal = true;
    MarginalChanged();
}

void MarginalWidget::SetMarginals (std::vector<fvec> marginals,
                                   std::vector< std::vector<fvec> > marginalGmm,
                                   std::vector<fvec> limits)
{
    this->marginals = marginals;
    this->marginalGmm = marginalGmm;
    this->limits = limits;
    bClassMarginal = false;
    MarginalChanged();
}

void MarginalWidget::MarginalChanged()
{
    DrawMarginals();
}


void MarginalWidget::DrawMarginals()
{
    int w = max(600,ui->display->width());
    int h = max(250,ui->display->height());
    int pad = 30;
    int index = ui->dimCombo->currentIndex();
    int steps;
    int nstates;
    int gmmCount = 1;
    if ( bClassMarginal ) {
    if(index >= classMarginals[0].size()) return;
        steps = classMarginals[0][index].size();
        gmmCount = classMarginalGmm.size();
        nstates = classMarginalGmm[0][index][0].size();
    } else {
        if ( index >= marginals.size() ) return;
        steps = marginals[index].size();
        nstates = marginalGmm[index][0].size();
    }
    float minx = limits[index][0];
    float maxx = limits[index][1];

    float minv = FLT_MAX;
    float maxv = -FLT_MAX;
    if ( bClassMarginal ) {
        FOR ( g, gmmCount ) {
            FOR ( i, steps ) {
                minv = min(minv, classMarginals[g][index][i]);
                maxv = max(maxv, classMarginals[g][index][i]);
            }
        }
    } else {
        FOR ( i, steps ) {
            minv = min(minv, marginals[index][i]);
            maxv = max(maxv, marginals[index][i]);
        }
    }

    QPixmap pixmap(w,h);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    FOR(g, gmmCount)
    {
        QPolygonF poly;
        painter.setPen(QPen(Qt::black,2));
        painter.setBrush(g ? SampleColor[g%SampleColorCnt] : Qt::lightGray);
        const fvec &values = bClassMarginal ? classMarginals[g][index] : marginals[index];
        FOR ( i, steps ) {
            float val = (values[i]-minv) / (maxv-minv);
            if(val != val) val = 0;
            QPointF point(i/(float)steps*(w - 2*pad) + pad, (1-val) * (h - 2*pad) + pad);
            poly << point;
        }
        FOR ( i, steps ) {
            poly << QPointF((steps-i-1)/float(steps)*(w-2*pad) + pad, h-pad);
        }
        painter.setOpacity(0.5);
        painter.drawPolygon(poly);
        painter.setOpacity(1);

        FOR ( j, nstates ) {
            QPainterPath path;
            const vector<fvec> &values = bClassMarginal ? classMarginalGmm[g][index] : marginalGmm[index];
            FOR ( i, steps ) {
                float val = (values[i][j]-minv) / (maxv-minv);
                QPointF point(i/(float)steps*(w - 2*pad) + pad, (1-val) * (h - 2*pad) + pad);
                if(!i) path.moveTo(point);
                else path.lineTo(point);
            }
            painter.setPen(QPen(Qt::black, 1, Qt::DashLine));
            painter.setBrush(Qt::NoBrush);
            painter.drawPath(path);
        }
    }

    painter.setPen(QPen(Qt::black, 2));
    QFont font = painter.font();
    font.setPointSize(12);
    painter.setFont(font);

    // we draw the horizontal axis
    painter.drawLine(pad, h - pad, w - pad, h - pad);
    if ( minx*maxx < 0 ) { // we have a zero
        float x = (0-minx)/(maxx-minx)*(w-2*pad);
        painter.drawLine(x + pad, h-pad, x + pad, h-pad+4);
        painter.drawText(x + pad - 30, h-pad+4, 60, pad-4, Qt::AlignHCenter|Qt::AlignTop, QString("0"));
    }

    // we need to find the largest human-readable unit
    float gridMin, gridMax, gridStep;
    FindHumanReadableLimits(minx, maxx, 3, gridMin, gridMax, gridStep);

    int labelWidth = int((w-2*pad)*gridStep/(maxx-minx));
    for (float v = gridMin; v <= gridMax; v += gridStep)
    {
        float x = (v-minx)/(maxx-minx)*(w-2*pad);
        if ( x < 0 || x >= w-2*pad || fabs(v) < gridStep*1e-3) continue;
        painter.drawLine(x + pad, h-pad, x + pad, h-pad+4);
        painter.drawText(x + pad - labelWidth/2, h-pad+4, labelWidth, pad-4, Qt::AlignHCenter|Qt::AlignTop, QString("%1").arg(v));
    }

    // we draw the vertical axis
    painter.drawLine(pad, pad, pad, h-pad);
    painter.drawLine(pad, h-pad, pad-4, h-pad);
    painter.drawText(0, h-pad - pad/2, pad-5, pad, Qt::AlignVCenter|Qt::AlignRight, QString("%1").arg(0));
    // we need to find the largest human-readable unit
    FindHumanReadableLimits(minv, maxv, 2, gridMin, gridMax, gridStep);
    for (float v = gridMin; v <= gridMax; v += gridStep)
    {
        float y = (1.f - (v-minv)/(maxv-minv))*(h-2*pad);
        if ( y < 0 || y >= h-2*pad) continue;
        painter.drawLine(pad, y+pad, pad-4, y+pad);
        painter.drawText(0, y + pad - pad/2, pad-5, pad, Qt::AlignVCenter|Qt::AlignRight, QString("%1").arg(v));
    }

    ui->display->setPixmap(pixmap);
    ui->display->repaint();
}

void MarginalWidget::SetDimensions(int dim, QStringList dimNames)
{
    // we fill in the marginal distributions for each dimension
    ui->dimCombo->blockSignals(true);
    int currentIndex = ui->dimCombo->currentIndex();
    ui->dimCombo->clear();
    FOR ( i, dim ) {
        if(i < dimNames.size()) ui->dimCombo->addItem(dimNames.at(i));
        else ui->dimCombo->addItem(QString("Dimension %1").arg(i+1));
    }
    if(ui->dimCombo->count() > currentIndex) ui->dimCombo->setCurrentIndex(currentIndex);
    ui->dimCombo->blockSignals(false);
}
