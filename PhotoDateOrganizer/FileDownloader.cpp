#include "FileDownloader.h"
 
FileDownloader::FileDownloader(QObject *parent) :
 QObject(parent)
{
 connect(
  &m_WebCtrl, SIGNAL (finished(QNetworkReply*)),
  this, SLOT (fileDownloaded(QNetworkReply*))
  );
}
 
FileDownloader::~FileDownloader() 
{ 
	disconnect(
  &m_WebCtrl, SIGNAL (finished(QNetworkReply*)),
  this, SLOT (fileDownloaded(QNetworkReply*))
  );
}
 
void FileDownloader::fileDownloaded(QNetworkReply* pReply) {
 m_DownloadedData = pReply->readAll();
 int statusCode = pReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
 //QString errStr = pReply->errorString();
 //emit a signal
 pReply->deleteLater();
 emit downloaded(this->downloadedData());
}
 
QByteArray FileDownloader::downloadedData() const {
 return m_DownloadedData;
}

void FileDownloader::download( QUrl url )
{
	QNetworkRequest request(url);
	m_WebCtrl.get(request);
}