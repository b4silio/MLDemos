#ifndef VISUALIZATION_H
#define VISUALIZATION_H

#include <public.h>
#include <canvas.h>
#include <datasetManager.h>
#include <QWidget>
#include <QPixmap>
#include <QEvent>

namespace Ui {
class Visualization;
}

class Visualization : public QWidget
{
    Q_OBJECT

private:
    void GenerateScatterPlot(bool bCheckOnly=false);
    void GenerateParallelCoords();
    void GenerateRadialGraph();
    void GenerateAndrewsPlot();
    void GenerateBubblePlot();
    void GenerateSplatterPlot();
    void GenerateIndividualPlot();
    void GenerateCorrelationPlot();
    void GenerateSampleDistancePlot();
    void GenerateDensityPlot();
    QPixmap GetGraphPixmap(int type, int inputType, int dim, int classCount, int index, int w, int h,
                           std::map<int,std::vector< std::pair<fvec,fvec> > > classGraphData,
                           std::vector< std::map<int,fvec> > dimClassData,
                           fvec mins, std::map<int,fvec> dimMins,
                           fvec maxes, std::map<int,fvec> dimMaxes);
    QPixmap GetStarPixmap(int inputType, int dim, int classCount, int index, int w, int h,
                          fvec mins, std::map<int,fvec> dimMins,
                          fvec maxes, std::map<int,fvec> dimMaxes);
    QPixmap GetRadialPixmap(std::map<int,std::vector< std::pair<fvec,fvec> > > classGraphData, int inputType, int dim, int classCount, int index, int w, int h, fvec mins, fvec maxes);
    fvec BoxPlot(fvec data);
    fvec Density(fvec data, float minv, float maxv, int bins=11);
    fvec KernelDensity(fvec data, float sigma, float minv, float maxv, int bins=31);
public:
    explicit Visualization(Canvas *canvas, QWidget *parent = 0);
    ~Visualization();

    bool eventFilter(QObject *obj, QEvent *event);
    void resizeEvent(QResizeEvent *event);
    bool SaveScreenshot(QString filename);
    QPixmap GetDisplayPixmap();

public slots:
    void OptionsChanged();
    void Update();
    void UpdateDims();

private:
    Ui::Visualization *ui;
    Canvas* canvas;
    DatasetManager* data;
    QPixmap displayPixmap;
    QWidget *spacer;
};

#endif // VISUALIZATION_H
