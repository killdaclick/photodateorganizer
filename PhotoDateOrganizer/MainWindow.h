#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define EXIV2_IMG_TIMESTAMP     "Image timestamp"
#define EXIV2_IMG_SPLIT         ": "
#define EXIV2_BIN               "bin\\exiv2.exe"
#define EXIV2_DLL               "bin\\libexpat.dll"

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

public slots:
    void selectFiles( void );
    void selectFolder( void );
    void start( void );
    void outputFolder( void );
	void newNameTemplateChanged(const QString & text);
	void subfoldersNameTemplateChanged(const QString & text);

private:
    Ui::MainWindow *ui;

    InFileList inFileList;
	FileExif firstFileDate;
};

#endif // MAINWINDOW_H
