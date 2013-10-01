#ifndef INTERFACECVOPROJECTION_H
#define INTERFACECVOPROJECTION_H

#include <vector>
#include <interfaces.h>
#include "ui_paramsCVO.h"

class QTableWidget;

class CVOProjection : public QObject, public ProjectorInterface
{
    Q_OBJECT
    Q_INTERFACES(ProjectorInterface)

public:
    CVOProjection();
    ~CVOProjection();
    // virtual functions to manage the algorithm creation
    Projector *GetProjector();
    void DrawInfo(Canvas *canvas, QPainter &painter, Projector *projector);
    void DrawModel(Canvas *canvas, QPainter &painter, Projector *projector);
    void DrawGL(Canvas *canvas, GLWidget *glw, Projector *projector) {}

    // virtual functions to manage the GUI and I/O
    QString GetName(){ return QString("Convex Optimisation Metric Learning"); }
    QString GetAlgoString();
    QString GetInfoFile() { return "CVO.html"; }
    QWidget* GetParameterWidget() { return widget; }
    void SetParams(Projector *projector);
    void SaveOptions(QSettings &settings);
    bool LoadOptions(QSettings &settings);
    void SaveParams(QTextStream &stream);
    bool LoadParams(QString name, float value);
    void SetParams(Projector *projector, fvec parameters);
    fvec GetParams();
    void GetParameterList(std::vector<QString> &parameterNames,
                                 std::vector<QString> &parameterTypes,
                                 std::vector< std::vector<QString> > &parameterValues);

private slots:
    void onOthersChkBoxClicked( bool checked );

    void onShowMatrixBtnClicked();

private:
    Ui::paramsCVO* params;
    QWidget* widget;
    QTableWidget* table;
};

#endif // INTERFACEICVOPROJECTION_H
