#ifndef FILEDOWNLOADER_H
#define FILEDOWNLOADER_H
 
#include <QObject>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
 
class FileDownloader : public QObject
{
 Q_OBJECT
 public:
  explicit FileDownloader(QObject *parent = 0);
  virtual ~FileDownloader();
  QByteArray downloadedData() const;
  void download( QUrl url );
 
 signals:
  void downloaded(QByteArray data);
 
 private slots:
  void fileDownloaded(QNetworkReply* pReply);
 
 private:
  QNetworkAccessManager m_WebCtrl;
  QByteArray m_DownloadedData;
  QUrl url;
};
 
#endif // FILEDOWNLOADER_H