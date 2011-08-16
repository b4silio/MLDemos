#ifndef FILEUTILS_H
#define FILEUTILS_H

#include<QDir>

class FileUtils
{
	QDir path;

public:
	FileUtils(QString pathName);
	QStringList GetFiles(QString prefix, QString ext="avi");
};

#endif // FILEUTILS_H
