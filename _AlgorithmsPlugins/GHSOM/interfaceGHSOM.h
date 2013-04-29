#ifndef INTERFACEGHSOM_H
#define INTERFACEGHSOM_H

#include <vector>
#include <interfaces.h>
#include "ui_paramsGHSOM.h"

class GHSOMProjector : public QObject, public ProjectorInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "GHSOMProjector" FILE "plugin.json")
    Q_INTERFACES(ProjectorInterface)
private:
    Ui::paramsGHSOM *params;
    QWidget *widget;
public:
    GHSOMProjector();
    ~GHSOMProjector();
    // virtual functions to manage the algorithm creation
    Projector *GetProjector();
    void DrawInfo(Canvas *canvas, QPainter &painter, Projector *projector);
    void DrawModel(Canvas *canvas, QPainter &painter, Projector *projector);
    void DrawGL(Canvas *canvas, GLWidget *glw, Projector *projector);

    // virtual functions to manage the GUI and I/O
    QString GetName(){return QString("Self Organizing Maps");}
    QString GetAlgoString();
    QString GetInfoFile(){return "ghsom.html";}
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
public slots:
    void ChangeOptions();
};

#endif // INTERFACEGHSOM_H
