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
public:
    Ui::DataGenerator *ui;
    explicit DataGenerator(Canvas *canvas, QWidget *parent = 0);
    ~DataGenerator();

    std::pair<std::vector<fvec>,ivec> Generate();

public slots:
    void OptionsChanged();
};

#endif // DATAGENERATOR_H
