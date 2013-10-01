#ifndef INTERFACELLEProjection_H
#define INTERFACELLEProjection_H

#include <vector>
#include <interfaces.h>
#include "ui_paramsLLE.h"

class LLEProjection : public QObject, public ProjectorInterface
{
    Q_OBJECT
    Q_INTERFACES(ProjectorInterface)
private:
    Ui::paramsLLE *params;
    QWidget *widget;

public:
    LLEProjection();
    ~LLEProjection();
    // virtual functions to manage the algorithm creation
    Projector *GetProjector();
    void DrawInfo(Canvas *canvas, QPainter &painter, Projector *projector);
    void DrawModel(Canvas *canvas, QPainter &painter, Projector *projector);
    void DrawGL(Canvas *canvas, GLWidget *glw, Projector *projector){}

    bool eventFilter(QObject *obj, QEvent *event);

    // virtual functions to manage the GUI and I/O
    QString GetName(){return QString("Locally Linear Embedding");}
    QString GetAlgoString();
    QString GetInfoFile(){return "LLE.html";}
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

#endif // INTERFACELLEProjection_H
