
#include "interfaceRGPRegressor.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>

using namespace std;

RegrRGPR::RegrRGPR()
{
    params = new Ui::ParametersRGPRregr();
    params->setupUi(widget = new QWidget());
    connect(params->kernelTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeOptions()));
    ChangeOptions();
}

RegrRGPR::~RegrRGPR()
{
    delete params;
}

void RegrRGPR::ChangeOptions()
{


    switch(params->kernelTypeCombo->currentIndex())
    {
    case 0: // RBF
        params->kernelTypeCombo->setVisible(true);
        break;
    }
}

void RegrRGPR::SetParams(Regressor *regressor)
{
    if(!regressor) return;
    RegressorRGPR *gpr = dynamic_cast<RegressorRGPR*>(regressor);
    if(!gpr) return;

    int randomType = params->randomTypeCombo->currentIndex();
    int randomRank = params->randRankSpin->value();
    int kernelType = params->kernelTypeCombo->currentIndex();
    float kernelGamma = params->kernelWidthSpin->value();
    double kernelNoise = params->noiseSpin->value();

    gpr->SetParams(randomType, randomRank, kernelType, 1 / kernelGamma, kernelNoise);
}

fvec RegrRGPR::GetParams()
{
    int randType = params->randomTypeCombo->currentIndex();
    int randRank = params->randRankSpin->value();
    int kernelType = params->kernelTypeCombo->currentIndex();
    float kernelGamma = params->kernelWidthSpin->value();
    double kernelNoise = params->noiseSpin->value();

    fvec par(5);
    par[0] = randType;
    par[1] = randRank;
    par[2] = kernelType;
    par[3] = kernelGamma;
    par[4] = kernelNoise;

    return par;
}

void RegrRGPR::SetParams(Regressor *regressor, fvec parameters)
{
    if(!regressor) return;
    RegressorRGPR *gpr = dynamic_cast<RegressorRGPR*>(regressor);
    if(!gpr) return;

    int i = 0;
    int randomType = parameters.size() > i ? parameters[i] : 0; i++;
    int randomRank = parameters.size() > i ? parameters[i] : 0; i++;
    int kernelType = parameters.size() > i ? parameters[i] : 0; i++;
    float kernelGamma = parameters.size() > i ? parameters[i] : 0; i++;
    double kernelNoise = parameters.size() > i ? parameters[i] : 0; i++;

    gpr->SetParams(randomType, randomRank, kernelType, kernelGamma, kernelNoise);
}

void RegrRGPR::GetParameterList(std::vector<QString> &parameterNames,
                             std::vector<QString> &parameterTypes,
                             std::vector< std::vector<QString> > &parameterValues)
{
    parameterNames.clear();
    parameterTypes.clear();
    parameterValues.clear();
    parameterNames.push_back("randomType");
    parameterNames.push_back("randomRank");
    parameterNames.push_back("kernelType");
    parameterNames.push_back("kernelGamma");
    parameterNames.push_back("kernelNoise");
    parameterTypes.push_back("List");
    parameterTypes.push_back("Integer");
    parameterTypes.push_back("List");
    parameterTypes.push_back("Real");
    parameterTypes.push_back("Real");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("Fourier");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("10");
    parameterValues.back().push_back("10000");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("RBF");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("0.00000001f");
    parameterValues.back().push_back("99999999999");
    parameterValues.push_back(vector<QString>());
    parameterValues.back().push_back("0.0000000001f");
    parameterValues.back().push_back("999999999999");
}

QString RegrRGPR::GetAlgoString()
{
    int randType = params->randomTypeCombo->currentIndex();
    int randRank = params->randRankSpin->value();
    float noise = params->noiseSpin->value();
    int kernelType = params->kernelTypeCombo->currentIndex();
    float kernelGamma = params->kernelWidthSpin->value();

    QString algo;

    algo += "Random GPR";
    switch(randType)
    {
    case RANDOM_FOURIER:
        algo += " Fourier";
    }

    algo += QString(" Rank %1 %2").arg(randRank).arg(noise);
    switch(kernelType)
    {
    case RAND_KERNEL_RBF:
        algo += " RBF";
        algo + QString(" %1").arg(kernelGamma);
        break;

    }

    return algo;
}

Regressor *RegrRGPR::GetRegressor()
{
    Regressor *regressor = 0;
    regressor = new RegressorRGPR();
    SetParams(regressor);
    return regressor;
}

void DrawArrow( const QPointF &ppt, const QPointF &pt, double sze, QPainter &painter)
{
    QPointF pd, pa, pb;
    double tangent;

    pd = ppt - pt;
    if (pd.x() == 0 && pd.y() == 0)
        return;
    tangent = atan2 ((double) pd.y(), (double) pd.x());
    pa.setX(sze * cos (tangent + M_PI / 7.f) + pt.x());
    pa.setY(sze * sin (tangent + M_PI / 7.f) + pt.y());
    pb.setX(sze * cos (tangent - M_PI / 7.f) + pt.x());
    pb.setY(sze * sin (tangent - M_PI / 7.f) + pt.y());
    //-- connect the dots...
    painter.drawLine(pt, ppt);
    painter.drawLine(pt, pa);
    painter.drawLine(pt, pb);
}

void RegrRGPR::DrawInfo(Canvas *canvas, QPainter &painter, Regressor *regressor)
{
    RegressorRGPR * gpr = dynamic_cast<RegressorRGPR*>(regressor);
    if(!gpr) return;
    painter.setRenderHint(QPainter::Antialiasing);
    int xIndex = canvas->xIndex;
    int yIndex = canvas->yIndex;

    int radius = 8;
    int dim = canvas->data->GetDimCount()-1;
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(Qt::red,3));
    FOR(i, gpr->GetBasisCount())
    {
        fvec basis = gpr->GetBasisVector(i);
        fvec testBasis(dim+1);
        FOR(d, dim) testBasis[d] = basis[d];
        fvec res = gpr->Test(testBasis);
        // we draw the basis circle
        float conf = basis[dim + xIndex];
        QPointF pt1 = canvas->toCanvasCoords(basis[xIndex],res[0]);
        painter.drawEllipse(pt1, radius, radius);
        // and the arrow of the direction
        QPointF pt2 = pt1 + QPointF(0,(conf>0 ? 1 : -1)*radius);
        QPointF pt3 = pt2 + QPointF(0,(conf>0 ? 1 : -1)*25*(0.5+min(2.f,fabs(conf)/5.f)));
        DrawArrow(pt2,pt3,10,painter);
    }
}

void RegrRGPR::DrawConfidence(Canvas *canvas, Regressor *regressor)
{

    RegressorRGPR *gpr = dynamic_cast<RegressorRGPR *>(regressor);
    if(gpr && gpr->_model)
    {
        /*
        int w = canvas->width();
        int h = canvas->height();
        int dim = canvas->data->GetDimCount()-1;
        int outputDim = regressor->outputDim;
        int xIndex = canvas->xIndex;
        int yIndex = canvas->yIndex;
        Matrix _testout;
        ColumnVector _testin(dim);
        QImage density(QSize(256,256), QImage::Format_RGB32);
        density.fill(0);
        // we draw a density map for the probability
        for (int i=0; i < density.width(); i++)
        {
            fvec sampleIn = canvas->toSampleCoords(i*w/density.width(),0);
            FOR(d, dim) _testin(d+1) = sampleIn[d];
            if(outputDim != -1 && outputDim < dim) _testin(outputDim+1) = sampleIn[dim];
            double sigma;
            _testout = gpr->sogp->predict(_testin, sigma);
            sigma = sigma*sigma;
            float testout = _testout(1,1);
            for (int j=0; j< density.height(); j++)
            {
                fvec sampleOut = canvas->toSampleCoords(i*w/density.width(),j*h/density.height());
                float val = gpr->GetLikelihood(testout, sigma, sampleOut[yIndex]);
                int color = min(255,(int)(128 + val*20));
                density.setPixel(i,j, qRgb(color,color,color));
            }
        }
        canvas->maps.confidence = QPixmap::fromImage(density.scaled(QSize(w,h),Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        */
    }
    else canvas->maps.confidence = QPixmap();
}

void RegrRGPR::DrawModel(Canvas *canvas, QPainter &painter, Regressor *regressor)
{
    painter.setRenderHint(QPainter::Antialiasing, true);
    int w = canvas->width();
    int h = canvas->height();
    int xIndex = canvas->xIndex;
    fvec sample = canvas->toSampleCoords(0,0);
    int dim = sample.size();
    if(dim > 2) return;

    RegressorRGPR * gpr = dynamic_cast<RegressorRGPR*>(regressor);
    if(!gpr) return;

    int steps = w;
    QPainterPath path, pathUp, pathDown;
    QPointF oldPoint(-FLT_MAX,-FLT_MAX);
    QPointF oldPointUp(-FLT_MAX,-FLT_MAX);
    QPointF oldPointDown(-FLT_MAX,-FLT_MAX);
    FOR(x, steps)
    {
        sample = canvas->toSampleCoords(x,0);
        fvec res = regressor->Test(sample);
        if(res[0] != res[0] || res[1] != res[1]) continue;
        QPointF point = canvas->toCanvasCoords(sample[xIndex], res[0]);
        QPointF pointUp = canvas->toCanvasCoords(sample[xIndex],res[0] + res[1]);
        QPointF pointDown = canvas->toCanvasCoords(sample[xIndex],res[0] - res[1]);
        if(x)
        {
            path.lineTo(point);
            pathUp.lineTo(pointUp);
            pathDown.lineTo(pointDown);
        }
        else
        {
            path.moveTo(point);
            pathUp.moveTo(pointUp);
            pathDown.moveTo(pointDown);
        }
        oldPoint = point;
        oldPointUp = pointUp;
        oldPointDown = pointDown;
    }
    painter.setBackgroundMode(Qt::TransparentMode);
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(Qt::black, 1));
    painter.drawPath(path);
    painter.setPen(QPen(Qt::black, 0.5, Qt::DashLine));
    painter.drawPath(pathUp);
    painter.drawPath(pathDown);
}

void RegrRGPR::SaveOptions(QSettings &settings)
{
    settings.setValue("randomType", params->randomTypeCombo->currentIndex());
    settings.setValue("randomRank", params->randRankSpin->value());
    settings.setValue("kernelType", params->kernelTypeCombo->currentIndex());
    settings.setValue("kernelWidth", params->kernelWidthSpin->value());
    settings.setValue("noiseSpin", params->noiseSpin->value());
}

bool RegrRGPR::LoadOptions(QSettings &settings)
{
    if(settings.contains("randomType")) params->randomTypeCombo->setCurrentIndex(settings.value("randomType").toInt());
    if(settings.contains("randomRank")) params->randRankSpin->setValue(settings.value("randomRank").toInt());
    if(settings.contains("kernelType")) params->kernelTypeCombo->setCurrentIndex(settings.value("kernelType").toInt());
    if(settings.contains("kernelWidth")) params->kernelWidthSpin->setValue(settings.value("kernelWidth").toFloat());
    if(settings.contains("noiseSpin")) params->noiseSpin->setValue(settings.value("noiseSpin").toFloat());

    return true;
}

void RegrRGPR::SaveParams(QTextStream &file)
{
    file << "regressionOptions" << ":" << "randomType" << " " << params->randomTypeCombo->currentIndex() << "\n";
    file << "regressionOptions" << ":" << "randomRank" << " " << params->randRankSpin->value() << "\n";
    file << "regressionOptions" << ":" << "kernelType" << " " << params->kernelTypeCombo->currentIndex() << "\n";
    file << "regressionOptions" << ":" << "kernelWidth" << " " << params->kernelWidthSpin->value() << "\n";
    file << "regressionOptions" << ":" << "noiseSpin" << " " << params->noiseSpin->value() << "\n";
}


bool RegrRGPR::LoadParams(QString name, float value)
{
    if(name.endsWith("randomType")) params->randomTypeCombo->setCurrentIndex(int(value));
    if(name.endsWith("randomRank")) params->randRankSpin->setValue((int)value);
    if(name.endsWith("kernelType")) params->kernelTypeCombo->setCurrentIndex((int)value);
    if(name.endsWith("kernelWidth")) params->kernelWidthSpin->setValue(value);
    if(name.endsWith("noiseSpin")) params->noiseSpin->setValue(value);

    return true;
}
