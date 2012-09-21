#ifndef INTERFACEICAPROJECTION_H
#define INTERFACEICAPROJECTION_H

#include <vector>
#include <interfaces.h>
#include "ui_paramsICA.h"

class ICAProjection : public QObject, public ProjectorInterface
{
    Q_OBJECT
    Q_INTERFACES(ProjectorInterface)
private:
    Ui::paramsICA *params;
    QWidget *widget;
public:
    ICAProjection();
    // virtual functions to manage the algorithm creation
    Projector *GetProjector();
    void DrawInfo(Canvas *canvas, QPainter &painter, Projector *projector);
    void DrawModel(Canvas *canvas, QPainter &painter, Projector *projector);

    // virtual functions to manage the GUI and I/O
    QString GetName(){return QString("Independent Component Analysis");}
    QString GetAlgoString();
    QString GetInfoFile(){return "ICA.html";}
    QWidget *GetParameterWidget(){return widget;}
    void SetParams(Projector *projector);
    void SaveOptions(QSettings &settings);
    bool LoadOptions(QSettings &settings);
    void SaveParams(QTextStream &stream);
    bool LoadParams(QString name, float value);
};

#endif // INTERFACEICAPROJECTION_H
