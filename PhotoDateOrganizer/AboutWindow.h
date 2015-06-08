#ifndef ABOUTWINDOW_H
#define ABOUTWINDOW_H

#include <QDialog>
#include "ui_AboutWindow.h"

class AboutWindow : public QDialog
{
	Q_OBJECT

friend class MainWindow;

public:
	AboutWindow(QWidget *parent = 0);
	~AboutWindow();

private:
	Ui::AboutWindow ui;
};

#endif // ABOUTWINDOW_H
