#ifndef INTERFACELLEProjection_H
#define INTERFACELLEProjection_H

#include <vector>
#include <interfaces.h>
#include "ui_paramsLLE.h"
//#include "ui_contourPlots.h"
//#include "eigen_pca.h"
//#include "qcontour.h"

class LLEProjection : public QObject, public ProjectorInterface
{
    Q_OBJECT
    Q_INTERFACES(ProjectorInterface)
private:
    Ui::paramsLLE *params;
    QWidget *widget;

    //Ui::ContourWidget *contours;
    //QWidget *contourWidget;
    //std::vector<fvec> contourSamples;
    //ivec contourSampleLabels;
    //std::map<int,QPixmap> contourPixmaps;
    //PCA *pcaPointer;
    //PCA contourPca;
    //int xIndex, yIndex;
    //double xmin,xmax;
    //double ymin,ymax;
    //void GetContoursPixmap(int index);    

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

public slots:
    //void ChangeOptions();
    //void ShowContours();
    //void ContoursChanged();
    //void DrawContours(int index);
    //void SaveScreenshot();
};

#endif // INTERFACELLEProjection_H
