#ifndef INTERFACESAMMONPROJECTION_H
#define INTERFACESAMMONPROJECTION_H

#include <vector>
#include <interfaces.h>
#include "ui_paramsSammon.h"

class SammonProjection : public QObject, public ProjectorInterface
{
    Q_OBJECT
    Q_INTERFACES(ProjectorInterface)
private:
    Ui::paramsSammon *params;
    QWidget *widget;
public:
    SammonProjection();
    ~SammonProjection();
    // virtual functions to manage the algorithm creation
    Projector *GetProjector();
    void DrawInfo(Canvas *canvas, QPainter &painter, Projector *projector);
    void DrawModel(Canvas *canvas, QPainter &painter, Projector *projector);
    void DrawGL(Canvas *canvas, GLWidget *glw, Projector *projector){}

    // virtual functions to manage the GUI and I/O
    QString GetName(){return QString("Sammon Projection");}
    QString GetAlgoString();
    QString GetInfoFile(){return "sammon.html";}
    QWidget *GetParameterWidget(){return widget;}
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
};

#endif // INTERFACESAMMONPROJECTION_H
