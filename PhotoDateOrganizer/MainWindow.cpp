#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "Utility.h"


MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
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
	}
	else
	{
		disconnect( ui->selectFilesBtn, SIGNAL(clicked()), this, SLOT(selectFiles()) );
		disconnect( ui->selectFolderBtn, SIGNAL(clicked()), this, SLOT(selectFolder()) );
		disconnect( ui->startBtn, SIGNAL(clicked()), this, SLOT(start()) );
		disconnect( ui->outputFolderBtn, SIGNAL(clicked()), this, SLOT(outputFolder()) );
		disconnect( ui->subfoldersNameTemplate, SIGNAL(textEdited(const QString &)), this, SLOT(subfoldersNameTemplateChanged(const QString & text)) );
		disconnect( ui->newNameTemplate, SIGNAL(textEdited(const QString &)), this, SLOT(newNameTemplateChanged(const QString & text)) );

	}
}

void MainWindow::selectFiles( void )
{
	QStringList files = QFileDialog::getOpenFileNames(
		this,
		tr("Wybierz pliki źródłowe"),
		QDir::currentPath(),
		tr("Zdjęcia (*.jpg *.png)"));
	
	QStringList::iterator fi = files.begin();
	if( fi == files.end() )
		return;

	// czyscimy
	inFileList.clear();
	firstFileDate = FileExif();
	ui->status->clear();
	
	QString inF;
	for( ; fi != files.end(); ++fi )
	{
		auto f = (*fi).replace("/","\\");
		inFileList.push_back( f );
		
		if( fi == files.begin() && ui->changeDatesUsingExif->isChecked() )
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

		inF.push_back( f );
		inF.push_back("<br>");
		//if( fi+1 == files.end() )
		//    inF.push_back("; ");
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

	// szukamy plikow w podkatalogach
	auto files = Utility::findAll( "*.jp*g", dir, ui->recursiveFoldersCheckbox->isChecked(), QDir::Files );

	QString inF;
	for( QStringList::iterator f = files.begin(); f != files.end(); ++f )
	{
		auto fName = f->replace("/","\\");
		inFileList.push_back( *f );
		
		if( f == files.begin() && ui->changeDatesUsingExif->isChecked() )
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

		inF.push_back( *f );
		inF.push_back("<br>");
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

	ui->startBtn->setEnabled(false);
	ui->progressBar->setValue(0);
	int fCnt = inFileList.count();
	int fStep = 100 / fCnt;
	ui->status->clear();

	if( ui->createOutputFiles->isChecked() && ui->outputFolder->text().isEmpty() )
	{
		QMessageBox::warning( this, tr("Błąd"), tr("Aby kontynuować musisz wybrać katalog docelowy!"), QMessageBox::Ok );
		ui->startBtn->setEnabled(true);
		return;
	}

	for( InFileList::iterator i = inFileList.begin(); i != inFileList.end(); ++i )
	{
		ui->progressBar->setValue(ui->progressBar->value() + fStep );
		auto dt = getExifImgDateTime( *i );
		//ui->status->appendHtml(*i);
		if( dt == nullptr )
		{
			ui->status->appendHtml(*i);
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
				ui->status->appendHtml(*i);
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
				if( !getSubfolderPath( fPath, dt, subPath ) )
				{
					delete dt;
					ui->status->appendHtml(*i);
					ui->status->appendHtml(tr("&nbsp;&nbsp;&nbsp;&nbsp;<font color='orange'>Błąd odczytu daty EXIF (3)</font><br>"));
					QApplication::processEvents();
					continue;
				}
				fPath.append("\\" + subPath);
				/* do dokonczenia
				createSubdirectories();*/
			}
			QFile::copy( *i, fPath + "\\" + fName );
		}

		ui->status->appendHtml(QString(fPath + "\\" + fName).replace("/","\\").replace("\\\\","\\"));
		if( changeFileTime( fPath + "\\" + fName, *dt ) )
			ui->status->appendHtml(tr("&nbsp;&nbsp;&nbsp;&nbsp;<font color='green'>OK</font><br>"));
		else
			ui->status->appendHtml(tr("&nbsp;&nbsp;&nbsp;&nbsp;<font color='red'>Błąd ustawiania daty</font><br>"));

		delete dt;

		QApplication::processEvents();
	}
	ui->status->appendHtml(tr("<b>Koniec.</b><br>"));
	ui->startBtn->setEnabled(true);
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
	/* do dokonczenia
	tu sie dzieje cos dziwnego bo w filePath przekazuje katalog a QFile::exists zwraca true */
	auto t1 = QFile::exists(filePath);
	
	if( filePath.isEmpty() || !QFile::exists(filePath) || fdt == nullptr )
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


