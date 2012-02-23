#ifndef INTERFACEKPCAProjection_H
#define INTERFACEKPCAProjection_H

#include <vector>
#include <interfaces.h>
#include "ui_paramsKPCA.h"

class KPCAProjection : public QObject, public ProjectorInterface
{
    Q_OBJECT
    Q_INTERFACES(ProjectorInterface)
private:
    Ui::paramsKPCA *params;
    QWidget *widget;
public:
    KPCAProjection();
    // virtual functions to manage the algorithm creation
    Projector *GetProjector();
    void DrawInfo(Canvas *canvas, QPainter &painter, Projector *projector);
    void DrawModel(Canvas *canvas, QPainter &painter, Projector *projector);

    // virtual functions to manage the GUI and I/O
    QString GetName(){return QString("KPCA");}
    QString GetAlgoString();
    QString GetInfoFile(){return "KPCA.html";}
    QWidget *GetParameterWidget(){return widget;}
    void SetParams(Projector *projector);
    void SaveOptions(QSettings &settings);
    bool LoadOptions(QSettings &settings);
    void SaveParams(QTextStream &stream);
    bool LoadParams(QString name, float value);
public slots:
    void ChangeOptions();
};

#endif // INTERFACEKPCAProjection_H
