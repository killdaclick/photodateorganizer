#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "Utility.h"
#include "AboutWindow.h"
#include <QStyleFactory>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	started(false),
	cancel(false),
	tim(nullptr)
{
	QStyle* fusion = QStyleFactory::create("fusion");
	qApp->setStyle(fusion);
	
	if( !QFile::exists(QDir::currentPath() + "/" + EXIV2_BIN) )
	{
		QMessageBox::warning(this, tr("Brak pliku aplikacji"), tr("Wykryto brak wymaganego pliku") + " " + QString(EXIV2_BIN), QMessageBox::Abort );
		QTimer::singleShot(0, this, SLOT(close()));
	}
	if( !QFile::exists(QDir::currentPath() + "/" + EXIV2_DLL) )
	{
		QMessageBox::warning(this, tr("Brak pliku aplikacji"), tr("Wykryto brak wymaganego pliku") + " " + QString(EXIV2_DLL), QMessageBox::Abort );
		QTimer::singleShot(0, this, SLOT(close()));
	}

	ui->setupUi(this);
	createDefaultSettings();
	deserializeSettings();
	enableSignals(true);
}

MainWindow::~MainWindow()
{
	enableSignals(false);
	
	delete ui;
}

bool MainWindow::changeFileTime( const QString& filePath, const QDateTime& t )
{
	//getthe handle to the file
	HANDLE filename = CreateFile((LPCWSTR)filePath.utf16(),
	GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
	FILE_ATTRIBUTE_NORMAL, NULL);
	if( filename == INVALID_HANDLE_VALUE )
		return false;

	SYSTEMTIME st;
	st.wDay = t.date().day();
	st.wMonth = t.date().month();
	st.wYear = t.date().year();
	st.wHour = t.toUTC().time().hour();
	st.wMinute = t.toUTC().time().minute();
	st.wSecond = t.toUTC().time().second();

	//creation of a filetimestruct and convert our new systemtime
	FILETIME ft;

	SystemTimeToFileTime(&st,&ft);

	//set the filetime on the file
	bool ret = SetFileTime(filename,&ft,(LPFILETIME) NULL,&ft);
	//close our handle.
	CloseHandle(filename);

	return ret;
}

void MainWindow::enableSignals( bool enable )
{
	if( enable)
	{
		bool t = false;
		t = connect( ui->selectFilesBtn, SIGNAL(clicked()), this, SLOT(selectFiles()) );
		t = connect( ui->selectFolderBtn, SIGNAL(clicked()), this, SLOT(selectFolder()) );
		t = connect( ui->startBtn, SIGNAL(clicked()), this, SLOT(start()) );
		t = connect( ui->outputFolderBtn, SIGNAL(clicked()), this, SLOT(outputFolder()) );
		t = connect( ui->subfoldersNameTemplate, SIGNAL(textEdited(const QString &)), this, SLOT(subfoldersNameTemplateChanged(const QString &)) );
		t = connect( ui->newNameTemplate, SIGNAL(textEdited(const QString &)), this, SLOT(newNameTemplateChanged(const QString &)) );
		t = connect( ui->actionRestoreDefaults, SIGNAL(triggered()), this, SLOT(restoreDefaultSettings()) );
		t = connect( qApp, SIGNAL(aboutToQuit()), this, SLOT(aboutToQuit()) );
		t = connect( ui->actionExit, SIGNAL(triggered()), this, SLOT(close()) );
		t = connect( ui->actionAbout, SIGNAL(triggered()), this, SLOT(aboutWindow()) );
		t = connect( this, SIGNAL(progressBarSetValue(int)), ui->progressBar, SLOT(setValue(int)) );
	}
	else
	{
		disconnect( ui->selectFilesBtn, SIGNAL(clicked()), this, SLOT(selectFiles()) );
		disconnect( ui->selectFolderBtn, SIGNAL(clicked()), this, SLOT(selectFolder()) );
		disconnect( ui->startBtn, SIGNAL(clicked()), this, SLOT(start()) );
		disconnect( ui->outputFolderBtn, SIGNAL(clicked()), this, SLOT(outputFolder()) );
		disconnect( ui->subfoldersNameTemplate, SIGNAL(textEdited(const QString &)), this, SLOT(subfoldersNameTemplateChanged(const QString & text)) );
		disconnect( ui->newNameTemplate, SIGNAL(textEdited(const QString &)), this, SLOT(newNameTemplateChanged(const QString & text)) );
		disconnect( ui->actionRestoreDefaults, SIGNAL(triggered()), this, SLOT(restoreDefaultSettings()) );
		disconnect( qApp, SIGNAL(aboutToQuit()), this, SLOT(aboutToQuit()) );
		disconnect( ui->actionExit, SIGNAL(triggered()), this, SLOT(close()) );
		disconnect( ui->actionAbout, SIGNAL(triggered()), this, SLOT(aboutWindow()) );
		disconnect( this, SIGNAL(progressBarSetValue(int)), ui->progressBar, SLOT(setValue(int)) );
	}
}

void MainWindow::selectFiles( void )
{
	QStringList files = QFileDialog::getOpenFileNames(
		this,
		tr("Wybierz pliki źródłowe"),
		QDir::currentPath(),
		tr("Zdjęcia (*.jpg)"));
	
	QStringList::iterator fi = files.begin();
	if( fi == files.end() )
		return;

	updateFoundFilesCount(files);

	// czyscimy
	inFileList.clear();
	firstFileDate = FileExif();
	ui->status->clear();
	ui->inputFilesList->clear();
	ui->subfoldersNameTemplatePreview->setText("");
	ui->newNameTemplatePreview->setText("");
	
	QString inF;
	int fNr = 1;
	int fCnt = files.count();
	for( ; fi != files.end(); ++fi )
	{
		auto f = (*fi).replace("/","\\");
		inFileList.push_back( f );
		
		if( fi == files.begin() && ui->useExifDate->isChecked() )
		{
			// szukamy pierwszego pliku z poprawna data EXIF
			auto fiTmp = fi;
			while( fiTmp != files.end() )
			{
				firstFileDate.first = (*fiTmp).replace("/","\\");
				QDateTime* dt = getExifImgDateTime(f);
				if( dt != nullptr && !dt->isNull() )
				{
					firstFileDate.second = *dt;
					delete dt;
					break;
				}
				else
				{
					ui->status->appendHtml(tr("<font color='orange'>Błąd odczytu daty EXIF</font> dla pliku: ") + firstFileDate.first + tr(" - <font color='orange'>sprawdzam następny plik...</font><br>"));
					QApplication::processEvents();
				}
			}
		}

		QString fNrStr = "<b>[" + QString::number(fNr) + " \\ " + QString::number(fCnt) + "] </b>";
		inF.push_back( fNrStr + f );
		inF.push_back("<br>");
		fNr++;
	}
	if( inF.isEmpty() )
		return;
	ui->inputFilesList->setText(inF);

	// odswiezamy podglad nowej nazwy pliku
	updateViews();
}

void MainWindow::updateViews( void )
{
	newNameTemplateChanged( ui->newNameTemplate->text() );
	subfoldersNameTemplateChanged( ui->subfoldersNameTemplate->text() );
}

void MainWindow::selectFolder( void )
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Wybierz katalog z plikami źródłowymi"),
		QDir::currentPath(),
		QFileDialog::ShowDirsOnly
		| QFileDialog::DontResolveSymlinks);

	if( dir == "" )
		return;

	// czyscimy
	inFileList.clear();
	firstFileDate = FileExif();
	ui->status->clear();
	ui->inputFilesList->clear();
	ui->subfoldersNameTemplatePreview->setText("");
	ui->newNameTemplatePreview->setText("");

	// szukamy plikow w podkatalogach
	auto files = Utility::findAll( "*.jp*g", dir, ui->recursiveFoldersCheckbox->isChecked(), QDir::Files );

	updateFoundFilesCount(files);

	QString inF;
	int fNr = 1;
	int fCnt = files.count();
	for( QStringList::iterator f = files.begin(); f != files.end(); ++f )
	{
		auto fName = f->replace("/","\\");
		inFileList.push_back( *f );
		
		if( f == files.begin() && ui->useExifDate->isChecked() )
		{
			// szukamy pierwszego pliku z poprawna data EXIF
			auto fiTmp = f;
			while( fiTmp != files.end() )
			{
				firstFileDate.first = (*fiTmp).replace("/","\\");
				QDateTime* dt = getExifImgDateTime(*f);
				if( dt != nullptr && !dt->isNull() )
				{
					firstFileDate.second = *dt;
					delete dt;
					break;
				}
				else
				{
					ui->status->appendHtml(tr("<font color='orange'>Błąd odczytu daty EXIF</font> dla pliku: ") + firstFileDate.first + tr(" - <font color='orange'>sprawdzam następny plik...</font><br>"));
					QApplication::processEvents();
				}
			}
		}

		QString fNrStr = "<b>[" + QString::number(fNr) + " \\ " + QString::number(fCnt) + "] </b>";
		inF.push_back( fNrStr + *f );
		inF.push_back("<br>");
		fNr++;
	}

	if( inF.isEmpty() )
		return;
	ui->inputFilesList->setText(inF);

	// odswiezamy podglad nowej nazwy pliku
	updateViews();
}

void MainWindow::start( void )
{
	if( inFileList.count() == 0 )
		return;

	if( started )
	{
		cancel = true;
		started = false;;
		ui->startBtn->setText(APP_START_BUTTON_TXT);
		return;
	}
	else
	{
		cancel = false;
		started = true;
		ui->startBtn->setText(APP_STOP_BUTTON_TXT);
	}

	ui->progressBar->setValue(0);
	int fNr = 1;
	int fCnt = inFileList.count();
	int fStep = 100 / fCnt;
	ui->status->clear();

	if( ui->createOutputFiles->isChecked() && ui->outputFolder->text().isEmpty() )
	{
		QMessageBox::warning( this, tr("Błąd"), tr("Aby kontynuować musisz wybrać katalog docelowy!"), QMessageBox::Ok );
		cancel = false;
		ui->startBtn->setText(APP_START_BUTTON_TXT);
		return;
	}

	if( tim != nullptr )
		delete tim;
	tim = new QTime();
	tim->start();
	for( InFileList::iterator i = inFileList.begin(); i != inFileList.end(); ++i )
	{
		if( cancel )
			break;

		QString fNrStr = "<b>[" + QString::number(fNr) + " \\ " + QString::number(fCnt) + "] </b>";
		emit progressBarSetValue(ui->progressBar->value() + fStep );
		QApplication::processEvents();
		auto dt = getExifImgDateTime( *i );
		//ui->status->appendHtml(*i);
		if( dt == nullptr )
		{
			ui->status->appendHtml(fNrStr + *i);
			ui->status->appendHtml(tr("&nbsp;&nbsp;&nbsp;&nbsp;<font color='orange'>Błąd odczytu daty EXIF (1)</font><br>"));
			QApplication::processEvents();
			continue;
		}

		// tworzymy nowe nazwy
		QFileInfo fi(*i);
		QString fName = fi.fileName();
		QString fPath = fi.path();
		if( ui->changeOutputFileName->isChecked() )
		{
			if( !getChangedFileName(*i, dt, fName ) )
			{
				delete dt;
				ui->status->appendHtml(fNrStr + *i);
				ui->status->appendHtml(tr("&nbsp;&nbsp;&nbsp;&nbsp;<font color='orange'>Błąd odczytu daty EXIF (2)</font><br>"));
				QApplication::processEvents();
				continue;
			}
		}

		// tworzymy sciezke do katalogu docelowego + subfoldery
		if( ui->createOutputFiles->isChecked() )
		{
			fPath = ui->outputFolder->text();
			if( ui->createOutputSubfolders->isChecked() )
			{
				QString subPath;
				if( !getSubfolderPath( *i, dt, subPath ) )
				{
					delete dt;
					ui->status->appendHtml(fNrStr + *i);
					ui->status->appendHtml(tr("&nbsp;&nbsp;&nbsp;&nbsp;<font color='orange'>Błąd odczytu daty EXIF (3)</font><br>"));
					QApplication::processEvents();
					continue;
				}
				fPath.append("\\" + subPath);
				
				Utility::mkPath( fPath );
			}
			ui->status->appendHtml(fNrStr + QString(fPath + "\\" + fName).replace("/","\\").replace("\\\\","\\"));
			QFileInfo dstI(fPath + "\\" + fName);
			int inc = 1;
			QString statInfo = "";
			if( dstI.exists() && dstI.isFile() )
			{
				statInfo += tr("&nbsp;&nbsp;&nbsp;&nbsp;<font color='orange'>Plik docelowy o nazwie: ") + fName + tr(" już istnieje, zamieniam nazwę na: </font>");
				QString baseName = dstI.baseName();
				while( dstI.exists() && dstI.isFile() )
				{
					// plik istnieje wiec zmienamy mu nazwe i probojemy do skutku
					fName = baseName + "_" + QString::number(inc) + "." + dstI.suffix();
					dstI.setFile(fPath + "\\" + fName);
					inc++;
				}
				ui->status->appendHtml(statInfo + "<font color='orange'>" + fName + "</font>");
			}
			QFile::copy( *i, fPath + "\\" + fName );
		}
		else
			ui->status->appendHtml(fNrStr + QString(fPath + "\\" + fName).replace("/","\\").replace("\\\\","\\"));

		if( changeFileTime( fPath + "\\" + fName, *dt ) )
			ui->status->appendHtml(tr("&nbsp;&nbsp;&nbsp;&nbsp;<font color='green'>OK</font><br>"));
		else
			ui->status->appendHtml(tr("&nbsp;&nbsp;&nbsp;&nbsp;<font color='red'>Błąd ustawiania daty</font><br>"));

		delete dt;

		fNr++;
		QApplication::processEvents();
	}

	if( cancel )
		ui->status->appendHtml(tr("<font color='red'>Przerwano pracę na życzenie użytkownika</font><br>"));
	else
		ui->progressBar->setValue(100);
	int elapsed = tim->elapsed();
	delete tim;
	tim = nullptr;
	ui->status->appendHtml(tr("<b>Zakończono pracę w ") + QString::number(elapsed/1000) + "." + QString::number(elapsed%1000) + tr(" sekundy.</b><br>"));
	cancel = false;
	started = false;
	ui->startBtn->setText(APP_START_BUTTON_TXT);
}

bool MainWindow::isNewNameTemplateValid( void )
{

	return true;
}

bool MainWindow::getChangedFileName( const QString& filePath, QDateTime* fdt, QString& newName )
{
	if( filePath.isEmpty() || !QFile::exists(filePath) || fdt == nullptr )
		return false;

	QString tmplate = ui->newNameTemplate->text();
	QString nn = "";
	QString ext = "";
	for( QString::iterator c = tmplate.begin(); c != tmplate.end(); ++c )
	{
		if( *c == '%' )
		{
			++c;
			if( c == tmplate.end() )
				break;
			switch( (*c).toLatin1() )
			{
				case 'D':
					nn.push_back( fdt->toString("yyyyMMdd-hhmmss") );
					break;

				case 'N':
					QFileInfo fi(filePath);
					nn.push_back(fi.fileName().replace("." + fi.suffix(), ""));
					ext = "." + fi.suffix();
					break;
			}
		}
		else
			nn.push_back(*c);
	}
	nn.push_back(ext);
	newName = nn;
	return true;
}

bool MainWindow::getSubfolderPath( const QString& filePath, QDateTime* fdt, QString& subPath )
{
	QFileInfo fi( filePath );
	
	if( filePath.isEmpty() || !fi.exists() || fi.isDir() || fdt == nullptr )
		return false;

	QString tmplate = ui->subfoldersNameTemplate->text();
	QString nn = "";
	for( QString::iterator c = tmplate.begin(); c != tmplate.end(); ++c )
	{
		if( *c == '%' )
		{
			++c;
			if( c == tmplate.end() )
				break;
			switch( (*c).toLatin1() )
			{
				case 'R':
					nn.push_back(fdt->toString("yyyy"));
					break;

				case 'M':
					nn.push_back(fdt->toString("MM"));
					break;

				case 'D':
					nn.push_back(fdt->toString("dd"));
					break;

				case 'h':
					nn.push_back(fdt->toString("hh"));
					break;

				case 'm':
					nn.push_back(fdt->toString("mm"));
					break;

				case 's':
					nn.push_back(fdt->toString("ss"));
					break;

				case '\\':
					nn.push_back("\\");
					break;
			}
		}
		else
			nn.push_back(*c);
	}
	subPath = nn;
	return true;
}

bool MainWindow::changeFileName( const QString& filePath, QDateTime* fdt )
{
	QString nn;
	if( !getChangedFileName( filePath, fdt, nn ) )
		return false;
	QFile f( filePath );
	return f.rename(nn);
}

void MainWindow::outputFolder( void )
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Wybierz katalog docelowy"),
		QDir::currentPath(),
		QFileDialog::ShowDirsOnly
		| QFileDialog::DontResolveSymlinks);

	ui->outputFolder->setText(dir.replace("/","\\").replace("\\\\","\\"));
}

QDateTime* MainWindow::getExifImgDateTime( const QString& filePath )
{
	if( filePath.isEmpty() || !QFile::exists(filePath) )
		return nullptr;

	QProcess p;
	QString startCmd = "\"" + QDir::currentPath().replace("/","\\") + "\\" + QString(EXIV2_BIN) + "\" \"" + filePath + "\"";
	p.start( startCmd );
	p.waitForFinished();
	while( p.canReadLine() )
	{
		auto l = QString(p.readLine());
		if( !l.contains(EXIV2_IMG_TIMESTAMP))
			continue;
		auto s = l.split(EXIV2_IMG_SPLIT, QString::SkipEmptyParts);
		auto t1 = s[1].replace("\r\n","");
		auto dttmp = QDateTime::fromString(s[1].replace("\r\n",""), "yyyy:MM:dd hh:mm:ss");
		if( !dttmp.isNull() )
			return new QDateTime(dttmp);
		else
			return nullptr;
	}

	return nullptr;
}

void MainWindow::newNameTemplateChanged(const QString & text)
{
	if( inFileList.isEmpty() || firstFileDate.second.isNull() )
		return;

	QString ffn = firstFileDate.first;	// first file name - bierzemy pierwszy plik dla tworzenia przykladu w ui->newNameTemplatePreview
	QDateTime dt = firstFileDate.second;
	if( ffn.isEmpty() || dt.isNull() )
		return;

	QString nameTmplate;
	if( getChangedFileName( ffn, &dt, nameTmplate ) )
	{
		ui->newNameTemplatePreview->setText( nameTmplate );
	}
}

void MainWindow::subfoldersNameTemplateChanged(const QString & text)
{
	if( inFileList.isEmpty() || firstFileDate.second.isNull() )
		return;

	QString ffn = firstFileDate.first;	// first file name - bierzemy pierwszy plik dla tworzenia przykladu w ui->newNameTemplatePreview
	QDateTime dt = firstFileDate.second;
	if( ffn.isEmpty() || dt.isNull() )
		return;

	QString subPath;
	if( getSubfolderPath( ffn, &dt, subPath ) )
	{
		ui->subfoldersNameTemplatePreview->setText( subPath );
	}
}

void MainWindow::serializeSettings( void )
{
	QFile f(APP_CONFIG_FILE);
	if( !f.open( QIODevice::WriteOnly) )
	{
		ui->status->appendHtml(tr("<font color='red'>Błąd zapisu do pliku konfiguracyjnego: ") + APP_CONFIG_FILE + "</font><br><br>");
		return;
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

	f.close();
}

void MainWindow::deserializeSettings( QByteArray* def )
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
			ui->status->appendHtml(tr("Brak pliku konfiguracyjnego - przywracam ustawienia domyślne<br><br>"));
			restoreDefaultSettings();
			return;
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
		ui->status->appendHtml(tr("<font color='red'>Plik konfiguracyjny w błędnej wersji - przywracam ustawienia domyślne</font><br><br>"));
		restoreDefaultSettings();
		if( ser != nullptr )
			delete ser;
		if( f != nullptr )
			delete f;
		return;
	}

	bool tb;
	QString ts;

	*ser >> tb;
	ui->recursiveFoldersCheckbox->setChecked(tb);

	*ser >> tb;
	ui->useExifDate->setChecked(tb);
	ui->useModificationDate->setChecked(!tb);

	*ser >> tb;
	ui->changeOutputFileName->setChecked(tb);

	*ser >> ts;
	ui->newNameTemplate->setText(ts);

	*ser >> tb;
	ui->createOutputFiles->setChecked(tb);

	*ser >> ts;
	ui->outputFolder->setText(ts);

	*ser >> tb;
	ui->createOutputSubfolders->setChecked(tb);

	*ser >> ts;
	ui->subfoldersNameTemplate->setText(ts);

	if( ser != nullptr )
		delete ser;
	if( f != nullptr )
		delete f;
}

void MainWindow::createDefaultSettings( void )
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
}

void MainWindow::restoreDefaultSettings( void )
{
	deserializeSettings(&defaultSettings);
	serializeSettings();
}

void MainWindow::aboutToQuit( void )
{
	serializeSettings();
}

QString MainWindow::getVersionString( void )
{
	return QString("v" + QString::number((appVer&0xFF0000)>>16) + "." + QString::number((appVer&0x00FF00)>>8) + "." + QString::number(appVer&0x0000FF));
}

void MainWindow::aboutWindow( void )
{
	AboutWindow* a = new AboutWindow(this);
	a->setAttribute( Qt::WA_DeleteOnClose );
	a->ui.version->setText( a->ui.version->text().replace(APP_VERSION_ABOUT_REPLACE_STR, getVersionString()) );
	a->exec();
}

void MainWindow::updateFoundFilesCount( const QStringList& files )
{
	ui->filesCnt->setText(QString::number(files.count()));
}


