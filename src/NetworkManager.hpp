/*
 * NetworkManager.hpp
 *
 *  Created on: Jun 17, 2016
 *      Author: Joshua
 */

#ifndef NETWORKMANAGER_HPP_
#define NETWORKMANAGER_HPP_

#include <QObject>
#include <QNetworkReply>
#include <QFile>
#include <QDate>
#include <QThread>
#include <QDebug>

#include "Download.hpp"
#include "DownloadInfo.hpp"

struct NetworkManager: public QObject
{
    Q_OBJECT

public slots:
    void headFinishedHandler();
    void newDownloadStarted();
    void finishedHandler( QString );
    void errorHandler( QString );
    void errorHandler( QNetworkReply::NetworkError );
signals:
    void error( QString );
    void finished( QString );
private:
    QMap<QString, DownloadComponent*>   active_download_list;
    QList<QString>                      inactive_downloads;
    int                                 max_number_of_threads;
    int                                 max_number_of_downloads;
    QString                             download_directory;

    QUrl redirectUrl( QUrl const & possibleRedirectUrl, QUrl const & oldRedirectUrl) const;
    void startDownloadImpl( DownloadComponent *download_component, unsigned int threads_to_use,
            QList<Information::ThreadInfo> thread_info_list = QList<Information::ThreadInfo>() );
    void addNewUrlImpl( QString const & url, QNetworkReply *response );
public:
    Q_INVOKABLE void addNewUrl( QString const & url, int threads, int download_limit, QString directory = QString() );
public:
    NetworkManager( QObject *parent = NULL );
    virtual ~NetworkManager();

    static QString GetFilename( QUrl const & url, QString const & parent_directory );
};
#endif /* NETWORKMANAGER_HPP_ */
