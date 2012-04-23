#ifndef DATAGENERATOR_H
#define DATAGENERATOR_H

#include <QDialog>
#include <QMutex>
#include <canvas.h>

namespace Ui {
class DataGenerator;
}

class DataGenerator : public QDialog
{
    Q_OBJECT
    Canvas *canvas;
    QMutex *mutex;
public:
    explicit DataGenerator(Canvas *canvas, QMutex *mutex, QWidget *parent = 0);
    ~DataGenerator();
    
private:
    Ui::DataGenerator *ui;
public slots:
    void Generate();
    void OptionsChanged();
};

#endif // DATAGENERATOR_H
