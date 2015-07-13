#include "AboutWindow.h"
#include "Preferences.h"
#include <QDesktopServices>
#include <QFile>
#include <QDir>
#include "Utility.h"

AboutWindow::AboutWindow(QWidget *parent)
	: QDialog(parent), donateTmpDir(nullptr)
{
	ui.setupUi(this);

	if( Preferences::Instance().getLanguage() == LANGUAGES::POLISH )
		ui.donateBtn->setIcon( QIcon(":/paypal/icons/paypal/paypal_donate_pl.gif") );
	else
		ui.donateBtn->setIcon( QIcon(":/paypal/icons/paypal/paypal_donate_en.gif") );

	connect( ui.donateBtn, SIGNAL(clicked()), this, SLOT(donateClicked()) );
}

AboutWindow::~AboutWindow()
{
	if( donateTmpDir != nullptr )
	{
		Utility::clearDir(donateTmpDir->path());
		donateTmpDir->remove();
		delete donateTmpDir;
		donateTmpDir = nullptr;
	}
	
	disconnect( ui.donateBtn, SIGNAL(clicked()), this, SLOT(donateClicked()) );
}

void AboutWindow::donateClicked( void )
{
	QString donateResF;
	QString donateResFname;
	if( Preferences::Instance().getLanguage() == LANGUAGES::POLISH )
	{
		donateResF = ":/paypal/icons/paypal/paypal_donate_pl.htm";
		donateResFname = "paypal_donate_pl.htm";
	}
	else
	{
		donateResF = ":/paypal/icons/paypal/paypal_donate_en.htm";
		donateResFname = "paypal_donate_en.htm";
	}
	
	if( donateTmpDir != nullptr )
		delete donateTmpDir;
	donateTmpDir = new QTemporaryDir();
	QString tmpFname = donateTmpDir->path() + "/" + donateResFname;
	if( QFile::copy( donateResF, tmpFname ) )
	{
		QFile::setPermissions( tmpFname, (QFileDevice::Permissions)0x7777 );
		QDesktopServices::openUrl("file:///" + tmpFname);
	}
}
