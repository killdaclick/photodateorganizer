// My utility

#include <QString>
#include <QDir>
#include <QFile>
#include <QRegExp>

#pragma once
class Utility
{
public:
	Utility(void);
	~Utility(void);

	static bool Utility::copyDir(const QString src, const QString dest, bool copySrcRootDir = false);
	static bool Utility::mkPath( QString path );
	static bool Utility::clearDir(const QString &dirPath);
	static QString Utility::find( const QString &name, const QString &path, bool recursive = false, QDir::Filters filter=(QDir::Dirs | QDir::Files) );
	static QStringList Utility::findAll( const QString &name, const QString &path, bool recursive = false, QDir::Filters filter=(QDir::Dirs | QDir::Files) );
};

