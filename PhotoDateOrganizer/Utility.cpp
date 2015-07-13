#include "Utility.h"

Utility::Utility(void)
{
}


Utility::~Utility(void)
{
}

bool Utility::copyDir(const QString src, const QString dest, bool copySrcRootDir)
{
	QString rootDir = "";
	if( copySrcRootDir )
	{
		auto srcTmp = src;
		srcTmp = srcTmp.replace("/","\\");
		rootDir = srcTmp.right(srcTmp.size() - srcTmp.lastIndexOf("\\"));
	}
	
	QDir srcDir(src);
	QDir destDir(dest + rootDir);

	destDir.mkpath(dest + rootDir);
	if(!srcDir.isReadable())
		return false;

	auto srcList = srcDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs );
	for( QFileInfoList::iterator entry = srcList.begin(); entry != srcList.end(); ++entry )
	{
		if(entry->isDir())
			copyDir(entry->filePath(),dest + rootDir + "/" +entry->fileName());
		else if(entry->isFile() && entry->isReadable())
		{
			QFile file(entry->filePath());
			file.copy(destDir.absoluteFilePath(entry->fileName()));
		}
		else
			return false;
	}

	return true;
}

bool Utility::mkPath( QString path )
{
	path.replace("\\","/").replace("//","/");
	
	// znajdujemy litere dysku
	QString drive;
	auto drives = QDir::drives();
	for( QFileInfoList::iterator driveLetter = drives.begin(); driveLetter != drives.end(); ++driveLetter )
	{
		auto dl = driveLetter->absolutePath();
		if( path.contains(dl, Qt::CaseInsensitive) )
		{
			drive = dl;
			break;
		}
	}
	
	auto pathNoDrive = path;
	auto dirs = pathNoDrive.remove(drive, Qt::CaseInsensitive).split("/",QString::SkipEmptyParts);
	QString currentPath = drive;
	for( QStringList::iterator dir = dirs.begin(); dir != dirs.end(); ++dir )
	{
		QDir d(currentPath);
		QString tmp = currentPath + "/" + *dir;
		if( !d.exists( tmp ) )
		{
			if( !d.mkdir( tmp ) )
				return false;
		}
		currentPath = tmp;
	}

	return true;
}

bool Utility::clearDir(const QString &dirPath)
{
	QDir dir(dirPath);

	if(!dir.isReadable())
		return false;

	auto dirs = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs );
	for( QFileInfoList::iterator entry = dirs.begin(); entry != dirs.end(); ++entry )
	{
		if(entry->isDir())
			clearDir(entry->filePath());
		else if(entry->isFile() && entry->isReadable())
		{
			QFile file(entry->filePath());
			file.remove();
		}
		else
			return false;
	}
	return dir.rmdir(dirPath);
}

QString Utility::find( const QString &name, const QString &path, bool recursive, QDir::Filters filter )
{
	QDir dir(path);

	if(!dir.isReadable())
		return false;

	QRegExp exp(name, Qt::CaseInsensitive, QRegExp::Wildcard);
	auto dirs = dir.entryInfoList( QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs | QDir::Hidden );
	for( QFileInfoList::iterator entry = dirs.begin(); entry != dirs.end(); ++entry )
	{
		if( entry->fileName() == "." || entry->fileName() == "" )
			continue;
		if(entry->isDir())
		{
			if( ((filter & QDir::Dirs) == QDir::Dirs) )
			{ 
				if( exp.exactMatch(entry->fileName()) )
					return entry->absoluteFilePath();
			}

			if( recursive )
				find(name, entry->filePath(), filter);
		}
		else if(entry->isFile() && ((filter & QDir::Files) == QDir::Files) )
		{
			if( exp.exactMatch(entry->fileName()) )
				return entry->absoluteFilePath();
		}
	}

	return "";
}

QStringList Utility::findAll( const QString &name, const QString &path, bool recursive, QDir::Filters filter )
{
	QStringList found;
	QDir dir(path);

	if(!dir.isReadable())
		return found;

	QRegExp exp(name, Qt::CaseInsensitive, QRegExp::Wildcard);
	auto dirs = dir.entryInfoList( QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs | QDir::Hidden );
	for( QFileInfoList::iterator entry = dirs.begin(); entry != dirs.end(); ++entry )
	{
		if(entry->isDir())
		{
			if( ((filter & QDir::Dirs) == QDir::Dirs) )
			{ 
				if( exp.exactMatch(entry->fileName()) )
					found.push_back(entry->absoluteFilePath());
			}

			if( recursive )
				found.append(findAll(name, entry->filePath(), filter));
		}
		else if(entry->isFile() && ((filter & QDir::Files) == QDir::Files) )
		{
			if( exp.exactMatch(entry->fileName()) )
					found.push_back(entry->absoluteFilePath());
		}
	}

	return found;
}
