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

struct NetworkManager: public QObject
{
    Q_OBJECT

public slots:
    void headFinishedHandler();
    void finishedHandler( QString );
    void errorHandler( QString );
    void errorHandler( QNetworkReply::NetworkError );
signals:
    void error( QString );
    void finished( QString );
private:
    QMap<QString, DownloadComponent*>   active_download_list;
    int                                 number_of_threads;
    QString                             download_directory;

    QUrl redirectUrl( QUrl const & possibleRedirectUrl, QUrl const & oldRedirectUrl) const;
    void addNewUrlImpl( QUrl const & url );
public:
    Q_INVOKABLE void addNewUrl( QString const & url, int threads, QString directory = QString() );
public:
    NetworkManager( QObject *parent = NULL );
    virtual ~NetworkManager();

    static QString GetFilename( QUrl const & url, QString const & parent_directory );
};
#endif /* NETWORKMANAGER_HPP_ */
