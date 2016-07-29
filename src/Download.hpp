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
#include <QFtp>
#include <QtNetwork/QNetworkReply>
#include <QFile>
#include <QDate>
#include <QMutex>
#include <QTimer>

#include "ApplicationData.hpp"

namespace CraneDM {

    class DownloadItem : public QObject
    {
        Q_OBJECT

    private:
        QUrl           url_;
        quint64        low_;
        quint64        high_;
        quint64        number_of_bytes_written_;
        unsigned int   use_range_;
        unsigned int   thread_number_;
        unsigned int   use_lock_;
        unsigned int   ftp_resuming_;
        QFile          *file_;
        QNetworkReply  *reply_;
        QFtp           *ftp_;
        QTime          timer;

        void flush();
        void flushImpl();
    public slots:
        void finishedHandler();
        void readyReadHandler();
        void errorHandler( QNetworkReply::NetworkError );
        void downloadProgressHandler( qint64, qint64 );
        void startDownload();
        void stopDownload();
        void onFtpReadyReadHandler();
        void onFtpRawCommandReply( int, QString const & );
        void onFtpCommandFinished( int, bool );
        void onFtpDone( bool );
    signals:
        void fileSizeObtained( quint64 );
        void completed();
        void stopped();
        void error( QString );
        void bytesWrittenStatus( unsigned int, qint64 );
        void progressStatus( unsigned int, int, double, QString );
    public:
        DownloadItem( QUrl url, qint64 low, qint64 high, qint64 bytes_written, QObject *parent = NULL );
        ~DownloadItem();

        void setThreadNumber( unsigned int number ) { thread_number_ = number; }
        void setFile( QFile *file ){ file_ = file; }
        void acceptRange( bool flag ){ use_range_ = flag; }
        QUrl url() const { return url_; }
        void useLock( bool ul = true ) { use_lock_ = ul; }
        void ftpResume( bool resuming ){ ftp_resuming_ = resuming; }
        static QNetworkAccessManager*   GetHttpNetworkManager() { return &http_network_manager; }
        static QNetworkAccessManager    http_network_manager;
        static QMutex mutex;
    };


    class DownloadComponent: public QObject
    {
        Q_OBJECT

    public:
        QFile                       *file;
        QList<DownloadItem*>        download_item_list;
        unsigned int                byte_range_specified;
        QSharedPointer<Information> download_information;
        unsigned int        max_number_of_threads;
        QString             download_directory;
        QTimer              timer;
    public slots:
        void errorHandler( QString );
        void errorHandler( QNetworkReply::NetworkError );
        void completionHandler();
        void headFinishedHandler();
        void updateBytesWritten( unsigned int, qint64 );
        void progressStatusHandler( unsigned int, int, double, QString );
        void startDownload();
        void cleanUpOnExit();
        void stopDownload();
        void onFileSizeObtained( quint64 );

    signals:
        void error( QString message, QString url );
        void completed( QString );
        void downloadStarted( QString );
        void progressChanged( QString );
        void stopped( QString );
        void closed();
    public:
        DownloadComponent( QString address, QString directory, unsigned int no_of_threads, QObject *parent = NULL );
        ~DownloadComponent();
        static QString GetFilename( QString const & url, QString const & parent_directory );

    private:
        void startDownloadImpl( int threads_to_use );
        void addNewHttpUrlImpl( QString const & url, QNetworkReply *response );
        void addNewFtpUrlImpl( QString const & url );
        void ftpDownloadFile();
        QUrl redirectUrl( QUrl const & possibleRedirectUrl, QUrl const & oldRedirectUrl) const;
        void connectSignalsCallbacks( DownloadItem*, QThread * );

        static bool fileExists( QString const & filename );
        static QString normalizeFilename( QString const & filename );
        static QString renameIfExistsFile( QString const & filename, QString const & directory );
        static QPair<QString, QString> splitFilenameExtension( QString const & filename );
        static void trimFileExtension( QString & file_extension );
    };
}
#endif /* DOWNLOAD_HPP_ */
