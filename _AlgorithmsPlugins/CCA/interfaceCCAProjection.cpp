#include "interfaceCCAProjection.h"
#include "projectorCCA.h"
#include <QDebug>

#include <iostream>

using namespace std;

CCAProjection::CCAProjection()
    : widget(new QWidget())
{
    params = new Ui::paramsCCA();
    params->setupUi(widget);
    table_root = NULL;
    table = NULL;
    table1 = table2 = NULL;
    connect(params->undockButton1, SIGNAL(clicked()), this, SLOT(Undock1()));
    connect(params->undockButton2, SIGNAL(clicked()), this, SLOT(Undock2()));
}

// virtual functions to manage the algorithm creation
Projector *CCAProjection::GetProjector()
{
    return new ProjectorCCA();
}

void CCAProjection::Undock1()
{
    if(!table_root) return;
    // we need to know if we're docked
    QObjectList children = params->tableWidget1->children();
    bool isChild = false;
    FOR(i, children.size())
    {
        if(table_root == children[i])
        {
            isChild = true;
            break;
        }
    }

    if(isChild) // we need to undock
    {
        DEL(table1);
        table1 = new QWidget();
        table1->setWindowTitle("CCA Results");
        table1->setLayout(new QHBoxLayout());
        table1->layout()->addWidget(table_root);
        table1->show();
    }
    else // we need to dock
    {
        params->tableWidget1->layout()->addWidget(table_root);
        DEL(table1);
    }
}

void CCAProjection::Undock2()
{
    if(!table) return;
    // we need to know if we're docked
    QObjectList children = params->tableWidget2->children();
    bool isChild = false;
    FOR(i, children.size())
    {
        if(table == children[i])
        {
            isChild = true;
            break;
        }
    }

    if(isChild) // we need to undock
    {
        DEL(table2);
        table2 = new QWidget();
        table2->setWindowTitle("Correlation P");
        table2->setLayout(new QHBoxLayout());
        table2->layout()->addWidget(table);
        table2->show();
    }
    else // we need to dock
    {
        params->tableWidget2->layout()->addWidget(table);
        DEL(table2);
    }
}

void CCAProjection::DrawInfo(Canvas *canvas, QPainter &painter, Projector *projector)
{
    if(!canvas || !projector) return;
    if(canvas->canvasType) return;
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
}

void CCAProjection::DrawModel(Canvas *canvas, QPainter &painter, Projector *projector)
{
    if(!canvas || !projector) return;
    if(canvas->canvasType) return;
    ProjectorCCA *cca = dynamic_cast<ProjectorCCA*>(projector);

    fvec p          = cca->getProbability();
    fvec chi_square = cca->getChiSquare();
    fvec wilks_lambda = cca->getWilksLambda();
    fvec latent_roots = cca->getLatentRoots();
    fvec canonical_roots = cca->getCanonicalRoots();

    if(!table_root)
    {
        table_root = new QTableWidget(params->tableWidget1);
        params->tableWidget1->layout()->addWidget(table_root);
    }
    else table_root->clear();
    table_root->setRowCount(canonical_roots.size());
    table_root->setColumnCount(5);
    table_root->setHorizontalHeaderLabels(QString("Canonical roots;Latent roots;wilks lambda;Chi square; prob").split(";"));

    FOR(i,canonical_roots.size())
    {
        table_root->setItem(i,0,new QTableWidgetItem(QString::number(canonical_roots[i])));
        table_root->setItem(i,1,new QTableWidgetItem(QString::number(latent_roots[i])));
        table_root->setItem(i,2,new QTableWidgetItem(QString::number(wilks_lambda[i])));
        table_root->setItem(i,3,new QTableWidgetItem(QString::number(chi_square[i])));
        table_root->setItem(i,4,new QTableWidgetItem(QString::number(p[i])));
        table_root->item(i,0)->setTextAlignment(Qt::AlignCenter);
        table_root->item(i,1)->setTextAlignment(Qt::AlignCenter);
        table_root->item(i,2)->setTextAlignment(Qt::AlignCenter);
        table_root->item(i,3)->setTextAlignment(Qt::AlignCenter);
        table_root->item(i,4)->setTextAlignment(Qt::AlignCenter);
    }
    table_root->show();

    std::vector<fvec> Sxy = cca->getSxy();
    projector->projected = Sxy;


    if(!table)
    {
        table = new QTableWidget(params->tableWidget2);
        params->tableWidget2->layout()->addWidget(table);
    }
    else table->clear();
    std::vector<fvec> clps = cca->getCLPS();

    table->setRowCount(clps.size());
    table->setColumnCount(clps[0].size());
    //table->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    QString h_header = "";
    FOR(i,clps.size())
    {
        if( i != clps.size()){
            h_header = h_header + QString::number(i+1) + ";";
        }else{
            h_header = h_header + QString::number(i+1);
        }
    }
    //Set Header Label Texts Here
    table->setHorizontalHeaderLabels(h_header.split(";"));
    table->setVerticalHeaderLabels(h_header.split(";"));

    double numd = 0;
    int numi = 0;
    FOR(i,clps.size())
    {
        FOR(j,clps[0].size())
        {
            numi = clps[i][j]*1000;
            numd = static_cast<double>(numi)/1000;
            table->setItem(i,j,new QTableWidgetItem(QString::number(numd)));
            table->item(i,j)->setTextAlignment(Qt::AlignCenter);
        }

    }

    table->show();

}

// virtual functions to manage the GUI and I/O
QString CCAProjection::GetAlgoString()
{
    return QString("CCA");
}

void CCAProjection::SetParams(Projector *projector)
{
    if(!projector) return;
    ((ProjectorCCA*) projector)->setSeperatingIndex(params->lineSeperatingIndexEdit->text().toInt());
}


fvec CCAProjection::GetParams()
{
    int separatingIndex = params->lineSeperatingIndexEdit->text().toInt();

    int i=0;
    fvec par(1);
    par[i++] = separatingIndex;
    return par;
}

void CCAProjection::SetParams(Projector *projector, fvec parameters)
{
    if(!projector) return;
    ProjectorCCA *cca = dynamic_cast<ProjectorCCA*>(projector);
    if(!cca) return;
    int i=0;
    int separatingIndex = parameters.size() > i ? parameters[i] : 0; i++;

    cca->setSeperatingIndex(separatingIndex);
}

void CCAProjection::GetParameterList(std::vector<QString> &parameterNames,
                                std::vector<QString> &parameterTypes,
                                std::vector< std::vector<QString> > &parameterValues)
{
    parameterNames.push_back("Separating Index");
    parameterTypes.push_back("Integer");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("1");
    parameterValues.back().push_back("9999999999");
}

void CCAProjection::SaveOptions(QSettings &settings)
{
    //settings.setValue("typeCombo", params->typeCombo->currentIndex());
}

bool CCAProjection::LoadOptions(QSettings &settings)
{
   // if(settings.contains("typeCombo")) params->typeCombo->setCurrentIndex(settings.value("typeCombo").toInt());
    return true;
}

void CCAProjection::SaveParams(QTextStream &file)
{
    //file << "clusterOptions" << ":" << "typeCombo" << " " << params->typeCombo->currentIndex() << "\n";
}

bool CCAProjection::LoadParams(QString name, float value)
{
   // if(name.endsWith("typeCombo")) params->typeCombo->setCurrentIndex((int)value);
    return true;
}


