#include "MainWindow.h"
#include <QApplication>
#include "Preferences.h"

int main(int argc, char *argv[])
{
	while(1)
	{
		QApplication a(argc, argv);
		auto& p = Preferences::Instance();
		bool restoreDefaults = !p.deserializeSettings();
		p.loadLanguage();
		MainWindow w;
		if( restoreDefaults )
			w.restoreDefaultSettings();
		else
			w.loadSettings();
		w.show();

		int ret = a.exec();
		if( ret != APP_RESTART_EXIT_CODE )
			return ret;
	}
}
