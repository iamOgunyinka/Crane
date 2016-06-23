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

class DownloadItem : public QObject
{
    Q_OBJECT

private:

    QUrl           url_;
    qint64         low_;
    qint64         high_;
    qint64         number_of_bytes_written;
    qint64         use_range;
    unsigned int   thread_number_;
    QFile          *file_;
    QNetworkReply  *reply_;

    void flush();
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
public:
    DownloadItem( QUrl url, qint64 low, qint64 high, QObject *parent = NULL );
    ~DownloadItem();

    void setThreadNumber( unsigned int number ) { thread_number_ = number; }
    void setFile( QFile *file ){ file_ = file; }
    void acceptRange( bool flag ){ use_range = flag; }
    QUrl url() const { return url_; }

    static QNetworkAccessManager* GetNetworkManager() { return &network_manager; }
    static QNetworkAccessManager network_manager;
    static QMutex mutex;
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
    unsigned int        use_lock; // if we're using a single thread, there's no need for synchronization, so no locking.
    unsigned int        accept_ranges;
    unsigned int        byte_range_specified;
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
