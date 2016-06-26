/*
 * NetworkManager.cpp
 *
 *  Created on: Jun 17, 2016
 *      Author: Joshua
 */

#include <src/DownloadManager.hpp>
#include <QNetworkRequest>
#include <QDir>
#include <QUuid>
#include <QFileInfo>
#include <src/DownloadInfo.hpp>

// static data initialized
QMap<QString, DownloadComponent*>   DownloadManager::active_download_list;
QList<QString>  DownloadManager::inactive_downloads;
int             DownloadManager::max_number_of_downloads = 2;
int             DownloadManager::max_number_of_threads = 2;

DownloadManager::DownloadManager( QObject *parent ): QObject( parent )
{
}

DownloadManager::~DownloadManager()
{

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

void DownloadManager::statusChangedHandler( QString url )
{
    emit statusChanged( url );
}

QSharedPointer<DownloadManager> CraneDownloader::m_pDownloadManager ( new DownloadManager );

CraneDownloader::CraneDownloader(): QObject( NULL )
{
    DownloadInfo download_info ( "data/download_info.json", this );
    download_info.readDownloadSettingsFile();

    QObject::connect( CraneDownloader::m_pDownloadManager.data(), SIGNAL( error( QString ) ),
            this, SLOT( errorHandler( QString ) ) );
    QObject::connect( CraneDownloader::m_pDownloadManager.data(), SIGNAL( status( QString ) ),
            this, SLOT( statusHandler( QString ) ) );
    QObject::connect( CraneDownloader::m_pDownloadManager.data(), SIGNAL( finished( QString ) ),
            this, SLOT( statusHandler( QString ) ) );
    QObject::connect( CraneDownloader::m_pDownloadManager.data(), SIGNAL( statusChanged( QString ) ),
            this, SLOT( statusChangedHandler( QString ) ) );
}

CraneDownloader::~CraneDownloader(){ }

void CraneDownloader::addNewUrlWithManager( QString const & address, QString location, DownloadManager *dm )
{
	QThread *new_download_thread = new QThread();
	DownloadComponent *new_download = new DownloadComponent( address, location,
	        DownloadManager::max_number_of_threads, new_download_thread );

	DownloadManager::active_download_list.insert( address, new_download );

	QObject::connect( new_download_thread, SIGNAL( started() ), new_download, SLOT( startDownload() ) );
    QObject::connect( new_download, SIGNAL( statusChanged( QString ) ), CraneDownloader::m_pDownloadManager.data(),
            SLOT( statusChangedHandler( QString ) ) );
	QObject::connect( new_download, SIGNAL( error( QString )), dm, SLOT( errorHandler( QString )) );
	QObject::connect( new_download, SIGNAL( downloadStarted( QString )), dm,
	        SLOT( downloadStartedHandler( QString )) );
	QObject::connect( new_download, SIGNAL( finished( QString ) ), dm, SLOT( finishedHandler( QString ) ) );
	QObject::connect( new_download, SIGNAL( finished( QString ) ), new_download_thread, SLOT( quit() ) );
	QObject::connect( new_download_thread, SIGNAL( finished() ), new_download_thread, SLOT( deleteLater() ) );

	new_download_thread->start();
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

	if( DownloadManager::inactive_downloads.contains( address ) ){
	    qDebug() << "Don't worry, we've queued the download for you already.";
	    return;
	}

    addNewUrlWithManager( address, location, CraneDownloader::m_pDownloadManager.data() );
}

void CraneDownloader::errorHandler( QString what )
{
    emit error( what );
}

void CraneDownloader::statusHandler( QString message )
{
    emit status( message );
}

void CraneDownloader::statusChangedHandler( QString url )
{
    emit statusChanged( url );
}
