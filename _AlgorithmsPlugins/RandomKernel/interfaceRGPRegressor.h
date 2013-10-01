#ifndef INTERFACERGPREGRESSOR_H
#define INTERFACERGPREGRESSOR_H

#include <vector>
#include <interfaces.h>
#include "regressorRGPR.h"
#include "ui_paramsRGPR.h"

class RegrRGPR : public QObject, public RegressorInterface
{
    Q_OBJECT
    Q_INTERFACES(RegressorInterface)
private:
    QWidget *widget;
    Ui::ParametersRGPRregr *params;
public:
    RegrRGPR();
    ~RegrRGPR();
    // virtual functions to manage the algorithm creation
    Regressor *GetRegressor();
    void DrawInfo(Canvas *canvas, QPainter &painter, Regressor *regressor);
    void DrawModel(Canvas *canvas, QPainter &painter, Regressor *regressor);
    void DrawGL(Canvas *canvas, GLWidget *glw, Regressor *regressor){}
    void DrawConfidence(Canvas *canvas, Regressor *regressor);

    // virtual functions to manage the GUI and I/O
    QString GetName(){return QString("Random GP Regression");}
    QString GetAlgoString();
    QString GetInfoFile(){return "GPR.html";}
    QWidget *GetParameterWidget(){return widget;}
    void SetParams(Regressor *regressor);
    void SaveOptions(QSettings &settings);
    bool LoadOptions(QSettings &settings);
    void SaveParams(QTextStream &stream);
    bool LoadParams(QString name, float value);
    void SetParams(Regressor *regressor, fvec parameters);
    fvec GetParams();
    void GetParameterList(std::vector<QString> &parameterNames,
                          std::vector<QString> &parameterTypes,
                          std::vector< std::vector<QString> > &parameterValues);

public slots:
    void ChangeOptions();
};

#endif // INTERFACERGPREGRESSOR_H
