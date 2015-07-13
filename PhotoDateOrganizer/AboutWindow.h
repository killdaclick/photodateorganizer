#ifndef ABOUTWINDOW_H
#define ABOUTWINDOW_H

#include <QDialog>
#include "ui_AboutWindow.h"
#include <QTemporaryDir>

class AboutWindow : public QDialog
{
	Q_OBJECT

friend class MainWindow;

public:
	AboutWindow(QWidget *parent = 0);
	~AboutWindow();

private:
	Ui::AboutWindow ui;
	QTemporaryDir* donateTmpDir;

public slots:
	void donateClicked( void );
};

#endif // ABOUTWINDOW_H
