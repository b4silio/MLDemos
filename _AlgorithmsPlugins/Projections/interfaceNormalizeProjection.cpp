#include "interfaceNormalizeProjection.h"
#include "projectorNormalize.h"
#include <QDebug>

using namespace std;

NormalizeProjection::NormalizeProjection()
    : widget(new QWidget())
{
    params = new Ui::paramsNormalize();
    params->setupUi(widget);
    connect(params->typeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeOptions()));
    connect(params->singleDimCheck, SIGNAL(clicked()), this, SLOT(ChangeOptions()));
    ChangeOptions();
}

NormalizeProjection::~NormalizeProjection()
{
    delete params;
}

void NormalizeProjection::ChangeOptions()
{
    params->rangeMaxLabel->setVisible(true);
    params->rangeMaxSpin->setVisible(true);
    switch(params->typeCombo->currentIndex())
    {
    case 0:
        params->rangeMinLabel->setText("Min");
        params->rangeMaxLabel->setText("Max");
        params->rangeMaxSpin->setRange(-99999998, 99999998);
        break;
    case 1:
        params->rangeMinLabel->setText("Mean");
        params->rangeMaxLabel->setText("Variance");
        params->rangeMaxSpin->setRange(0.00001, 99999998);
        break;
    case 2:
        params->rangeMaxLabel->setVisible(false);
        params->rangeMaxSpin->setVisible(false);
        params->rangeMinLabel->setText("Center");
        break;
    }
    params->dimensionSpin->setVisible(params->singleDimCheck->isChecked());
}

// virtual functions to manage the algorithm creation
Projector *NormalizeProjection::GetProjector()
{
    return new ProjectorNormalize();
}

void NormalizeProjection::SetParams(Projector *projector)
{
    ProjectorNormalize* pNormalize = dynamic_cast<ProjectorNormalize*>(projector);
    if(!pNormalize) return;
    int type = params->typeCombo->currentIndex();
    int dimension = params->singleDimCheck->isChecked() ? params->dimensionSpin->value() : -1;
    float rangeMin = params->rangeMinSpin->value();
    float rangeMax = params->rangeMaxSpin->value();
    pNormalize->SetParams(type, rangeMin, rangeMax, dimension);
}

fvec NormalizeProjection::GetParams()
{
    int type = params->typeCombo->currentIndex();
    int dimension = params->singleDimCheck->isChecked() ? params->dimensionSpin->value() : -1;
    float rangeMin = params->rangeMinSpin->value();
    float rangeMax = params->rangeMaxSpin->value();

    fvec par(4);
    par[0] = type;
    par[1] = dimension;
    par[2] = rangeMin;
    par[3] = rangeMax;
    return par;
}

void NormalizeProjection::GetParameterList(std::vector<QString> &parameterNames,
                             std::vector<QString> &parameterTypes,
                             std::vector< std::vector<QString> > &parameterValues)
{
    parameterNames.push_back("Normalization Type");
    parameterNames.push_back("Range Min");
    parameterNames.push_back("Range Max");
    parameterNames.push_back("Dimension");
    parameterTypes.push_back("List");
    parameterTypes.push_back("Real");
    parameterTypes.push_back("Real");
    parameterTypes.push_back("Integer");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("Range");
    parameterValues.back().push_back("Variance");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("-9999999");
    parameterValues.back().push_back("9999999");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("-9999999");
    parameterValues.back().push_back("9999999");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("-1");
    parameterValues.back().push_back("999");
}

void NormalizeProjection::SetParams(Projector *projector, fvec parameters)
{
    if(!projector) return;
    int type = parameters.size() > 0 ? parameters[0] : 0;
    int dimension = parameters.size() > 1 ? parameters[1] : 0.1;
    float rangeMin = parameters.size() > 2 ? parameters[2] : 1;
    float rangeMax = parameters.size() > 3 ? parameters[3] : 1;

    ProjectorNormalize* pNormalize = dynamic_cast<ProjectorNormalize*>(projector);
    if(!pNormalize) return;
    // we add 1 to the kernel type because we have taken out the linear kernel
    pNormalize->SetParams(type, rangeMin, rangeMax, dimension);
}

void NormalizeProjection::DrawInfo(Canvas *canvas, QPainter &painter, Projector *projector)
{
    if(!canvas || !projector) return;
    if(canvas->canvasType) return;
    int dim = canvas->data->GetDimCount();
    if(params->dimensionSpin->value() >= dim) params->dimensionSpin->setValue(dim-1);
}

void NormalizeProjection::DrawModel(Canvas *canvas, QPainter &painter, Projector *projector)
{
    /*
    if(!canvas || !projector) return;
    if(canvas->canvasType) return;
    vector<fvec> samples = projector->source;
    vector<fvec> projected = projector->projected;
    ivec labels = canvas->data->GetLabels();
    if(!samples.size()) return;
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(Qt::black,0.5f));
    FOR(i, samples.size())
    {
        projected[i] = projector->Project(samples[i]);
        QPointF p1 = canvas->toCanvasCoords(samples[i]);
        QPointF p2 = canvas->toCanvasCoords(projected[i]);
        painter.drawLine(p1, p2);
    }
    painter.setOpacity(0.3);
    FOR(i, samples.size())
    {
        painter.setBrush(SampleColor[labels[i]%SampleColorCnt]);
        painter.setPen(Qt::black);
        QPointF p1 = canvas->toCanvasCoords(samples[i]);
        painter.drawEllipse(p1, 5, 5);
    }
    painter.setPen(Qt::black);
    FOR(i, projected.size())
    {
        painter.setBrush(SampleColor[labels[i]%SampleColorCnt]);
        QPointF p1 = canvas->toCanvasCoords(projected[i]);
        painter.drawEllipse(p1, 5, 5);
    }
    */
}

// virtual functions to manage the GUI and I/O
QString NormalizeProjection::GetAlgoString()
{
    return QString("Normalize");
}

void NormalizeProjection::SaveOptions(QSettings &settings)
{
    settings.setValue("typeCombo", params->typeCombo->currentIndex());
    settings.setValue("rangeMinSpin", params->rangeMinSpin->value());
    settings.setValue("rangeMaxSpin", params->rangeMaxSpin->value());
    settings.setValue("singleDimCheck", params->singleDimCheck->isChecked());
    settings.setValue("dimensionSpin", params->dimensionSpin->value());
}

bool NormalizeProjection::LoadOptions(QSettings &settings)
{
    if(settings.contains("typeCombo")) params->typeCombo->setCurrentIndex(settings.value("typeCombo").toInt());
    if(settings.contains("rangeMinSpin")) params->rangeMinSpin->setValue(settings.value("rangeMinSpin").toFloat());
    if(settings.contains("rangeMaxSpin")) params->rangeMaxSpin->setValue(settings.value("rangeMaxSpin").toFloat());
    if(settings.contains("singleDimCheck")) params->singleDimCheck->setChecked(settings.value("singleDimCheck").toBool());
    if(settings.contains("dimensionSpin")) params->dimensionSpin->setValue(settings.value("dimensionSpin").toInt());
    return true;
}

void NormalizeProjection::SaveParams(QTextStream &file)
{
    file << "projectOptions" << ":" << "typeCombo" << " " << params->typeCombo->currentIndex() << "\n";
    file << "projectOptions" << ":" << "rangeMinSpin" << " " << params->rangeMinSpin->value() << "\n";
    file << "projectOptions" << ":" << "rangeMaxSpin" << " " << params->rangeMaxSpin->value() << "\n";
    file << "projectOptions" << ":" << "singleDimCheck" << " " << params->singleDimCheck->isChecked() << "\n";
    file << "projectOptions" << ":" << "dimensionSpin" << " " << params->dimensionSpin->value() << "\n";
}

bool NormalizeProjection::LoadParams(QString name, float value)
{
    if(name.endsWith("typeCombo")) params->typeCombo->setCurrentIndex((int)value);
    if(name.endsWith("rangeMinSpin")) params->rangeMinSpin->setValue((float)value);
    if(name.endsWith("rangeMaxSpin")) params->rangeMaxSpin->setValue((float)value);
    if(name.endsWith("singleDimCheck")) params->singleDimCheck->setChecked((int)value);
    if(name.endsWith("dimensionSpin")) params->dimensionSpin->setValue((int)value);
    return true;
}
