#ifndef ALGORITHMWIDGET_H
#define ALGORITHMWIDGET_H

#include <QtWidgets>

class BaseWidget : public QWidget{

    Q_OBJECT

public:

    BaseWidget(QWidget *parent = 0);
    ~BaseWidget();

protected:
    void closeEvent(QCloseEvent * event);

signals:

    void closed();

};


#endif // ALGORITHMWIDGET_H
