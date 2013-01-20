#include "gridsearch.h"
#include <QPixmap>
#include <QClipboard>
#include <basicMath.h>
#include "ui_gridsearch.h"

using namespace std;

GridLabel::GridLabel(QWidget *parent)
    : QLabel(parent)
{
    setMouseTracking(true);
    setCursor(Qt::CrossCursor);
}

void GridLabel::focusOutEvent(QFocusEvent *ev)
{
    qDebug() << "focus out";
    emit(MouseMove(new QMouseEvent(QEvent::MouseMove, QPoint(-1,-1),Qt::NoButton,Qt::NoButton,Qt::NoModifier)));
}

void GridLabel::mouseMoveEvent(QMouseEvent *event)
{
    emit(MouseMove(event));
}

GridSearch::GridSearch(Canvas *canvas, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GridSearch),
    canvas(canvas),
    mapX(0),mapY(0),
    classifier(0),
    clusterer(0),
    regressor(0),
    dynamical(0),
    avoider(0),
    maximizer(0),
    reinforcement(0),
    projector(0)
{
    ui->setupUi(this);
    connect(ui->names1Combo, SIGNAL(currentIndexChanged(int)), this, SLOT(OptionsChanged()));
    connect(ui->names2Combo, SIGNAL(currentIndexChanged(int)), this, SLOT(OptionsChanged()));
    connect(ui->runButton, SIGNAL(clicked()), this, SLOT(Run()));
    connect(ui->clipboardButton, SIGNAL(clicked()), this, SLOT(Clipboard()));
    connect(ui->displayLabel, SIGNAL(MouseMove(QMouseEvent*)), this, SLOT(MouseMove(QMouseEvent*)));
    connect(ui->colorCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(DisplayChanged()));
    connect(ui->resultCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(DisplayChanged()));
    ui->displayLabel->setScaledContents(true);
}

GridSearch::~GridSearch()
{
    delete ui;
}

void GridSearch::Clipboard()
{
    QClipboard *clipboard = QApplication::clipboard();
    QPixmap screenshot = pixmap;
    if(screenshot.isNull()) return;
    clipboard->setImage(screenshot.toImage());
    clipboard->setPixmap(screenshot);
}

void GridSearch::DisplayChanged()
{
    DisplayResults();
    repaint();
}

ivec toBinary(ivec labels)
{
    ivec newLabels(labels.size(), 1);
    map<int,int> class2labels;
    int cnt=0;
    FOR(i, labels.size())
    {
        if(!class2labels.count(labels[i])) class2labels[labels[i]] = cnt++;
    }
    if(class2labels.size() == 1) return newLabels;
    if(class2labels.count(0)) // we have a negative class!
    {
        FOR(i, labels.size()) if(labels[i] == 0) newLabels[i] = -1;
    }
    else // we pick the first one as the positive class
    {
        if(class2labels.count(1)) // we have a positive class!
        {
            FOR(i, labels.size()) if(labels[i] != 1) newLabels[i] = -1;
        }
        else
        {
            int posClass = labels[0];
            FOR(i, labels.size()) if(labels[i] != posClass) newLabels[i] = -1;
        }
    }
    return newLabels;
}

void GridSearch::MouseMove(QMouseEvent *event)
{
    mousePoint = QPoint(event->x(), event->y());
    if(map.size())
    {
        /*
        int x = event->x() * mapX / pixmap.width();
        int y = event->y() * mapY / pixmap.height();
        float value = map[x + y * mapX];
        qDebug() << "value" << x << y << value;
        */
        repaint();
    }
}

void GridSearch::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    if(pixmap.isNull()) return;
    if(mousePoint.x() == -1 && mousePoint.y() == -1)
    {
        ui->displayLabel->setPixmap(pixmap);
        return;
    }
    int W = pixmap.width();
    int H = pixmap.height();
    int x = mousePoint.x() * mapX / W;
    int y = mousePoint.y() * mapY / H;
    int pX = (x+0.5)*W/mapX;
    int pY = (y+0.5)*H/mapY;
    int rW = 100;
    int rH = 40;
    int rX = pX + 5;
    int rY = pY + 5;
    if(rX + rW > W) rX = pX - rW - 5;
    if(rY + rH > H) rY = pY - rH - 5;

    int index1 = ui->names1Combo->currentIndex();
    int index2 = ui->names2Combo->currentIndex();
    QString name1 = index1 < names.size() ? names[index1] : "none";
    QString name2 = index2 < names.size() ? names[index2] : "none";
    fPair ranges = GetParamsRange();
    float param1 = index1 < names.size() ? (x/(float)(mapX-1))*(ranges.first.second - ranges.first.first) + ranges.first.first : 0;
    float param2 = index2 < names.size() ? (y/(float)(mapY-1))*(ranges.second.second - ranges.second.first) + ranges.second.first : 0;
    float value = map.size() && mapX && mapY ? map[x+y*mapX] : 0;
    QString displayString;
    if(index1 < names.size())
    {
        if(types[index1] == "List") displayString += QString("%1:%2\n").arg(name1).arg(values[index1][x]);
        else displayString += QString("%1:%2\n").arg(name1).arg(param1);
    }
    else displayString += QString("None\n");
    if(index2 < names.size())
    {
        if(types[index2] == "List") displayString += QString("%1:%2\n").arg(name2).arg(values[index2][y]);
        else displayString += QString("%1:%2\n").arg(name2).arg(param2);
    }
    else displayString += QString("None\n");
    displayString += QString("%1: %2").arg(ui->resultCombo->currentText()).arg(value, 0, 'f', 4);

    QPixmap newPixmap = pixmap;
    QPainter painter(&newPixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setOpacity(0.6);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::white);
    painter.drawRect(rX-2, rY-2, rW+4, rH+4);
    painter.setOpacity(1);
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(Qt::white, 1));
    painter.drawRect(rX-2, rY-2, rW+4, rH+4);
    painter.setPen(QPen(Qt::white, 3));
    painter.drawEllipse(QPoint(pX, pY), 3, 3);
    painter.setPen(QPen(Qt::black, 1));
    QFont font = painter.font();
    font.setPointSize(9);
    painter.setFont(font);
    painter.drawText(rX, rY, rW, rH, Qt::AlignVCenter|Qt::AlignLeft,displayString);
    ui->displayLabel->setPixmap(newPixmap);
}

void GridSearch::DisplayResults()
{
    int colorScheme = ui->colorCombo->currentIndex();
    int index = ui->resultCombo->currentIndex();
    if(index == -1) return;
    map = mapList[ui->resultCombo->currentText()];

    float minVal = FLT_MAX, maxVal = -FLT_MAX;
    FOR(i, map.size())
    {
        minVal = min(minVal, map[i]);
        maxVal = max(maxVal, map[i]);
    }
    if(minVal == maxVal)
    {
        minVal = 0;
        maxVal = 1;
    }

    int xSteps = mapX;
    int ySteps = mapY;
    QImage tinyMap(xSteps, ySteps, QImage::Format_RGB32);
    FOR(y, ySteps)
    {
        FOR(x, xSteps)
        {
            float v = (map[x+y*xSteps]-minVal)/(maxVal-minVal);
            QRgb color = Canvas::GetColorMapValue(v, colorScheme);
            tinyMap.setPixel(x,y,color);
        }
    }
    int W = ui->displayLabel->width();
    int H = ui->displayLabel->height();

    pixmap = QPixmap::fromImage(tinyMap.scaled(W,H,Qt::IgnoreAspectRatio, Qt::FastTransformation));
    QPainter painter(&pixmap);
    painter.setPen(QPen(Qt::black, 0.5f));
    FOR(y, ySteps+1)
    {
        int h = y*pixmap.height()/(ySteps);
        if(y == ySteps) h = pixmap.height()-1;
        painter.drawLine(0,h, pixmap.width(), h);
    }
    FOR(x, xSteps+1)
    {
        int w = x*pixmap.width()/(xSteps);
        if(x == xSteps) w = pixmap.width()-1;
        painter.drawLine(w, 0, w, pixmap.height());
    }

    ui->displayLabel->setPixmap(pixmap);
    ui->displayLabel->repaint();
}

fPair GridSearch::GetParamsRange()
{
    int xSteps = ui->steps1Spin->value();
    int ySteps = ui->steps2Spin->value();
    int xIndex = ui->names1Combo->currentIndex();
    int yIndex = ui->names2Combo->currentIndex();
    bool bNone1 = xIndex == ui->names1Combo->count()-1;
    bool bNone2 = yIndex == ui->names2Combo->count()-1;
    float xMin=0, xMax=0, yMin=0, yMax=0;
    if(bNone1) xSteps = 1; // none!
    else
    {
        if(types[xIndex] == "List")
        {
            xMin = 0; xMax = values[xIndex].size();
        }
        else
        {
            xMin = ui->start1Spin->value();
            xMax = ui->stop1Spin->value();
        }
        if(types[xIndex] == "Integer") xSteps = min(xSteps, (int)(xMax-xMin)+1);
    }
    if(bNone2) ySteps = 1; // none!
    else
    {
        if(types[yIndex] == "List")
        {
            yMin = 0; yMax = values[yIndex].size();
        }
        else
        {
            yMin = ui->start2Spin->value();
            yMax = ui->stop2Spin->value();
        }
        if(types[yIndex] == "Integer") ySteps = min(ySteps, (int)(yMax-yMin)+1);
    }
    if(xMin==xMax) xSteps = 1;
    if(yMin==yMax) ySteps = 1;
    fPair ranges;
    ranges.first = make_pair(xMin,xMax);
    ranges.second = make_pair(yMin,yMax);
    return ranges;
}

void GridSearch::Run()
{
    mapList.clear();
    int xSteps = ui->steps1Spin->value();
    int ySteps = ui->steps2Spin->value();
    int xIndex = ui->names1Combo->currentIndex();
    int yIndex = ui->names2Combo->currentIndex();
    bool bNone1 = xIndex == ui->names1Combo->count()-1;
    bool bNone2 = yIndex == ui->names2Combo->count()-1;
    if(bNone1 && bNone2) return;
    float xMin=0, xMax=0, yMin=0, yMax=0;
    if(bNone1) xSteps = 1; // none!
    else
    {
        if(types[xIndex] == "List")
        {
            xMin = 0; xMax = values[xIndex].size();
        }
        else
        {
            xMin = ui->start1Spin->value();
            xMax = ui->stop1Spin->value();
        }
        if(types[xIndex] == "Integer") xSteps = min(xSteps, (int)(xMax-xMin)+1);
    }
    if(bNone2) ySteps = 1; // none!
    else
    {
        if(types[yIndex] == "List")
        {
            yMin = 0; yMax = values[yIndex].size();
        }
        else
        {
            yMin = ui->start2Spin->value();
            yMax = ui->stop2Spin->value();
        }
        if(types[yIndex] == "Integer") ySteps = min(ySteps, (int)(yMax-yMin)+1);
    }
    if(xMin==xMax) xSteps = 1;
    if(yMin==yMax) ySteps = 1;
    int folds = ui->foldSpin->value();
    fvec oldParams;
    if(classifier) oldParams = classifier->GetParams();
    if(clusterer) oldParams = clusterer->GetParams();
    if(regressor) oldParams = regressor->GetParams();
    if(dynamical) oldParams = dynamical->GetParams();
    if(avoider) oldParams = avoider->GetParams();
    if(maximizer) oldParams = maximizer->GetParams();
    fvec params = oldParams;
    float trainRatio = 0.66;
    vector<fvec> samples = canvas->data->GetSamples();
    ivec labels = canvas->data->GetLabels();
    ivec binLabels = toBinary(labels);
    fvec errorMap(xSteps*ySteps);
    fvec fmeasureMap(xSteps*ySteps);
    FOR(y, ySteps)
    {
        FOR(x, xSteps)
        {
            if(!bNone1) params[xIndex] = x / (float) (xSteps-1) * (xMax - xMin) + xMin;
            if(!bNone2) params[yIndex] = y / (float) (ySteps-1) * (yMax - yMin) + yMin;
            int trainCount = (int)(trainRatio * samples.size());
            u32 *perm = randPerm(samples.size());
            vector<fvec> trainSamples(trainCount);
            ivec trainLabels(trainCount);
            ivec trainBinLabels(trainCount);
            vector<fvec> testSamples(samples.size()-trainCount);
            ivec testLabels(samples.size()-trainCount);
            ivec testBinLabels(samples.size()-trainCount);
            fvec measure1(folds, 0);
            fvec measure2(folds, 0);
            FOR(f, folds)
            {
                int foldOffset = (f*samples.size()/folds);
                FOR(i, samples.size())
                {
                    if(i < trainCount)
                    {
                        trainSamples[i] = samples[perm[(foldOffset + i) % samples.size()]];
                        trainLabels[i] = labels[perm[(foldOffset + i) % labels.size()]];
                        trainBinLabels[i] = binLabels[perm[(foldOffset + i) % labels.size()]];
                    }
                    else
                    {
                        testSamples[i-trainCount] = samples[perm[(foldOffset + i) % samples.size()]];
                        testLabels[i-trainCount] = labels[perm[(foldOffset + i) % labels.size()]];
                        testBinLabels[i-trainCount] = binLabels[perm[(foldOffset + i) % labels.size()]];
                    }
                }

                if(classifier)
                {
                    if(!samples.size()) return;
                    Classifier *c = classifier->GetClassifier();
                    classifier->SetParams(c, params);
                    if(c->IsMultiClass()) c->Train(trainSamples, trainLabels);
                    else c->Train(trainSamples, trainBinLabels);
                    float error=0, invError=0;
                    bool bBinary = false;
                    rocData rocdata;
                    FOR(i, testSamples.size())
                    {
                        if(c->IsMultiClass())
                        {
                            fvec res = c->TestMulti(testSamples[i]);
                            if(res.size() == 1)
                            {
                                bBinary = true;
                                // we use invError because we don't know in which order the classifier
                                // has learned the classes, and which has become the de facto positive class
                                if(res[0] * testBinLabels[i] < 0) error += 1.f;
                                else invError += 1.f;
                                rocdata.push_back(f32pair(res[0], (testBinLabels[i]+1)/2));
                            }
                            else
                            {

                                int winner = 0;
                                float score = res[0];
                                FOR(j, res.size())
                                {
                                    if(res[j] > score)
                                    {
                                        score = res[j];
                                        winner = j;
                                    }
                                }
                                if(winner != testLabels[i]) error += 1.f;
                                rocdata.push_back(f32pair(winner, testLabels[i]));
                            }
                        }
                        else
                        {
                            bBinary = true;
                            float res = c->Test(testSamples[i]);
                            if(res * testBinLabels[i] < 0) error += 1.f;
                            else invError += 1.f;
                            rocdata.push_back(f32pair(res, (testBinLabels[i]+1)/2));
                        }
                    }
                    rocdata = FixRocData(rocdata);
                    if(bBinary) error = min(error, invError);
                    error /= testSamples.size();
                    measure1[f] = error;
                    // we use micro f-measure for multi-class
                    float eff = bBinary ? GetRocValueAt(rocdata, 0) : GetMicroMacroFMeasure(rocdata).first;
                    measure2[f] = eff;
                    DEL(c);
                }
                else if(clusterer);
                else if(regressor)
                {
                    if(!samples.size()) return;
                    Regressor *r = regressor->GetRegressor();
                    regressor->SetParams(r, params);
                    int outputDim = samples[0].size()-1;
                    r->SetOutputDim(outputDim);
                    r->Train(trainSamples, trainLabels);
                    float error = 0;
                    FOR(i, testSamples.size())
                    {
                        fvec res = r->Test(testSamples[i]);
                        // we compute the mse
                        error += sqrtf((res[0] - trainSamples[i][outputDim])*(res[0] - trainSamples[i][outputDim]));
                    }
                    error /= testSamples.size();
                    measure1[f] = error;
                }
                else if(dynamical);
                else if(avoider);
                else if(maximizer);
                else if(reinforcement);
                else if(projector);
            }
            KILL(perm);
            // now we fill the error map
            float mean = 0, effMean = 0;
            FOR(f, folds)
            {
                mean += measure1[f];
                effMean += measure2[f];
            }
            mean /= folds;
            effMean /= folds;
            qDebug() << "mean" << mean << "fmeasure" << effMean;
            errorMap[x+y*xSteps] = mean;
            fmeasureMap[x+y*xSteps] = effMean;
        }
    }
    if(classifier)
    {
        mapList["Error"] = errorMap;
        mapList["FMeasure"] = fmeasureMap;
        map = errorMap;
    }
    else if(regressor)
    {
        mapList["Error"] = errorMap;
    }
    bool bSig = ui->resultCombo->blockSignals(true);
    ui->resultCombo->clear();
    for(std::map<QString,fvec>::iterator it = mapList.begin(); it != mapList.end(); it++)
    {
        ui->resultCombo->addItem(it->first);
    }
    ui->resultCombo->setCurrentIndex(0);
    ui->resultCombo->blockSignals(bSig);
    mapX = xSteps;
    mapY = ySteps;
    DisplayResults();
    repaint();
}

void GridSearch::OptionsChanged()
{
    int index1 = ui->names1Combo->currentIndex();
    int index2 = ui->names2Combo->currentIndex();
    if(index1 < types.size())
    {
        bool bList = types[index1] == "List";
        ui->start1Spin->setEnabled(!bList);
        ui->stop1Spin->setEnabled(!bList);
        ui->steps1Spin->setEnabled(!bList);
        float start=0, stop=99999;
        if(!bList)
        {
            if(values[index1].size() > 0) start = values[index1][0].toFloat();
            if(values[index1].size() > 1) stop = values[index1][1].toFloat();
            if(ui->start1Spin->value() < start) ui->start1Spin->setValue(start);
            if(ui->stop1Spin->value() < ui->start1Spin->value()) ui->stop1Spin->setValue(ui->start1Spin->value()+1);
            if(ui->stop1Spin->value() > stop) ui->stop1Spin->setValue(stop);
            ui->steps1Spin->setValue(10);
            if(types[index1] == "Integer")
            {
                ui->start1Spin->setSingleStep(1);
                ui->stop1Spin->setSingleStep(1);
                ui->start1Spin->setDecimals(0);
                ui->stop1Spin->setDecimals(0);
            }
            else if(types[index1] == "Real")
            {
                ui->start1Spin->setSingleStep((stop-start)*0.001);
                ui->stop1Spin->setSingleStep((stop-start)*0.001);
                ui->start1Spin->setDecimals((int)(log((stop-start)*0.001f)/log(10.f)));
                ui->stop1Spin->setDecimals((int)(log((stop-start)*0.001f)/log(10.f)));
            }
        }
        else ui->steps1Spin->setValue(values[index1].size());
    }
    else
    {
        ui->start1Spin->setEnabled(false);
        ui->stop1Spin->setEnabled(false);
        ui->steps1Spin->setEnabled(false);
    }
    if(index2 < types.size())
    {
        bool bList = types[index2] == "List";
        ui->start2Spin->setEnabled(!bList);
        ui->stop2Spin->setEnabled(!bList);
        ui->steps2Spin->setEnabled(!bList);
        float start=0, stop=99999;
        if(!bList)
        {
            if(values[index2].size() > 0) start = values[index2][0].toFloat();
            if(values[index2].size() > 1) stop = values[index2][1].toFloat();
            if(ui->start2Spin->value() < start) ui->start2Spin->setValue(start);
            if(ui->stop2Spin->value() < ui->start2Spin->value()) ui->stop2Spin->setValue(ui->start2Spin->value()+1);
            if(ui->stop2Spin->value() > stop) ui->stop2Spin->setValue(stop);
            ui->steps2Spin->setValue(10);
            if(types[index2] == "Integer")
            {
                ui->start2Spin->setSingleStep(1);
                ui->stop2Spin->setSingleStep(1);
            }
            else if(types[index2] == "Real")
            {
                ui->start2Spin->setSingleStep((stop-start)*0.001);
                ui->stop2Spin->setSingleStep((stop-start)*0.001);
                ui->start2Spin->setDecimals((int)(log((stop-start)*0.001f)/log(10.f)));
                ui->stop2Spin->setDecimals((int)(log((stop-start)*0.001f)/log(10.f)));
            }
        }
        else ui->steps2Spin->setValue(values[index2].size());
    }
    else
    {
        ui->start2Spin->setEnabled(false);
        ui->stop2Spin->setEnabled(false);
        ui->steps2Spin->setEnabled(false);
    }
}

void GridSearch::Update()
{
    names.clear();
    types.clear();
    values.clear();
    if(classifier)
        classifier->GetParameterList(names, types, values);
    if(clusterer)
        clusterer->GetParameterList(names, types, values);
    if(regressor)
        regressor->GetParameterList(names, types, values);
    if(dynamical)
        dynamical->GetParameterList(names, types, values);
    if(avoider)
        avoider->GetParameterList(names, types, values);
    if(maximizer)
        maximizer->GetParameterList(names, types, values);
    if(reinforcement)
        reinforcement->GetParameterList(names, types, values);
    if(projector)
        projector->GetParameterList(names, types, values);
    ui->names1Combo->clear();
    ui->names2Combo->clear();
    FOR(i, names.size())
    {
        ui->names1Combo->addItem(names[i]);
        ui->names2Combo->addItem(names[i]);
    }
    ui->names1Combo->addItem("None");
    ui->names2Combo->addItem("None");
    if(names.size() > 0) ui->names1Combo->setCurrentIndex(0);
    if(names.size() > 1) ui->names2Combo->setCurrentIndex(1);
}

void GridSearch::SetClassifier(ClassifierInterface *c)
{
    if(classifier == c) return;
    classifier = c;
    clusterer = 0;
    regressor = 0;
    dynamical = 0;
    avoider = 0;
    maximizer = 0;
    reinforcement = 0;
    projector = 0;
    pixmap = QPixmap();
    mousePoint = QPoint(-1,-1);
    Update();
}

void GridSearch::SetClusterer(ClustererInterface *c)
{
    if(clusterer == c) return;
    classifier = 0;
    clusterer = c;
    regressor = 0;
    dynamical = 0;
    avoider = 0;
    maximizer = 0;
    reinforcement = 0;
    projector = 0;
    pixmap = QPixmap();
    mousePoint = QPoint(-1,-1);
    Update();
}

void GridSearch::SetRegressor(RegressorInterface *c)
{
    if(regressor == c) return;
    classifier = 0;
    clusterer = 0;
    regressor = c;
    dynamical = 0;
    avoider = 0;
    maximizer = 0;
    reinforcement = 0;
    projector = 0;
    pixmap = QPixmap();
    mousePoint = QPoint(-1,-1);
    Update();
}

void GridSearch::SetDynamical(DynamicalInterface *c)
{
    if(dynamical == c) return;
    classifier = 0;
    clusterer = 0;
    regressor = 0;
    dynamical = c;
    avoider = 0;
    maximizer = 0;
    reinforcement = 0;
    projector = 0;
    pixmap = QPixmap();
    mousePoint = QPoint(-1,-1);
    Update();
}

void GridSearch::SetAvoidance(AvoidanceInterface *c)
{
    if(avoider == c) return;
    classifier = 0;
    clusterer = 0;
    regressor = 0;
    dynamical = 0;
    avoider = c;
    maximizer = 0;
    reinforcement = 0;
    projector = 0;
    pixmap = QPixmap();
    mousePoint = QPoint(-1,-1);
    Update();
}

void GridSearch::SetMaximizer(MaximizeInterface *c)
{
    if(maximizer == c) return;
    classifier = 0;
    clusterer = 0;
    regressor = 0;
    dynamical = 0;
    avoider = 0;
    maximizer = c;
    reinforcement = 0;
    projector = 0;
    pixmap = QPixmap();
    mousePoint = QPoint(-1,-1);
    Update();
}

void GridSearch::SetReinforcement(ReinforcementInterface *c)
{
    if(reinforcement == c) return;
    classifier = 0;
    clusterer = 0;
    regressor = 0;
    dynamical = 0;
    avoider = 0;
    maximizer = 0;
    reinforcement = c;
    projector = 0;
    pixmap = QPixmap();
    mousePoint = QPoint(-1,-1);
    Update();
}

void GridSearch::SetProjector(ProjectorInterface *c)
{
    if(projector == c) return;
    classifier = 0;
    clusterer = 0;
    regressor = 0;
    dynamical = 0;
    avoider = 0;
    maximizer = 0;
    reinforcement = 0;
    projector = c;
    pixmap = QPixmap();
    mousePoint = QPoint(-1,-1);
    Update();
}
