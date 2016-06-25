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

#include "DownloadInfo.hpp"

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

    void flush();
    void flushImpl();
public slots:
    void finishedHandler();
    void readyReadHandler();
    void downloadProgressHandler( qint64, qint64 );
    void startDownload();
    void stopDownload();
signals:
    void finished( unsigned int );
    void stopped( unsigned int );
    void error( QString );
    void status( unsigned int, qint64 );
public:
    DownloadItem( QUrl url, qint64 low, qint64 high, QObject *parent = NULL );
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

    struct ThreadData {
        Information::ThreadInfo thread_info;
        unsigned int thread_completed;
    };
public:
    QString             old_url;
    QString             new_url;
    QFile               *file;
    QList<QThread*>     download_threads;
    QList<ThreadData>   thread_data;
    qint64              size_in_bytes;
    unsigned int        accept_ranges;
    unsigned int        byte_range_specified;

    unsigned int        max_number_of_threads;
    QString             download_directory;

    QDateTime           time_started;
    QDateTime           time_completed;
public slots:
    void errorHandler( QString );
    void errorHandler( QNetworkReply::NetworkError );
    void finishedHandler( unsigned int );
    void headFinishedHandler();
    void updateThread( unsigned int, qint64 );
    void updateDownloadInfo( Information *, bool isCompleted = false );
    void updateCreateDownloadInfo();
    void startDownload();
    void stopDownload( QString address );
signals:
    void error( QString );
    void finished( QString );
    void downloadStarted( QString );
public:
    DownloadComponent( QString address, QString directory, unsigned int no_of_threads, QObject *parent = NULL );
    ~DownloadComponent();
    static QString GetFilename( QString const & url, QString const & parent_directory );
private:
    void startDownloadImpl( unsigned int threads_to_use,
            QList<Information::ThreadInfo> thread_info_list = QList<Information::ThreadInfo>() );
    void addNewUrlImpl( QString const & url, QNetworkReply *response );
    QUrl redirectUrl( QUrl const & possibleRedirectUrl, QUrl const & oldRedirectUrl) const;
    Information* find( QString const & url );
};
#endif /* DOWNLOAD_HPP_ */
