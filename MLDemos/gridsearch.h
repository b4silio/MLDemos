#ifndef GRIDSEARCH_H
#define GRIDSEARCH_H

#include <QWidget>
#include <QLabel>
#include <QList>
#include <vector>
#include "interfaces.h"

namespace Ui {
class GridSearch;
}

class GridLabel : public QLabel
{
    Q_OBJECT
public:
    GridLabel(QWidget *parent=0);
protected:
    void focusOutEvent(QFocusEvent *ev);
    void mouseMoveEvent(QMouseEvent *event);
signals:
    void MouseMove(QMouseEvent *event);
};

typedef std::pair< std::pair<float,float>, std::pair<float,float> > fPair;
class GridSearch : public QWidget
{
    Q_OBJECT
public:
    Canvas *canvas;
    ClassifierInterface* classifier;
    ClustererInterface* clusterer;
    RegressorInterface* regressor;
    DynamicalInterface* dynamical;
    AvoidanceInterface* avoider;
    MaximizeInterface* maximizer;
    ReinforcementInterface* reinforcement;
    ProjectorInterface* projector;
private:
    Ui::GridSearch *ui;
    std::vector<QString> names, types;
    std::vector< std::vector<QString> > values;
    fvec map;
    std::map<QString,fvec> mapList;
    QPixmap pixmap;
    int mapX;
    int mapY;
    QPoint mousePoint;

public:
    explicit GridSearch(Canvas *canvas, QWidget *parent = 0);
    ~GridSearch();

protected:
    void paintEvent(QPaintEvent *);

private:
    fPair GetParamsRange();
    void DisplayResults();

public slots:
    void MouseMove(QMouseEvent *);
    void Close();
    void OptionsChanged();
    void DisplayChanged();
    void Run();
    void Clipboard();
    void Update();
    void SetClassifier(ClassifierInterface *c);
    void SetClusterer(ClustererInterface *c);
    void SetRegressor(RegressorInterface *c);
    void SetDynamical(DynamicalInterface *c);
    void SetAvoidance(AvoidanceInterface *c);
    void SetMaximizer(MaximizeInterface *c);
    void SetReinforcement(ReinforcementInterface *c);
    void SetProjector(ProjectorInterface *c);
};

#endif // GRIDSEARCH_H
