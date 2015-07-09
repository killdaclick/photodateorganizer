#include "MainWindow.h"
#include <QApplication>
#include "Preferences.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
	auto p = Preferences::Instance();
	p.deserializeSettings();
    MainWindow w;
    w.show();

    return a.exec();
}
