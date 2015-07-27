#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "Utility.h"
#include "AboutWindow.h"
#include "Preferences.h"
#include <QStyleFactory>
#include <QTransform>

extern Q_CORE_EXPORT int qt_ntfs_permission_lookup;

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	started(false),
	cancel(false),
	timToF(nullptr),
	convFilesSizeTim(nullptr),
	selFilePath("")
{
	// turn on NTFS permission handling
	qt_ntfs_permission_lookup++;
	
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
	// TODO
	/*if( !QFile::exists(QDir::currentPath() + "/" + EXIV2_DLL) )
	{
		QMessageBox::warning(this, tr("Brak pliku aplikacji"), tr("Wykryto brak wymaganego pliku") + " " + QString(JPEGTRAN_BIN), QMessageBox::Abort );
		QTimer::singleShot(0, this, SLOT(close()));
	}*/

	ui->setupUi(this);
	ui->inputFilesList->setModel(&inputFilesModel);
	ui->inputFilesList->setItemDelegate(&htmlDelegate);
	ui->imgPreview->setContextMenuPolicy( Qt::CustomContextMenu );
	auto& p = Preferences::Instance();
	p.createDefaultSettings(ui);
	p.loadSettings( ui );
	enableSignals(true);
	ui->dateFrom->setDate( QDate::currentDate() );
	ui->dateTo->setDate( QDate::currentDate() );
	createExifTranslationTable();

	update.checkUpdate();
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
		t = connect( ui->actionSetupNewDate, SIGNAL(triggered()), this, SLOT(actionSetupNewDateSlot()) );
		t = connect( ui->actionSetupUpdate, SIGNAL(triggered()), this, SLOT(actionSetupUpdateSlot()) );
		t = connect( ui->actionSetupOrgNew, SIGNAL(triggered()), this, SLOT(actionSetupOrgNewSlot()) );
		t = connect( ui->actionSetupOrgNewDate, SIGNAL(triggered()), this, SLOT(actionSetupOrgNewDateSlot()) );
		t = connect( ui->actionLang, SIGNAL(triggered()), this, SLOT(actionChangeLang()) );
		t = connect( ui->inputFilesList, SIGNAL(currentChangedSignal(const QModelIndex&, const QModelIndex&)), this, SLOT(inputFileClicked(const QModelIndex&, const QModelIndex&)) );
		t = connect( ui->imgPreview, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(imgPreviewMenuRequested(QPoint)) );
		t = connect( ui->exifExtendedInfo, SIGNAL(stateChanged(int)), this, SLOT( exifExtendedInfoStateChanged(int) ) );
		t = connect( ui->setExifToModificationDT, SIGNAL(stateChanged(int)), this, SLOT(setExifToModificationDTchanged(int)) );
		t = connect( ui->setModificationToExifDT, SIGNAL(stateChanged(int)), this, SLOT(setModificationToExifDTchanged(int)) );
		t = false;
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
		disconnect( ui->actionSetupNewDate, SIGNAL(triggered()), this, SLOT(actionSetupNewDateSlot()) );
		disconnect( ui->actionSetupUpdate, SIGNAL(triggered()), this, SLOT(actionSetupUpdateSlot()) );
		disconnect( ui->actionSetupOrgNew, SIGNAL(triggered()), this, SLOT(actionSetupOrgNewSlot()) );
		disconnect( ui->actionSetupOrgNewDate, SIGNAL(triggered()), this, SLOT(actionSetupOrgNewDateSlot()) );
		disconnect( ui->actionLang, SIGNAL(trigerred()), this, SLOT(actionChangeLang()) );
		disconnect( ui->inputFilesList, SIGNAL(currentChangedSignal(const QModelIndex&, const QModelIndex&)), this, SLOT(inputFileClicked(const QModelIndex&, const QModelIndex&)) );
		disconnect( ui->imgPreview, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(imgPreviewMenuRequested(QPoint)) );
		disconnect( ui->exifExtendedInfo, SIGNAL(stateChanged(int)), this, SLOT( exifExtendedInfoStateChanged(int) ) );
	}
}

void MainWindow::selectFiles( void )
{
	QString dir = lastPath;
	if( !QFile::exists(dir) )
		dir = QDir::currentPath();
	QStringList files = QFileDialog::getOpenFileNames(
		this,
		tr("Wybierz pliki źródłowe"),
		dir,
		tr("Zdjęcia (*.jpg)"));
	
	QStringList::iterator fi = files.begin();
	if( fi == files.end() )
		return;

	updateFoundFilesCount(files);

	// czyscimy
	inFileList.clear();
	firstFileDate = FileExif();
	ui->status->clear();
	//ui->inputFilesList->clear();
	ui->subfoldersNameTemplatePreview->setText("");
	ui->newNameTemplatePreview->setText("");
	filesSize = 0;
	ui->filesCnt->setText("0");
	ui->filesSize->setText("0 MB");
	ui->totalSize->setText("0 MB");
	updateAvgTimeToFinish(0);
	emit progressBarSetValue(0);
	inputFilesModel.clear();
	inputFilesModel.insertRow(0, new QStandardItem("<b>Ładuję...</b>"));
	//ui->inputFilesList->setText(tr("<b>Ładuję...</b>"));
	QApplication::processEvents();
	
	//QString inF;
	int fNr = 1;
	int fCnt = files.count();
	for( ; fi != files.end(); ++fi )
	{
		auto f = (*fi).replace("/","\\");
		inFileList.push_back( f );
		
		if( fi == files.begin() /*TODO && ui->useExifDate->isChecked()*/ )
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

		QFileInfo fInfo(f);

		// wybieramy pliki spelniajace filtr daty/czasu
		if( ui->filterDate->isChecked() )
		{
			if( true /*TODO ui->useExifDate->isChecked()*/ )
			{
				QDateTime* dt = getExifImgDateTime(f);
				if( !(*dt > ui->dateFrom->dateTime() && *dt < ui->dateTo->dateTime()) )
					continue;
			}
			/*else if( ui->useModificationDate->isChecked() )
			{
				// TODO
			}*/
		}

		// podliczamy rozmiar plikow
		filesSize += fInfo.size();

		// zapamietujemy sciezke do katalogu z plikami (dla wygody uzytkowania - pozniej gdy klikamy wybor plikow albo folderow domyslnie otwiera sie ostatni katalog)
		lastPath = fInfo.absolutePath();

		QString fNrStr = "<b>[" + QString::number(fNr) + " \\ " + QString::number(fCnt) + "] </b>";
		inputFilesModel.insertRow(inputFilesModel.rowCount(), new QStandardItem(fNrStr + f));
		//inF.push_back( fNrStr + f );
		//inF.push_back("<br>");
		fNr++;
	}
	if( inputFilesModel.rowCount() <= 1 /*inF.isEmpty()*/ )
	{
		inputFilesModel.clear();
		//ui->inputFilesList->setText("");
		return;
	}

	// printujemy wszystkie sciezki plikow
	inputFilesModel.removeRow(0);	// usuwamy element "Ładuję..."
	//ui->inputFilesList->setText(inF);
	// printujemy sumaryczny rozmiar plikow
	updateFileSizeLabel( ui->filesSize, filesSize );
	updateFileSizeLabel( ui->totalSize, filesSize );

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
	QString dir = lastPath;
	if( !QFile::exists(dir) )
		dir = QDir::currentPath();
	dir = QFileDialog::getExistingDirectory(this, tr("Wybierz katalog z plikami źródłowymi"),
		dir,
		QFileDialog::ShowDirsOnly
		| QFileDialog::DontResolveSymlinks);

	if( dir == "" )
		return;

	// czyscimy
	inFileList.clear();
	firstFileDate = FileExif();
	ui->status->clear();
	//ui->inputFilesList->clear();
	ui->subfoldersNameTemplatePreview->setText("");
	ui->newNameTemplatePreview->setText("");
	filesSize = 0;
	ui->filesCnt->setText("0");
	ui->filesSize->setText("0 MB");
	ui->totalSize->setText("0 MB");
	updateAvgTimeToFinish(0);
	emit progressBarSetValue(0);
	inputFilesModel.clear();
	inputFilesModel.insertRow(0, new QStandardItem("<b>Ładuję...</b>"));
	//ui->inputFilesList->setText(tr("<b>Ładuję...</b>"));
	QApplication::processEvents();

	// zapamietujemy sciezke do katalogu z plikami (dla wygody uzytkowania - pozniej gdy klikamy wybor plikow albo folderow domyslnie otwiera sie ostatni katalog)
	lastPath = dir;
	// zapamietujemy katalog zrodlowy - potrzebne gdy wybrana opcja saveOrgSubfolders
	srcFolder = dir;
	// szukamy plikow w podkatalogach
	auto files = Utility::findAll( "*.jp*g", dir, ui->recursiveFoldersCheckbox->isChecked(), QDir::Files );

	updateFoundFilesCount(files);

	//QString inF;
	int fNr = 1;
	int fCnt = files.count();
	for( QStringList::iterator f = files.begin(); f != files.end(); ++f )
	{
		auto fName = f->replace("/","\\");
		inFileList.push_back( *f );
		
		if( f == files.begin() /* TODO && ui->useExifDate->isChecked()*/ )
		{
			// szukamy pierwszego pliku z poprawna data EXIF
			auto fiTmp = f;
			while( fiTmp != files.end() )
			{
				firstFileDate.first = (*fiTmp).replace("/","\\");
				QDateTime* dt = getExifImgDateTime(*fiTmp);
				if( dt != nullptr && !dt->isNull() )
				{
					firstFileDate.second = *dt;
					delete dt;
					ui->status->appendHtml(tr("Odczyt daty EXIF do podglądu: <font color='green'>OK</font><br>"));
					break;
				}
				else
				{
					ui->status->appendHtml(tr("<font color='orange'>Błąd odczytu daty EXIF</font> dla pliku: ") + firstFileDate.first + tr(" - <font color='orange'>sprawdzam następny plik...</font><br>"));
					QApplication::processEvents();
				}
				++fiTmp;
			}
		}

		QFileInfo fInfo(fName);

		// wybieramy pliki spelniajace filtr daty/czasu
		if( ui->filterDate->isChecked() )
		{
			if( true /* TODO ui->useExifDate->isChecked()*/ )
			{
				QDateTime* dt = getExifImgDateTime(fName);
				if( dt == nullptr || !(*dt > ui->dateFrom->dateTime() && *dt < ui->dateTo->dateTime()) )
					continue;
			}
			/*else if( ui->useModificationDate->isChecked() )
			{
				// TODO
			}*/
		}

		// podliczamy rozmiar plikow
		filesSize += fInfo.size();

		QString fNrStr = "<b>[" + QString::number(fNr) + " \\ " + QString::number(fCnt) + "] </b>";
		inputFilesModel.insertRow(inputFilesModel.rowCount(), new QStandardItem(fNrStr + *f));
		//inF.push_back( fNrStr + *f );
		//inF.push_back("<br>");
		fNr++;
	}

	if( inputFilesModel.rowCount() <= 1 /*inF.isEmpty()*/ )
	{
		inputFilesModel.clear();
		//ui->inputFilesList->setText("");
		return;
	}

	// printujemy wszystkie sciezki do plikow
	inputFilesModel.removeRow(0);	// usuwamy element "Ładuję..."
	//ui->inputFilesList->setText(inF);
	// printujemy sumaryczny rozmiar plikow
	updateFileSizeLabel( ui->filesSize, filesSize );
	updateFileSizeLabel( ui->totalSize, filesSize );

	// odswiezamy podglad nowej nazwy pliku
	updateViews();
}

void MainWindow::start( void )
{
	QDateTime dt = QDateTime::currentDateTime();
	bool t1 = getShiftedDateTime( ui->shiftExifDate->text(), &dt );
	QString t2 = dt.toString("yyyy-MM-dd hh:mm:ss");
	
	
	
	
	
	
	
	
	
	return;
	
	
	
	
	
	
	
	
	
	
	
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
	float prog = 0;
	float fStep = (float)100 / (float)fCnt;
	int errCnt = 0;
	ui->status->clear();

	if( ui->createOutputFiles->isChecked() && ui->outputFolder->text().isEmpty() )
	{
		QMessageBox::warning( this, tr("Błąd"), tr("Aby kontynuować musisz wybrać katalog docelowy!"), QMessageBox::Ok );
		cancel = false;
		ui->startBtn->setText(APP_START_BUTTON_TXT);
		return;
	}

	if( timToF != nullptr )
		delete timToF;
	convFilesSize = 0;
	QStringList addFilesList;	// TODO
	qint64 totFsize = filesSize;	// calkowity rozmiar plikow do przetworzenia ktory moze sie zmniejszac ze wzgledu na pliki ktorych nie mozna byly przekonwertowac
	timToF = new TimeToFinish( 100, fCnt );
	convFilesSizeTim = new SizeSpeed( ui->sizeTime, &convFilesSize );
	timToF->start();
	for( InFileList::iterator i = inFileList.begin(); i != inFileList.end(); ++i )
	{
		if( cancel )
			break;

		QString fNrStr = "<b>[" + QString::number(fNr) + " \\ " + QString::number(fCnt) + "] </b>";
		prog += fStep;
		emit progressBarSetValue(/*ui->progressBar->value() + fStep*/ (int)prog );
		QApplication::processEvents();
		auto dt = getExifImgDateTime( *i );
		//ui->status->appendHtml(*i);
		if( dt == nullptr )
		{
			ui->status->appendHtml(fNrStr + *i);
			ui->status->appendHtml(tr("&nbsp;&nbsp;&nbsp;&nbsp;<font color='orange'>Błąd odczytu daty EXIF (1)</font><br>"));
			fNr++;
			errCnt++;
			timToF->step();
			updateAvgTimeToFinish( timToF->getAvgTimeToFinish() );
			
			QFileInfo fiTmp(*i);
			totFsize -= fiTmp.size();
			updateFileSizeLabel( ui->totalSize, totFsize );
			
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
				fNr++;
				errCnt++;
				timToF->step();
				updateAvgTimeToFinish( timToF->getAvgTimeToFinish() );

				QFileInfo fiTmp(*i);
				totFsize -= fiTmp.size();
				updateFileSizeLabel( ui->totalSize, totFsize );

				QApplication::processEvents();
				continue;
			}

			// jezeli nie tworzymy nowych plikow a tylko wybrano zmiane nazwy to zmieniamy nazwe
			if( !ui->createOutputFiles->isChecked() )
			{
				if( !QFile::rename( *i, fPath + "/" + fName ) )
				{
					delete dt;
					ui->status->appendHtml(fNrStr + *i);
					ui->status->appendHtml(tr("&nbsp;&nbsp;&nbsp;&nbsp;<font color='red'>Błąd zmiany nazwy pliku</font><br>"));
					fNr++;
					errCnt++;
					timToF->step();
					updateAvgTimeToFinish( timToF->getAvgTimeToFinish() );

					QFileInfo fiTmp(*i);
					totFsize -= fiTmp.size();
					updateFileSizeLabel( ui->totalSize, totFsize );

					QApplication::processEvents();
					continue;
				}
			}
		}

		// tworzymy sciezke do katalogu docelowego + subfoldery
		bool copySuccess = true;
		if( ui->createOutputFiles->isChecked() )
		{
			// tworzymy subfoldery zwiazane z oryginlana struktura
			QString orgSub = "";
			if( ui->saveOrgSubfolders->isChecked() )
				orgSub = fPath.replace(srcFolder, "");	// czyscimy czesc wspolna i dodajemy do sciezki katalogu wynikowego
			fPath = ui->outputFolder->text();
			fPath += orgSub;
			if( ui->createOutputSubfolders->isChecked() )
			{
				QString subPath;
				if( !getSubfolderPath( *i, dt, subPath ) )
				{
					delete dt;
					ui->status->appendHtml(fNrStr + *i);
					ui->status->appendHtml(tr("&nbsp;&nbsp;&nbsp;&nbsp;<font color='orange'>Błąd odczytu daty EXIF (3)</font><br>"));
					fNr++;
					errCnt++;
					timToF->step();
					updateAvgTimeToFinish( timToF->getAvgTimeToFinish() );

					QFileInfo fiTmp(*i);
					totFsize -= fiTmp.size();
					updateFileSizeLabel( ui->totalSize, totFsize );

					QApplication::processEvents();
					continue;
				}
				fPath.append("\\" + subPath);
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

			if( !Utility::mkPath( fPath ) )
				ui->status->appendHtml(tr("&nbsp;&nbsp;&nbsp;&nbsp;<font color='red'>Błąd tworzenia ścieżki</font><br>"));
			copySuccess = QFile::copy( *i, fPath + "\\" + fName );
			if( !copySuccess )
				ui->status->appendHtml(tr("&nbsp;&nbsp;&nbsp;&nbsp;<font color='red'>Błąd kopiowania pliku</font><br>"));
		}
		else
			ui->status->appendHtml(fNrStr + QString(fPath + "\\" + fName).replace("/","\\").replace("\\\\","\\"));

		if( copySuccess )
		{
			if( changeFileTime( fPath + "\\" + fName, *dt ) )
				ui->status->appendHtml(tr("&nbsp;&nbsp;&nbsp;&nbsp;<font color='green'>OK</font><br>"));
			else
				ui->status->appendHtml(tr("&nbsp;&nbsp;&nbsp;&nbsp;<font color='red'>Błąd ustawiania daty</font><br>"));
		}

		// sumujemy rozmiar plikow
		convFilesSize += fi.size();
		updateFileSizeLabel( ui->sizeToFinish, convFilesSize );

		// TODO
		// dodajemy do listy pliki dodatkowe do kopiowania o ustalonych rozszerzeniach (jezeli istnieja)
		//if( ui->copyAdditionalFiles->isChecked() && ui->addFilesExt->text() != "" )
		//	addAdditionalFilesToCopy( *i, addFilesList );
		// \TODO

		delete dt;

		fNr++;
		timToF->step();
		updateAvgTimeToFinish( timToF->getAvgTimeToFinish() );
		QApplication::processEvents();
	}

	if( cancel )
		ui->status->appendHtml(tr("<font color='red'>Przerwano pracę na życzenie użytkownika</font><br>"));
	else
		ui->progressBar->setValue(100);
	delete convFilesSizeTim;
	convFilesSizeTim = nullptr;
	int elapsed = timToF->getTotalTime();
	delete timToF;
	timToF = nullptr;
	QString allOk = "";
	if( fNr-1-errCnt != fCnt )
		allOk = "<font color='red'>";
	else
		allOk = "<font color='green'>";
	ui->status->appendHtml(tr("<b>Przekonwertowano poprawnie ") + allOk + QString::number(fNr-1-errCnt) + "</font>" + tr(" z <font color='green'>") + QString::number(fCnt) +
		tr("</font> plików w ") + QString::number(elapsed/1000) + "." + QString::number(elapsed%1000) + tr(" sekundy.</font></b><br>"));

	// TODO
	/*if( ui->copyAdditionalFiles->isChecked() && ui->addFilesExt->text() != "" )
	{
		auto ret = QMessageBox::question( this, tr("Kopiowanie plików dodatkowych"), tr("Zakończono konwertowanie plików, czy rozpocząć kopiowanie plików dodatkowych o wybranych rozszerzeniach: ") + 
			ui->addFilesExt->text() + " ?", QMessageBox::Ok, QMessageBox::Cancel );
		if( ret == QMessageBox::Ok )
		{

		}
	}*/
	// \TODO

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
				case 'Y':
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
				case 'Y':
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
		auto dttmp = QDateTime::fromString(s[1].replace("\r\n",""), "yyyy:MM:dd hh:mm:ss");
		if( !dttmp.isNull() )
			return new QDateTime(dttmp);
		else
			return nullptr;
	}

	return nullptr;
}

QString MainWindow::getExifOutput( const QString& filePath, LANGUAGES lang, bool extendedInfo )
{
	if( filePath.isEmpty() || !QFile::exists(filePath) )
		return "";

	QProcess p;
	QString extendedInfoStr = "";
	if( ui->exifExtendedInfo->isChecked() || extendedInfo )
		extendedInfoStr = EXIV2_EXTENDED_INFO;
	QString startCmd = "\"" + QDir::currentPath().replace("/","\\") + "\\" + QString(EXIV2_BIN) + "\" " + extendedInfoStr + " \"" + filePath + "\"";
	p.start( startCmd );
	p.waitForFinished();
	if( lang == LANGUAGES::ENGLISH || ui->exifExtendedInfo->isChecked() || extendedInfo )
		return p.readAll();

	QString out;
	while( p.canReadLine() )
		out.push_back( translateExif( QString(p.readLine()), lang ) );
	return out;
}

ExifOrientation MainWindow::getExifOrientation( const QString& filePath )
{
	// TODO
	
	if( filePath.isEmpty() || !QFile::exists(filePath) )
		return EO_ERROR;

	QProcess p;
	QString extendedInfoStr = "";
	QString startCmd = "\"" + qApp->applicationDirPath().replace("/","\\") + "\\" + QString(EXIV2_BIN) + "\" " + EXIV2_GET_ORIENTATION + " \"" + filePath + "\"";
	p.start( startCmd );
	p.waitForFinished();

	QString out = "";
	while( p.canReadLine() )
	{
		QString r = p.readLine();
		if( r.contains(EXIV2_ORIENTATION_STR) )
		{
			out = r;
			break;
		}
	}

	ExifOrientation orient = EO_ERROR;
	//if( out == "" )


	return orient;
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

void MainWindow::restoreDefaultSettings( void )
{
	Preferences::Instance().restoreDefaultSettings(ui);

	/*auto p = Preferences::Instance();
	p.deserializeSettings(&p.defaultSettings);
	p.serializeSettings(ui);*/
}

void MainWindow::aboutToQuit( void )
{
	Preferences::Instance().serializeSettings(ui);
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

void MainWindow::updateAvgTimeToFinish( int timeToF )
{
	int min = 0;
	int sec = 0;
	if( timeToF != 0 )
	{
		min = (timeToF /1000) / 60;
		sec = (timeToF - (min * 60000))/1000;
	}
	ui->timeToFinish->setText( QString::number(min) + "m : " + QString::number(sec) + "s" );
}


void MainWindow::updateFileSizeLabel( QLabel* label, qint64 size )
{
	qint64 sMB = size/1000000;
	qint64 sKB = (size%1000000)/1000;
	label->setText( QString::number(sMB) + "." + QString::number(sKB) + " MB" );
}

void MainWindow::actionSetupUpdateSlot( void )
{
	ui->createOutputFiles->setChecked(false);
	ui->changeOutputFileName->setChecked(false);
}

void MainWindow::actionSetupOrgNewSlot( void )
{
	ui->createOutputFiles->setChecked(true);
	ui->saveOrgSubfolders->setChecked(true);
	ui->createOutputSubfolders->setChecked(false);
	ui->changeOutputFileName->setChecked(false);
}

void MainWindow::actionSetupNewDateSlot( void )
{
	ui->createOutputFiles->setChecked(true);
	ui->saveOrgSubfolders->setChecked(false);
	ui->createOutputSubfolders->setChecked(true);
	ui->changeOutputFileName->setChecked(true);
}

void MainWindow::actionSetupOrgNewDateSlot( void )
{
	ui->createOutputFiles->setChecked(true);
	ui->saveOrgSubfolders->setChecked(true);
	ui->createOutputSubfolders->setChecked(true);
	ui->changeOutputFileName->setChecked(true);
}

void MainWindow::actionChangeLang( void )
{
	ChangeLanguage* a = new ChangeLanguage(Preferences::Instance().language, this);
	a->setAttribute( Qt::WA_DeleteOnClose );
	int l = a->exec();
	if( l == ChangeLanguage::Rejected )
		return;
	changeLanguage((LANGUAGES)l);
}

void MainWindow::changeLanguage( LANGUAGES lang )
{
	auto& p = Preferences::Instance();
	p.language = lang;
	auto ret = QMessageBox::information( this, tr("Zmiana języka - restart"), tr("Aby zmienić język wymagane jest ponowne uruchomienie aplikacji. Czy chcesz uruchomić aplikację ponownie?"), QMessageBox::Ok, QMessageBox::Cancel );
	if( ret == QMessageBox::Cancel )
		return;
	//p.serializeSettings();
	restart();
}

void MainWindow::restart( void )
{
	qApp->exit(APP_RESTART_EXIT_CODE);
	//QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
}

void MainWindow::inputFileClicked( const QModelIndex& index, const QModelIndex& last )
{
	last;
	if( !index.isValid() )
	{
		ui->exifInfo->setText("");
		ui->imgPreview->setPixmap(QPixmap());
		ui->imgPreview->setText(tr("Podgląd"));
		selFilePath = "";
		return;
	}

	// *infoTab*
	ui->inputFilesList->scrollTo( index );
	QString path = index.data().toString();
	selFilePath = path;
	// wyciagamy sama sciezke
	path = path.right( path.count() - path.indexOf("</b>") - 4 );
	auto info = getExifOutput( path, Preferences::Instance().getLanguage() );
	ui->exifInfo->setText( info );

	// *previewTab*
	QPixmap px(path);

	//auto s = ui->imgPreview->geometry().bottomRight().x() - ui->imgPreview->geometry().bottomLeft().x();
	//px = px.scaledToWidth( s );

	auto s = ui->imgPreview->geometry().bottomRight().y() - ui->imgPreview->geometry().topRight().y();
	px = px.scaledToHeight( s );

	ui->imgPreview->setPixmap(px);
}

void MainWindow::createExifTranslationTable( void )
{
	TranslationSet t;

	t.eng = "File name";
	t.pol = "Nazwa pliku";
	exifTransTable.push_back(t);

	t.eng = "File size";
	t.pol = "Rozmiar pliku";
	exifTransTable.push_back(t);

	t.eng = "MIME type";
	t.pol = "Typ MIME";
	exifTransTable.push_back(t);

	t.eng = "Image size";
	t.pol = "Rozmiar obrazu";
	exifTransTable.push_back(t);

	t.eng = "Camera make";
	t.pol = "Producent aparatu";
	exifTransTable.push_back(t);

	t.eng = "Camera model";
	t.pol = "Model aparatu";
	exifTransTable.push_back(t);

	t.eng = "Image timestamp";
	t.pol = "Data wykonania";
	exifTransTable.push_back(t);

	t.eng = "Image number";
	t.pol = "Numer obrazu";
	exifTransTable.push_back(t);

	t.eng = "Czas ekspozycji";
	t.pol = "Nazwa pliku";
	exifTransTable.push_back(t);

	t.eng = "Przesłona";
	t.pol = "Nazwa pliku";
	exifTransTable.push_back(t);

	t.eng = "Exposure bias";
	t.pol = "Korekta ekspozycji";
	exifTransTable.push_back(t);

	t.eng = "Flash bias";
	t.pol = "Korekta flash";
	exifTransTable.push_back(t);

	t.eng = "Flash";
	t.pol = "Flash";
	exifTransTable.push_back(t);

	t.eng = "Focal length";
	t.pol = "Ogniskowa";
	exifTransTable.push_back(t);

	t.eng = "Subject distance";
	t.pol = "Odległość od przedmiotu";
	exifTransTable.push_back(t);

	t.eng = "ISO speed";
	t.pol = "Prędkość ISO";
	exifTransTable.push_back(t);

	t.eng = "Exposure mode";
	t.pol = "Tryb ekspozycji";
	exifTransTable.push_back(t);

	t.eng = "Metering mode";
	t.pol = "Tryb pomiaru";
	exifTransTable.push_back(t);

	t.eng = "Macro mode";
	t.pol = "Tryb makro";
	exifTransTable.push_back(t);

	t.eng = "Image quality";
	t.pol = "Jakość obrazu";
	exifTransTable.push_back(t);

	t.eng = "Exif Resolution";
	t.pol = "Rozdzielczość Exif";
	exifTransTable.push_back(t);

	t.eng = "White balance";
	t.pol = "Balans bieli";
	exifTransTable.push_back(t);

	t.eng = "Thumbnail";
	t.pol = "Miniatura";
	exifTransTable.push_back(t);

	t.eng = "Copyright";
	t.pol = "Prawa autorskie";
	exifTransTable.push_back(t);

	t.eng = "Exif comment";
	t.pol = "Komentarz Exif";
	exifTransTable.push_back(t);
}

QString MainWindow::translateExif( QString input, LANGUAGES lang )
{
	for( TranslationTable::iterator itr = exifTransTable.begin(); itr != exifTransTable.end(); ++itr )
	{
		if( input.contains( itr->eng ) )
		{
			if( lang == LANGUAGES::POLISH )
				return input.replace( itr->eng, itr->pol, Qt::CaseInsensitive );
		}
	}

	return "";
}

void MainWindow::imgPreviewMenuRequested( QPoint p )
{
	if( ui->imgPreview->pixmap() == 0 )
		return;
	imgPreviewMenu = new QMenu(this);
	imgPreviewMenu->addAction(tr("Obróć w lewo"), this, SLOT(imgRotateLeft()));
	imgPreviewMenu->addAction(tr("Obróć w prawo"), this, SLOT(imgRotateRight()));
	//imgPreviewMenu->addSeparator();
	//imgPreviewMenu->addAction(tr("Automatyczny obrót"), this, SLOT(imgRotateAuto()));
	connect( imgPreviewMenu, SIGNAL(aboutToHide()), imgPreviewMenu, SLOT(deleteLater()) );
	imgPreviewMenu->exec( ui->imgPreview->mapToGlobal(p) );
}

void MainWindow::imgRotateLeft( void )
{
	imgRotate(90);
}

void MainWindow::imgRotateRight( void )
{
	imgRotate(-90);
}

void MainWindow::imgRotate( int direction )
{
	QTransform trans;
	trans = trans.rotate(direction);
	auto px = ui->imgPreview->pixmap();
	QPixmap pxnew = px->transformed(trans);
	//QPixmap* pxnew = new QPixmap(px->transformed(trans));
	//auto s = ui->imgPreview->geometry().bottomRight().x() - ui->imgPreview->geometry().bottomLeft().x();
	//*pxnew = pxnew->scaledToWidth( s );
	ui->imgPreview->setPixmap(pxnew);
}

void MainWindow::imgRotateAuto()
{
	if( selFilePath == "" )
		return;
	//getExifOrientation
	//ui->imgPreview->pixmap()->fi
}

void MainWindow::exifExtendedInfoStateChanged( int state )
{
	auto inx = ui->inputFilesList->currentIndex();
	if( !inx.isValid() )
		return;
	QString path = inx.data().toString();
	// wyciagamy sama sciezke
	path = path.right( path.count() - path.indexOf("</b>") - 4 );
	QString info;
	
	if( state == Qt::Checked )	// extended info
		info = getExifOutput( path, Preferences::Instance().getLanguage(), true );
	else if( state == Qt::Unchecked )
		info = getExifOutput( path, Preferences::Instance().getLanguage(), false );
	else
		return;

	ui->exifInfo->setText( info );
}

bool MainWindow::jpegtrans_loselessRotate( const QString& inFilePath, const QString& outFilePath, int rotate, bool& perfectRotNotPossible, bool perfect )
{
	if( inFilePath.isEmpty() || !QFile::exists(inFilePath) )
		return "";

	QProcess p;
	perfectRotNotPossible = false;
	QString perf = "";
	if( perfect )
		perf = JPEGTRAN_LOSELESS_PERFECT;
	QString startCmd = "\"" + qApp->applicationDirPath().replace("/","\\") + "\\" + QString(JPEGTRAN_BIN) + "\" " + perfect + " " + JPEGTRAN_LOSELESS_ROT + 
		QString::number(rotate) + " \"" + inFilePath + "\"" + " \"" + outFilePath + "\"";
	p.start( startCmd );
	p.waitForFinished();
	auto ret = p.readAll();
	if( ret.contains(JPEGTRAN_LOSELESS_NOTPERFECT) )
	{
		perfectRotNotPossible = true;
		return false;
	}
	else if( ret.contains(JPEGTRAN_HELLO_STR) )
		return false;
	else if( QString::fromLatin1(ret) == "\r\n" )
		return true;

	return false;
}

void MainWindow::setExifToModificationDTchanged( int state )
{
	// grupa checkbox: setExifToModificationDT oraz setModificationToExifDT
	// dopuszczamy mozliwosc ekskluzywnego wyboru checkboxow albo brak jakiegokolwiek wyboru
	if( state == Qt::Checked )
		ui->setModificationToExifDT->setChecked( false );
}

void MainWindow::setModificationToExifDTchanged( int state )
{
	// grupa checkbox: setExifToModificationDT oraz setModificationToExifDT
	// dopuszczamy mozliwosc ekskluzywnego wyboru checkboxow albo brak jakiegokolwiek wyboru
	if( state == Qt::Checked )
		ui->setExifToModificationDT->setChecked( false );
}

bool MainWindow::getShiftedDateTime( const QString& templ, QDateTime* dt )
{
	if( templ == "" || dt == nullptr )
		return false;

	// rozdzielamy elementy
	QString tmp = templ.simplified();
	QStringList elems = tmp.split(" ", QString::SkipEmptyParts);
	for( QStringList::iterator elem = elems.begin(); elem != elems.end(); ++elem )
	{
		for( QString::iterator c = elem->begin(); c < elem->end(); ++c )
		{
			if( *c == '%' )
			{
				// najpierw sprawdzamy czy mamy poprawne oznaczenie daty i czasu
				++c;
				if( c == elem->end() )
					break;
				QChar symb = *c;
				if( !(symb == 'Y' || symb == 'M' || symb == 'D' || symb == 'h' || symb == 'm' || symb == 's') )
					return false;
				
				// sprawdzamy czy dodajemy czy odejmujemy
				++c;
				if( c == elem->end() )
					break;
				int plusMinus;
				if( *c == '+' )
					plusMinus = 1;
				else if( *c == '-' )
					plusMinus = -1;
				else
					return false;

				// wyciagamy cala liczbe po znaku dodawania lub odejmowania
				++c;
				if( c == elem->end() )
					break;
				QString shiftStr = "";
				for( ; c != elem->end(); ++c )
					shiftStr += *c;
				bool ok;
				int shift = shiftStr.toInt(&ok);
				if( !ok )
					return false;

				switch( symb.toLatin1() )
				{
				case 'Y':
					*dt = dt->addYears( shift * plusMinus );
					break;
				case 'M':
					*dt = dt->addMonths( shift * plusMinus );
					break;
				case 'D':
					*dt = dt->addDays( shift * plusMinus );
					break;
				case 'h':
					*dt = dt->addSecs( shift * plusMinus * 60 * 60 );
					break;
				case 'm':
					*dt = dt->addSecs( shift * plusMinus * 60 );
					break;
				case 's':
					*dt = dt->addSecs( shift * plusMinus );
					break;
				default:
					return false;
				}
			}
			else
				return false;
		}
	}
	return true;
}


// TODO
void MainWindow::addAdditionalFilesToCopy( const QString& filePath, QStringList& addFilesList )
{

}















TimeToFinish::TimeToFinish(int avgStepVar, int maxStepCnt) : tim(nullptr),
								avgStepVar_(avgStepVar),
								maxStepCnt_(maxStepCnt)
{
}

TimeToFinish::~TimeToFinish()
{
	if( tim != nullptr )
		delete tim;
}

void TimeToFinish::start( void )
{
	if( tim != nullptr )
		stop();
	tim = new QTime();
	tim->start();
}

void TimeToFinish::stop( void )
{
	if( tim != nullptr )
		delete tim;
	tim = nullptr;
	timeSteps.clear();
	avgTimeToFinish = 0;
}

bool TimeToFinish::step( void )
{
	if( tim == nullptr )
		return false;
	timeSteps.push_back(tim->elapsed());
	avgTimeToFinish = calcAvgTimeToFinish();
	tim->restart();
	return true;
}

int TimeToFinish::lastStepsTime( int avgStepVar ) const
{
	if( avgStepVar == -1 )
		avgStepVar = avgStepVar_;
	if( timeSteps.count() < avgStepVar )
		avgStepVar = timeSteps.count();
	int t = 0;
	for( int i = timeSteps.count()-avgStepVar; i < timeSteps.count(); i++ )
	{
		t += timeSteps[i];
	}

	return t;
}

int TimeToFinish::avgStepTimeFromLastNsteps( int avgStepVar ) const
{
	if( avgStepVar == -1 )
		avgStepVar = avgStepVar_;
	if( timeSteps.count() < avgStepVar )
		avgStepVar = timeSteps.count();
	int avg = 0;
	for( int i = timeSteps.count()-avgStepVar; i < timeSteps.count(); i++ )
	{
		avg += timeSteps[i];
	}
	avg = avg / avgStepVar;

	return avg;
}

int TimeToFinish::calcAvgTimeToFinish( void ) const
{
	//if( timeSteps.count() <= avgStepVar_ )	// czekamy na zebranie danych
	//	return 0;

	int avg = avgStepTimeFromLastNsteps();	// sredni czas wykonania na krok
	int stepsToFinish = maxStepCnt_ - timeSteps.count();	// ilosc krokow do konca
	return avg * stepsToFinish;
}

int TimeToFinish::getAvgTimeToFinish( void )
{
	return avgTimeToFinish;
}

int TimeToFinish::getTotalTime( void )
{
	int tot = 0;
	for( int i=0; i < timeSteps.count(); i++ )
		tot += timeSteps[i];
	return tot;
}















SizeSpeed::SizeSpeed( QLabel* qlabel, qint64* convFilesSize ) : label(qlabel), convFilesSize(convFilesSize)
{
	tim.setInterval(1000);
	connect( &tim, SIGNAL(timeout()), this, SLOT(timeout()) );
	tim.start();
	et.start();
}

SizeSpeed::~SizeSpeed()
{
	tim.stop();
	disconnect( &tim, SIGNAL(timeout()), this, SLOT(timeout()) );
}

void SizeSpeed::timeout( void )
{	
	auto delta = *convFilesSize - lastSize;
	if( delta < 0 )
		delta *= -1;
	sizeSpeedSteps.push_back(delta);

	qint64 speed = ((*convFilesSize) / (et.elapsed()/1000));
	qint64 sMB = speed/ /*nLast/*/ 1000000;
	qint64 sKB = ((speed /*/nLast*/ )%1000000)/1000;

	label->setText( QString::number(sMB) + "." + QString::number(sKB) + " MB/s" );
	lastSize = *convFilesSize;
}










void HTMLDelegate::paint(QPainter* painter, const QStyleOptionViewItem & option, const QModelIndex &index) const
{
	QStyleOptionViewItemV4 options = option;
	initStyleOption(&options, index);
	
	painter->save();
	
	QTextDocument doc;
	doc.setHtml(options.text);
	
	options.text = "";
	options.widget->style()->drawControl(QStyle::CE_ItemViewItem, &options, painter);
	
	// shift text right to make icon visible
	QSize iconSize = options.icon.actualSize(options.rect.size());
	painter->translate(options.rect.left()+iconSize.width(), options.rect.top());
	QRect clip(0, 0, options.rect.width()+iconSize.width(), options.rect.height());
	
	//doc.drawContents(painter, clip);
	
	painter->setClipRect(clip);
	QAbstractTextDocumentLayout::PaintContext ctx;
	// set text color to red for selected item
	if (option.state & QStyle::State_Selected)
		ctx.palette.setColor(QPalette::Text, QColor("red"));
	ctx.clip = clip;
	doc.documentLayout()->draw(painter, ctx);
	
	painter->restore();
}

QSize HTMLDelegate::sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
	QStyleOptionViewItemV4 options = option;
	initStyleOption(&options, index);
	
	QTextDocument doc;
	doc.setHtml(options.text);
	doc.setTextWidth(options.rect.width());
	return QSize(doc.idealWidth(), doc.size().height());
}






















InputFilesView::InputFilesView(QWidget* parent) :  QListView(parent)
{

}

InputFilesView::~InputFilesView()
{

}

void InputFilesView::currentChanged(const QModelIndex & current, const QModelIndex & previous)
{
	emit currentChangedSignal(current, previous);
}














Update::Update(QWidget* parent) : QWidget(parent)
{
	connect(&fdUpdate, SIGNAL(downloaded(QByteArray)), this, SLOT(updateDownloaded(QByteArray)));

	QString path = QDir::currentPath() + "/" + APP_UPDATE_FILE_CFG;
	QFile f( path );
	if( f.open( QIODevice::ReadOnly | QIODevice::Text ) )
	{
		QByteArray data = f.readLine();
		QUrl u(data);
		if( u.isValid() )
			updateUrl = u;
	}
	f.close();
}

Update::~Update()
{

}

void Update::updateDownloaded( QByteArray data )
{
	// parse data
	QString ver = data;
	bool brk = true;
}

void Update::checkUpdate( void )
{
	if( updateUrl.isValid() )
		fdUpdate.download(updateUrl);
}


