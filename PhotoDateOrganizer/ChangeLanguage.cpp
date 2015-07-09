#include "ChangeLanguage.h"

ChangeLanguage::ChangeLanguage(LANGUAGES initLang, QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	if( initLang == LANGUAGES::ENGLISH )
		ui.english->setChecked(true);
	else
		ui.polish->setChecked(true);

	connect( ui.okButton, SIGNAL(clicked()), this, SLOT(okClicked()) );
	connect( ui.cancelButton, SIGNAL(clicked()), this, SLOT(cancelClicked()) );
}

ChangeLanguage::~ChangeLanguage()
{
	disconnect( ui.okButton, SIGNAL(clicked()), this, SLOT(okClicked()) );
	disconnect( ui.cancelButton, SIGNAL(clicked()), this, SLOT(cancelClicked()) );
}

void ChangeLanguage::okClicked( void )
{
	if( ui.polish->isChecked() )
		done((int)LANGUAGES::POLISH);
	else if( ui.english->isChecked() )
		done((int)LANGUAGES::ENGLISH);
	else
		reject();
}

void ChangeLanguage::cancelClicked( void )
{
	reject();
}



