#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define EXIV2_IMG_TIMESTAMP     "Image timestamp"
#define EXIV2_IMG_SPLIT         ": "
#define EXIV2_BIN               "bin\\exiv2.exe"
#define EXIV2_DLL               "bin\\libexpat.dll"
#define APP_CONFIG_FILE			"config.ini"
#define APP_CONFIG_VERSION		1
#define APP_VERSION_ABOUT_STR	"<html><head/><body><p><span style=' font-size:12pt; font-weight:600;'>VER_REPLACE</span></p></body></html>"
#define APP_VERSION_ABOUT_REPLACE_STR	"VER_REPLACE"
#define APP_START_BUTTON_TXT	"Start"
#define APP_STOP_BUTTON_TXT		"Stop"

const int appVer = 0x010004;

#include <QMainWindow>


#include <stdio.h>

#include <QDateTime>
#include <QFileDialog>
#include <QProcess>
#include <QByteArray>
#include <QList>
#include <QMessageBox>
#include <QTimer>

#include <windows.h>

typedef QStringList					InFileList;
typedef QPair<QString,QDateTime>	FileExif;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

	bool changeFileTime( const QString& filePath, const QDateTime& t );
	QDateTime* getExifImgDateTime( const QString& filePath );
	void enableSignals( bool enable );
	bool changeFileName( const QString& filePath, QDateTime* fdt );
	bool getChangedFileName( const QString& filePath, QDateTime* fdt, QString& newName );
	bool isNewNameTemplateValid( void );
	bool getSubfolderPath( const QString& filePath, QDateTime* fdt, QString& subPath );
	void updateViews( void );
	void createDefaultSettings( void );
	void serializeSettings( void );
	void deserializeSettings( QByteArray* def = nullptr );
	QString getVersionString( void );
	void updateFoundFilesCount( const QStringList& files );

public slots:
	void selectFiles( void );
	void selectFolder( void );
	void start( void );
	void outputFolder( void );
	void newNameTemplateChanged(const QString & text);
	void subfoldersNameTemplateChanged(const QString & text);
	void restoreDefaultSettings( void );
	void aboutToQuit( void );
	void aboutWindow( void );

private:
	Ui::MainWindow *ui;

	InFileList inFileList;
	FileExif firstFileDate;
	QByteArray defaultSettings;
	bool started;
	bool cancel;
	QTime* tim;

signals:
	void progressBarSetValue( int val );
};

#endif // MAINWINDOW_H
