#ifndef PREFERENCES_H
#define PREFERENCES_H

#include "ChangeLanguage.h"
#include <QByteArray>
#include "ui_MainWindow.h"
#include <QTranslator>

#define APP_CONFIG_VERSION		7
#define APP_CONFIG_FILE			"config.ini"

class Preferences
{
	friend class MainWindow;

public:
	static Preferences& Instance( void );
	~Preferences();

	bool deserializeSettings( QByteArray* def = nullptr );
	void restoreDefaultSettings( Ui::MainWindow *ui );
	void createDefaultSettings( Ui::MainWindow *ui );
	bool serializeSettings( Ui::MainWindow *ui );
	bool serializeSettings( void );
	void loadSettings( Ui::MainWindow *ui );
	void loadLanguage( void );
	LANGUAGES getLanguage( void );
	void setDontCheckVersion( unsigned int ver ) { dontCheckVerNr = ver; }
	unsigned int getDontCheckVersion( void ) { return dontCheckVerNr; }
	QString getLastPath( void ) { return lastPath; }
	void setLastPath( const QString& lastp ) { lastPath = lastp; }

private:
	Preferences();

	QByteArray defaultSettings;

	bool recursiveFolders;
	bool setModificationToExifDT;
	bool changeOutputFileName;
	QString newNameTemplate;
	bool createOutputFiles;
	QString outputFolder;
	bool createOutputSubfolders;
	QString subfoldersNameTemplate;
	bool saveOrgSubfolders;
	QString lastPath;
	LANGUAGES language;
	QTranslator* langTrans;
	bool exifExtendedInfo;
	bool copyAdditionalFiles;
	unsigned int dontCheckVerNr;
};

#endif // PREFERENCES_H
