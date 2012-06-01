#ifndef INTERFACEPCAPROJECTION_H
#define INTERFACEPCAPROJECTION_H

#include <interfaces.h>
#include <projector.h>
#include "ui_paramsPCA.h"
#include <QTableWidget>

class PCAProjection : public QObject, public ProjectorInterface
{
    Q_OBJECT
    Q_INTERFACES(ProjectorInterface)
private:
    QWidget *widget;
    Ui::paramsPCA *params;
    QWidget *eigenWidget;
    QTableWidget *eigenTable;
public:
    PCAProjection();
    // virtual functions to manage the algorithm creation
    Projector *GetProjector();
    void DrawInfo(Canvas *canvas, QPainter &painter, Projector *projector);
    void DrawModel(Canvas *canvas, QPainter &painter, Projector *projector);

    // virtual functions to manage the GUI and I/O
    QString GetName(){return QString("Principal Component Analysis");}
    QString GetAlgoString();
    QString GetInfoFile(){return "PCA.html";}
    QWidget *GetParameterWidget(){return widget;}
    void SetParams(Projector *projector);
    void SaveOptions(QSettings &settings);
    bool LoadOptions(QSettings &settings);
    void SaveParams(QTextStream &stream);
    bool LoadParams(QString name, float value);

public slots:
    void ShowEigenVectors();
    void ChangeOptions();
};

#endif // INTERFACEPCAPROJECTION_H
