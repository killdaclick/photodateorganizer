#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define APP_RESTART_EXIT_CODE	10
#define EXIV2_IMG_TIMESTAMP     "Image timestamp"
#define EXIV2_IMG_SPLIT         ": "
#define EXIV2_BIN               "bin\\exiv2.exe"
#define EXIV2_DLL               "bin\\libexpat.dll"
#define APP_VERSION_ABOUT_STR	"<html><head/><body><p><span style=' font-size:12pt; font-weight:600;'>VER_REPLACE</span></p></body></html>"
#define APP_VERSION_ABOUT_REPLACE_STR	"VER_REPLACE"
#define APP_START_BUTTON_TXT	"Start"
#define APP_STOP_BUTTON_TXT		"Stop"

const int appVer = 0x010402;

#include <QMainWindow>

#include <stdio.h>

#include <QDateTime>
#include <QFileDialog>
#include <QProcess>
#include <QByteArray>
#include <QList>
#include <QMessageBox>
#include <QTimer>
#include <QVector>
#include <QElapsedTimer>
#include <QTranslator>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QListView>

#include <windows.h>

#include "ChangeLanguage.h"

struct TranslationSet
{
	QString pol;
	QString eng;
};

typedef QStringList					InFileList;
typedef QPair<QString,QDateTime>	FileExif;
typedef QVector<int>	TimeStepList;
typedef QVector<int>	SizeSpeedList;
typedef QList<TranslationSet> TranslationTable;

enum mainTabs
{
	TAB_OPTIONS=0,
	TAB_GUIDE,
	TAB_INFO,
	TAB_COUNT
};

class HTMLDelegate : public QStyledItemDelegate
{
protected:
	void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
	QSize sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const;
};

class InputFilesView : public QListView
{
	Q_OBJECT

public:
	InputFilesView(QWidget* parent = 0);
	~InputFilesView();

protected slots:
	void currentChanged(const QModelIndex & current, const QModelIndex & previous);

signals:
	void currentChangedSignal(const QModelIndex & current, const QModelIndex & previous);
};

class SizeSpeed : QObject
{
	Q_OBJECT

public:
	SizeSpeed( QLabel* qlabel, qint64* convFilesSize );
	~SizeSpeed();

public slots:
	void timeout( void );

private:
	QTimer tim;
	QElapsedTimer et;
	QLabel* label;
	qint64 lastSize;
	qint64* convFilesSize;
	SizeSpeedList sizeSpeedSteps;
};

class TimeToFinish
{
public:
	TimeToFinish( int avgStepVar, int maxStepCnt );
	~TimeToFinish();

	void start( void );
	bool step( void );
	void stop( void );
	int lastStepsTime( int avgStepVar = -1 ) const;
	int avgStepTimeFromLastNsteps( int avgStepVar = -1 ) const;
	int getAvgTimeToFinish( void );
	int getTotalTime( void );

private:
	int avgStepVar_;
	int maxStepCnt_;
	int avgTimeToFinish;
	QTime* tim;
	TimeStepList timeSteps;

	int calcAvgTimeToFinish( void ) const;
};

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
	QString getExifOutput( const QString& filePath, LANGUAGES lang );
	void enableSignals( bool enable );
	bool changeFileName( const QString& filePath, QDateTime* fdt );
	bool getChangedFileName( const QString& filePath, QDateTime* fdt, QString& newName );
	bool isNewNameTemplateValid( void );
	bool getSubfolderPath( const QString& filePath, QDateTime* fdt, QString& subPath );
	void updateViews( void );
	QString getVersionString( void );
	void updateFoundFilesCount( const QStringList& files );
	void updateAvgTimeToFinish( int timeToF );
	void updateFileSizeLabel( QLabel* label, qint64 size );
	void changeLanguage( LANGUAGES lang );
	void restart( void );
	QString translateExif( QString input, LANGUAGES lang );

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
	void actionSetupUpdateSlot( void );
	void actionSetupOrgNewSlot( void );
	void actionSetupNewDateSlot( void );
	void actionSetupOrgNewDateSlot( void );
	void actionChangeLang( void );
	void inputFileClicked( const QModelIndex& index, const QModelIndex& last );

private:
	Ui::MainWindow *ui;

	InFileList inFileList;
	FileExif firstFileDate;
	QByteArray defaultSettings;
	bool started;
	bool cancel;
	QString srcFolder;
	TimeToFinish* timToF;
	qint64 filesSize;
	qint64 convFilesSize;
	SizeSpeed* convFilesSizeTim;
	QString lastPath;
	QStandardItemModel inputFilesModel;
	HTMLDelegate htmlDelegate;
	TranslationTable exifTransTable;

	void createExifTranslationTable( void );

signals:
	void progressBarSetValue( int val );
};

#endif // MAINWINDOW_H
