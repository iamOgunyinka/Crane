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
#include <QReadWriteLock>

typedef unsigned int download_range;

class DownloadItem : public QObject
{
    Q_OBJECT

private:

    QUrl           url_;
    download_range low_;
    download_range high_;
    unsigned int   thread_number_;
    unsigned int   number_of_bytes_written;
    QFile          *file_;
    QNetworkReply  *reply_;


public slots:
    void finishedHandler();
    void readyReadHandler();
    void downloadProgressHandler( qint64, qint64 );
    void startDownload();
signals:
    void finished( unsigned int );
    void error( QString );
public:
    DownloadItem( QUrl url, download_range low, download_range high, QObject *parent = NULL );
    ~DownloadItem();
    void setThreadNumber( unsigned int number ) { thread_number_ = number; }
    void setFile( QFile *file ){ file_ = file; }
    QUrl url() const { return url_; }

    static QNetworkAccessManager* GetNetworkManager() { return &network_manager; }
    static QNetworkAccessManager network_manager;
    static QReadWriteLock rw_lock;
};

class DownloadComponent: public QObject
{
    Q_OBJECT

public:
    QUrl                url;
    QFile               *file;
    QList<QThread*>     download_threads;
    QList<unsigned int> threads;
    unsigned int        size_in_bytes;
    unsigned int        accept_ranges;
    QDateTime           time_started;
    QDateTime           time_completed;
public slots:
    void errorHandler( QString );
    void finishedHandler( unsigned int );
signals:
    void error( QString );
    void finished( QString );
public:
    DownloadComponent( QObject *parent = NULL );
};
#endif /* DOWNLOAD_HPP_ */
