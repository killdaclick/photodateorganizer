#include "Preferences.h"
#include <QFile>
#include <QDataStream>
#include <QDir>

Preferences::Preferences() : langTrans(nullptr),
	language(LANGUAGES::POLISH)
{
}

Preferences::~Preferences()
{

}

Preferences& Preferences::Instance( void )
{
	static Preferences instance;
	return instance;
}

bool Preferences::deserializeSettings( QByteArray* def )
{
	QDataStream* ser = nullptr;
	QFile* f = nullptr;;
	if( def == nullptr )
	{
		// nie podano argumentu jako dane wejsciowe wiec czytamy z pliku
		f = new QFile(APP_CONFIG_FILE);
		if( !f->exists() || !f->open(QIODevice::ReadOnly) )
		{
			if( f != nullptr )
				delete f;
			//ui->status->appendHtml(tr("Brak pliku konfiguracyjnego - przywracam ustawienia domyœlne<br><br>"));
			//restoreDefaultSettings();
			return false;
		}
		ser = new QDataStream(f);
	}
	else
	{
		// czytamy serializacje z danych podanych w argumencie funkcji
		ser = new QDataStream(*def);
	}
	ser->setVersion( QDataStream::Qt_5_3 );

	int app_config_version;
	(*ser) >> app_config_version;
	if( app_config_version != APP_CONFIG_VERSION )
	{
		//ui->status->appendHtml(tr("<font color='red'>Plik konfiguracyjny w b³êdnej wersji - przywracam ustawienia domyœlne</font><br><br>"));
		//restoreDefaultSettings();
		if( ser != nullptr )
			delete ser;
		if( f != nullptr )
			delete f;
		return false;
	}

	*ser >> recursiveFolders;
	*ser >> useExif;
	*ser >> changeOutputFileName;
	*ser >> newNameTemplate;
	*ser >> createOutputFiles;
	*ser >> outputFolder;
	*ser >> createOutputSubfolders;
	*ser >> subfoldersNameTemplate;
	*ser >> saveOrgSubfolders;
	*ser >> lastPath;
	if( def == nullptr )
	{
		int ti;
		*ser >> ti;
		language = (LANGUAGES)ti;
	}
	*ser >> exifExtendedInfo;
	*ser >> copyAdditionalFiles;
	*ser >> addFilesExt;

	if( ser != nullptr )
		delete ser;
	if( f != nullptr )
		delete f;

	return true;
}

void Preferences::createDefaultSettings( Ui::MainWindow *ui )
{
	defaultSettings.clear();
	QDataStream def(&defaultSettings, QIODevice::WriteOnly);
	def.setVersion( QDataStream::Qt_5_3 );

	def << APP_CONFIG_VERSION;
	def << ui->recursiveFoldersCheckbox->isChecked();
	def << ui->useExifDate->isChecked();
	def << ui->changeOutputFileName->isChecked();
	def << ui->newNameTemplate->text();
	def << ui->createOutputFiles->isChecked();
	def << ui->outputFolder->text();
	def << ui->createOutputSubfolders->isChecked();
	def << ui->subfoldersNameTemplate->text();
	def << ui->saveOrgSubfolders->isChecked();
	def << QDir::currentPath();	// lastPath
	def << LANGUAGES::POLISH;
	def << ui->exifExtendedInfo->isChecked();
	def << ui->copyAdditionalFiles->isChecked();
	def << ui->addFilesExt->text();
}

bool Preferences::serializeSettings( void )
{
	QFile f(APP_CONFIG_FILE);
	if( !f.open( QIODevice::WriteOnly) )
	{
		//ui->status->appendHtml(tr("<font color='red'>B³¹d zapisu do pliku konfiguracyjnego: ") + APP_CONFIG_FILE + "</font><br><br>");
		return false;
	}

	QDataStream ser(&f);
	ser.setVersion( QDataStream::Qt_5_3 );

	ser << APP_CONFIG_VERSION;
	ser << recursiveFolders;
	ser << useExif;
	ser << changeOutputFileName;
	ser << newNameTemplate;
	ser << createOutputFiles;
	ser << outputFolder;
	ser << createOutputSubfolders;
	ser << subfoldersNameTemplate;
	ser << saveOrgSubfolders;
	ser << lastPath;
	ser << (int)language;
	ser << exifExtendedInfo;
	ser << copyAdditionalFiles;
	ser << addFilesExt;

	f.close();

	return true;
}

bool Preferences::serializeSettings( Ui::MainWindow *ui )
{
	recursiveFolders = ui->recursiveFoldersCheckbox->isChecked();
	useExif = ui->useExifDate->isChecked();
	changeOutputFileName = ui->changeOutputFileName->isChecked();
	newNameTemplate = ui->newNameTemplate->text();
	createOutputFiles = ui->createOutputFiles->isChecked();
	outputFolder = ui->outputFolder->text();
	createOutputSubfolders = ui->createOutputSubfolders->isChecked();
	subfoldersNameTemplate = ui->subfoldersNameTemplate->text();
	saveOrgSubfolders = ui->saveOrgSubfolders->isChecked();
	lastPath = lastPath;
	exifExtendedInfo = ui->exifExtendedInfo->isChecked();
	copyAdditionalFiles = ui->copyAdditionalFiles->isChecked();
	addFilesExt = ui->addFilesExt->text();

	return serializeSettings();
}

void Preferences::loadSettings( Ui::MainWindow *ui )
{
	ui->recursiveFoldersCheckbox->setChecked(recursiveFolders);

	ui->useExifDate->setChecked(useExif);
	ui->useModificationDate->setChecked(!useExif);

	ui->changeOutputFileName->setChecked(changeOutputFileName);

	ui->newNameTemplate->setText(newNameTemplate);

	ui->createOutputFiles->setChecked(createOutputFiles);

	ui->outputFolder->setText(outputFolder);

	ui->createOutputSubfolders->setChecked(createOutputSubfolders);

	ui->subfoldersNameTemplate->setText(subfoldersNameTemplate);

	ui->saveOrgSubfolders->setChecked(saveOrgSubfolders);

	ui->exifExtendedInfo->setChecked(exifExtendedInfo);

	ui->copyAdditionalFiles->setChecked(copyAdditionalFiles);
	ui->addFilesExt->setText(addFilesExt);
}

void Preferences::restoreDefaultSettings( Ui::MainWindow *ui )
{
	deserializeSettings(&defaultSettings);
	serializeSettings();
	loadSettings(ui);
}

void Preferences::loadLanguage( void )
{
	// sprawdzamy wymuszenie jezyka przez plik
	if( QFile::exists( qApp->applicationDirPath() + "/" + LANG_OVERIDE_POLISH ) )
	{
		language = LANGUAGES::POLISH;
		QFile::remove( qApp->applicationDirPath() + "/" + LANG_OVERIDE_POLISH );
	}
	if( QFile::exists( qApp->applicationDirPath() + "/" + LANG_OVERIDE_ENGLISH ) )
	{
		language = LANGUAGES::ENGLISH;
		QFile::remove( qApp->applicationDirPath() + "/" + LANG_OVERIDE_ENGLISH );
	}
	
	// ustaw tlumaczenia
	langTrans = new QTranslator();
	if( language == LANGUAGES::ENGLISH )
		langTrans->load(":/translations/photodateorganizer_en.qm");
	else if( language == LANGUAGES::POLISH )
	{
		langTrans->load(":/translations/qt_pl.qm");
		langTrans->load(":/translations/qtbase_pl.qm");
		false;
	}
	qApp->installTranslator(langTrans);
}

LANGUAGES Preferences::getLanguage( void )
{
	return language;
}
