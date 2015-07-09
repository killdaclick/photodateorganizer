#include "Preferences.h"
#include <QFile>
#include <QDataStream>
#include <QDir>

Preferences::Preferences()
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
			restoreDefaultSettings();
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
		restoreDefaultSettings();
		if( ser != nullptr )
			delete ser;
		if( f != nullptr )
			delete f;
		return false;
	}

	*ser >> recursiveFolders;
	//ui->recursiveFoldersCheckbox->setChecked(tb);

	*ser >> useExif;
	//ui->useExifDate->setChecked(tb);
	//ui->useModificationDate->setChecked(!tb);

	*ser >> changeOutputFileName;
	//ui->changeOutputFileName->setChecked(tb);

	*ser >> newNameTemplate;
	//ui->newNameTemplate->setText(ts);

	*ser >> createOutputFiles;
	//ui->createOutputFiles->setChecked(tb);

	*ser >> outputFolder;
	//ui->outputFolder->setText(ts);

	*ser >> createOutputSubfolders;
	//ui->createOutputSubfolders->setChecked(tb);

	*ser >> subfoldersNameTemplate;
	//ui->subfoldersNameTemplate->setText(ts);

	*ser >> saveOrgSubfolders;
	//ui->saveOrgSubfolders->setChecked(tb);

	*ser >> lastPath;

	int ti;
	*ser >> ti;
	language = (LANGUAGES)ti;

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
	ser << ui->recursiveFoldersCheckbox->isChecked();
	ser << ui->useExifDate->isChecked();
	ser << ui->changeOutputFileName->isChecked();
	ser << ui->newNameTemplate->text();
	ser << ui->createOutputFiles->isChecked();
	ser << ui->outputFolder->text();
	ser << ui->createOutputSubfolders->isChecked();
	ser << ui->subfoldersNameTemplate->text();
	ser << ui->saveOrgSubfolders->isChecked();
	ser << lastPath;
	ser << language;

	f.close();

	return true;
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
}

void Preferences::restoreDefaultSettings( void )
{
	deserializeSettings(&defaultSettings);
	serializeSettings();
}
