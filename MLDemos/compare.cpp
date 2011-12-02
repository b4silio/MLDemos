#include "compare.h"
#include <QtGui>

using namespace std;

CompareAlgorithms::CompareAlgorithms(QWidget *parent)
	: display(0)
{
	compareDisplay = new Ui::CompareDisplay();
    compareDisplay->setupUi(compareWidget = new QWidget(parent));
	if(!parent) compareWidget->setWindowTitle("Comparison Results");
	else
	{
		parent->layout()->setContentsMargins(12,0,0,0);
		parent->layout()->setSpacing(0);
		parent->layout()->addWidget(compareWidget);
		compareWidget->layout()->setContentsMargins(0,0,0,0);
	}
	display = compareDisplay->display;
	connect(compareDisplay->resultCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(Update()));
	connect(compareDisplay->displayTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(Update()));
}

CompareAlgorithms::~CompareAlgorithms()
{
	DEL(compareDisplay);
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
	QBitmap bitmap;
	bitmap.clear();
	pixmap.setMask(bitmap);
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
	compareWidget->show();
}
