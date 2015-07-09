#ifndef PREFERENCES_H
#define PREFERENCES_H

#include "ChangeLanguage.h"
#include <QByteArray>
#include "ui_MainWindow.h"

#define APP_CONFIG_VERSION		4
#define APP_CONFIG_FILE			"config.ini"

class Preferences
{
	friend class MainWindow;

public:
	static Preferences& Instance( void );
	~Preferences();

	bool deserializeSettings( QByteArray* def = nullptr );
	void restoreDefaultSettings( void );
	void createDefaultSettings( Ui::MainWindow *ui );
	bool serializeSettings( void );
	void loadSettings( Ui::MainWindow *ui );

private:
	Preferences();

	QByteArray defaultSettings;

	bool recursiveFolders;
	bool useExif;
	bool changeOutputFileName;
	QString newNameTemplate;
	bool createOutputFiles;
	QString outputFolder;
	bool createOutputSubfolders;
	QString subfoldersNameTemplate;
	bool saveOrgSubfolders;
	QString lastPath;
	LANGUAGES language;
};

#endif // PREFERENCES_H
