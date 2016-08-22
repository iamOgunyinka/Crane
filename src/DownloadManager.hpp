/*
 * DownloadManager.hpp
 *
 *  Created on: Jun 17, 2016
 *      Author: Joshua
 */

#ifndef DOWNLOADMANAGER_HPP_
#define DOWNLOADMANAGER_HPP_

#include <QObject>
#include <QNetworkReply>
#include <QFile>
#include <QDate>
#include <QThread>
#include <QDebug>

#include "Download.hpp"
#include "ApplicationData.hpp"

namespace CraneDM {

    struct DownloadManager: public QObject
    {
        Q_OBJECT

        public slots:
        void completionHandler( QString );
        void errorHandler( QString, QString );
        void downloadStartedHandler( QString url );
        void progressHandler( QString );
        void onDownloadStopped( QString );
    signals:
        void error( QString, QString );
        void completed( QString );
        void status( QString );
        void progressed( QString );
        void downloadStarted( QString );
        void downloadStopped( QString );
    public:
        static QMap<QString, DownloadComponent*>   active_download_list;
        static QList<QString>                      inactive_downloads;
        static unsigned int                        max_number_of_downloads;
        static unsigned int                        max_number_of_threads;
    public:
        DownloadManager( QObject *parent = NULL );
        virtual ~DownloadManager();
    };

    struct CraneDownloader : public QObject
    {
        Q_OBJECT
    public:
        static QSharedPointer<DownloadManager> m_pDownloadManager;
        static void addNewUrlWithManager( QString const & url, QString directory, DownloadManager *dm );
    public:
        Q_INVOKABLE static void addNewUrl( QString const & url, unsigned int threads, unsigned int download_limit,
                QString directory = QString() );
        Q_INVOKABLE static void stopDownload( QString const & url, bool toPause );
        Q_INVOKABLE static void removeItem( QString const & url, bool deleteFile );
        CraneDownloader();
        ~CraneDownloader();
    signals:
        void error( QString );
        void downloadStarted( QString );
        void progressed( QString );
        void completed( QString );
    public slots:
        void errorHandler( QString, QString );
        void onDownloadCompleted( QString );
        void onDownloadStarted( QString );
        void progressHandler( QString );
        Q_INVOKABLE void aboutToExit();
    };
}
#endif /* DOWNLOADMANAGER_HPP_ */
