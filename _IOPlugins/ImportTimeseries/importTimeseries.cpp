/*********************************************************************
MLDemos: A User-Friendly visualization toolkit for machine learning
Copyright (C) 2011 Chrstophe Paccolat
Contact: mldemos@b4silio.com

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public License,
version 3 as published by the Free Software Foundation.

This library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free
Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*********************************************************************/
#include "fileUtils.h"
#include "importTimeseries.h"

Q_EXPORT_PLUGIN2(IO_ImportTimeseries, ImportTimeseries)

using namespace std;

ImportTimeseries::ImportTimeseries()
: guiDialog(0)
{
}

ImportTimeseries::~ImportTimeseries()
{
    if(gui && guiDialog) guiDialog->hide();
    DEL(inputParser);
}

void ImportTimeseries::Start()
{
	gui = new Ui::ImportTimeseriesDialog();
    gui->setupUi(guiDialog = new QDialog());
	guiDialog->setAcceptDrops(true);
    inputParser = new CSVParser();
	connect(gui->closeButton, SIGNAL(clicked()), this, SLOT(Close()));
	connect(guiDialog, SIGNAL(finished(int)), this, SLOT(Close()));
	connect(gui->loadButton, SIGNAL(clicked()), this, SLOT(LoadFolder()));
    guiDialog->show();
}

void ImportTimeseries::Stop()
{
    guiDialog->hide();
}

void ImportTimeseries::Close()
{
    emit(Done(this));
}

void ImportTimeseries::LoadFolder()
{
	QString folder = QFileDialog::getExistingDirectory(NULL, tr("Import Folder"), QDir::currentPath());
	Update(folder);
}

void ImportTimeseries::Update(QString folderName)
{
	if(folderName.isEmpty()) return;
	FileUtils fu(folderName);
	QStringList list = fu.GetFiles("", "txt");
	if(!list.size()) return;

	vector<TimeSerie> series;
	vector<string> names;
	map<string,TimeSerie> seriesMap;
	map<long,long> timeStamps;
	int dim=0, count=0, frames=0;
	long timeZero = 0;
	long timeCounter = 0;
	FOR(i, list.size())
	{
		CSVParser parser;
		parser.parse(list[i].toStdString().c_str());
		vector<vector<string> > data = parser.Data();
		if(!data.size() || !data[0].size()) continue;
		if(!dim)
		{
			dim = data[0].size()-3;
			timeZero = atof(data[0][0].c_str());
			count = data.size();
			frames = list.size();
			names.resize(count);
			qDebug() << " dim: " << dim << " count: " << count << " frames: " << frames;
			fvec datum(dim,0);
			FOR(j, count)
			{
				qDebug() << "name " << j << ": " << data[j][1].c_str();
				string name = data[j][1];
				names[j] = name;
				TimeSerie t;
				t.name = name;
				t.timestamps.resize(frames,-1);
//				FOR(f, t.timestamps.size()) t.timestamps[f] = f;
				t.data.resize(frames);
				FOR(f, t.data.size()) t[f] = datum;
				seriesMap[name] = t;
			}
		}
		long timestamp = atoi(data[0][0].c_str()) - timeZero;
		timeStamps[timestamp] = timeCounter++;
		FOR(j, data.size())
		{
			string name = data[j][1];
			if(!seriesMap.count(name))
			{
				TimeSerie t;
				t.name = name;
				t.timestamps.resize(frames,-1);
//				FOR(f, t.timestamps.size()) t.timestamps[f] = f;
				t.data.resize(frames);
				fvec datum(dim,0);
				FOR(f, t.data.size()) t[f] = datum;
				seriesMap[name] = t;
			}
			seriesMap[name].timestamps[i] = i;
			seriesMap[name][i] = fvec(dim,0);
			FOR(d, dim)
			{
				seriesMap[name][i][d] = atof(data[j][2+d].c_str()) * 0.01f;
				//qDebug() << j << " " << i << " " << d << series[j][i][d];
			}
		}
	}
	series.resize(seriesMap.size());
	int cnt=0;
	for(map<string,TimeSerie>::iterator it = seriesMap.begin(); it != seriesMap.end(); it++)
	{
		TimeSerie& t = series[cnt++] = it->second;
		FOR(i, t.timestamps.size())
		{
//			t.timestamps[i] = timeStamps[t.timestamps[i]];
		}
	}
	if(series.size()) emit(SetTimeseries(series));
	Close();
}
