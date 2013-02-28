#ifndef MARGINALWIDGET_H
#define MARGINALWIDGET_H

#include <public.h>
#include "ui_marginalWidget.h"
#include <QWidget>

class MarginalWidget : public QObject
{
    Q_OBJECT

private:
    static std::vector< std::vector<fvec> > classMarginals;
    static std::vector< std::vector< std::vector<fvec> > > classMarginalGmm;
    static std::vector<fvec> marginals;
    static std::vector< std::vector<fvec> > marginalGmm;
    static std::vector<fvec> limits;
    static bool bClassMarginal;

public:
    static QWidget *widget;
    static Ui::MarginalWidget *ui;

    MarginalWidget();
    ~MarginalWidget();

    bool eventFilter(QObject *obj, QEvent *event);

    void SetClassMarginals (std::vector< std::vector<fvec> > classMarginals,
                            std::vector< std::vector< std::vector<fvec> > > classMarginalGmm,
                            std::vector<fvec> limits);
    void SetMarginals (std::vector<fvec> marginals,
                       std::vector< std::vector<fvec> > marginalGmm,
                       std::vector<fvec> limits);
    void DrawMarginals();
    void SetDimensions(int dim, QStringList dimNames);

    void Show(){widget->show();}
    void Hide(){widget->hide();}

public slots:
    void MarginalChanged();
    
};

#endif // MARGINALWIDGET_H
