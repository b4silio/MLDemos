#ifndef DATAGENERATOR_H
#define DATAGENERATOR_H

#include <QDialog>
#include <QMutex>
#include <canvas.h>
#include "ui_datagenerator.h"

namespace Ui {
class DataGenerator;
}

class DataGenerator : public QDialog
{
    Q_OBJECT
private:
    Canvas *canvas;
    QMutex *mutex;
public:
    Ui::DataGenerator *ui;
    explicit DataGenerator(Canvas *canvas, QMutex *mutex, QWidget *parent = 0);
    ~DataGenerator();
    
public slots:
    void Generate();
    void OptionsChanged();
};

#endif // DATAGENERATOR_H
