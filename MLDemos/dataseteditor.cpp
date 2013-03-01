#include "dataseteditor.h"
#include "ui_dataseteditor.h"
#include <QDebug>

using namespace std;

DatasetEditor::DatasetEditor(Canvas *canvas, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DatasetEditor),
    canvas(canvas), data(canvas->data), dragLabel(0)
{
    ui->setupUi(this);
    setWindowTitle("Dataset Editor");
    connect(ui->classNameCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateClassName()));
    connect(ui->dimNameCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateDimName()));
    connect(ui->categoryNameCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateCategoryName()));
    connect(ui->classNameButton, SIGNAL(clicked()), this, SLOT(ChangeClassName()));
    connect(ui->dimNameButton, SIGNAL(clicked()), this, SLOT(ChangeDimName()));
    connect(ui->categoryNameButton, SIGNAL(clicked()), this, SLOT(ChangeCategoryName()));
    connect(ui->shiftButton, SIGNAL(clicked()), this, SLOT(ShiftDimensions()));
    connect(ui->resetButton, SIGNAL(clicked()), this, SLOT(ResetDimensions()));
    connect(ui->swapButton, SIGNAL(clicked()), this, SLOT(SwapDimensions()));
    connect(ui->asClassButton, SIGNAL(clicked()), this, SLOT(SetAsClass()));

    ui->dimLabelsWidget->setAcceptDrops(true);
    ui->dimLabelsWidget->installEventFilter(this);
    if(!ui->dimLabelsWidget->layout())
    {
        QLayout *layout = new QGridLayout();
        ui->dimLabelsWidget->setLayout(layout);
    }
    Update();
}

DatasetEditor::~DatasetEditor()
{
    delete ui;
}

void DatasetEditor::Update()
{
    int dimCount = data->GetDimCount();
    FOR(i, dimLabels.size())
    {
        ui->dimLabelsWidget->layout()->removeWidget(dimLabels[i]);
        delete dimLabels[i];
        dimIndices.clear();
        dragLabel = 0;
    }
    dimLabels.clear();
    FOR(i, dimCount)
    {
        QString dimensionName = QString("Dimension %1").arg(i+1);
        if(canvas->dimNames.size() > i) dimensionName = canvas->dimNames.at(i);
        QLabel *label = new QLabel(dimensionName);
        label->setStyleSheet(QString("QLabel { background-color : #%1%2%3; color : %4; qproperty-alignment: AlignCenter;}")
                             .arg((int)(i*255.f/(dimCount-1)),0,16)
                             .arg((int)(i*255.f/(dimCount-1)),0,16)
                             .arg((int)(i*255.f/(dimCount-1)),0,16)
                             .arg(i < dimCount / 2 ? "white" : "black"));
        //if((i%2 +(i/4)%2)%2) label->setStyleSheet("QLabel { background-color : white; color : black;  qproperty-alignment: AlignCenter;}");
        //else label->setStyleSheet("QLabel { background-color : black; color : white;  qproperty-alignment: AlignCenter;}");
        label->installEventFilter(this);
        dimLabels.push_back(label);
        dimIndices.push_back(i);
        ((QGridLayout *)ui->dimLabelsWidget->layout())->addWidget(dimLabels.back(), i/4, i%4);
    }
    int c1 = ui->classNameCombo->currentIndex();
    int c2 = ui->dimNameCombo->currentIndex();
    ui->classNameCombo->blockSignals(true);
    ui->dimNameCombo->blockSignals(true);
    ui->categoryNameCombo->blockSignals(true);
    ui->classNameCombo->clear();
    ui->dimNameCombo->clear();
    ui->categoryNameCombo->clear();
    int classCount = data->GetClassCount(data->GetLabels());
    FOR(i, classCount) ui->classNameCombo->addItem((canvas->classNames.count(i) ? canvas->classNames[i] : QString("Class %1").arg(i)));
    FOR(i, dimCount) ui->dimNameCombo->addItem(canvas->dimNames.size() > i ? canvas->dimNames[i] : QString("Dimension %1").arg(i+1));
    if(c1 >= 0 && c1 < ui->classNameCombo->count()) ui->classNameCombo->setCurrentIndex(c1);
    if(c2 >= 0 && c2 < ui->dimNameCombo->count()) ui->dimNameCombo->setCurrentIndex(c2);
    ui->classNameCombo->blockSignals(false);
    ui->dimNameCombo->blockSignals(false);
    ui->categoryNameCombo->blockSignals(false);
    UpdateDimName();
}

bool DatasetEditor::eventFilter(QObject *obj, QEvent *event)
{
    if(obj == ui->dimLabelsWidget)
    {
        if(event->type() == QEvent::DragEnter)
        {
            if(((QDragEnterEvent*)event)->mimeData()->hasFormat("text/plain")) ((QDragEnterEvent*)event)->acceptProposedAction();
        }
        else if(event->type() == QEvent::Drop)
        {
            QDropEvent *e = dynamic_cast<QDropEvent*>(event);
            if(e->mimeData()->hasFormat("text/plain"))
            {
                e->acceptProposedAction();
                QPoint pos = e->pos();
                int labelIndex = -1;
                FOR(i, dimLabels.size())
                {
                    QPoint labelPos = dimLabels[i]->mapFrom(ui->dimLabelsWidget, pos);
                    if(labelPos.x() < 0 || labelPos.y() < 0) continue;
                    if(labelPos.x() > dimLabels[i]->width()) continue;
                    if(labelPos.y() > dimLabels[i]->height()) continue;
                    labelIndex = i;
                }
                QStringList list = e->mimeData()->text().split(":");
                int dim = dragIndex;
                bool bReplaceLabels = false;
                // we move the label at the end of the list
                int bkpLabel = dimIndices[dim];
                if(labelIndex == -1 || labelIndex == dimLabels.size()-1)
                {
                    if(dim <= dimLabels.size()-1)
                    {
                        dimLabels.erase(dimLabels.begin() + dim);
                        dimLabels.push_back(dragLabel);
                        dimIndices.erase(dimIndices.begin() + dim);
                        dimIndices.push_back(bkpLabel);
                        bReplaceLabels = true;
                    }
                }
                // we shift everything else
                else if(dim != labelIndex)
                {
                    if(dim == dimLabels.size()-1)
                    {
                        dimLabels.pop_back();
                        dimIndices.pop_back();
                    }
                    else
                    {
                        dimLabels.erase(dimLabels.begin()+dim);
                        dimIndices.erase(dimIndices.begin()+dim);
                    }
                    dimLabels.insert(dimLabels.begin()+labelIndex, dragLabel);
                    dimIndices.insert(dimIndices.begin()+labelIndex, bkpLabel);
                    bReplaceLabels = true;
                }
                //dragLabel->show();
                if(bReplaceLabels)
                {
                    FOR(i, dimLabels.size())
                    {
                        ui->dimLabelsWidget->layout()->removeWidget(dimLabels[i]);
                    }
                    FOR(i, dimLabels.size())
                    {
                        ((QGridLayout*)ui->dimLabelsWidget->layout())->addWidget(dimLabels[i],i/4,i%4);
                    }
                }
                return true;
            }
        }
    }
    if(dynamic_cast<QLabel*>(obj))
    {
        QLabel *label = dynamic_cast<QLabel*>(obj);
        if(event->type() == QEvent::MouseButtonDblClick)
        {
            int labelIndex = -1;
            FOR(i, dimLabels.size())
            {
                if(label == dimLabels[i])
                {
                    labelIndex = i;
                    break;
                }
            }
            if(labelIndex != -1)
            {
                QLabel *label = dimLabels[labelIndex];
                dimLabels.erase(dimLabels.begin() + labelIndex);
                dimIndices.erase(dimIndices.begin() + labelIndex);
                ui->dimLabelsWidget->layout()->removeWidget(label);
                delete label;
                FOR(i, dimLabels.size())
                {
                    ui->dimLabelsWidget->layout()->removeWidget(dimLabels[i]);
                }
                FOR(i, dimLabels.size())
                {
                    ((QGridLayout*)ui->dimLabelsWidget->layout())->addWidget(dimLabels[i],i/4,i%4);
                }
            }
            return true;
        }
        else if(event->type() == QEvent::MouseMove)
        {
            QMouseEvent *e = dynamic_cast<QMouseEvent*>(event);
            if(e->buttons() == Qt::LeftButton)
            {
                int labelIndex = -1;
                FOR(i, dimLabels.size())
                {
                    if(label == dimLabels[i])
                    {
                        labelIndex = i;
                        break;
                    }
                }
                dragLabel = label;
                dragIndex = labelIndex;
                //label->hide();
                QDrag *drag = new QDrag(this);
                QMimeData *mimeData = new QMimeData;
                QString dimensionName = QString("Dimension %1").arg(dimIndices[dragIndex]+1);
                if(canvas->dimNames.size() > dragIndex) dimensionName = canvas->dimNames.at(dimIndices[dragIndex]);
                mimeData->setText(QString("%1(%2)").arg(dimensionName).arg(labelIndex));
                drag->setMimeData(mimeData);
                drag->exec();
                return true;
            }
        }
        // pass the event on to the parent class
        return QWidget::eventFilter(obj, event);
    }
    // pass the event on to the parent class
    else return QWidget::eventFilter(obj, event);
}

void DatasetEditor::UpdateClassName()
{
    ui->classNameEdit->setText(ui->classNameCombo->currentText());
}

void DatasetEditor::UpdateDimName()
{
    int index = ui->dimNameCombo->currentIndex();
    if(data->IsCategorical(index))
    {
        ui->categoryNameCombo->setEnabled(true);
        ui->categoryNameCombo->blockSignals(true);
        int c = ui->categoryNameCombo->currentIndex();
        ui->categoryNameCombo->clear();
        FOR(i, data->categorical[index].size()) ui->categoryNameCombo->addItem(QString(data->categorical[index][i].c_str()));
        if(c >= 0 && c < ui->categoryNameCombo->count()) ui->categoryNameCombo->setCurrentIndex(c);
        else ui->categoryNameCombo->setCurrentIndex(0);
        ui->categoryNameCombo->blockSignals(false);
        UpdateCategoryName();
    }
    else ui->categoryNameCombo->setEnabled(false);
    ui->dimNameEdit->setText(ui->dimNameCombo->currentText());
}

void DatasetEditor::UpdateCategoryName()
{
    ui->categoryNameEdit->setText(ui->categoryNameCombo->currentText());
}

void DatasetEditor::ShiftDimensions()
{
    int newDim = dimIndices.size();
    vector<fvec> samples = data->GetSamples();
    FOR(i, samples.size())
    {
        fvec s = samples[i];
        fvec nS(newDim);
        FOR(d, newDim) nS[d] = s[dimIndices[d]];
        samples[i] = nS;
    }
    data->SetSamples(samples);
    QStringList dimNames = canvas->dimNames;
    QStringList newDimNames;
    FOR(d, newDim)
    {
        int index = dimIndices[d];
        if(index < dimNames.size())
        {
            newDimNames << dimNames[index];
        }
    }
    canvas->dimNames = newDimNames;
    map<int, vector<string> > categorical, newCategorical;
    categorical = data->categorical;
    FOR(d, newDim)
    {
        int index = dimIndices[d];
        if(data->IsCategorical(index))
        {
            newCategorical[d] = categorical[index];
        }
    }
    data->categorical = newCategorical;
    emit(DataEdited());
    ResetDimensions();
}

void DatasetEditor::SwapDimensions()
{
    int dimCount = data->GetDimCount();
    int count = data->GetCount();
    vector<fvec> samples = data->GetSamples();
    ivec labels = data->GetLabels();
    vector<fvec> newSamples;
    ivec newLabels;
    fvec sample(count);
    FOR(d, dimCount)
    {
        FOR(i, count)
        {
            sample[i] = samples[d][i];
        }
        newSamples.push_back(sample);
        newLabels.push_back(0);
    }
    data->SetSamples(newSamples);
    data->SetLabels(labels);
    data->categorical.clear();
    canvas->dimNames.clear();
    canvas->classNames.clear();
    emit(DataEdited());
    ResetDimensions();
}

void DatasetEditor::ResetDimensions()
{
    int dimCount = data->GetDimCount();
    FOR(i, dimLabels.size())
    {
        ui->dimLabelsWidget->layout()->removeWidget(dimLabels[i]);
        delete dimLabels[i];
    }
    dimLabels.clear();
    FOR(i, dimCount)
    {
        QString dimensionName = QString("Dimension %1").arg(i+1);
        if(canvas->dimNames.size() > i) dimensionName = canvas->dimNames.at(i);
        QLabel *label = new QLabel(dimensionName);
        label->setStyleSheet(QString("QLabel { background-color : #%1%2%3; color : %4; qproperty-alignment: AlignCenter;}")
                             .arg((int)(i*255.f/(dimCount-1)),0,16)
                             .arg((int)(i*255.f/(dimCount-1)),0,16)
                             .arg((int)(i*255.f/(dimCount-1)),0,16)
                             .arg(i < dimCount / 2 ? "white" : "black"));
        label->installEventFilter(this);
        dimLabels.push_back(label);
        ((QGridLayout *)ui->dimLabelsWidget->layout())->addWidget(dimLabels.back(), i/4, i%4);
    }
}

void DatasetEditor::SetAsClass()
{
    vector<fvec> samples = data->GetSamples();
    int index = ui->dimNameCombo->currentIndex();
    ivec labels = data->GetLabels();
    ivec newLabels;
    std::map<int,QString> newClassNames;
    FOR(i, labels.size())
    {
        int label = (int)samples[i][index];
        newLabels.push_back(label);
    }
    FOR(i, samples.size())
    {
        samples[i][index] = labels[i];
    }
    data->SetSamples(samples);
    data->SetLabels(newLabels);
    if(canvas->dimNames.size()>index) canvas->dimNames[index] = "Class";

    vector<string> newCat;
    int cnt=0;
    FORIT(canvas->classNames, int, QString)
    {
        newCat[cnt++] = it->second.toStdString();
    }

    if(data->IsCategorical(index))
    {
        FOR(i, data->categorical[index].size()) newClassNames[i] = data->categorical[index][i].c_str();
    }
    canvas->classNames = newClassNames;
    data->categorical[index] = newCat;

    emit(DataEdited());
    ResetDimensions();
}

void DatasetEditor::ChangeClassName()
{
    int index = ui->classNameCombo->currentIndex();
    QString name = ui->classNameEdit->text();
    if(name.isEmpty()) name = QString("Class %1").arg(index);
    canvas->classNames[index] = name;
    ui->classNameCombo->setItemText(index, name);
    canvas->repaint();
}

void DatasetEditor::ChangeDimName()
{
    int index = ui->dimNameCombo->currentIndex();
    QString name = ui->dimNameEdit->text();
    if(name.isEmpty()) name = QString("Dimension %1").arg(index);
    if(index < canvas->dimNames.size()) canvas->dimNames[index] = name;
    else
    {
        while(canvas->dimNames.size() < index) canvas->dimNames.push_back(QString("Dimension %1").arg(canvas->dimNames.size()));
        canvas->dimNames.push_back(name);
    }
    ui->dimNameCombo->setItemText(index, name);
    canvas->repaint();
}

void DatasetEditor::ChangeCategoryName()
{
    int dimIndex = ui->dimNameCombo->currentIndex();
    int index = ui->categoryNameCombo->currentIndex();
    QString name = ui->categoryNameEdit->text();
    if(name.isEmpty()) name = QString ("%1").arg(index);
    if(data->categorical.count(dimIndex) && index < data->categorical[dimIndex].size()) data->categorical[dimIndex][index] = name.toStdString();
    ui->categoryNameCombo->setItemText(index, name);
    canvas->repaint();
}

