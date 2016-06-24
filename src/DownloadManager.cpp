/*
 * NetworkManager.cpp
 *
 *  Created on: Jun 17, 2016
 *      Author: adonai
 */

#include <src/DownloadManager.hpp>
#include <QNetworkRequest>
#include <QDir>
#include <QUuid>
#include <QFileInfo>
#include <src/DownloadInfo.hpp>

QMap<QString, DownloadComponent*>   DownloadManager::active_download_list;
QList<QString>           DownloadManager::inactive_downloads;
unsigned int             DownloadManager::max_number_of_downloads = 2;
unsigned int             DownloadManager::max_number_of_threads = 2;

DownloadManager::DownloadManager( QObject *parent ): QObject( parent )
{
}

DownloadManager::~DownloadManager()
{

}

CraneDownloader::CraneDownloader(): QObject( NULL )
{
    QThread      *new_thread = new QThread( this );
    DownloadInfo *download_info = new DownloadInfo( "data/download_info.db", new_thread );

    QObject::connect( new_thread, SIGNAL( started() ), download_info, SLOT( readDownloadSettingsFile() ) );
    QObject::connect( download_info, SIGNAL( error( QString )), this, SLOT( errorHandler( QString ) ) );
    QObject::connect( download_info, SIGNAL( finished() ), new_thread, SLOT(quit() ) );
    QObject::connect( download_info, SIGNAL( error( QString )), new_thread, SLOT( quit() ) );
    QObject::connect( new_thread, SIGNAL( finished() ), new_thread, SLOT( deleteLater() ) );

    new_thread->start();

}

void CraneDownloader::addNewUrl( QString const & address, unsigned int threads_to_use,
        unsigned int download_limit, QString location )
{
    DownloadManager::max_number_of_downloads = download_limit;
    DownloadManager::max_number_of_threads = threads_to_use;

	if( DownloadManager::active_download_list.size() >= DownloadManager::max_number_of_downloads ){
        DownloadManager::inactive_downloads.push_back( address );
        qDebug() << "New download queued up";
        return;
    }

	QThread *new_download_thread = new QThread();
	DownloadComponent *new_download = new DownloadComponent( address, location, threads_to_use, new_download_thread );
	DownloadManager *download_manager = new DownloadManager;

	DownloadManager::active_download_list.insert( address, new_download );

	QObject::connect( new_download_thread, SIGNAL( started() ), new_download, SLOT( startDownload() ) );
	QObject::connect( new_download, SIGNAL( error( QString )), download_manager, SLOT( errorHandler( QString )) );
	QObject::connect( new_download, SIGNAL( downloadStarted( QString )), download_manager,
	        SLOT( downloadStartedHandler( QString )) );
	QObject::connect( new_download, SIGNAL( finished( QString ) ), download_manager, SLOT( finishedHandler( QString ) ) );
	QObject::connect( new_download, SIGNAL( finished( QString ) ), new_download_thread, SLOT( quit() ) );
	QObject::connect( new_download_thread, SIGNAL( finished() ), new_download_thread, SLOT( deleteLater() ) );

	new_download_thread->start();
}

void DownloadManager::errorHandler( QString what )
{
    qDebug() << "Error: " << what;
    emit error( what );
}

void DownloadManager::finishedHandler( QString url )
{
    DownloadManager::active_download_list.remove( url ); // belongs to a deleted thread

    emit status( url + " downloaded successfully" );
    qDebug() << url << " downloaded successfully";
    if( DownloadManager::inactive_downloads.size() > 0 ){
        QString next_download = inactive_downloads.front();
        DownloadManager::inactive_downloads.pop_front();
        CraneDownloader::addNewUrl( next_download, DownloadManager::max_number_of_threads, DownloadManager::max_number_of_downloads );
    }
}

void DownloadManager::downloadStartedHandler( QString url )
{
    qDebug() << "Download started on " << url;
    emit status( "Download started on " + url );
}

void CraneDownloader::errorHandler( QString what )
{
    emit error( what );
}

void CraneDownloader::statusHandler( QString message )
{
    emit status( message );
}
