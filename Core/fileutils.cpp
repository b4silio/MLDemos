#include "fileutils.h"
#include <QStringList>
#include <QDebug>
FileUtils::FileUtils(QString pathName)
{
	path = QDir(pathName);
}

QStringList FileUtils::GetFiles(QString prefix, QString ext)
{
	QStringList filters;
	filters << prefix+"*."+ext;
	QFileInfoList list = path.entryInfoList(filters);
	QStringList files;
	for(int i=0; i<list.size(); i++) files << list.at(i).absoluteFilePath();
	//qDebug() << "path: " << path;
	//qDebug() << "files: " << files;
	return files;
}
