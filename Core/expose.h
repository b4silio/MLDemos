#ifndef EXPOSE_H
#define EXPOSE_H

#include <QWidget>
#include <QStringList>
#include <canvas.h>
#include "ui_expose.h"


namespace Ui {
    class Expose;
}

class Expose : public QWidget
{
    Q_OBJECT

public:
    explicit Expose(Canvas *canvas, QWidget *parent = 0);
    ~Expose();

    static void DrawData(QPixmap& pixmap, std::vector<fvec> samples, ivec labels, std::vector<dsmFlags> flags, int type, bool bProjected=false, QStringList names=QStringList(), std::pair<fvec,fvec> bounds=std::make_pair(fvec(),fvec()));
    static void DrawData(QPixmap& pixmap, std::vector<fvec> samples, std::vector<QColor> sampleColors, std::vector<dsmFlags> flags, int type, bool bProjected=false, bool bLearned=false, QStringList names=QStringList(), std::pair<fvec,fvec> bounds=std::make_pair(fvec(),fvec()));
    static void DrawTrajectories(QPixmap& pixmap, std::vector< std::vector<fvec> > trajectories, ivec labels, int type, int drawMode, std::pair<fvec,fvec> bounds=std::make_pair(fvec(),fvec()));
    static void DrawTrajectories(QPixmap& pixmap, std::vector< std::vector<fvec> > trajectories, std::vector<QColor> sampleColors, int type, int drawMode, std::pair<fvec,fvec> bounds=std::make_pair(fvec(),fvec()));
    static void DrawVariableData(QPixmap& pixmap, std::vector<fvec> samples, ivec labels, int type, fvec params, bool bProjected=false, QStringList names=QStringList());
    static void DrawVariableData(QPixmap& pixmap, std::vector<fvec> samples, std::vector<QColor> sampleColors, int type, fvec params, bool bProjected=false, bool bLearned=false, QStringList names=QStringList());

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent( QResizeEvent *event );

private:
    void GenerateScatterPlot(bool bCheckOnly=false);
    void GenerateParallelCoords();
    void GenerateRadialGraph();
    void GenerateAndrewsPlots();

private slots:
    void Clipboard();
    void Repaint();

private:
    Ui::Expose *ui;
    Canvas *canvas;
    QPixmap pixmap;
};

#endif // EXPOSE_H
