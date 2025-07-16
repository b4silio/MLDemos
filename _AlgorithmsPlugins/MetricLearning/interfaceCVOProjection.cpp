#include <QTableWidget>
#include <iostream>

#include "interfaceCVOProjection.h"
#include "projectorCVO.h"


using namespace std;

CVOProjection::CVOProjection()
    : widget(new QWidget())
    , table(0)
{
    params = new Ui::paramsCVO();
    params->setupUi(widget);
    connect( params->othersCheckBox, SIGNAL(clicked(bool)), this, SLOT(onOthersChkBoxClicked(bool)) );
    connect( params->showMatrixBtn, SIGNAL(clicked()), this, SLOT(onShowMatrixBtnClicked()) );
    params->alphaLineEdit->setValidator( new QDoubleValidator(0.0000000001, 1000000000, 10));
    params->stepsLineEdit->setValidator( new QIntValidator(5, 1000000));
}

CVOProjection::~CVOProjection()
{
    if( table )
        table->deleteLater();
    delete params;
}

// virtual functions to manage the algorithm creation
Projector* CVOProjection::GetProjector()
{
    return new ProjectorCVO();
}

fvec CVOProjection::GetParams()
{
    return fvec();
}

void CVOProjection::SetParams( Projector* projector, fvec parameters )
{
}

void CVOProjection::GetParameterList( std::vector<QString>& parameterNames,
                             std::vector<QString>& parameterTypes,
                             std::vector< std::vector<QString> >& parameterValues )
{
}

void CVOProjection::DrawInfo( Canvas* canvas, QPainter& painter, Projector* projector )
{
    if(!canvas || !projector)
        return;

    if( !table )
        table = new QTableWidget;

    ProjectorCVO* cvo = dynamic_cast<ProjectorCVO*>(projector);
    if( !cvo )
        return;

    std::vector<fvec> res = cvo->matrixCoeff();
    if( res.empty() )
        return;

    table->clear();
    table->setRowCount(res.size());
    table->setColumnCount(res.at(0).size());

    for( size_t i = 0; i < table->rowCount(); ++i ) {
        for( size_t j = 0; j < table->columnCount(); ++j ) {
            table->setItem(i,j, new QTableWidgetItem(QString::number(res.at(i).at(j))));
        }
    }
    table->show();
}

void CVOProjection::DrawModel( Canvas* canvas, QPainter& painter, Projector* projector )
{
    if( !canvas || !projector )
        return;
    if( canvas->canvasType )
        return;

    const vector<fvec>& samples = projector->source;
    ivec labels = canvas->data->GetLabels();
    if( !samples.size() )
        return;
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(Qt::black, 0.5f));
    for( size_t i = 0; i < projector->projected.size(); ++i ) {
        painter.setBrush(SampleColor[labels[i]%SampleColorCnt]);
        QPointF p1 = canvas->toCanvasCoords(projector->projected[i]);
        painter.drawEllipse(p1, 5, 5);
    }
}

// virtual functions to manage the GUI and I/O
QString CVOProjection::GetAlgoString()
{
    return QString("CVO");
}

void CVOProjection::SetParams(Projector* projector )
{
    if( !projector ) return;
    ProjectorCVO* cvo = dynamic_cast<ProjectorCVO*>(projector);
    if( !cvo )
        return;

    cvo->setMethod(params->methodCombo->currentIndex());
    cvo->setClasses(params->simBox->value(), params->dissimBox->value(), params->othersCheckBox->isChecked());
    cvo->setNormalizeData(params->normalizeBox->isChecked());
    bool ok = false;
    float alpha = params->alphaLineEdit->text().toFloat(&ok);
    if( ok && alpha > 0.0 )
        cvo->setAlpha(alpha);
    int steps = params->stepsLineEdit->text().toInt(&ok);
    if( ok && steps > 0 )
        cvo->setSteps(steps);
}

void CVOProjection::SaveOptions( QSettings& settings )
{
    //settings.setValue("kernelCluster", params->kernelClusterSpin->value());
}

bool CVOProjection::LoadOptions( QSettings& settings )
{
    //if(settings.contains("kernelCluster")) params->kernelClusterSpin->setValue(settings.value("kernelCluster").toFloat());
    return true;
}

void CVOProjection::SaveParams( QTextStream& file )
{
    //file << "projectOptions" << ":" << "kernelCluster" << " " << params->kernelClusterSpin->value() << "\n";
}

bool CVOProjection::LoadParams(QString name, float value)
{
    //if(name.endsWith("kernelCluster")) params->kernelClusterSpin->setValue((int)value);
    return true;
}

void CVOProjection::onOthersChkBoxClicked( bool checked )
{
    params->dissimBox->setEnabled(!checked);
}

void CVOProjection::onShowMatrixBtnClicked()
{
    if( table ) {
        table->show();
    }
}
