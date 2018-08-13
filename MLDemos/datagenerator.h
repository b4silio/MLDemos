#ifndef DATAGENERATOR_H
#define DATAGENERATOR_H

#include <QDialog>
#include <QMutex>
#include <canvas.h>
#include "ui_datasetgenerator.h"

namespace Ui {
class DatasetGenerator;
}

class DataGenerator : public QWidget
{
    Q_OBJECT
private:
    Canvas *canvas;
public:
    Ui::DatasetGenerator *ui;
    explicit DataGenerator(Canvas *canvas, QWidget *parent = 0);
    ~DataGenerator();

    std::pair<std::vector<fvec>,ivec> Generate();

public slots:
    void OptionsChanged();
};

#endif // DATAGENERATOR_H
