#include "compare.h"
#include <QtGui>

using namespace std;

CompareAlgorithms::CompareAlgorithms(Canvas *canvas)
    : display(0), canvas(canvas)
{
    params = new Ui::optionsCompare();
    params->setupUi(paramsWidget = new BaseWidget());

	compareDisplay = new Ui::CompareDisplay();
    compareDisplay->setupUi(compareWidget = new BaseWidget(params->resultWidget));
    paramsWidget->setWindowTitle("Comparison Results");

    if (!params->resultWidget->layout() ) {
        params->resultWidget->setLayout(new QHBoxLayout());
    }
    params->resultWidget->layout()->setContentsMargins(12,0,0,0);
    params->resultWidget->layout()->setSpacing(0);
    params->resultWidget->layout()->addWidget(compareWidget);
    compareWidget->layout()->setContentsMargins(0,0,0,0);

    display = compareDisplay->display;
	connect(compareDisplay->resultCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(Update()));
	connect(compareDisplay->displayTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(Update()));

    params->datasetADisplay->setScaledContents(true);
    params->datasetBDisplay->setScaledContents(true);
    params->datasetADisplay->installEventFilter(this);
    params->datasetBDisplay->installEventFilter(this);

    connect(params->screenshotButton, SIGNAL(clicked()), this, SLOT(CompareScreenshot()));
    connect(params->clearButton, SIGNAL(clicked()), this, SLOT(CompareClear()));
    connect(params->removeButton, SIGNAL(clicked()), this, SLOT(CompareRemove()));
    connect(params->saveButton, SIGNAL(clicked()), this, SLOT(CompareSave()));
    connect(params->loadButton, SIGNAL(clicked()), this, SLOT(CompareLoad()));
    connect(params->datasetAButton, SIGNAL(clicked()), this, SLOT(CompareSetTrain()));
    connect(params->datasetBButton, SIGNAL(clicked()), this, SLOT(CompareSetTest()));

}

CompareAlgorithms::~CompareAlgorithms()
{
	DEL(compareDisplay);
    DEL(compareWidget);
    DEL(params);
    DEL(paramsWidget);
}

bool CompareAlgorithms::eventFilter(QObject *obj, QEvent *event)
{
    if (params &&  obj == params->datasetADisplay && event->type() == QEvent::MouseButtonDblClick) {
        CompareResetTrain();
        return true;
    }
    if (params && obj == params->datasetBDisplay && event->type() == QEvent::MouseButtonDblClick) {
        CompareResetTest();
        return true;
    }
    return QObject::eventFilter(obj, event);;
}

void CompareAlgorithms::Clear()
{
	results.clear();
	algorithms.clear();
	compareDisplay->resultCombo->clear();
}

void CompareAlgorithms::SetActiveResult(int index)
{
    if(index >= compareDisplay->resultCombo->count()) return;
    compareDisplay->resultCombo->setCurrentIndex(index);
}

void CompareAlgorithms::AddResults(fvec result, QString name, QString algorithm)
{
	if(!result.size()) return;
	results[name].push_back(result);
	algorithms[name].push_back(algorithm);
	bool exists = false;
	FOR(i, compareDisplay->resultCombo->count())
	{
		if(name == compareDisplay->resultCombo->itemText(i))
		{
			exists = true;
			break;
		}
	}
	if(!exists) compareDisplay->resultCombo->addItem(name);
}

void CompareAlgorithms::Update()
{
	int displayType = compareDisplay->displayTypeCombo->currentIndex();
	QString name = compareDisplay->resultCombo->currentText();
	vector<fvec> result = results[name];
	QStringList names = algorithms[name];
	if(!names.size() || !results.size()) return;

	int rW = 150;
	int hPad = 20;
	QSize boxSize(rW*names.size(),200);

	pixmap = QPixmap(boxSize.width(),boxSize.height()+2*hPad);
    //QBitmap bitmap;
    //bitmap.clear();
    //pixmap.setMask(bitmap);
	pixmap.fill(Qt::transparent);
	QPainter painter(&pixmap);

	//painter.setFont(QFont("Lucida Grande", 12));
	painter.setPen(QPen(Qt::black, 0.5));
	painter.setBrush(Qt::NoBrush);
	FOR(i, result.size())
	{
		QRect rect(boxSize.width() * i/ result.size(), 0, rW, hPad);
		painter.drawText(rect, Qt::AlignHCenter | Qt::AlignTop, names[i]);
	}

    switch(displayType)
    {
    case 0:
    {
		QPixmap box = BoxPlot(result, boxSize);
		painter.drawPixmap(0,2*hPad,box.width(), box.height(), box);
	}
        break;
    case 1:
	{
		QPixmap box = Histogram(result, boxSize);
		painter.drawPixmap(0,2*hPad,box.width(), box.height(), box);
	}
        break;
    case 2:
        QPixmap box = RawData(result, boxSize);
        painter.drawPixmap(0,2*hPad,box.width(), box.height(), box);
        break;
    }
	display->setPixmap(pixmap);
}

QString CompareAlgorithms::ToString()
{
    QString text;
    FOR(x, compareDisplay->resultCombo->count())
    {
        QString name = compareDisplay->resultCombo->itemText(x);
        vector<fvec> result = results[name];
        QStringList names = algorithms[name];
        if(!names.size() || !results.size()) return text;
        text += name + "\n";
        FOR(n, names.size())
        {
            fvec& data = result[n];
            int nanCount = 0;
            FOR(i, data.size()) if(data[i] != data[i]) nanCount++;
            float mean = 0;
            float sigma = 0;
            FOR(i, data.size()) if(data[i]==data[i]) mean += data[i] / (data.size()-nanCount);
            FOR(i, data.size()) if(data[i]==data[i]) sigma += powf(data[i]-mean,2);
            sigma = sqrtf(sigma/(data.size()-nanCount));

            text += names[n] + "\t";
            text += QString("%1\t%2").arg(mean, 0, 'f', 4).arg(sigma, 0, 'f', 4);
            text += "\n";
        }
        text += "\n";
    }
    return text;
}

void CompareAlgorithms::Show()
{
	Update();
    paramsWidget->show();
}

void CompareAlgorithms::Add(QString parameterData, QString displayString)
{
    params->algoList->addItem(displayString);
    compareOptions.push_back(parameterData);
}

void CompareAlgorithms::CompareScreenshot()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setImage(pixmap.toImage());
    clipboard->setText(ToString());
}

void CompareAlgorithms::CompareClear()
{
    params->algoList->clear();
    compareOptions.clear();
}

void CompareAlgorithms::CompareRemove()
{
    int offset = 0;
    FOR (i, params->algoList->count()) {
        if (params->algoList->item(i)->isSelected()) {
            compareOptions.removeAt(i-offset);
            offset++;
        }
    }
    QList<QListWidgetItem *> selected = params->algoList->selectedItems();
    FOR (i, selected.size()) delete selected[i];
    if (params->algoList->count()) params->algoList->item(0)->setSelected(true);
}

void CompareAlgorithms::CompareSave()
{
    QString filename = QFileDialog::getSaveFileName(0, "Save Compare List", QString(), tr("Compare List (*.mlcomp)"));
    if (!filename.endsWith(".mlcomp")) filename += QString(".mlcomp");
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Cannot open file for saving";
        return;
    }
    QTextStream out(&file);
    FOR (i, compareOptions.size()) {
        out << compareOptions[i] << "[STOP]\n";
        out << params->algoList->item(i)->text() << "\n";
    }
    file.close();
    qDebug() << "Compare list saved successfully";
}

void CompareAlgorithms::CompareLoad()
{
    QString filename = QFileDialog::getOpenFileName(0, "Load Compare List", QString(), tr("Compare List (*.mlcomp)"));
    if (!filename.endsWith(".mlcomp")) filename += QString(".mlcomp");
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Cannot open file for loading";
        return;
    }
    params->algoList->clear();
    compareOptions.clear();
    QTextStream in(&file);
    QString parameterString = "";
    while (!in.atEnd()) {
        QString text = in.readLine();
        if (text.endsWith("[STOP]")) {
            compareOptions.push_back(parameterString);
            parameterString = "";
            QString textString = in.readLine();
            params->algoList->addItem(textString);
        } else {
            parameterString += text + "\n";
        }
    }
    file.close();
    qDebug() << "Compare list loaded successfully";
}

void CompareAlgorithms::CompareSetTrain()
{
    datasetA = canvas->data->GetSamples();
    labelsA = canvas->data->GetLabels();
    int xIndex=canvas->xIndex, yIndex=canvas->yIndex;
    int s=50;
    QPixmap pixmap(s,s);
    if (!datasetA.size()) {
        pixmap.fill(Qt::transparent);
        params->datasetADisplay->setPixmap(pixmap);
        return;
    }
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    float minx=FLT_MAX, miny=FLT_MAX, maxx=-FLT_MAX, maxy=-FLT_MAX;
    FOR (i, datasetA.size()) {
        minx = min(minx,datasetA[i][xIndex]);
        maxx = max(maxx,datasetA[i][xIndex]);
        miny = min(miny,datasetA[i][yIndex]);
        maxy = max(maxy,datasetA[i][yIndex]);
    }
    FOR (i, datasetA.size()) {
        float x = (datasetA[i][xIndex]-minx)/(maxx-minx)*(s-4);
        float y = (datasetA[i][yIndex]-miny)/(maxy-miny)*(s-4);
        painter.setBrush(SampleColor[labelsA[i]%SampleColorCnt]);
        painter.drawEllipse(QPointF(x+2,y+2), 2, 2);
    }
    params->datasetADisplay->setPixmap(pixmap);
}

void CompareAlgorithms::CompareSetTest()
{
    datasetB = canvas->data->GetSamples();
    labelsB = canvas->data->GetLabels();
    int xIndex=canvas->xIndex, yIndex=canvas->yIndex;
    int s=50;
    QPixmap pixmap(s,s);
    if (!datasetB.size()) {
        pixmap.fill(Qt::transparent);
        params->datasetBDisplay->setPixmap(pixmap);
        return;
    }
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    float minx=FLT_MAX, miny=FLT_MAX, maxx=-FLT_MAX, maxy=-FLT_MAX;
    FOR (i, datasetB.size()) {
        minx = min(minx,datasetB[i][xIndex]);
        maxx = max(maxx,datasetB[i][xIndex]);
        miny = min(miny,datasetB[i][yIndex]);
        maxy = max(maxy,datasetB[i][yIndex]);
    }
    FOR (i, datasetB.size()) {
        float x = (datasetB[i][xIndex]-minx)/(maxx-minx)*(s-4);
        float y = (datasetB[i][yIndex]-miny)/(maxy-miny)*(s-4);
        painter.setBrush(SampleColor[labelsB[i]%SampleColorCnt]);
        painter.drawEllipse(QPointF(x+2,y+2), 2, 2);
    }
    params->datasetBDisplay->setPixmap(pixmap);
}

void CompareAlgorithms::CompareResetTrain()
{
    datasetA.clear();
    labelsA.clear();
    QPixmap pixmap(50,50);
    pixmap.fill(Qt::transparent);
    params->datasetADisplay->setPixmap(pixmap);
    params->datasetADisplay->setText("click\nto\nreset");
}

void CompareAlgorithms::CompareResetTest()
{
    datasetB.clear();
    labelsB.clear();
    QPixmap pixmap(50,50);
    pixmap.fill(Qt::transparent);
    params->datasetBDisplay->setPixmap(pixmap);
    params->datasetBDisplay->setText("click\nto\nreset");
}
