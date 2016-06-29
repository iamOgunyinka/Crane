/*
 * Download.hpp
 *
 *  Created on: Jun 14, 2016
 *      Author: Joshua
 */

#ifndef DOWNLOAD_HPP_
#define DOWNLOAD_HPP_

#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QFile>
#include <QDate>
#include <QMutex>
#include <QTimer>

#include "ApplicationData.hpp"

class DownloadItem : public QObject
{
    Q_OBJECT

private:
    QUrl           url_;
    qint64         low_;
    qint64         high_;
    qint64         number_of_bytes_written_;
    unsigned int   use_range_;
    unsigned int   thread_number_;
    unsigned int   use_lock_;
    QFile          *file_;
    QNetworkReply  *reply_;
    QTime          timer;

    void flush();
    void flushImpl();
public slots:
    void finishedHandler();
    void readyReadHandler();
    void downloadProgressHandler( qint64, qint64 );
    void startDownload();
    void stopDownload();
signals:
    void finished();
    void stopped( unsigned int );
    void error( QString );
    void status( unsigned int, qint64 );
    void status( int, double, QString );
public:
    DownloadItem( QUrl url, qint64 low, qint64 high, qint64 bytes_written, QObject *parent = NULL );
    ~DownloadItem();

    void setThreadNumber( unsigned int number ) { thread_number_ = number; }
    void setFile( QFile *file ){ file_ = file; }
    void acceptRange( bool flag ){ use_range_ = flag; }
    QUrl url() const { return url_; }
    void useLock( bool ul = true ) { use_lock_ = ul; }

    static QNetworkAccessManager* GetNetworkManager() { return &network_manager; }
    static QNetworkAccessManager network_manager;
    static QMutex mutex;
};


class DownloadComponent: public QObject
{
    Q_OBJECT

public:
    QFile                       *file;
    QList<QThread*>             download_threads;
    unsigned int                byte_range_specified;
    QSharedPointer<Information> download_information;

    unsigned int        max_number_of_threads;
    QString             download_directory;
    QTimer              timer;
public slots:
    void errorHandler( QString );
    void errorHandler( QNetworkReply::NetworkError );
    void finishedHandler();
    void headFinishedHandler();
    void updateThread( unsigned int, qint64 );
    void statusHandler( int, double, QString );
    void startDownload();
    void cleanUpOnExit();
    void stopDownload();
signals:
    void error( QString message, QString url );
    void finished( QString );
    void downloadStarted( QString );
    void statusChanged( QString );
    void stopped();
    void closed();
public:
    DownloadComponent( QString address, QString directory, unsigned int no_of_threads, QObject *parent = NULL );
    ~DownloadComponent();
    static QString GetFilename( QString const & url, QString const & parent_directory );

private:
    void startDownloadImpl( int threads_to_use );
    void addNewUrlImpl( QString const & url, QNetworkReply *response );
    QUrl redirectUrl( QUrl const & possibleRedirectUrl, QUrl const & oldRedirectUrl) const;
    Information* find( QString const & url );
    QSharedPointer<Information> urlSearch( QString const & url );
};
#endif /* DOWNLOAD_HPP_ */
