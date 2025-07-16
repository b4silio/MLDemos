#ifndef DATASETEDITOR_H
#define DATASETEDITOR_H

#include <QWidget>
#include <QLabel>
#include <datasetManager.h>
#include <canvas.h>

namespace Ui {
class DatasetEditor;
}

class DatasetEditor : public QWidget
{
    Q_OBJECT
    
public:
    explicit DatasetEditor(Canvas *canvas, QWidget *parent = 0);
    ~DatasetEditor();
    void Update();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

public slots:
    void UpdateClassName();
    void UpdateDimName();
    void UpdateCategoryName();
    void ChangeClassName();
    void ChangeDimName();
    void ChangeCategoryName();
    void ShiftDimensions();
    void SwapDimensions();
    void ResetDimensions();
    void SetAsClass();

signals:
    void DataEdited();

public:
    Canvas *canvas;
    DatasetManager *data;
private:
    Ui::DatasetEditor *ui;
    std::vector<QLabel*> dimLabels;
    ivec dimIndices;
    QLabel *dragLabel;
    int dragIndex;
};

#endif // DATASETEDITOR_H
