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

struct DownloadManager: public QObject
{
    Q_OBJECT

public slots:
    void finishedHandler( QString );
    void errorHandler( QString, QString );
    void downloadStartedHandler( QString url );
    void statusChangedHandler( QString url );
signals:
    void error( QString, QString );
    void finished( QString );
    void status( QString );
    void statusChanged( QString );
public:
    static QMap<QString, DownloadComponent*>   active_download_list;
    static QList<QString>                      inactive_downloads;
    static int                                 max_number_of_downloads;
    static int                                 max_number_of_threads;
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
    CraneDownloader();
    ~CraneDownloader();
signals:
    void error( QString );
    void status( QString );
    void statusChanged( QString );
public slots:
    void errorHandler( QString, QString );
    void statusHandler( QString );
    void statusChangedHandler( QString );
    Q_INVOKABLE void aboutToExit();
};
#endif /* DOWNLOADMANAGER_HPP_ */
