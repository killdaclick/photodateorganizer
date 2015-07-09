#ifndef CHANGELANGUAGE_H
#define CHANGELANGUAGE_H

#include <QDialog>
#include "ui_ChangeLanguage.h"

enum LANGUAGES
{
	POLISH = 1,
	ENGLISH
};

class ChangeLanguage : public QDialog
{
	Q_OBJECT

friend class MainWindow;

public:
	ChangeLanguage( LANGUAGES initLang, QWidget *parent = 0);
	~ChangeLanguage();

private:
	Ui::ChangeLanguage ui;
	LANGUAGES lang;

public slots:
	void okClicked( void );
	void cancelClicked( void );
};

#endif // CHANGELANGUAGE_H
