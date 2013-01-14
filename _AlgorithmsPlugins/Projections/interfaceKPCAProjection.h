#ifndef INTERFACEKPCAProjection_H
#define INTERFACEKPCAProjection_H

#include <vector>
#include <interfaces.h>
#include "ui_paramsKPCA.h"
#include "ui_contourPlots.h"
#include "eigen_pca.h"
#include "qcontour.h"

class KPCAProjection : public QObject, public ProjectorInterface
{
    Q_OBJECT
    Q_INTERFACES(ProjectorInterface)
private:
    Ui::paramsKPCA *params;
    Ui::ContourWidget *contours;
    QWidget *widget, *contourWidget;
    QLabel *contourLabel;
    std::vector<fvec> contourSamples;
    ivec contourSampleLabels;
    std::map<int,QPixmap> contourPixmaps;
    PCA *pcaPointer;
    PCA contourPca;
    int xIndex, yIndex;
    double xmin,xmax;
    double ymin,ymax;
    void GetContoursPixmap(int index);

public:
    KPCAProjection();
    // virtual functions to manage the algorithm creation
    Projector *GetProjector();
    void DrawInfo(Canvas *canvas, QPainter &painter, Projector *projector);
    void DrawModel(Canvas *canvas, QPainter &painter, Projector *projector);
    void DrawGL(Canvas *canvas, GLWidget *glw, Projector *projector){}

    // virtual functions to manage the GUI and I/O
    QString GetName(){return QString("Kernel PCA");}
    QString GetAlgoString();
    QString GetInfoFile(){return "KPCA.html";}
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
    void ShowContours();
    void ContoursChanged();
    void DrawContours(int index);
    void SaveScreenshot();
};

#endif // INTERFACEKPCAProjection_H
